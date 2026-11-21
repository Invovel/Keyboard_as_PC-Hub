/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : 模块A（STM32版）主程序入口
  * @mcu            : STM32F103C8T6
  ******************************************************************************
  */

/* ***************************** 头文件包含 *********************************** */
// STM32底层核心头文件
#include "stm32f1xx_hal.h"

// 全局配置头文件
#include "config.h"

// 硬件模块头文件（按功能分层）
#include "light.h"            // 指示灯模块
#include "trigger.h"          // 类雷蛇拨片模块
#include "charge.h"           // 供电+外设Hub模块
#include "hub_for_pc.h"       // PC数据交互核心模块
#include "bsp_driver.h"       // 硬件驱动层统一接口

// 核心应用+拓展模块头文件
#include "core_app.h"         // 核心业务逻辑
#include "extend_module.h"    // 拓展模块预留接口

/* ***************************** 私有函数声明 ********************************* */
static void SystemClock_Config(void);  // 系统时钟配置（STM32标准底层）
static void Error_Handler(void);        // 全局错误处理函数

/* ***************************** 全局变量定义 ********************************* */
// 系统滴答计数器（1ms递增，SysTick中断驱动，volatile确保中断可见）
volatile uint32_t g_sys_tick_cnt = 0;

// 全局系统状态结构体（所有模块共享）
SystemStatus_Typedef g_system_status = {
    .enc_mode = 0,            // 旋钮默认模式：0-音量 1-亮度
    .power_mode = 0,          // 供电默认模式：0-电脑供电 1-PD供电
    .vs_code_mode = 0,        // VS Code调试模式：0-正常 1-调试中
    .current_volume = 50,     // 默认音量50%
    .current_bright = 80      // 默认亮度80%
};

/* ***************************** 中断服务函数 ********************************* */
/**
  * @brief  系统滴答定时器中断服务函数
  * @note   每1ms触发一次，仅更新全局计数器，不执行业务逻辑
  */
void SysTick_Handler(void)
{
    HAL_IncTick();
    g_sys_tick_cnt++;
}

/* ***************************** 主程序入口 *********************************** */
int main(void)
{
    /* ========================= 第一步：STM32底层初始化 ========================= */
    // 1. HAL库初始化（初始化系统时钟、中断优先级等）
    HAL_Init();
    
    // 2. 系统时钟配置（72MHz主频）
    SystemClock_Config();

    /* ========================= 第二步：硬件模块初始化 ========================= */
    // 1. 硬件驱动统一初始化
    BSP_DriverInit();
    
    // 2. 指示灯模块初始化
    LED_Init();
    
    // 3. 类雷蛇拨片模块初始化
    TRIGGER_Init();
    
    // 4. 供电+外设Hub模块初始化
    CHARGE_Init();
    
    // 5. PC数据交互核心初始化
    HUB_for_PC_Init();

    /* ========================= 第三步：业务层初始化 ========================= */
    // 1. 核心应用逻辑初始化
    Core_AppInit();
    
    // 2. 拓展模块初始化（无拓展时为空执行）
    Extend_ModuleInit();

    /* ========================= 第四步：主循环（非阻塞调度） ========================= */
    while (1)
    {
        /* ------------------------ 10ms 基础任务调度 ------------------------ */
        if (g_sys_tick_cnt % 10 == 0)
        {
            LED_Task();               // 指示灯状态刷新
            TRIGGER_Task();           // 拨片扫描+连续触发逻辑
            CHARGE_Task();            // 供电检测+外设端口状态更新
            HUB_for_PC_Task();        // PC数据收发+拓展模块回调
        }

        /* ------------------------ 50ms OLED刷新调度 ------------------------ */
        if (g_sys_tick_cnt % 50 == 0)
        {
            Core_OLEDTask();          // OLED显示状态刷新
        }

        /* ------------------------ 100ms 系统状态调度 ------------------------ */
        if (g_sys_tick_cnt % 100 == 0)
        {
            Core_PowerTask();         // 供电模式检测+切换
            LED_UpdateBySystemStatus(); // 指示灯跟随系统状态更新
            CHARGE_OverVoltageProtect(); // 过压保护检测
            CHARGE_OverCurrentProtect(); // 过流保护检测
        }

        /* ------------------------ 按键/编码器/HID任务调度 ------------------------ */
        if (g_sys_tick_cnt % KEY_SCAN_INTERVAL == 0)
        {
            Core_KeyTask();           // 热插拔按键扫描
        }
        if (g_sys_tick_cnt % ENC_SCAN_INTERVAL == 0)
        {
            Core_EncoderTask();       // 旋钮扫描
        }
        if (g_sys_tick_cnt % HID_REPORT_INTERVAL == 0)
        {
            Core_HIDTask();           // HID指令上报
        }

        /* ------------------------ 拓展模块循环调度 ------------------------ */
        if (g_sys_tick_cnt % 10 == 0)
        {
            Extend_ModuleLoop();      // 拓展模块统一调度
        }

        /* ------------------------ 计数器溢出保护 ------------------------ */
        if (g_sys_tick_cnt >= 1000)
        {
            g_sys_tick_cnt = 0;
        }
    }
}

/* ***************************** 私有函数占位实现 ********************************* */
/**
  * @brief  系统时钟配置函数
  * @note   STM32F103C8T6 72MHz主频标准配置
  */
static void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    // 配置外部8MHz晶振作为PLL输入，倍频到72MHz
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        Error_Handler();
    }

    // 配置系统时钟、AHB/APB分频
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
    {
        Error_Handler();
    }
}

/**
  * @brief  全局错误处理函数
  * @note   硬件初始化/外设异常时触发
  */
static void Error_Handler(void)
{
    // 错误状态：指示灯快闪提示
    LED_SetState(LED_ID_POWER, LED_BLINK_FAST);
    LED_SetState(LED_ID_DEBUG, LED_BLINK_FAST);
    LED_SetState(LED_ID_HUB, LED_BLINK_FAST);
    
    // 异常死循环，停止业务逻辑
    while(1)
    {
        // 空循环，仅做异常锁定
    }
}

/* ***************************** 硬件驱动统一初始化实现 ********************************* */
/**
  * @brief  所有硬件驱动统一初始化
  */
void BSP_DriverInit(void)
{
    // 此处统一初始化GPIO、I2C、SPI、ADC等底层外设
    // 占位实现，后续按需补充
}

/* ***************************** 核心应用层占位实现 ********************************* */
/**
  * @brief  核心应用逻辑初始化
  */
void Core_AppInit(void)
{
    // 占位实现，后续补充按键映射、HID初始化等
}

void Core_KeyTask(void)
{
    // 占位实现，按键扫描逻辑
}

void Core_EncoderTask(void)
{
    // 占位实现，旋钮扫描逻辑
}

void Core_SwitchTask(void)
{
    // 占位实现，拨片开关逻辑
}

void Core_OLEDTask(void)
{
    // 占位实现，OLED屏幕刷新逻辑
}

void Core_PowerTask(void)
{
    // 占位实现，供电模式检测逻辑
}

void Core_HIDTask(void)
{
    // 占位实现，HID指令上报逻辑
}

/* ***************************** 拓展模块占位实现 ********************************* */
/**
  * @brief  拓展模块统一初始化
  */
void Extend_ModuleInit(void)
{
    // 占位实现，后续新增模块在此处初始化
}

/**
  * @brief  拓展模块统一循环调度
  */
void Extend_ModuleLoop(void)
{
    // 占位实现，后续新增模块在此处调度
}

#ifdef USE_FULL_ASSERT
/**
  * @brief  断言失败回调函数
  */
void assert_failed(uint8_t *file, uint32_t line)
{
    // 占位实现，调试用
}
#endif