/**
  ******************************************************************************
  * @file    hub_for_pc.c
  * @brief   与PC连接的核心数据交互模块
  * @note    仅数据传输，不从PC取电，承载所有板载/外设/拓展模块的数据流
  ******************************************************************************
  */

#include "hub_for_pc.h"
#include "usbd_core.h"
#include "usbd_desc.h"
#include "usbd_composite.h"

// ====================== 私有变量 ======================
// USB复合设备句柄
static USBD_HandleTypeDef hUsbDeviceFS;
// 拓展模块回调函数表
static HUB_ReceiveCallback receive_callback[HUB_DATA_TYPE_MAX] = {NULL};
static HUB_SendCompleteCallback send_complete_callback[HUB_DATA_TYPE_MAX] = {NULL};
// PC连接状态
static uint8_t pc_connected_flag = 0;

// ====================== 核心函数实现 ======================
/**
  * @brief  USB复合设备初始化
  * @note   仅启用数据传输，关闭VBUS取电，完全依赖外部供电
  */
void HUB_for_PC_Init(void)
{
    // USB底层初始化
    USBD_Init(&hUsbDeviceFS, &FS_Desc, DEVICE_FS);
    USBD_RegisterClass(&hUsbDeviceFS, &USBD_Composite);
    USBD_Start(&hUsbDeviceFS);

    // 初始化状态
    pc_connected_flag = 0;
    for(uint8_t i=0; i<HUB_DATA_TYPE_MAX; i++)
    {
        receive_callback[i] = NULL;
        send_complete_callback[i] = NULL;
    }
}

/**
  * @brief  向PC发送指定类型的数据
  */
uint8_t HUB_for_PC_SendData(HUB_DataType_Typedef type, uint8_t *data, uint16_t len)
{
    if(type >= HUB_DATA_TYPE_MAX || data == NULL || len == 0) return 1;
    if(!HUB_for_PC_IsConnected()) return 2;

    // 对应类型数据发送实现（USB复合设备对应接口发送）
    // 此处为框架占位，后续根据USB设备驱动填充
    switch(type)
    {
        case HUB_DATA_TYPE_HID_KEY:
            // HID键盘报文发送
            break;
        case HUB_DATA_TYPE_HID_CONSUMER:
            // HID多媒体报文发送
            break;
        case HUB_DATA_TYPE_CDC_DEBUG:
            // CDC虚拟串口数据发送
            break;
        case HUB_DATA_TYPE_HUB_FORWARD:
            // 外设接口数据转发
            break;
        case HUB_DATA_TYPE_EXTEND_MODULE:
            // 拓展模块数据发送
            break;
        default:
            break;
    }

    return 0;
}

/**
  * @brief  从PC接收指定类型的数据
  */
uint8_t HUB_for_PC_ReceiveData(HUB_DataType_Typedef type, uint8_t *buf, uint16_t max_len)
{
    if(type >= HUB_DATA_TYPE_MAX || buf == NULL || max_len == 0) return 1;
    // 对应类型数据接收实现，框架占位
    return 0;
}

/**
  * @brief  注册拓展模块接收回调
  */
void HUB_for_PC_RegisterReceiveCallback(HUB_DataType_Typedef type, HUB_ReceiveCallback callback)
{
    if(type >= HUB_DATA_TYPE_MAX) return;
    receive_callback[type] = callback;
}

/**
  * @brief  注册拓展模块发送完成回调
  */
void HUB_for_PC_RegisterSendCompleteCallback(HUB_DataType_Typedef type, HUB_SendCompleteCallback callback)
{
    if(type >= HUB_DATA_TYPE_MAX) return;
    send_complete_callback[type] = callback;
}

/**
  * @brief  检测与PC的连接状态
  */
uint8_t HUB_for_PC_IsConnected(void)
{
    return pc_connected_flag;
}

/**
  * @brief  主循环调度任务
  * @note   主循环10ms时间片调度，处理数据收发与状态更新
  */
void HUB_for_PC_Task(void)
{
    // 1. 更新PC连接状态
    pc_connected_flag = (USBD_GetState(&hUsbDeviceFS) == USBD_STATE_CONFIGURED) ? 1 : 0;

    // 2. 处理接收数据，调用对应拓展模块回调
    for(uint8_t i=0; i<HUB_DATA_TYPE_MAX; i++)
    {
        if(receive_callback[i] != NULL)
        {
            uint8_t recv_buf[64] = {0};
            uint16_t recv_len = 0;
            if(HUB_for_PC_ReceiveData(i, recv_buf, 64) == 0 && recv_len > 0)
            {
                receive_callback[i](recv_buf, recv_len);
            }
        }
    }
}

// ====================== USB中断回调函数 ======================
// USB连接状态变更回调
void USBD_ConnectStateChangedCallback(uint8_t state)
{
    pc_connected_flag = state;
}

// 数据接收完成回调
void USBD_DataReceivedCallback(uint8_t type, uint8_t *data, uint16_t len)
{
    if(type < HUB_DATA_TYPE_MAX && receive_callback[type] != NULL)
    {
        receive_callback[type](data, len);
    }
}

// 数据发送完成回调
void USBD_DataSendCompleteCallback(uint8_t type)
{
    if(type < HUB_DATA_TYPE_MAX && send_complete_callback[type] != NULL)
    {
        send_complete_callback[type]();
    }
}