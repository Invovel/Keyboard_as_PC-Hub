# 版本和项目历史

| 阶段 | 可追溯材料 | RevC 处理 |
|---|---|---|
| R0 / R2 / R3 | [早期外壳参数](../../hardware/enclosure/README.md)、[旧 README](../history/README-before-2026-10-09.md) | 仅追溯机械与实验，以 RevC 冻结要求为准 |
| R3.1 Core＋Dock | [结构](../../hardware/enclosure/dimensions-r3-1.md)、[端口供电](../ports-and-power.md)、[决策](../R3_1_DECISIONS.md) | 继承 228×76 外形方向、Core 单上行、Dock 预留、右侧附件关系。分离键面、双侧 MAG、可调滑轨式公头和旧屏托退出 |
| 79 封装工程快照 | [交接数据](baseline/matrix_schema.json)和[旧板展示](baseline/RevC_Interactive.html) | 仅历史坐标/走线。旧 EC50、独立薄型/轻触四键、HTR3212SSQER 等均已替代 |
| RevB 修复与 UI | [2026-11-18 状态](../pc-hub-revb-status-20261118.md)、[五页固件](../../firmware/deck-ui/README.md) | 以现存经复审的 116 封装修复工程继承 PCB。保留五页、内容槽、手势和 HID；新增覆盖层 |
| RevC 2026-11-19 | [交接规范](baseline/HANDOFF.md)、[现场审查](REVIEW.md) | 成为 main 的现行入口，候选与未知值见矩阵与字典 |

旧 R3.1、RevB、固件试验资料按路径就地追溯。先前 ISO 口切换为 Core 输入仅为讨论，RevC 沿用隔离输出职责。
