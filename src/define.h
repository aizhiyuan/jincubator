#define MAX_MODBUS_LEN 32768

#define MAX_RING_TIME (0x278d00)

#define ON (1)
#define OFF (0)

// 温度异常最大值
#define VAL_MAX_TEMP 5000
#define VAL_MIN_TEMP 0

// 翻蛋时间
#define EGG_FILP_MAX_TIME 3600

// 响铃模式
#define RING_OFF_LED_OFF (0x00)
#define RING_OFF_LED_ON (0x01)
#define RING_ON_LED_ON (0x02)

// 模式状态
#define STATUS_START_SEND (0X01)
#define STATUS_START_RECV (0x02)
#define STATUS_STOP_SENT (0x04)
#define STATUS_STOP_RECV (0x08)
#define STATUS_PRE_SEND (0x10)
#define STARTS_PRE_RECV (0x20)
#define STATUS_CLOSE_ALA_SEND (0x80)
#define STATUS_CLOSE_ALA_RECV (0x100)
#define STATUS_INIT_SEND (0x400)
#define STATUS_INIT_REVV (0x800)
#define STATUS_CHECK_SEND (0x1000)
#define STATUS_CHECK_REVV (0x2000)
// 模块状态
#define STATUS_SYSTEM_TIME (0x01)     // 系统时间线程
#define STATUS_DAMPER (0x02)          // 风门线程
#define STATUS_CONNEL_TEMP (0x04)     // 数据采集-温度
#define STATUS_CONNEL_CO2 (0x08)      // 数据采集-CO2
#define STATUS_BTN_DATA (0x10)        // 测试按钮
#define STATUS_LOGIC_TEMP_MAIN (0x20) // 逻辑控制-主温度
#define STATUS_LOGIC_HUMI (0x40)      // 逻辑控制-湿度
#define STATUS_LOGIC_PRE_TEMP (0x80)  // 逻辑控制-回流温度
#define STATUS_IO_LED (0x100)         // 控制按钮状态
#define STATUS_ALARM (0x200)          // 报警线程

// 报警状态模式
#define ALARM_HIGH_TEMP (0x01)
#define ALARM_LOW_TEMP (0x02)
#define ALARM_HIGH_HUMI (0x04)
#define ALARM_LOW_HUMI (0x08)
#define ALARM_REFLUX_HIGH_TEMPE (0x10)
#define ALARM_REFLUX_LOW_TEMP (0x20)
#define ALARM_CO2 (0x40)
#define ALARM_ABNORMAL_SENSOR (0x80)
#define ALARM_FAN_STOPPED (0x100)
#define ALARM_EGG_FLIPPING (0x200)
#define ALARM_SENSOR_MUTATION (0x400)

// CONNEL_DATA
// logic_control
// 原始IO区
#define DI_INIT 0
#define IO_I1 0   // J8_01 J8_01 电压检测 100V
#define IO_I2 1   // J8_02
#define IO_I3 2   // J8_03 风门全开检测 OPEN
#define IO_I4 3   // J8_04 风门全关检测 CLOSE
#define IO_I5 4   // J8_05 风扇接触器吸合检测 M.SW
#define IO_I6 5   // J8_06 辅助高温报警检测 H.AL
#define IO_I7 6   // J8_07 风扇停止检测 FAN.LS
#define IO_I8 7   // J8_08 门开关检测 DOOR.LS
#define IO_I9 8   // J8_09 DRY 干燥信号
#define IO_I10 9  // J7_01 100V信号检测
#define IO_I11 10 // J7_02
#define IO_I12 11 // J7_03
#define IO_I13 12 // J7_04
#define IO_I14 13 // J7_05
#define IO_I15 14 // J7_06
#define IO_I16 15 // J7_07
#define IO_I17 16 // J9_01、J9_02 电压检测 220V
#define IO_I18 17 // J9_03、J9_04 风扇检测 FAN
#define IO_I19 18 // J9_05、J9_06
#define IO_I20 19 // J9_07、J6_08 翻蛋检测 TURN（输入24V)
#define IO_I21 20 // J5_01
#define IO_I22 21 // J5_02
#define IO_I23 22 // J5_03
#define IO_I24 23 // J5_04

#define DO_INIT 24
#define IO_O1 24 // J10_04 运行灯输出（继电器）
#define IO_O2 25 // J10_05 警报灯输出（继电器）
#define IO_O3 26 // J10_06 主加热输出（SSR）
#define IO_O4 27 // J5_05
#define IO_O5 28 // J5_06
#define IO_O6 29 // J5_07
#define IO_O7 30 // J5_08
#define IO_O8 31 // J5_09

#define IO_O9 32  // J6_04、J6_05 报警输出AL（继电器）
#define IO_O10 33 // J6_03 辅助加热AUX（继电器）
#define IO_O11 34 // J6_02
#define IO_O12 35 // J6_01 风机运行输出（接触器）
#define IO_O13 36 // J1_01 水冷电磁阀2-1 COOL2-1
#define IO_O14 37 // J1_02 水冷电磁阀2-2 COOL2-2
#define IO_O15 38 // J1_03 水冷电磁阀2-3 COOL2-3
#define IO_O16 39 // J1_04 水冷电磁阀2-4 COOL2-4
#define IO_O17 40 // J1_05 冷热水切换阀 W.WARM
#define IO_O18 41 // J1_06 鼓风机 BLO（继电器）
#define IO_O19 42 // J2_01 风门电机开 OPEN
#define IO_O20 43 // J2_02 风门电机关 CLOSE
#define IO_O21 44 // J2_03 水冷电磁阀 COOL
#define IO_O22 45 // J2_04 加湿电磁阀 SPRAY
#define IO_O23 46 // J2_05
#define IO_O24 47 // J2_06

#define DETECT_VOL_100V IO_I1           // 检测100V电压
#define DETECT_DAMPER_FULLY_OPEN IO_I3  // 风门全开检测
#define DETECT_DAMPER_FULLY_CLOSE IO_I4 // 风门全关检测
#define DETECT_FAN_SUCTION IO_I5        // 风扇接触器吸合检测
#define DETECT_AUX_HIGH_TEMP IO_I6      // 辅助高温报警检测
#define DETECT_FAN_STOP IO_I7           // 风扇停止检测
#define DETECT_DOOR_SWITCH IO_I8        // 门开关检测
#define DETECT_VOL_220V IO_I17          // 检测220V电压
#define DETECT_FAN IO_I18               // 风扇检测
#define DETECT_FLIP_EGG IO_I20          // 翻蛋检测（输入24V)

#define CONTROL_INIT IO_O1
#define CONTROL_RUN_LIGHTS IO_O1    // 运行灯输出（继电器）
#define CONTROL_WARN_LIGHT IO_O2    // 警报灯输出（继电器）
#define CONTROL_MAIN_HEAT IO_O3     // 主加热输出（SSR）
#define CONTROL_ELECTRIC_BELL IO_O9 // 响铃输出AL（继电器）
#define CONTROL_AUX_HEAT IO_O10     // 辅助加热AUX（继电器）
#define CONTROL_TEST IO_O11         // 未接设备
#define CONTROL_FAN IO_O12          // 风机运行输出（接触器）
#define CONTROL_COOL2_INIT IO_O12
#define CONTROL_COOL2_1 IO_O13        // 水冷电磁阀2-1 COOL2-1
#define CONTROL_COOL2_2 IO_O14        // 水冷电磁阀2-2 COOL2-2
#define CONTROL_COOL2_3 IO_O15        // 水冷电磁阀2-3 COOL2-3
#define CONTROL_COOL2_4 IO_O16        // 水冷电磁阀2-4 COOL2-4
#define CONTROL_COLD_HOT_WATER IO_O17 // 冷热水切换阀 W.WARM
#define CONTROL_BLOWER IO_O18         // 鼓风机 BLO（继电器）
#define CONTROL_DAMPER_OPEN IO_O19    // 风门电机开 OPEN
#define CONTROL_DAMPER_CLOSE IO_O20   // 风门电机关 CLOSE
#define CONTROL_COOL IO_O21           // 水冷电磁阀 COOL
#define CONTROL_SPRAY IO_O22          // 加湿电磁阀 SPRAY
#define FILP_EGG_ON IO_O23            // 翻蛋左边 COOL
#define FILP_EGG_OFF IO_O24           // 翻蛋右边 SPRAY

// 原始通道值
#define PT_INIT 48
#define PT_AI1 48 // PT100通道值1
#define PT_AI2 49 // PT100通道值2
#define PT_AI3 50 // PT100通道值3
#define PT_AI4 51 // PT100通道值4
#define PT_AI5 52 // PT100通道值5
#define PT_AI6 53 // PT100通道值6

#define NANOPI_GPIO_PG_11 54 // NanoPi GPIO PG-11

#define MODULE_STATUS 60               // 模块状态
#define DAMPER_TIME_LEN_ALL_CLOSE 61   // 风门全关总时长
#define DAMPER_TIME_LEN_ALL_OPEN 62    // 风门全开总时长
#define STATUS_DAMPER_ALL_CLOSE_CAL 63 // 风门全关校准
#define STASUS_DAMPER_ALL_OPEN_CAL 64  // 风门全开校准
#define STATUS_DAMPER_CHECK 65         // 风门校准状态
#define STATUS_DAMPER_ALL_CLOSE_CON 66 // 风门全关控制
#define STATUS_DAMPER_ALL_OPEN_CON 67  // 风门全开控制
#define DAMPER_SETTING_VALUE 68        // 风门设定值
#define DAMPER_SETTING_STATUS 69       // 风门控制状态
#define FIRE_ALARM_MODE 70             // 消警模式状态
#define PID_TEMP_MAIN_STATUS 71        // 主温度PID标志位
#define PID_HUMI_STATUS 72             // 湿度PID标志位
#define ELECTRIC_BELL 73               // 电铃状态
#define ALARM_STATUS_MODE 74           // 报警状态模式
#define ALARM_PT100_TOTAL 75           // 传感器报警
#define ALARM_PT100_ERROR 76           // 传感器突变
#define ALARM_MODULE 77                // 报警状态
#define EGG_FLIPP_INGINTERVAL 78       // 翻蛋运行时间
#define CALIBRATION_MODE 79            // 校准模式

// 风门最大开度和最小开度
#define DAMPER_MAX_LEN 80         // 风门最大长度
#define DAMPER_MIN_LEN 81         // 风门最小长度
#define SENSOR_MUTATION_VALUE 82  // 传感器突变值
#define SENSOR_MUTATION_STATUS 83 // 传感器突状态
#define REFLUX_TEMPERATURE 84     // 回流温度模式
#define RUN_MODE_STATUS 85        // 运行模式
#define RUN_MODE_TIME 86          // 停留时间
#define DAMPER_TIME_OPEN 87       // 风门开单位时间内所需的时间
#define DAMPER_TIME_CLOSE 88      // 风门关单位时间内所需的时间

// 风门优先级标志位
#define STATUS_DAMPER_CHECK_ALL_OPEN (0x01)    // 全关->全开
#define STATUS_DAMPER_CHECK_ALL_CLOSE (0x02)   // 全开->
#define STATUS_DAMPER_CONTROL_ALL_OPEN (0x04)  //
#define STATUS_DAMPER_CONTROL_ALL_CLOSE (0x08) //

// 回流温度模式
#define MODE_REF_TEMP_INIT_START (0x01)
#define MODE_REF_TEMP_INIT_END (0x02)
#define MODE_REF_TEMP_RUN_START (0x04)
#define MODE_REF_TEMP_RUN_END (0x08)
#define MODE_REF_TEMP_STOP_START (0x10)
#define MODE_REF_TEMP_STOP_END (0x20)

#define P_CO2_STATUS 89     // 二氧化碳模式
#define P_PT_WAIT 90        // 数据刷新时间
#define P_PT_PIC 91         // 通道值上下浮动的数值
#define P_EGG_TIME 92       // 翻蛋时间
#define P_FAN_TIME 93       // 翻蛋报警模式（1：正常，0：消警）
#define P_EDIT_CO2_VALUE 94 // 二氧化碳修正值
#define P_SAVE_VALUE 95     // 保存值
// 参数区
#define P_INIT 100
#define P_AO_TP_MAIN 100 // 设定值-温度
#define P_AO_HM 101      // 设定值-湿度
#define P_AO_CO 102      // 设定值-CO2
#define P_AO_AD 103      // 设定值-风门
#define R_AO_TP_RF_INIT 103
#define P_AO_TP_RF1 104    // 设定值-回流温度1
#define P_AO_TP_RF2 105    // 设定值-回流温度2
#define P_AO_TP_RF3 106    // 设定值-回流温度3
#define P_AO_TP_RF4 107    // 设定值-回流温度4
#define P_EDIT_TP_MAIN 108 // 修正值-温度
#define P_EDIT_HM 109      // 修正值-湿度
#define P_EDIT_CO 110      // 修正值-CO2
#define P_EDIT_AD 111      // 修正值-风门
#define P_EDIT_TP_RF1 112  // 修正值-回流温度1
#define P_EDIT_TP_RF2 113  // 修正值-回流温度2
#define P_EDIT_TP_RF3 114  // 修正值-回流温度3
#define P_EDIT_TP_RF4 115  // 修正值-回流温度4

#define P_RTEMP_AH 116  // 高温警报的设定
#define P_RTEMP_AL 117  // 低温警报的设定
#define P_RTEMP_ALO 118 // 低温警报的温度设定模式（0＝绝对温度、1＝相对温度）
#define P_RTEMP_SPH 119 // 设定温度的上限值
#define P_RTEMP_SPL 120 // 设定温度的下限值
#define P_RTEMP_TO 121  // 输出周期
#define P_RTEMP_P 122   // PID控制参数 比例带
#define P_RTEMP_I 123   // PID控制参数 积分
#define P_RTEMP_D 124   // PID控制参数 微分
#define P_RTEMP_OHI 125 // 最高输出值　（％）
#define P_RTEMP_OLO 126 // 最低输出值　（％）
#define P_RTEMP_OER 127 // 警报时的输出值 （％）
#define P_RTEMP_RCO 128 //
#define P_RTEMP_ZQC 129 // 水冷2周期次数
#define P_RTEMP_ZQS 130 // 周期时间
#define P_RTEMP_GZS 131 // 水冷2工作时间
#define P_RTEMP_HYS 132 // 回流温度控制灵敏度

#define P_TEMP_AH 133  // 高温警报的设定
#define P_TEMP_AL 134  // 低温警报的设定
#define P_TEMP_ALO 135 // 低温警报的温度设定模式（0＝绝对温度、1＝相对温度）
#define P_TEMP_SPH 136 // 设定温度的上限值
#define P_TEMP_SPL 137 // 设定温度的下限值
#define P_TEMP_TO 138  // 输出周期
#define P_TEMP_P 139   // PID控制参数 比例带
#define P_TEMP_I 140   // PID控制参数 积分
#define P_TEMP_D 141   // PID控制参数 微分
#define P_TEMP_OHI 142 // 最高输出值　（％）
#define P_TEMP_OLO 143 // 最低输出值　（％）
#define P_TEMP_OER 144 // 警报时的输出值 （％）（传感器异常）

#define P_HUM_AH 145  // 高湿警报的设定
#define P_HUM_AL 146  // 低湿警报的设定
#define P_HUM_ALO 147 // 低湿警报的温度设定模式 （0＝绝对温度、1＝相对温度）
#define P_HUM_SPH 148 // 设定湿度的上限值
#define P_HUM_SPL 149 // 设定湿度的下限值
#define P_HUM_TO 150  // 输出周期
#define P_HUM_P 151   // PID控制参数 比例带
#define P_HUM_I 152   // PID控制参数 积分
#define P_HUM_D 153   // PID控制参数 微分
#define P_HUM_OHI 154 // 最高输出值　（％）
#define P_HUM_OLO 155 // 最低输出值　（％）
#define P_HUM_OER 156 // 警报时的输出值 （％）
#define P_HUM_SPR 157 // 强制中止加湿控制的温度点
#define P_HUM_SPO 158 // 风门开设定值
#define P_HUM_DBO 159 // 风门开度设定值

#define P_HYS 160  // 输出警报的滞后温度
#define P_AUH 161  // 辅助加热动作点 （设定温度的相对值）
#define P_OPE 162  // 风门驱动（开）动作点 （PID输出的%设定）
#define P_CLO 163  // 风门驱动（关）动作点 （PID输出的%设定）
#define P_COL 164  // 水冷动作点 （PID输出的%设定）
#define P_BLO 165  // 鼓风机动作点 （PID输出的%设定）
#define P_HHI 166  // 主加热控制 100％点（PID输出的%设定）
#define P_HLO 167  // 主加热控制 0％点（PID输出的%设定）
#define P_TAU 168  // 辅助加热管最低运转时间（秒）
#define P_TOP 169  // 风门（开）最低运转时间（秒）
#define P_TCL 170  // 风门（关）最低运转时间（秒）
#define P_TCO 171  // 水冷最低运转时间（秒）
#define P_TBL 172  // 鼓风机最低运转时间（秒）
#define P_HH0 173  // 回流温度传感器忘挂
#define P_HH1 174  // 翻蛋异常延时
#define P_HH2 175  // 出雏器高温报警延时
#define P_HH3 176  // 测试运转时间
#define P_HH4 177  // 消毒时间
#define P_HH5 178  // 回流温度控制检测时间
#define P_HH6 179  // 干燥开关最长运行时间
#define P_HH7 180  // 风扇停止警报时间长度
#define P_HH8 181  // 翻蛋警报检测时间
#define P_HH9 182  // 校准时间
#define P_HH10 183 // 翻蛋时间
#define P_HH11 184 // 高湿报警延时时间

#define P_CO2_HYS 185      // 二氧化碳灵敏度
#define P_CO2_AH 186       // 二氧化碳控制风门设定值
#define P_CO2_ZQC 187      // 二氧化碳周期时间
#define P_CO2_GZS 188      // 二氧化碳工作时间
#define P_HUMI_CONTROL 189 // 湿度工作温度差值//control

#define P_EGG_FLIPPING_TIME 190             // 翻蛋时间
#define P_EGG_FLIPPING_DIRECTION 191        // 翻蛋方向 (1为左 0为右)
#define P_EGG_FLIPPING_EXECUTION_STATUS 192 // 翻蛋执行状态

#define EGG_FLIPPING_TRIGGER_STATUS 193     // 翻蛋触发状态
#define EGG_FLIPPING_ACCUMULATION_STATE 194 // 翻蛋累加状态
#define EGG_FLIPPING_CONTROL_STATUS 195     // 翻蛋控制状态

// 运行数据区
#define RUN_INIT 200

#define R_AI_TP_MAIN 200 // 温度
#define R_AI_HM 201      // 湿度
#define R_AI_CO 202      // CO2
#define R_AI_AD 203      // 风门测量值
#define R_AI_TP_RF_INIT 203
#define R_AI_TP_RF1 204 // 回流温度测量值1
#define R_AI_TP_RF2 205 // 回流温度测量值2
#define R_AI_TP_RF3 206 // 回流温度测量值3
#define R_AI_TP_RF4 207 // 回流温度测量值4
#define R_AI_TP_EGG 208 // 翻蛋次数

#define R_INIT_AI_TP_MAIN 210 // 温度
#define R_INIT_AI_HM 211      // 湿度
#define R_INIT_AI_CO 212      // CO2
#define R_INIT_AI_AD 213      // 风门测量值
#define R_INIT_AI_TP_RF_INIT 214
#define R_INIT_AI_TP_RF1 215 // 回流温度测量值1
#define R_INIT_AI_TP_RF2 216 // 回流温度测量值2
#define R_INIT_AI_TP_RF3 217 // 回流温度测量值3
#define R_INIT_AI_TP_RF4 218 // 回流温度测量值4

// 测试按钮
#define R_BTN_TP_MAIN 220  // 主加热按钮
#define R_BTN_TP_AUX 221   // 附加热按钮
#define R_BTN_DR_ON 222    // 开门按钮
#define R_BTN_DR_OFF 223   // 关门按钮
#define R_BTN_HM 224       // 加湿按钮
#define R_BTN_EGG 225      // 翻蛋按钮
#define R_BTN_WC 226       // 水冷按钮
#define R_BTN_WC_RF1 227   // 辅助水冷1按钮
#define R_BTN_WC_RF2 228   // 辅助水冷2按钮
#define R_BTN_WC_RF3 229   // 辅助水冷3按钮
#define R_BTN_WC_RF4 230   // 辅助水冷4按钮
#define R_BTN_WC_PLATE 231 // 压板升按钮
#define R_BTN_WC_FAN 232   // 风扇按钮
#define R_BTN_TIME 233     // 测试时间

//-------------------------------------------------------------------------------------------------
// 同步电机内部控制数据
//-------------------------------------------------------------------------------------------------
// 同步电机地址
#define SYNC_MOTOR_MB_EMAX_FREQ_ADDR 0xA00A             // 控制最大频率地址
#define SYNC_MOTOR_MB_UPPER_LIMIT_FREQ_SOUR_ADDR 0xA00B // 控制上限频率源
#define SYNC_MOTOR_MB_UPPER_LIMIT_FREQ_ADDR 0xA00C      // 控制上限频率
#define SYNC_MOTOR_MB_LOWER_LIMIT_FREQ_ADDR 0xA00E      // 控制下限频率
#define SYNC_MOTOR_MB_RUN_FREQ_ADDR 0x1000              // 控制运行频率地址
#define SYNC_MOTOR_MB_CURRENT_FREQ_ADDR 0x1001          // 读取运行频率地址
#define SYNC_MOTOR_MB_CONTROL_CMD_ADDR 0x2000           // 控制命令地址
#define SYNC_MOTOR_MB_CONTROL_PARA_ADDR 0x3000          // 读取控制器状态地址
#define SYNC_MOTOR_MB_CONTROL_FAULT_ADDR 0x8000         // 读取控制器故障地址
#define SYNC_MOTOR_MB_ERROR_CODE_ADDR 0x8001            // 读取通信错误码地址

// 同步电机控制功能
#define SYNC_MOTOR_MB_CONTROL_JUST_RUN 0x0001    // 正转运行
#define SYNC_MOTOR_MB_CONTROL_BACK_RUN 0x0002    // 反转运行
#define SYNC_MOTOR_MB_CONTROL_FREE_STOP 0x0005   // 自由停机
#define SYNC_MOTOR_MB_CONTROL_DECEL_STOP 0x0006  // 减速停机
#define SYNC_MOTOR_MB_CONTROL_FAULT_RESET 0x0007 // 故障复位

// 同步电机状态
#define SYNC_MOTOR_MB_READ_JUST_RUN 0x0001 // 正转运行
#define SYNC_MOTOR_MB_READ_BACK_RUN 0x0002 // 反转运行
#define SYNC_MOTOR_MB_READ_STOP 0x0003     // 停机

#define S_SYNC_MOTOR_UPPER_LIMIT_FREQ 238 // 上限频率
#define S_SYNC_MOTOR_LOWER_LIMIT_FREQ 239 // 下限频率
#define S_SYNC_MOTOR_CONTROL_CMD 240      // 控制命令
#define S_SYNC_MOTOR_CONTROLRUN_FREQ 241  // 控制运行频率
#define R_SYNC_MOTOR_CONTROL_PARA 242     // 读取控制器状态
#define R_SYNC_MOTOR_CONTROL_FAULT 243    // 读取控制器故障
#define R_SYNC_MOTOR_ERROR_CODE 244       // 读取通信错误码
#define R_SYNC_MOTOR_CONNECT_STATUS 245   // 读取通信状态
#define R_SYNC_MOTOR_CURRENT_FREQ 246     // 读取当前频率
#define R_SYNC_MOTOR_MODE 247             // 电机模式
#define R_SYNC_HATCH_MODE 248             // 孵化模式
// 控制逻辑
#define SYSTEM_INIT 250
#define R_START_SEND 250               // 启动发送地址
#define R_START_RECV 251               // 启动接收地址
#define R_STOP_SEND 252                // 停止发送地址
#define R_STOP_RECV 253                // 停止接收地址
#define R_PRE_SEND 254                 // 预热发送地址
#define R_PRE_RECV 255                 // 预热接收地址
#define R_PRE_STATUS 256               // 预热状态
#define R_CLOSE_ALA_SEND 257           // 消除报警发送地址
#define R_CLOSE_ALA_RECV 258           // 消除报警接收地址
#define R_CLOSE_ALA_TIME 259           // 消除报警时间(s)
#define R_INIT_SEND 260                // 初始化发送地址
#define R_INIT_RECV 261                // 初始化接收地址
#define R_CHECK_SEND 262               // 校准模式发送地址
#define R_CHECK_RECV 263               // 校准模式接收地址
#define R_INIT_TIME_HIGH 264           // 入孵时间-高位
#define R_INIT_TIME_LOW 265            // 入孵时间-低位
#define R_SYSTEM_TIME_HIGH 266         // 系统时间-高位
#define R_SYSTEM_TIME_LOW 267          // 系统时间-低位
#define S_SYSTEM_TIME_HIGH 268         // 设置系统时间-高位
#define S_SYSTEM_TIME_LOW 269          // 设置系统时间-低位
#define R_PRE_START_TIME_HIGH 270      // 预热时间启动时间-高位
#define R_PRE_START_TIME_LOW 271       // 预热时间启动时间-低位
#define R_PRE_STOP_TIME_HIGH 272       // 预热时间停止时间-高位
#define R_PRE_STOP_TIME_LOW 273        // 预热时间停止时间-低位
#define R_RUN_DAY 274                  // 运行天数和小时
#define R_RUN_MAX_DAY 275              // 允许运行最大天数
#define R_RUN_SECOND_HIGH 276          // 运行秒数-高位
#define R_RUN_SECOND_LOW 277           // 运行秒数-低位
#define R_RUN_SECOND_STATUS 278        // 运行秒数-状态
#define R_RUN_SYSTEM_SECOND_STATUS 279 // 系统运行秒数-状态
#define R_RUN_SYSTEM_SECOND_HIGH 280   // 系统运行秒数-高位
#define R_RUN_SYSTEM_SECOND_LOW 281    // 系统运行秒数-低位
#define S_RUN_SYSTEM_SECOND_HIGH 282   // 设置系统运行秒数-高位
#define S_RUN_SYSTEM_SECOND_LOW 283    // 设置系统运行秒数-低位
#define S_RUN_SECOND_HIGH 284          // 设置运行秒数-高位
#define S_RUN_SECOND_LOW 285           // 设置运行秒数-低位
#define R_BTN_DRY_SEND 286             // 干燥按钮
#define R_BTN_DRY_RECV 287             // 干燥按钮
#define R_DRYING_TIMER_HIGH 288        // 干燥计时-高位
#define R_DRYING_TIMER_LOW 289         // 干燥计时-低位
#define S_TIME_RESET_HIGH 290          // 初始化时间-高位
#define S_TIME_RESET_LOW 291           // 初始化时间-低位
#define S_SYNC_MOTOR_BASICS 292        // 风机基础值
#define S_SYNC_MOTOR_STAGE 293         // 风机上升值

#define P_PT1_AO1 300 // 第一个温度
#define P_PT1_AO2 301 // 第二个温度
#define P_PT1_AO3 302 // 第一个通道值
#define P_PT1_AO4 303 // 第二个通道值
#define P_PT1_AO5 304 // 温度差值
#define P_PT2_AO1 305 // 第一个温度
#define P_PT2_AO2 306 // 第二个温度
#define P_PT2_AO3 307 // 第一个通道值
#define P_PT2_AO4 308 // 第二个通道值
#define P_PT2_AO5 309 // 温度差值
#define P_PT3_AO1 310 // 第一个温度
#define P_PT3_AO2 311 // 第二个温度
#define P_PT3_AO3 312 // 第一个通道值
#define P_PT3_AO4 313 // 第二个通道值
#define P_PT3_AO5 314 // 温度差值
#define P_PT4_AO1 315 // 第一个温度
#define P_PT4_AO2 316 // 第二个温度
#define P_PT4_AO3 317 // 第一个通道值
#define P_PT4_AO4 318 // 第二个通道值
#define P_PT4_AO5 319 // 温度差值
#define P_PT5_AO1 320 // 第一个温度
#define P_PT5_AO2 321 // 第二个温度
#define P_PT5_AO3 322 // 第一个通道值
#define P_PT5_AO4 323 // 第二个通道值
#define P_PT5_AO5 324 // 温度差值
#define P_PT6_AO1 325 // 第一个温度
#define P_PT6_AO2 326 // 第二个温度
#define P_PT6_AO3 327 // 第一个通道值
#define P_PT6_AO4 328 // 第二个通道值
#define P_PT6_AO5 329 // 温度差值

#define P_HH0_TIME 400            // HH0
#define P_HH1_TIME 401            // HH1
#define P_HH2_TIME 402            // HH2
#define P_HH3_TIME 403            // HH3
#define P_HH4_TIME 404            // HH4
#define P_HH5_TIME 405            // HH5
#define P_HH6_TIME 406            // HH6
#define P_HH7_TIME 407            // HH7
#define P_HH8_TIME 408            // HH8
#define P_HH9_TIME 409            // HH9
#define P_HH10_TIME 410           // HH10
#define P_FAN_OPEN_TIME 411       // FAN_OPEN
#define P_FAN_CLOSE_TIME 412      // FAN_CLOSE
#define P_CLEAR_ALARM_TIME 413    // CLEAR_ALARM
#define P_COLD_HOT_WATER_TIME 413 // COLD_HOT_WATER

#define ALARM_SUPPRESSION_TIME 415           // 消警时间
#define HIGH_HUMIDITY_ALARM_DELAY_STATUS 416 // 高湿报警延迟状态
#define HIGH_HUMIDITY_ALARM_DELAY_TIME 417   // 高湿报警延迟时间

#define BACKUP_OPERATION_MODE 420                // 备份运行模式
#define CALIBRATION_RUN_TIME 421                 // 校准运行时间
#define BACKUP_PREHEATING_START_TIME_HIGH 422    // 备份预热开启时间-高位
#define BACKUP_PREHEATING_START_TIME_LOW 423     // 备份预热开启时间-低位
#define BACKUP_PREHEATING_SHUTDOWN_TIME_HIGH 424 // 备份预热关闭时间-高位
#define BACKUP_PREHEATING_SHUTDOWN_TIME_LOW 425  // 备份预热关闭时间-低位
#define BACKUP_PRE_STATUS 426                    // 备份预热初始化

#define EGG_FLIPPING_SIGNAL_RINGING_STATUS 430    // 翻蛋信号响铃状态
#define NO_EGG_FLIPPING_SIGNAL_RINGING_STATUS 431 // 没有翻蛋信号响铃状态
#define EGG_FLIPPING_SIGNAL_ALARM_TIME 432        // 翻蛋信号报警时间
#define NO_EGG_FLIPPING_SIGNAL_ALARM_TIME 433     // 没有翻蛋信号报警时间

#define CARBON_DIOXIDE_WARNING_LIGHT 440      // 二氧化碳报警状态
#define CARBON_DIOXIDE_OPERATING_TIME 441     // 二氧化碳运行时间
#define CARBON_DIOXIDE_STOP_TIME 442          // 二氧化碳停止时间
#define CARBON_DIOXIDE_INIT 443               // 二氧化碳初始化
#define CARBON_DIOXIDE_CALIBRATION_STATUS 444 // 二氧化碳校准状态

#define FAN_ALARM_TIME 450         // 风机报警时间
#define ABNORMAL_SENSOR_STATUS 451 // 传感器异常状态

#define AIR_DOOR_OPEN_RUNNING_TIME 460      // 风门开运行时间
#define AIR_DOOR_CLOSING_RUNNING_TIME 461   // 风门关运行时间
#define AIR_DOOR_OPEN_RUNNING_STATUS 462    // 风门开运行状态
#define AIR_DOOR_CLOSING_RUNNING_STATUS 463 // 风门关运行状态

#define THERE_IS_AN_EGG_FLIPPING_SIGNAL_ALARM 501 // 有翻蛋信号报警
#define NO_EGG_FLIPPING_SIGNAL_ALARM 502          // 没有翻蛋信号报警

#define SENSOR_1_ABNORMAL_ALARM 510 // 传感器1异常报警
#define SENSOR_2_ABNORMAL_ALARM 511 // 传感器2异常报警
#define SENSOR_3_ABNORMAL_ALARM 512 // 传感器3异常报警
#define SENSOR_4_ABNORMAL_ALARM 513 // 传感器4异常报警
#define SENSOR_5_ABNORMAL_ALARM 514 // 传感器5异常报警
#define SENSOR_6_ABNORMAL_ALARM 515 // 传感器6异常报警
#define SENSOR_CO2_ALARM 516        // 传感器CO2报警

#define TEST_MAIN_HEAT_MODE 520    // 测试模式-主加热
#define TEST_AUX_HEAT_MODE 521     // 测试模式-辅助加热
#define TEST_DAMPER_OPEN_MODE 522  // 测试模式-风门开
#define TEST_DAMPER_CLOSE_MODE 523 // 测试模式-风门关
#define TEST_SPRAY_MODE 524        // 测试模式-加湿
#define TEST_FILP_EGG_MODE 525     // 测试模式-翻蛋
#define TEST_COOL_MODE 526         // 测试模式-水冷
#define TEST_COOL2_1_MODE 527      // 测试模式-辅助水冷1
#define TEST_COOL2_2_MODE 528      // 测试模式-辅助水冷2
#define TEST_COOL2_3_MODE 529      // 测试模式-辅助水冷3
#define TEST_COOL2_4_MODE 529      // 测试模式-辅助水冷4
#define TEST_BLOWER_MODE 530       // 测试模式-鼓风机
#define TEST_CONTROL_MODE 531      // 测试模式-状态
#define TEST_BTN_TIME 532          // 测试模式时间

#define REFLUX_TEMP_1_DELAY_TIME 540             // 回流温度1延迟时间
#define REFLUX_TEMP_2_DELAY_TIME 541             // 回流温度2延迟时间
#define REFLUX_TEMP_3_DELAY_TIME 542             // 回流温度3延迟时间
#define REFLUX_TEMP_4_DELAY_TIME 543             // 回流温度4延迟时间
#define REFLUX_TEMP_1_WORK_TIME 544              // 回流温度1工作时间
#define REFLUX_TEMP_2_WORK_TIME 545              // 回流温度2工作时间
#define REFLUX_TEMP_3_WORK_TIME 546              // 回流温度3工作时间
#define REFLUX_TEMP_4_WORK_TIME 547              // 回流温度4工作时间
#define RETURN_TEMP_1_STOP_TIME 548              // 回流温度1停止时间
#define RETURN_TEMP_2_STOP_TIME 549              // 回流温度2停止时间
#define RETURN_TEMP_3_STOP_TIME 550              // 回流温度3停止时间
#define RETURN_TEMP_4_STOP_TIME 551              // 回流温度4停止时间
#define RETURN_TEMP_1_WORK_CYCLE 552             // 回流温度1工作周期
#define RETURN_TEMP_2_WORK_CYCLE 553             // 回流温度2工作周期
#define RETURN_TEMP_3_WORK_CYCLE 554             // 回流温度3工作周期
#define RETURN_TEMP_4_WORK_CYCLE 555             // 回流温度4工作周期
#define RETURN_TEMP_1_HOT_WATER_OPERAT_INIT 556  // 回流温度1热水工作初始化
#define RETURN_TEMP_2_HOT_WATER_OPERAT_INIT 557  // 回流温度2热水工作初始化
#define RETURN_TEMP_3_HOT_WATER_OPERAT_INIT 558  // 回流温度3热水工作初始化
#define RETURN_TEMP_4_HOT_WATER_OPERAT_INIT 559  // 回流温度4热水工作初始化
#define REFLOW_TEMP_1_COLD_WATER_OPERAT_INIT 560 // 回流温度1冷水工作初始化
#define REFLOW_TEMP_2_COLD_WATER_OPERAT_INIT 561 // 回流温度2冷水工作初始化
#define REFLOW_TEMP_3_COLD_WATER_OPERAT_INIT 562 // 回流温度3冷水工作初始化
#define REFLOW_TEMP_4_COLD_WATER_OPERAT_INIT 563 // 回流温度4冷水工作初始化

#define HIGH_TEMP_ALARM_VALUE 570                   // 高温报警值
#define LOW_TEMP_ALARM_VALUE 571                    // 低温报警值
#define HIGH_HUMI_ALARM_VALUE 572                   // 高湿报警值
#define LOW_HUMI_ALARM_VALUE 573                    // 低湿报警值
#define HIGH_TEMP_ALARM_VALUE_FOR_REFLUX_TEMP_1 574 // 回流温度1高温报警值
#define LOW_TEMP_ALARM_VALUE_FOR_REFLUX_TEMP_1 575  // 回流温度1低温报警值
#define HIGH_TEMP_ALARM_VALUE_FOR_REFLUX_TEMP_2 576 // 回流温度2高温报警值
#define LOW_TEMP_ALARM_VALUE_FOR_REFLUX_TEMP_2 577  // 回流温度2低温报警值
#define HIGH_TEMP_ALARM_VALUE_FOR_REFLUX_TEMP_3 578 // 回流温度3高温报警值
#define LOW_TEMP_ALARM_VALUE_FOR_REFLUX_TEMP_3 579  // 回流温度3低温报警值
#define HIGH_TEMP_ALARM_VALUE_FOR_REFLUX_TEMP_4 580 // 回流温度4高温报警值
#define LOW_TEMP_ALARM_VALUE_FOR_REFLUX_TEMP_4 581  // 回流温度4低温报警值

#define HIGH_TEMP_ALARM_STATUS 582                // 高温报警状态
#define LOW_TEMP_ALARM_STATUS 583                 // 低温报警状态
#define HIGH_HUMI_ALARM_STATUS 584                // 高湿报警状态
#define LOW_HUMI_ALARM_STATUS 585                 // 低湿报警状态
#define RETURN_TEMPE_HIGH_TEMP_ALARM_STATUS_1 586 // 回流温度高温报警值1
#define RETURN_TEMP_LOW_TEMP_ALARM_STATUS_1 587   // 回流温度低温报警值1
#define RETURN_TEMPE_HIGH_TEMP_ALARM_STATUS_2 588 // 回流温度高温报警值2
#define RETURN_TEMP_LOW_TEMP_ALARM_STATUS_2 589   // 回流温度低温报警值2
#define RETURN_TEMPE_HIGH_TEMP_ALARM_STATUS_3 590 // 回流温度高温报警值3
#define RETURN_TEMP_LOW_TEMP_ALARM_STATUS_3 591   // 回流温度低温报警值3
#define RETURN_TEMPE_HIGH_TEMP_ALARM_STATUS_4 592 // 回流温度高温报警值4
#define RETURN_TEMP_LOW_TEMP_ALARM_STATUS_4 593   // 回流温度低温报警值4

#define RETURN_TEMP_HIGH_TEMP_ALARM_TIME 594    // 回流温度高温报警时间
#define RETURN_TEMP_LOW_TEMP_ALARM_TIME 595     // 回流温度低温报警时间
#define RETURN_TEMP_HIGH_TEMP_ALARM_STATUS 596  // 回流温度高温设置状态
#define RETURN_TEMP_LOW_TEMP_ALARM_STATUS 597   // 回流温度低温设置状态
#define RETURN_TEMP_TOTAL_TEMP_ALARM_STATUS 598 // 回流温度总状态
// 指示灯状态
#define OUT_ST_FAN_STATUS 600      // 风扇
#define OUT_ST_TP_MAIN_STATUS 601  // 主温度加热指示灯
#define OUT_ST_TP_AID_STATUS 602   // 辅助加热状态灯
#define OUT_ST_HM_STATUS 603       // 加湿状态灯
#define OUT_ST_EGG_STATUS 604      // 翻蛋状态灯
#define OUT_ST_AD_OPEN_STATUS 605  // 风门开启指示灯
#define OUT_ST_AD_CLOSE_STATUS 606 // 风门关闭指示灯
#define OUT_ST_WT_CL_STATUS 607    // 水冷指示灯
#define OUT_ST_TP_RF1_STATUS 608   // 水冷电磁阀2-1指示灯
#define OUT_ST_TP_RF2_STATUS 609   // 水冷电磁阀2-2指示灯
#define OUT_ST_TP_RF3_STATUS 610   // 水冷电磁阀2-3指示灯
#define OUT_ST_TP_RF4_STATUS 611   // 水冷电磁阀2-4指示灯

#define OUT_ST_AD_ALL_CLOSE_STATUS 612 // 风门全关指示灯模式为OFF
#define OUT_ST_AD_ALL_OPEN_STATUS 613  // 风门全关指示灯模式为OFF
#define OUT_ST_EGG_DIRECTION 614       // 翻蛋方向

// 报警灯状态 direction
#define OUT_ST_INIT_ALARM 620
#define OUT_ST_TP_HIGH_ALARM 620         // 高温报警灯
#define OUT_ST_TP_LOW_ALARM 621          // 低温报警灯
#define OUT_ST_HM_HIGH_ALARM 622         // 高湿报警灯
#define OUT_ST_HM_LOW_ALARM 623          // 低湿报警灯
#define OUT_ST_FAN_ALARM 624             // 风扇报警灯
#define OUT_ST_REF_TEMP_ALARM 625        // 回流温度高低温报警灯
#define OUT_ST_SENSOR_ABNORMAL_ALARM 626 // 传感器异常报警灯
#define OUT_ST_EGG_ALARM 627             // 翻蛋异常报警灯

#define OUT_ST_CO2_ALARM 628             // CO2报警灯
#define OUT_ST_SENSOR_MUTATION_ALARM 629 // 传感器突变报警灯

#define OUT_ST_DOOR_SWITCH_ALARM 630 // 门开关报警

// filter
#define FILTER_PID_TEMP_ARR 700 // PID过滤值
#define FILTER_PID_HUMI_ARR 800 // PID过滤值

#define PID_TEMP_MODE_STATUS 900         // PID状态
#define PID_TEMP_SUM 902                 // PID累加
#define PID_TEMP_VALUE 904               // PID 温度 原始值
#define PID_TEMP_OLD_FILTER_VALUE 906    // PID 温度 旧的过滤值
#define PID_TEMP_NEW_FILTER_VALUE 908    // PID 温度 新的过滤值
#define PID_TEMP_MAIN_HEATER 910         // PID 温度 新的主加热
#define PID_PERIOD_COUNT_MAIN_HEATER 912 // PID 周期计数主加热
#define PID_ON_COUNT_MAIN_HEATER 914     // PID 开启计数主加热
#define PID_ON_COUNT_COOL 916            // PID 开启计数水冷
#define PID_ON_COUNT_BLOWER 918          // PID 开启计数鼓风机
#define PID_ON_COUNT_DB_OPEN 920         // PID 开启计数风门开
#define PID_ON_COUNT_DB_CLOSE 922        // PID 开启计数风门关
#define PID_TEMP_COOL 924                // PID 温度 水冷
#define PID_TEMP_BLOWER 926              // PID 温度 鼓风机
#define PID_TEMP_DB_OPEN 928             // PID 温度 风门开
#define PID_TEMP_DB_CLOSE 930            // PID 温度 风门关
#define PID_HUMI_MODE_STATUS 932         // PID 状态
#define PID_HUMI_SUM 934                 // PID 累加
#define PID_HUMI_VALUE 936               // PID 湿度 值
#define PID_HUMI_OLD_HUMIDIFICATION 938  // PID 旧值 湿度 加湿
#define PID_HUMI_NEW_HUMIDIFICATION 940  // PID 新值 湿度 加湿
#define PID_PERIOD_COUNT_HUMI 942        // PID 周期计数加湿
#define PID_ON_COUNT_HUMI 944            // PID 开启计数加湿

#define PID_AD_VALUE 945                         // PID风门值
#define PID_AD_OPEN_STATUS 946                   // PID风门开控制状态
#define PID_AD_CLOSE_STATUS 947                  // PID风门开控制状态
#define CONTROL_DB_OPEN_CALIBRATION_STATUS 948   // 风门开到位校准状态
#define CONTROL_DB_CLISE_CALIBRATION_STATUS 949  // 风门开到位校准状态
#define CONTROL_DB_CLOSE_TO_OPEN_TIME 950        // 风门从关到开的时间
#define CONTROL_DB_OPEN_TO_CLOSE_TIME 951        // 风门从开到关的时间
#define CONTROL_DB_CLOSE_TO_OPEN_TIME_STATUS 952 // 风门从关到开的时间状态
#define CONTROL_DB_OPEN_TO_CLOSE_TIME_STATUS 953 // 风门从开到关的时间状态

// 湿度
// #define PID_TEMP_P 960
// #define PID_TEMP_I 961
// #define PID_TEMP_D 962
#define PID_TEMP_PARA 963
// #define PID_TEMP_TO 964

// #define PID_HUMI_P 965
// #define PID_HUMI_I 966
// #define PID_HUMI_D 967
#define PID_HUMI_PARA 968

#define PID_LOOP 969
// 设定参数初始化
#define S_PRE_TP_MAIN 1 // 预热值-温度
#define S_PRE_HM 2      // 预热值-湿度
#define S_PRE_CO 3      // 预热值-CO2
#define S_PRE_AD 4      // 预热值-风门
#define S_PRE_TP_RF1 5  // 预热值-回流温度1
#define S_PRE_TP_RF2 6  // 预热值-回流温度2
#define S_PRE_TP_RF3 7  // 预热值-回流温度3
#define S_PRE_TP_RF4 8  // 预热值-回流温度4

// 预热参数
#define P_PRE_TP_MAIN 1001 // 预热值-温度
#define P_PRE_HM 1101      // 预热值-湿度
#define P_PRE_CO 1201      // 预热值-CO2
#define P_PRE_AD 1301      // 预热值-风门
#define P_PRE_TP_RF1 1401  // 预热值-回流温度1
#define P_PRE_TP_RF2 1501  // 预热值-回流温度2
#define P_PRE_TP_RF3 1601  // 预热值-回流温度3
#define P_PRE_TP_RF4 1701  // 预热值-回流温度4

// 回流温度 热水天数
#define P_PRE_TP_MAIN_DAY 1000 // 预热天数-温度
#define P_PRE_HM_DAY 1100      // 预热天数-湿度
#define P_PRE_CO_DAY 1200      // 预热天数-CO2
#define P_PRE_AD_DAY 1300      // 预热天数-风门
#define P_PRE_TP_RF1_DAY 1400  // 热水天数-回流温度1
#define P_PRE_TP_RF2_DAY 1500  // 热水天数-回流温度2
#define P_PRE_TP_RF3_DAY 1600  // 热水天数-回流温度3
#define P_PRE_TP_RF4_DAY 1700  // 热水天数-回流温度4

// 预设值天数，默认为30天
#define P_SET_DAY 30 // 预设值天数，默认为30天

// 30天起始预设参数
#define P_SET_TP_MAIN_DAY 1002 // 预设值1-温度-天数小时
#define P_SET_TP_MAIN_VAL 1003 // 预设值1-温度-数值
#define P_SET_HM_DAY 1102      // 预设值1-湿度-天数小时
#define P_SET_HM_VAL 1103      // 预设值1-湿度-数值
#define P_SET_CO_DAY 1202      // 预设值1-CO2-天数小时
#define P_SET_CO_VAL 1203      // 预设值1-CO2-数值
#define P_SET_AD_DAY 1302      // 预设值1-风门-天数小时
#define P_SET_AD_VAL 1303      // 预设值1-风门-数值
#define P_SET_TP_RF1_DAY 1402  // 预设值1-回流温度1-天数小时
#define P_SET_TP_RF1_VAL 1403  // 预设值1-回流温度1-数值
#define P_SET_TP_RF2_DAY 1502  // 预设值1-回流温度2-天数小时
#define P_SET_TP_RF2_VAL 1503  // 预设值1-回流温度2-数值
#define P_SET_TP_RF3_DAY 1602  // 预设值1-回流温度3-天数小时
#define P_SET_TP_RF3_VAL 1603  // 预设值1-回流温度3-数值
#define P_SET_TP_RF4_DAY 1702  // 预设值1-回流温度4-天数小时
#define P_SET_TP_RF4_VAL 1703  // 预设值1-回流温度4-数值
