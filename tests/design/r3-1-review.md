# R3.1实施与检查记录

2026-10-19。用户输入包已导入，迁移依据见[差异台账](../../docs/R3_1_DECISIONS.md)。本次完成WP0文档/展示迁移、WP1/WP2资料审查和机械空间示意，不是Rev A电路完工或Rev B实物交付。

| 项目 | 本轮证据 / 状态 |
| --- | --- |
| 输入包 | SHA256及两份导入原文校验见[来源记录](../../docs/r3-1-package-source.json) |
| 架构一致性 | [check_r3_1.py](check_r3_1.py)：Core唯一上行、六逻辑下游、PD仅在Dock、无固定旧功率、附件保留 |
| 空间研究 | 48×18三分区、8mm板面间距、名义安装孔/中央避让；只二维边界，不代表真实连接器干涉通过 |
| 历史保护 | 与输入时baseline commit比较，固件/镜像/原始日志、R3参数/SCAD/布局、KiCad文件内容不变 |
| 补充材料与旧入口 | 四份TXT原文SHA256保留；旧generate.py实际返回2且无写入，早期相反提案不作为当前需求 |
| Hairline | 原HTML更新；校验固定引擎/模板和浏览器八状态、固定中心/复位、磁吸/Slider及可选Dock显隐 |
| 文档 | 对待提交快照运行相对Markdown链接检查 |
| 未做 | 编译固件/烧录、实体装配、单线功耗/USB实测、KiCad ERC/DRC、OpenSCAD实体编译、USB信号/温升、Gerber |

```powershell
python hardware/enclosure/tools/update_r3_1.py
python tests/design/check_r3_1.py
# 在design/visualization设置HAIRLINE_LOOK_CACHE后运行
node D:/Codex/.codex/skills/hairline-create/look.mjs keyboard-hub.js --answer 0,0,31 --edge 110,35,24 --zoom rest
node check-interactions.cjs
```

[一致性结果](r3-1-result.json) · [八状态](../../design/visualization/hairline-keyboard-hub-look.png) · [机构/电源阻塞](../../docs/R3_1_ENGINEERING_REVIEW.md)

与旧100W单体方案的差异已经修正文档、图形和参数；没有把旧KiCad被动载板重命名冒充原生PCB。下一步是Core实测峰值功耗/上游限制与Dock配对器件冻结，然后才进行受控电气工程操作。
