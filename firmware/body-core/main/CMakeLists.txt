idf_component_register(SRCS "app_main.c" "body_input.c" "body_hid.c" "body_audio.c"
    "body_screen.c" "body_ui.c" "touchpad_gesture.c" "gesture_selftest.c" "body_encoder.c"
    INCLUDE_DIRS "." PRIV_REQUIRES esp_driver_gpio esp_driver_i2c esp_driver_i2s
    esp_driver_spi esp_timer esp_tinyusb esp_lcd esp_lcd_st77916 esp_lcd_touch_cst816s
    esp_lvgl_port nvs_flash esp_driver_pcnt)
