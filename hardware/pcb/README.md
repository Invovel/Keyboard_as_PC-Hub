# PCB 设计

目前从 [RevC 审查与工程基线](../../docs/revc/REVIEW.md)进入。保存版为 116 封装修复工程，真实四层铜和检验结果见[本次证据](../../docs/revc/evidence/README.md)。

## 2026-11-18 RevB 进展

实际工程修改和验证记录见[RevB 当前状态](../../docs/pc-hub-revb-status-20261118.md)。PCB 已有正反面真实铜线、87 个过孔和四个地铜区，当前剩余 120 个未连接缺口、54 个未布通网络。背面晶振与隔离去耦、屏幕 QSPI 以及部分电源和磁吸保护支路已实施，整板尚未达到制造验收条件。

右侧保留磁吸，左侧新增双向回中拨轮位置。当前 GitHub 同步范围为文字记录，最新本地工程、DRC 和总览媒体未随本次提交上传。下面按日期保留的旧阶段描述供追溯，后续硬件改动暂缓。

**当前状态：尚无完成布线、通过器件装配验证的 PCB 工程，也没有可下单 Gerber。**

2026-11-02[ISO／DEBUG首版范围](../../docs/iso-debug-r3-1.md)已收敛到正常调试所需的USB2隔离、隔离供电、集成输出保护、数据ESD和本地故障提示。[功能连接原理图](../../design/drawings/iso-debug-minimal-schematic-r3-1.svg)、[候选BOM](../electronics/iso-debug-candidate-bom.csv)及[后续PCB元件依据](references/iso-debug-pcb-component-plan-r3-1.md)供主工程原理图实施；现有KiCad网表仍未接入该支路。本轮不绘制PCB。

R3.1已转为Core＋可选下置Dock，A3/POWER移出Core，唯一电脑上行在PC-C。原R3载板不升级冒充新原生主板；具体电气、料号、功率/叠装与布局先决条件见[WP1/WP2评审](../../docs/R3_1_ENGINEERING_REVIEW.md)。

2026-10-22[马达布局修订](references/core-haptic-placement-r3-1.md)：锁定iPhone 7 Plus，两端顶面各30 mm留白，下方装配区各15×54×6 mm；[接线与来源](../../docs/haptic-iphone-7-plus.md)。10月19日已通过Konnect建立216×64板框、23个封装及旧机械参考；本次长槽和接口移动的[原生执行状态](references/haptic-7plus-native-update.md)单独记录。新板尚未接网/布线，DRC未通过；[原生历史导出](exports/core-r3-1-haptic-study/README.md)不是制造文件。

[R3.1具体布局建议及顶/底面分区图](references/core-placement-r3-1.md)：按当前架构整理可尝试的功能区坐标、上下安装层、USB板边退缩、中央避让和Dock投影冲突；是布局研究建议，不替代机械基准或真实封装布局。
历史[R3接口载板工程](sources/body-r3/README.md)包含可编辑KiCad原理图、216×64×1.6 mm布局草稿、接口CSV和BOM草稿。由既有GPIO推导模块连接；未测量的开发板与FPC使用线束接口，不猜测封装。尚未布线、铺铜或进行KiCad ERC/DRC，不能作为R3.1原生主板。

| 目录 | 后续存放内容 |
| --- | --- |
| [sources](sources/README.md) | 可编辑原理图与 PCB 工程，建议每块板单独一个工程文件夹 |
| [exports](exports/README.md) | 经检查的 Gerber、钻孔、BOM、贴片坐标及原理图 PDF |
| [references](references/keyboard-port-survey.csv) | 接口尺寸调查等设计依据 |

未来主板、按键感应板、磁吸接口板按模块分工程，不能把候选参数当成制造文件。
当前结构约束见[外壳R3.1尺寸](../enclosure/dimensions-r3-1.md)，[R3](../enclosure/dimensions-r3.md)和[R2](../enclosure/dimensions-r2.md)为历史；功能要求见[产品规划](../../docs/product-and-integration.md)。
