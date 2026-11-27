// SPDX-License-Identifier: GPL-3.0-or-later
// Adapted from fholmer/numrow-touchpad main/touch.cpp at
// 810f02853f7163d13cea1dbb414312a43fc51f3c (GPL-3.0-or-later).
// The FT3168 I2C driver and AMOLED-specific code are deliberately omitted.
#include "touchpad_gesture.h"

enum { TAP_TIME_MS = 150, TAP_DISTANCE = 4, TAP_DRAG_MS = 220,
       TAP_DRAG_DISTANCE = 100, DOUBLE_CLICK_GAP_MS = 40 };

typedef enum { IDLE, PRESSED, DRAGGING } gesture_state_t;
static gesture_state_t state;
static bool was_down;
static bool left_dead_zone;
static bool tap_button_down;
static bool double_click_pending;
static uint32_t press_time;
static uint32_t tap_release_at;
static uint32_t double_click_at;
static int16_t start_x, start_y, prev_x, prev_y, last_tap_x, last_tap_y;

static bool due(uint32_t now, uint32_t deadline)
{
    return (int32_t)(now - deadline) >= 0;
}

static int8_t clamp_delta(int32_t value)
{
    if (value > 127) return 127;
    if (value < -127) return -127;
    return (int8_t)value;
}

void touchpad_gesture_reset(const touchpad_output_t *output)
{
    if (tap_button_down && output && output->button) output->button(false);
    state = IDLE;
    was_down = false;
    left_dead_zone = false;
    tap_button_down = false;
    double_click_pending = false;
    press_time = tap_release_at = double_click_at = 0;
}

void touchpad_gesture_sample(bool down, int16_t x, int16_t y, uint32_t now,
                             const touchpad_output_t *output)
{
    if (tap_button_down && state != DRAGGING && due(now, tap_release_at)) {
        output->button(false);
        tap_button_down = false;
    }
    if (double_click_pending && due(now, double_click_at)) {
        double_click_pending = false;
        output->button(true);
        output->button(false);
    }

    if (down && !was_down) {
        press_time = now;
        start_x = prev_x = x;
        start_y = prev_y = y;
        left_dead_zone = false;
        state = PRESSED;
        if (tap_button_down) {
            int32_t dx = x - last_tap_x, dy = y - last_tap_y;
            if (dx < 0) dx = -dx;
            if (dy < 0) dy = -dy;
            if (dx <= TAP_DRAG_DISTANCE && dy <= TAP_DRAG_DISTANCE) state = DRAGGING;
        }
    } else if (!down && was_down) {
        bool is_tap = !left_dead_zone && (now - press_time) <= TAP_TIME_MS;
        if (state == DRAGGING) {
            output->button(false);
            tap_button_down = false;
            if (is_tap) {
                double_click_pending = true;
                double_click_at = now + DOUBLE_CLICK_GAP_MS;
            }
        } else if (is_tap) {
            // The release is deferred so a second nearby touch can claim it for dragging.
            output->button(true);
            tap_button_down = true;
            tap_release_at = now + TAP_DRAG_MS;
            last_tap_x = prev_x;
            last_tap_y = prev_y;
        }
        state = IDLE;
        left_dead_zone = false;
    } else if (down && was_down) {
        int32_t travel_x = x - start_x, travel_y = y - start_y;
        if (travel_x < 0) travel_x = -travel_x;
        if (travel_y < 0) travel_y = -travel_y;
        if (travel_x + travel_y > TAP_DISTANCE) left_dead_zone = true;
        if (left_dead_zone && (x != prev_x || y != prev_y)) {
            output->move(clamp_delta(x - prev_x), clamp_delta(y - prev_y));
        }
    }

    prev_x = x;
    prev_y = y;
    was_down = down;
}
