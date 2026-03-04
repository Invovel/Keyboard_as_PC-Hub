#ifndef __BSP_USB_HID_H
#define __BSP_USB_HID_H

#include "stm32f1xx_hal.h"

// 按键码宏定义（占位，后续按需补充）
#define KEY_CTRL        0x01
#define KEY_SHIFT       0x02
#define KEY_ALT         0x04
#define KEY_PAGE_UP     0x4B
#define KEY_PAGE_DOWN   0x4E
#define KEY_TAB         0x2B

// HID发送函数声明（占位）
uint8_t BSP_USBHID_SendKey(uint16_t key_code);
uint8_t BSP_USBHID_SendConsumer(uint16_t consumer_code);

#endif