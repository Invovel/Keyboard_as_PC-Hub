// SPDX-License-Identifier: GPL-3.0-or-later
#include "body_hid.h"
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "tinyusb.h"
#include "tinyusb_default_config.h"
#include "class/hid/hid_device.h"

#define TAG "body-hid"
enum { REPORT_KEYBOARD = 1, REPORT_MOUSE = 2, REPORT_CONSUMER = 3 };
#define HID_DESC_LEN (TUD_CONFIG_DESC_LEN + CFG_TUD_HID * TUD_HID_DESC_LEN)

static const uint8_t report_descriptor[] = {
    TUD_HID_REPORT_DESC_KEYBOARD(HID_REPORT_ID(REPORT_KEYBOARD)),
    TUD_HID_REPORT_DESC_MOUSE(HID_REPORT_ID(REPORT_MOUSE)),
    TUD_HID_REPORT_DESC_CONSUMER(HID_REPORT_ID(REPORT_CONSUMER)),
};
static const char *strings[] = {
    (char[]){0x09, 0x04}, "Invovel", "Keyboard as PC-Hub Body", "BODY-CORE-001", "Body HID"
};
static const uint8_t configuration_descriptor[] = {
    TUD_CONFIG_DESCRIPTOR(1, 1, 0, HID_DESC_LEN, TUSB_DESC_CONFIG_ATT_REMOTE_WAKEUP, 100),
    TUD_HID_DESCRIPTOR(0, 4, false, sizeof(report_descriptor), 0x81, 16, 1),
};

uint8_t const *tud_hid_descriptor_report_cb(uint8_t instance)
{
    (void)instance;
    return report_descriptor;
}
uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t type,
                               uint8_t *buffer, uint16_t reqlen)
{
    (void)instance; (void)report_id; (void)type; (void)buffer; (void)reqlen;
    return 0;
}
void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t type,
                           uint8_t const *buffer, uint16_t len)
{
    (void)instance; (void)report_id; (void)type; (void)buffer; (void)len;
}

typedef enum { MSG_MOVE, MSG_WHEEL, MSG_BUTTON, MSG_KEY, MSG_CONSUMER } msg_type_t;
typedef struct {
    msg_type_t type;
    int8_t x, y;
    uint8_t source, down, modifier, keycode;
    uint16_t usage;
} hid_msg_t;
static QueueHandle_t queue;
static uint8_t mouse_sources;
static bool was_mounted;
static uint8_t release_step;
static uint8_t key_step;
static hid_msg_t pending;

static void enqueue(hid_msg_t msg)
{
    if (queue && xQueueSend(queue, &msg, 0) != pdTRUE) {
        ESP_LOGW(TAG, "HID queue full; release all");
        body_hid_release_all();
    }
}

void body_hid_init(void)
{
    queue = xQueueCreate(32, sizeof(hid_msg_t));
    ESP_ERROR_CHECK(queue ? ESP_OK : ESP_ERR_NO_MEM);
    tinyusb_config_t cfg = TINYUSB_DEFAULT_CONFIG();
    cfg.descriptor.device = NULL;
    cfg.descriptor.full_speed_config = configuration_descriptor;
    cfg.descriptor.string = strings;
    cfg.descriptor.string_count = sizeof(strings) / sizeof(strings[0]);
#if (TUD_OPT_HIGH_SPEED)
    cfg.descriptor.high_speed_config = configuration_descriptor;
#endif
    ESP_ERROR_CHECK(tinyusb_driver_install(&cfg));
    ESP_LOGI(TAG, "keyboard + mouse + consumer HID ready");
}

bool body_hid_mounted(void) { return tud_mounted(); }

void body_hid_release_all(void)
{
    if (queue) xQueueReset(queue);
    mouse_sources = 0;
    key_step = 0;
    release_step = 1;
}

void body_hid_mouse_move(int8_t dx, int8_t dy)
{
    enqueue((hid_msg_t){.type=MSG_MOVE, .x=dx, .y=dy});
}
void body_hid_mouse_wheel(int8_t wheel)
{
    enqueue((hid_msg_t){.type=MSG_WHEEL, .y=wheel});
}
void body_hid_mouse_button(uint8_t source, bool down)
{
    enqueue((hid_msg_t){.type=MSG_BUTTON, .source=source, .down=down});
}
void body_hid_key(uint8_t modifier, uint8_t keycode)
{
    enqueue((hid_msg_t){.type=MSG_KEY, .modifier=modifier, .keycode=keycode});
}
void body_hid_consumer(uint16_t usage)
{
    enqueue((hid_msg_t){.type=MSG_CONSUMER, .usage=usage});
}

void body_hid_tick(void)
{
    bool mounted = tud_mounted();
    if (!mounted) {
        if (was_mounted) body_hid_release_all();
        was_mounted = false;
        return;
    }
    if (!was_mounted) {
        was_mounted = true;
        body_hid_release_all();
    }
    if (!tud_hid_ready()) return;

    // Release reports have priority after STOP, disconnect, or reconnect.
    if (release_step) {
        bool sent = false;
        if (release_step == 1) {
            sent = tud_hid_mouse_report(REPORT_MOUSE, 0, 0, 0, 0, 0);
        } else if (release_step == 2) {
            sent = tud_hid_keyboard_report(REPORT_KEYBOARD, 0, NULL);
        } else {
            uint16_t zero = 0;
            sent = tud_hid_report(REPORT_CONSUMER, &zero, sizeof(zero));
        }
        if (sent) release_step = release_step == 3 ? 0 : release_step + 1;
        return;
    }
    if (key_step) {
        bool sent;
        if (key_step == 1) {
            uint8_t keycodes[6] = {pending.keycode};
            sent = tud_hid_keyboard_report(REPORT_KEYBOARD, pending.modifier, keycodes);
        } else if (key_step == 2) {
            sent = tud_hid_keyboard_report(REPORT_KEYBOARD, 0, NULL);
        } else if (key_step == 3) {
            sent = tud_hid_report(REPORT_CONSUMER, &pending.usage, sizeof(pending.usage));
        } else {
            uint16_t zero = 0;
            sent = tud_hid_report(REPORT_CONSUMER, &zero, sizeof(zero));
        }
        if (sent) key_step = (key_step == 2 || key_step == 4) ? 0 : key_step + 1;
        return;
    }
    hid_msg_t msg;
    if (xQueueReceive(queue, &msg, 0) != pdTRUE) return;
    if (msg.type == MSG_KEY || msg.type == MSG_CONSUMER) {
        pending = msg;
        key_step = msg.type == MSG_KEY ? 1 : 3;
        return;
    }
    if (msg.type == MSG_BUTTON) {
        if (msg.down) mouse_sources |= msg.source;
        else mouse_sources &= (uint8_t)~msg.source;
    }
    int8_t dx = msg.type == MSG_MOVE ? msg.x : 0;
    int8_t dy = msg.type == MSG_MOVE ? msg.y : 0;
    int8_t wheel = msg.type == MSG_WHEEL ? msg.y : 0;
    if (!tud_hid_mouse_report(REPORT_MOUSE, mouse_sources ? MOUSE_BUTTON_LEFT : 0,
                              dx, dy, wheel, 0)) {
        // A failed transport must not leave a held button on the host.
        body_hid_release_all();
    }
}
