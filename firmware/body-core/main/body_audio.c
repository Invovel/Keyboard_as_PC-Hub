// SPDX-License-Identifier: GPL-3.0-or-later
#include "body_audio.h"
#include <stdint.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/i2s_std.h"
#include "esp_log.h"

#define TAG "body-audio"
#define AUDIO_RATE 16000
static QueueHandle_t tone_queue;
static i2s_chan_handle_t tx;

static void play_tone(body_tone_t tone)
{
    int hz = tone == TONE_ERROR ? 330 : tone == TONE_START ? 523 : 784;
    int frames = tone == TONE_ERROR ? 2400 : 1440;
    int16_t data[128 * 2];
    int phase = 0;
    while (frames > 0) {
        int count = frames < 128 ? frames : 128;
        for (int i = 0; i < count; ++i) {
            int16_t sample = phase < AUDIO_RATE / 2 ? 4000 : -4000;
            data[2 * i] = sample;
            data[2 * i + 1] = sample;
            phase += hz;
            if (phase >= AUDIO_RATE) phase -= AUDIO_RATE;
        }
        size_t written = 0;
        if (i2s_channel_write(tx, data, count * 2 * sizeof(int16_t), &written, 1000) != ESP_OK) break;
        frames -= count;
    }
    memset(data, 0, sizeof(data));
    size_t written = 0;
    i2s_channel_write(tx, data, sizeof(data), &written, 1000);
}

static void audio_task(void *unused)
{
    (void)unused;
    body_tone_t tone;
    while (true) if (xQueueReceive(tone_queue, &tone, portMAX_DELAY) == pdTRUE) play_tone(tone);
}

void body_audio_init(void)
{
    tone_queue = xQueueCreate(8, sizeof(body_tone_t));
    ESP_ERROR_CHECK(tone_queue ? ESP_OK : ESP_ERR_NO_MEM);
    i2s_chan_config_t chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_0, I2S_ROLE_MASTER);
    chan_cfg.auto_clear_after_cb = true; // Silence DMA after a short tone ends.
    ESP_ERROR_CHECK(i2s_new_channel(&chan_cfg, &tx, NULL));
    i2s_std_config_t cfg = {
        .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(AUDIO_RATE),
        .slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_16BIT,
                                                        I2S_SLOT_MODE_STEREO),
        .gpio_cfg = {.mclk = I2S_GPIO_UNUSED, .bclk = GPIO_NUM_47, .ws = GPIO_NUM_48,
                     .dout = GPIO_NUM_21, .din = I2S_GPIO_UNUSED},
    };
    ESP_ERROR_CHECK(i2s_channel_init_std_mode(tx, &cfg));
    ESP_ERROR_CHECK(i2s_channel_enable(tx));
    BaseType_t ok = xTaskCreate(audio_task, "body-audio", 3072, NULL, 4, NULL);
    ESP_ERROR_CHECK(ok == pdPASS ? ESP_OK : ESP_ERR_NO_MEM);
    ESP_LOGI(TAG, "MAX98357A I2S GPIO47/48/21 initialized");
}

void body_audio_play(body_tone_t tone)
{
    if (tone_queue) xQueueSend(tone_queue, &tone, 0);
}
