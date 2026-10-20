# 实物测试日志

[logs/body-core](logs/body-core/) 保存主体启动、GPIO、编码器及集成串口原始记录。
日志保持原始字节，不将早期失败记录改写成通过；完整时间线和当前状态见[验收说明](../../docs/body-core-bringup.md)。

在仓库根目录、ESP-IDF Python 环境中记录新的串口测试：

```sh
python firmware/body-core/tools/capture_serial.py --port COM5 --seconds 180 --output tests/hardware/logs/body-core/session-new.txt
```

使用新的文件名避免覆盖历史。录制期间不要同时运行其他串口工具。
