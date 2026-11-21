# PC-Hub 圆屏四键内容槽固件

2026-11-16，独立 ESP-IDF 6.1 / ESP32-S3 / LVGL 固件。

- 四个96×79内容槽固定在对应实体键的四象限，未绑定也保留区域。
- 只在主界面置顶；其他页面与过渡期间隐藏。
- 首页轻点控制整层显隐；设置中的“边框显示”控制区域轮廓并保存到NVS。
- 默认顺时针：语音入口、Codex、设置、Chrome。物理驱动编号为1/4/3/2。
- 语音只进入待机界面，不启动采集或转写。
- 任意图片上传、本地配置网站、自由快捷键编辑尚未实现。天气/指标页面仍保留旧实现。

设备最终镜像验证见 floating-layer-validation.json 和 floating-layer.log。八段过渡渲染约30.6–42.4FPS，无内容槽泄漏到过渡帧。自动测试不代表光学测量或所有负载下最低帧率保证。

## 字库与构建

本次实机字库来自本机字体，未获得公开分发许可，所以不提交生成的 main/deck_font.c 及包含它的固件二进制。请准备具有适当授权、包含简体中文的TTF字体，然后运行：

```powershell
python tools/generate_font.py --font 'D:/fonts/your-font.ttf'
./tools/build.ps1 -Stage 'D:/build/pc-hub-deck'
```

生成工具需要 Node.js、npm，并通过 npx 调用 lv_font_conv。build.ps1 的默认 ESP-IDF、工具链和Python路径来自验证机器，请按本机环境传入 IdFRoot/ToolsRoot/PythonEnv 参数。

仅包含主应用源码和组件锁文件，没有上传临时构建目录、NVS/整机备份、私有配置或常驻服务。GPIO、USB描述符与先前板卡驱动保持一致；上传源码不代表其他屏幕型号兼容。
