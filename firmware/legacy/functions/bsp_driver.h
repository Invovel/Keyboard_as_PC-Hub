#ifndef __BSP_DRIVER_H
#define __BSP_DRIVER_H

#include "config.h"
#include "light.h"
#include "trigger.h"
#include "charge.h"
#include "hub_for_pc.h"

// 所有硬件驱动统一初始化入口
void BSP_DriverInit(void);

#endif