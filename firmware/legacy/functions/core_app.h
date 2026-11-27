#ifndef __CORE_APP_H
#define __CORE_APP_H

#include "config.h"
#include "stm32f1xx_hal.h"

// 全局系统状态管理结构体
typedef struct {
    uint8_t enc_mode;       // 旋钮模式：0-音量 1-亮度
    uint8_t power_mode;     // 供电模式：0-电脑供电 1-PD供电
    uint8_t vs_code_mode;   // 编程模式状态
    uint16_t current_volume;// 当前音量值
    uint16_t current_bright;// 当前亮度值
} SystemStatus_Typedef;

// 核心功能统一初始化入口
void Core_AppInit(void);

// 核心功能循环调度入口
void Core_KeyTask(void);
void Core_EncoderTask(void);
void Core_SwitchTask(void);
void Core_OLEDTask(void);
void Core_PowerTask(void);
void Core_HIDTask(void);

#endif