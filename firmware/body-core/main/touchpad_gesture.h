// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef struct {
    void (*move)(int8_t dx, int8_t dy);
    void (*button)(bool down);
} touchpad_output_t;

void touchpad_gesture_sample(bool down, int16_t x, int16_t y, uint32_t now_ms,
                             const touchpad_output_t *output);
void touchpad_gesture_reset(const touchpad_output_t *output);
bool touchpad_gesture_selftest(void);
