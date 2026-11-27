# 固件代码

| 目录 | 用途 |
| --- | --- |
| [body-core](body-core/README.md) | 当前 ESP32-S3 主体：显示、触控、HID、按键、编码器和音频 |
| [gpio-smoke](gpio-smoke/TEST-2026-10-16.md) | 历史 GPIO 探测工程，不能当作完整主体固件 |
| [legacy](legacy/README.md) | 旧 STM32/功能框架，只供参考 |

主体工程继续在原路径构建，驱动和 GPIO 逻辑未因目录整理修改。
烧录包统一放 [releases/body-core](../releases/body-core/README.md)，串口日志放 [tests/hardware](../tests/hardware/README.md)，设计图不放在固件目录。
