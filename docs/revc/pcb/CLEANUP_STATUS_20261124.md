# RevC 正式收口 · 2026-11-24 阶段记录

## 2026-11-26 实施结果与剩余修改框架

按用户 10 项 MAG-R 要求逐项验收，至少 5 项仍未完全闭合（供电实测、外部 ESD、触点实物与线束、三维干涉、附件通信），因此未闭合条目至少 50%，超过 30%。这是条目闭合比例，不是工时或总工程量百分比。

剩余框架：① 独立 READY/FAULT 主控采样资源及入口 ESD；② 正式 8 触点型号、配合面镜像和柔性线束；③ 与右马达、ISO 插合、固定柱和双磁铁的三维装配；④ 100kHz 附件枚举、五键/104mm 触控条和故障状态机联调；⑤ 实测限流、热插拔、掉电、ESD，再评估 400kHz。机械未知值继续为 null。

已实际写入：U10.5 从附件 INT 分离为 MAG_R_READY；R56 10k 上拉到受控 3V3；TP25 测试点及电源/READY 铜线已连接。R56 产品坐标 (+76.5,-0.5)，TP25 (+79.325,-1.125)，均 F.Cu。JG2 (+76.5,+6) 未移动，仍为 I²C；保留附件 INT 及电源故障现有路径。READY 当前为独立测试点，尚未分配 MCU 输入。

保存板 SHA：`942accd835ab9017354a0cfade39002de317d729a9ded73473988973c95fae7d`。相对 9a5b069… 基线新增 R56/TP25，删除封装 0；新增铜线 18 段、过孔 2 个，移除旧 READY 分支铜线 5 段、过孔 2 个，原有其余铜对象变更 0 个。159 封装、569 焊盘、253 过孔。DNP 修正 JH1/JH2/R101，JM1 为正式装配，13 条旧注释已修正。

保存文件原生读回及连通分析通过 109 条指定端点路径，包含 READY 与 MAG-R；L2 主地与 ISO_GND 独立，L3 11 个主地区域连至主地。短路、铜间距、装配边界重叠和悬空过孔均 0。

整工程仍有 DRC 325 项（12 钻孔、99 库封装差异、88 丝印重叠、126 丝印压焊盘），80 未连接、94 图板差异，ERC 2 错误/7 警告。相对历史 DRC 321 项净增 4 项：库差异 +1、丝印重叠 +2、丝印压焊盘 +1，尚待修复；不能称整板完成。状态 **MANUFACTURING_NOT_RELEASED**，MAG-R 实物通信未验证。

证据：[保存板审计](../evidence/Audit_Source.json)、[铜线增量](../evidence/Cleanup_MAG_Delta.json)、[端点连通](../evidence/Continuation_Connectivity.json)、[四层线路图](../evidence/PC_Hub_Pure_Circuit_Overview.svg)、[内层电源地](../evidence/PC_Hub_Inner_Planes.svg)、[未连接端点](../evidence/Unconnected_Items.csv)。


**2026-11-26 复核更新。JH1/JH2/R101 真实 DNP、JM1 正式装配描述及 13 处注释已经通过 Konnect 写入。MAG-R 原理图 READY 分离完成，PCB 同步仍在实施。制造状态为 MANUFACTURING_NOT_RELEASED。**

清理后的 PCB SHA 为 `178c878e83ca023ef502227162fd40ebc81542dd659845d58e9926f78bac59e8`。最新证据见 [Cleanup_MAG_Delta.json](../evidence/Cleanup_MAG_Delta.json)。L2 的 1 个主地区域及 L3 的 11 个主地区域均已通过保存铜形状验证连接 U1 地。以下旧基准及待写入记录描述本批启动时状态，以本段及最新证据为准。

保存板 SHA-256 为 `9a5b069014011d6112070d0b913e055b93e656b90effbd8b87aed80c10b56416`。2026-11-24 重新读取 PCB 与四张原理图，全部 SHA 匹配现有检查证据。PCB 原生读回 157 个封装、566 个焊盘。该版本有 253 个过孔。见[源文件取证](../evidence/Audit_Source.json)和[四层线路图](../evidence/PC_Hub_Pure_Circuit_Overview.svg)。

当前完整检查为 DRC 321 项、未连接 80 项、图板差异 94 项、ERC 2 错误和 7 警告。MAG-R 新增原理图器件及网络尚未完成板级同步，图板差异较清理前的 87 项增加 7 项。

## 已保存到 PCB 的成果

- 四颗 OH49E 经滤波接入 ADS1015，ADC 的 I²C 连接到 MCU。四颗 RGB 级联、GPIO10 数据、GPIO42 使能及默认下拉已落板。
- SWJ1 三路已接 GPIO38/40/41。右 MAG 的数据、检测、中断和本地保护连线已落实。两组马达差分输出已布通。
- 顶底层走信号及局部电源引线。L2 为连续主 GND，ISO_GND 独立。L3 为 3V3、5V_CORE、剩余 GND 和独立 ISO_GND，11 个主地填铜几何区域仍需逐块复核。
- 保存板已验证 107 条指定端子路径。完整变更量、布线证据与限制见[保存板实施记录](../CONTINUATION_20261123.md)。

## Batch 1 正式清理

已登记 157 个保存板封装和 7 个历史退役位号。R5、JK1–JK4、JG1、JA1 在 PCB、完整原理图及保存 BOM 中均未出现。见[删除与迁移台账](REMOVAL_AUDIT.csv)。

| 对象 | 真实保存状态 | 待实施或保留条件 |
|---|---|---|
| JH1、JH2、R101 | 图板实际 DNP 均为是，已读回 | 调试与隔离验证策略已落实。R101 装配须经隔离验证 |
| JM1 | 图板 DNP 均为否，Value 为 INMP441 MICROPHONE INTERFACE，Assembly 为 REVC_POPULATE | 正式麦克风装配描述已落实 |
| JPOWER1 | 图板 DNP 均为否，BOM 的 DNP 列为空 | 保留原型供电。PC-C／可选 Dock 正式电源闭合后再定正式装配策略 |
| JE1、JU1、JB1 | 均保留 | JE1 等 EC56 正式替代，JU1 等板内 USB 路径验收，JB1 继续承担键盘下游接口 |

13 处过期注释已通过 Konnect 定点更新。预检、写入及保存读回已完成。PCB 从 9a5b0690 更新为 178c878e。原理图另已新增 MAG-R 的 R56、TP25 和 READY 独立网络，板级同步待完成。

## 检查与未完成项

当前保存报告为 DRC 违规 321 项、真实未连接 80 项、图板一致性差异 94 项、ERC 2 错误和 7 警告。321 项包含钻孔 12、库封装差异 98、丝印重叠 86、丝印压开窗 125。见[检查与逐端缺口](../evidence/README.md)。

下一步完成 MAG-R 独立 READY 的板级同步、局部布线并复跑检查。主 Hub、USB2 分支、ISO 隔离供电、键盘受控 VBUS 和 Dock 供电仍待闭合。EC56 与浮动 Type-C 需要正式尺寸和实物配合证据。机械寿命、真实功耗、嘉立创报价、ERC、DRC、USB 通信和三维干涉仍是制造放行门槛。
