#ifndef __EXTEND_MODULE_H
#define __EXTEND_MODULE_H

#include "config.h"
#include "stm32f1xx_hal.h"

// 拓展模块统一接口结构体
typedef struct {
    void (*Init)(void);    // 模块初始化函数
    void (*Loop)(void);    // 模块循环调度函数
    void (*DeInit)(void);  // 模块卸载函数
} ExtendModule_Typedef;

// 拓展模块统一初始化入口
void Extend_ModuleInit(void);

// 拓展模块统一循环调度入口
void Extend_ModuleLoop(void);

#endif