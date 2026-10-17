# 主体整合镜像（基础功能实测通过）

`pcnt-20261017/` 为 2026-10-17 已编译、通过 COM5 烧录校验并启动的整合版本。
应用大小 640016 字节。它实际接入 ESP-IDF 官方 PCNT 示例。早期连接状态下计数为零，
用户调整实物后同一镜像已捕获双向旋转，原生 USB 音量/滚动复测后用户确认所有功能正常。
未因此再次修改或烧录固件。Touchpad 延迟尚未优化，专项验收证据边界见上级实测文档。

目录包含应用、引导程序、分区表、实际构建 sdkconfig、各文件 SHA256，以及
构建时 main 源文件的 SHA256（UTF-8、换行统一为 LF 后计算，避免 Git 换行转换导致误报）。打包时逐文件核对 main 与英文路径构建副本一致。
依赖版本见上级 `dependencies.lock`，源码和许可随仓库提供。

在 `pcnt-20261017` 目录使用 ESP-IDF Python 环境：

```sh
python -m esptool --chip esp32s3 --port COM5 write-flash --flash-mode dio --flash-freq 80m --flash-size 16MB 0x0 bootloader.bin 0x8000 partition-table.bin 0x10000 body_core.bin
```

`pre-pcnt-app.bin` 是烧录本版本前从该开发板 `0x10000` 读取的 632144 字节应用备份。
这是上一版边沿中断诊断固件（同样尚未解决旋转），不包含 NVS 或全片内容。
在本目录恢复应用：

```sh
python -m esptool --chip esp32s3 --port COM5 write-flash 0x10000 pre-pcnt-app.bin
```

无需擦除全片。上面的 COM5 是本机 FTDI 接口；其他电脑应替换成实际串口。
