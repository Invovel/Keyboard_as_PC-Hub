// SPDX-License-Identifier: GPL-3.0-or-later
//
// FT3168 capacitive touch driver for Waveshare ESP32-S3-Touch-AMOLED-1.43
// Based on firmware/boards/waveshare_s3_amoled_1_43/board/touch.cpp from
// touchy-pad branch board-waveshare-s3-amoled-1-43 (not in upstream
// touchy-pad). The FT3168 register protocol there is Waveshare's, from their
// touch_bsp.c.
//
// Single-touch only (FT3168 doesn't support multi-touch on this board).
// I2C address 0x38, no RST/INT wired.
//
// Implements the laptop-touchpad gesture vocabulary, following libinput's
// evdev-mt-touchpad-tap.c. This is the model from that same branch, not
// upstream touchy-pad's: upstream recognises a hold by dwell (tap_time +
// hold_time inside tap_distance), which on a trackpad means a resting finger
// presses the button. README lists what the gestures do; what follows is the
// one part of it that is not obvious from the outside.
//
// The pivot is that a tap sends the button *press* immediately but defers the
// *release* by GEST_TAP_DRAG_TIME_MS. If another touch lands inside that window
// the release is simply never sent: the button stays down and the gesture
// becomes one continuous drag instead of a second click. A bare hold with no
// preceding tap is only pointer movement - dwelling on the glass must never
// press a button on a trackpad.

#include "touch.h"
#include "backlight.h"
#include "display.h"
#include "usb_mouse.h"

#include "esp_timer.h"
#include "driver/i2c_master.h"

#define I2C_PORT     I2C_NUM_0
#define I2C_SDA      GPIO_NUM_47
#define I2C_SCL      GPIO_NUM_48
// 100 kHz, not 400: at 400 kHz the FT3168 sometimes clocked out a stale byte
// mid-burst. See docs/hardware-notes.md.
#define I2C_CLK_HZ   100000
#define TOUCH_ADDR   0x38

// Gesture detection configuration. Only GEST_TAP_TIME_MS comes from upstream
// touchy-pad; the rest were tuned for this panel on the
// board-waveshare-s3-amoled-1-43 branch, in pages/trackpad.py and
// trackpad_widget.cpp. See PORTING.md.
const uint32_t GEST_TAP_TIME_MS = 150;        // max time for a tap
const uint32_t GEST_TAP_DISTANCE = 4;         // max movement for a tap, and the
                                              // dead zone before the pointer moves (px)
const uint32_t GEST_TAP_DRAG_TIME_MS = 220;   // how long a tap's release is deferred
const uint32_t GEST_TAP_DRAG_DISTANCE = 100;  // how close the next press must land to
                                              // where the tap lifted (px). Far wider than
                                              // GEST_TAP_DISTANCE: a fingertip covers a third
                                              // of this panel, so the reported point drifts.
const uint32_t GEST_DOUBLE_CLICK_GAP_MS = 40; // gap before the synthesised second click

// How long a click stays armed after the pointer last moved.
//
// The controller occasionally reports a contact nobody made, indistinguishable
// from a real tap in every property except context: a genuine click has pointer
// motion right behind it, a phantom arrives out of silence. So the click is
// gated on recent motion. See docs/hardware-notes.md.
const uint32_t GEST_CLICK_ARM_WINDOW_MS = 30000;

// Touch state machine states
enum class TouchState {
    IDLE,           // No touch
    PRESSED,        // Finger down, may still become a tap
    DRAGGING,       // Finger claimed a tap's pending release: button is held
};

// Current touch state
static TouchState s_state = TouchState::IDLE;
static TouchPoint s_current = {0, 0, false};
static TouchPoint s_previous = {0, 0, false};
static TouchPoint s_tap_start = {0, 0, false};

// Gesture bookkeeping
static uint32_t s_press_time = 0;       // when the current touch went down
static bool s_left_dead_zone = false;   // current touch travelled past GEST_TAP_DISTANCE
static bool s_tap_button_down = false;  // left button held by a tap, release still pending
static uint32_t s_tap_release_at = 0;   // deadline for that pending release
static int16_t s_last_tap_x = 0;        // where the last tap lifted
static int16_t s_last_tap_y = 0;
static bool s_dbl_click_pending = false;  // second click of a double-tap is scheduled
static uint32_t s_dbl_click_at = 0;
// Deadline, not a timestamp, so it compares with due() like the other two and
// needs no companion flag for "has never moved": zero is already in the past
// at boot, which is the correct answer there.
static uint32_t s_click_armed_until = 0;

// Deadline comparison that survives the 49-day wrap of millis().
static bool due(uint32_t now, uint32_t deadline)
{
    return static_cast<int32_t>(now - deadline) >= 0;
}

// Touch device handle
static i2c_master_bus_handle_t s_i2c_bus = nullptr;
static i2c_master_dev_handle_t s_touch_dev = nullptr;

static uint32_t millis()
{
    return static_cast<uint32_t>(esp_timer_get_time() / 1000);
}

// Read TD_STATUS and the first touch point in one transaction:
// frame[0] = finger count, then XH, XL, YH, YL, weight, area.
static esp_err_t read_frame(uint8_t frame[7])
{
    uint8_t reg = 0x02;
    return i2c_master_transmit_receive(s_touch_dev, &reg, 1, frame, 7, 10);
}

// The coordinates are 12 bit. The top nibble of XH carries the event flag and
// the top nibble of YH the touch id, so both are masked off here.
static void decode_frame(const uint8_t frame[7], uint16_t *x, uint16_t *y)
{
    *x = static_cast<uint16_t>(((static_cast<uint16_t>(frame[1]) & 0x0F) << 8) | frame[2]);
    *y = static_cast<uint16_t>(((static_cast<uint16_t>(frame[3]) & 0x0F) << 8) | frame[4]);
}

// How often to re-issue the mode write while the controller has yet to report
// a contact. Also the worst case for how long after boot the pad stays dead if
// the power-up race below is lost.
#define TOUCH_REINIT_GAP_MS  500

// Put the FT3168 into the state this firmware wants: normal (polling) mode in
// register 0x00, and no automatic monitor mode in 0x86.
//
// There is deliberately no read-back check here. Register 0x00 is DEVICE_MODE
// and its power-up value is already 0x00, so reading 0x00 back says nothing
// about whether the part was awake enough to act on the write - a check that
// cannot fail is worse than no check, because it reads like a guarantee. What
// stands in for it is the retry in read_touch(): the only trustworthy evidence
// that this controller is configured is that it eventually reports a finger.
// Both writes go out on those same terms, so the retry covers them equally.
static void touch_set_normal_mode(void)
{
    const uint8_t normal_mode[2] = {0x00, 0x00};
    i2c_master_transmit(s_touch_dev, normal_mode, sizeof(normal_mode), 100);

    // Pin the part in active mode: register 0x86 (CTRL) = 0x00.
    //
    // It comes up on 0x01, which drops it into monitor mode after the 10 s of
    // touch idleness held in 0x87. Monitor mode is a state this firmware can
    // neither observe nor steer out of, and the datasheet says no coordinates
    // are reported while the part is in it - which from the host is the exact
    // signature of known issue 1: healthy bus, well-formed frames, zero
    // fingers forever. The only documented exit is a valid touch. So this
    // write removes the state rather than coping with it.
    //
    // Prophylaxis, not recovery: it lands while the part is freshly powered
    // and active, and does nothing for one that has already wedged. It costs
    // 1.5 mA against 30 uA, which on a bus-powered device is nothing.
    // See docs/known-issues.md, candidate d.
    const uint8_t ctrl_active[2] = {0x86, 0x00};
    i2c_master_transmit(s_touch_dev, ctrl_active, sizeof(ctrl_active), 100);
}

// Has the controller ever reported a contact since boot? Until it has, we have
// no evidence it took the mode write, so keep re-issuing it.
//
// This is the only thing standing between a lost power-up race and a dead pad.
// The controller shares the display flex, so the panel reset in display_init()
// leaves it still powering up when touch_init() reaches it, and it ACKs its
// I2C address well before it can act on a register write. Losing that race is
// terminal and near-invisible: every read returns a well-formed frame
// reporting zero fingers, so the bus looks perfectly healthy.
//
// Detecting readiness at init instead is not an option - there is no signal
// that reliably says so - and a settle delay before the first write is
// deliberately *not* here: this retry was measured to be sufficient on its own.
// See docs/hardware-notes.md.
static bool s_seen_finger = false;

static void reinit_until_it_answers(uint32_t now)
{
    static uint32_t s_next = 0;
    if (!due(now, s_next)) return;
    s_next = now + TOUCH_REINIT_GAP_MS;
    touch_set_normal_mode();
}

// Read touch data from FT3168
static bool read_touch(int16_t *x, int16_t *y)
{
    static int64_t s_last_read_us = 0;
    const int64_t now_us = esp_timer_get_time();

    // The FT3168 updates at ~60 Hz; polling faster only re-reads the same frame.
    if (s_last_read_us != 0 && (now_us - s_last_read_us) < 10000) {
        return s_current.pressed;
    }
    s_last_read_us = now_us;

    uint8_t frame[7] = {};
    if (read_frame(frame) != ESP_OK) {
        return false;
    }

    // TD_STATUS carries the contact count in its low nibble; the high nibble is
    // reserved and reads zero in every good frame.
    if ((frame[0] & 0x0F) == 0) {
        if (!s_seen_finger) {
            reinit_until_it_answers(static_cast<uint32_t>(now_us / 1000));
        }
        return false;
    }
    s_seen_finger = true;

    uint16_t rx, ry;
    decode_frame(frame, &rx, &ry);

    // Clamp into the panel, so an impossible coordinate can never reach the
    // gesture machine.
    if (rx >= DISPLAY_WIDTH)  rx = DISPLAY_WIDTH - 1;
    if (ry >= DISPLAY_HEIGHT) ry = DISPLAY_HEIGHT - 1;

    *x = static_cast<int16_t>(rx);
    *y = static_cast<int16_t>(ry);
    return true;
}

void touch_init(void)
{
    i2c_master_bus_config_t bus_cfg = {};
    bus_cfg.clk_source = I2C_CLK_SRC_DEFAULT;
    bus_cfg.i2c_port = I2C_PORT;
    bus_cfg.scl_io_num = I2C_SCL;
    bus_cfg.sda_io_num = I2C_SDA;
    bus_cfg.glitch_ignore_cnt = 7;
    bus_cfg.flags.enable_internal_pullup = true;
    ESP_ERROR_CHECK(i2c_new_master_bus(&bus_cfg, &s_i2c_bus));

    i2c_device_config_t dev_cfg = {};
    dev_cfg.dev_addr_length = I2C_ADDR_BIT_LEN_7;
    dev_cfg.device_address = TOUCH_ADDR;
    dev_cfg.scl_speed_hz = I2C_CLK_HZ;
    ESP_ERROR_CHECK(i2c_master_bus_add_device(s_i2c_bus, &dev_cfg, &s_touch_dev));

    touch_set_normal_mode();
}

// Clamp a pixel delta into the int8_t range an HID mouse report carries.
// Casting straight to int8_t would wrap a fast swipe into the opposite
// direction.
static int8_t clamp_delta(int32_t d)
{
    if (d > 127) return 127;
    if (d < -127) return -127;
    return static_cast<int8_t>(d);
}

// Read the hardware, run the gesture state machine, emit USB HID reports.
void touch_process(void)
{
    static bool s_was_pressed = false;
    bool is_pressed = read_touch(&s_current.x, &s_current.y);
    s_current.pressed = is_pressed;

    const uint32_t now = millis();

    // --- Deferred work: a tap's pending release, and the second click of
    // a double-tap. Both are deadlines rather than timers because this
    // function is already polled every few ms.
    if (s_tap_button_down && s_state != TouchState::DRAGGING && due(now, s_tap_release_at)) {
        usb_mouse_button_up(1);
        s_tap_button_down = false;
    }
    if (s_dbl_click_pending && due(now, s_dbl_click_at)) {
        s_dbl_click_pending = false;
        usb_mouse_click(1);
    }

    // --- Press: Finger just touched down ---
    if (is_pressed && !s_was_pressed) {
        // Activity: relight the panel if the idle timer blanked it, and
        // restart the countdown. The touch is not consumed by this - it goes
        // straight on into the gesture machine below. Swallowing the first
        // stroke after a pause would be a worse artefact than the screen
        // having been dark for it.
        backlight_wake();

        s_press_time = now;
        s_tap_start = s_current;  // Save location for distance check
        s_left_dead_zone = false;
        s_state = TouchState::PRESSED;

        // Tap-hold: a touch landing while a tap's release is still pending,
        // close to where that tap lifted, claims the release. Nothing is
        // sent - the button simply stays down, so the host sees one
        // continuous press rather than a click followed by a second press.
        if (s_tap_button_down) {
            int32_t dx = s_current.x - s_last_tap_x;
            int32_t dy = s_current.y - s_last_tap_y;
            if (dx < 0) dx = -dx;
            if (dy < 0) dy = -dy;
            if (dx <= (int32_t)GEST_TAP_DRAG_DISTANCE && dy <= (int32_t)GEST_TAP_DRAG_DISTANCE) {
                s_state = TouchState::DRAGGING;
            }
        }
    }
    // --- Release: Finger just lifted ---
    else if (!is_pressed && s_was_pressed) {
        const uint32_t touch_duration = now - s_press_time;
        const bool is_tap = !s_left_dead_zone && touch_duration <= GEST_TAP_TIME_MS;

        if (s_state == TouchState::DRAGGING) {
            // This touch owned the button, either because it dragged or
            // because it turned out to be the second tap of a double-tap.
            usb_mouse_button_up(1);
            s_tap_button_down = false;
            if (is_tap) {
                // The release just sent completed the *first* click. A
                // double-click needs a whole second click behind it; leave a
                // gap, or the host's debounce filter swallows it.
                s_dbl_click_pending = true;
                s_dbl_click_at = now + GEST_DOUBLE_CLICK_GAP_MS;
            }
        }
        else if (is_tap && due(now, s_click_armed_until)) {
            // A tap out of a long stillness. Swallow it - but arm the window
            // on the way out, so the tap you make right after this one lands.
            // Without that, "put the pointer somewhere, read for a while, then
            // click" costs a tap every single time and never recovers; with
            // it, the cost is one tap once. The phantom this guards against
            // arrives alone, so arming on it gives up very little.
            s_click_armed_until = now + GEST_CLICK_ARM_WINDOW_MS;
        }
        else if (is_tap) {
            // Press now, release later: the deferred release is what lets a
            // following touch turn this tap into a drag.
            usb_mouse_button_down(1);
            s_tap_button_down = true;
            s_tap_release_at = now + GEST_TAP_DRAG_TIME_MS;
            s_last_tap_x = s_current.x;
            s_last_tap_y = s_current.y;
        }
        // A long or far-travelling touch was plain pointer movement: nothing
        // to send, and it arms nothing.

        s_state = TouchState::IDLE;
        s_left_dead_zone = false;
    }
    // --- Held: Finger still down ---
    else if (is_pressed && s_was_pressed) {
        // Dead zone: the pointer does not move until the finger has
        // travelled GEST_TAP_DISTANCE from where it landed. Without it a
        // tap nudges the cursor, and a stationary second tap would drift
        // far enough for the host to cancel double-click detection.
        int32_t travel_x = s_current.x - s_tap_start.x;
        int32_t travel_y = s_current.y - s_tap_start.y;
        if (travel_x < 0) travel_x = -travel_x;
        if (travel_y < 0) travel_y = -travel_y;
        if (travel_x + travel_y > (int32_t)GEST_TAP_DISTANCE) {
            s_left_dead_zone = true;
        }

        // Move the pointer with the finger. This runs in both held states:
        // a plain finger slide is what makes the device a trackpad, and in
        // DRAGGING the same movement drags with the left button down.
        if (s_left_dead_zone) {
            int32_t dx = s_current.x - s_previous.x;
            int32_t dy = s_current.y - s_previous.y;
            if (dx != 0 || dy != 0) {
                usb_mouse_move(clamp_delta(dx), clamp_delta(dy));
                // Motion the host actually saw is what arms a click. Taking
                // it from here rather than from the press edge is the whole
                // point: a phantom lasts a single poll, so it never reaches a
                // held frame and can never arm the window for the next one.
                s_click_armed_until = now + GEST_CLICK_ARM_WINDOW_MS;
            }
        }
    }

    // Save previous state
    s_previous = s_current;
    s_was_pressed = is_pressed;
}
