# 外壳与机械设计

**当前有参数化 OpenSCAD 草模，尚未完成实体编译、装配验证或可制造 STEP/STL。**

- [dimensions-r3-1.md](dimensions-r3-1.md)：当前Core＋可选下置Dock机械说明。
- [interface-parameters-r3-1.json](interface-parameters-r3-1.json)：当前参数与未冻结项。
- [body-r3-1.scad](sources/body-r3-1.scad)：新草模，含Dock包络/分区/剖面研究。
- [update_r3_1.py](tools/update_r3_1.py)：只生成R3.1 JSON和SVG，不写KiCad或旧R3。
- [dimensions-r3.md](dimensions-r3.md)、[interface-parameters-r3.json](interface-parameters-r3.json)、[body-r3.scad](sources/body-r3.scad)：历史R3。
- [dimensions-r2.md](dimensions-r2.md)、[interface-parameters.json](interface-parameters.json)：历史 R2 尺寸与参数。
- [dimensions-r0.md](dimensions-r0.md)：历史方案。
- [sources](sources/README.md)：后续可编辑 CAD、参数化建模源文件。
- [exports](exports/README.md)：后续 STEP、分件 STL、机械工程图及装配清单。

主壳、屏幕托架、旋转环、键面及磁吸附件按零件分文件，共享装配基准。
概念展示位于 [design](../../design/README.md)，PCB 工程位于 [pcb](../pcb/README.md)。
