# Codex 启动指令 — Keyboard as PC Hub R3.1

请先读取 `docs/R3_1_CODEX_HANDOFF.md`，它是当前会话已确认的 R3.1 架构目标。再读取仓库 `README.md`、`firmware/body-core/README.md`、`docs/body-core-bringup.md`、`hardware/electronics/current-wiring.md`、`hardware/enclosure/dimensions-r3.md`、`hardware/pcb/sources/body-r3/README.md`、`docs/ports-and-power.md` 和 `docs/desktop-power-dock.md`。

**第一阶段先做 WP0**：逐条列出 R3.1 与旧仓库的冲突、给出最小文件变更计划；将旧版“外部 PD 作为主体必须供电、背部 A×3+POWER”定位迁移到 `Core + 可选 Power Dock`，Core 保留 A1/A2/FAST/PC-C/KBD/CTRL、将 A3 逻辑替换为底部 Dock P4，下置 Dock 没有电脑上行。同步链接和参数，不改写真实测试日志，不擅自更改已运行 GPIO，不声称制造验证通过。

**第二阶段按 WP1/2 设计评审**：评估 USB7206C 单线供电可行性、Hub 独立启动、Type-C CC/方向、端口保护、中央旋钮机械干涉、Dock 连接器和 USB3/供电管理。先输出具体审查结果及待定阻塞，不要凭候选位置直接生成“可下单 Gerber”。

只在实际有工具、器件规格和实测条件时进行工程操作。每轮输出变更文件、检查命令及结果、未完成风险、下一步。保留其他已决定项目：五键 + 前部 104mm 横向触控条，MAG 为低功耗，Rev B 才实现 Power Dock。不要扩展竖触控条、双向供电、第二上行、手机 PD 快充、读卡、电池。
