# 主体整合固件

ESP32-S3 N16R8，ESP-IDF 6.1。目标为先跑通主体；Hub、磁吸附件和 eFlesh 暂不参与。
完整实测状态见 [bring-up 记录](../../docs/body-core-bringup.md)。实物调整后，编码器双路及正反转事件已恢复，原生 USB 音量/滚动复测后用户确认全部基础功能正常。Touchpad 延迟后续再优化。

## 运行与接口

| 模块 | GPIO / 接口 |
| --- | --- |
| QSPI LCD | CS16、SCLK6、D0=15、D1=7、D2=11、D3=12；360×360 |
| I²C 触控 | SDA8、SCL9；地址 0x15；无独立 INT/RST |
| 四键①②③④ | 42、41、40、38；上一曲、播放/暂停、下一曲、静音 |
| 编码器旋转 | 固件 A1、B2；PCNT 官方示例适配；双路及正反转事件已实测 |
| 按压 | 39；媒体播放/暂停，触摸板页鼠标左键 |
| MAX98357A | BCLK47、WS48、DIN21 |
| BOOT | GPIO0；运行时禁用/恢复电脑控制输出 |
| 原生 USB | GPIO19/20；键盘、鼠标、Consumer Control 复合 HID |
| FTDI USB | 当前枚举 COM5；烧录与日志，与原生 HID 接口分工 |

页面包括 Home、Media、Touchpad、Settings。设置保存灵敏度和声音开关；Settings 的 A/B 为 GPIO 边沿计数，P 为 PCNT 硬件有符号计数。P 每累积四个有效计数产生一次旋转动作；实际每格脉冲数和方向仍需实测。媒体页旋转调音量，触摸板页旋转滚动。

圆屏采用本机既有点屏证据中的寄存器表，通过 ST77916 驱动传输；不因此断言屏幕实际 IC 型号。触控同样只确认兼容坐标协议。来源和许可证见 [THIRD_PARTY.md](THIRD_PARTY.md)。

## 构建与烧录

依赖版本固定在 `main/idf_component.yml` 与 `dependencies.lock`。先在 ESP-IDF 6.1 环境中执行：

```sh
idf.py build
idf.py -p COM5 flash
idf.py -p COM5 monitor
```

本机中文路径需要同步到英文目录构建。仓库源码是唯一权威版本；辅助脚本的默认工具链路径为本机配置，可通过参数覆盖：

```powershell
./tools/build.ps1 -Stage C:\esp-work\body-core
./tools/build.ps1 -Stage C:\esp-work\body-core -Flash -Port COM5
```

在固件目录记录串口（录制期间勿同时运行 monitor/esptool）：

```sh
python tools/capture_serial.py --port COM5 --seconds 180 --reset --output ../../tests/hardware/logs/body-core/session-new.txt
```

编译成功后构建目录包含 `body_core.bin`、`bootloader/bootloader.bin`、`partition_table/partition-table.bin` 和 `flash_args`。优先按生成的 `flash_args` 烧录。只更新应用时地址为 `0x10000`，不擦除 NVS。GPIO 计数静止时不要以提示音或驱动初始化成功判断旋转通过。

归档镜像位于 [releases/body-core](../../releases/body-core/README.md)，原始日志位于 [tests/hardware](../../tests/hardware/README.md)，不与固件源文件混放。
