# Keyboard as PC Hub

ESP32-S3圆屏控制台＋键盘直插USB Hub。当前方案为独立主机、磁吸五键模块和触控条；中央旋钮保留旋转/按压，外围四个eFlesh软键独立。操作面约5°，无须与键盘边框等宽对齐。

**2026-10-09状态：资料研究及散件验证规划。尚无可制造的PCB、完成的CAD或整机验收结果。** 用户已接好N16R8、圆屏、触控和MAX98357A，扬声器报告测试完成；其他功能按表逐项验证。

## 先看这些文件

1. [键盘接口兼容性与PCB尺寸研究](docs/keyboard-compatibility.md)：公开数据、未知尺寸、可调公头与100×80mm候选板框。
2. [接口、独立高速口与供电](docs/ports-and-power.md)：背部3A＋PC-C＋POWER-C，侧面FAST-C，前侧键盘公头。
3. [当前配件、GPIO和状态](hardware/current-wiring.md)：用户提供的11个GPIO基准。
4. [完整功能、散件验证和单板集成](docs/product-and-integration.md)：先组装验证，再绘制集成PCB。
5. [接口调查数据](hardware/keyboard-port-survey.csv)与[机械设计起始参数](mechanical/interface-parameters.json)。

## 功能与范围

保留现有ESP32-S3 N16R8、360×360圆形触控屏、MAX98357A、扬声器及调试扩展板。主机提供媒体/演示/桌面三种操作模式、圆屏界面、机械旋钮、四软键、五宏键、触控条、总禁用和USB HID。独立XIAO视觉节点在本地识别手势，通过ESP-NOW发送事件。

USB高速数据由独立Hub处理，ESP32仅作为下游HID设备。键盘从Hub获得数据连接与供电；主机及外设的电力由外部电源与管理电路分配，不要求键盘给主机反向供电。支持范围以USB有线数据键盘及实际试插结果为准。

## 文件状态

- `docs/`、`hardware/`、`mechanical/interface-parameters.json`：当前方案，尚待实物验证。
- `Functions/`、`STM32F103C8T6/`：原有历史示例，保留，不是当前ESP32固件。
- `设计图/`：历史草图资料，旧尺寸不用于当前制造。
- [此前首页原文](docs/history/README-before-2026-10-09.md)：保留历史思路，其旧预算和接线不作为当前依据。

本次没有上传本地设备备份、串口日志、账号资料或第三方整机CAD。开源参考链接在各方案中；后续引入代码/硬件设计时保留各自来源和许可证。
