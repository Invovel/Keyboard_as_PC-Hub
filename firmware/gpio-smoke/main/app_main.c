#include <stdbool.h>
#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"

#define TAG "gpio-discovery"
#define POLL_MS 10
#define DEBOUNCE_POLLS 3

typedef struct {
    gpio_num_t pin;
    bool stable_low;
    bool raw_low;
    uint8_t same_count;
} input_t;

// No peripheral is driven by this firmware. Pins used by Flash/PSRAM, UART0,
// native USB, and reset-time strapping GPIO3/45/46 are excluded.
#define IN(n) { .pin = GPIO_NUM_##n }
static input_t inputs[] = {
    IN(0), IN(1), IN(2), IN(4), IN(5), IN(6), IN(7), IN(8), IN(9), IN(10),
    IN(11), IN(12), IN(13), IN(14), IN(15), IN(16), IN(17), IN(18), IN(21),
    IN(38), IN(39), IN(40), IN(41), IN(42), IN(47), IN(48),
};

void app_main(void)
{
    uint64_t mask = 0;
    for (size_t i = 0; i < sizeof(inputs) / sizeof(inputs[0]); ++i) {
        mask |= 1ULL << inputs[i].pin;
    }
    gpio_config_t cfg = {
        .pin_bit_mask = mask,
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    ESP_ERROR_CHECK(gpio_config(&cfg));
    ESP_LOGI(TAG, "Discovery mode: input pull-ups only; no LCD/I2S/USB HID output");
    ESP_LOGI(TAG, "Press each button separately, then rotate encoder slowly");
    ESP_LOGI(TAG, "GPIO3/19/20/26-37/43-46 excluded; GPIO0 is BOOT strap");
    for (size_t i = 0; i < sizeof(inputs) / sizeof(inputs[0]); ++i) {
        input_t *in = &inputs[i];
        in->raw_low = in->stable_low = gpio_get_level(in->pin) == 0;
        if (in->stable_low) ESP_LOGW(TAG, "GPIO%d LOW at startup", in->pin);
    }

    while (true) {
        for (size_t i = 0; i < sizeof(inputs) / sizeof(inputs[0]); ++i) {
            input_t *in = &inputs[i];
            bool low = gpio_get_level(in->pin) == 0;
            if (low != in->raw_low) {
                in->raw_low = low;
                in->same_count = 1;
                ESP_LOGI(TAG, "GPIO%d RAW %s", in->pin, low ? "LOW" : "HIGH");
            } else if (in->same_count < DEBOUNCE_POLLS) {
                ++in->same_count;
            }
            if (in->same_count >= DEBOUNCE_POLLS && low != in->stable_low) {
                in->stable_low = low;
                ESP_LOGI(TAG, "GPIO%d STABLE %s", in->pin, low ? "DOWN" : "UP");
            }
        }
        vTaskDelay(pdMS_TO_TICKS(POLL_MS));
    }
}
