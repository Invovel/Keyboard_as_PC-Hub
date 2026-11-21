#ifndef __HUB_FOR_PC_H
#define __HUB_FOR_PC_H

#include "stm32f1xx_hal.h"
#include "config.h"

// ====================== USB复合设备接口定义 ======================
// 数据流分类：所有板载/拓展设备的数据均通过该接口统一与PC交互
typedef enum {
    HUB_DATA_TYPE_HID_KEY = 0,    // 按键/拨片/旋钮HID快捷键数据
    HUB_DATA_TYPE_HID_CONSUMER,   // 多媒体/系统控制HID数据
    HUB_DATA_TYPE_CDC_DEBUG,      // 调试/屏幕状态串口数据
    HUB_DATA_TYPE_HUB_FORWARD,    // 外设接口（A/C口）数据转发
    HUB_DATA_TYPE_EXTEND_MODULE,   // 后续拓展模块数据预留
    HUB_DATA_TYPE_MAX
} HUB_DataType_Typedef;

// ====================== 拓展模块数据回调预留 ======================
typedef void (*HUB_ReceiveCallback)(uint8_t *data, uint16_t len);
typedef void (*HUB_SendCompleteCallback)(void);

// ====================== 函数接口声明 ======================
// 核心初始化：USB复合设备初始化，仅数据传输，不从PC取电
void HUB_for_PC_Init(void);

// 核心数据收发接口
uint8_t HUB_for_PC_SendData(HUB_DataType_Typedef type, uint8_t *data, uint16_t len);
uint8_t HUB_for_PC_ReceiveData(HUB_DataType_Typedef type, uint8_t *buf, uint16_t max_len);

// 拓展模块预留接口：注册数据收发回调，新增模块无需修改核心代码
void HUB_for_PC_RegisterReceiveCallback(HUB_DataType_Typedef type, HUB_ReceiveCallback callback);
void HUB_for_PC_RegisterSendCompleteCallback(HUB_DataType_Typedef type, HUB_SendCompleteCallback callback);

// 状态检测接口
uint8_t HUB_for_PC_IsConnected(void);  // 检测与PC的连接状态
void HUB_for_PC_Task(void);             // 主循环时间片调度任务

#endif