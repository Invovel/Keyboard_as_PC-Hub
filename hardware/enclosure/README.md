# 外壳与机械设计

**当前有参数化 OpenSCAD 草模，尚未完成实体编译、装配验证或可制造 STEP/STL。**

- [dimensions-r3.md](dimensions-r3.md)：当前 R3 对称长条结构与候选尺寸。
- [interface-parameters-r3.json](interface-parameters-r3.json)：R3 参数数据。
- [body-r3.scad](sources/body-r3.scad)：可选择底壳、顶框、环、托架和键面的草模。
- [dimensions-r2.md](dimensions-r2.md)、[interface-parameters.json](interface-parameters.json)：历史 R2 尺寸与参数。
- [dimensions-r0.md](dimensions-r0.md)：历史方案。
- [sources](sources/README.md)：后续可编辑 CAD、参数化建模源文件。
- [exports](exports/README.md)：后续 STEP、分件 STL、机械工程图及装配清单。

主壳、屏幕托架、旋转环、键面及磁吸附件按零件分文件，共享装配基准。
概念展示位于 [design](../../design/README.md)，PCB 工程位于 [pcb](../pcb/README.md)。
