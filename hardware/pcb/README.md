# PCB 设计

**当前状态：尚无完成布线、通过器件装配验证的 PCB 工程，也没有可下单 Gerber。**
当前 [R3 接口载板工程](sources/body-r3/README.md)包含可编辑 KiCad 原理图、216×64×1.6 mm 布局草稿、接口 CSV 和 BOM 草稿。由现有 GPIO 推导模块连接；未测量的开发板与 FPC 使用线束接口，不猜测封装。尚未布线、铺铜或进行 KiCad ERC/DRC。

| 目录 | 后续存放内容 |
| --- | --- |
| [sources](sources/README.md) | 可编辑原理图与 PCB 工程，建议每块板单独一个工程文件夹 |
| [exports](exports/README.md) | 经检查的 Gerber、钻孔、BOM、贴片坐标及原理图 PDF |
| [references](references/keyboard-port-survey.csv) | 接口尺寸调查等设计依据 |

未来主板、按键感应板、磁吸接口板按模块分工程，不能把候选参数当成制造文件。
结构约束见[外壳 R3 尺寸](../enclosure/dimensions-r3.md)，[R2](../enclosure/dimensions-r2.md) 为历史；功能要求见[产品规划](../../docs/product-and-integration.md)。
