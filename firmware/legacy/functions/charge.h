#ifndef __CHARGE_H
#define __CHARGE_H

#include "stm32f1xx_hal.h"
#include "config.h"
#include "hub_for_pc.h"

// ====================== 供电模式枚举 ======================
typedef enum {
    POWER_MODE_CHARGER = 0,    // 独立充电器供电（一加/三星）
    POWER_MODE_CHARGE_STATION,  // 桌面充电站供电（酷态科）
    POWER_MODE_OFF,             // 无供电
    POWER_MODE_ERROR            // 供电异常
} POWER_Mode_Typedef;

// ====================== 外设接口枚举 ======================
typedef enum {
    PORT_TYPE_C1 = 0,   // Type-C口1：耳机接收器+数据传输
    PORT_TYPE_C2,       // Type-C口2：预留拓展
    PORT_TYPE_A1,       // USB-A口1：鼠标连接+数据
    PORT_TYPE_A2,       // USB-A口2：键盘连接+数据
    PORT_TYPE_A3,       // USB-A口3：预留拓展
    PORT_TYPE_MAX
} PORT_ID_Typedef;

// ====================== 接口状态枚举 ======================
typedef enum {
    PORT_STATE_IDLE = 0,    // 空闲
    PORT_STATE_CONNECTED,   // 设备已连接
    PORT_STATE_DATA_TRANS,  // 数据传输中
    PORT_STATE_ERROR        // 异常
} PORT_State_Typedef;

// ====================== 函数接口声明 ======================
// 基础初始化
void CHARGE_Init(void);
void CHARGE_Task(void);  // 主循环时间片调度任务

// 供电管理接口
POWER_Mode_Typedef CHARGE_GetPowerMode(void);
uint8_t CHARGE_GetPowerVoltage(void);  // 获取当前供电电压
uint8_t CHARGE_GetPowerCurrent(void);  // 获取当前输出电流

// 预留两种供电方案函数（仅占位，无需具体实现）
uint8_t CHARGE_ChargerSupply_Init(void);    // 独立充电器供电初始化
uint8_t CHARGE_ChargeStationSupply_Init(void);  // 桌面充电站供电初始化

// 外设接口管理
PORT_State_Typedef CHARGE_GetPortState(PORT_ID_Typedef port_id);
uint8_t CHARGE_PortEnable(PORT_ID_Typedef port_id);
uint8_t CHARGE_PortDisable(PORT_ID_Typedef port_id);

// 保护功能
void CHARGE_OverVoltageProtect(void);  // 过压保护
void CHARGE_OverCurrentProtect(void);  // 过流保护

#endif