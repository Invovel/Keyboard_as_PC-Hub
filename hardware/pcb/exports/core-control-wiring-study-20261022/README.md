# Pc-Hub 主体控制电路验证快照

本目录记录 **2026-10-22 开始、2026-10-23 整合回主工程的电气验证**。`Core-Control-RevA` 是同一块主体主板的控制电路层次页/文件名，不是第二个产品或新增独立实体 PCB。

- [原主工程两页 PDF](pc-hub-main.pdf)、[主工程首页](pc-hub-main.png)
- [控制页 PNG](schematic.png)、[可缩放 SVG](schematic.svg)、[控制页 PDF](schematic.pdf)
- [导出网表](schematic.net)、[器件表](components.csv)
- [ERC 结果](erc.json)、[MCP 检查证据](verification.json)
- [独立网表逐脚核对](independent-netcheck.json)
- [物理引脚—符号—焊盘逐项表](physical-pinmap.csv)、[物理接受状态](physical-pinmap-status.json)
- [七类封装实放核对图](physical-scratch-7parts-labelled.png)、[原生 KiCad SVG](7plus-pinmap-board.svg)、[逐 pad 比对](physical-scratch-check.json)。核对图的白色编号来自 MCP 读回，为原生导出的补充标注，不是 KiCad 原生绘制内容。
- [控制电路/线束/装配接口说明](../../references/core-control-reva-electrical.md)

本次已得到：64 个符号（含 4 个电源标记）、48 个实际命名网络；CLI 网表另有 15 个明确 NC 的独立占位网。ERC 为 0 error / 0 warning，直接短路检测为 0，连线和器件连接检查均无浮空。外部网表对 48 个网络的逐脚核对通过。

`components.csv` 的 Assembly 列是本次装配计划；当前 Konnect 字段接口写的是装配属性文字，没有伪称已经设置 KiCad 原生 DNP 标记。制作正式贴片 BOM 时需要把选装/条件不装规则真正应用到制造输出。

原 `Pc-Hub.kicad_sch` 已经通过 MCP 整合为主板首页 + `Main_Control` 控制层次页；旧 CN7/CN10 内容已按精确 SHA256 备份后移除。原主工程两页 ERC 同样为 0 error / 0 warning，导出主网表已独立确认 48 个控制网保持、四个机械孔存在、旧 CN7/CN10 不再作为器件出现。

七类 MCU/Buck/mux/DRV/GH 封装已经完成实际临时测试板 readback、逐 pad 与库核对及渲染检查。用户随后明确要求只操作原文件，因此 L1 和台架输入焊盘的后续验收在原主板完成，不能写成临时板刷新已经通过。

这些结果证明导出时的电气拓扑连通，不证明拆机马达参数、USB3 链路或整机电源已经验证。实际主板封装/布线、Hub 与电源选择仍不在电气检查通过范围。当前状态：**不可投板**。
