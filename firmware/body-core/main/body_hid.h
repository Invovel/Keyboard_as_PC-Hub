// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <stdbool.h>
#include <stdint.h>

enum { HID_MOUSE_FROM_TOUCH = 1, HID_MOUSE_FROM_PRESS = 2 };

void body_hid_init(void);
void body_hid_tick(void);
bool body_hid_mounted(void);
void body_hid_release_all(void);
void body_hid_mouse_move(int8_t dx, int8_t dy);
void body_hid_mouse_wheel(int8_t wheel);
void body_hid_mouse_button(uint8_t source, bool down);
void body_hid_key(uint8_t modifier, uint8_t keycode);
void body_hid_consumer(uint16_t usage);
