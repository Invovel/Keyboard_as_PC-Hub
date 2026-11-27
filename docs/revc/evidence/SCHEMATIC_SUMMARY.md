本页记录第一批修改前的基线。当前实施见[第一批保存结果](../BATCH1.md)。

# 原理图网表差异

Konnect 在本次审查中从根图完整导出层级网表。根图、Main_Control、MAG_Protection、ISO_Debug_Output 的 SHA-256 见 [Audit_Source.json](Audit_Source.json)。

| 网表证据 | 结论 |
|---|---|
| JK1–JK4，KEY1–4 到 GPIO42/41/40/38 | 现有四数字键桥接；Hall、每路 ADC 和 RGB 驱动待实现 |
| JE1 与 ENC_A/B/SW 到 GPIO1/2/39 | 现有编码器预留；EC56 变体的物理端子须签核 |
| SWJ1 有 CCW、CW、COMMON、PUSH 四个未接网 | 已有实体封装；三个输入动作与公共触点尚待闭合 |
| JM1 值为 INMP441 MODULE OPTION DNP | 可追溯历史板接口。独立麦克风小板、电路与装配仍待实施 |
| DRV2605LDGSR ×2、TCA9543APWR 与 HAP_L/R 分路 | JML1/JMR1 的 1/2 分别标为各侧 MOTOR_P/N，保存板尚需真实走线。JH1/JH2 为调试接口；实物马达线束留待导通测量 |
| U102 ADuM3165BRSZ、ISO_GND、ISO_USB_VBUS、J101 | 仍有两个 ISO 电源驱动错误；隔离 DC/DC 路径待补齐 |
| KBD USB2 PROTECTED、KBD_5V_PROTECTED、HUB_P5 D± | 标签及座位存在；主 Hub、分支 Hub、CC/供电/防反灌的完整端点待补齐 |

本地全量输出见 `outputs/pc-hub-revc-20261119/evidence/Connectivity.net`。本次结论取自保存副本，结合真实板级连接和样机结果完成验收。
