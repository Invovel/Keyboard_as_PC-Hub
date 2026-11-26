# 当前开发入口 · RevC

按照 [AGENTS.md](AGENTS.md) 的分工，从 [RevC 项目入口](docs/revc/README.md) 打开[当前 KiCad 项目](hardware/pcb/sources/revc/Pc-Hub_RevB.kicad_pro)。开工时将[检查证据](docs/revc/evidence/README.md)与当前保存文件核对。下方 R3.1 内容保留用于追溯，当前工作以 RevC 的工程文件、要求和分工为准。

## R3.1 历史开发说明

先看[最新决策与差异](docs/R3_1_DECISIONS.md)、[右后ISO调试口新增决定](docs/iso-debug-r3-1.md)、[工程评审](docs/R3_1_ENGINEERING_REVIEW.md)和[README](README.md)。用户输入包原文保留在[交接资料](docs/R3_1_CODEX_HANDOFF.md)与[原启动说明](CODEX_START_R3_1.md)，其“待执行”清单不能替代本轮状态记录。

定位为Desktop Control Hub：Core唯一PC-C上行，A1/A2/FAST/KBD/CTRL及底部P4，Core目标单线独立使用；Power Dock可选、Rev B下置，单向供电且只有下游数据链路。原A3改为P4，POWER输入移至Dock；MAG保持低功耗。

2026-10-31新增最右侧ISO／DEBUG USB-A研究口：PC-C候选X+64、ISO候选X+91；P6经USB2双下游候选继续承载CTRL并增加隔离支路。隔离供电、保护、采样、右马达/安装孔干涉与真实插座均待验，不能依据草图制板。

本轮WP0文档/参数/展示迁移与WP1/WP2资料级评审已经完成；单线功率、Hub配置、中央传动、Dock配对连接器仍阻塞真实布局。下一步先测Core峰值电流并确认上游电力，再冻结Dock DATA/PWR/CTRL料号/高度，不凭候选坐标出制造文件。

保持原固件/GPIO、日志、镜像不变，Touchpad延迟不优化。本轮不烧录、不改KiCad、不出Gerber。之后KiCad读写按Konnect工作流；旧R3生成脚本属于历史，不运行覆盖新版。新的[机械生成器](hardware/enclosure/tools/update_r3_1.py)只写R3.1 JSON/SVG。

历史目录及配套材料见[目录规范](docs/repository-layout.md)、[机械说明](hardware/enclosure/dimensions-r3-1.md)、[原 HTML](design/visualization/hairline-keyboard-hub.html)与[检查证据](tests/design/r3-1-review.md)。
