/**
  ******************************************************************************
  * @file    light.c
  * @brief   模块A指示灯核心控制逻辑
  * @note    采用非阻塞调度，无阻塞delay，与主循环时间片联动
  ******************************************************************************
  */

#include "light.h"

// ====================== 指示灯硬件映射表（与config.h引脚定义联动） ======================
typedef struct {
    GPIO_TypeDef* port;
    uint16_t pin;
    LED_State_Typedef current_state;
    uint32_t blink_tick;
} LED_Info_Typedef;

static LED_Info_Typedef led_info_list[LED_ID_MAX] = {
    // 旋钮红灯
    {LED_ENC_RED_PORT, LED_ENC_RED_PIN, LED_OFF, 0},
    // 旋钮绿灯
    {LED_ENC_GREEN_PORT, LED_ENC_GREEN_PIN, LED_OFF, 0},
    // 供电状态灯
    {LED_POWER_PORT, LED_POWER_PIN, LED_OFF, 0},
    // 调试状态灯
    {LED_DEBUG_PORT, LED_DEBUG_PIN, LED_OFF, 0},
    // Hub高速状态灯
    {LED_HUB_PORT, LED_HUB_PIN, LED_OFF, 0}
};

// ====================== 闪烁周期配置 ======================
#define BLINK_SLOW_PERIOD  500  // 慢闪周期（ms）
#define BLINK_FAST_PERIOD  200  // 快闪周期（ms）

// ====================== 基础硬件接口实现 ======================
/**
  * @brief  指示灯GPIO初始化
  * @note   推挽输出模式，上电默认关闭
  */
void LED_Init(void)
{
    GPIO_InitTypeDef gpio_init_struct = {0};

    // 使能对应GPIO时钟（此处以PA/PB为例，可根据config.h修改）
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    // 批量初始化所有指示灯GPIO
    gpio_init_struct.Mode = GPIO_MODE_OUTPUT_PP;
    gpio_init_struct.Pull = GPIO_NOPULL;
    gpio_init_struct.Speed = GPIO_SPEED_FREQ_LOW;

    for(uint8_t i=0; i<LED_ID_MAX; i++)
    {
        gpio_init_struct.Pin = led_info_list[i].pin;
        HAL_GPIO_Init(led_info_list[i].port, &gpio_init_struct);
        // 上电默认关闭
        HAL_GPIO_WritePin(led_info_list[i].port, led_info_list[i].pin, GPIO_PIN_RESET);
        led_info_list[i].current_state = LED_OFF;
        led_info_list[i].blink_tick = 0;
    }
}

/**
  * @brief  单个指示灯状态设置
  * @param  led_id: 指示灯ID
  * @param  state: 目标状态（关/亮/慢闪/快闪）
  */
void LED_SetState(LED_ID_Typedef led_id, LED_State_Typedef state)
{
    if(led_id >= LED_ID_MAX) return;

    led_info_list[led_id].current_state = state;
    led_info_list[led_id].blink_tick = g_sys_tick_cnt;

    // 常亮/常闭直接设置电平
    if(state == LED_ON)
    {
        HAL_GPIO_WritePin(led_info_list[led_id].port, led_info_list[led_id].pin, GPIO_PIN_SET);
    }
    else if(state == LED_OFF)
    {
        HAL_GPIO_WritePin(led_info_list[led_id].port, led_info_list[led_id].pin, GPIO_PIN_RESET);
    }
}

/**
  * @brief  指示灯状态刷新任务
  * @note   主循环10ms时间片调度，处理闪烁逻辑，无阻塞
  */
void LED_Task(void)
{
    uint32_t current_tick = g_sys_tick_cnt;

    for(uint8_t i=0; i<LED_ID_MAX; i++)
    {
        // 仅处理闪烁状态
        if(led_info_list[i].current_state == LED_BLINK_SLOW)
        {
            if(current_tick - led_info_list[i].blink_tick >= BLINK_SLOW_PERIOD)
            {
                HAL_GPIO_TogglePin(led_info_list[i].port, led_info_list[i].pin);
                led_info_list[i].blink_tick = current_tick;
            }
        }
        else if(led_info_list[i].current_state == LED_BLINK_FAST)
        {
            if(current_tick - led_info_list[i].blink_tick >= BLINK_FAST_PERIOD)
            {
                HAL_GPIO_TogglePin(led_info_list[i].port, led_info_list[i].pin);
                led_info_list[i].blink_tick = current_tick;
            }
        }
    }
}

// ====================== 业务逻辑联动实现 ======================
/**
  * @brief  跟随系统状态自动更新灯效
  * @note   主循环100ms调度一次，与全局系统状态联动
  */
void LED_UpdateBySystemStatus(void)
{
    // 1. 旋钮模式指示灯：亮度模式红灯常亮，音量模式红灯关闭
    if(g_system_status.enc_mode == 1)
    {
        LED_SetState(LED_ID_ENC_RED, LED_ON);
    }
    else
    {
        LED_SetState(LED_ID_ENC_RED, LED_OFF);
    }

    // 2. 供电状态指示灯：PD供电模式常亮，电脑供电模式关闭
    if(g_system_status.power_mode == 1)
    {
        LED_SetState(LED_ID_POWER, LED_ON);
    }
    else
    {
        LED_SetState(LED_ID_POWER, LED_OFF);
    }

    // 3. VS Code调试状态：调试模式快闪，正常模式关闭
    if(g_system_status.vs_code_mode == 1)
    {
        LED_SetState(LED_ID_DEBUG, LED_BLINK_FAST);
    }
    else
    {
        LED_SetState(LED_ID_DEBUG, LED_OFF);
    }
}

/**
  * @brief  调试触发单次闪烁
  * @note   按下调试按键时调用，给用户操作反馈
  */
void LED_TriggerDebugBlink(void)
{
    LED_SetState(LED_ID_DEBUG, LED_ON);
    HAL_Delay(50);  // 短延时不影响主逻辑，仅做单次点亮反馈
    LED_SetState(LED_ID_DEBUG, LED_OFF);
}

/**
  * @brief  高速接口传输触发闪烁
  * @note   Hub高速接口有数据传输时调用，指示工作状态
  */
void LED_TriggerHubBlink(void)
{
    static uint32_t last_tick = 0;
    if(g_sys_tick_cnt - last_tick >= 100)  // 限制闪烁频率
    {
        HAL_GPIO_TogglePin(led_info_list[LED_ID_HUB].port, led_info_list[LED_ID_HUB].pin);
        last_tick = g_sys_tick_cnt;
    }
}

// ====================== 拓展模块预留接口实现 ======================
/**
  * @brief  拓展模块指示灯控制
  * @param  extend_led_id: 拓展灯ID（从LED_ID_MAX开始）
  * @param  state: 目标状态
  * @note   后续模块B/C/D新增指示灯时调用，无需修改核心逻辑
  */
void LED_ExtendSetState(uint8_t extend_led_id, LED_State_Typedef state)
{
    // 拓展灯实现逻辑，后续新增硬件时填充
    // 示例：拓展灯1对应模块B的RGB灯效，直接调用对应控制函数
}