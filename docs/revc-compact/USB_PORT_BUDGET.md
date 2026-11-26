# RevC Compact USB 端口预算

2026-11-25 · 原理图分配候选

USB7206C 有五个同时兼容 SuperSpeed 与 USB2 的标准下游口，以及一个 USB2 专用下游口。以下分配复用现行键盘 P5 命名，将纯 USB2 的隔离与 ESP32 支路放在 P6。P1–P4 分配为本轮设计选择，最终随可布线性和配置验证确认。[Microchip USB7206C 数据表](https://ww1.microchip.com/downloads/aemDocuments/documents/NCS/ProductDocuments/DataSheets/USB7206C-Data-Sheet-DS00003850.pdf)

| Hub 口 | 分配 | 数据与电源要求 |
| --- | --- | --- |
| Upstream | PC-C | 唯一电脑上行。Gen 2 和 USB2，Type-C 数据角色、翻转、CC、VBUS 检测、防反灌和输入预算。 |
| P1 | A1 | Gen 2 和 USB2，独立受控 VBUS、故障切断、过流回报、端口 ESD。 |
| P2 | A2 | Gen 2 和 USB2，与 A1 分别验证。 |
| P3 | FAST-C | Gen 2 和 USB2，下游 Type-C Source/DFP 的 CC、插向处理、VBUS 放电、过流、ESD。 |
| P4 | Dock 数据预留 | 研究下游数据与附件供电角色，保留 USB2 和 SS 能力的设计空间。Dock 线束、速率、供电和配置均待定。 |
| P5 | CC64-RGB 键盘 | 仅接 USB2 和受控 VBUS，保留原 P5 逻辑。JB1 四针与浮动公头之间使用短 USB2 数据线，CC 在公头方案闭合。P5 的 SS 端按芯片配置处理。 |
| P6 | CH318T-A | USB2 专用 DP6/DM6 分别对应 USB7206C 42/41 脚，主地侧连接 CH318T-A 上行。配置和电源管理用芯片参考电路核对。 |
| CH318T-A 本地下游 | ESP32-S3 | 原生 USB FS 设备，独立 HID / 控制。验证隔离侧断电、反复插拔或失联时的枚举和恢复。 |
| CH318T-B 下游之一 | 右后 ISO-A | USB2 HS 480 Mbps 目标。独立 ISO VBUS、ISO_GND、ESD、限流及屏蔽接地策略。第二隔离侧下游目前无产品用途，依芯片正式配置处理。 |

P1–P3 应分别在高速及 USB2 回退模式下验证。三口共用 PC-C 上行，吞吐测试包含单口和多口并发。Microchip 硬件核对表提供端口电源控制、翻转参考及 P6 专用电路图。P4 的外部接口验证包含 USB2、SuperSpeed（若启用）、上下电、过流，及 PC/Dock 反灌。键盘的旧 `100mA TARGET` 只作起点，实测背光启动和高亮稳态电流后再确定限流与总预算。[Microchip 硬件核对表](https://www.microchip.com/content/dam/mchp/documents/UNG/ProductDocuments/DesignChecklist/USB7206C-Hardware-Design-Checklist-DS00003336.pdf)

## 双 CH318T 签核

沁恒的产品资料确认 CH318 主侧提供一个本地 USB2 下游，下位机侧提供两个下游，官方手册说明支持电容与网络变压器耦合。这支持继续验证上述端口分配。仍需从正式手册与参考电路逐针确认 CH318T 封装与上下位配置、复位、时钟、上下游端口、耦合件和每端供电。确认 USB 拓扑深度以及主侧本地端口与 ISO-A 同时枚举。功能样机覆盖 ESP32 FS、ISO HS/FS/LS、同时插拔、断电重启及不同电脑上的枚举，独立验证隔离 DC/DC、跨屏障元件、屏蔽、外壳、安装件、爬电/电气净距及供电温升。[沁恒 CH318 产品目录](https://www.wch.cn/products/productsCenter/mcuInterface?categoryId=1&tName=USB%E5%BB%B6%E9%95%BF) · [沁恒 CH318 技术手册](https://www.wch.cn/downloads/CH318DS1_PDF.html)

六个主 Hub 下游位置均已有分配。若 CH318T-A 本地口无法可靠服务 ESP32，且 Dock 数据、ESP32、ISO 必须同时在线，应在 P6 评估独立 USB2 分支 Hub 加已验证的隔离路径。完整验证前，当前 U102 原型隔离路径继续保留。ISO 的独立供电、键盘 VBUS、PC/Dock 防反灌和主电源总电流均为独立签核项。
