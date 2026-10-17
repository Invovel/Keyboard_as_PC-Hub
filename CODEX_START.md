# 下一阶段入口

先读 [项目起始规划](docs/project-start.md)、[README](README.md) 和 [接线基准](hardware/electronics/current-wiring.md)。

当前任务为 P01：把输入事件、UI、动作映射、HID、总禁用和外部事件接入分成模块；列出启动禁用、保持键释放、断连及重复包的主机模拟用例。尚未开始实现。

本轮只建立后续实施起点，不开串口、读板、刷写或改接线。保留历史COM3/COM4；板型和端口身份不能靠COM号推断。后续硬件操作需用户明确授权。

每阶段完成后记录学习过程和验收证据，更新 docs/project-start.md。旧代码集中在 firmware/legacy/，仅作参考，不作为可刷入ESP32的固件。

后续直接在 main 上优化。目录按[仓库规范](docs/repository-layout.md)维护：固件、设计图、PCB、外壳、烧录包和测试日志分别存放。PCB 与外壳当前仅有规划资料，不得将展示图标为生产工程。
