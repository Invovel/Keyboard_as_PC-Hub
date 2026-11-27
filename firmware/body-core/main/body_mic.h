// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include "esp_err.h"

#define BODY_MIC_RATE 16000
#define BODY_MIC_FRAME_SAMPLES 256
#define BODY_MIC_BCLK 4
#define BODY_MIC_WS 5
#define BODY_MIC_SD 17

typedef struct {
    bool configured, capturing;
    uint32_t frames, nonzero_samples, clipped_samples, dropped_frames, read_errors;
    float rms_dbfs;
    uint16_t peak;
} body_mic_status_t;

// Optional INMP441, left channel (L/R tied to GND), 64 BCLK per frame.
// A configured I2S receiver is NOT proof that a microphone is connected.
esp_err_t body_mic_init(void);
esp_err_t body_mic_set_capture(bool active);
body_mic_status_t body_mic_status(void);
// PCM16 mono, fixed 256-sample frames; one consumer; no capture files/network/USB UAC.
esp_err_t body_mic_read_pcm(int16_t *out, size_t capacity, size_t *samples, uint32_t timeout_ms);
