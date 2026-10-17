#pragma once
#include "body_screen.h"
typedef enum { PAGE_HOME, PAGE_MEDIA, PAGE_PAD, PAGE_SETTINGS } body_page_t;
enum { UI_HOME=1, UI_MEDIA, UI_PAD, UI_SETTINGS, UI_PREV, UI_PLAY, UI_NEXT,
       UI_MUTE, UI_VOL_DOWN, UI_VOL_UP, UI_SENS, UI_SOUND, UI_ENABLE, UI_TEST_KEY };
void body_ui_init(void);
void body_ui_page(body_page_t page);
bool body_ui_action(int *action);
void body_ui_touch(body_touch_t point, bool to_gui);
void body_ui_status(bool usb, bool enabled, int sensitivity, bool sound,
                    const char *last, body_touch_t point, uint32_t a, uint32_t b, int32_t pcnt);
