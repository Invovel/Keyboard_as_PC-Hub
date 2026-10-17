# 仓库目录规范

后续开发基于 **main**。按用户要求将代码、设计图、PCB 和外壳工程分开，现有文件已实际迁移。

```text
firmware/
  body-core/               当前 ESP32-S3 固件源码、依赖与构建工具
  gpio-smoke/              GPIO 诊断工程
  legacy/                  历史 STM32 与功能框架
design/
  concepts/                外观概念图
  drawings/                布局、装配表达图与历史 PDF
  visualization/           交互展示、绘图源码、预览截图
hardware/
  electronics/             散件与实物接线
  pcb/
    sources/               R3 KiCad 接口工程及后续板件
    exports/               后续 Gerber、钻孔、BOM、贴片坐标
    references/            接口调查等依据
  enclosure/
    sources/               R3 OpenSCAD 草模及后续装配工程
    exports/               后续 STEP、分件 STL、机械工程图
    dimensions-r3.md       当前候选尺寸
    interface-parameters-r3.json
releases/body-core/        烧录包、配置及校验清单
tests/hardware/logs/       原始实物测试日志
docs/                     规划、研究和验收说明
```

## 放置原则

1. 固件代码以 `firmware/body-core` 为权威来源，目录整理不更改 GPIO 或运行逻辑。构建缓存不提交。
2. 概念图、结构示意和 HTML 不是可加工文件，统一在 `design`。HTML 的绘图源码与展示资源保持同目录，方便离线打开和重建。
3. PCB 与外壳分别维护源工程和导出文件。未来按模块及修订号分子目录；一个修订的导出必须能追溯到对应源工程。
4. PCB 板框、孔位和外壳装配基准需要共同核对，但不合并成同一个杂项目录。
5. 原始日志保存到 `tests`；测试结论写在 `docs`，不能改写历史失败日志。
6. 镜像保存到 `releases`，包含烧录说明、依赖/源码来源、校验值。后续生成新版本，不覆盖旧包。

## 当前交付状态

主体固件及镜像已存在，基础功能用户实测正常。R3 已新增 KiCad 模块接口原理图与未布线 PCB、OpenSCAD 参数化草模。PCB 没有 KiCad ERC/DRC 或制造文件；草模没有实体编译、STEP/STL 或装配验证。麦克风代码已编译，未接实物。

## 旧路径迁移

| 原位置 | 新位置 |
| --- | --- |
| Functions/ | firmware/legacy/functions/ |
| STM32F103C8T6/ | firmware/legacy/stm32f103c8t6/ |
| 设计图/ | design/drawings/history/ |
| assets/ | design/concepts/ |
| visualization/ | design/visualization/ |
| mechanical/layout-r2.svg | design/drawings/layout-r2.svg |
| mechanical/ 的尺寸与参数 | hardware/enclosure/ |
| hardware/current-wiring.md | hardware/electronics/current-wiring.md |
| hardware/keyboard-port-survey.csv | hardware/pcb/references/keyboard-port-survey.csv |
| firmware/body-core/artifacts/ | releases/body-core/ |
| firmware/body-core/logs/ | tests/hardware/logs/body-core/ |

旧聊天中的绝对文件链接可能仍指向迁移前位置；从仓库首页或本表进入新目录。
