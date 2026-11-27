# Core Control Rev A：控制子电路边界与装配接口

本文件定义**同一块 Pc-Hub 主体主板上的控制电路分块**，没有增加第二块实际独立控制板。`Core-Control-RevA/` 只是电气验证暂存目录；验证通过后以层次页整合回用户指定 `Pc-Hub` 主工程。它不代替尚未完成的 Hub / Power Dock 电路，也不把旧 CN7/CN10 原理图当成新控制板。全部原理图变更通过 Konnect MCP。

## 可直接集成 PCB 的部分

| 电路 | 本版选择 | 集成条件 |
|---|---|---|
| MCU | ESP32-S3-WROOM-1U-N16R8 | 41 个逻辑焊盘；外接天线需另固定，N16R8 的 GPIO35/36/37 不对外使用 |
| 控制侧 3V3 | AP63203WU-7，同步降压 | 受控、限流后的 5V_CORE 输入；3.9µH、10µF 输入、2×22µF 输出、100nF bootstrap 为参考起点；布局及具体电感仍需确认 |
| MCU 支持 | 10k EN 上拉、1µF EN 复位、10k BOOT 上拉、10µF + 100nF 去耦 | 保留手动 RESET / BOOT 和 3V3 UART 下载口 |
| 输入电路 | 四键、编码器 A/B/SW 各 10k 上拉 | 机械开关放在面板/托架，通过线束短接到 GND；固件去抖保留 |
| 双触觉驱动 | 两颗 DRV2605LDGSR | 每颗 VDD 1µF + 100nF，REG **1µF**；IN/TRIG 接地，EN 硬件上拉；驱动参数未验证前不可宣称与拆机马达已匹配 |
| 同地址隔离 | TCA9543APWR | 0x70；通道 0 左、通道 1 右，DRV 均为 0x5A；固件只能单选通道，不写 0x03 同时导通 |
| 总线支持 | 上游/各下游 I²C 上拉，mux 去耦和复位 | 屏模块自带上拉时，上游 R3/R4 不重复安装；TCA RESET 接 MCU_EN |
| USB FS 支路 | GPIO19/20，各串 22Ω | 内部 USB2 支路，仅接后续 Hub 下游 P6；不是 USB3 / Type-C 母座设计 |

AP63203 的 2A 是转换器额定能力，**不是 USB 上游允许取用的电流**。整机原材料基线中的 TPS62133RGTR 属于后续 Hub 共用电源方案；本控制侧 AP63203 是独立备选，二者输出不可并联。Hub 电源冻结时必须决定使用哪一套控制侧 3V3。

## 需要基座、孔位、线束的部分

| 实体 | 固定方法 | 电气连接 |
|---|---|---|
| 圆屏 + 触控板 | 屏幕活动托架、导向与独立行程限位；固定件避开玻璃和排线 | JL1 六根 QSPI 信号；JT1 四根触控/3V3/GND。新主板针序必须按实际模块丝印制作线束，不等同原 LCD FPC 针序 |
| EC11 | 面板/支架承受轴向和扭力，线束应力释放 | JE1：GND、A=GPIO1、B=GPIO2、SW=GPIO39 |
| 四个按钮 | 对称感应中心定位，按钮/软键结构独立于主板 | JK1..4：GND、GPIO42/41/40/38 |
| iPhone 7 Plus 马达 | 左右壳体预留长槽/可更换托架；马达与外壳机械耦合，不能靠 PCB 焊盘承重 | 每个马达专用两针 OUT+/OUT−；六个拆机测试焊盘仅接最外两点，中间四点绝缘；不是原 Apple FPC 接头定义 |
| 外置天线 | 壳内专用位置与同轴线应力释放；避开马达金属本体、USB 插座和屏蔽结构 | 接 WROOM-1U 自带射频连接器；不能把天线当普通 GPIO 线束 |
| 现有 MAX98357A + 喇叭 | 保留模块固定柱、扬声器线束；仅作现有声音调试备选 | JA1 保留现有 I²S 接线；MAX 差分输出与触觉驱动输出分开 |
| 可选 INMP441 | 可拆模块和声孔/密封垫；避免直接耦合马达振动 | JM1，默认不装；麦克风代码已选但此新装配没有实测 |

马达位置、净空、托架与接线来源以项目当前的 7 Plus 机械/触觉资料为准。电气原理图不推定用户提供的尺寸已通过实物量测。

## 新 PCB 线束针序

这里的针号均指**新 PCB 接插件 pad 编号**。JST GH 安装面图的 pin 1 在排针 X 负端；反面、线端和配对视图会镜像，必须按座上 pin 1 标记逐根对照，不能按图片左右猜。

| 参考号 | 新 PCB pin 1 → 末针 | 备注 |
|---|---|---|
| JL1，6p | CS16、SCLK6、D0=15、D1=7、D2=11、D3=12 | 仅六根显示信号 |
| JT1，4p | SDA8、SCL9、3V3、GND | 显示/触控共用电源及回流 |
| JK1..4，2p | GND、GPIO42/41/40/38 | 左上①、右上②、右下③、左下④ |
| JE1，4p | GND、A1、B2、SW39 | 沿实际运行源码保留；历史实物线标存在歧义，装配必须逐线验证 |
| JP1，2p | GND、GPIO0 | BOOT / 固件禁用开关，不是断电开关 |
| JR1，2p | GND、MCU_EN | 手动复位 |
| JD1，6p | 3V3参考、GND、TX43、RX44、EN、BOOT0 | 外部串口必须使用 3V3 逻辑，不能将 5V UART 接 GPIO |
| JU1，4p | NC、D−、D+、GND | pin1 不回接 VBUS；内部 USB2 支路，无外部连接器 ESD 保护，测试转接板须提供保护 |
| JA1，6p | 5V_CORE、GND、BCLK47、WS48、DIN21、NC | 外接已测试功放模块；不是马达连接器 |
| JM1，6p | 3V3、GND、BCLK4、WS5、SD17、GND | pin6 将麦克风 L/R 选为左通道；默认不装 |
| JH1/JH2，6p | HAPTIC_VDD、GND、SDA_L/R、SCL_L/R、HAPTIC_EN、NC | 调试接口，默认不装；主板已集成驱动时，不接第二块驱动产生地址冲突 |
| JML1/JMR1，2p | MOTOR_L/R_P、MOTOR_L/R_N | 仅马达差分输出，两根均不能接 GND |

没有为独立屏幕按压分配 GPIO。GPIO39 仅是当前 EC11 按压，不能同时当成独立输入。

## 仍阻断整机投板的具体事项

1. USB7206C、Hub 电源/时钟/配置、PC 与 Dock 电源切换尚未完成原理图；USB3 物理母座与 Dock 高速连接器未冻结。它们不是控制原理图上的“已经接通的网络”。
2. JPOWER1 仅为受控 5V 入口的电气边界，物理电源座和线材电流预算未冻结。L1 的具体可采购 MPN、Isat、DCR 与尺寸尚未验收。
3. 7 Plus 拆机马达的额定电压、允许过驱、线圈阻抗及参数离散未测定。本版 HAPTIC_VDD 起点为 3V3，不把 USB 5V 直接接 DRV，也不填造 RATED_VOLTAGE / OD_CLAMP 寄存器值。
4. 当前电气验证和新板的实际封装放置/布线/阻抗/机械装配是不同验收；未完成新板 DRC 与装配核对前不能下单投产。

## 官方依据

- [Espressif WROOM-1 / WROOM-1U datasheet v1.8](https://documentation.espressif.com/esp32-s3-wroom-1_wroom-1u_datasheet_en.pdf)：共同 41 脚电气定义、1U 机械封装、N16R8 资源约束。
- [Diodes AP63203 DS41326 Rev.3-2](https://www.diodes.com/datasheet/download/AP63200-AP63201-AP63203-AP63205.pdf)：TSOT26 top view，固定 3V3 拓扑，Table 2 外围参数。
- [TI DRV2605L SLOS854D](https://www.ti.com/lit/ds/symlink/drv2605l.pdf)：DGS10 top view、REG/VDD 1µF、IN/TRIG 未用接地、VDD/NC 第6针可接 VDD、差分输出。
- [TI TCA9543A SCPS206B](https://www.ti.com/lit/ds/symlink/tca9543a.pdf)：PW14 top view、独立下游上拉、地址与复位。
- [JST GH 官方图册](https://www.jst-mfg.com/product/pdf/eng/eGH.pdf)：mounting-surface view、pin1 和电气/机械焊盘；线束和座子视图不得混淆。

原理图及网表实际完成程度请查看同目录的构建验收记录，本文不能单独作为 ERC / DRC 通过证明。
