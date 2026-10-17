# 下一阶段入口

当前定位：[键盘联接式桌面供电与控制底座](docs/desktop-power-dock.md)。后续直接在**main**继续，不新建同用途分支或重复仓库。

主体圆屏、触控、HID、四键、编码器和提示音已有实测；[固件状态](docs/body-core-bringup.md)与新增[可选麦克风](docs/microphone-inmp441.md)分开记录。麦克风只编译，未接实物；本轮未烧录。

设计以[R3](hardware/enclosure/dimensions-r3.md)为准。原[HTML](design/visualization/hairline-keyboard-hub.html)在原文件上更新，四键齐平、磁吸五键保持高度。PCB草稿可编辑但未布线；OpenSCAD草模未做实体编译或装配验证。

下一步围绕自供电Hub台架、输入/分路电源和插头选型验证，见[端口表](hardware/electronics/dock-port-selection.csv)。控制载板、Hub子板、电源子板和磁吸端面小板分开；现有载板J9只接5V。

代码、设计、PCB、外壳、烧录包与测试记录按[目录规范](docs/repository-layout.md)分存。旧[起始规划](docs/project-start.md)保留为历史，不能将其中“尚未实现主体”状态覆盖当前实测结果。
