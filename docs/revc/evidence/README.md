# 保存板证据

PCB SHA-256 `3f082629954bdb488613209f4213198f4b84ce832a26fdd4af76abb786354d88`。见 [源文件清单](Audit_Source.json) 与 [KiCad 几何统计](Overview_Manifest.json)。

- [真实四层综合图 SVG](PC_Hub_Pure_Circuit_Overview.svg) · [同图 PNG](PC_Hub_Pure_Circuit_Overview.png)。KiCad 原生几何生成，含线路、实际焊盘、孔、板框。大面积地铜仅隐藏显示。F/B 有 381/171 段，In1/In2 各有两区地铜、零布线段。
- [DRC](DRC.json) · [ERC](ERC.json)。Konnect 对经哈希核对的保存副本执行，未同步实时编辑器，也未保存分析用填区。DRC 12 钻孔错误、82 丝印压铜、54 丝印重叠、52 库封装不一致，以及 120 未连接与 41 图板一致性问题。ERC 有 2 个 ISO J101 电源引脚未驱动错误，另有 5 个单端标签和 2 个名称歧义警告。
- [焊盘与钻孔坐标](Pad_Coordinate_Map.csv)。列包含产品 XY 与原生 KiCad XY，各轴单位 mm；绝对 Z 需装配基准。四层复合图的几何可按产品坐标通过矩阵交叉定位。
- [原理图全层级导出摘要](SCHEMATIC_SUMMARY.md)。完整网表保存在本地审查目录。
