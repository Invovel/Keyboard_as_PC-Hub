# MAG-R 电气与装配审核

## 2026-11-26 实施结果与剩余修改框架

按用户 10 项 MAG-R 要求逐项验收，至少 5 项仍未完全闭合（供电实测、外部 ESD、触点实物与线束、三维干涉、附件通信），因此未闭合条目至少 50%，超过 30%。这是条目闭合比例，不是工时或总工程量百分比。

剩余框架：① 独立 READY/FAULT 主控采样资源及入口 ESD；② 正式 8 触点型号、配合面镜像和柔性线束；③ 与右马达、ISO 插合、固定柱和双磁铁的三维装配；④ 100kHz 附件枚举、五键/104mm 触控条和故障状态机联调；⑤ 实测限流、热插拔、掉电、ESD，再评估 400kHz。机械未知值继续为 null。

已实际写入：U10.5 从附件 INT 分离为 MAG_R_READY；R56 10k 上拉到受控 3V3；TP25 测试点及电源/READY 铜线已连接。R56 产品坐标 (+76.5,-0.5)，TP25 (+79.325,-1.125)，均 F.Cu。JG2 (+76.5,+6) 未移动，仍为 I²C；保留附件 INT 及电源故障现有路径。READY 当前为独立测试点，尚未分配 MCU 输入。

保存板 SHA：`942accd835ab9017354a0cfade39002de317d729a9ded73473988973c95fae7d`。相对 9a5b069… 基线新增 R56/TP25，删除封装 0；新增铜线 18 段、过孔 2 个，移除旧 READY 分支铜线 5 段、过孔 2 个，原有其余铜对象变更 0 个。159 封装、569 焊盘、253 过孔。DNP 修正 JH1/JH2/R101，JM1 为正式装配，13 条旧注释已修正。

保存文件原生读回及连通分析通过 109 条指定端点路径，包含 READY 与 MAG-R；L2 主地与 ISO_GND 独立，L3 11 个主地区域连至主地。短路、铜间距、装配边界重叠和悬空过孔均 0。

整工程仍有 DRC 325 项（12 钻孔、99 库封装差异、88 丝印重叠、126 丝印压焊盘），80 未连接、94 图板差异，ERC 2 错误/7 警告。相对历史 DRC 321 项净增 4 项：库差异 +1、丝印重叠 +2、丝印压焊盘 +1，尚待修复；不能称整板完成。状态 **MANUFACTURING_NOT_RELEASED**，MAG-R 实物通信未验证。

证据：[保存板审计](../evidence/Audit_Source.json)、[铜线增量](../evidence/Cleanup_MAG_Delta.json)、[端点连通](../evidence/Continuation_Connectivity.json)、[四层线路图](../evidence/PC_Hub_Pure_Circuit_Overview.svg)、[内层电源地](../evidence/PC_Hub_Inner_Planes.svg)、[未连接端点](../evidence/Unconnected_Items.csv)。

## 历史评审依据（2026-11-24，实施状态以上文为准）


2026-11-24，状态为 INCOMPLETE / MANUFACTURING_NOT_RELEASED。本报告采用保存导出 Connectivity.net、GPIO_CONTRACT.csv 和 Footprint_Mechanical.csv。几何证据板 SHA 为 9a5b069014011d6112070d0b913e055b93e656b90effbd8b87aed80c10b56416。执行中的最终源文件 SHA 与 ERC/DRC 由 PCB 工作线另行绑定，本报告不宣称已实施下列改线。

JG2 为 8 针 I²C 附件线束座，产品锚点 (+76.5,+6)。引脚表见 MAG_R_PINMAP.csv。第 1 针当前真实 NC，第 2/8 针 GND，第 3 针受控 3V3，第 4/5 针 SDA/SCL，第 6 针低有效插入检测，第 7 针附件开漏中断。附件识别通过 I²C 描述符实现，第 1 针用途保持待定。

## U10 与 READY

TCA4307DGKR 八脚与官方定义一致。1 EN、8 VCC 接 MAG_R_3V3_SW，4 GND，3 SCLIN/6 SDAIN 接 mux CH1 的 PRE 网络，2 SCLOUT/7 SDAOUT 经 R52/R51 33Ω 到外部 SCL/SDA。5 READY 在修改前接 MAG_R_INT_CARD，与 R42.2、R54.2、Q4.2 共网。READY 为开漏，启动或断开时低，接通后释放。与同域开漏附件 INT 共用没有推挽争用，但低电平来源无法区分。优先分离 READY 并独立 10k 上拉至受控电源，保留附件 INT 路径。独立 READY 采样须确定 MCU 资源或输入扩展。GPIO21/47/48 在现有契约中仍有固件兼容保留。官方引脚说明中的 rise-time accelerator control 不能单凭名称推断外部拉低一定改变加速器行为。[TI TCA4307 数据手册](https://www.ti.com/lit/ds/symlink/tca4307.pdf?ts=1794238191923)

U8 FAULT 同样并入 MCU MAG_INT_N。READY 分离后仍需处理“附件 INT / 电源故障”共享低电平。ACTIVE 状态先有界读取附件事件并清除 IRQ，读取失败、DET 异常或持续低超时再关闭供电。完整故障可辨识方案应同时独立 FAULT 监测。

## U8 限流与电源

实装定义 TPS2553DBVR-1 为高有效 EN、锁断型。脚 1 IN=3V3，2 GND，3 EN=MAG_R_POWER_EN，4 FAULT_N=MAG_INT_N，5 ILIM 经 R41 210kΩ 1% 到地，6 OUT=MAG_R_3V3_SW。官方 210k 条件下限流为 110/130/150mA 最小/典型/最大。用手册公式及电阻容差计算下界约 109.2mA、上界约 152.2mA。超时过流锁断后须切换 EN 或输入电源恢复。[TI TPS2553 数据手册](https://www.ti.com/lit/ds/symlink/tps2553.pdf)

100mA 是附件设计目标。U10、电源泄放 R46、上拉及附件共同消耗受控电流，需用实际负载预算扣除主板开销，并验证触控条启动充电和线束压降。上游 3V3 同时承担 MCU、霍尔等负载，不能从 U8 限流值推断整机预算已满足。R44 1M 下拉、GPIO14 低态、Q7/Q9 夹断及 Q3 的 DET 联锁应按插入、拔出、复位、高阻和掉电逐项测量。

## ESD 与外部线束

D6/D7/D8/D9/D10 分别保护 SDA、SCL、DET、INT、受控电源，阴极到信号，阳极到 GND，位于主板背面 JG2 附近。PESD5V0L1UL 的 5V 工作反向电压允许 3.3V 常态信号，典型电容 25pF，标称钳位约 12V。该钳位值仍要求核对被保护引脚的瞬态承受能力及串阻，不能作为 3.3V 管脚绝对安全结论。[Nexperia 数据手册](https://assets.nexperia.com/documents/data-sheet/PESD5V0L1UL.pdf)

外壳触点至 JG2 存在线束，当前主板 ESD 器件无法证明整段暴露线束的浪涌路径。外部触点小板应在入口布置保护及短地回路，结合接触顺序、磁铁金属件、机壳回流路径做实物试验。线束长度、弯折、回路电感和附件附加上拉仍未知。100kHz 先验证上升时间及低电平，400kHz 需重新测量总电容和边沿。

## 可执行修改清单

| 文件/对象 | 修改 | 依赖与验收 |
|---|---|---|
| MAG_Protection.kicad_sch / U10.5 | READY 与 MAG_R_INT_CARD 分网，增加独立上拉及测点 | Konnect 预检，导出真实网表，保留附件 INT 及 Q4/Q6 路径 |
| 主板 PCB / U10.5 | 仅重构 READY 末端铜线及对应新器件 | 保存读回，端点连通、短路与间距检查，保持 JG2 坐标 |
| 主控接口 | 分配独立 READY 输入，并评估独立 FAULT 输入 | 固件 GPIO 保留资源协调完成后实施，避免占用旧扬声器试验资源造成隐式冲突 |
| 外部触点小板与线束 | 8 路同名映射，入口 ESD、GND 回流、应力释放 | 精确触点订货图、接触顺序、磁铁与外壳尺寸，当前 BLOCKED_MECHANICAL |

## 机械检查与附件协议

机械参数见 MAG_R_MECHANICAL_INTERFACE.json。右侧面 X=+114 为设计要求。触点组 Y=0、2.5mm 间距为已有候选，Z、板厚、磁铁中心与尺寸未知。右马达实际包络、J101 USB 插合扫掠、右后固定柱及螺钉均需机械工作线提供三维装配模型。当前只具备板上锚点，三维干涉检查未完成。

附件通过 I²C 上传五键位图、104mm 触控条归一化坐标、设备 ID、协议版本、状态与事件序号。实际 I²C 地址和触控控制器型号待定。完整状态机见 MAG_R_STATE_MACHINE.md。真实附件通信、热插拔、ESD、短路恢复和400kHz验证均待完成。
