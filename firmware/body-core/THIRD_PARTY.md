# Open-source integration

This firmware combines GPL-3.0-or-later gesture code with the components below.
Distribute corresponding source, this notice, and `LICENSE` with firmware releases.
Registry dependencies are pinned in `main/idf_component.yml` and `dependencies.lock`;
their original license files remain in the downloaded component distributions.

| Project | Version/source | Integration | License |
| --- | --- | --- | --- |
| numrow-touchpad | https://github.com/fholmer/numrow-touchpad/tree/810f02853f7163d13cea1dbb414312a43fc51f3c | `main/touchpad_gesture.c`: state-machine adaptation; FT3168/AMOLED drivers omitted | GPL-3.0-or-later |
| ESP-IDF | v6.1 local toolchain | GPIO, SPI, I2C, I2S, NVS, FreeRTOS integration | Apache-2.0; see bundled component notices |
| ESP-IDF rotary_encoder example | Local v6.1 `examples/peripherals/pcnt/rotary_encoder` | `body_encoder.c` adapts hardware quadrature configuration, with overflow accumulation and GPIO1/2 | CC0-1.0; original copyright/SPDX retained |
| esp_lcd_st77916 | 2.0.2 | QSPI panel transport with custom panel register data | Apache-2.0 |
| esp_lcd_touch_cst816s | 1.1.1 | CST816-compatible coordinate reader at 0x15 | Apache-2.0 |
| esp_lvgl_port | 2.8.0 | LVGL display, timer and locking | Apache-2.0 |
| LVGL | 9.5.0 | Four-page UI | MIT |
| esp_tinyusb / TinyUSB | 2.4.0 / resolved lock version | Composite HID | Apache-2.0 / MIT |
| FreeTouchDeck | https://github.com/DustinWatts/FreeTouchDeck | Design reference only; no source copied | Not linked into this firmware |

The original numrow `touch.cpp` and full GPL license are retained under
`third_party/numrow-touchpad/`. Its attribution comments are preserved there.
Changes: hardware-independent C API; callbacks replace USB calls; 360x360 coordinates
come from the local touch adapter; page ownership and reset are controlled by the main loop.

`main/panel_verified_init.h` preserves the module register data already tested in
`embedded-learning-lab/projects/screen-reference/diagnostics/main/jd9855_init.h`.
Original register reference:
https://lingflux.com/zh-cn/experiments/esp32/esp32-s3-1inch5-tft-jd9855-qspi-animation/ .
The local 2026-10-01 evidence records correct color and touch points using QSPI mode 3,
20 MHz, RGB565. Using this data through an ST77916 transport is not a claim that the
physical LCD controller ID is ST77916 or JD9855. No new analog drive parameters were invented.
