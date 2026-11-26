# RevC PCB 接口契约 · 第一批

[GPIO 全表](GPIO_CONTRACT.csv)逐项对照当前 RevC 保存原理图网表、ESP32 模组实体焊盘和 `firmware/deck-ui/main` 中的 `body_input.c`、`body_encoder.c`、`body_screen.c`、`body_mic.h` 和 `body_audio.c`。CSV 第三列是当前保存网名，省略 `/Main_Control/` 层级前缀；第四列保留已审查固件的旧用法，不能当作 RevC 已实现的软件映射。当前引脚证据见[保存网表](evidence/Continuation_Pins.tsv)。

## 输入和灯

- ADC1 的 GPIO1/2 是编码器，3 为绑带，4/5 是麦克风，6/7 是 QSPI，8/9 是 I²C。仅 GPIO10 未被占用，不能满足四模拟输入。选择 ADS1015 接 GPIO8/9，ADDR 接 GND 时地址 0x48。四路按 K1–K4 对应 AIN0–AIN3，按传感器和磁回路实测重新标定。触摸 0x15、TCA9543A 0x70、右 MAG 开关 0x71 留原位。
- GPIO10 模组焊盘 18 分配为 RGB 数据。首颗 DIN 需确定 5 V 逻辑门限与上电状态。每像素近端去耦；数据和使能为独立概念，单纯拉低数据无法清除已锁存颜色。总禁用、失联、启动和固件挂起的行为需结合电源关断验证。
- 2026-11-23：GPIO10 到 U22 和四灯级联铜线已连通；GPIO42（U1.35）已正式改为 RGB_EN，接 U21.ON 与 U22.OE。旧 R5 上拉已从原理图和 PCB 删除，R68 4.7k 下拉保留。四路霍尔使用 ADS1015，相关信号及 L1 输出至传感器/ADC/主控的供电路径已逐端检查。SWJ1 三路及上拉已全部接通。固件迁移、真实采样与关断时序仍待验证。
- SWJ1 实体端子 1（CCW）、T（按压）、2（CW）分别分配 GPIO38、40、41，公共端 C 接主地，R8、R7、R6 复用为 10 kΩ 上拉。当前工具保留三个 PCB 网络名 `KEY4_GPIO38`、`KEY3_GPIO40`、`KEY2_GPIO41` 以沿用现有主控端铜线，物理用途由 SWJ1 引脚定义。JK1–JK4 已退出原理图和保存板。旧固件仍把三个 GPIO 识别为旧按键，RevC 使用前须切换软件映射并验证霍尔采集。GPIO21/47/48 仍由现有固件的 I²S0 发声初始化占用。
- EC56 A/B 继承 GPIO1/2，GPIO39 继承独立屏按压。JE1 的旧四针网用于原型联调。EC56 订货图需写明相序、是否带开关、焊孔基准、配合高度和公差；候选 XY 原点与最终钻孔保持分别记录。

参考 [Espressif WROOM-1/1U v1.8](https://www.espressif.com/sites/default/files/documentation/esp32-s3-wroom-1_wroom-1u_datasheet_en.pdf)、[TI ADS1015](https://www.ti.com/product/ADS1015)、[ALPS SLLB510100](https://tech.alpsalpine.com/e/products/detail/SLLB510100/)。

## 保存板已存在的实物接口

| 接口 | 焊盘与网络 | 后续闭合依据 |
|---|---|---|
| U4 / U5 双马达 | 各驱动 7/9 对接 JML1/JMR1 1/2；分别 MOTOR_L/R_P/N，不能接到地 | OUT± 铜连通及导通测试；接头和马达绕组电阻、脉冲电流、反电动势、温升 |
| JM1 | 1 3V3，2 GND，3 BCLK4，4 WS5，5 SD17，6 GND | 保留六针小板接口，落实底部声孔、供电去耦、通道选择和声学安装；根据实际线束确认键向和 DNP 去除 |
| JB1 / 浮动 USB-C | 1 KBD_5V_PROTECTED，2 HUB_P5_DM，3 HUB_P5_DP，4 GND | JB1 是四针主板接口，CC 在公头载板或扩展线束闭合。确认单/双 CC、插接方向、Rp/源端控制、受控 VBUS、放电、ESD、反灌及线对回流。100 mA 是旧标签目标 |
| J101 | 1 ISO VBUS，2 ISO D−，3 ISO D+，4 ISO GND，SH ISO_SHIELD | 分支 USB2512B、U102 ADuM3165、隔离电源与输出保护端点闭合；屏蔽连接单独签核，保持两域隔离 |
| JG2 右 MAG | 1 未接，2 GND，3 开关 3.3 V，4 SDA，5 SCL，6 检测，7 中断，8 GND | 保持 U8 电源开关、U6 总线分段、端子失效安全及隔离边界 |
| JL1 / JT1 | JL1 1 CS、2 CLK、3–6 D0–D3；JT1 1 SDA、2 SCL、3 3V3、4 GND | 保持屏接口。总装测量 FPC 弯折、长度、壳内净空和信号回路 |
| Dock | DATA / PWR / CTRL 研究围栏 | 保留，等待真连接器、公差链及上行/供电的独立启动验证 |

## 成本和电源验收

四层仍采用保存板的 F.Cu / In1.Cu / In2.Cu / B.Cu，现有 1.6 mm 是设计输入。嘉立创目录给出的 1.6 mm 候选包括 JLC04161H-7628 与 JLC04161H-3313，其表层到邻近内层介质分别为 0.2104 mm 和 0.0994 mm。两者不能共用凭经验指定的阻抗线宽。尚未选择生产叠层，需用实际订单铜厚、介质、参考平面、阻焊和计算器求线宽/线距，并审核跨 ISO 分割的回流。当前 U1 的 12 个 0.2 mm 钻孔仍违反项目 0.3 mm 下限，规则保持原值。[嘉立创阻抗与叠层目录](https://jlcpcb.com/impedance)

首批采购打样记录为 [器件采购与封装状态](PARTS_BATCH1.csv)。主板目标 170–180 元，配件约 50 元，整机上限 250 元。2026-11-19 查到 ADS1015 单料美元目录阶梯，整套装配费、线束、样件数量和实际报价待核。真实总价与差额留空。[功耗预算表](POWER_BUDGET.json)记录各支路、测量工况及未知输入。[嘉立创 ADS1015IDGSR 目录](https://jlcpcb.com/partdetail/ADS1015IDGSR/C193969)
