# PC-Hub 工程协作

PC-Hub 当前产品基准为 [RevC](docs/revc/README.md)。工程改动从[当前本地 KiCad 项目](hardware/pcb/sources/revc/Pc-Hub_RevB.kicad_pro)继续。每次开工核对编辑器绑定和保存板 SHA，结合[当前检查证据](docs/revc/evidence/README.md)、[约束](docs/revc/CONSTRAINTS.md)和[分工](docs/revc/PLAN.md)判断实际状态。工程文件名中的 RevB 是保留的 KiCad 工程标识。

- 原 PCB 工作线持续拥有原理图和 PCB。原外壳及主要设计线持续拥有外壳、机构、附件及装配。系统总控管理接口、范围和放行。固件与主机保留各自的上游责任。
- 设计和制造判断使用各自的证据。按产品坐标登记，明确保存源文件实测、冻结需求、候选与未知。以 `null` 留空未知参数。
- [差异审查](docs/revc/REVIEW.md)中的 116 封装修复板是继承来源。由当前本地保存板继续实施，发现更新时先核对其差异。旧版本资料用于解释沿革和继承已核实的设计。Konnect 预检拒绝时，记录工具或工程层面的具体原因，保留当前设计，继续可独立实施的工作。
- PCB 写入经 Konnect。保存后原生读回并更新[矩阵](docs/revc/matrix_numeric.json)、[字典](docs/revc/component_dictionary.json)和[变更](docs/revc/CHANGELOG.md)。用保存板证据分别记录器件放置、焊盘网络、铜线连通和验收结果。空网焊盘单列，避免误读 DRC 未连接数量。
- PCB 实改后重新输出单张四层真实铜线/焊盘/连接孔总览，并注明保存板 SHA、DRC/ERC、真实未连接和验证缺口。
- 先在本地实施、修正并验收。用户明确要求提交时才提交，明确要求同步时才更新 GitHub。七项制造门槛证据齐备前维持制造未放行。
