/*
 * SPDX-FileCopyrightText: 2010-2026 Espressif Systems (Shanghai) CO LTD
 * SPDX-License-Identifier: CC0-1.0
 *
 * Adapted from ESP-IDF v6.1 examples/peripherals/pcnt/rotary_encoder.
 * https://github.com/espressif/esp-idf/tree/master/examples/peripherals/pcnt/rotary_encoder
 * Local changes: GPIO1/2, explicit pull-ups, accumulated count, no sleep/demo task.
 */
#include "body_encoder.h"
#include "driver/pulse_cnt.h"
#include "driver/gpio.h"
#include "esp_check.h"

static pcnt_unit_handle_t unit;

void body_encoder_init(void)
{
    pcnt_unit_config_t config = {
        .low_limit = -30000, .high_limit = 30000,
        .flags.accum_count = true,
    };
    ESP_ERROR_CHECK(pcnt_new_unit(&config, &unit));
    pcnt_glitch_filter_config_t filter = {.max_glitch_ns = 1000};
    ESP_ERROR_CHECK(pcnt_unit_set_glitch_filter(unit, &filter));
    pcnt_chan_config_t ca = {.edge_gpio_num = 1, .level_gpio_num = 2};
    pcnt_chan_config_t cb = {.edge_gpio_num = 2, .level_gpio_num = 1};
    pcnt_channel_handle_t a, b;
    ESP_ERROR_CHECK(pcnt_new_channel(unit, &ca, &a));
    ESP_ERROR_CHECK(pcnt_new_channel(unit, &cb, &b));
    ESP_ERROR_CHECK(gpio_pullup_en(1));
    ESP_ERROR_CHECK(gpio_pullup_en(2));
    ESP_ERROR_CHECK(pcnt_channel_set_edge_action(a, PCNT_CHANNEL_EDGE_ACTION_DECREASE,
                                                  PCNT_CHANNEL_EDGE_ACTION_INCREASE));
    ESP_ERROR_CHECK(pcnt_channel_set_level_action(a, PCNT_CHANNEL_LEVEL_ACTION_KEEP,
                                                   PCNT_CHANNEL_LEVEL_ACTION_INVERSE));
    ESP_ERROR_CHECK(pcnt_channel_set_edge_action(b, PCNT_CHANNEL_EDGE_ACTION_INCREASE,
                                                  PCNT_CHANNEL_EDGE_ACTION_DECREASE));
    ESP_ERROR_CHECK(pcnt_channel_set_level_action(b, PCNT_CHANNEL_LEVEL_ACTION_KEEP,
                                                   PCNT_CHANNEL_LEVEL_ACTION_INVERSE));
    ESP_ERROR_CHECK(pcnt_unit_add_watch_point(unit, -30000));
    ESP_ERROR_CHECK(pcnt_unit_add_watch_point(unit, 30000));
    ESP_ERROR_CHECK(pcnt_unit_enable(unit));
    ESP_ERROR_CHECK(pcnt_unit_clear_count(unit));
    ESP_ERROR_CHECK(pcnt_unit_start(unit));
}

int body_encoder_count(void)
{
    int count;
    ESP_ERROR_CHECK(pcnt_unit_get_count(unit, &count));
    return count;
}
