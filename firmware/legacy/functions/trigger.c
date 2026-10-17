/**
  ******************************************************************************
  * @file    trigger.c
  * @brief   类雷蛇滚轮拨片模块核心逻辑
  * @note    适配复位型3脚3档钮子开关，非阻塞调度，与主循环10ms时间片联动
  ******************************************************************************
  */

#include "trigger.h"

// ====================== 硬件引脚映射（与config.h联动） ======================
typedef struct {
    GPIO_TypeDef* port;
    uint16_t pin;
} TRIGGER_Pin_Typedef;

// 左/右拨片引脚，对应config.h中定义的开关引脚
static const TRIGGER_Pin_Typedef trigger_pin[2] = {
    {SWITCH_LEFT_PORT, SWITCH_LEFT_PIN},   // 左拨引脚
    {SWITCH_RIGHT_PORT, SWITCH_RIGHT_PIN}  // 右拨引脚
};

// ====================== 静态私有变量 ======================
static TRIGGER_State_Typedef current_state = TRIGGER_STATE_IDLE;  // 当前状态
static TRIGGER_Mode_Typedef current_mode = TRIGGER_MODE_SHORT;     // 当前触发模式
static uint32_t debounce_tick = 0;       // 防抖计时
static uint32_t press_tick = 0;          // 按下持续计时
static uint32_t trigger_tick = 0;        // 持续触发间隔计时
static uint8_t is_triggered = 0;         // 首次触发标记
static TRIGGER_Callback user_callback = NULL;  // 用户自定义回调

// ====================== 基础硬件接口实现 ======================
/**
  * @brief  拨片GPIO初始化
  * @note   上拉输入模式，公共端接GND，左/右拨对应引脚拉低
  */
void TRIGGER_Init(void)
{
    GPIO_InitTypeDef gpio_init_struct = {0};

    // 使能GPIO时钟（与config.h引脚对应）
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    // 批量初始化左/右拨片引脚
    gpio_init_struct.Mode = GPIO_MODE_INPUT;
    gpio_init_struct.Pull = GPIO_PULLUP;  // 上拉输入，默认高电平
    gpio_init_struct.Speed = GPIO_SPEED_FREQ_LOW;

    for(uint8_t i=0; i<2; i++)
    {
        gpio_init_struct.Pin = trigger_pin[i].pin;
        HAL_GPIO_Init(trigger_pin[i].port, &gpio_init_struct);
    }

    // 初始化状态
    current_state = TRIGGER_STATE_IDLE;
    current_mode = TRIGGER_MODE_SHORT;
    debounce_tick = g_sys_tick_cnt;
}

/**
  * @brief  获取当前拨片状态
  */
TRIGGER_State_Typedef TRIGGER_GetState(void)
{
    return current_state;
}

/**
  * @brief  获取当前触发模式
  */
TRIGGER_Mode_Typedef TRIGGER_GetMode(void)
{
    return current_mode;
}

/**
  * @brief  注册用户自定义触发回调
  * @note   后续拓展模块可重写触发逻辑，无需修改核心代码
  */
void TRIGGER_SetCallback(TRIGGER_Callback callback)
{
    user_callback = callback;
}

// ====================== 核心触发逻辑实现 ======================
/**
  * @brief  拨片扫描与触发任务
  * @note   主循环10ms时间片调度，非阻塞实现，完全复刻雷蛇滚轮逻辑
  */
void TRIGGER_Task(void)
{
    uint32_t current_tick = g_sys_tick_cnt;
    TRIGGER_State_Typedef raw_state = TRIGGER_STATE_IDLE;

    // 1. 读取原始引脚状态
    GPIO_PinState left_level = HAL_GPIO_ReadPin(trigger_pin[0].port, trigger_pin[0].pin);
    GPIO_PinState right_level = HAL_GPIO_ReadPin(trigger_pin[1].port, trigger_pin[1].pin);

    // 左拨：左引脚拉低，右引脚高电平
    if(left_level == GPIO_PIN_RESET && right_level == GPIO_PIN_SET)
    {
        raw_state = TRIGGER_STATE_LEFT;
    }
    // 右拨：右引脚拉低，左引脚高电平
    else if(right_level == GPIO_PIN_RESET && left_level == GPIO_PIN_SET)
    {
        raw_state = TRIGGER_STATE_RIGHT;
    }
    // 中间复位：两个引脚均为高电平
    else
    {
        raw_state = TRIGGER_STATE_IDLE;
    }

    // 2. 防抖处理
    if(raw_state != current_state)
    {
        if(current_tick - debounce_tick >= TRIGGER_DEBOUNCE_TIME)
        {
            // 防抖通过，更新状态
            current_state = raw_state;
            debounce_tick = current_tick;
            // 状态切换时重置标记
            is_triggered = 0;
            press_tick = current_tick;
            trigger_tick = current_tick;
        }
    }
    else
    {
        debounce_tick = current_tick;
    }

    // 3. 核心触发逻辑：持续触发→松手停止
    if(current_state != TRIGGER_STATE_IDLE)
    {
        // 计算按下持续时间，判断短按/长按模式
        if(current_tick - press_tick >= TRIGGER_LONG_PRESS_TIME)
        {
            current_mode = TRIGGER_MODE_LONG;
        }
        else
        {
            current_mode = TRIGGER_MODE_SHORT;
        }

        // 首次触发立即执行，后续按间隔持续触发（类雷蛇滚轮连续滚动）
        if(is_triggered == 0 || current_tick - trigger_tick >= TRIGGER_INTERVAL)
        {
            // 执行HID快捷键发送
            if(current_state == TRIGGER_STATE_LEFT)
            {
                if(current_mode == TRIGGER_MODE_SHORT)
                {
                    // 短按左拨：VS Code上一标签
                    BSP_USBHID_SendKey(KEY_CTRL | KEY_PAGE_UP);
                }
                else
                {
                    // 长按左拨：全局上一窗口
                    BSP_USBHID_SendKey(KEY_ALT | KEY_TAB);
                }
                // 联动指示灯：触发时闪烁Hub状态灯
                LED_TriggerHubBlink();
            }
            else if(current_state == TRIGGER_STATE_RIGHT)
            {
                if(current_mode == TRIGGER_MODE_SHORT)
                {
                    // 短按右拨：VS Code下一标签
                    BSP_USBHID_SendKey(KEY_CTRL | KEY_PAGE_DOWN);
                }
                else
                {
                    // 长按右拨：全局下一窗口
                    BSP_USBHID_SendKey(KEY_ALT | KEY_SHIFT | KEY_TAB);
                }
                // 联动指示灯：触发时闪烁Hub状态灯
                LED_TriggerHubBlink();
            }

            // 执行用户自定义回调（预留拓展）
            if(user_callback != NULL)
            {
                user_callback(current_state, current_mode);
            }

            // 更新触发计时
            trigger_tick = current_tick;
            is_triggered = 1;
        }
    }
    else
    {
        // 松手复位：重置所有状态，立即停止触发
        current_mode = TRIGGER_MODE_SHORT;
        is_triggered = 0;
        press_tick = current_tick;
        trigger_tick = current_tick;
    }
}