# PCB 设计

**当前状态：尚无完成布线、通过器件装配验证的 PCB 工程，也没有可下单 Gerber。**

R3.1已转为Core＋可选下置Dock，A3/POWER移出Core，唯一电脑上行在PC-C。原R3载板不升级冒充新原生主板；具体电气、料号、功率/叠装与布局先决条件见[WP1/WP2评审](../../docs/R3_1_ENGINEERING_REVIEW.md)。本轮未修改任何KiCad文件。
历史[R3接口载板工程](sources/body-r3/README.md)包含可编辑KiCad原理图、216×64×1.6 mm布局草稿、接口CSV和BOM草稿。由既有GPIO推导模块连接；未测量的开发板与FPC使用线束接口，不猜测封装。尚未布线、铺铜或进行KiCad ERC/DRC，不能作为R3.1原生主板。

| 目录 | 后续存放内容 |
| --- | --- |
| [sources](sources/README.md) | 可编辑原理图与 PCB 工程，建议每块板单独一个工程文件夹 |
| [exports](exports/README.md) | 经检查的 Gerber、钻孔、BOM、贴片坐标及原理图 PDF |
| [references](references/keyboard-port-survey.csv) | 接口尺寸调查等设计依据 |

未来主板、按键感应板、磁吸接口板按模块分工程，不能把候选参数当成制造文件。
当前结构约束见[外壳R3.1尺寸](../enclosure/dimensions-r3-1.md)，[R3](../enclosure/dimensions-r3.md)和[R2](../enclosure/dimensions-r2.md)为历史；功能要求见[产品规划](../../docs/product-and-integration.md)。
