# 用户确认的当前硬件与接线基准

日期：2026-10-09。依据用户本次提供的硬件表；优先于旧文档。记录接线不等于本轮重新通电测试。没有打开串口、刷写或修改硬件。

## 已有配件

ESP32-S3 N16R8、GPIO Extension Board、TK015F5785_PB圆形360×360屏及板载I²C触控、MAX98357A、4–8Ω小型扬声器、可用Type-C供电/烧录/串口连接。全部复用，不另买屏幕或功放。

## 固定GPIO

| 信号 | GPIO | 接口 |
| --- | ---: | --- |
| LCD CS | 16 | 下方6Pin QSPI |
| LCD SCK | 6 | 下方6Pin QSPI |
| LCD D0 | 15 | 下方6Pin QSPI |
| LCD D1 | 7 | 下方6Pin QSPI |
| LCD D2 | 11 | 下方6Pin QSPI |
| LCD D3 | 12 | 下方6Pin QSPI |
| Touch SDA | 8 | 上方4Pin |
| Touch SCL | 9 | 上方4Pin |
| Audio BCLK | 47 | MAX98357A BCLK |
| Audio LRC/WS | 48 | MAX98357A LRC |
| Audio DIN | 21 | MAX98357A DIN |

共11个已接GPIO。音频不使用GPIO4/5/17，但“未用于音频”不等于已经为新功能分配。

屏幕4Pin是SDA/SCL/3V3/GND，没有接INT/RST。触控地址及控制器型号以本次实际扫描/驱动验证为准；不能把旧记录0x15当作今天扫描成功。

MAX98357A VIN接5V，GAIN/SD按现有模块默认状态未接。功放OUT+与OUT−只接扬声器两端；OUT−不是系统GND。保留扬声器原2Pin插头，配匹配插座到裸线的转接线，插座间距和极性仍需核对。

## 电压与USB

- 屏幕/触控3.3V；功放VIN 5V；逻辑系统共地。
- 扩展板标识、跳线与实际电压一致后使用，不能仅凭线排颜色判断。
- 当前Type-C可烧录/串口，不足以证明该口直接连接原生USB。
- 原生USB D−=GPIO19、D+=GPIO20，预留给HID；另需核对开发板接口线路。GPIO26–37涉及Flash/PSRAM时不可随意复用，0/3/45/46有启动约束，具体以模组和板级图为准。

来源：[Espressif原生USB](https://docs.espressif.com/projects/esp-usb/en/latest/esp32s3/usb_device.html)、[GPIO约束](https://docs.espressif.com/projects/esp-idf/en/v6.0/esp32s3/api-reference/peripherals/gpio.html)。

成品Hub和多个外设不从ESP32开发板5V针脚向外供电，改由独立受保护5V电源树分配，见[接口供电方案](../docs/ports-and-power.md)。

## 当前状态

| 项目 | 状态 |
| --- | --- |
| 主控上电、USB供电、圆屏3.3V、背光 | 用户已确认 |
| LCD图像、触控扫描、触摸坐标 | 待当前配置测试 |
| MAX98357A接线 | 已完成 |
| 扬声器播放 | 用户报告完成测试 |
| I²S整合 | 用户表同时标软件待测，故保留为当前整机需复测，不宣称音频整合通过 |

测试顺序：ESP32/PSRAM → I²C扫描 → LCD纯色 → 触摸坐标 → 复测音频 → 整合。

电气接线已明确；屏幕PCB长宽厚、孔位、排线出口、旋钮机构高度仍未提供。360×360是像素分辨率，TK015F5785_PB是型号，均不能替代毫米尺寸；在机械模型中保留参数。
