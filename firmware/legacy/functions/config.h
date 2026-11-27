#ifndef __CONFIG_H__
#define __CONFIG_H__

#include "stm32f1xx_hal.h"

// ====================== 功能开关配置 ======================
#define FUNCTION_KEY_ENABLE        1  // 热插拔按键功能使能
#define FUNCTION_ENCODER_ENABLE    1  // 旋钮功能使能
#define FUNCTION_SWITCH_ENABLE     1  // 类雷蛇滚轮钮子开关使能
#define FUNCTION_OLED_ENABLE       1  // OLED状态显示使能
#define FUNCTION_USB_HID_ENABLE    1  // USB HID快捷键功能使能
#define FUNCTION_POWER_DETECT_ENABLE 1 // 双供电模式检测使能

// ====================== 扫描间隔配置 ======================
#define KEY_SCAN_INTERVAL     10  // 按键扫描间隔（ms）
#define ENC_SCAN_INTERVAL     5   // 旋钮扫描间隔（ms）
#define SWITCH_SCAN_INTERVAL  10  // 开关扫描间隔（ms）
#define OLED_REFRESH_INTERVAL 50  // 屏幕刷新间隔（ms）
#define HID_REPORT_INTERVAL   20  // HID指令上报间隔（ms）

// ====================== 引脚定义占位（后续按需补充） ======================
// 热插拔按键引脚
#define KEY1_PIN        GPIO_PIN_0
#define KEY1_PORT       GPIOA
// 旋钮引脚
#define ENC_CLK_PIN     GPIO_PIN_5
#define ENC_CLK_PORT    GPIOA
// 拨片开关引脚
#define SWITCH_LEFT_PIN GPIO_PIN_0
#define SWITCH_LEFT_PORT GPIOB
// 指示灯引脚
#define LED_ENC_RED_PORT GPIOA
#define LED_ENC_RED_PIN  GPIO_PIN_8
#define LED_POWER_PORT   GPIOA
#define LED_POWER_PIN    GPIO_PIN_9
#define LED_DEBUG_PORT   GPIOA
#define LED_DEBUG_PIN    GPIO_PIN_10
#define LED_HUB_PORT     GPIOB
#define LED_HUB_PIN      GPIO_PIN_0

#endif