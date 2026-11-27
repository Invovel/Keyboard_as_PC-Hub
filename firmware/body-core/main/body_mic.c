// SPDX-License-Identifier: GPL-3.0-or-later
// I2S timing: TDK DS-INMP441-00 rev 1.1, pages 11-12.
#include "body_mic.h"
#include "sdkconfig.h"
#include <math.h>
#include <string.h>
#include <stdatomic.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/i2s_std.h"
#include "driver/gpio.h"
#include "esp_log.h"

static body_mic_status_t status = {.rms_dbfs = -96.0f};
static portMUX_TYPE lock = portMUX_INITIALIZER_UNLOCKED;
#if CONFIG_BODY_MIC_ENABLE
typedef struct { int16_t pcm[BODY_MIC_FRAME_SAMPLES]; } pcm_frame_t;
static QueueHandle_t frames;
static i2s_chan_handle_t rx;
static atomic_bool requested;

static void mic_task(void *unused)
{
    (void)unused;
    int32_t raw[BODY_MIC_FRAME_SAMPLES * 2];
    pcm_frame_t frame;
    bool running = false;
    while (true) {
        bool want = atomic_load(&requested);
        if (want != running) {
            esp_err_t err = want ? i2s_channel_enable(rx) : i2s_channel_disable(rx);
            if (err != ESP_OK) {
                atomic_store(&requested, false);
                ESP_LOGE("body-mic", "Capture transition: %s", esp_err_to_name(err));
                vTaskDelay(pdMS_TO_TICKS(50));
                continue;
            }
            running = want;
            xQueueReset(frames);
            portENTER_CRITICAL(&lock);
            status.capturing = running;
            status.peak = 0; status.rms_dbfs = -96.0f;
            portEXIT_CRITICAL(&lock);
        }
        if (!running) { vTaskDelay(pdMS_TO_TICKS(20)); continue; }
        size_t bytes = 0;
        esp_err_t err = i2s_channel_read(rx, raw, sizeof(raw), &bytes, 100);
        if (err != ESP_OK || bytes != sizeof(raw)) {
            portENTER_CRITICAL(&lock); ++status.read_errors; portEXIT_CRITICAL(&lock);
            continue;
        }
        // Read both 32-bit slots to guarantee 64 clocks/frame; keep left slot only.
        // The 24-bit signed I2S sample occupies the upper 24 bits of each word.
        uint64_t sumsq = 0;
        uint32_t nonzero = 0, clipped = 0;
        uint16_t peak = 0;
        for (size_t i=0; i<BODY_MIC_FRAME_SAMPLES; ++i) {
            int32_t sample = raw[2*i] >> 16;
            frame.pcm[i] = (int16_t)sample;
            uint16_t magnitude = sample < 0 ? (uint16_t)(-sample) : (uint16_t)sample;
            if (magnitude > peak) peak = magnitude;
            sumsq += (int64_t)sample * sample;
            nonzero += sample != 0;
            clipped += magnitude >= 32700;
        }
        float rms = sqrtf((float)sumsq / BODY_MIC_FRAME_SAMPLES);
        float dbfs = rms > 0 ? 20.0f*log10f(rms/32768.0f) : -96.0f;
        bool dropped = false;
        if (xQueueSend(frames, &frame, 0) != pdTRUE) {
            pcm_frame_t stale;
            xQueueReceive(frames, &stale, 0);
            dropped = true;
            xQueueSend(frames, &frame, 0);
        }
        portENTER_CRITICAL(&lock);
        ++status.frames; status.nonzero_samples += nonzero; status.clipped_samples += clipped;
        status.dropped_frames += dropped; status.peak = peak;
        status.rms_dbfs = fmaxf(-96.0f, dbfs);
        portEXIT_CRITICAL(&lock);
    }
}
#endif

esp_err_t body_mic_init(void)
{
#if CONFIG_BODY_MIC_ENABLE
    if (rx) return ESP_ERR_INVALID_STATE;
    // A disconnected SD line must not float during the unpopulated-module test.
    gpio_config_t sd_cfg = {.pin_bit_mask=1ULL<<BODY_MIC_SD, .mode=GPIO_MODE_INPUT,
                            .pull_down_en=GPIO_PULLDOWN_ENABLE};
    esp_err_t err = gpio_config(&sd_cfg);
    if (err != ESP_OK) return err;
    i2s_chan_config_t channel = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_1, I2S_ROLE_MASTER);
    err = i2s_new_channel(&channel, NULL, &rx);
    if (err != ESP_OK) return err;
    i2s_std_config_t cfg = {
        .clk_cfg=I2S_STD_CLK_DEFAULT_CONFIG(BODY_MIC_RATE),
        .slot_cfg=I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_32BIT, I2S_SLOT_MODE_STEREO),
        .gpio_cfg={.mclk=I2S_GPIO_UNUSED,.bclk=BODY_MIC_BCLK,.ws=BODY_MIC_WS,
                   .dout=I2S_GPIO_UNUSED,.din=BODY_MIC_SD},
    };
    err = i2s_channel_init_std_mode(rx, &cfg);
    if (err != ESP_OK) { i2s_del_channel(rx); rx=NULL; return err; }
    frames=xQueueCreate(8,sizeof(pcm_frame_t));
    if (!frames) { i2s_del_channel(rx); rx=NULL; return ESP_ERR_NO_MEM; }
    if (xTaskCreate(mic_task,"body-mic",6144,NULL,3,NULL) != pdPASS) {
        vQueueDelete(frames);frames=NULL;i2s_del_channel(rx);rx=NULL;return ESP_ERR_NO_MEM;
    }
    portENTER_CRITICAL(&lock); status.configured=true; portEXIT_CRITICAL(&lock);
    ESP_LOGI("body-mic","INMP441 interface configured, capture OFF; hardware UNVERIFIED");
    return ESP_OK;
#else
    ESP_LOGI("body-mic","INMP441 not fitted; enable BODY_MIC_ENABLE to build capture support");
    return ESP_ERR_NOT_SUPPORTED;
#endif
}

esp_err_t body_mic_set_capture(bool active)
{
#if CONFIG_BODY_MIC_ENABLE
    if (!rx || !frames) return ESP_ERR_INVALID_STATE;
    atomic_store(&requested,active);
    return ESP_OK;
#else
    (void)active;return ESP_ERR_NOT_SUPPORTED;
#endif
}
body_mic_status_t body_mic_status(void)
{
    portENTER_CRITICAL(&lock);body_mic_status_t copy=status;portEXIT_CRITICAL(&lock);
    return copy;
}
esp_err_t body_mic_read_pcm(int16_t *out,size_t capacity,size_t *samples,uint32_t timeout_ms)
{
    if (!out || !samples || capacity < BODY_MIC_FRAME_SAMPLES) return ESP_ERR_INVALID_ARG;
    *samples=0;
#if CONFIG_BODY_MIC_ENABLE
    if (!body_mic_status().capturing) return ESP_ERR_INVALID_STATE;
    pcm_frame_t frame;
    if (xQueueReceive(frames,&frame,pdMS_TO_TICKS(timeout_ms)) != pdTRUE) return ESP_ERR_TIMEOUT;
    memcpy(out,frame.pcm,sizeof(frame.pcm));*samples=BODY_MIC_FRAME_SAMPLES;return ESP_OK;
#else
    (void)timeout_ms;return ESP_ERR_NOT_SUPPORTED;
#endif
}
