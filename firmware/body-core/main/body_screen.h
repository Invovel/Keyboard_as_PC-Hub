#pragma once
#include <stdbool.h>
#include <stdint.h>
#include "esp_err.h"
typedef struct { bool down; uint16_t x, y; } body_touch_t;
esp_err_t body_screen_init(void);
esp_err_t body_touch_init(void);
esp_err_t body_touch_read(body_touch_t *point);
