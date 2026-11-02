# 7 Plus 马达布局：原生执行记录

2026-10-22。目标为用户指定的`C:\Users\indeg\OneDrive\文档\KiCad\10.0\projects\Pc-Hub\Pc-Hub\Pc-Hub.kicad_pcb`。

本轮已更新参考图、JSON和接线说明。初始IPC未监听和自动审批网络问题随后已解决，现已通过Konnect绑定指定实时板图、修改并保存，`requested_open=true`、`get_board_info.source=ipc`。**7 Plus机械长槽及接口移动已完成；新原理图和PCB连线的完成情况须另行检查，不由机械修改推定。**

已写入：H1机械参考中两侧15×54长槽及电源候选区、三处文字；JH1/JH2到产品坐标(±78,−6)，JG1/JG2到(±76.5,+6)。修改前保存并备份PCB/SCH/PRO/H1库；未发现锁定对象。库先修改、读回、预检，再按准确plan_revision刷新H1，确认1项应用、0冲突。再次读回确认仍23个封装、50个机械图元，只有上述四封装移动，其他位置/旋转/层保持不变。原生文件和库均通过Konnect修改。

本轮[原生导出与检查](../exports/core-r3-1-7plus-study/README.md)：DRC已同步当前实时板图后运行，共12错误、38警告；仍未通过。布局检查未报告庭院重叠，但圆角板框超出其几何证明能力，结果为`outline_unproven`，不能把分数100写成检查通过。当前板网数0，尚未接网/布线。

10月19日[原生预览与DRC](../exports/core-r3-1-haptic-study/README.md)是旧20×36×8布局的历史快照，不能作为本轮长槽已完成或DRC已通过的证据。原旧载板原理图仍需备份、重建和核对后，才可同步新PCB网络。
