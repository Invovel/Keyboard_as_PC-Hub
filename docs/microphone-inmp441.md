# 麦克风接入：INMP441（本轮不接实物）

建议在现有 ESP32-S3 工程中选 **INMP441 模块**。本轮已增加代码、页面和 PCB 逻辑接口；只进行编译与静态检查，不烧录、不宣称音频实测通过。

## INMP441 与 MAX9814

| 项目 | INMP441 | MAX9814 模块 |
| --- | --- | --- |
| 信号 | 内置 ADC 的 I²S 数字输出 | 驻极体麦克风 + 模拟前置放大器输出，需另行 ADC 采样 |
| 当前工程适配 | ESP-IDF I²S1 RX | 需要设计 ADC、偏置、滤波与抗干扰 |
| 增益 | 后续在软件处理 | IC 自带自动增益，动态特性由硬件设置 |
| 电气布局重点 | 时钟、数字串扰、声孔与电源去耦 | 模拟走线、地回路、电源噪声、ADC 满量程 |
| 本轮选择 | 使用现成模块，先建立 PCM 数据通路 | 留作模拟方案参考，不实现未经验证的双方案驱动 |

INMP441 官方目录状态为 **Production (NRND)**，现成模块可用于本次原型，后续产品化需重新选型与确认供货；不在这里硬指定未验证的新料替代。MAX9814 是放大器 IC，本身不是完整数字麦克风。[TDK 产品资料](https://product.tdk.com/en/search/sw_piezo/mic/mems-mic/info?part_no=INMP441)、[MAX9814 官方资料](https://www.analog.com/en/products/max9814.html)。

## 新增代码与实际边界

- [body_mic.c](../firmware/body-core/main/body_mic.c)：ESP-IDF 新 I²S 驱动，独立 I²S1 RX，默认不启用。
- [body_mic.h](../firmware/body-core/main/body_mic.h)：启动/停止采集、状态快照、PCM16 单声道帧读取 API。
- Home 增加 Mic 页入口；不编译采集支持时明确显示 build disabled。启用支持后仍默认 Capture OFF，由页面按钮开启。
- 帧长 256 点、16 kHz；I²S 使用 32-bit×2 时隙，保证每采样帧 64 BCLK，取左通道高 16 位形成 PCM16。
- 页面提供峰值、RMS dBFS、帧数及错误数。dBFS 是数字幅度，不是校准后的环境分贝；没有识别词、声源定位或唤醒词功能。
- `body_mic_read_pcm()` 提供 8 帧队列，一个消费者读取；没有消费者时旧帧被丢弃并计数，不无限占用内存。
- 本版不导出 USB Audio Class，不让电脑自动识别为录音麦克风；也不自动上传、存储录音或把声音转成 HID 指令。
- BOOT 总禁用会请求停止采集；重新启用输出不会自动重开录音。停止生效需等待当前 I²S 读取返回，最长等待配置为 100 ms 加任务调度。

## 预留接线（尚未接实物）

| INMP441 模块 | ESP32-S3 / 电源 | 说明 |
| --- | --- | --- |
| VDD | 3.3V | 不接 5V |
| GND | GND | 与主体共地 |
| SCK/BCLK | GPIO4 | I²S1 主机输出 |
| WS/LRCLK | GPIO5 | 16 kHz 帧时钟 |
| SD | GPIO17 | I²S1 接收输入 |
| L/R | GND | 选择左时隙 |

GPIO4/5/17 是新增预留，未改变已经运行的屏幕、按键、编码器和音频 GPIO。实际模块的引脚顺序不能按本表位置猜测。若模块引出 CHIPEN，应按数据手册使能，未引出时需确认模块板已有相应连接。

TDK 数据手册规定 24-bit I²S 数据、64 BCLK/立体声帧；建议 SD 加 100 kΩ 下拉，电源近端加 100 nF。本代码额外启用内部弱下拉以减少空接口漂浮，但它不能替代 PCB 外部电阻与去耦。底部声孔不得被 PCB、胶或外壳堵住。[INMP441 数据手册第 11–12 页](https://product.tdk.com/system/files/dam/doc/product/sw_piezo/mic/mems-mic/data_sheet/inmp441.pdf)。

现有 MAX98357A 继续使用 I²S0、GPIO47/48/21；麦克风与扬声器用不同 I²S 控制器及引脚。两者同时运行仍可能发生声学反馈，外壳分腔和后续回声处理不能由“全向”特性自动解决。

## 编译方式

在 ESP-IDF 工程中运行 `idf.py menuconfig`，进入 PC Hub body peripherals，设置 BODY_MIC_ENABLE。默认关闭，不占用新增 I²S 引脚。

本机英文路径构建工具支持：

```powershell
# 仓库根目录；关闭麦克风支持的默认构建
./firmware/body-core/tools/build.ps1 -Stage C:\esp-work\body-core
# 独立 sdkconfig.mic 配置，开启采集支持；只编译，未使用 -Flash
./firmware/body-core/tools/build.ps1 -Stage C:\esp-work\body-core -WithMicrophone
```

两种配置均已编译通过；采样对齐、声学效果、实际输入信号、UI 实物操作及与扬声器同时运行均未实测。接口初始化成功也不证明麦克风存在。
