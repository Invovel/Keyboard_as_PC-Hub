# 当前本地 RevC 保存工程 · 2026-11-23

## 2026-11-26 实施结果与剩余修改框架

按用户 10 项 MAG-R 要求逐项验收，至少 5 项仍未完全闭合（供电实测、外部 ESD、触点实物与线束、三维干涉、附件通信），因此未闭合条目至少 50%，超过 30%。这是条目闭合比例，不是工时或总工程量百分比。

剩余框架：① 独立 READY/FAULT 主控采样资源及入口 ESD；② 正式 8 触点型号、配合面镜像和柔性线束；③ 与右马达、ISO 插合、固定柱和双磁铁的三维装配；④ 100kHz 附件枚举、五键/104mm 触控条和故障状态机联调；⑤ 实测限流、热插拔、掉电、ESD，再评估 400kHz。机械未知值继续为 null。

已实际写入：U10.5 从附件 INT 分离为 MAG_R_READY；R56 10k 上拉到受控 3V3；TP25 测试点及电源/READY 铜线已连接。R56 产品坐标 (+76.5,-0.5)，TP25 (+79.325,-1.125)，均 F.Cu。JG2 (+76.5,+6) 未移动，仍为 I²C；保留附件 INT 及电源故障现有路径。READY 当前为独立测试点，尚未分配 MCU 输入。

保存板 SHA：`942accd835ab9017354a0cfade39002de317d729a9ded73473988973c95fae7d`。相对 9a5b069… 基线新增 R56/TP25，删除封装 0；新增铜线 18 段、过孔 2 个，移除旧 READY 分支铜线 5 段、过孔 2 个，原有其余铜对象变更 0 个。159 封装、569 焊盘、253 过孔。DNP 修正 JH1/JH2/R101，JM1 为正式装配，13 条旧注释已修正。

保存文件原生读回及连通分析通过 109 条指定端点路径，包含 READY 与 MAG-R；L2 主地与 ISO_GND 独立，L3 11 个主地区域连至主地。短路、铜间距、装配边界重叠和悬空过孔均 0。

整工程仍有 DRC 325 项（12 钻孔、99 库封装差异、88 丝印重叠、126 丝印压焊盘），80 未连接、94 图板差异，ERC 2 错误/7 警告。相对历史 DRC 321 项净增 4 项：库差异 +1、丝印重叠 +2、丝印压焊盘 +1，尚待修复；不能称整板完成。状态 **MANUFACTURING_NOT_RELEASED**，MAG-R 实物通信未验证。

证据：[保存板审计](./Audit_Source.json)、[铜线增量](./Cleanup_MAG_Delta.json)、[端点连通](./Continuation_Connectivity.json)、[四层线路图](./PC_Hub_Pure_Circuit_Overview.svg)、[内层电源地](./PC_Hub_Inner_Planes.svg)、[未连接端点](./Unconnected_Items.csv)。


2026-11-26 最新清理保存板 SHA 为 `178c878e83ca023ef502227162fd40ebc81542dd659845d58e9926f78bac59e8`。DNP 和注释修复已保存，MAG-R 原理图 READY 已分离，板级同步仍在实施。[当前改动统计](Cleanup_MAG_Delta.json)、[真实装配属性](Population_Inventory.json)、[产品坐标 CPL](CPL_Product_Coordinates.csv)及本目录 DRC/ERC 为当前导出。以下 11 月 23 日说明保留为继承基准。

2026-11-24 重新核对保存 PCB 和四张原理图的 SHA，均与本页取证记录一致。[正式清理阶段记录](../pcb/CLEANUP_STATUS_20261124.md)补充了实际 DNP 状态和退役位号复核结果。

[当前 PCB](../../../hardware/pcb/sources/revc/Pc-Hub_RevB.kicad_pcb) SHA-256：`9a5b069014011d6112070d0b913e055b93e656b90effbd8b87aed80c10b56416`。

霍尔/ADC、RGB 级联与 GPIO42 使能、SWJ1 三路、右 MAG 本地保护连接已完成本批铜线落实。L1 3V3 输出已接 MCU/ADC、四颗霍尔及电平转换器。移除旧 R5，JK1–JK4 维持退出。位置冻结与三维装配未完成。

当前 157 封装、566 焊盘、253 过孔；顶层 988 段、底层 595 段。L2 连续主地，L3 3V3/5V 电源分区，两层 ISO_GND 独立。内层无信号铜线。

整板未连接 80、图板一致性 87；无短路、铜间距和悬空线违规。仍有 12 钻孔、98 库封装、86 丝印重叠、125 丝印压铜问题；ERC 2 错误、7 警告。制造未放行。

[本轮完整说明](../CONTINUATION_20261123.md) · [改动与前后 SHA](Continuation_Delta.json) · [107 条端子路径](Continuation_Connectivity.json) · [引脚网表](Continuation_Pins.tsv) · [GPIO 契约](../GPIO_CONTRACT.csv)

[纯线路综合 SVG](PC_Hub_Pure_Circuit_Overview.svg) · [PNG](PC_Hub_Pure_Circuit_Overview.png) · [实际内层铜区](PC_Hub_Inner_Planes.svg) · [铜线与过孔](Copper_Items.json) · [焊盘及钻孔](Pad_Coordinate_Map.csv) · [机械锚点](Footprint_Mechanical.csv)

[DRC](DRC.json) · [ERC](ERC.json) · [真实未连接](Unconnected_Items.csv) · [当前 BOM](Saved_BOM.csv) · [取证哈希](Audit_Source.json)

`continuation/` 保存本轮各批次的几何预检计划、Konnect 写入及读回记录。旧 `RevC_Rebuild_Readback.json` 等阶段文件仅代表各自注明的历史 SHA。
