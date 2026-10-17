/**
  ******************************************************************************
  * @file    charge.c
  * @brief   供电管理+外设Hub接口模块
  * @note    完全外部供电，不从PC取电，管理所有Type-C/USB-A接口
  ******************************************************************************
  */

#include "charge.h"
#include "vl813_hub.h"  // VL813 USB3.0 Hub芯片驱动

// ====================== 私有变量 ======================
static POWER_Mode_Typedef current_power_mode = POWER_MODE_OFF;
static PORT_State_Typedef port_state_list[PORT_TYPE_MAX] = {PORT_STATE_IDLE};
static uint32_t protect_tick = 0;

// ====================== 基础初始化 ======================
/**
  * @brief  供电与外设Hub模块初始化
  */
void CHARGE_Init(void)
{
    // 1. ADC初始化（用于电压/电流检测）
    // 此处为框架占位，后续根据ADC配置填充

    // 2. VL813 USB3.0 Hub芯片初始化
    VL813_Hub_Init();

    // 3. 外设接口GPIO初始化
    // 此处为框架占位，后续根据引脚配置填充

    // 4. 供电模式初始化（预留两种方案，默认关闭）
    current_power_mode = POWER_MODE_OFF;
    for(uint8_t i=0; i<PORT_TYPE_MAX; i++)
    {
        port_state_list[i] = PORT_STATE_IDLE;
    }
    protect_tick = g_sys_tick_cnt;
}

// ====================== 供电管理实现 ======================
/**
  * @brief  获取当前供电模式
  */
POWER_Mode_Typedef CHARGE_GetPowerMode(void)
{
    return current_power_mode;
}

/**
  * @brief  获取当前供电电压
  */
uint8_t CHARGE_GetPowerVoltage(void)
{
    // ADC采样获取电压，框架占位
    return 5;  // 默认5V
}

/**
  * @brief  获取当前输出电流
  */
uint8_t CHARGE_GetPowerCurrent(void)
{
    // ADC采样获取电流，框架占位
    return 2;  // 默认2A
}

// ====================== 预留两种供电方案函数（仅占位） ======================
/**
  * @brief  独立充电器供电初始化（一加/三星）
  */
uint8_t CHARGE_ChargerSupply_Init(void)
{
    // 独立充电器供电协议初始化、PD诱骗等实现
    current_power_mode = POWER_MODE_CHARGER;
    return 0;
}

/**
  * @brief  桌面充电站供电初始化（酷态科）
  */
uint8_t CHARGE_ChargeStationSupply_Init(void)
{
    // 桌面充电站供电协议初始化、PD诱骗、米家联动对接等实现
    current_power_mode = POWER_MODE_CHARGE_STATION;
    return 0;
}

// ====================== 外设接口管理实现 ======================
/**
  * @brief  获取指定接口状态
  */
PORT_State_Typedef CHARGE_GetPortState(PORT_ID_Typedef port_id)
{
    if(port_id >= PORT_TYPE_MAX) return PORT_STATE_ERROR;
    return port_state_list[port_id];
}

/**
  * @brief  启用指定接口
  */
uint8_t CHARGE_PortEnable(PORT_ID_Typedef port_id)
{
    if(port_id >= PORT_TYPE_MAX) return 1;
    // Hub芯片端口使能实现，框架占位
    port_state_list[port_id] = PORT_STATE_IDLE;
    return 0;
}

/**
  * @brief  禁用指定接口
  */
uint8_t CHARGE_PortDisable(PORT_ID_Typedef port_id)
{
    if(port_id >= PORT_TYPE_MAX) return 1;
    // Hub芯片端口禁用实现，框架占位
    port_state_list[port_id] = PORT_STATE_IDLE;
    return 0;
}

// ====================== 保护功能实现 ======================
/**
  * @brief  过压保护
  */
void CHARGE_OverVoltageProtect(void)
{
    uint8_t voltage = CHARGE_GetPowerVoltage();
    if(voltage > 6)  // 过压阈值6V
    {
        // 关闭所有接口输出，进入保护状态
        for(uint8_t i=0; i<PORT_TYPE_MAX; i++)
        {
            CHARGE_PortDisable(i);
        }
        current_power_mode = POWER_MODE_ERROR;
    }
}

/**
  * @brief  过流保护
  */
void CHARGE_OverCurrentProtect(void)
{
    uint8_t current = CHARGE_GetPowerCurrent();
    if(current > 5)  // 过流阈值5A
    {
        // 关闭所有接口输出，进入保护状态
        for(uint8_t i=0; i<PORT_TYPE_MAX; i++)
        {
            CHARGE_PortDisable(i);
        }
        current_power_mode = POWER_MODE_ERROR;
    }
}

// ====================== 主循环调度任务 ======================
/**
  * @brief  主循环调度任务
  * @note   主循环10ms时间片调度
  */
void CHARGE_Task(void)
{
    uint32_t current_tick = g_sys_tick_cnt;

    // 1. 更新所有接口状态
    for(uint8_t i=0; i<PORT_TYPE_MAX; i++)
    {
        // 通过Hub芯片读取端口状态，框架占位
        port_state_list[i] = VL813_Hub_GetPortState(i);

        // 接口数据传输中，同步转发数据到PC
        if(port_state_list[i] == PORT_STATE_DATA_TRANS)
        {
            uint8_t trans_buf[64] = {0};
            uint16_t trans_len = 0;
            if(VL813_Hub_GetPortData(i, trans_buf, &trans_len) == 0 && trans_len > 0)
            {
                HUB_for_PC_SendData(HUB_DATA_TYPE_HUB_FORWARD, trans_buf, trans_len);
            }
        }
    }

    // 2. 供电保护检测（100ms执行一次）
    if(current_tick - protect_tick >= 100)
    {
        CHARGE_OverVoltageProtect();
        CHARGE_OverCurrentProtect();
        protect_tick = current_tick;
    }
}