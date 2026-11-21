// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
typedef enum { TONE_START, TONE_CONFIRM, TONE_ERROR } body_tone_t;
void body_audio_init(void);
void body_audio_play(body_tone_t tone);
