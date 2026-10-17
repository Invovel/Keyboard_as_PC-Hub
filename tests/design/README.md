# R3设计与可选麦克风检查

2026-10-17。区分代码编译、图形交互与制造/硬件验证。

| 检查 | 结果与边界 |
| --- | --- |
| 原Hairline HTML更新 | 原文件名与源码入口保留；bench/kernel校验通过 |
| 八状态截图 | 默认、响应、小尺寸及响应、滑块两端、深浅主题通过取景/读数/控制台检查 |
| 附加浏览器交互 | 等面积凹弧键、固定中心、点击固定/复位、两种磁吸、滑块端点、导览、play、离线、清理均通过 |
| 几何和文件解析 | kiutils 1.4.8可读板图和原理图：15封装、34网络（含0）、11接口符号、79连接器引脚行 |
| 默认固件 | ESP-IDF 6.1，麦克风关闭配置编译通过，应用大小0x9c830；未烧录 |
| 麦克风固件 | BODY_MIC_ENABLE=y编译通过，应用大小0x9d3a0；未烧录、未接麦克风 |
| KiCad ERC/DRC | 未运行，环境无KiCad；板图没有走线或铺铜 |
| OpenSCAD实体编译 | 未运行，环境无OpenSCAD；未生成STL/STEP |
| 实物装配/麦克风采样 | 未进行；已有主体功能测试不代表新增设计通过 |

构建使用[build.ps1](../../firmware/body-core/tools/build.ps1)，分别不带开关与带`-WithMicrophone`，同步到英文路径副本，仓库为权威源。没有使用`-Flash`；旧烧录包及校验文件保持原样。工具链输出的IDF Kconfig上游提示不被当成麦克风实测结果。

```powershell
python tests/design/check_r3_geometry.py --libs C:\path\to\kiutils-library-directory
# 仓库根目录；设置HAIRLINE_LOOK_CACHE后运行
node design/visualization/check-interactions.cjs
```

[几何解析记录](r3-geometry-result.json) · [八状态与源码](../../design/visualization/README.md) · [麦克风说明](../../docs/microphone-inmp441.md)

几何检查只核对包络、镜像中心、边框/间隙与接口表，不能替代器件间距、装配公差、电源或高速信号验证。
