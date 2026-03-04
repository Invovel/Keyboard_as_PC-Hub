#ifndef __TRIGGER_H
#define __TRIGGER_H

#include "stm32f1xx_hal.h"
#include "config.h"
#include "bsp_usb_hid.h"
#include "light.h"

// ====================== 拨片状态枚举 ======================
typedef enum {
    TRIGGER_STATE_IDLE = 0,    // 中间复位状态（无触发）
    TRIGGER_STATE_LEFT,         // 左拨触发
    TRIGGER_STATE_RIGHT         // 右拨触发
} TRIGGER_State_Typedef;

// ====================== 触发模式枚举 ======================
typedef enum {
    TRIGGER_MODE_SHORT = 0,     // 短按模式：VS Code标签页切换
    TRIGGER_MODE_LONG            // 长按模式：全局窗口切换
} TRIGGER_Mode_Typedef;

// ====================== 核心参数配置 ======================
#define TRIGGER_DEBOUNCE_TIME    50    // 防抖时间（ms）
#define TRIGGER_LONG_PRESS_TIME  500   // 长按阈值（ms）
#define TRIGGER_INTERVAL         100   // 持续触发间隔（ms，类雷蛇滚轮滚动速度）

// ====================== 函数接口声明 ======================
// 基础硬件接口
void TRIGGER_Init(void);                                  // 拨片GPIO初始化
void TRIGGER_Task(void);                                  // 拨片扫描与触发任务（主循环时间片调度）
TRIGGER_State_Typedef TRIGGER_GetState(void);            // 获取当前拨片状态
TRIGGER_Mode_Typedef TRIGGER_GetMode(void);              // 获取当前触发模式

// 拓展预留接口：自定义触发回调（后续模块可重写触发逻辑）
typedef void (*TRIGGER_Callback)(TRIGGER_State_Typedef state, TRIGGER_Mode_Typedef mode);
void TRIGGER_SetCallback(TRIGGER_Callback callback);     // 注册自定义触发回调

#endif