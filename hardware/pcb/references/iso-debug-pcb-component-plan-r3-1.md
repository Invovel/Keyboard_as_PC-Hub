# R3.1 ISO／DEBUG：后续 PCB 元件与分区依据

本页把[隔离调试口电气决定](../../../docs/iso-debug-r3-1.md)转换为 PCB 装配清单和接口边界。首版功能连接见[原理图视图](../../../design/drawings/iso-debug-minimal-schematic-r3-1.svg)。用户已明确要求直接修改现有 `Pc-Hub` 主工程：该工程当前以 `Main_Control.kicad_sch` 为控制层次页；隔离电路应增加为同一工程的功能页和主板隔离分区，而非另建独立交付工程。它是原理图到布局的交接，不是已放置或布线的 PCB，也不授权采购。具体位号、封装及网名须以主工程原理图和其导出 BOM／网表为准；[候选材料 CSV](../../electronics/iso-debug-candidate-bom.csv)保存型号研究状态。

## 板间接口

| 逻辑接口 | 必要信号 | 连接原则 |
| --- | --- | --- |
| Core P6 输入 | D+、D−、Core GND、经过预算核准的 Core 供电 | 只进入二口 USB2 Hub 的上游；不可并接原 CTRL 与 ISO |
| 原 CTRL 输出 | Hub 下游 1 D+、D−、Core GND；VBUS 感测按主控最终电路确定 | ESP32 控制通路仍在 Core 域，不跨 ISO 绝缘边界 |
| 状态与控制 | 首版为 ISO 域本地故障 LED；Core SDA/SCL、精确计量和跨域故障返回留作后续选装 | FLT/PG 只连 ISO 域，不能直接跨绝缘边界接 Core GPIO |
| ISO 外部口 | 受保护 VBUS、D+、D−、ISO GND、屏蔽壳 | 只接最右侧 USB-A；屏蔽壳与固定金属件的接地方案待 EMC 和隔离测试 |

原理图核对时至少应出现下列**相互独立的网组**：`P6_DP/DM → USB2 Hub 上游`、`Hub DN1_DP/DM → CTRL`、`Hub DN2_DP/DM → 数据隔离器 Core 侧`、`数据隔离器 ISO 侧 DP/DM → USB-A`；`CORE_5V → 输入软启动 → 隔离 DC/DC → ISO_5V_RAW → 集成输出保护 → ISO_USB_VBUS`；`ISO_FLT → ISO 域本地故障 LED`。这些是功能网名建议，具体位号与管脚以保存的原理图为准。`CORE_GND` 与 `ISO_GND` 在网表中必须是两条不同网络。INA238、ISO1640、ADuM1201和分流电阻为后续选装，不计入首版必装器件。

电路默认集成在现有 Core 主板工程中，后侧可做局部隔离区域；只有实体装配研究证明主板无法容纳时，才重新讨论子板。USB2 信号和隔离边界不能在机械拼缝中用随意线束延长。新增 Hub 需要独立 3.3V 预算，隔离 DC/DC 需要受控 5V 输入和启动浪涌限制。

目前选型研究给出的最小实体参考：USB2512B 为 6×6 mm QFN36；ADuM3165 为 SSOP20；VRB0505S-5WR3 为 22×9.5×12 mm SIP。这些是器件本体资料，不包括 USB 插座后伸、绝缘槽、散热、线缆和固定结构；不能由它们直接反推子板外形。依据：[Microchip USB2512B](https://www.microchip.com/en-us/product/USB2512B)、[ADI ADuM3165](https://www.analog.com/en/products/ADUM3165.html)、[MORNSUN VRB0505S-5WR3](https://www.mornsun.cn/index/index/downpdf/pdf/VRB0505S-5WR3.pdf/name/VRB0505S-5WR3.html)。

## 布局分区和对应元件

| 分区 | PCB 元件类别 | 布置约束 |
| --- | --- | --- |
| Core 域 | P6/CTRL 连接、USB2512B 双口 Hub、其配置/时钟/去耦、Core 侧数据和状态隔离器、必要的 3.3V 稳压 | 保留 P6 的 USB2 差分回流；Core GND 只在隔离边界左侧铺铜 |
| 绝缘跨越区 | ADuM3165、隔离 DC/DC、隔离槽与禁布区 | 实际封装、爬电/电气间隙按最终器件核算；槽上不能通过过孔、铜、安装件或马达线 |
| ISO 域 | 输出 eFuse、本地故障 LED、电阻、USB2 ESD、USB-A 母座、ISO 测试点 | ISO GND 不直接回 Core；短路电流与散热回路按选定额定值设计 |
| 机械件 | USB-A 绝缘套、后壁止推/螺钉、隔板、线束压扣 | 插拔力不由隔离槽或 IC 焊脚承受；金属固定件不得桥接两地 |

**首要 3D 冲突**：候选 ISO 中心 X≈+91 与右侧 7 Plus 马达 X=85.5～100.5 重叠，右后孔 (+104,+28) 邻近插座；详情见[右后研究图](../../../design/drawings/iso-debug-rear-study-r3-1.svg)。未选定 USB-A 实体封装与子板高度前，不定 PCB 外形、孔位和插座焊盘。

## 进入 PCB 绘制前的冻结条件

1. 原理图中每个真实器件有明确 MPN／后缀、物理针脚映射和封装；无直跨 Core/ISO 两地的网。
2. 输出保护的锁断、阈值、失电默认状态及 ISO 域本地故障指示由电路实现，并有启动/短路台架记录；跨域故障返回属于后续选装。
3. Core 单线无 Dock 预算能够说明 ISO 口允许输出电流；不足时有硬件保持关闭的路径。
4. USB-A、PC-C、右侧马达和后孔的实体模型完成装配干涉检查；屏蔽及螺钉方案不破坏隔离。
5. 完成原理图 ERC 与导出网表核对后，再在主 PCB/子板中放置封装、做阻抗、回流和隔离槽；PCB DRC 与实物隔离测试仍是后续任务。
