# PC-Hub 工程协作

PC-Hub 当前产品基准为 [RevC](docs/revc/README.md)。先看[差异审查](docs/revc/REVIEW.md)、[约束](docs/revc/CONSTRAINTS.md)和[分工](docs/revc/PLAN.md)，再开始相应工作线。

- 原 PCB 工作线持续拥有原理图和 PCB。原外壳及主要设计线持续拥有外壳、机构、附件及装配。系统总控管理接口、范围和放行。固件与主机保留各自的上游责任。
- 设计和制造判断使用各自的证据。按产品坐标登记，明确保存源文件实测、冻结需求、候选与未知。以 `null` 留空未知参数。
- 本地工程起点见 [审查报告](docs/revc/REVIEW.md)。PCB 写入只经 Konnect，读回验证并更新[矩阵](docs/revc/matrix_numeric.json)、[字典](docs/revc/component_dictionary.json)和[变更](docs/revc/CHANGELOG.md)。旧资料用来解释沿革。
- PCB 实改后重新输出单张四层真实铜线/焊盘/连接孔总览，并注明保存板 SHA、DRC/ERC、真实未连接和验证缺口。
- 将经过检查的 RevC 变更直接同步 GitHub `main`。交付文件路径、变更 SHA 和仍待验收项。七项制造门槛证据齐备前维持制造未放行。
