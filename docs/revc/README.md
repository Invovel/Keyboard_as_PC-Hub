# PC-Hub RevC

2026-11-26 MAG-R 当前原板实施进度见 [MAG_R_PROGRESS](pcb/MAG_R_PROGRESS.md)：READY 分网和铜线完成；实物与整板验收未完成。该记录针对 hardware/pcb/sources/revc，不代表 RevC-Compact 工程已实施。

**2026-11-19 · 产品方向冻结 · 制造未放行**

**2026-11-25 最新产品方向**见 [RevC Compact 实施基准](../revc-compact/DESIGN_SPEC.md)，含新旧需求差异、当前保存板读回、紧凑布局与接口迁移。下方记录保留为现行 RevC 工程的沿革与证据。

2026-11-24 已复核保存板、原理图及装配状态，并登记正式清理进度。见[阶段成果与未完成项](pcb/CLEANUP_STATUS_20261124.md)、[删除与迁移台账](pcb/REMOVAL_AUDIT.csv)及[真实布线、供电分层与检查](CONTINUATION_20261123.md)。

1. [冻结设计交接](baseline/HANDOFF.md)和[交接展示](baseline/RevC_Interactive.html)。展示中的铜线来自旧 79 封装板。
2. [差异审查](REVIEW.md)、[执行计划与分工](PLAN.md)和[候选装配预检](ASSEMBLY.md)。
3. [坐标与工程约束](CONSTRAINTS.md)、[项目沿革](HISTORY.md)、[变更记录](CHANGELOG.md)。
4. [当前数字矩阵](matrix_numeric.json)、[字段](matrix_schema.json)、[器件字典](component_dictionary.json)。交接原件见 baseline。
5. [当前保存板四层叠加](evidence/PC_Hub_Pure_Circuit_Overview.svg)、[验证报告](evidence/README.md)。所有导出与保存板 SHA-256 绑定。
6. [第一批实际实施](BATCH1.md)、[GPIO 全表](GPIO_CONTRACT.csv)、[接口契约](INTERFACE_CONTRACT.md)、[机械接口](MECHANICAL_INTERFACE.md)、[采购候选](PARTS_BATCH1.csv)及[功耗预算](POWER_BUDGET.json)。

当前可编辑工程为 [RevC KiCad 项目](../../hardware/pcb/sources/revc/Pc-Hub_RevB.kicad_pro)。沿用工程内部文件名以保持 KiCad 层级与 UUID；产品版本为 RevC。每批同步保存源、矩阵、字典和证据，制造状态仍以七项门槛判定。
