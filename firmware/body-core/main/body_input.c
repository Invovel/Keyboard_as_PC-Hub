#include "body_input.h"
#include "body_encoder.h"
#include "driver/gpio.h"
#include "esp_timer.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include <stdatomic.h>

// Four keys and switch follow the latest ordered COM5 test. B=2 remains
// provisional: never synthesize direction from A alone.
static const int pins[] = {42, 41, 40, 38, 39};
static bool raw[5], stable[5];
static int64_t changed[5];
static QueueHandle_t events;
static uint8_t previous_ab;
static int quarter_steps;
static int previous_count;
static atomic_int pcnt_count;
static atomic_uint a_edges, b_edges, invalid, dropped;

static void emit(body_input_event_t event)
{
    if (xQueueSend(events, &event, 0) != pdTRUE) atomic_fetch_add(&dropped, 1);
}

static void sample(void *unused)
{
    (void)unused;
    int64_t now = esp_timer_get_time();
    for (int i = 0; i < 5; ++i) {
        bool down = !gpio_get_level(pins[i]);
        if (raw[i] != down) { raw[i] = down; changed[i] = now; }
        if (stable[i] != down && now - changed[i] >= 25000) {
            stable[i] = down;
            emit((body_input_event_t){i == 4 ? BODY_INPUT_PRESS : BODY_INPUT_KEY,
                                     i + 1, down});
        }
    }
    int count = body_encoder_count();
    atomic_store(&pcnt_count, count);
    // Modular subtraction also handles the signed accumulator wrapping.
    int32_t difference = (int32_t)((uint32_t)count - (uint32_t)previous_count);
    previous_count = count;
    if (difference > 256 || difference < -256) {
        // Do not flood HID with a stale or implausibly large batch.
        quarter_steps = 0;
        atomic_fetch_add(&dropped, 1);
        return;
    }
    quarter_steps += difference;
    while (quarter_steps >= 4 || quarter_steps <= -4) {
        int direction = quarter_steps > 0 ? 1 : -1;
        emit((body_input_event_t){BODY_INPUT_ROTATE, direction, false});
        quarter_steps -= 4 * direction;
    }
}

// Capture both phases at their edges rather than waiting for the 1 ms button
// timer. These are diagnostics only; the PCNT peripheral owns direction decoding.
static void encoder_edge(void *unused)
{
    (void)unused;
    uint8_t ab = (gpio_get_level(1) << 1) | gpio_get_level(2);
    uint8_t diff = previous_ab ^ ab;
    if (diff & 2) atomic_fetch_add(&a_edges, 1);
    if (diff & 1) atomic_fetch_add(&b_edges, 1);
    if (diff == 3) atomic_fetch_add(&invalid, 1);
    previous_ab = ab;
}

void body_input_init(void)
{
    events = xQueueCreate(64, sizeof(body_input_event_t));
    ESP_ERROR_CHECK(events ? ESP_OK : ESP_ERR_NO_MEM);
    uint64_t mask = (1ULL << 1) | (1ULL << 2);
    for (int i = 0; i < 5; ++i) mask |= 1ULL << pins[i];
    gpio_config_t cfg = {.pin_bit_mask=mask, .mode=GPIO_MODE_INPUT,
                        .pull_up_en=GPIO_PULLUP_ENABLE, .intr_type=GPIO_INTR_DISABLE};
    ESP_ERROR_CHECK(gpio_config(&cfg));
    for (int i = 0; i < 5; ++i) raw[i] = stable[i] = !gpio_get_level(pins[i]);
    body_encoder_init();
    previous_ab = (gpio_get_level(1) << 1) | gpio_get_level(2);
    ESP_ERROR_CHECK(gpio_install_isr_service(0));
    ESP_ERROR_CHECK(gpio_isr_handler_add(1, encoder_edge, NULL));
    ESP_ERROR_CHECK(gpio_isr_handler_add(2, encoder_edge, NULL));
    ESP_ERROR_CHECK(gpio_set_intr_type(1, GPIO_INTR_ANYEDGE));
    ESP_ERROR_CHECK(gpio_set_intr_type(2, GPIO_INTR_ANYEDGE));
    ESP_ERROR_CHECK(gpio_intr_enable(1));
    ESP_ERROR_CHECK(gpio_intr_enable(2));
    esp_timer_handle_t timer;
    esp_timer_create_args_t args = {.callback=sample, .name="body-input",
                                    .skip_unhandled_events=true};
    ESP_ERROR_CHECK(esp_timer_create(&args, &timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(timer, 1000));
    ESP_LOGI("body-input", "1ms buttons: keys=42/41/40/38, switch=39; official PCNT encoder A/B=1/2, initial=%u", previous_ab);
}

bool body_input_read(body_input_event_t *event) { return xQueueReceive(events, event, 0) == pdTRUE; }
body_input_stats_t body_input_stats(void)
{
    return (body_input_stats_t){atomic_load(&a_edges), atomic_load(&b_edges),
                               atomic_load(&invalid), atomic_load(&dropped), atomic_load(&pcnt_count)};
}
