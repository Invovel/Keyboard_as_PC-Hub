# 🎮 适配60%键盘的小键盘式拓展模块完整方案
结合你的键盘构造和最新需求，我帮你把模块设计成**小键盘分区形态**，外壳长度与键盘完全一致，视觉上和键盘连成一体，同时保证操作顺手、功能完整。

---

## ✨ 核心需求整理（精准匹配你的键盘）
### 1. 形态约束
- **外壳长度**：与60%键盘完全一致（约280mm），视觉上连成一条线，整体感强
- **模块形态**：类似小键盘的分区布局（非方形），宽度比键盘窄（约40mm），高度与键盘持平（约15mm）
- **空缺处理**：功能区外的空缺部分用透明盖板占位，后续可拓展，不影响当前操作
- **防尘结构**：三面包围式外壳（仅右侧开放接口），与键盘透明外壳材质呼应（推荐透明PLA）

### 2. 连接与固定
- **主连接**：模块左侧伸出凸出USB-C公头，直接插入键盘左上角USB-C母座（取电+基础连接）
- **固定方式**：模块两侧加装3D打印卡扣，扣住键盘左侧边缘，稳固不晃，拆装方便
- **拓展预留**：空缺盖板下方预留pogo pin触点，后续可加拓展模块（如额外按键板）

### 3. 功能保留（操作优先）
- 核心控制：旋钮（带双色LED）+ 类雷蛇滚轮（复位钮子开关）+ 5个热插拔按键
- 状态显示：0.96寸OLED屏 + 3个彩色LED（供电/调试/高速接口状态）
- Hub拓展：USB-C上行 + 3×USB-A + 2×USB-C（含高速接口）+ PD双供电切换

---

## 🎨 模块形态与布局方案（适配60%键盘）
### 1. 整体形态（视觉统一）
- 外壳：透明PLA材质，长度280mm（与键盘一致），宽度40mm（比键盘窄5-10mm，视觉协调），高度15mm（与键盘持平）
- 结构：三面包围防尘（左/上/下封闭），仅右侧开放接口，左侧用凸出USB-C公头连接键盘，空缺部分用透明盖板占位（与外壳一体3D打印，成本低）
- 视觉效果：模块与键盘连成一条线，透明材质呼应键盘外壳，整体简洁统一

### 2. 功能区布局（小键盘式，操作顺手）
按小键盘的“上中下”分区布局，左手自然放置即可操作，不影响右手敲键盘：
```
┌──────────────────────────────────────────────────────────────┐
│ [透明盖板（空缺占位）]  │  [OLED屏]  [红/黄/蓝状态LED]  │  上区：状态可视化
├──────────────────────────────────────────────────────────────┤
│ [EC11旋钮+双色LED]  │  [热插拔按键×5（2×3紧凑布局）]  │  中区：核心控制
├──────────────────────────────────────────────────────────────┤
│ [复位钮子开关（类雷蛇滚轮）]  │  [供电切换拨动开关]  │  下区：滚轮+供电控制
└──────────────────────────────────────────────────────────────┘
模块右侧（开放端）：USB-C上行口 + 3×USB-A + 2×USB-C + PD供电口
模块左侧（靠键盘端）：凸出USB-C公头（插键盘左上母座）+ 卡扣
模块背面：主控+Hub芯片+散热片 + 预留pogo pin触点（盖板下方）
```

### 3. 按键功能映射（编程场景优化）
| 按键位置 | 功能映射（VS Code优先） |
|----------|--------------------------|
| 按键1    | F5（调试启动）|
| 按键2    | Ctrl+S（快速保存）|
| 按键3    | Ctrl+Shift+B（项目构建）|
| 按键4    | Ctrl+Shift+P（命令面板）|
| 按键5    | Esc（退出调试/取消）|

---

## 🛠️ 硬件清单（预算精准控制，总235元）
| 模块               | 选型（高性价比）| 预算（元） | 核心作用                     |
|--------------------|-------------------------|------------|------------------------------|
| 主控               | 全新树莓派Pico（RP2040） | 20         | 核心控制，贴模块背面省空间 |
| USB Hub芯片        | VL813（USB3.0，1上5下） | 25         | 满足3A+2C接口，高速兼容 |
| 控制核心组件       | EC11旋钮（带LED）×1 + 复位钮子开关×1 + Kailh热插拔底座×5 + 二手MX红轴×5 | 38         | 小键盘式布局，操作顺手 |
| 连接&固定组件      | 凸出USB-C公头（焊板款）×1 + pogo pin（4针，拓展用）×1 + 3D打印卡扣×2 | 12         | 插键盘左上母座，卡扣固定 |
| 供电组件           | PD20W诱骗器+小型拨动开关+自恢复保险丝 | 13         | 双供电切换，避免过载 |
| 显示&状态组件      | 0.96寸OLED（I2C，二手）+ 彩色LED×3 + 限流电阻 | 15         | 状态可视化，抬头可见 |
| 接口&辅料          | USB3.0 A/C母座×5 + 屏蔽线（高速用）+ 散热片 + 焊锡/杜邦线 | 22         | 接口集中右侧，防干扰 |
| 3D打印外壳+盖板    | 透明PLA，三面包围，长度280mm | 40         | 与键盘长度一致，透明呼应 |
| 其他（缓冲垫/防尘条） | 硅胶缓冲垫+海绵防尘条    | 5          | 防刮键盘，增强防尘 |
| **合计**           | ——                       | **235**    | 完全在200-300元预算内 |

---

## 📌 关键落地细节（避坑+优化）
### 1. 外壳适配（透明+防尘）
- 材质：透明PLA，与你的键盘外壳材质呼应，视觉统一
- 空缺盖板：与外壳一体3D打印，后续需要拓展时拆掉盖板即可，成本低，灵活性高
- 防尘优化：三面包围内侧贴海绵防尘条，接口端配硅胶防尘塞（可选，1元），兼顾防尘与实用

### 2. 操作优化（顺手优先）
- 旋钮位置：放在中区左侧，左手拇指自然就能调节音量/亮度，无需抬手
- 滚轮开关：放在下区右侧，拇指下方拨动顺手，不与旋钮冲突
- 按键布局：2×3紧凑排列，左手食指/中指操作，类似小键盘的数字键布局，记忆成本低

### 3. 成本控制（实用为主）
- 外壳：长度与键盘一致，但宽度窄，3D打印耗材比方形多10元左右，但视觉效果提升明显
- 配件：OLED屏、MX轴选二手，功能无影响，单配件省5-10元
- 拓展触点：仅预留位置，暂不焊接，后续需要再补，当前不增加成本

---

我可以帮你生成**模块外壳的3D打印尺寸图**，包含精确的开孔、卡扣位置和空缺盖板尺寸，你直接发给打印方就能精准制作，需要吗？

## 固件说明
### 开发环境
- 主控：RP2040（树莓派Pico）
- 开发框架：CircuitPython / Arduino
- 依赖库：`adafruit_hid`（HID快捷键模拟）、`adafruit_ssd1306`（OLED屏驱动）

### 核心功能逻辑
1.  **类雷蛇滚轮**：持续检测钮子开关状态，左/右拨动时每100ms发送一次快捷键（触发间隔可自定义），松手开关自动复位后立即停止触发，内置50ms防抖避免误触
2.  **旋钮控制**：旋转编码器脉冲信号映射为系统音量/亮度快捷键，按压切换功能模式，双色LED亮灭反馈当前模式
3.  **热插拔按键**：按下触发对应VS Code/系统快捷键，支持长按连续触发，可通过修改固件自定义映射规则

### 极简核心代码示例（CircuitPython）
```python
import board
import digitalio
import usb_hid
import time
from adafruit_hid.keyboard import Keyboard
from adafruit_hid.keycode import Keycode
from adafruit_hid.consumer_control import ConsumerControl
from adafruit_hid.consumer_control_code import ConsumerControlCode

# 初始化设备
keyboard = Keyboard(usb_hid.devices)
consumer_control = ConsumerControl(usb_hid.devices)

# 引脚定义
# 旋钮引脚
enc_clk = digitalio.DigitalInOut(board.D2)
enc_dt = digitalio.DigitalInOut(board.D3)
enc_sw = digitalio.DigitalInOut(board.D4)
enc_clk.switch_to_input(pull=digitalio.Pull.UP)
enc_dt.switch_to_input(pull=digitalio.Pull.UP)
enc_sw.switch_to_input(pull=digitalio.Pull.UP)

# 类雷蛇滚轮钮子开关引脚
switch_left = digitalio.DigitalInOut(board.D5)
switch_right = digitalio.DigitalInOut(board.D6)
switch_left.switch_to_input(pull=digitalio.Pull.UP)
switch_right.switch_to_input(pull=digitalio.Pull.UP)

# 全局变量
mode = 0  # 0=音量模式，1=亮度模式
enc_last_state = enc_clk.value
TRIGGER_INTERVAL = 0.1  # 滚轮触发间隔
DEBOUNCE_DELAY = 0.05   # 防抖延迟

# 主循环
while True:
    # 旋钮功能逻辑
    enc_current_state = enc_clk.value
    if enc_current_state != enc_last_state:
        if enc_dt.value != enc_current_state:
            # 顺时针旋转
            if mode == 0:
                consumer_control.send(ConsumerControlCode.VOLUME_INCREMENT)
            else:
                keyboard.press(Keycode.BRIGHTNESS_INCREMENT)
                keyboard.release_all()
        else:
            # 逆时针旋转
            if mode == 0:
                consumer_control.send(ConsumerControlCode.VOLUME_DECREMENT)
            else:
                keyboard.press(Keycode.BRIGHTNESS_DECREMENT)
                keyboard.release_all()
    enc_last_state = enc_current_state

    # 旋钮按压切换模式
    if not enc_sw.value:
        time.sleep(DEBOUNCE_DELAY)
        if not enc_sw.value:
            mode = 1 - mode
            time.sleep(0.2)

    # 类雷蛇滚轮逻辑
    if not switch_left.value:
        time.sleep(DEBOUNCE_DELAY)
        if not switch_left.value:
            keyboard.press(Keycode.CONTROL, Keycode.PAGE_UP)
            keyboard.release_all()
            time.sleep(TRIGGER_INTERVAL)
    elif not switch_right.value:
        time.sleep(DEBOUNCE_DELAY)
        if not switch_right.value:
            keyboard.press(Keycode.CONTROL, Keycode.PAGE_DOWN)
            keyboard.release_all()
            time.sleep(TRIGGER_INTERVAL)

    time.sleep(0.01)
```

---

## 安装与组装步骤
1.  **3D打印外壳**：按设计尺寸打印透明PLA外壳、空缺盖板、固定卡扣，确认开孔与器件尺寸匹配
2.  **硬件焊接**：按接线图焊接主控、Hub芯片、接口、控制组件、供电单元，焊接完成后测试通断
3.  **组装固定**：将焊接完成的PCB板装入外壳，安装按键、旋钮、开关、屏幕，用卡扣将模块固定在键盘左侧，USB-C公头插入键盘左上角母座
4.  **固件烧录**：给树莓派Pico烧录CircuitPython固件，将核心代码拷贝至Pico磁盘，重启后测试功能
5.  **功能测试**：依次测试按键、旋钮、滚轮、Hub接口、屏幕显示、供电切换功能，确认全部正常运行

---

## 拓展件规划（按字母顺序命名）
| 拓展件名称 | 核心功能 | 适配位置 |
|------------|----------|----------|
| 模块B | 额外4键热插拔按键拓展板 | 模块A上区空缺盖板位 |
| 模块C | RGB灯效拓展板（与键盘灯效同步） | 模块A正面边缘 |
| 模块D | 2.4G/蓝牙无线接收器拓展板 | 模块A背面预留位 |
| 模块E | 迷你摇杆拓展模块（替代滚轮，实现鼠标滚轮/光标控制） | 模块A下区预留位 |

---

## 注意事项
1.  焊接操作请做好防静电措施，避免静电击穿主控、Hub芯片，焊接完成后务必测试通断，避免短路
2.  高速USB 3.0接口请使用屏蔽线布线，与控制信号线垂直走向，避免电磁干扰导致传输降速
3.  切换PD供电模式前，请确认拨动开关档位正确，避免同时接入两路供电导致设备损坏
4.  热插拔轴体请在断电状态下操作，避免带电插拔损坏底座与PCB板
5.  模块安装时请加装硅胶缓冲垫，避免刮花键盘外壳，卡扣固定时请勿用力过猛导致断裂

---

## 常见问题排查
1.  **按键/旋钮无响应**：检查焊接是否虚焊、引脚定义是否与固件匹配、主控是否正常供电
2.  **Hub接口无法识别设备**：检查VL813芯片焊接是否正常、上行USB-C线是否连接到位、供电是否充足
3.  **滚轮误触发**：调整固件防抖延迟时间，检查钮子开关触点是否氧化、接线是否松动
4.  **高速传输降速**：检查屏蔽线是否接地良好、布线是否与控制信号线平行、Hub芯片散热是否正常