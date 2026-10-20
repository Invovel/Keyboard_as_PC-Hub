#ifndef __LIGHT_H
#define __LIGHT_H

#include "stm32f1xx_hal.h"
#include "config.h"
#include "core_app.h"

// ====================== 指示灯ID定义（模块A核心灯组） ======================
typedef enum {
    LED_ID_ENC_RED = 0,    // 旋钮双色灯-红灯：亮度模式指示
    LED_ID_ENC_GREEN,       // 旋钮双色灯-绿灯：静音/特殊状态指示
    LED_ID_POWER,           // 供电状态灯：PD供电模式指示
    LED_ID_DEBUG,           // 调试状态灯：VS Code调试模式指示
    LED_ID_HUB,             // Hub状态灯：高速接口连接指示
    LED_ID_MAX              // 灯组总数，用于拓展预留
} LED_ID_Typedef;

// ====================== 指示灯工作状态枚举 ======================
typedef enum {
    LED_OFF = 0,            // 关闭
    LED_ON,                 // 常亮
    LED_BLINK_SLOW,         // 慢闪（500ms亮/500ms灭）
    LED_BLINK_FAST          // 快闪（200ms亮/200ms灭）
} LED_State_Typedef;

// ====================== 函数接口声明 ======================
// 基础硬件接口
void LED_Init(void);                                  // 所有指示灯GPIO初始化
void LED_SetState(LED_ID_Typedef led_id, LED_State_Typedef state);  // 单个灯状态设置
void LED_Task(void);                                  // 灯状态刷新任务（主循环时间片调度）

// 业务逻辑联动接口
void LED_UpdateBySystemStatus(void);                 // 跟随系统状态自动更新灯效
void LED_TriggerDebugBlink(void);                    // 调试触发单次闪烁
void LED_TriggerHubBlink(void);                      // 高速接口传输触发闪烁

// 拓展模块预留接口
void LED_ExtendSetState(uint8_t extend_led_id, LED_State_Typedef state);  // 拓展灯控制

#endif