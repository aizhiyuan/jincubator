#include "jincubator.h"

//----------------------------------------------------------------------------------------------------------
// 预定义
//----------------------------------------------------------------------------------------------------------
// 两天时间
const int TWO_DAY_SECONDS = (60 * 60 * 24 * 2);
short g_us_pid_temp_arr[15] = {0}; // PID容器
unsigned short g_sync_data[65535];
char *g_filename = NULL;
char g_filenaem_ini[256] = {0};
int g_fan_status = 0;

// int pid_temp_sum_en = 0;           // 温度PID积分项
// int pid_humi_sum_en = 0;           // 湿度PID积分项

//----------------------------------------------------------------------------------------------------------
// 全局变量-ZLOG变量
//----------------------------------------------------------------------------------------------------------
zlog_category_t *g_zlog_zc = NULL;

//----------------------------------------------------------------------------------------------------------
// 全局变量-INI配置参数
//----------------------------------------------------------------------------------------------------------
struct incubator_t g_st_jincubator;

//----------------------------------------------------------------------------------------------------------
// 全局变量-共享映射地址
//----------------------------------------------------------------------------------------------------------
unsigned short *shm_out = NULL; // 写入共享内存指针
unsigned short *shm_old = NULL; // 历史写入共享内存指针

//---------------------------------------------------------
// 获取参数
//---------------------------------------------------------
short get_val(unsigned short index)
{
    return shm2short(&shm_out[index]);
}

short get_old(unsigned short index)
{
    return shm2short(&shm_old[index]);
}

unsigned short get_uval(unsigned short index)
{
    return shm2ushort(&shm_out[index]);
}

unsigned short get_uold(unsigned short index)
{
    return shm2ushort(&shm_old[index]);
}

float get_fval(unsigned short index)
{
    return shm2float(&shm_out[index]);
}

double get_dval(unsigned short index)
{
    return shm2double(&shm_out[index]);
}

short get_x10_val(unsigned short index)
{
    return pidshm2short(&shm_out[index]);
}

short get_e10_val(unsigned short index)
{
    return shm2shortpid(&shm_out[index]);
}

int get_int(unsigned short index)
{
    int m_time;
    // 获取设置时间
    memcpy(&m_time, &shm_out[index], sizeof(int));
    dcba(&m_time, sizeof(int));
    return m_time;
}

//---------------------------------------------------------
// 设置参数
//---------------------------------------------------------
void set_val(unsigned short index, short data)
{
    shm_out[index] = short2shm(data);
}
void set_old(unsigned short index, short data)
{
    shm_old[index] = short2shm(data);
}
void set_uval(unsigned short index, unsigned short data)
{
    shm_out[index] = ushort2shm(data);
}
void set_uold(unsigned short index, unsigned short data)
{
    shm_old[index] = ushort2shm(data);
}
void set_fval(unsigned short index, float data)
{
    shm_out[index] = float2shm(data);
}
void set_fold(unsigned short index, float data)
{
    shm_old[index] = float2shm(data);
}
void set_dval(unsigned short index, double data)
{
    double2shm(&shm_out[index], data);
}
void set_dold(unsigned short index, double data)
{
    double2shm(&shm_old[index], data);
}
void mem_val(unsigned short index, void *data, unsigned short len)
{
    memcpy((&shm_out[index]), data, len);
}
void set_int(unsigned short index, int data)
{
    int m_time = data;
    dcba(&m_time, sizeof(int));
    memcpy(&shm_out[index], &m_time, sizeof(int));
}

//----------------------------------------------------------------------------------------------------------
// INI文件初始化
//----------------------------------------------------------------------------------------------------------
void init_ini_file(const char *ini_path, struct incubator_t *st_incubator)
{
    char date_str[256];
    print_current_time(date_str, sizeof(date_str));
    xini_file_t ini(ini_path);
    strcpy(st_incubator->conf_zlog_file, ini["GENERAL SETTING"]["LOG_CONF_FILE"]);
    fprintf(stdout, "%s %-40sJLOG文件:%s\n", date_str, "[init_ini_file]", st_incubator->conf_zlog_file);
    strcpy(st_incubator->conf_sysc_file, ini["GENERAL SETTING"]["SYNC_CONF_FILE"]);
    fprintf(stdout, "%s %-40s同步文件:%s\n", date_str, "[init_ini_file]", st_incubator->conf_sysc_file);
    strcpy(st_incubator->co2_serial_device, ini["GENERAL SETTING"]["CO2_SERIAL_DEVICE"]);
    fprintf(stdout, "%s %-40s串口设备1:%s\n", date_str, "[init_ini_file]", st_incubator->co2_serial_device);
    strcpy(st_incubator->motor_serial_device, ini["GENERAL SETTING"]["MOTOR_SERIAL_DEVICE"]);
    fprintf(stdout, "%s %-40s串口设备2:%s\n", date_str, "[init_ini_file]", st_incubator->motor_serial_device);

    st_incubator->shmem_input_key = ini["GENERAL SETTING"]["SHMEM_INPUT_KEY"];
    fprintf(stdout, "%s %-40s输入共享内存KEY:%d\n", date_str, "[init_ini_file]", st_incubator->shmem_input_key);
    st_incubator->shmem_output_key = ini["GENERAL SETTING"]["SHMEM_OUTPUT_KEY"];
    fprintf(stdout, "%s %-40s输出共享内存KEY:%d\n", date_str, "[init_ini_file]", st_incubator->shmem_output_key);
    st_incubator->shmem_record_len = ini["GENERAL SETTING"]["SHMEM_RECORD_LEN"];
    fprintf(stdout, "%s %-40s申请共享内存长度:%d\n", date_str, "[init_ini_file]", st_incubator->shmem_record_len);
    st_incubator->shmem_record_num = ini["GENERAL SETTING"]["SHMEM_RECORD_NUM"];
    fprintf(stdout, "%s %-40s申请共享内存长度:%d\n", date_str, "[init_ini_file]", st_incubator->shmem_record_num);

    strcpy(st_incubator->db_setting_ip, ini["GENERAL SETTING"]["DB_SETTING_IP"]);
    fprintf(stdout, "%s %-40s数据库路径/域名:%s\n", date_str, "[init_ini_file]", st_incubator->db_setting_ip);
    strcpy(st_incubator->db_setting_user, ini["GENERAL SETTING"]["DB_SETTING_USER"]);
    fprintf(stdout, "%s %-40s用户名:%s\n", date_str, "[init_ini_file]", st_incubator->db_setting_user);
    strcpy(st_incubator->db_setting_passwd, ini["GENERAL SETTING"]["DB_SETTING_PASSWD"]);
    fprintf(stdout, "%s %-40s密码:%s\n", date_str, "[init_ini_file]", st_incubator->db_setting_passwd);
    strcpy(st_incubator->db_setting_name, ini["GENERAL SETTING"]["DB_SETTING_NAME"]);
    fprintf(stdout, "%s %-40s数据库名称:%s\n", date_str, "[init_ini_file]", st_incubator->db_setting_name);

    st_incubator->pt1.pt_0_temp = ini["GENERAL SETTING"]["PT1_0_TEMP"];
    fprintf(stdout, "%s %-40sPT100-1 温度0:%d\n", date_str, "[init_ini_file]", st_incubator->pt1.pt_0_temp);
    st_incubator->pt1.pt_40_temp = ini["GENERAL SETTING"]["PT1_40_TEMP"];
    fprintf(stdout, "%s %-40sPT100-1 温度40:%d\n", date_str, "[init_ini_file]", st_incubator->pt1.pt_40_temp);
    st_incubator->pt1.pt_0_channel = ini["GENERAL SETTING"]["PT1_0_CHANNEL"];
    fprintf(stdout, "%s %-40sPT100-1 通道值0:%d\n", date_str, "[init_ini_file]", st_incubator->pt1.pt_0_channel);
    st_incubator->pt1.pt_40_channel = ini["GENERAL SETTING"]["PT1_40_CHANNEL"];
    fprintf(stdout, "%s %-40sPT100-1 通道值40:%d\n", date_str, "[init_ini_file]", st_incubator->pt1.pt_40_channel);

    st_incubator->pt2.pt_0_temp = ini["GENERAL SETTING"]["PT2_0_TEMP"];
    fprintf(stdout, "%s %-40sPT100-2 温度0:%d\n", date_str, "[init_ini_file]", st_incubator->pt2.pt_0_temp);
    st_incubator->pt2.pt_40_temp = ini["GENERAL SETTING"]["PT2_40_TEMP"];
    fprintf(stdout, "%s %-40sPT100-2 温度40:%d\n", date_str, "[init_ini_file]", st_incubator->pt2.pt_40_temp);
    st_incubator->pt2.pt_0_channel = ini["GENERAL SETTING"]["PT2_0_CHANNEL"];
    fprintf(stdout, "%s %-40sPT100-2 通道值0:%d\n", date_str, "[init_ini_file]", st_incubator->pt2.pt_0_channel);
    st_incubator->pt2.pt_40_channel = ini["GENERAL SETTING"]["PT2_40_CHANNEL"];
    fprintf(stdout, "%s %-40sPT100-2 通道值40:%d\n", date_str, "[init_ini_file]", st_incubator->pt2.pt_40_channel);

    st_incubator->pt3.pt_0_temp = ini["GENERAL SETTING"]["PT3_0_TEMP"];
    fprintf(stdout, "%s %-40sPT100-3 温度0:%d\n", date_str, "[init_ini_file]", st_incubator->pt3.pt_0_temp);
    st_incubator->pt3.pt_40_temp = ini["GENERAL SETTING"]["PT3_40_TEMP"];
    fprintf(stdout, "%s %-40sPT100-3 温度40:%d\n", date_str, "[init_ini_file]", st_incubator->pt3.pt_40_temp);
    st_incubator->pt3.pt_0_channel = ini["GENERAL SETTING"]["PT3_0_CHANNEL"];
    fprintf(stdout, "%s %-40sPT100-3 通道值0:%d\n", date_str, "[init_ini_file]", st_incubator->pt3.pt_0_channel);
    st_incubator->pt3.pt_40_channel = ini["GENERAL SETTING"]["PT3_40_CHANNEL"];
    fprintf(stdout, "%s %-40sPT100-3 通道值40:%d\n", date_str, "[init_ini_file]", st_incubator->pt3.pt_40_channel);

    st_incubator->pt4.pt_0_temp = ini["GENERAL SETTING"]["PT4_0_TEMP"];
    fprintf(stdout, "%s %-40sPT100-4 温度0:%d\n", date_str, "[init_ini_file]", st_incubator->pt4.pt_0_temp);
    st_incubator->pt4.pt_40_temp = ini["GENERAL SETTING"]["PT4_40_TEMP"];
    fprintf(stdout, "%s %-40sPT100-4 温度40:%d\n", date_str, "[init_ini_file]", st_incubator->pt4.pt_40_temp);
    st_incubator->pt4.pt_0_channel = ini["GENERAL SETTING"]["PT4_0_CHANNEL"];
    fprintf(stdout, "%s %-40sPT100-4 通道值0:%d\n", date_str, "[init_ini_file]", st_incubator->pt4.pt_0_channel);
    st_incubator->pt4.pt_40_channel = ini["GENERAL SETTING"]["PT4_40_CHANNEL"];
    fprintf(stdout, "%s %-40sPT100-4 通道值40:%d\n", date_str, "[init_ini_file]", st_incubator->pt4.pt_40_channel);

    st_incubator->pt5.pt_0_temp = ini["GENERAL SETTING"]["PT5_0_TEMP"];
    fprintf(stdout, "%s %-40sPT100-5 温度0:%d\n", date_str, "[init_ini_file]", st_incubator->pt5.pt_0_temp);
    st_incubator->pt5.pt_40_temp = ini["GENERAL SETTING"]["PT5_40_TEMP"];
    fprintf(stdout, "%s %-40sPT100-5 温度40:%d\n", date_str, "[init_ini_file]", st_incubator->pt5.pt_40_temp);
    st_incubator->pt5.pt_0_channel = ini["GENERAL SETTING"]["PT5_0_CHANNEL"];
    fprintf(stdout, "%s %-40sPT100-5 通道值0:%d\n", date_str, "[init_ini_file]", st_incubator->pt5.pt_0_channel);
    st_incubator->pt5.pt_40_channel = ini["GENERAL SETTING"]["PT5_40_CHANNEL"];
    fprintf(stdout, "%s %-40sPT100-5 通道值40:%d\n", date_str, "[init_ini_file]", st_incubator->pt5.pt_40_channel);

    st_incubator->pt6.pt_0_temp = ini["GENERAL SETTING"]["PT6_0_TEMP"];
    fprintf(stdout, "%s %-40sPT100-6 温度0:%d\n", date_str, "[init_ini_file]", st_incubator->pt6.pt_0_temp);
    st_incubator->pt6.pt_40_temp = ini["GENERAL SETTING"]["PT6_40_TEMP"];
    fprintf(stdout, "%s %-40sPT100-6 温度40:%d\n", date_str, "[init_ini_file]", st_incubator->pt6.pt_40_temp);
    st_incubator->pt6.pt_0_channel = ini["GENERAL SETTING"]["PT6_0_CHANNEL"];
    fprintf(stdout, "%s %-40sPT100-6 通道值0:%d\n", date_str, "[init_ini_file]", st_incubator->pt6.pt_0_channel);
    st_incubator->pt6.pt_40_channel = ini["GENERAL SETTING"]["PT6_40_CHANNEL"];
    fprintf(stdout, "%s %-40sPT100-6 通道值40:%d\n", date_str, "[init_ini_file]", st_incubator->pt6.pt_40_channel);
}

//----------------------------------------------------------------------------------------------------------
// ZLOG日志初始化
//----------------------------------------------------------------------------------------------------------
int init_zlog_file(const char *conf_zlog_file, const char *process_name)
{
    char date_str[256];
    print_current_time(date_str, sizeof(date_str));
    fprintf(stderr, "%s %-40s配置文件: %s, 程序名称：%s\n", date_str, "[init_zlog_file]", conf_zlog_file, process_name);
    int i_ret = zlog_init(conf_zlog_file);
    if (i_ret)
    {
        fprintf(stderr, "%s %-40s打开日志配置文件: %s失败!\n", date_str, "[init_zlog_file]", process_name);
        return -1;
    }

    fprintf(stdout, "%s %-40s打开日志配置文件: %s成功!\n", date_str, "[init_zlog_file]", process_name);
    g_zlog_zc = zlog_get_category(process_name);
    if (!(g_zlog_zc))
    {
        fprintf(stderr, "%s %-40s日志初始化失败!\n", date_str, "[init_zlog_file]");
        zlog_fini();
        return -2;
    }
    zlog_info(g_zlog_zc, "%-40s日志初始化成功!", "[init_zlog_file]");
    return 0;
}

//----------------------------------------------------------------------------------------------------------
// 运行灯控制
//----------------------------------------------------------------------------------------------------------
void control_run_light(int status)
{
    set_val(CONTROL_RUN_LIGHTS, status);
}

//----------------------------------------------------------------------------------------------------------
// 报警灯控制
//----------------------------------------------------------------------------------------------------------
void control_warn_light(int status)
{
    set_val(CONTROL_WARN_LIGHT, status);
}

//----------------------------------------------------------------------------------------------------------
// 响铃控制
//----------------------------------------------------------------------------------------------------------
void control_electric_light(int status)
{
    set_val(CONTROL_ELECTRIC_BELL, status);
}

//----------------------------------------------------------------------------------------------------------
// 主加热控制
//----------------------------------------------------------------------------------------------------------
void control_main_heat(int status)
{
    // 测试模式优先
    if (get_val(TEST_MAIN_HEAT_MODE))
    {
        return;
    }

    if (get_val(SENSOR_1_ABNORMAL_ALARM))
    {
        set_val(CONTROL_MAIN_HEAT, OFF);
        return;
    }

    // 高温报警
    if (get_val(OUT_ST_TP_HIGH_ALARM))
    {
        set_val(CONTROL_MAIN_HEAT, OFF);
        return;
    }

    if ((g_fan_status == 1) && ((get_val(R_SYNC_MOTOR_CONTROL_PARA) != 1) && (get_val(R_SYNC_MOTOR_CONTROL_PARA) != 2)))
    {
        set_val(CONTROL_MAIN_HEAT, OFF);
        return;
    }

    set_val(CONTROL_MAIN_HEAT, status);
}

//----------------------------------------------------------------------------------------------------------
// 辅助加热控制
//----------------------------------------------------------------------------------------------------------
void control_aux_heat(int status)
{
    // 测试模式优先
    if (get_val(TEST_AUX_HEAT_MODE))
    {
        return;
    }

    // 高温报警
    if (get_val(OUT_ST_TP_HIGH_ALARM))
    {
        set_val(CONTROL_AUX_HEAT, OFF);
        return;
    }

    if ((g_fan_status == 1) && ((get_val(R_SYNC_MOTOR_CONTROL_PARA) != 1) && (get_val(R_SYNC_MOTOR_CONTROL_PARA) != 2)))
    {
        set_val(CONTROL_AUX_HEAT, OFF);
        return;
    }

    set_val(CONTROL_AUX_HEAT, status);
}

//----------------------------------------------------------------------------------------------------------
// 风机运行控制
//----------------------------------------------------------------------------------------------------------
void control_fan(int status)
{
    set_val(CONTROL_FAN, status);
}

//----------------------------------------------------------------------------------------------------------
// 水冷电磁阀2-1
//----------------------------------------------------------------------------------------------------------
void control_cool2_1(int status)
{
    // 测试模式优先
    if (get_val(TEST_COOL2_1_MODE))
    {
        return;
    }

    if (get_val(SENSOR_3_ABNORMAL_ALARM))
    {
        return;
    }

    if ((g_fan_status == 1) && ((get_val(R_SYNC_MOTOR_CONTROL_PARA) != 1) && (get_val(R_SYNC_MOTOR_CONTROL_PARA) != 2)))
    {
        set_val(CONTROL_COOL2_1, OFF);
        return;
    }

    set_val(CONTROL_COOL2_1, status);
}

//----------------------------------------------------------------------------------------------------------
// 水冷电磁阀2-2
//----------------------------------------------------------------------------------------------------------
void control_cool2_2(int status)
{
    // 测试模式优先
    if (get_val(TEST_COOL2_2_MODE))
    {
        return;
    }

    if (get_val(SENSOR_4_ABNORMAL_ALARM))
    {
        return;
    }

    if ((g_fan_status == 1) && ((get_val(R_SYNC_MOTOR_CONTROL_PARA) != 1) && (get_val(R_SYNC_MOTOR_CONTROL_PARA) != 2)))
    {
        set_val(CONTROL_COOL2_2, OFF);
        return;
    }

    set_val(CONTROL_COOL2_2, status);
}

//----------------------------------------------------------------------------------------------------------
// 水冷电磁阀2-3
//----------------------------------------------------------------------------------------------------------
void control_cool2_3(int status)
{
    // 测试模式优先
    if (get_val(TEST_COOL2_3_MODE))
    {
        return;
    }

    if (get_val(SENSOR_5_ABNORMAL_ALARM))
    {
        return;
    }

    if ((g_fan_status == 1) && ((get_val(R_SYNC_MOTOR_CONTROL_PARA) != 1) && (get_val(R_SYNC_MOTOR_CONTROL_PARA) != 2)))
    {
        set_val(CONTROL_COOL2_3, OFF);
        return;
    }

    set_val(CONTROL_COOL2_3, status);
}

//----------------------------------------------------------------------------------------------------------
// 水冷电磁阀2-4
//----------------------------------------------------------------------------------------------------------
void control_cool2_4(int status)
{
    // 测试模式优先
    if (get_val(TEST_COOL2_4_MODE))
    {
        return;
    }

    if (get_val(SENSOR_6_ABNORMAL_ALARM))
    {
        return;
    }

    if ((g_fan_status == 1) && ((get_val(R_SYNC_MOTOR_CONTROL_PARA) != 1) && (get_val(R_SYNC_MOTOR_CONTROL_PARA) != 2)))
    {
        set_val(CONTROL_COOL2_4, OFF);
    }

    set_val(CONTROL_COOL2_4, status);
}

//----------------------------------------------------------------------------------------------------------
// 水冷电磁阀2
//----------------------------------------------------------------------------------------------------------
void control_cool2(int type, int status)
{
    // 测试模式优先
    switch (type)
    {
    case 1:
        control_cool2_1(status);
        break;
    case 2:
        control_cool2_2(status);
        break;
    case 3:
        control_cool2_3(status);
        break;
    case 4:
        control_cool2_4(status);
        break;
    }
}

//----------------------------------------------------------------------------------------------------------
// 冷热水电磁阀
//----------------------------------------------------------------------------------------------------------
void control_cold_hot_water(int status)
{
    set_val(CONTROL_COLD_HOT_WATER, status);
}

//----------------------------------------------------------------------------------------------------------
// 鼓风机
//----------------------------------------------------------------------------------------------------------
void control_blower(int status)
{
    // 测试模式优先
    if (get_val(TEST_BLOWER_MODE))
    {
        return;
    }

    if (g_fan_status == 1)
    {
        set_val(CONTROL_BLOWER, OFF);
        return;
    }

    set_val(CONTROL_BLOWER, status);
}

//----------------------------------------------------------------------------------------------------------
// 风门开控制
//----------------------------------------------------------------------------------------------------------
void control_open_damper(int status)
{
    // 检测到开到位停止控制
    if (get_val(DETECT_DAMPER_FULLY_OPEN))
    {
        set_val(CONTROL_DAMPER_OPEN, OFF);
    }
    else
    {
        // 测试模式优先
        set_val(CONTROL_DAMPER_CLOSE, OFF);
        set_val(CONTROL_DAMPER_OPEN, status);
    }
}

//----------------------------------------------------------------------------------------------------------
// 风门关控制
//----------------------------------------------------------------------------------------------------------
void control_close_damper(int status, int schedule)
{
    // 检测到关到位停止控制
    if (get_val(DETECT_DAMPER_FULLY_CLOSE))
    {
        set_val(CONTROL_DAMPER_CLOSE, OFF);
    }
    else
    {
        // 风门关开启，风门开关闭
        set_val(CONTROL_DAMPER_OPEN, OFF);
        // 当schedule为1时，风门小于设定值不会操作
        if (1 == schedule)
        {
            // 当风门测量值大于风门设定值，风门才会操作
            if (get_val(R_AI_AD) > get_val(P_AO_AD))
            {
                set_val(CONTROL_DAMPER_CLOSE, status);
            }
            else
            {
                set_val(CONTROL_DAMPER_CLOSE, OFF);
            }
        }
        // 当schedule为0时，风门关到位
        else
        {
            set_val(CONTROL_DAMPER_CLOSE, status);
        }
    }
}

//----------------------------------------------------------------------------------------------------------
// 风门控制
//----------------------------------------------------------------------------------------------------------
void control_damper(int type, int status, int schedule)
{
    if (get_val(TEST_DAMPER_OPEN_MODE) || get_val(TEST_DAMPER_CLOSE_MODE))
    {
        return;
    }

    if ((g_fan_status == 1) && ((get_val(R_SYNC_MOTOR_CONTROL_PARA) != 1) && (get_val(R_SYNC_MOTOR_CONTROL_PARA) != 2)))
    {
        control_open_damper(OFF);
        return;
    }

    switch (type)
    {
    case 1:
        control_open_damper(status);
        break;
    case 2:
        control_close_damper(status, schedule);
        break;
    }
}

//----------------------------------------------------------------------------------------------------------
// 水冷控制
//----------------------------------------------------------------------------------------------------------
void control_cool(int status)
{
    // 测试模式优先
    if (get_val(TEST_COOL_MODE))
    {
        return;
    }

    if ((g_fan_status == 1) && ((get_val(R_SYNC_MOTOR_CONTROL_PARA) != 1) && (get_val(R_SYNC_MOTOR_CONTROL_PARA) != 2)))
    {
        set_val(CONTROL_COOL, OFF);
        return;
    }

    set_val(CONTROL_COOL, status);
}

//----------------------------------------------------------------------------------------------------------
// 加湿电磁阀控制
//----------------------------------------------------------------------------------------------------------
void control_spray(int status)
{
    // 测试模式优先
    if (get_val(TEST_SPRAY_MODE))
    {
        return;
    }

    if (get_val(SENSOR_2_ABNORMAL_ALARM))
    {
        set_val(CONTROL_SPRAY, OFF);
        return;
    }

    // 高湿报警
    if (OUT_ST_HM_HIGH_ALARM)
    {
        set_val(CONTROL_SPRAY, OFF);
        return;
    }

    if ((g_fan_status == 1) && ((get_val(R_SYNC_MOTOR_CONTROL_PARA) != 1) && (get_val(R_SYNC_MOTOR_CONTROL_PARA) != 2)))
    {
        set_val(CONTROL_SPRAY, OFF);
        return;
    }

    set_val(CONTROL_SPRAY, status);
}

//----------------------------------------------------------------------------------------------------------
// 翻蛋控制
//----------------------------------------------------------------------------------------------------------
void control_egg()
{
    if (!get_val(TEST_FILP_EGG_MODE))
    {
        // 翻蛋方向
        unsigned short v_uc_egg_filpping_direction = get_uval(P_EGG_FLIPPING_DIRECTION);
        // 翻蛋时间
        unsigned short v_i_egg_filping_time = get_uval(P_HH10) * 60;

        zlog_debug(g_zlog_zc, "%-40s翻动方向:[%d]", "[翻蛋控制]", v_uc_egg_filpping_direction);
        // 测试模式优先

        // 当翻蛋方向为左时
        if (v_uc_egg_filpping_direction)
        {
            // 执行翻蛋控制输出
            set_val(P_EGG_FLIPPING_EXECUTION_STATUS, ON);
            // 开始翻蛋计数
            set_val(P_EGG_FLIPPING_TIME, v_i_egg_filping_time);
            // 切换翻蛋方向
            set_val(P_EGG_FLIPPING_DIRECTION, !v_uc_egg_filpping_direction);
        }
        // 当翻蛋方向为右时
        else
        {
            // 执行翻蛋控制输出
            set_val(P_EGG_FLIPPING_EXECUTION_STATUS, ON);
            // 开始翻蛋计数
            set_val(P_EGG_FLIPPING_TIME, v_i_egg_filping_time);
            // 切换翻蛋方向
            set_val(P_EGG_FLIPPING_DIRECTION, !v_uc_egg_filpping_direction);
        }
        set_val(EGG_FLIPPING_TRIGGER_STATUS, ON);
    }
}

//----------------------------------------------------------------------------------------------------------
// 预设参数初始化
//----------------------------------------------------------------------------------------------------------
void preset_para_init(int mode)
{
    // 预设值-温度
    if (S_PRE_TP_MAIN == mode)
    {
        // 预热值
        set_val(P_PRE_TP_MAIN, 2500);

        // 30天天数-1
        // 30天数值-1
        set_val(P_SET_TP_MAIN_DAY, 0);
        set_val(P_SET_TP_MAIN_VAL, 3810);

        // 30天天数-2
        // 30天数值-2
        set_val(P_SET_TP_MAIN_DAY + 2 * 1, 100);
        set_val(P_SET_TP_MAIN_VAL + 2 * 1, 3800);

        // 30天天数-3
        // 30天数值-3
        set_val(P_SET_TP_MAIN_DAY + 2 * 2, 200);
        set_val(P_SET_TP_MAIN_VAL + 2 * 2, 3790);

        // 30天天数-4
        // 30天数值-4
        set_val(P_SET_TP_MAIN_DAY + 2 * 3, 300);
        set_val(P_SET_TP_MAIN_VAL + 2 * 3, 3790);

        // 30天天数-5
        // 30天数值-5
        set_val(P_SET_TP_MAIN_DAY + 2 * 4, 400);
        set_val(P_SET_TP_MAIN_VAL + 2 * 4, 3790);

        // 30天天数-6
        // 30天数值-6
        set_val(P_SET_TP_MAIN_DAY + 2 * 5, 500);
        set_val(P_SET_TP_MAIN_VAL + 2 * 5, 3790);

        // 30天天数-7
        // 30天数值-7
        set_val(P_SET_TP_MAIN_DAY + 2 * 6, 600);
        set_val(P_SET_TP_MAIN_VAL + 2 * 6, 3790);

        // 30天天数-8
        // 30天数值-8
        set_val(P_SET_TP_MAIN_DAY + 2 * 7, 700);
        set_val(P_SET_TP_MAIN_VAL + 2 * 7, 3780);

        // 30天天数-9
        // 30天数值-9
        set_val(P_SET_TP_MAIN_DAY + 2 * 8, 800);
        set_val(P_SET_TP_MAIN_VAL + 2 * 8, 3780);

        // 30天天数-10
        // 30天数值-10
        set_val(P_SET_TP_MAIN_DAY + 2 * 9, 900);
        set_val(P_SET_TP_MAIN_VAL + 2 * 9, 3780);

        // 30天天数-11
        // 30天数值-11
        set_val(P_SET_TP_MAIN_DAY + 2 * 10, 1000);
        set_val(P_SET_TP_MAIN_VAL + 2 * 10, 3770);

        // 30天天数-12
        // 30天数值-12
        set_val(P_SET_TP_MAIN_DAY + 2 * 11, 1100);
        set_val(P_SET_TP_MAIN_VAL + 2 * 11, 3770);

        // 30天天数-13
        // 30天数值-13
        set_val(P_SET_TP_MAIN_DAY + 2 * 12, 1200);
        set_val(P_SET_TP_MAIN_VAL + 2 * 12, 3770);

        // 30天天数-14
        // 30天数值-14
        set_val(P_SET_TP_MAIN_DAY + 2 * 13, 1300);
        set_val(P_SET_TP_MAIN_VAL + 2 * 13, 3770);

        // 30天天数-15
        // 30天数值-15
        set_val(P_SET_TP_MAIN_DAY + 2 * 14, 1400);
        set_val(P_SET_TP_MAIN_VAL + 2 * 14, 3760);

        // 30天天数-16
        // 30天数值-16
        set_val(P_SET_TP_MAIN_DAY + 2 * 15, 1500);
        set_val(P_SET_TP_MAIN_VAL + 2 * 15, 3760);

        // 30天天数-17
        // 30天数值-17
        set_val(P_SET_TP_MAIN_DAY + 2 * 16, 1600);
        set_val(P_SET_TP_MAIN_VAL + 2 * 16, 3760);

        // 30天天数-18
        // 30天数值-18
        set_val(P_SET_TP_MAIN_DAY + 2 * 17, 1700);
        set_val(P_SET_TP_MAIN_VAL + 2 * 17, 3760);

        // 30天天数-19
        // 30天数值-19
        set_val(P_SET_TP_MAIN_DAY + 2 * 18, 1800);
        set_val(P_SET_TP_MAIN_VAL + 2 * 18, 3750);

        // 30天天数-20
        // 30天数值-20
        set_val(P_SET_TP_MAIN_DAY + 2 * 19, 1900);
        set_val(P_SET_TP_MAIN_VAL + 2 * 19, 3750);

        // 30天天数-21
        // 30天数值-21
        set_val(P_SET_TP_MAIN_DAY + 2 * 20, 2000);
        set_val(P_SET_TP_MAIN_VAL + 2 * 20, 3720);

        // 30天天数-22
        // 30天数值-22
        set_val(P_SET_TP_MAIN_DAY + 2 * 21, 2100);
        set_val(P_SET_TP_MAIN_VAL + 2 * 21, 3720);

        // 30天天数-23
        // 30天数值-23
        set_val(P_SET_TP_MAIN_DAY + 2 * 22, 2200);
        set_val(P_SET_TP_MAIN_VAL + 2 * 22, 3710);

        // 30天天数-24
        // 30天数值-24
        set_val(P_SET_TP_MAIN_DAY + 2 * 23, 2300);
        set_val(P_SET_TP_MAIN_VAL + 2 * 23, 3710);

        // 30天天数-25
        // 30天数值-25
        set_val(P_SET_TP_MAIN_DAY + 2 * 24, 2400);
        set_val(P_SET_TP_MAIN_VAL + 2 * 24, 3700);

        // 30天天数-26
        // 30天数值-26
        set_val(P_SET_TP_MAIN_DAY + 2 * 25, 2500);
        set_val(P_SET_TP_MAIN_VAL + 2 * 25, 3700);

        // 30天天数-27
        // 30天数值-27
        set_val(P_SET_TP_MAIN_DAY + 2 * 26, 2600);
        set_val(P_SET_TP_MAIN_VAL + 2 * 26, 3650);

        // 30天天数-28
        // 30天数值-28
        set_val(P_SET_TP_MAIN_DAY + 2 * 27, 2700);
        set_val(P_SET_TP_MAIN_VAL + 2 * 27, 3650);

        // 30天天数-29
        // 30天数值-29
        set_val(P_SET_TP_MAIN_DAY + 2 * 28, 2800);
        set_val(P_SET_TP_MAIN_VAL + 2 * 28, 3600);

        // 30天天数-30
        // 30天数值-30
        set_val(P_SET_TP_MAIN_DAY + 2 * 29, 2900);
        set_val(P_SET_TP_MAIN_VAL + 2 * 29, 3600);
    }

    // 预设值-湿度
    else if (S_PRE_HM == mode)
    {
        // 预热值
        set_val(P_PRE_HM, 3150);

        // 30天天数-1
        // 30天数值-1
        set_val(P_SET_HM_DAY, 0);
        set_val(P_SET_HM_VAL, 3150);

        // 30天天数-2
        // 30天数值-2
        set_val(P_SET_HM_DAY + 2 * 1, 100);
        set_val(P_SET_HM_VAL + 2 * 1, 3150);

        // 30天天数-3
        // 30天数值-3
        set_val(P_SET_HM_DAY + 2 * 2, 200);
        set_val(P_SET_HM_VAL + 2 * 2, 3150);

        // 30天天数-4
        // 30天数值-4
        set_val(P_SET_HM_DAY + 2 * 3, 300);
        set_val(P_SET_HM_VAL + 2 * 3, 3150);

        // 30天天数-5
        // 30天数值-5
        set_val(P_SET_HM_DAY + 2 * 4, 400);
        set_val(P_SET_HM_VAL + 2 * 4, 3000);

        // 30天天数-6
        // 30天数值-6
        set_val(P_SET_HM_DAY + 2 * 5, 500);
        set_val(P_SET_HM_VAL + 2 * 5, 3000);

        // 30天天数-7
        // 30天数值-7
        set_val(P_SET_HM_DAY + 2 * 6, 600);
        set_val(P_SET_HM_VAL + 2 * 6, 3000);

        // 30天天数-8
        // 30天数值-8
        set_val(P_SET_HM_DAY + 2 * 7, 700);
        set_val(P_SET_HM_VAL + 2 * 7, 3000);

        // 30天天数-9
        // 30天数值-9
        set_val(P_SET_HM_DAY + 2 * 8, 800);
        set_val(P_SET_HM_VAL + 2 * 8, 3000);

        // 30天天数-10
        // 30天数值-10
        set_val(P_SET_HM_DAY + 2 * 9, 900);
        set_val(P_SET_HM_VAL + 2 * 9, 3000);

        // 30天天数-11
        // 30天数值-11
        set_val(P_SET_HM_DAY + 2 * 10, 1000);
        set_val(P_SET_HM_VAL + 2 * 10, 2950);

        // 30天天数-12
        // 30天数值-12
        set_val(P_SET_HM_DAY + 2 * 11, 1100);
        set_val(P_SET_HM_VAL + 2 * 11, 2950);

        // 30天天数-13
        // 30天数值-13
        set_val(P_SET_HM_DAY + 2 * 12, 1200);
        set_val(P_SET_HM_VAL + 2 * 12, 2950);

        // 30天天数-14
        // 30天数值-14
        set_val(P_SET_HM_DAY + 2 * 13, 1300);
        set_val(P_SET_HM_VAL + 2 * 13, 2950);

        // 30天天数-15
        // 30天数值-15
        set_val(P_SET_HM_DAY + 2 * 14, 1400);
        set_val(P_SET_HM_VAL + 2 * 14, 2950);

        // 30天天数-16
        // 30天数值-16
        set_val(P_SET_HM_DAY + 2 * 15, 1500);
        set_val(P_SET_HM_VAL + 2 * 15, 2900);

        // 30天天数-17
        // 30天数值-17
        set_val(P_SET_HM_DAY + 2 * 16, 1600);
        set_val(P_SET_HM_VAL + 2 * 16, 2900);

        // 30天天数-18
        // 30天数值-18
        set_val(P_SET_HM_DAY + 2 * 17, 1700);
        set_val(P_SET_HM_VAL + 2 * 17, 2900);

        // 30天天数-19
        // 30天数值-19
        set_val(P_SET_HM_DAY + 2 * 18, 1800);
        set_val(P_SET_HM_VAL + 2 * 18, 2900);

        // 30天天数-20
        // 30天数值-20
        set_val(P_SET_HM_DAY + 2 * 19, 1900);
        set_val(P_SET_HM_VAL + 2 * 19, 2900);

        // 30天天数-21
        // 30天数值-21
        set_val(P_SET_HM_DAY + 2 * 20, 2000);
        set_val(P_SET_HM_VAL + 2 * 20, 2850);

        // 30天天数-22
        // 30天数值-22
        set_val(P_SET_HM_DAY + 2 * 21, 2100);
        set_val(P_SET_HM_VAL + 2 * 21, 2850);

        // 30天天数-23
        // 30天数值-23
        set_val(P_SET_HM_DAY + 2 * 22, 2200);
        set_val(P_SET_HM_VAL + 2 * 22, 2850);

        // 30天天数-24
        // 30天数值-24
        set_val(P_SET_HM_DAY + 2 * 23, 2300);
        set_val(P_SET_HM_VAL + 2 * 23, 2850);

        // 30天天数-25
        // 30天数值-25
        set_val(P_SET_HM_DAY + 2 * 24, 2400);
        set_val(P_SET_HM_VAL + 2 * 24, 2850);

        // 30天天数-26
        // 30天数值-26
        set_val(P_SET_HM_DAY + 2 * 25, 2500);
        set_val(P_SET_HM_VAL + 2 * 25, 2800);

        // 30天天数-27
        // 30天数值-27
        set_val(P_SET_HM_DAY + 2 * 26, 2600);
        set_val(P_SET_HM_VAL + 2 * 26, 2800);

        // 30天天数-28
        // 30天数值-28
        set_val(P_SET_HM_DAY + 2 * 27, 2700);
        set_val(P_SET_HM_VAL + 2 * 27, 2800);

        // 30天天数-29
        // 30天数值-29
        set_val(P_SET_HM_DAY + 2 * 28, 2800);
        set_val(P_SET_HM_VAL + 2 * 28, 2800);

        // 30天天数-30
        // 30天数值-30
        set_val(P_SET_HM_DAY + 2 * 29, 2900);
        set_val(P_SET_HM_VAL + 2 * 29, 2800);
    }

    // 预设值-CO2
    else if (S_PRE_CO == mode)
    {
        // 预热值
        set_val(P_PRE_CO, 5000);

        // 30天天数-1
        // 30天数值-1
        set_val(P_SET_CO_DAY, 0);
        set_val(P_SET_CO_VAL, 10000);

        // 30天天数-2
        // 30天数值-2
        set_val(P_SET_CO_DAY + 2 * 1, 100);
        set_val(P_SET_CO_VAL + 2 * 1, 10000);

        // 30天天数-3
        // 30天数值-3
        set_val(P_SET_CO_DAY + 2 * 2, 200);
        set_val(P_SET_CO_VAL + 2 * 2, 10000);

        // 30天天数-4
        // 30天数值-4
        set_val(P_SET_CO_DAY + 2 * 3, 300);
        set_val(P_SET_CO_VAL + 2 * 3, 10000);

        // 30天天数-5
        // 30天数值-5
        set_val(P_SET_CO_DAY + 2 * 4, 400);
        set_val(P_SET_CO_VAL + 2 * 4, 10000);

        // 30天天数-6
        // 30天数值-6
        set_val(P_SET_CO_DAY + 2 * 5, 500);
        set_val(P_SET_CO_VAL + 2 * 5, 10000);

        // 30天天数-7
        // 30天数值-7
        set_val(P_SET_CO_DAY + 2 * 6, 600);
        set_val(P_SET_CO_VAL + 2 * 6, 10000);

        // 30天天数-8
        // 30天数值-8
        set_val(P_SET_CO_DAY + 2 * 7, 700);
        set_val(P_SET_CO_VAL + 2 * 7, 10000);

        // 30天天数-9
        // 30天数值-9
        set_val(P_SET_CO_DAY + 2 * 8, 800);
        set_val(P_SET_CO_VAL + 2 * 8, 10000);

        // 30天天数-10
        // 30天数值-10
        set_val(P_SET_CO_DAY + 2 * 9, 900);
        set_val(P_SET_CO_VAL + 2 * 9, 10000);

        // 30天天数-11
        // 30天数值-11
        set_val(P_SET_CO_DAY + 2 * 10, 1000);
        set_val(P_SET_CO_VAL + 2 * 10, 9000);

        // 30天天数-12
        // 30天数值-12
        set_val(P_SET_CO_DAY + 2 * 11, 1100);
        set_val(P_SET_CO_VAL + 2 * 11, 9000);

        // 30天天数-13
        // 30天数值-13
        set_val(P_SET_CO_DAY + 2 * 12, 1200);
        set_val(P_SET_CO_VAL + 2 * 12, 9000);

        // 30天天数-14
        // 30天数值-14
        set_val(P_SET_CO_DAY + 2 * 13, 1300);
        set_val(P_SET_CO_VAL + 2 * 13, 9000);

        // 30天天数-15
        // 30天数值-15
        set_val(P_SET_CO_DAY + 2 * 14, 1400);
        set_val(P_SET_CO_VAL + 2 * 14, 9000);

        // 30天天数-16
        // 30天数值-16
        set_val(P_SET_CO_DAY + 2 * 15, 1500);
        set_val(P_SET_CO_VAL + 2 * 15, 8000);

        // 30天天数-17
        // 30天数值-17
        set_val(P_SET_CO_DAY + 2 * 16, 1600);
        set_val(P_SET_CO_VAL + 2 * 16, 8000);

        // 30天天数-18
        // 30天数值-18
        set_val(P_SET_CO_DAY + 2 * 17, 1700);
        set_val(P_SET_CO_VAL + 2 * 17, 8000);

        // 30天天数-19
        // 30天数值-19
        set_val(P_SET_CO_DAY + 2 * 18, 1800);
        set_val(P_SET_CO_VAL + 2 * 18, 8000);

        // 30天天数-20
        // 30天数值-20
        set_val(P_SET_CO_DAY + 2 * 19, 1900);
        set_val(P_SET_CO_VAL + 2 * 19, 8000);

        // 30天天数-21
        // 30天数值-21
        set_val(P_SET_CO_DAY + 2 * 20, 2000);
        set_val(P_SET_CO_VAL + 2 * 20, 7000);

        // 30天天数-22
        // 30天数值-22
        set_val(P_SET_CO_DAY + 2 * 21, 2100);
        set_val(P_SET_CO_VAL + 2 * 21, 7000);

        // 30天天数-23
        // 30天数值-23
        set_val(P_SET_CO_DAY + 2 * 22, 2200);
        set_val(P_SET_CO_VAL + 2 * 22, 7000);

        // 30天天数-24
        // 30天数值-24
        set_val(P_SET_CO_DAY + 2 * 23, 2300);
        set_val(P_SET_CO_VAL + 2 * 23, 7000);

        // 30天天数-25
        // 30天数值-25
        set_val(P_SET_CO_DAY + 2 * 24, 2400);
        set_val(P_SET_CO_VAL + 2 * 24, 7000);

        // 30天天数-26
        // 30天数值-26
        set_val(P_SET_CO_DAY + 2 * 25, 2500);
        set_val(P_SET_CO_VAL + 2 * 25, 6000);

        // 30天天数-27
        // 30天数值-27
        set_val(P_SET_CO_DAY + 2 * 26, 2600);
        set_val(P_SET_CO_VAL + 2 * 26, 6000);

        // 30天天数-28
        // 30天数值-28
        set_val(P_SET_CO_DAY + 2 * 27, 2700);
        set_val(P_SET_CO_VAL + 2 * 27, 6000);

        // 30天天数-29
        // 30天数值-29
        set_val(P_SET_CO_DAY + 2 * 28, 2800);
        set_val(P_SET_CO_VAL + 2 * 28, 6000);

        // 30天天数-30
        // 30天数值-30
        set_val(P_SET_CO_DAY + 2 * 29, 2900);
        set_val(P_SET_CO_VAL + 2 * 29, 6000);
    }

    // 预设值-风门
    else if (S_PRE_AD == mode)
    {
        // 预热值
        set_val(P_PRE_AD, 30);

        // 30天天数-1
        // 30天数值-1
        set_val(P_SET_AD_DAY, 0);
        set_val(P_SET_AD_VAL, 0);

        // 30天天数-2
        // 30天数值-2
        set_val(P_SET_AD_DAY + 2 * 1, 100);
        set_val(P_SET_AD_VAL + 2 * 1, 0);

        // 30天天数-3
        // 30天数值-3
        set_val(P_SET_AD_DAY + 2 * 2, 200);
        set_val(P_SET_AD_VAL + 2 * 2, 0);

        // 30天天数-4
        // 30天数值-4
        set_val(P_SET_AD_DAY + 2 * 3, 300);
        set_val(P_SET_AD_VAL + 2 * 3, 0);

        // 30天天数-5
        // 30天数值-5
        set_val(P_SET_AD_DAY + 2 * 4, 400);
        set_val(P_SET_AD_VAL + 2 * 4, 0);

        // 30天天数-6
        // 30天数值-6
        set_val(P_SET_AD_DAY + 2 * 5, 500);
        set_val(P_SET_AD_VAL + 2 * 5, 0);

        // 30天天数-7
        // 30天数值-7
        set_val(P_SET_AD_DAY + 2 * 6, 600);
        set_val(P_SET_AD_VAL + 2 * 6, 0);

        // 30天天数-8
        // 30天数值-8
        set_val(P_SET_AD_DAY + 2 * 7, 700);
        set_val(P_SET_AD_VAL + 2 * 7, 0);

        // 30天天数-9
        // 30天数值-9
        set_val(P_SET_AD_DAY + 2 * 8, 800);
        set_val(P_SET_AD_VAL + 2 * 8, 0);

        // 30天天数-10
        // 30天数值-10
        set_val(P_SET_AD_DAY + 2 * 9, 900);
        set_val(P_SET_AD_VAL + 2 * 9, 0);

        // 30天天数-11
        // 30天数值-11
        set_val(P_SET_AD_DAY + 2 * 10, 1000);
        set_val(P_SET_AD_VAL + 2 * 10, 10);

        // 30天天数-12
        // 30天数值-12
        set_val(P_SET_AD_DAY + 2 * 11, 1100);
        set_val(P_SET_AD_VAL + 2 * 11, 10);

        // 30天天数-13
        // 30天数值-13
        set_val(P_SET_AD_DAY + 2 * 12, 1200);
        set_val(P_SET_AD_VAL + 2 * 12, 10);

        // 30天天数-14
        // 30天数值-14
        set_val(P_SET_AD_DAY + 2 * 13, 1300);
        set_val(P_SET_AD_VAL + 2 * 13, 10);

        // 30天天数-15
        // 30天数值-15
        set_val(P_SET_AD_DAY + 2 * 14, 1400);
        set_val(P_SET_AD_VAL + 2 * 14, 15);

        // 30天天数-16
        // 30天数值-16
        set_val(P_SET_AD_DAY + 2 * 15, 1500);
        set_val(P_SET_AD_VAL + 2 * 15, 15);

        // 30天天数-17
        // 30天数值-17
        set_val(P_SET_AD_DAY + 2 * 16, 1600);
        set_val(P_SET_AD_VAL + 2 * 16, 15);

        // 30天天数-18
        // 30天数值-18
        set_val(P_SET_AD_DAY + 2 * 17, 1700);
        set_val(P_SET_AD_VAL + 2 * 17, 15);

        // 30天天数-19
        // 30天数值-19
        set_val(P_SET_AD_DAY + 2 * 18, 1800);
        set_val(P_SET_AD_VAL + 2 * 18, 15);

        // 30天天数-20
        // 30天数值-20
        set_val(P_SET_AD_DAY + 2 * 19, 1900);
        set_val(P_SET_AD_VAL + 2 * 19, 20);

        // 30天天数-21
        // 30天数值-21
        set_val(P_SET_AD_DAY + 2 * 20, 2000);
        set_val(P_SET_AD_VAL + 2 * 20, 20);

        // 30天天数-22
        // 30天数值-22
        set_val(P_SET_AD_DAY + 2 * 21, 2100);
        set_val(P_SET_AD_VAL + 2 * 21, 20);

        // 30天天数-23
        // 30天数值-23
        set_val(P_SET_AD_DAY + 2 * 22, 2200);
        set_val(P_SET_AD_VAL + 2 * 22, 20);

        // 30天天数-24
        // 30天数值-24
        set_val(P_SET_AD_DAY + 2 * 23, 2300);
        set_val(P_SET_AD_VAL + 2 * 23, 20);

        // 30天天数-25
        // 30天数值-25
        set_val(P_SET_AD_DAY + 2 * 24, 2400);
        set_val(P_SET_AD_VAL + 2 * 24, 25);

        // 30天天数-26
        // 30天数值-26
        set_val(P_SET_AD_DAY + 2 * 25, 2500);
        set_val(P_SET_AD_VAL + 2 * 25, 25);

        // 30天天数-27
        // 30天数值-27
        set_val(P_SET_AD_DAY + 2 * 26, 2600);
        set_val(P_SET_AD_VAL + 2 * 26, 25);

        // 30天天数-28
        // 30天数值-28
        set_val(P_SET_AD_DAY + 2 * 27, 2700);
        set_val(P_SET_AD_VAL + 2 * 27, 25);

        // 30天天数-29
        // 30天数值-29
        set_val(P_SET_AD_DAY + 2 * 28, 2800);
        set_val(P_SET_AD_VAL + 2 * 28, 25);

        // 30天天数-30
        // 30天数值-30
        set_val(P_SET_AD_DAY + 2 * 29, 2900);
        set_val(P_SET_AD_VAL + 2 * 29, 25);
    }

    // 预设值-回流温度1
    else if (S_PRE_TP_RF1 == mode)
    {
        // 冷水天数
        // 预热值
        set_val(P_PRE_TP_RF1_DAY, 900);
        set_val(P_PRE_TP_RF1, -10);

        // 30天天数-1
        // 30天数值-1
        set_val(P_SET_TP_RF1_DAY, 0);
        set_val(P_SET_TP_RF1_VAL, -10);

        // 30天天数-2
        // 30天数值-2
        set_val(P_SET_TP_RF1_DAY + 2 * 1, 100);
        set_val(P_SET_TP_RF1_VAL + 2 * 1, -10);

        // 30天天数-3
        // 30天数值-3
        set_val(P_SET_TP_RF1_DAY + 2 * 2, 200);
        set_val(P_SET_TP_RF1_VAL + 2 * 2, -10);

        // 30天天数-4
        // 30天数值-4
        set_val(P_SET_TP_RF1_DAY + 2 * 3, 300);
        set_val(P_SET_TP_RF1_VAL + 2 * 3, -10);

        // 30天天数-5
        // 30天数值-5
        set_val(P_SET_TP_RF1_DAY + 2 * 4, 400);
        set_val(P_SET_TP_RF1_VAL + 2 * 4, -10);

        // 30天天数-6
        // 30天数值-6
        set_val(P_SET_TP_RF1_DAY + 2 * 5, 500);
        set_val(P_SET_TP_RF1_VAL + 2 * 5, -10);

        // 30天天数-7
        // 30天数值-7
        set_val(P_SET_TP_RF1_DAY + 2 * 6, 600);
        set_val(P_SET_TP_RF1_VAL + 2 * 6, -10);

        // 30天天数-8
        // 30天数值-8
        set_val(P_SET_TP_RF1_DAY + 2 * 7, 700);
        set_val(P_SET_TP_RF1_VAL + 2 * 7, -10);

        // 30天天数-9
        // 30天数值-9
        set_val(P_SET_TP_RF1_DAY + 2 * 8, 800);
        set_val(P_SET_TP_RF1_VAL + 2 * 8, -10);

        // 30天天数-10
        // 30天数值-10
        set_val(P_SET_TP_RF1_DAY + 2 * 9, 900);
        set_val(P_SET_TP_RF1_VAL + 2 * 9, 20);

        // 30天天数-11
        // 30天数值-11
        set_val(P_SET_TP_RF1_DAY + 2 * 10, 1000);
        set_val(P_SET_TP_RF1_VAL + 2 * 10, 20);

        // 30天天数-12
        // 30天数值-12
        set_val(P_SET_TP_RF1_DAY + 2 * 11, 1100);
        set_val(P_SET_TP_RF1_VAL + 2 * 11, 20);

        // 30天天数-13
        // 30天数值-13
        set_val(P_SET_TP_RF1_DAY + 2 * 12, 1200);
        set_val(P_SET_TP_RF1_VAL + 2 * 12, 20);

        // 30天天数-14
        // 30天数值-14
        set_val(P_SET_TP_RF1_DAY + 2 * 13, 1300);
        set_val(P_SET_TP_RF1_VAL + 2 * 13, 20);

        // 30天天数-15
        // 30天数值-15
        set_val(P_SET_TP_RF1_DAY + 2 * 14, 1400);
        set_val(P_SET_TP_RF1_VAL + 2 * 14, 20);

        // 30天天数-16
        // 30天数值-16
        set_val(P_SET_TP_RF1_DAY + 2 * 15, 1500);
        set_val(P_SET_TP_RF1_VAL + 2 * 15, 15);

        // 30天天数-17
        // 30天数值-17
        set_val(P_SET_TP_RF1_DAY + 2 * 16, 1600);
        set_val(P_SET_TP_RF1_VAL + 2 * 16, 15);

        // 30天天数-18
        // 30天数值-18
        set_val(P_SET_TP_RF1_DAY + 2 * 17, 1700);
        set_val(P_SET_TP_RF1_VAL + 2 * 17, 15);

        // 30天天数-19
        // 30天数值-19
        set_val(P_SET_TP_RF1_DAY + 2 * 18, 1800);
        set_val(P_SET_TP_RF1_VAL + 2 * 18, 15);

        // 30天天数-20
        // 30天数值-20
        set_val(P_SET_TP_RF1_DAY + 2 * 19, 1900);
        set_val(P_SET_TP_RF1_VAL + 2 * 19, 15);

        // 30天天数-21
        // 30天数值-21
        set_val(P_SET_TP_RF1_DAY + 2 * 20, 2000);
        set_val(P_SET_TP_RF1_VAL + 2 * 20, 20);

        // 30天天数-22
        // 30天数值-22
        set_val(P_SET_TP_RF1_DAY + 2 * 21, 2100);
        set_val(P_SET_TP_RF1_VAL + 2 * 21, 20);

        // 30天天数-23
        // 30天数值-23
        set_val(P_SET_TP_RF1_DAY + 2 * 22, 2200);
        set_val(P_SET_TP_RF1_VAL + 2 * 22, 20);

        // 30天天数-24
        // 30天数值-24
        set_val(P_SET_TP_RF1_DAY + 2 * 23, 2300);
        set_val(P_SET_TP_RF1_VAL + 2 * 23, 20);

        // 30天天数-25
        // 30天数值-25
        set_val(P_SET_TP_RF1_DAY + 2 * 24, 2400);
        set_val(P_SET_TP_RF1_VAL + 2 * 24, 20);

        // 30天天数-26
        // 30天数值-26
        set_val(P_SET_TP_RF1_DAY + 2 * 25, 2500);
        set_val(P_SET_TP_RF1_VAL + 2 * 25, 30);

        // 30天天数-27
        // 30天数值-27
        set_val(P_SET_TP_RF1_DAY + 2 * 26, 2600);
        set_val(P_SET_TP_RF1_VAL + 2 * 26, 30);

        // 30天天数-28
        // 30天数值-28
        set_val(P_SET_TP_RF1_DAY + 2 * 27, 2700);
        set_val(P_SET_TP_RF1_VAL + 2 * 27, 30);

        // 30天天数-29
        // 30天数值-29
        set_val(P_SET_TP_RF1_DAY + 2 * 28, 2800);
        set_val(P_SET_TP_RF1_VAL + 2 * 28, 30);

        // 30天天数-30
        // 30天数值-30
        set_val(P_SET_TP_RF1_DAY + 2 * 29, 2900);
        set_val(P_SET_TP_RF1_VAL + 2 * 29, 30);
    }

    // 预设值-回流温度2
    else if (S_PRE_TP_RF2 == mode)
    {
        // 冷水天数
        // 预热值
        set_val(P_PRE_TP_RF2_DAY, 900);
        set_val(P_PRE_TP_RF2, -10);

        // 30天天数-1
        // 30天数值-1
        set_val(P_SET_TP_RF2_DAY, 0);
        set_val(P_SET_TP_RF2_VAL, -10);

        // 30天天数-2
        // 30天数值-2
        set_val(P_SET_TP_RF2_DAY + 2 * 1, 100);
        set_val(P_SET_TP_RF2_VAL + 2 * 1, -10);

        // 30天天数-3
        // 30天数值-3
        set_val(P_SET_TP_RF2_DAY + 2 * 2, 200);
        set_val(P_SET_TP_RF2_VAL + 2 * 2, -10);

        // 30天天数-4
        // 30天数值-4
        set_val(P_SET_TP_RF2_DAY + 2 * 3, 300);
        set_val(P_SET_TP_RF2_VAL + 2 * 3, -10);

        // 30天天数-5
        // 30天数值-5
        set_val(P_SET_TP_RF2_DAY + 2 * 4, 400);
        set_val(P_SET_TP_RF2_VAL + 2 * 4, -10);

        // 30天天数-6
        // 30天数值-6
        set_val(P_SET_TP_RF2_DAY + 2 * 5, 500);
        set_val(P_SET_TP_RF2_VAL + 2 * 5, -10);

        // 30天天数-7
        // 30天数值-7
        set_val(P_SET_TP_RF2_DAY + 2 * 6, 600);
        set_val(P_SET_TP_RF2_VAL + 2 * 6, -10);

        // 30天天数-8
        // 30天数值-8
        set_val(P_SET_TP_RF2_DAY + 2 * 7, 700);
        set_val(P_SET_TP_RF2_VAL + 2 * 7, -10);

        // 30天天数-9
        // 30天数值-9
        set_val(P_SET_TP_RF2_DAY + 2 * 8, 800);
        set_val(P_SET_TP_RF2_VAL + 2 * 8, -10);

        // 30天天数-10
        // 30天数值-10
        set_val(P_SET_TP_RF2_DAY + 2 * 9, 900);
        set_val(P_SET_TP_RF2_VAL + 2 * 9, 20);

        // 30天天数-11
        // 30天数值-11
        set_val(P_SET_TP_RF2_DAY + 2 * 10, 1000);
        set_val(P_SET_TP_RF2_VAL + 2 * 10, 20);

        // 30天天数-12
        // 30天数值-12
        set_val(P_SET_TP_RF2_DAY + 2 * 11, 1100);
        set_val(P_SET_TP_RF2_VAL + 2 * 11, 20);

        // 30天天数-13
        // 30天数值-13
        set_val(P_SET_TP_RF2_DAY + 2 * 12, 1200);
        set_val(P_SET_TP_RF2_VAL + 2 * 12, 20);

        // 30天天数-14
        // 30天数值-14
        set_val(P_SET_TP_RF2_DAY + 2 * 13, 1300);
        set_val(P_SET_TP_RF2_VAL + 2 * 13, 20);

        // 30天天数-15
        // 30天数值-15
        set_val(P_SET_TP_RF2_DAY + 2 * 14, 1400);
        set_val(P_SET_TP_RF2_VAL + 2 * 14, 20);

        // 30天天数-16
        // 30天数值-16
        set_val(P_SET_TP_RF2_DAY + 2 * 15, 1500);
        set_val(P_SET_TP_RF2_VAL + 2 * 15, 15);

        // 30天天数-17
        // 30天数值-17
        set_val(P_SET_TP_RF2_DAY + 2 * 16, 1600);
        set_val(P_SET_TP_RF2_VAL + 2 * 16, 15);

        // 30天天数-18
        // 30天数值-18
        set_val(P_SET_TP_RF2_DAY + 2 * 17, 1700);
        set_val(P_SET_TP_RF2_VAL + 2 * 17, 15);

        // 30天天数-19
        // 30天数值-19
        set_val(P_SET_TP_RF2_DAY + 2 * 18, 1800);
        set_val(P_SET_TP_RF2_VAL + 2 * 18, 15);

        // 30天天数-20
        // 30天数值-20
        set_val(P_SET_TP_RF2_DAY + 2 * 19, 1900);
        set_val(P_SET_TP_RF2_VAL + 2 * 19, 15);

        // 30天天数-21
        // 30天数值-21
        set_val(P_SET_TP_RF2_DAY + 2 * 20, 2000);
        set_val(P_SET_TP_RF2_VAL + 2 * 20, 20);

        // 30天天数-22
        // 30天数值-22
        set_val(P_SET_TP_RF2_DAY + 2 * 21, 2100);
        set_val(P_SET_TP_RF2_VAL + 2 * 21, 20);

        // 30天天数-23
        // 30天数值-23
        set_val(P_SET_TP_RF2_DAY + 2 * 22, 2200);
        set_val(P_SET_TP_RF2_VAL + 2 * 22, 20);

        // 30天天数-24
        // 30天数值-24
        set_val(P_SET_TP_RF2_DAY + 2 * 23, 2300);
        set_val(P_SET_TP_RF2_VAL + 2 * 23, 20);

        // 30天天数-25
        // 30天数值-25
        set_val(P_SET_TP_RF2_DAY + 2 * 24, 2400);
        set_val(P_SET_TP_RF2_VAL + 2 * 24, 20);

        // 30天天数-26
        // 30天数值-26
        set_val(P_SET_TP_RF2_DAY + 2 * 25, 2500);
        set_val(P_SET_TP_RF2_VAL + 2 * 25, 30);

        // 30天天数-27
        // 30天数值-27
        set_val(P_SET_TP_RF2_DAY + 2 * 26, 2600);
        set_val(P_SET_TP_RF2_VAL + 2 * 26, 30);

        // 30天天数-28
        // 30天数值-28
        set_val(P_SET_TP_RF2_DAY + 2 * 27, 2700);
        set_val(P_SET_TP_RF2_VAL + 2 * 27, 30);

        // 30天天数-29
        // 30天数值-29
        set_val(P_SET_TP_RF2_DAY + 2 * 28, 2800);
        set_val(P_SET_TP_RF2_VAL + 2 * 28, 30);

        // 30天天数-30
        // 30天数值-30
        set_val(P_SET_TP_RF2_DAY + 2 * 29, 2900);
        set_val(P_SET_TP_RF2_VAL + 2 * 29, 30);
    }

    // 预设值-回流温度3
    else if (S_PRE_TP_RF3 == mode)
    {
        // 冷水天数
        // 预热值
        set_val(P_PRE_TP_RF3_DAY, 900);
        set_val(P_PRE_TP_RF3, -10);

        // 30天天数-1
        // 30天数值-1
        set_val(P_SET_TP_RF3_DAY, 0);
        set_val(P_SET_TP_RF3_VAL, -10);

        // 30天天数-2
        // 30天数值-2
        set_val(P_SET_TP_RF3_DAY + 2 * 1, 100);
        set_val(P_SET_TP_RF3_VAL + 2 * 1, -10);

        // 30天天数-3
        // 30天数值-3
        set_val(P_SET_TP_RF3_DAY + 2 * 2, 200);
        set_val(P_SET_TP_RF3_VAL + 2 * 2, -10);

        // 30天天数-4
        // 30天数值-4
        set_val(P_SET_TP_RF3_DAY + 2 * 3, 300);
        set_val(P_SET_TP_RF3_VAL + 2 * 3, -10);

        // 30天天数-5
        // 30天数值-5
        set_val(P_SET_TP_RF3_DAY + 2 * 4, 400);
        set_val(P_SET_TP_RF3_VAL + 2 * 4, -10);

        // 30天天数-6
        // 30天数值-6
        set_val(P_SET_TP_RF3_DAY + 2 * 5, 500);
        set_val(P_SET_TP_RF3_VAL + 2 * 5, -10);

        // 30天天数-7
        // 30天数值-7
        set_val(P_SET_TP_RF3_DAY + 2 * 6, 600);
        set_val(P_SET_TP_RF3_VAL + 2 * 6, -10);

        // 30天天数-8
        // 30天数值-8
        set_val(P_SET_TP_RF3_DAY + 2 * 7, 700);
        set_val(P_SET_TP_RF3_VAL + 2 * 7, -10);

        // 30天天数-9
        // 30天数值-9
        set_val(P_SET_TP_RF3_DAY + 2 * 8, 800);
        set_val(P_SET_TP_RF3_VAL + 2 * 8, -10);

        // 30天天数-10
        // 30天数值-10
        set_val(P_SET_TP_RF3_DAY + 2 * 9, 900);
        set_val(P_SET_TP_RF3_VAL + 2 * 9, 20);

        // 30天天数-11
        // 30天数值-11
        set_val(P_SET_TP_RF3_DAY + 2 * 10, 1000);
        set_val(P_SET_TP_RF3_VAL + 2 * 10, 20);

        // 30天天数-12
        // 30天数值-12
        set_val(P_SET_TP_RF3_DAY + 2 * 11, 1100);
        set_val(P_SET_TP_RF3_VAL + 2 * 11, 20);

        // 30天天数-13
        // 30天数值-13
        set_val(P_SET_TP_RF3_DAY + 2 * 12, 1200);
        set_val(P_SET_TP_RF3_VAL + 2 * 12, 20);

        // 30天天数-14
        // 30天数值-14
        set_val(P_SET_TP_RF3_DAY + 2 * 13, 1300);
        set_val(P_SET_TP_RF3_VAL + 2 * 13, 20);

        // 30天天数-15
        // 30天数值-15
        set_val(P_SET_TP_RF3_DAY + 2 * 14, 1400);
        set_val(P_SET_TP_RF3_VAL + 2 * 14, 20);

        // 30天天数-16
        // 30天数值-16
        set_val(P_SET_TP_RF3_DAY + 2 * 15, 1500);
        set_val(P_SET_TP_RF3_VAL + 2 * 15, 15);

        // 30天天数-17
        // 30天数值-17
        set_val(P_SET_TP_RF3_DAY + 2 * 16, 1600);
        set_val(P_SET_TP_RF3_VAL + 2 * 16, 15);

        // 30天天数-18
        // 30天数值-18
        set_val(P_SET_TP_RF3_DAY + 2 * 17, 1700);
        set_val(P_SET_TP_RF3_VAL + 2 * 17, 15);

        // 30天天数-19
        // 30天数值-19
        set_val(P_SET_TP_RF3_DAY + 2 * 18, 1800);
        set_val(P_SET_TP_RF3_VAL + 2 * 18, 15);

        // 30天天数-20
        // 30天数值-20
        set_val(P_SET_TP_RF3_DAY + 2 * 19, 1900);
        set_val(P_SET_TP_RF3_VAL + 2 * 19, 15);

        // 30天天数-21
        // 30天数值-21
        set_val(P_SET_TP_RF3_DAY + 2 * 20, 2000);
        set_val(P_SET_TP_RF3_VAL + 2 * 20, 20);

        // 30天天数-22
        // 30天数值-22
        set_val(P_SET_TP_RF3_DAY + 2 * 21, 2100);
        set_val(P_SET_TP_RF3_VAL + 2 * 21, 20);

        // 30天天数-23
        // 30天数值-23
        set_val(P_SET_TP_RF3_DAY + 2 * 22, 2200);
        set_val(P_SET_TP_RF3_VAL + 2 * 22, 20);

        // 30天天数-24
        // 30天数值-24
        set_val(P_SET_TP_RF3_DAY + 2 * 23, 2300);
        set_val(P_SET_TP_RF3_VAL + 2 * 23, 20);

        // 30天天数-25
        // 30天数值-25
        set_val(P_SET_TP_RF3_DAY + 2 * 24, 2400);
        set_val(P_SET_TP_RF3_VAL + 2 * 24, 20);

        // 30天天数-26
        // 30天数值-26
        set_val(P_SET_TP_RF3_DAY + 2 * 25, 2500);
        set_val(P_SET_TP_RF3_VAL + 2 * 25, 30);

        // 30天天数-27
        // 30天数值-27
        set_val(P_SET_TP_RF3_DAY + 2 * 26, 2600);
        set_val(P_SET_TP_RF3_VAL + 2 * 26, 30);

        // 30天天数-28
        // 30天数值-28
        set_val(P_SET_TP_RF3_DAY + 2 * 27, 2700);
        set_val(P_SET_TP_RF3_VAL + 2 * 27, 30);

        // 30天天数-29
        // 30天数值-29
        set_val(P_SET_TP_RF3_DAY + 2 * 28, 2800);
        set_val(P_SET_TP_RF3_VAL + 2 * 28, 30);

        // 30天天数-30
        // 30天数值-30
        set_val(P_SET_TP_RF3_DAY + 2 * 29, 2900);
        set_val(P_SET_TP_RF3_VAL + 2 * 29, 30);
    }

    // 预设值-回流温度4
    else if (S_PRE_TP_RF4 == mode)
    {
        // 冷水天数
        // 预热值
        set_val(P_PRE_TP_RF4_DAY, 900);
        set_val(P_PRE_TP_RF4, -10);

        // 30天天数-1
        // 30天数值-1
        set_val(P_SET_TP_RF4_DAY, 0);
        set_val(P_SET_TP_RF4_VAL, -10);

        // 30天天数-2
        // 30天数值-2
        set_val(P_SET_TP_RF4_DAY + 2 * 1, 100);
        set_val(P_SET_TP_RF4_VAL + 2 * 1, -10);

        // 30天天数-3
        // 30天数值-3
        set_val(P_SET_TP_RF4_DAY + 2 * 2, 200);
        set_val(P_SET_TP_RF4_VAL + 2 * 2, -10);

        // 30天天数-4
        // 30天数值-4
        set_val(P_SET_TP_RF4_DAY + 2 * 3, 300);
        set_val(P_SET_TP_RF4_VAL + 2 * 3, -10);

        // 30天天数-5
        // 30天数值-5
        set_val(P_SET_TP_RF4_DAY + 2 * 4, 400);
        set_val(P_SET_TP_RF4_VAL + 2 * 4, -10);

        // 30天天数-6
        // 30天数值-6
        set_val(P_SET_TP_RF4_DAY + 2 * 5, 500);
        set_val(P_SET_TP_RF4_VAL + 2 * 5, -10);

        // 30天天数-7
        // 30天数值-7
        set_val(P_SET_TP_RF4_DAY + 2 * 6, 600);
        set_val(P_SET_TP_RF4_VAL + 2 * 6, -10);

        // 30天天数-8
        // 30天数值-8
        set_val(P_SET_TP_RF4_DAY + 2 * 7, 700);
        set_val(P_SET_TP_RF4_VAL + 2 * 7, -10);

        // 30天天数-9
        // 30天数值-9
        set_val(P_SET_TP_RF4_DAY + 2 * 8, 800);
        set_val(P_SET_TP_RF4_VAL + 2 * 8, -10);

        // 30天天数-10
        // 30天数值-10
        set_val(P_SET_TP_RF4_DAY + 2 * 9, 900);
        set_val(P_SET_TP_RF4_VAL + 2 * 9, 20);

        // 30天天数-11
        // 30天数值-11
        set_val(P_SET_TP_RF4_DAY + 2 * 10, 1000);
        set_val(P_SET_TP_RF4_VAL + 2 * 10, 20);

        // 30天天数-12
        // 30天数值-12
        set_val(P_SET_TP_RF4_DAY + 2 * 11, 1100);
        set_val(P_SET_TP_RF4_VAL + 2 * 11, 20);

        // 30天天数-13
        // 30天数值-13
        set_val(P_SET_TP_RF4_DAY + 2 * 12, 1200);
        set_val(P_SET_TP_RF4_VAL + 2 * 12, 20);

        // 30天天数-14
        // 30天数值-14
        set_val(P_SET_TP_RF4_DAY + 2 * 13, 1300);
        set_val(P_SET_TP_RF4_VAL + 2 * 13, 20);

        // 30天天数-15
        // 30天数值-15
        set_val(P_SET_TP_RF4_DAY + 2 * 14, 1400);
        set_val(P_SET_TP_RF4_VAL + 2 * 14, 20);

        // 30天天数-16
        // 30天数值-16
        set_val(P_SET_TP_RF4_DAY + 2 * 15, 1500);
        set_val(P_SET_TP_RF4_VAL + 2 * 15, 15);

        // 30天天数-17
        // 30天数值-17
        set_val(P_SET_TP_RF4_DAY + 2 * 16, 1600);
        set_val(P_SET_TP_RF4_VAL + 2 * 16, 15);

        // 30天天数-18
        // 30天数值-18
        set_val(P_SET_TP_RF4_DAY + 2 * 17, 1700);
        set_val(P_SET_TP_RF4_VAL + 2 * 17, 15);

        // 30天天数-19
        // 30天数值-19
        set_val(P_SET_TP_RF4_DAY + 2 * 18, 1800);
        set_val(P_SET_TP_RF4_VAL + 2 * 18, 15);

        // 30天天数-20
        // 30天数值-20
        set_val(P_SET_TP_RF4_DAY + 2 * 19, 1900);
        set_val(P_SET_TP_RF4_VAL + 2 * 19, 15);

        // 30天天数-21
        // 30天数值-21
        set_val(P_SET_TP_RF4_DAY + 2 * 20, 2000);
        set_val(P_SET_TP_RF4_VAL + 2 * 20, 20);

        // 30天天数-22
        // 30天数值-22
        set_val(P_SET_TP_RF4_DAY + 2 * 21, 2100);
        set_val(P_SET_TP_RF4_VAL + 2 * 21, 20);

        // 30天天数-23
        // 30天数值-23
        set_val(P_SET_TP_RF4_DAY + 2 * 22, 2200);
        set_val(P_SET_TP_RF4_VAL + 2 * 22, 20);

        // 30天天数-24
        // 30天数值-24
        set_val(P_SET_TP_RF4_DAY + 2 * 23, 2300);
        set_val(P_SET_TP_RF4_VAL + 2 * 23, 20);

        // 30天天数-25
        // 30天数值-25
        set_val(P_SET_TP_RF4_DAY + 2 * 24, 2400);
        set_val(P_SET_TP_RF4_VAL + 2 * 24, 20);

        // 30天天数-26
        // 30天数值-26
        set_val(P_SET_TP_RF4_DAY + 2 * 25, 2500);
        set_val(P_SET_TP_RF4_VAL + 2 * 25, 30);

        // 30天天数-27
        // 30天数值-27
        set_val(P_SET_TP_RF4_DAY + 2 * 26, 2600);
        set_val(P_SET_TP_RF4_VAL + 2 * 26, 30);

        // 30天天数-28
        // 30天数值-28
        set_val(P_SET_TP_RF4_DAY + 2 * 27, 2700);
        set_val(P_SET_TP_RF4_VAL + 2 * 27, 30);

        // 30天天数-29
        // 30天数值-29
        set_val(P_SET_TP_RF4_DAY + 2 * 28, 2800);
        set_val(P_SET_TP_RF4_VAL + 2 * 28, 30);

        // 30天天数-30
        // 30天数值-30
        set_val(P_SET_TP_RF4_DAY + 2 * 29, 2900);
        set_val(P_SET_TP_RF4_VAL + 2 * 29, 30);
    }
}

//----------------------------------------------------------------------------------------------------------
// 初始化参数
//----------------------------------------------------------------------------------------------------------
void init_para()
{
    //----------------------------------------------------------------------------------------------------------
    // 参数定义
    //----------------------------------------------------------------------------------------------------------
    // 定义恢复共享内存数据备份文件名
    char sync_config_read_file[512];

    // 定义恢复状态
    uint16_t sync_config_read_status = 0;

    // 定义返回值
    int v_i_rc = 0;

    //----------------------------------------------------------------------------------------------------------
    // 恢复数据
    //----------------------------------------------------------------------------------------------------------
    // 第一次恢复数据
    //----------------------------------------------------------------------------------------------------------
    int sync_file_index = 1;
    char sync_file_name[512] = "";
    if (1 == sync_file_index)
    {
        snprintf(sync_file_name, 512, "%s_%d", g_st_jincubator.conf_sysc_file, sync_file_index);
        v_i_rc = syn_ini_to_shm(sync_file_name, 0, 5000);
        if (v_i_rc != 0)
        {
            ++sync_file_index;
            zlog_error(g_zlog_zc, "%-40s备份文件:%s 恢复失败!", "[init_para]", sync_file_name);
        }
        else
        {
            int v_i_status = (get_time(R_RUN_SECOND_HIGH) > 0) && (get_time(R_RUN_SYSTEM_SECOND_HIGH) > 0);

            if (!v_i_status)
            {
                ++sync_file_index;
                zlog_error(g_zlog_zc, "%-40s备份文件:%s 恢复失败, 重要数据为空!", "[init_para]", sync_file_name);
            }
            else
            {
                zlog_info(g_zlog_zc, "%-40s备份文件:%s 恢复成功!", "[init_para]", sync_file_name);
            }
        }
    }

    //----------------------------------------------------------------------------------------------------------
    // 第二次恢复数据
    //----------------------------------------------------------------------------------------------------------
    if (2 == sync_file_index)
    {
        snprintf(sync_file_name, 512, "%s_%d", g_st_jincubator.conf_sysc_file, sync_file_index);
        v_i_rc = syn_ini_to_shm(sync_file_name, 0, 5000);
        if (v_i_rc != 0)
        {
            ++sync_file_index;
            zlog_error(g_zlog_zc, "%-40s备份文件:%s 恢复失败!", "[init_para]", sync_file_name);
        }
        else
        {
            int v_i_status = (get_time(R_RUN_SECOND_HIGH) > 0) && (get_time(R_RUN_SYSTEM_SECOND_HIGH) > 0);
            if (!v_i_status)
            {
                ++sync_file_index;
                zlog_error(g_zlog_zc, "%-40s备份文件:%s 恢复失败, 重要数据为空!", "[init_para]", sync_file_name);
            }
            else
            {
                zlog_info(g_zlog_zc, "%-40s备份文件:%s 恢复成功!", "[init_para]", sync_file_name);
            }
        }
    }

    //----------------------------------------------------------------------------------------------------------
    // 第三次恢复数据
    //----------------------------------------------------------------------------------------------------------
    if (3 == sync_file_index)
    {
        snprintf(sync_file_name, 512, "%s_%d", g_st_jincubator.conf_sysc_file, sync_file_index);
        v_i_rc = syn_ini_to_shm(sync_file_name, 0, 5000);
        if (v_i_rc != 0)
        {
            ++sync_file_index;
            zlog_error(g_zlog_zc, "%-40s备份文件:%s 恢复失败!", "[init_para]", sync_file_name);
        }
        else
        {
            int v_i_status = (get_time(R_RUN_SECOND_HIGH) > 0) && (get_time(R_RUN_SYSTEM_SECOND_HIGH) > 0);
            if (!v_i_status)
            {
                ++sync_file_index;
                zlog_error(g_zlog_zc, "%-40s备份文件:%s 恢复失败, 重要数据为空!", "[init_para]", sync_file_name);
            }
            else
            {
                zlog_info(g_zlog_zc, "%-40s备份文件:%s 恢复成功!", "[init_para]", sync_file_name);
            }
        }
    }

    //----------------------------------------------------------------------------------------------------------
    // 第四次恢复数据
    //----------------------------------------------------------------------------------------------------------
    if (4 == sync_file_index)
    {
        snprintf(sync_file_name, 512, "%s_%d", g_st_jincubator.conf_sysc_file, sync_file_index);
        v_i_rc = syn_ini_to_shm(sync_file_name, 0, 5000);
        if (v_i_rc != 0)
        {
            ++sync_file_index;
            zlog_error(g_zlog_zc, "%-40s备份文件:%s 恢复失败!", "[init_para]", sync_file_name);
        }
        else
        {
            int v_i_status = (get_time(R_RUN_SECOND_HIGH) > 0) && (get_time(R_RUN_SYSTEM_SECOND_HIGH) > 0);
            if (!v_i_status)
            {
                ++sync_file_index;
                zlog_error(g_zlog_zc, "%-40s备份文件:%s 恢复失败, 重要数据为空!", "[init_para]", sync_file_name);
            }
            else
            {
                zlog_info(g_zlog_zc, "%-40s备份文件:%s 恢复成功!", "[init_para]", sync_file_name);
            }
        }
    }

    //----------------------------------------------------------------------------------------------------------
    // 第五次恢复数据
    //----------------------------------------------------------------------------------------------------------
    if (5 == sync_file_index)
    {
        snprintf(sync_file_name, 512, "%s_%d", g_st_jincubator.conf_sysc_file, sync_file_index);
        v_i_rc = syn_ini_to_shm(sync_file_name, 0, 5000);
        if (v_i_rc != 0)
        {
            ++sync_file_index;
            zlog_error(g_zlog_zc, "%-40s备份文件:%s 恢复失败!", "[init_para]", sync_file_name);
        }
        else
        {
            int v_i_status = (get_time(R_RUN_SECOND_HIGH) > 0) && (get_time(R_RUN_SYSTEM_SECOND_HIGH) > 0);
            if (!v_i_status)
            {
                ++sync_file_index;
                zlog_error(g_zlog_zc, "%-40s备份文件:%s 恢复失败, 重要数据为空!", "[init_para]", sync_file_name);
            }
            else
            {
                zlog_info(g_zlog_zc, "%-40s备份文件:%s 恢复成功!", "[init_para]", sync_file_name);
            }
        }
    }

    //----------------------------------------------------------------------------------------------------------
    // 第六次恢复数据
    //----------------------------------------------------------------------------------------------------------
    if (6 == sync_file_index)
    {
        snprintf(sync_file_name, 512, "%s_%d", g_st_jincubator.conf_sysc_file, sync_file_index);
        v_i_rc = syn_ini_to_shm(sync_file_name, 0, 5000);
        if (v_i_rc != 0)
        {
            ++sync_file_index;
            zlog_error(g_zlog_zc, "%-40s备份文件:%s 恢复失败!", "[init_para]", sync_file_name);
        }
        else
        {
            int v_i_status = (get_time(R_RUN_SECOND_HIGH) > 0) && (get_time(R_RUN_SYSTEM_SECOND_HIGH) > 0);
            if (!v_i_status)
            {
                ++sync_file_index;
                zlog_error(g_zlog_zc, "%-40s备份文件:%s 恢复失败, 重要数据为空!", "[init_para]", sync_file_name);
            }
            else
            {
                zlog_info(g_zlog_zc, "%-40s备份文件:%s 恢复成功!", "[init_para]", sync_file_name);
            }
        }
    }

    //----------------------------------------------------------------------------------------------------------
    // 第七次恢复数据
    //----------------------------------------------------------------------------------------------------------
    if (7 == sync_file_index)
    {
        snprintf(sync_file_name, 512, "%s_%d", g_st_jincubator.conf_sysc_file, sync_file_index);
        v_i_rc = syn_ini_to_shm(sync_file_name, 0, 5000);
        if (v_i_rc != 0)
        {
            ++sync_file_index;
            zlog_error(g_zlog_zc, "%-40s备份文件:%s 恢复失败!", "[init_para]", sync_file_name);
        }
        else
        {
            int v_i_status = (get_time(R_RUN_SECOND_HIGH) > 0) && (get_time(R_RUN_SYSTEM_SECOND_HIGH) > 0);
            if (!v_i_status)
            {
                ++sync_file_index;
                zlog_error(g_zlog_zc, "%-40s备份文件:%s 恢复失败, 重要数据为空!", "[init_para]", sync_file_name);
            }
            else
            {
                zlog_info(g_zlog_zc, "%-40s备份文件:%s 恢复成功!", "[init_para]", sync_file_name);
            }
        }
    }

    //----------------------------------------------------------------------------------------------------------
    // 第八次恢复数据
    //----------------------------------------------------------------------------------------------------------
    if (8 == sync_file_index)
    {
        snprintf(sync_file_name, 512, "%s_%d", g_st_jincubator.conf_sysc_file, sync_file_index);
        v_i_rc = syn_ini_to_shm(sync_file_name, 0, 5000);
        if (v_i_rc != 0)
        {
            ++sync_file_index;
            zlog_error(g_zlog_zc, "%-40s备份文件:%s 恢复失败!", "[init_para]", sync_file_name);
        }
        else
        {
            int v_i_status = (get_time(R_RUN_SECOND_HIGH) > 0) && (get_time(R_RUN_SYSTEM_SECOND_HIGH) > 0);
            if (!v_i_status)
            {
                ++sync_file_index;
                zlog_error(g_zlog_zc, "%-40s备份文件:%s 恢复失败, 重要数据为空!", "[init_para]", sync_file_name);
            }
            else
            {
                zlog_info(g_zlog_zc, "%-40s备份文件:%s 恢复成功!", "[init_para]", sync_file_name);
            }
        }
    }

    //----------------------------------------------------------------------------------------------------------
    // 第九次恢复数据
    //----------------------------------------------------------------------------------------------------------
    if (9 == sync_file_index)
    {
        snprintf(sync_file_name, 512, "%s_%d", g_st_jincubator.conf_sysc_file, sync_file_index);
        v_i_rc = syn_ini_to_shm(sync_file_name, 0, 5000);
        if (v_i_rc != 0)
        {
            ++sync_file_index;
            zlog_error(g_zlog_zc, "%-40s备份文件:%s 恢复失败!", "[init_para]", sync_file_name);
        }
        else
        {
            int v_i_status = (get_time(R_RUN_SECOND_HIGH) > 0) && (get_time(R_RUN_SYSTEM_SECOND_HIGH) > 0);
            if (!v_i_status)
            {
                ++sync_file_index;
                zlog_error(g_zlog_zc, "%-40s备份文件:%s 恢复失败, 重要数据为空!", "[init_para]", sync_file_name);
            }
            else
            {
                zlog_info(g_zlog_zc, "%-40s备份文件:%s 恢复成功!", "[init_para]", sync_file_name);
            }
        }
    }

    //----------------------------------------------------------------------------------------------------------
    // 第十次恢复数据
    //----------------------------------------------------------------------------------------------------------
    if (10 == sync_file_index)
    {
        snprintf(sync_file_name, 512, "%s_%d", g_st_jincubator.conf_sysc_file, sync_file_index);
        v_i_rc = syn_ini_to_shm(sync_file_name, 0, 5000);
        if (v_i_rc != 0)
        {
            ++sync_file_index;
            zlog_error(g_zlog_zc, "%-40s备份文件:%s 恢复失败!", "[init_para]", sync_file_name);
        }
        else
        {
            int v_i_status = (get_time(R_RUN_SECOND_HIGH) > 0) && (get_time(R_RUN_SYSTEM_SECOND_HIGH) > 0);
            if (!v_i_status)
            {
                ++sync_file_index;
                zlog_error(g_zlog_zc, "%-40s备份文件:%s 恢复失败, 重要数据为空!", "[init_para]", sync_file_name);
            }
            else
            {
                zlog_info(g_zlog_zc, "%-40s备份文件:%s 恢复成功!", "[init_para]", sync_file_name);
            }
        }
    }

    if (sync_file_index > 10)
    {
        snprintf(sync_file_name, 512, "%s_%d", g_st_jincubator.conf_sysc_file, sync_file_index);
        v_i_rc = syn_ini_to_shm(sync_file_name, 0, 5000);
        if (v_i_rc != 0)
        {
            ++sync_file_index;
            zlog_error(g_zlog_zc, "%-40s备份文件:%s 恢复失败!", "[init_para]", sync_file_name);
        }
    }

    if (get_val(R_SYNC_HATCH_MODE) != 0)
    {
        while (1)
        {
            zlog_debug(g_zlog_zc, "当前模式无法打开孵化器", "[init_para]", sync_file_name);
            wait_ms(60000);
        }
    }
    wait_ms(5000);

    unsigned short para_tmp[100] = {0};
    memset(para_tmp, 0, sizeof(unsigned short) * 100);

    // 初始化-温度
    if (memcmp(para_tmp, &shm_out[P_PRE_TP_MAIN_DAY], sizeof(unsigned short) * 100) == 0)
    {
        preset_para_init(S_PRE_TP_MAIN);
    }

    // 初始化-湿度
    if (memcmp(para_tmp, &shm_out[P_PRE_HM_DAY], sizeof(unsigned short) * 100) == 0)
    {
        preset_para_init(S_PRE_HM);
    }

    // 初始化-二氧化碳
    if (memcmp(para_tmp, &shm_out[P_PRE_CO_DAY], sizeof(unsigned short) * 100) == 0)
    {
        preset_para_init(S_PRE_CO);
    }

    // 初始化-风门
    if (memcmp(para_tmp, &shm_out[P_PRE_AD_DAY], sizeof(unsigned short) * 100) == 0)
    {
        preset_para_init(S_PRE_AD);
    }

    // 初始化-回流温度1
    if (memcmp(para_tmp, &shm_out[P_PRE_TP_RF1_DAY], sizeof(unsigned short) * 100) == 0)
    {
        preset_para_init(S_PRE_TP_RF1);
    }

    // 初始化-回流温度2
    if (memcmp(para_tmp, &shm_out[P_PRE_TP_RF2_DAY], sizeof(unsigned short) * 100) == 0)
    {
        preset_para_init(S_PRE_TP_RF2);
    }

    // 初始化-回流温度3
    if (memcmp(para_tmp, &shm_out[P_PRE_TP_RF3_DAY], sizeof(unsigned short) * 100) == 0)
    {
        preset_para_init(S_PRE_TP_RF3);
    }

    // 初始化-回流温度4
    if (memcmp(para_tmp, &shm_out[P_PRE_TP_RF4_DAY], sizeof(unsigned short) * 100) == 0)
    {
        preset_para_init(S_PRE_TP_RF4);
    }

    // 温度0
    if (get_uval(P_PT1_AO1) == 0)
    {
        zlog_info(g_zlog_zc, "%-40sP_PT1_AO1:%u", "[init_para]", g_st_jincubator.pt1.pt_0_temp);
        set_uval(P_PT1_AO1, g_st_jincubator.pt1.pt_0_temp);
    }

    if (get_uval(P_PT2_AO1) == 0)
    {
        zlog_info(g_zlog_zc, "%-40sP_PT2_AO1:%u", "[init_para]", g_st_jincubator.pt2.pt_0_temp);
        set_uval(P_PT2_AO1, g_st_jincubator.pt2.pt_0_temp);
    }

    if (get_uval(P_PT3_AO1) == 0)
    {
        zlog_info(g_zlog_zc, "%-40sP_PT3_AO1:%u", "[init_para]", g_st_jincubator.pt3.pt_0_temp);
        set_uval(P_PT3_AO1, g_st_jincubator.pt3.pt_0_temp);
    }

    if (get_uval(P_PT4_AO1) == 0)
    {
        zlog_info(g_zlog_zc, "%-40sP_PT4_AO1:%u", "[init_para]", g_st_jincubator.pt4.pt_0_temp);
        set_uval(P_PT4_AO1, g_st_jincubator.pt4.pt_0_temp);
    }

    if (get_uval(P_PT5_AO1) == 0)
    {
        zlog_info(g_zlog_zc, "%-40sP_PT5_AO1:%u", "[init_para]", g_st_jincubator.pt5.pt_0_temp);
        set_uval(P_PT5_AO1, g_st_jincubator.pt5.pt_0_temp);
    }

    if (get_uval(P_PT6_AO1) == 0)
    {
        zlog_info(g_zlog_zc, "%-40sP_PT6_AO2:%u", "[init_para]", g_st_jincubator.pt6.pt_0_temp);
        set_uval(P_PT6_AO1, g_st_jincubator.pt6.pt_0_temp);
    }

    // 温度40
    if (get_uval(P_PT1_AO2) == 0)
    {
        zlog_info(g_zlog_zc, "%-40sP_PT1_AO2:%u", "[init_para]", g_st_jincubator.pt1.pt_40_temp);
        set_uval(P_PT1_AO2, g_st_jincubator.pt1.pt_40_temp);
    }

    if (get_uval(P_PT2_AO2) == 0)
    {
        zlog_info(g_zlog_zc, "%-40sP_PT2_AO2:%u", "[init_para]", g_st_jincubator.pt2.pt_40_temp);
        set_uval(P_PT2_AO2, g_st_jincubator.pt2.pt_40_temp);
    }

    if (get_uval(P_PT3_AO2) == 0)
    {
        zlog_info(g_zlog_zc, "%-40sP_PT3_AO2:%u", "[init_para]", g_st_jincubator.pt3.pt_40_temp);
        set_uval(P_PT3_AO2, g_st_jincubator.pt3.pt_40_temp);
    }

    if (get_uval(P_PT4_AO2) == 0)
    {
        zlog_info(g_zlog_zc, "%-40sP_PT4_AO2:%u", "[init_para]", g_st_jincubator.pt4.pt_40_temp);
        set_uval(P_PT4_AO2, g_st_jincubator.pt4.pt_40_temp);
    }

    if (get_uval(P_PT5_AO2) == 0)
    {
        zlog_info(g_zlog_zc, "%-40sP_PT5_AO2:%u", "[init_para]", g_st_jincubator.pt5.pt_40_temp);
        set_uval(P_PT5_AO2, g_st_jincubator.pt5.pt_40_temp);
    }

    if (get_uval(P_PT6_AO2) == 0)
    {
        zlog_info(g_zlog_zc, "%-40sP_PT6_AO2:%u", "[init_para]", g_st_jincubator.pt6.pt_40_temp);
        set_uval(P_PT6_AO2, g_st_jincubator.pt6.pt_40_temp);
    }

    // 0温度-通道值
    if (get_uval(P_PT1_AO3) == 0)
    {
        zlog_info(g_zlog_zc, "%-40sP_PT1_AO3:%u", "[init_para]", g_st_jincubator.pt1.pt_0_channel);
        set_uval(P_PT1_AO3, g_st_jincubator.pt1.pt_0_channel);
    }

    if (get_uval(P_PT2_AO3) == 0)
    {
        zlog_info(g_zlog_zc, "%-40sP_PT2_AO3:%u", "[init_para]", g_st_jincubator.pt2.pt_0_channel);
        set_uval(P_PT2_AO3, g_st_jincubator.pt2.pt_0_channel);
    }

    if (get_uval(P_PT3_AO3) == 0)
    {
        zlog_info(g_zlog_zc, "%-40sP_PT3_AO3:%u", "[init_para]", g_st_jincubator.pt3.pt_0_channel);
        set_uval(P_PT3_AO3, g_st_jincubator.pt3.pt_0_channel);
    }

    if (get_uval(P_PT4_AO3) == 0)
    {
        zlog_info(g_zlog_zc, "%-40sP_PT4_AO3:%u", "[init_para]", g_st_jincubator.pt4.pt_0_channel);
        set_uval(P_PT4_AO3, g_st_jincubator.pt4.pt_0_channel);
    }

    if (get_uval(P_PT5_AO3) == 0)
    {
        zlog_info(g_zlog_zc, "%-40sP_PT5_AO3:%u", "[init_para]", g_st_jincubator.pt5.pt_0_channel);
        set_uval(P_PT5_AO3, g_st_jincubator.pt5.pt_0_channel);
    }

    if (get_uval(P_PT6_AO3) == 0)
    {
        zlog_info(g_zlog_zc, "%-40sP_PT6_AO3:%u", "[init_para]", g_st_jincubator.pt6.pt_0_channel);
        set_uval(P_PT6_AO3, g_st_jincubator.pt6.pt_0_channel);
    }

    // 40温度-通道值
    if (get_uval(P_PT1_AO4) == 0)
    {
        zlog_info(g_zlog_zc, "%-40sP_PT1_AO4:%u", "[init_para]", g_st_jincubator.pt1.pt_40_channel);
        set_uval(P_PT1_AO4, g_st_jincubator.pt1.pt_40_channel);
    }

    if (get_uval(P_PT2_AO4) == 0)
    {
        zlog_info(g_zlog_zc, "%-40sP_PT2_AO4:%u", "[init_para]", g_st_jincubator.pt2.pt_40_channel);
        set_uval(P_PT2_AO4, g_st_jincubator.pt2.pt_40_channel);
    }

    if (get_uval(P_PT3_AO4) == 0)
    {
        zlog_info(g_zlog_zc, "%-40sP_PT3_AO4:%u", "[init_para]", g_st_jincubator.pt3.pt_40_channel);
        set_uval(P_PT3_AO4, g_st_jincubator.pt3.pt_40_channel);
    }

    if (get_uval(P_PT4_AO4) == 0)
    {
        zlog_info(g_zlog_zc, "%-40sP_PT4_AO4:%u", "[init_para]", g_st_jincubator.pt4.pt_40_channel);
        set_uval(P_PT4_AO4, g_st_jincubator.pt4.pt_40_channel);
    }

    if (get_uval(P_PT5_AO4) == 0)
    {
        zlog_info(g_zlog_zc, "%-40sP_PT5_AO4:%u", "[init_para]", g_st_jincubator.pt5.pt_40_channel);
        set_uval(P_PT5_AO4, g_st_jincubator.pt5.pt_40_channel);
    }

    if (get_uval(P_PT6_AO4) == 0)
    {
        zlog_info(g_zlog_zc, "%-40sP_PT6_AO4:%u", "[init_para]", g_st_jincubator.pt6.pt_40_channel);
        set_uval(P_PT6_AO4, g_st_jincubator.pt6.pt_40_channel);
    }

    //----------------------------------------------------------------------------------------------------------
    // 回流温度 高温警报的设定
    //----------------------------------------------------------------------------------------------------------
    if (get_uval(P_RTEMP_AH) == 0)
    {
        set_uval(P_RTEMP_AH, 150);
    }

    //----------------------------------------------------------------------------------------------------------
    // 回流温度 低温警报的设定
    //----------------------------------------------------------------------------------------------------------
    if (get_uval(P_RTEMP_AL) == 0)
    {
        set_uval(P_RTEMP_AL, 150);
    }

    //----------------------------------------------------------------------------------------------------------
    // 回流温度 低温警报的温度设定模式
    //----------------------------------------------------------------------------------------------------------
    if (get_uval(P_RTEMP_ALO) == 0)
    {
        set_uval(P_RTEMP_ALO, 1);
    }

    //----------------------------------------------------------------------------------------------------------
    // 回流温度 水冷2周期次数
    //----------------------------------------------------------------------------------------------------------
    if (get_uval(P_RTEMP_ZQC) == 0)
    {
        set_uval(P_RTEMP_ZQC, 3);
    }

    //----------------------------------------------------------------------------------------------------------
    // 回流温度 周期时间
    //----------------------------------------------------------------------------------------------------------
    if (get_uval(P_RTEMP_ZQS) == 0)
    {
        set_uval(P_RTEMP_ZQS, 30);
    }

    //----------------------------------------------------------------------------------------------------------
    // 回流温度 水冷2工作时间
    //----------------------------------------------------------------------------------------------------------
    if (get_uval(P_RTEMP_GZS) == 0)
    {
        set_uval(P_RTEMP_GZS, 20);
    }

    //----------------------------------------------------------------------------------------------------------
    // 回流温度 回流温度控制灵敏度
    //----------------------------------------------------------------------------------------------------------
    if (get_uval(P_RTEMP_HYS) == 0)
    {
        set_uval(P_RTEMP_HYS, 1);
    }

    //----------------------------------------------------------------------------------------------------------
    // 辅助加热动作点
    //----------------------------------------------------------------------------------------------------------
    if (get_uval(P_AUH) == 0)
    {
        set_uval(P_AUH, 40);
    }

    //----------------------------------------------------------------------------------------------------------
    // 风门驱动（开）动作点
    //----------------------------------------------------------------------------------------------------------
    if (get_uval(P_OPE) == 0)
    {
        set_uval(P_OPE, 35);
    }

    //----------------------------------------------------------------------------------------------------------
    // 风门驱动（关）动作点
    //----------------------------------------------------------------------------------------------------------
    if (get_uval(P_CLO) == 0)
    {
        set_uval(P_CLO, 65);
    }

    //----------------------------------------------------------------------------------------------------------
    // 水冷动作点
    //----------------------------------------------------------------------------------------------------------
    if (get_uval(P_COL) == 0)
    {
        set_uval(P_COL, 40);
    }

    //----------------------------------------------------------------------------------------------------------
    // 鼓风机动作点
    //----------------------------------------------------------------------------------------------------------
    if (get_uval(P_BLO) == 0)
    {
        set_uval(P_BLO, 35);
    }

    //----------------------------------------------------------------------------------------------------------
    // 主加热控制 100％点
    //----------------------------------------------------------------------------------------------------------
    if (get_uval(P_HHI) == 0)
    {
        set_uval(P_HHI, 80);
    }

    //----------------------------------------------------------------------------------------------------------
    // 主加热控制 0％点
    //----------------------------------------------------------------------------------------------------------
    if (get_uval(P_HLO) == 0)
    {
        set_uval(P_HLO, 55);
    }

    //----------------------------------------------------------------------------------------------------------
    // 辅助加热管最低运转时间（秒）
    //----------------------------------------------------------------------------------------------------------
    if (get_uval(P_TAU) == 0)
    {
        set_uval(P_TAU, 1);
    }

    //----------------------------------------------------------------------------------------------------------
    // 风门（开）最低运转时间（秒）
    //----------------------------------------------------------------------------------------------------------
    if (get_uval(P_TOP) == 0)
    {
        set_uval(P_TOP, 1);
    }

    //----------------------------------------------------------------------------------------------------------
    // 风门（关）最低运转时间（秒）
    //----------------------------------------------------------------------------------------------------------
    if (get_uval(P_TCL) == 0)
    {
        set_uval(P_TCL, 1);
    }

    //----------------------------------------------------------------------------------------------------------
    // 水冷最低运转时间（秒）
    //----------------------------------------------------------------------------------------------------------
    if (get_uval(P_TCO) == 0)
    {
        set_uval(P_TCO, 1);
    }

    //----------------------------------------------------------------------------------------------------------
    // 鼓风机最低运转时间（秒）
    //----------------------------------------------------------------------------------------------------------
    if (get_uval(P_TBL) == 0)
    {
        set_uval(P_TBL, 1);
    }

    //----------------------------------------------------------------------------------------------------------
    // 回流温度控制灵敏度
    //----------------------------------------------------------------------------------------------------------
    if (get_uval(P_RTEMP_HYS) == 0)
    {
        set_uval(P_RTEMP_HYS, 3);
    }

    //----------------------------------------------------------------------------------------------------------
    // 高温警报的设定
    //----------------------------------------------------------------------------------------------------------
    if (get_uval(P_TEMP_AH) == 0)
    {
        set_uval(P_TEMP_AH, 50);
    }

    //----------------------------------------------------------------------------------------------------------
    // 低温警报的设定
    //----------------------------------------------------------------------------------------------------------
    if (get_uval(P_TEMP_AL) == 0)
    {
        set_uval(P_TEMP_AL, 50);
    }

    //----------------------------------------------------------------------------------------------------------
    // 低温警报的温度设定模式
    //----------------------------------------------------------------------------------------------------------
    if (get_uval(P_TEMP_ALO) == 0)
    {
        set_uval(P_TEMP_ALO, 1);
    }

    //----------------------------------------------------------------------------------------------------------
    // 输出周期
    //----------------------------------------------------------------------------------------------------------
    if (get_uval(P_TEMP_TO) == 0)
    {
        set_uval(P_TEMP_TO, 60);
    }

    //----------------------------------------------------------------------------------------------------------
    // PID控制参数 输出周期
    //----------------------------------------------------------------------------------------------------------
    if (get_uval(P_TEMP_P) == 0)
    {
        set_uval(P_TEMP_P, 6);
    }

    //----------------------------------------------------------------------------------------------------------
    // PID控制参数 积分
    //----------------------------------------------------------------------------------------------------------
    if (get_uval(P_TEMP_I) == 0)
    {
        set_uval(P_TEMP_I, 9990);
    }

    //----------------------------------------------------------------------------------------------------------
    // PID控制参数 微分
    //----------------------------------------------------------------------------------------------------------
    if (get_uval(P_TEMP_D) == 0)
    {
        set_uval(P_TEMP_D, 100);
    }

    //----------------------------------------------------------------------------------------------------------
    // 最高输出
    //----------------------------------------------------------------------------------------------------------
    if (get_uval(P_TEMP_OHI) == 0)
    {
        set_uval(P_TEMP_OHI, 100);
    }

    //----------------------------------------------------------------------------------------------------------
    // 高湿警报的设定
    //----------------------------------------------------------------------------------------------------------
    if (get_uval(P_HUM_AH) == 0)
    {
        set_uval(P_HUM_AH, 100);
    }

    //----------------------------------------------------------------------------------------------------------
    // 低湿警报的设定
    //----------------------------------------------------------------------------------------------------------
    if (get_uval(P_HUM_AL) == 0)
    {
        set_uval(P_HUM_AL, 100);
    }

    //----------------------------------------------------------------------------------------------------------
    // 低湿警报的温度设定模式
    //----------------------------------------------------------------------------------------------------------
    if (get_uval(P_HUM_ALO) == 0)
    {
        set_uval(P_HUM_ALO, 1);
    }

    // 湿度PID 输出周期
    //----------------------------------------------------------------------------------------------------------
    if (get_uval(P_HUM_TO) == 0)
    {
        set_uval(P_HUM_TO, 60);
    }

    //----------------------------------------------------------------------------------------------------------
    // 湿度PID P
    //----------------------------------------------------------------------------------------------------------
    if (get_uval(P_HUM_P) == 0)
    {
        set_uval(P_HUM_P, 10);
    }

    //----------------------------------------------------------------------------------------------------------
    // 湿度PID I
    //----------------------------------------------------------------------------------------------------------
    if (get_uval(P_HUM_I) == 0)
    {
        set_uval(P_HUM_I, 9990);
    }

    //----------------------------------------------------------------------------------------------------------
    // 湿度PID D
    //----------------------------------------------------------------------------------------------------------
    if (get_uval(P_HUM_D) == 0)
    {
        set_uval(P_HUM_D, 300);
    }

    //----------------------------------------------------------------------------------------------------------
    // 湿度PID 最高输出
    //----------------------------------------------------------------------------------------------------------
    if (get_uval(P_HUM_OHI) == 0)
    {
        set_uval(P_HUM_OHI, 60);
    }

    //----------------------------------------------------------------------------------------------------------
    // 强制中止加湿控制的温度点
    //----------------------------------------------------------------------------------------------------------
    if (get_uval(P_HUM_SPR) == 0)
    {
        set_uval(P_HUM_SPR, 50);
    }

    //----------------------------------------------------------------------------------------------------------
    // 风门开设定值
    //----------------------------------------------------------------------------------------------------------
    if (get_uval(P_HUM_SPO) == 0)
    {
        set_uval(P_HUM_SPO, 150);
    }

    //----------------------------------------------------------------------------------------------------------
    // 风门开度设定值
    //----------------------------------------------------------------------------------------------------------
    if (get_uval(P_HUM_DBO) == 0)
    {
        set_uval(P_HUM_DBO, 30);
    }

    //----------------------------------------------------------------------------------------------------------
    // 输出警报的滞后温度
    //----------------------------------------------------------------------------------------------------------
    if (get_uval(P_HYS) == 0)
    {
        set_uval(P_HYS, 1);
    }

    //----------------------------------------------------------------------------------------------------------
    // 回流温度传感器忘挂
    //----------------------------------------------------------------------------------------------------------
    if (get_uval(P_HH0) == 0)
    {
        set_uval(P_HH0, 300);
    }

    //----------------------------------------------------------------------------------------------------------
    // 翻蛋异常延时
    //----------------------------------------------------------------------------------------------------------
    if (get_uval(P_HH1) == 0)
    {
        set_uval(P_HH1, 90);
    }

    //----------------------------------------------------------------------------------------------------------
    // 测试运转时间
    //----------------------------------------------------------------------------------------------------------
    if (get_uval(P_HH3) == 0)
    {
        set_uval(P_HH3, 2);
    }

    //----------------------------------------------------------------------------------------------------------
    // 回流温度控制检测时间
    //----------------------------------------------------------------------------------------------------------
    if (get_uval(P_HH5) == 0)
    {
        set_uval(P_HH5, 3);
    }

    //----------------------------------------------------------------------------------------------------------
    // 风扇停止警报时间长度
    //----------------------------------------------------------------------------------------------------------
    if (get_uval(P_HH7) == 0)
    {
        set_uval(P_HH7, 10);
    }

    //----------------------------------------------------------------------------------------------------------
    // 翻蛋警报检测时间
    //----------------------------------------------------------------------------------------------------------
    if (get_uval(P_HH8) == 0)
    {
        set_uval(P_HH8, 30);
    }

    //----------------------------------------------------------------------------------------------------------
    // 校准时间
    //----------------------------------------------------------------------------------------------------------
    if (get_uval(P_HH9) == 0)
    {
        set_uval(P_HH9, 30);
    }

    //----------------------------------------------------------------------------------------------------------
    // 翻蛋时间
    //----------------------------------------------------------------------------------------------------------
    if (get_uval(P_HH10) == 0)
    {
        set_uval(P_HH10, 60);
    }

    //----------------------------------------------------------------------------------------------------------
    // 高湿报警延时时间
    //----------------------------------------------------------------------------------------------------------
    if (get_uval(P_HH11) == 0)
    {
        set_uval(P_HH11, 3);
    }

    //----------------------------------------------------------------------------------------------------------
    // 二氧化碳灵敏度
    //----------------------------------------------------------------------------------------------------------
    if (get_uval(P_CO2_HYS) == 0)
    {
        set_uval(P_CO2_HYS, 30);
    }

    //----------------------------------------------------------------------------------------------------------
    // 二氧化碳控制风门设定值
    //----------------------------------------------------------------------------------------------------------
    if (get_uval(P_CO2_AH) == 0)
    {
        set_uval(P_CO2_AH, 2000);
    }

    //----------------------------------------------------------------------------------------------------------
    // 二氧化碳周期时间
    //----------------------------------------------------------------------------------------------------------
    if (get_uval(P_CO2_ZQC) == 0)
    {
        set_uval(P_CO2_ZQC, 60);
    }

    //----------------------------------------------------------------------------------------------------------
    // 二氧化碳工作时间
    //----------------------------------------------------------------------------------------------------------
    if (get_uval(P_CO2_GZS) == 0)
    {
        set_uval(P_CO2_GZS, 50);
    }

    //----------------------------------------------------------------------------------------------------------
    // 翻蛋执行时间
    //----------------------------------------------------------------------------------------------------------
    if (get_uval(EGG_FLIPP_INGINTERVAL) == 0)
    {
        set_uval(EGG_FLIPP_INGINTERVAL, 10);
    }

    //----------------------------------------------------------------------------------------------------------
    // 风门执行时间
    //----------------------------------------------------------------------------------------------------------
    if (get_uval(DAMPER_TIME_LEN_ALL_CLOSE) == 0)
    {
        set_uval(DAMPER_TIME_LEN_ALL_CLOSE, 1700);
    }
    if (get_uval(DAMPER_TIME_LEN_ALL_OPEN) == 0)
    {
        set_uval(DAMPER_TIME_LEN_ALL_OPEN, 1700);
    }

    //----------------------------------------------------------------------------------------------------------
    // 回流温度模式
    //----------------------------------------------------------------------------------------------------------
    // if (get_uval(REFLUX_TEMPERATURE) == 0)
    // {
    //     set_uval(REFLUX_TEMPERATURE, 15);
    // }

    //----------------------------------------------------------------------------------------------------------
    // 数据刷新时间
    //----------------------------------------------------------------------------------------------------------
    if (get_uval(P_PT_WAIT) == 0)
    {
        set_uval(P_PT_WAIT, 200);
    }

    //----------------------------------------------------------------------------------------------------------
    // 通道值上下浮动
    //----------------------------------------------------------------------------------------------------------
    if (get_uval(P_PT_PIC) == 0)
    {
        set_uval(P_PT_PIC, 2);
    }

    //----------------------------------------------------------------------------------------------------------
    // 校准模式的时间
    //----------------------------------------------------------------------------------------------------------
    if (get_uval(P_HH9) == 0)
    {
        set_uval(P_HH9, 30);
    }

    //----------------------------------------------------------------------------------------------------------
    // 最大运行天数
    //----------------------------------------------------------------------------------------------------------
    if (get_uval(R_RUN_MAX_DAY) == 0)
    {
        set_uval(R_RUN_MAX_DAY, 3500);
    }

    //----------------------------------------------------------------------------------------------------------
    // 风门最大长度
    //----------------------------------------------------------------------------------------------------------
    if (get_uval(DAMPER_MAX_LEN) == 0)
    {
        set_uval(DAMPER_MAX_LEN, 160);
    }

    //----------------------------------------------------------------------------------------------------------
    // 风门最小长度
    //----------------------------------------------------------------------------------------------------------
    if (get_uval(DAMPER_MIN_LEN) == 0)
    {
        set_uval(DAMPER_MIN_LEN, 0);
    }

    //----------------------------------------------------------------------------------------------------------
    // 风门开单位时间内所需要的时间
    //----------------------------------------------------------------------------------------------------------
    // int v_i_fan_open_time = get_uval(DAMPER_TIME_LEN_ALL_OPEN) / (get_uval(DAMPER_MAX_LEN) - get_uval(DAMPER_MIN_LEN));
    // set_uval(DAMPER_TIME_OPEN, v_i_fan_open_time);

    //----------------------------------------------------------------------------------------------------------
    // 风门关单位时间内所需要的时间
    //----------------------------------------------------------------------------------------------------------
    // int v_i_fan_close_time = get_uval(DAMPER_TIME_LEN_ALL_CLOSE) / (get_uval(DAMPER_MAX_LEN) - get_uval(DAMPER_MIN_LEN));
    // set_uval(DAMPER_TIME_CLOSE, v_i_fan_close_time);

    //----------------------------------------------------------------------------------------------------------
    // 风门开单位时间内所需的时间
    //----------------------------------------------------------------------------------------------------------
    if (get_uval(DAMPER_TIME_OPEN) < 78)
    {
        set_uval(DAMPER_TIME_OPEN, 78);
    }

    //----------------------------------------------------------------------------------------------------------
    // 风门关单位时间内所需的时间
    //----------------------------------------------------------------------------------------------------------
    if (get_uval(DAMPER_TIME_CLOSE) < 78)
    {
        set_uval(DAMPER_TIME_CLOSE, 78);
    }

    // 初始化PID值
    set_time(PID_TEMP_SUM, 0);
    set_time(PID_HUMI_SUM, 0);
    set_time(PID_PERIOD_COUNT_MAIN_HEATER, 0);
    set_time(PID_PERIOD_COUNT_HUMI, 0);
    set_val(PID_TEMP_MODE_STATUS, 0);

    memset(shm_old, 0, sizeof(short) * 1000);

    //----------------------------------------------------------------------------------------------------------
    // 电机初始化
    //----------------------------------------------------------------------------------------------------------
    memset(&shm_out[S_SYNC_MOTOR_CONTROL_CMD], 0, sizeof(short) * 7);

    //----------------------------------------------------------------------------------------------------------
    // 初始化DO输出为空
    //----------------------------------------------------------------------------------------------------------
    memset(&shm_out[DO_INIT], 0, sizeof(short) * 24);
    memset(&shm_out[DI_INIT], 0, sizeof(short) * 24);

    //----------------------------------------------------------------------------------------------------------
    // 清除报警状态
    //----------------------------------------------------------------------------------------------------------
    memset(&shm_out[P_HH0_TIME], 0, sizeof(short) * 100);
    memset(&shm_out[THERE_IS_AN_EGG_FLIPPING_SIGNAL_ALARM], 0, sizeof(short) * 100);

    //----------------------------------------------------------------------------------------------------------
    // 测试模式初始化
    //----------------------------------------------------------------------------------------------------------
    memset(&shm_out[R_BTN_TP_MAIN], 0, sizeof(short) * 14);

    //----------------------------------------------------------------------------------------------------------
    // 初始化状态灯为空
    //----------------------------------------------------------------------------------------------------------
    memset(&shm_out[OUT_ST_FAN_STATUS], OFF, sizeof(short) * 100);

    //----------------------------------------------------------------------------------------------------------
    // PID过滤值
    //----------------------------------------------------------------------------------------------------------
    memset(&shm_out[FILTER_PID_TEMP_ARR], 0, sizeof(short) * 100);
    memset(&shm_out[FILTER_PID_HUMI_ARR], 0, sizeof(short) * 100);
    memset(&shm_out[PID_TEMP_MODE_STATUS], 0, sizeof(short) * 100);

    // 重置运行时间
    // 获取当前入孵秒数
    unsigned int current_incubation_date = get_time(R_RUN_SECOND_HIGH);
    // 获取当前系统日期
    unsigned int current_system_date = time(NULL);
    // 获取之前的系统日期
    unsigned int previous_system_date = get_time(R_SYSTEM_TIME_HIGH);
    // 获取关机时间
    unsigned int current_shutdown_time = (current_system_date - previous_system_date) + current_incubation_date;
    // 40天秒数
    unsigned int current_40_days_and_seconds = 60 * 60 * 24 * 40;

    if (current_shutdown_time < current_40_days_and_seconds)
    {
        set_time(R_RUN_SECOND_HIGH, current_shutdown_time);
    }

    //----------------------------------------------------------------------------------------------------------
    // 入孵计时状态初始化
    //----------------------------------------------------------------------------------------------------------
    // 开启入孵计时
    set_val(R_RUN_SECOND_STATUS, ON);
    // 预热状态为1
    set_uval(R_PRE_STATUS, 1);

    //----------------------------------------------------------------------------------------------------------
    // 风门校准状态
    //----------------------------------------------------------------------------------------------------------
    set_uval(STATUS_DAMPER_CHECK, OFF);

    //----------------------------------------------------------------------------------------------------------
    // 初始化翻蛋报警模式
    //----------------------------------------------------------------------------------------------------------
    set_uval(P_FAN_TIME, ON);

    //----------------------------------------------------------------------------------------------------------
    // 初始化设置系统时间
    //----------------------------------------------------------------------------------------------------------
    set_time(S_SYSTEM_TIME_HIGH, 0xffffffff);

    //----------------------------------------------------------------------------------------------------------
    // 有翻蛋信号初始化
    //----------------------------------------------------------------------------------------------------------
    set_uval(EGG_FLIPPING_SIGNAL_RINGING_STATUS, ON);
    set_uval(NO_EGG_FLIPPING_SIGNAL_RINGING_STATUS, ON);

    //----------------------------------------------------------------------------------------------------------
    // 历史事件初始化
    //----------------------------------------------------------------------------------------------------------
    set_uold(P_HH0, get_uval(P_HH0));
    set_uold(P_HH1, get_uval(P_HH1));
    set_uold(P_HH2, get_uval(P_HH2));
    set_uold(P_HH3, get_uval(P_HH3));
    set_uold(P_HH4, get_uval(P_HH4));
    set_uold(P_HH5, get_uval(P_HH5));
    set_uold(P_HH6, get_uval(P_HH6));
    set_uold(P_HH7, get_uval(P_HH7));
    set_uold(P_HH8, get_uval(P_HH8));
    set_uold(P_HH9, get_uval(P_HH9));
    set_uold(P_HH10, get_uval(P_HH10));
    set_uold(P_HH11, get_uval(P_HH11));

    //----------------------------------------------------------------------------------------------------------
    // 启动模式
    //----------------------------------------------------------------------------------------------------------
    if (get_val(RUN_MODE_STATUS))
    {
        wait_ms(get_val(RUN_MODE_TIME) * 1000);
        // 开启启动模式
        // set_run_mode(STATUS_START_SEND);
        unsigned short run_mode = get_run_mode(1);
        if (!((run_mode & STATUS_START_RECV) || (run_mode & STARTS_PRE_RECV)))
        {
            // 开启停止模式
            set_run_mode(STATUS_STOP_SENT);
        }
    }
    else
    {
        // 开启停止模式
        set_run_mode(STATUS_STOP_SENT);
    }
}

//----------------------------------------------------------------------------------------------------------
// 将共享内存写入INI文化中
//----------------------------------------------------------------------------------------------------------
int syn_shm_to_ini(const char *file_name, unsigned short index, int size)
{
    // unsigned short tmp_data[65535];
    // memcpy(tmp_data, &shm_out[index], sizeof(short) * size);
    FILE *file = NULL, *md5_file = NULL; // 文件指
    // xini_file_t ini(file);
    char c_name[256] = "";
    char c_md5_name[256] = "";
    char c_md5_data[33] = "";
    int i_ret = 0;
    snprintf(c_name, 256, "%s.dat", file_name);
    snprintf(c_md5_name, 256, "%s.md5", file_name);

    file = fopen(c_name, "w+");

    if (file == NULL)
    {
        return -1;
    }

    // if (index >= DI_INIT)
    // {
    //     memset(&tmp_data[index], 0, sizeof(short) * 48);
    // }

    // if (index >= R_BTN_TP_MAIN)
    // {
    //     memset(&tmp_data[R_BTN_TP_MAIN], 0, sizeof(short) * 14);
    // }

    // if (index >= P_HH0_TIME)
    // {
    //     memset(&tmp_data[R_BTN_TP_MAIN], 0, sizeof(short) * 100);
    // }

    // if (index >= P_EGG_FLIPPING_DIRECTION)
    // {
    //     memset(&tmp_data[P_EGG_FLIPPING_DIRECTION], 0, sizeof(short) * 100);
    // }

    // if (index >= OUT_ST_FAN_STATUS)
    // {
    //     memset(&tmp_data[OUT_ST_FAN_STATUS], 0, sizeof(short) * 100);
    // }

    // if (index >= FILTER_PID_TEMP_ARR)
    // {
    //     memset(&tmp_data[FILTER_PID_TEMP_ARR], 0, sizeof(short) * 100);
    // }

    // if (index >= FILTER_PID_HUMI_ARR)
    // {
    //     memset(&tmp_data[FILTER_PID_HUMI_ARR], 0, sizeof(short) * 100);
    // }

    // if (index >= PID_TEMP_MODE_STATUS)
    // {
    //     memset(&tmp_data[PID_TEMP_MODE_STATUS], 0, sizeof(short) * 100);
    // }

    // 写入二进制数据
    fwrite(&shm_out[index], sizeof(unsigned short), size, file);

    // 关闭文件
    fclose(file);

    i_ret = compute_md5_file(c_name, c_md5_data);

    // zlog_debug(g_zlog_zc, "%-40s%s md5 %s len %lu","[syn_shm_to_ini]", c_name, c_md5_data, strlen(c_md5_data) + 1);
    if (i_ret != 0)
    {
        return -2;
    }

    md5_file = fopen(c_md5_name, "w+");

    if (md5_file == NULL)
    {
        return -3;
    }

    // 写入二进制数据
    fwrite(c_md5_data, sizeof(unsigned char), strlen(c_md5_data) + 1, md5_file);
    fclose(md5_file);
    return 0;
}

//----------------------------------------------------------------------------------------------------------
// 从INI文件同步数据到共享内存
//----------------------------------------------------------------------------------------------------------
int syn_ini_to_shm(const char *file_name, unsigned short index, int size)
{
    FILE *file = NULL, *md5_file = NULL; // 文件指针
    char c_name[256] = "";
    char c_md5_name[256] = "";
    char c_md5_data_file[33] = "";
    char c_md5_data_shm[33] = "";
    int i_ret = 0;
    int bytes_read = 0;
    int md5_read = 0;
    int i_return = 0;

    snprintf(c_name, 256, "%s.dat", file_name);
    snprintf(c_md5_name, 256, "%s.md5", file_name);
    // printf("c_name:%s, c_md5_name:%s\n", c_name, c_md5_name);

    md5_file = fopen(c_md5_name, "r");
    // printf("md5_file:[%s]%p\n", c_md5_name, md5_file);
    if (md5_file == NULL)
    {
        return -1;
    }

    file = fopen(c_name, "r");
    // printf("md5_file:[%s]%p\n", c_name, file);
    if (file == NULL)
    {
        return -2;
    }

    bytes_read = fread(&c_md5_data_file, sizeof(unsigned char), 33, md5_file);
    fclose(md5_file);

    i_ret = compute_md5_file(c_name, c_md5_data_shm);
    // zlog_debug(g_zlog_zc, "%-40s共享内存的Md5: %s, 同步文件的Md5: %s 恢复状态: %d","[syn_ini_to_shm]", c_md5_data_shm, c_md5_data_file, i_ret);
    if (i_ret != 0)
    {
        return -3;
    }

    if (strcmp(c_md5_data_shm, c_md5_data_file) == 0)
    {
        // zlog_debug(g_zlog_zc, "%-40s文件:%s 恢复数据","[syn_ini_to_shm]", c_name);
        md5_read = fread(&shm_out[index], sizeof(unsigned short), size, file);
        fclose(file);
        return 0;
    }
    fclose(file);
    return -1;
}

//----------------------------------------------------------------------------------------------------------
// 从共享内存获取当前时间
//----------------------------------------------------------------------------------------------------------
unsigned int get_time(unsigned short addr)
{
    unsigned int m_time;
    // 获取设置时间
    memcpy(&m_time, &shm_out[addr], sizeof(unsigned int));
    dcba(&m_time, sizeof(unsigned int));
    return m_time;
}

unsigned int get_time_old(unsigned short addr)
{
    unsigned int m_time;
    // 获取设置时间
    memcpy(&m_time, &shm_old[addr], sizeof(unsigned int));
    dcba(&m_time, sizeof(unsigned int));
    return m_time;
}

//----------------------------------------------------------------------------------------------------------
// 将当前时间保存到共享内存中
//----------------------------------------------------------------------------------------------------------
void set_time(unsigned short addr, unsigned int time)
{
    unsigned int m_time = time;
    dcba(&m_time, sizeof(unsigned int));
    memcpy(&shm_out[addr], &m_time, sizeof(unsigned int));
}

void set_time_old(unsigned short addr, unsigned int time)
{
    unsigned int m_time = time;
    dcba(&m_time, sizeof(unsigned int));
    memcpy(&shm_old[addr], &m_time, sizeof(unsigned int));
}

//----------------------------------------------------------------------------------------------------------
// PID初始化
//----------------------------------------------------------------------------------------------------------
void pid_temp_init()
{
    // 温度PID值 原始值
    int g_us_pid_temp_val = pid_calc1(get_val(R_AI_TP_MAIN), get_val(P_AO_TP_MAIN));
    set_int(PID_TEMP_VALUE, g_us_pid_temp_val);
    zlog_debug(g_zlog_zc, "[PID温度]\t pid_calc1:[%d]", g_us_pid_temp_val);

    // 温度PID 过滤值
    int g_us_pid_ave_temp_val = pid_filter(g_us_pid_temp_val, (int *)&shm_out[FILTER_PID_TEMP_ARR], 10);
    // ba(&shm_out[FILTER_PID_TEMP_ARR], sizeof(short) * 10);
    set_int(PID_TEMP_NEW_FILTER_VALUE, g_us_pid_ave_temp_val);
    zlog_debug(g_zlog_zc, "[PID温度]\t pid_filter:[%d]", g_us_pid_ave_temp_val);

    // 温度PID 主加热 新值
    unsigned short pid_temp_curr_main = main_heater(g_us_pid_ave_temp_val); // Main heater duty
    set_uval(PID_TEMP_MAIN_HEATER, pid_temp_curr_main);
    zlog_debug(g_zlog_zc, "[PID温度]\t main_heater:[%u]", pid_temp_curr_main);

    // 温度PID 水冷
    int pid_cool = cool(g_us_pid_ave_temp_val); // Water cooler
    set_uval(PID_TEMP_COOL, pid_cool);
    zlog_debug(g_zlog_zc, "[PID温度]\t cool:[%d]", pid_cool);

    // 温度PID 鼓风机
    // int pid_blower = blower(g_us_pid_ave_temp_val);
    // set_uval(PID_TEMP_BLOWER, pid_blower);
    // zlog_debug(g_zlog_zc, "[PID温度]\t blower:[%d]", pid_blower);

    // 温度PID 风门开
    int pid_ad_open = db_open(g_us_pid_ave_temp_val); // DRV-BOX OPEN
    set_uval(PID_TEMP_DB_OPEN, pid_ad_open);
    zlog_debug(g_zlog_zc, "[PID温度]\t db_open:[%d]", pid_ad_open);

    // 温度PID 风门关
    int pid_ad_close = db_close(g_us_pid_ave_temp_val); // DRV-BOX CLOSE
    set_uval(PID_TEMP_DB_CLOSE, pid_ad_close);
    zlog_debug(g_zlog_zc, "[PID温度]\t db_close:[%d]", pid_ad_close);
}

void pid_humi_init()
{
    // 湿度PID 原始值
    int pid_humi_val = pid_calc2(get_val(R_AI_HM), get_val(P_AO_HM));
    set_int(PID_HUMI_VALUE, pid_humi_val);
    zlog_debug(g_zlog_zc, "[PID湿度]\t pid_calc2:[%d]", pid_humi_val);

    // 湿度PID 加湿值
    int pid_spray = spray_duty(pid_humi_val);
    set_int(PID_HUMI_NEW_HUMIDIFICATION, pid_spray);
    zlog_debug(g_zlog_zc, "[PID湿度]\t spray_duty:[%d]", pid_spray);
}

//----------------------------------------------------------------------------------------------------------
// 更新设置参数
//----------------------------------------------------------------------------------------------------------
void update_set_para()
{
    // 30天的参数
    const int SET_PARA_DAY = 30;
    // 当前的入孵天数和小数
    unsigned short now_para_time = get_uval(R_RUN_DAY);

    // 温度设定值为0
    set_uval(P_AO_TP_MAIN, get_uval(P_SET_TP_MAIN_VAL));
    // 湿度设定值为0
    set_uval(P_AO_HM, get_uval(P_SET_HM_VAL));
    // 二氧化铁设定值为0
    set_uval(P_AO_CO, get_uval(P_SET_CO_VAL));
    // 风门设定值为0
    set_uval(P_AO_AD, get_uval(P_SET_AD_VAL));
    // 回流温度1设定值为0
    set_uval(P_AO_TP_RF1, get_uval(P_AO_TP_MAIN) + get_uval(P_SET_TP_RF1_VAL));
    // 回流温度2设定值为0
    set_uval(P_AO_TP_RF2, get_uval(P_AO_TP_MAIN) + get_uval(P_SET_TP_RF2_VAL));
    // 回流温度3设定值为0
    set_uval(P_AO_TP_RF3, get_uval(P_AO_TP_MAIN) + get_uval(P_SET_TP_RF3_VAL));
    // 回流温度4设定值为0
    set_uval(P_AO_TP_RF4, get_uval(P_AO_TP_MAIN) + get_uval(P_SET_TP_RF4_VAL));

    unsigned short head_para_time = 0;
    // 更新 温度设置
    for (size_t index = 0; index < SET_PARA_DAY; index++)
    {
        // 获取当前配置时间
        unsigned short current_para_time = get_uval(P_SET_TP_MAIN_DAY + index * 2);

        if (index > 0)
        {
            head_para_time = get_uval(P_SET_TP_MAIN_DAY + (index - 1) * 2);
        }

        // 0~29天
        if (index < (SET_PARA_DAY - 1))
        {
            unsigned short next_para_time = get_uval(P_SET_TP_MAIN_DAY + (index + 1) * 2);

            if (now_para_time >= get_uval(P_SET_TP_MAIN_DAY))
            {
                // 比对符合配置
                if ((now_para_time >= head_para_time) && (now_para_time >= current_para_time) && ((now_para_time < next_para_time) || (next_para_time <= 0)))
                {
                    // 获取当前配置数值
                    short current_para_value = get_val(P_SET_TP_MAIN_VAL + index * 2);
                    zlog_info(g_zlog_zc, "%-40s更新温度配置 [%u]:%d", "[update_set_para]", current_para_time, current_para_value);

                    // 设置配置
                    set_uval(P_AO_TP_MAIN, current_para_value);
                    break;
                }
            }
            else
            {
                zlog_info(g_zlog_zc, "%-40s更新温度配置 [%u]:%d", "[update_set_para]", now_para_time, get_val(P_SET_TP_MAIN_VAL));
                // 设置配置
                set_uval(P_AO_TP_MAIN, get_val(P_SET_TP_MAIN_VAL));
            }
        }
        else
        {
            // 比对符合配置
            if ((now_para_time >= head_para_time) && (now_para_time >= current_para_time))
            {
                // 获取当前配置数值
                short current_para_value = get_val(P_SET_TP_MAIN_VAL + index * 2);
                zlog_info(g_zlog_zc, "%-40s更新温度配置 [%u]:%d", "[update_set_para]", current_para_time, current_para_value);

                // 设置配置
                set_uval(P_AO_TP_MAIN, current_para_value);
                break;
            }
        }
    }

    head_para_time = 0;
    // 更新 湿度设置
    for (size_t index = 0; index < SET_PARA_DAY; index++)
    {
        // 获取当前配置时间
        unsigned short current_para_time = get_uval(P_SET_HM_DAY + index * 2);

        if (index > 0)
        {
            head_para_time = get_uval(P_SET_TP_MAIN_DAY + (index - 1) * 2);
        }

        // 0~29天
        if (index < (SET_PARA_DAY - 1))
        {
            unsigned short next_para_time = get_uval(P_SET_HM_DAY + (index + 1) * 2);

            // 比对符合配置
            if ((now_para_time >= head_para_time) && (now_para_time >= current_para_time) && ((now_para_time < next_para_time) || (next_para_time <= 0)))
            {
                // 获取当前配置数值
                short current_para_value = get_val(P_SET_HM_VAL + index * 2);
                zlog_info(g_zlog_zc, "%-40s更新湿度配置 [%u]:%d", "[update_set_para]", current_para_time, current_para_value);
                // 设置配置
                set_uval(P_AO_HM, current_para_value);
                break;
            }
        }
        else
        {
            // 比对符合配置
            if ((now_para_time >= head_para_time) && (now_para_time >= current_para_time))
            {
                // 获取当前配置数值
                short current_para_value = get_val(P_SET_HM_VAL + index * 2);
                zlog_info(g_zlog_zc, "%-40s更新湿度配置 [%u]:%d", "[update_set_para]", current_para_time, current_para_value);
                // 设置配置
                set_uval(P_AO_HM, current_para_value);
                break;
            }
        }
    }

    head_para_time = 0;
    // 更新 二氧化碳设置
    for (size_t index = 0; index < SET_PARA_DAY; index++)
    {
        // 获取当前配置时间
        unsigned short current_para_time = get_uval(P_SET_CO_DAY + index * 2);

        if (index > 0)
        {
            head_para_time = get_uval(P_SET_TP_MAIN_DAY + (index - 1) * 2);
        }

        // 0~29天
        if (index < (SET_PARA_DAY - 1))
        {
            unsigned short next_para_time = get_uval(P_SET_CO_DAY + (index + 1) * 2);
            // 比对符合配置
            if ((now_para_time >= head_para_time) && (now_para_time >= current_para_time) && ((now_para_time < next_para_time) || (next_para_time <= 0)))
            {
                // 获取当前配置数值
                short current_para_value = get_val(P_SET_CO_VAL + index * 2);
                zlog_info(g_zlog_zc, "%-40s更新二氧化碳配置 [%u]:%d", "[update_set_para]", current_para_time, current_para_value);
                // 设置配置
                set_uval(P_AO_CO, current_para_value);
                break;
            }
        }
        else
        {
            // 比对符合配置
            if ((now_para_time >= head_para_time) && (now_para_time >= current_para_time))
            {
                // 获取当前配置数值
                short current_para_value = get_val(P_SET_CO_VAL + index * 2);
                zlog_info(g_zlog_zc, "%-40s更新二氧化碳配置 [%u]:%d", "[update_set_para]", current_para_time, current_para_value);
                // 设置配置
                set_uval(P_AO_CO, current_para_value);
                break;
            }
        }
    }

    head_para_time = 0;
    // 更新 风门设置
    for (size_t index = 0; index < SET_PARA_DAY; index++)
    {
        // 获取当前配置时间
        unsigned short current_para_time = get_uval(P_SET_AD_DAY + index * 2);
        if (index > 0)
        {
            head_para_time = get_uval(P_SET_TP_MAIN_DAY + (index - 1) * 2);
        }

        // 0~29天
        if (index < (SET_PARA_DAY - 1))
        {
            unsigned short next_para_time = get_uval(P_SET_AD_DAY + (index + 1) * 2);
            // 比对符合配置
            if ((now_para_time >= head_para_time) && (now_para_time >= current_para_time) && ((now_para_time < next_para_time) || (next_para_time <= 0)))
            {
                // 获取当前配置数值
                short current_para_value = get_val(P_SET_AD_VAL + index * 2);
                zlog_info(g_zlog_zc, "%-40s更新风门配置 [%u]:%d", "[update_set_para]", current_para_time, current_para_value);

                // 设置配置
                set_uval(P_AO_AD, current_para_value);
                break;
            }
        }
        else
        {
            // 比对符合配置
            if ((now_para_time >= head_para_time) && (now_para_time >= current_para_time))
            {
                // 获取当前配置数值
                short current_para_value = get_val(P_SET_AD_VAL + index * 2);
                zlog_info(g_zlog_zc, "%-40s更新风门配置 [%u]:%d", "[update_set_para]", current_para_time, current_para_value);

                // 设置配置
                set_uval(P_AO_AD, current_para_value);
                break;
            }
        }
    }

    head_para_time = 0;
    // 更新 回流温度1设置
    for (size_t index = 0; index < SET_PARA_DAY; index++)
    {
        // 获取当前配置时间
        unsigned short current_para_time = get_uval(P_SET_TP_RF1_DAY + index * 2);

        if (index > 0)
        {
            head_para_time = get_uval(P_SET_TP_MAIN_DAY + (index - 1) * 2);
        }

        // 0~29天
        if (index < (SET_PARA_DAY - 1))
        {
            unsigned short next_para_time = get_uval(P_SET_TP_RF1_DAY + (index + 1) * 2);
            // 比对符合配置
            if ((now_para_time >= head_para_time) && (now_para_time >= current_para_time) && ((now_para_time < next_para_time) || (next_para_time <= 0)))
            {
                // 获取当前配置数值
                short current_para_value = get_val(P_AO_TP_MAIN) + get_val(P_SET_TP_RF1_VAL + index * 2);
                zlog_info(g_zlog_zc, "%-40s更新回流温度1配置 [%u]:%d", "[update_set_para]", current_para_time, current_para_value);
                // 设置配置
                set_uval(P_AO_TP_RF1, current_para_value);
                break;
            }
        }
        else
        {
            // 比对符合配置
            if ((now_para_time >= head_para_time) && (now_para_time >= current_para_time))
            {
                // 获取当前配置数值
                short current_para_value = get_val(P_AO_TP_MAIN) + get_val(P_SET_TP_RF1_VAL + index * 2);
                zlog_info(g_zlog_zc, "%-40s更新回流温度1配置 [%u]:%d", "[update_set_para]", current_para_time, current_para_value);
                // 设置配置
                set_uval(P_AO_TP_RF1, current_para_value);
                break;
            }
        }
    }

    head_para_time = 0;
    // 更新 回流温度2设置
    for (size_t index = 0; index < SET_PARA_DAY; index++)
    {
        // 获取当前配置时间
        unsigned short current_para_time = get_uval(P_SET_TP_RF2_DAY + index * 2);
        if (index > 0)
        {
            head_para_time = get_uval(P_SET_TP_MAIN_DAY + (index - 1) * 2);
        }

        // 0~29天
        if (index < (SET_PARA_DAY - 1))
        {
            unsigned short next_para_time = get_uval(P_SET_TP_RF2_DAY + (index + 1) * 2);

            // 比对符合配置
            if ((now_para_time >= head_para_time) && (now_para_time >= current_para_time) && ((now_para_time < next_para_time) || (next_para_time <= 0)))
            {
                // 获取当前配置数值
                short current_para_value = get_val(P_AO_TP_MAIN) + get_val(P_SET_TP_RF2_VAL + index * 2);
                zlog_info(g_zlog_zc, "%-40s更新回流温度2配置 [%u]:%d", "[update_set_para]", current_para_time, current_para_value);
                // 设置配置
                set_uval(P_AO_TP_RF2, current_para_value);
                break;
            }
        }
        else
        {
            // 比对符合配置
            if ((now_para_time >= head_para_time) && (now_para_time >= current_para_time))
            {
                // 获取当前配置数值
                short current_para_value = get_val(P_AO_TP_MAIN) + get_val(P_SET_TP_RF2_VAL + index * 2);
                zlog_info(g_zlog_zc, "%-40s更新回流温度2配置 [%u]:%d", "[update_set_para]", current_para_time, current_para_value);
                // 设置配置
                set_uval(P_AO_TP_RF2, current_para_value);
                break;
            }
        }
    }

    head_para_time = 0;
    // 更新 回流温度3设置
    for (size_t index = 0; index < SET_PARA_DAY; index++)
    {
        // 获取当前配置时间
        unsigned short current_para_time = get_uval(P_SET_TP_RF3_DAY + index * 2);

        if (index > 0)
        {
            head_para_time = get_uval(P_SET_TP_RF3_DAY + (index - 1) * 2);
        }

        // 0~29天
        if (index < (SET_PARA_DAY - 1))
        {
            unsigned short next_para_time = get_uval(P_SET_TP_RF3_DAY + (index + 1) * 2);

            // 比对符合配置
            if ((now_para_time >= head_para_time) && (now_para_time >= current_para_time) && ((now_para_time < next_para_time) || (next_para_time <= 0)))
            {
                // 获取当前配置数值
                short current_para_value = get_val(P_AO_TP_MAIN) + get_val(P_SET_TP_RF3_VAL + index * 2);
                zlog_info(g_zlog_zc, "%-40s更新回流温度3配置 [%u]:%d", "[update_set_para]", current_para_time, current_para_value);
                // 设置配置
                set_uval(P_AO_TP_RF3, current_para_value);
                break;
            }
        }
        else
        {
            // 比对符合配置
            if ((now_para_time >= head_para_time) && (now_para_time >= current_para_time))
            {
                // 获取当前配置数值
                short current_para_value = get_val(P_AO_TP_MAIN) + get_val(P_SET_TP_RF3_VAL + index * 2);
                zlog_info(g_zlog_zc, "%-40s更新回流温度3配置 [%u]:%d", "[update_set_para]", current_para_time, current_para_value);
                // 设置配置
                set_uval(P_AO_TP_RF3, current_para_value);
                break;
            }
        }
    }

    head_para_time = 0;
    // 更新 回流温度4设置
    for (size_t index = 0; index < SET_PARA_DAY; index++)
    {
        // 获取当前配置时间
        unsigned short current_para_time = get_uval(P_SET_TP_RF4_DAY + index * 2);

        if (index > 0)
        {
            head_para_time = get_uval(P_SET_TP_RF4_DAY + (index - 1) * 2);
        }

        // 0~29天
        if (index < (SET_PARA_DAY - 1))
        {
            unsigned short next_para_time = get_uval(P_SET_TP_RF4_DAY + (index + 1) * 2);
            // 比对符合配置
            if ((now_para_time >= head_para_time) && (now_para_time >= current_para_time) && ((now_para_time < next_para_time) || (next_para_time <= 0)))
            {
                // 获取当前配置数值
                short current_para_value = get_val(P_AO_TP_MAIN) + get_val(P_SET_TP_RF4_VAL + index * 2);
                zlog_info(g_zlog_zc, "%-40s更新回流温度4配置 [%u]:%d", "[update_set_para]", current_para_time, current_para_value);
                // 设置配置
                set_uval(P_AO_TP_RF4, current_para_value);
                break;
            }
        }
        else
        {
            // 比对符合配置
            if ((now_para_time >= head_para_time) && (now_para_time >= current_para_time))
            {
                // 获取当前配置数值
                short current_para_value = get_val(P_AO_TP_MAIN) + get_val(P_SET_TP_RF4_VAL + index * 2);
                zlog_info(g_zlog_zc, "%-40s更新回流温度4配置 [%u]:%d", "[update_set_para]", current_para_time, current_para_value);
                // 设置配置
                set_uval(P_AO_TP_RF4, current_para_value);
                break;
            }
        }
    }
}

//----------------------------------------------------------------------------------------------------------
// 预热设置参数
//----------------------------------------------------------------------------------------------------------
void preheat_set_para()
{
    // 初始化配置，当未获取到参数时，默认为0;
    // 温度设定值为0
    set_uval(P_AO_TP_MAIN, 0);
    // 湿度设定值为0
    set_uval(P_AO_HM, 0);
    // 二氧化铁设定值为0
    set_uval(P_AO_CO, 0);
    // 风门设定值为0
    set_uval(P_AO_AD, 0);
    // 回流温度1设定值为0
    set_uval(P_AO_TP_RF1, 0);
    // 回流温度2设定值为0
    set_uval(P_AO_TP_RF2, 0);
    // 回流温度3设定值为0
    set_uval(P_AO_TP_RF3, 0);
    // 回流温度4设定值为0
    set_uval(P_AO_TP_RF4, 0);

    // 预热温度配置
    short pre_tp_main = get_val(P_PRE_TP_MAIN);
    zlog_info(g_zlog_zc, "%-40s更新预热值温度配置 [%d]", "[preheat_set_para]", pre_tp_main);
    set_uval(P_AO_TP_MAIN, pre_tp_main);

    // 预热湿度配置
    short pre_hm = get_val(P_PRE_HM);
    zlog_info(g_zlog_zc, "%-40s更新预热值湿度配置 [%d]", "[preheat_set_para]", pre_hm);
    set_uval(P_AO_HM, pre_hm);

    // 预热二氧化碳配置
    short pre_co2 = get_val(P_PRE_CO);
    zlog_info(g_zlog_zc, "%-40s更新预热值二氧化碳配置 [%d]", "[preheat_set_para]", pre_co2);
    set_uval(P_AO_CO, pre_co2);

    // 预热风门配置
    short pre_ad = get_val(P_PRE_AD);
    zlog_info(g_zlog_zc, "%-40s更新预热值风门配置 [%d]", "[preheat_set_para]", pre_ad);
    set_uval(P_AO_AD, pre_ad);

    // 预热回流温度1配置
    short pre_tp_rf1 = get_val(P_AO_TP_MAIN) + get_val(P_PRE_TP_RF1);
    zlog_info(g_zlog_zc, "%-40s更新预热值回流温度1配置 [%d]", "[preheat_set_para]", pre_tp_rf1);
    set_uval(P_AO_TP_RF1, pre_tp_rf1);

    // 预热回流温度2配置
    short pre_tp_rf2 = get_val(P_AO_TP_MAIN) + get_val(P_PRE_TP_RF2);
    zlog_info(g_zlog_zc, "%-40s更新预热值回流温度2配置 [%d]", "[preheat_set_para]", pre_tp_rf2);
    set_uval(P_AO_TP_RF2, pre_tp_rf2);

    // 预热回流温度3配置
    short pre_tp_rf3 = get_val(P_AO_TP_MAIN) + get_val(P_PRE_TP_RF3);
    zlog_info(g_zlog_zc, "%-40s更新预热值回流温度3配置 [%d]", "[preheat_set_para]", pre_tp_rf3);
    set_uval(P_AO_TP_RF3, pre_tp_rf3);

    // 预热回流温度4配置
    short pre_tp_rf4 = get_val(P_AO_TP_MAIN) + get_val(P_PRE_TP_RF4);
    zlog_info(g_zlog_zc, "%-40s更新预热值回流温度4配置 [%d]", "[preheat_set_para]", pre_tp_rf4);
    set_uval(P_AO_TP_RF4, pre_tp_rf4);
}

//----------------------------------------------------------------------------------------------------------
// 系统时间函数
//----------------------------------------------------------------------------------------------------------
void run_sys_time()
{
    zlog_debug(g_zlog_zc, "%-40s进入到系统时间函数", "[run_sys_time]");
    // 判断设置系统时间是否被更改
    unsigned int set_sys_time = get_time(S_SYSTEM_TIME_HIGH);
    if (set_sys_time != 0xffffffff)
    {
        if (set_sys_time > 0)
        {
            // 设置系统时间
            set_system_time(set_sys_time);
            system("hwclock -w");
        }
        set_time(S_SYSTEM_TIME_HIGH, 0xffffffff);
    }

    // 判断程序运行时间是否被修改
    unsigned int run_sys_second_time = get_time(S_RUN_SYSTEM_SECOND_HIGH);
    if (run_sys_second_time != 0xffffffff)
    {
        zlog_debug(g_zlog_zc, "%-40s正在设置程序运行时间 [%u]", "[run_sys_time]", run_sys_second_time);
        // 设置程序运行时间
        set_val(R_RUN_SYSTEM_SECOND_STATUS, OFF);
        set_time(R_RUN_SYSTEM_SECOND_HIGH, run_sys_second_time);
        set_val(R_RUN_SYSTEM_SECOND_STATUS, ON);
        set_time(S_RUN_SYSTEM_SECOND_HIGH, 0xffffffff);
    }

    // 判断入孵时间是否被修改
    // unsigned int run_incubation_second_time = get_time(S_RUN_SECOND_HIGH);
    // if (run_incubation_second_time != 0xffffffff)
    // {
    //     zlog_debug(g_zlog_zc, "%-40s正在设置入孵运行时间 [%u]", "[run_sys_time]", run_incubation_second_time);
    //     // 设置程序运行时间
    //     set_val(R_RUN_SECOND_STATUS, OFF);
    //     set_time(R_RUN_SECOND_HIGH, run_incubation_second_time);

    //     set_val(R_RUN_SECOND_STATUS, ON);
    //     set_time(S_RUN_SECOND_HIGH, 0xffffffff);
    // }

    // 获取系统时间
    unsigned int sys_new_time = time(NULL);

    // 预热模式不对入孵天数和日期进行计算

    // 获取运行秒数
    unsigned int sys_second_time = get_time(R_RUN_SECOND_HIGH);

    // 获取运行时间的天数和小数
    struct Time sys_time = timer(sys_second_time);
    unsigned short para_time = sys_time.days * 100 + sys_time.hours;

    // 设置天数和小时
    set_uval(R_RUN_DAY, para_time);

    // 获取入孵时间
    unsigned int init_new_time = sys_new_time - sys_second_time;
    zlog_debug(g_zlog_zc, "%-40s更新入孵时间 [%u] 系统时间 [%u] 天数和小时 [%u] 运行秒数 [%u]", "[run_sys_time]", init_new_time, sys_new_time, para_time, sys_second_time);

    // 设置入孵时间
    set_time(R_INIT_TIME_HIGH, init_new_time);

    // 时间差大于35天入孵时间更新到当前值
    if (para_time >= get_val(R_RUN_MAX_DAY))
    {
        // 开启停止模式
        set_run_mode(STATUS_STOP_SENT);
        // 启动初始化
        set_run_mode(STATUS_INIT_SEND);
    }

    // 将系统时间输出到共享内存中
    set_time(R_SYSTEM_TIME_HIGH, sys_new_time);

    // 将风门值同步到原始值
    set_val(R_INIT_AI_AD, get_val(R_AI_AD));

    // 更新风门校正值
    if ((get_old(P_EDIT_AD) != get_val(P_EDIT_AD)))
    {
        if ((get_val(P_EDIT_AD) >= get_val(DAMPER_MIN_LEN)) && (get_val(P_EDIT_AD) <= get_val(DAMPER_MAX_LEN)))
        {
            // 校准模式更新风门
            set_val(R_AI_AD, get_val(P_EDIT_AD));
        }
        set_old(P_EDIT_AD, 0xffff);
        set_val(P_EDIT_AD, 0xffff);
    }

    //----------------------------------------------------------------------------------------------------------
    // 时间改变时 触发数据变更
    //----------------------------------------------------------------------------------------------------------
    // 回流温度传感器忘挂
    //----------------------------------------------------------------------------------------------------------
    if (get_uval(P_HH0) != get_uold(P_HH0))
    {
        // 设置回流温度低温报警时间
        set_uval(RETURN_TEMP_LOW_TEMP_ALARM_TIME, get_val(P_HH0) * 60);
        // 设置回流温度低温报警时间
        // set_uval(RETURN_TEMP_HIGH_TEMP_ALARM_TIME, 0);

        set_uold(P_HH0, get_uval(P_HH0));
    }

    //----------------------------------------------------------------------------------------------------------
    // 翻蛋异常延时
    //----------------------------------------------------------------------------------------------------------
    if (get_uval(P_HH1) != get_uold(P_HH1))
    {
        // 设置有翻蛋信号报警时间为 HH1_VAL * 60
        set_uval(EGG_FLIPPING_SIGNAL_ALARM_TIME, get_uval(P_HH1) * 60);

        // 设置没有翻蛋信号报警时间为0
        // set_uval(NO_EGG_FLIPPING_SIGNAL_ALARM_TIME, 0);

        set_uold(P_HH1, get_uval(P_HH1));
    }

    //----------------------------------------------------------------------------------------------------------
    // 出雏器高温报警延时
    //----------------------------------------------------------------------------------------------------------
    if (get_uval(P_HH2) != get_uold(P_HH2))
    {
        set_uold(P_HH2, get_uval(P_HH2));
    }

    //----------------------------------------------------------------------------------------------------------
    // 测试运转时间
    //----------------------------------------------------------------------------------------------------------
    if (get_uval(P_HH3) != get_uold(P_HH3))
    {
        // 测试模式时间
        set_uval(TEST_BTN_TIME, get_val(P_HH3) * 60);
        set_uold(P_HH3, get_uval(P_HH3));
    }

    //----------------------------------------------------------------------------------------------------------
    // 消毒时间
    //----------------------------------------------------------------------------------------------------------
    if (get_uval(P_HH4) != get_uold(P_HH4))
    {
        set_uold(P_HH4, get_uval(P_HH4));
    }

    //----------------------------------------------------------------------------------------------------------
    // 回流温度控制检测时间
    //----------------------------------------------------------------------------------------------------------
    if (get_uval(P_HH5) != get_uold(P_HH5))
    {
        // 设置 回流温度 高温报警时间
        set_uval(RETURN_TEMP_HIGH_TEMP_ALARM_TIME, get_val(P_HH5) * 60);
        // 设置 回流温度 低温报警时间
        // set_uval(RETURN_TEMP_LOW_TEMP_ALARM_TIME, 0);
        set_uold(P_HH5, get_uval(P_HH5));
    }

    //----------------------------------------------------------------------------------------------------------
    // 干燥开关最长运行时间
    //----------------------------------------------------------------------------------------------------------
    if (get_uval(P_HH6) != get_uold(P_HH6))
    {
        set_uold(P_HH6, get_uval(P_HH6));
    }

    //----------------------------------------------------------------------------------------------------------
    // 风扇停止警报时间长度
    //----------------------------------------------------------------------------------------------------------
    if (get_uval(P_HH7) != get_uold(P_HH7))
    {
        // 风机停止报警时间
        set_uval(FAN_ALARM_TIME, get_val(P_HH7) * 60);
        set_uold(P_HH7, get_uval(P_HH7));
    }

    //----------------------------------------------------------------------------------------------------------
    // 翻蛋警报检测时间
    //----------------------------------------------------------------------------------------------------------
    if (get_uval(P_HH8) != get_uold(P_HH8))
    {
        // 设置 没有 翻蛋报警信号的时间为  (HH8_VAL * 60)
        set_uval(NO_EGG_FLIPPING_SIGNAL_ALARM_TIME, get_uval(P_HH8) * 60);
        // 设置 有翻蛋 报警信号的时间为0
        // set_uval(EGG_FLIPPING_SIGNAL_ALARM_TIME, 0);

        set_uold(P_HH8, get_uval(P_HH8));
    }

    //----------------------------------------------------------------------------------------------------------
    // 风门开10MM后 再启动的延时时间
    //----------------------------------------------------------------------------------------------------------
    if (get_uval(P_HH9) != get_uold(P_HH9))
    {
        // 设置校准时间
        set_uval(CALIBRATION_RUN_TIME, get_uval(P_HH9) * 60);

        set_uold(P_HH9, get_uval(P_HH9));
    }

    //----------------------------------------------------------------------------------------------------------
    // 翻蛋时间
    //----------------------------------------------------------------------------------------------------------
    if (get_uval(P_HH10) != get_uold(P_HH10))
    {
        set_val(P_EGG_FLIPPING_TIME, get_val(P_HH10) * 60);
        set_uold(P_HH10, get_uval(P_HH10));
    }

    //----------------------------------------------------------------------------------------------------------
    // 高湿报警延时时间
    //----------------------------------------------------------------------------------------------------------
    if (get_uval(P_HH11) != get_uold(P_HH11))
    {
        set_uval(HIGH_HUMIDITY_ALARM_DELAY_TIME, get_uval(P_HH11) * 60);
        set_uold(P_HH11, get_uval(P_HH11));
    }
}

//----------------------------------------------------------------------------------------------------------
// 测试控制模式
//----------------------------------------------------------------------------------------------------------
void test_control_mode()
{
    zlog_debug(g_zlog_zc, "%-40s进入到测试控制模式", "[test_control_mode]");
    unsigned short status_test_control = get_test_mode();
    if ((status_test_control > 0) && (get_val(TEST_CONTROL_MODE) == 0))
    {
        switch (status_test_control)
        {
        case 0x01:
        {
            // 传感器无异常
            // 主加热按钮
            set_val(TEST_MAIN_HEAT_MODE, ON);
            set_val(CONTROL_MAIN_HEAT, ON);
            zlog_debug(g_zlog_zc, "%-40s触发开启 主加热按钮", "[test_control_mode]");
            set_val(TEST_CONTROL_MODE, status_test_control);
            set_uval(TEST_BTN_TIME, get_val(P_HH3) * 60);

            break;
        }
        case 0x02:
        {
            // 附加热按钮
            set_val(TEST_AUX_HEAT_MODE, ON);
            set_val(CONTROL_AUX_HEAT, ON);
            zlog_debug(g_zlog_zc, "%-40s触发开启 附加热按钮", "[test_control_mode]");
            set_val(TEST_CONTROL_MODE, status_test_control);
            set_uval(TEST_BTN_TIME, get_val(P_HH3) * 60);
            break;
        }
        case 0x04:
        {
            if (get_val(DETECT_DAMPER_FULLY_OPEN) == 0)
            {
                // 开门按钮
                set_val(TEST_DAMPER_OPEN_MODE, ON);
                control_open_damper(ON);
                zlog_debug(g_zlog_zc, "%-40s触发开启 开门按钮", "[test_control_mode]");
                set_uval(TEST_CONTROL_MODE, status_test_control);
                set_uval(TEST_BTN_TIME, get_val(P_HH3) * 60);
            }
            break;
        }
        case 0x08:
        {
            if (get_val(DETECT_DAMPER_FULLY_CLOSE) == 0)
            {
                // 关门按钮
                set_val(TEST_DAMPER_CLOSE_MODE, ON);
                control_close_damper(ON, 0);
                zlog_debug(g_zlog_zc, "%-40s触发开启 关门按钮", "[test_control_mode]");
                set_uval(TEST_CONTROL_MODE, status_test_control);
                set_uval(TEST_BTN_TIME, get_val(P_HH3) * 60);
            }
            break;
        }
        case 0x10:
        {
            // 传感器无异常
            // 加湿按钮
            set_val(TEST_SPRAY_MODE, ON);
            set_val(CONTROL_SPRAY, ON);
            zlog_debug(g_zlog_zc, "%-40s触发开启 加湿按钮", "[test_control_mode]");
            set_uval(TEST_CONTROL_MODE, status_test_control);
            set_uval(TEST_BTN_TIME, get_val(P_HH3) * 60);
            break;
        }
        case 0x20:
        {
            set_val(TEST_FILP_EGG_MODE, ON);
            // 翻蛋按钮
            unsigned short v_uc_egg_filpping_direction = get_uval(P_EGG_FLIPPING_DIRECTION);
            // 翻蛋时间
            unsigned short v_i_egg_filping_time = get_uval(P_HH10) * 60;

            // 当翻蛋方向为左时
            if (v_uc_egg_filpping_direction)
            {
                // 执行翻蛋控制输出
                set_val(P_EGG_FLIPPING_EXECUTION_STATUS, ON);
                // 开始翻蛋计数
                set_val(P_EGG_FLIPPING_TIME, v_i_egg_filping_time);
                // 切换翻蛋方向
                set_val(P_EGG_FLIPPING_DIRECTION, !v_uc_egg_filpping_direction);
            }
            // 当翻蛋方向为右时
            else
            {
                // 执行翻蛋控制输出
                set_val(P_EGG_FLIPPING_EXECUTION_STATUS, ON);
                // 开始翻蛋计数
                set_val(P_EGG_FLIPPING_TIME, v_i_egg_filping_time);
                // 切换翻蛋方向
                set_val(P_EGG_FLIPPING_DIRECTION, !v_uc_egg_filpping_direction);
            }

            set_val(EGG_FLIPPING_TRIGGER_STATUS, ON);
            zlog_debug(g_zlog_zc, "%-40s触发开启 翻蛋控制", "[test_control_mode]");
            set_uval(TEST_BTN_TIME, get_val(P_HH3) * 60);
            set_uval(TEST_CONTROL_MODE, status_test_control);
            break;
        }
        case 0x40:
        {
            // 水冷按钮
            set_val(TEST_COOL_MODE, ON);
            set_val(CONTROL_COOL, ON);
            zlog_debug(g_zlog_zc, "%-40s触发开启 水冷按钮", "[test_control_mode]");
            set_uval(TEST_CONTROL_MODE, status_test_control);
            set_uval(TEST_BTN_TIME, get_val(P_HH3) * 60);
            break;
        }
        case 0x80:
        {
            // 传感器无异常

            // 辅助水冷1按钮
            set_val(TEST_COOL2_1_MODE, ON);
            // control_cool2_1(ON);
            set_val(CONTROL_COOL2_1, ON);
            zlog_debug(g_zlog_zc, "%-40s触发开启 辅助水冷1按钮", "[test_control_mode]");
            set_uval(TEST_CONTROL_MODE, status_test_control);
            set_uval(TEST_BTN_TIME, get_val(P_HH3) * 60);

            break;
        }
        case 0x100:
        {
            // 传感器无异常

            // 辅助水冷2按钮
            set_val(TEST_COOL2_2_MODE, ON);
            set_val(CONTROL_COOL2_2, ON);
            zlog_debug(g_zlog_zc, "%-40s触发开启 辅助水冷2按钮", "[test_control_mode]");
            set_uval(TEST_CONTROL_MODE, status_test_control);
            set_uval(TEST_BTN_TIME, get_val(P_HH3) * 60);

            break;
        }
        case 0x200:
        {
            // 传感器无异常

            // 辅助水冷3按钮
            set_val(TEST_COOL2_3_MODE, ON);
            // control_cool2_3(ON);
            set_val(CONTROL_COOL2_3, ON);
            zlog_debug(g_zlog_zc, "%-40s触发开启 辅助水冷3按钮", "[test_control_mode]");
            set_uval(TEST_CONTROL_MODE, status_test_control);
            set_uval(TEST_BTN_TIME, get_val(P_HH3) * 60);

            break;
        }
        case 0x400:
        {
            // 传感器无异常

            // 辅助水冷4按钮
            set_val(TEST_COOL2_4_MODE, ON);
            // control_cool2_4(ON);
            set_val(CONTROL_COOL2_4, ON);
            zlog_debug(g_zlog_zc, "%-40s触发开启 辅助水冷4按钮", "[test_control_mode]");
            set_uval(TEST_CONTROL_MODE, status_test_control);
            set_uval(TEST_BTN_TIME, get_val(P_HH3) * 60);

            break;
        }
        case 0x800:
        {
            // 压板升按钮
            set_val(TEST_BLOWER_MODE, ON);
            // 判断 风机状态 执行 压板控制
            if (get_val(CONTROL_FAN) == ON)
            {
                set_val(CONTROL_BLOWER, OFF);
            }
            else
            {
                set_val(CONTROL_BLOWER, ON);
            }

            zlog_debug(g_zlog_zc, "%-40s触发开启 鼓风机", "[test_control_mode]");
            set_uval(TEST_CONTROL_MODE, status_test_control);
            set_uval(TEST_BTN_TIME, get_val(P_HH3) * 60);
            break;
        }
        default:
        {
            set_test_mode(0);
        }
        }
    }
    else if ((status_test_control == 0) && (get_val(TEST_CONTROL_MODE) > 0))
    {
        set_uval(TEST_BTN_TIME, 0);
        switch (get_uval(TEST_CONTROL_MODE))
        {
        case 0x01:
        {
            // 主加热按钮
            set_val(CONTROL_MAIN_HEAT, OFF);
            set_val(TEST_MAIN_HEAT_MODE, OFF);
            set_test_mode(0);
            set_val(TEST_CONTROL_MODE, OFF);
            zlog_debug(g_zlog_zc, "%-40s触发关闭 主加热按钮", "[test_control_mode]");
            break;
        }
        case 0x02:
        {
            // 附加热按钮
            set_val(CONTROL_AUX_HEAT, OFF);
            set_val(TEST_AUX_HEAT_MODE, OFF);
            set_test_mode(0);
            set_val(TEST_CONTROL_MODE, OFF);
            zlog_debug(g_zlog_zc, "%-40s触发关闭 附加热按钮", "[test_control_mode]");
            break;
        }
        case 0x04:
        {
            // 开门按钮
            control_open_damper(OFF);
            set_val(TEST_DAMPER_OPEN_MODE, OFF);
            set_test_mode(0);
            set_val(TEST_CONTROL_MODE, OFF);
            zlog_debug(g_zlog_zc, "%-40st触发关闭 开门按钮", "[test_control_mode]");
            break;
        }
        case 0x08:
        {
            // 关门按钮
            control_close_damper(OFF, 0);
            set_val(TEST_DAMPER_CLOSE_MODE, OFF);
            set_test_mode(0);
            set_val(TEST_CONTROL_MODE, OFF);
            zlog_debug(g_zlog_zc, "%-40s触发关闭 关门按钮", "[test_control_mode]");
            break;
        }
        case 0x10:
        {
            // 加湿按钮
            set_val(CONTROL_SPRAY, OFF);
            set_val(TEST_SPRAY_MODE, OFF);
            set_test_mode(0);
            set_val(TEST_CONTROL_MODE, OFF);
            zlog_debug(g_zlog_zc, "%-40s触发关闭 加湿按钮", "[test_control_mode]");
            break;
        }
        case 0x20:
        {
            // 翻蛋按钮
            set_val(P_EGG_FLIPPING_EXECUTION_STATUS, OFF);
            set_val(TEST_FILP_EGG_MODE, OFF);
            set_test_mode(0);
            set_val(TEST_CONTROL_MODE, OFF);
            zlog_debug(g_zlog_zc, "%-40s触发关闭 翻蛋控制", "[test_control_mode]");
            break;
        }
        case 0x40:
        {
            // 水冷按钮
            set_val(CONTROL_COOL, OFF);
            set_val(TEST_COOL_MODE, OFF);
            set_test_mode(0);
            set_val(TEST_CONTROL_MODE, OFF);
            zlog_debug(g_zlog_zc, "%-40s触发关闭 水冷按钮", "[test_control_mode]");
            break;
        }
        case 0x80:
        {
            // 辅助水冷1按钮
            // control_cool2_1(OFF);
            set_val(CONTROL_COOL2_1, OFF);
            set_val(TEST_COOL2_1_MODE, OFF);
            set_test_mode(0);
            set_val(TEST_CONTROL_MODE, OFF);
            zlog_debug(g_zlog_zc, "%-40s触发关闭 辅助水冷1按钮", "[test_control_mode]");
            break;
        }
        case 0x100:
        {
            // 辅助水冷2按钮
            // control_cool2_2(OFF);
            set_val(CONTROL_COOL2_2, OFF);
            set_val(TEST_COOL2_2_MODE, OFF);
            set_test_mode(0);
            set_val(TEST_CONTROL_MODE, OFF);
            zlog_debug(g_zlog_zc, "%-40s触发关闭 辅助水冷2按钮", "[test_control_mode]");
            break;
        }
        case 0x200:
        {
            // 辅助水冷3按钮
            // control_cool2_3(OFF);
            set_val(CONTROL_COOL2_3, OFF);
            set_val(TEST_COOL2_3_MODE, OFF);
            set_test_mode(0);
            set_val(TEST_CONTROL_MODE, OFF);
            zlog_debug(g_zlog_zc, "%-40s触发关闭 辅助水冷3按钮", "[test_control_mode]");
            break;
        }
        case 0x400:
        {
            // 辅助水冷4按钮
            // control_cool2_4(OFF);
            set_val(CONTROL_COOL2_4, OFF);
            set_val(TEST_COOL2_4_MODE, OFF);
            set_test_mode(0);
            set_val(TEST_CONTROL_MODE, OFF);
            zlog_debug(g_zlog_zc, "%-40s触发关闭 辅助水冷4按钮", "[test_control_mode]");
            break;
        }
        case 0x800:
        {
            // 压板升按钮
            set_val(CONTROL_BLOWER, OFF);
            set_val(TEST_BLOWER_MODE, OFF);
            set_test_mode(0);
            set_val(TEST_CONTROL_MODE, OFF);
            zlog_debug(g_zlog_zc, "%-40s触发关闭 鼓风机", "[test_control_mode]");
            break;
        }
        default:
        {
            set_test_mode(0);
        }
        }
    }
    else if ((status_test_control > 0) && (get_val(TEST_CONTROL_MODE) > 0) && (status_test_control == get_val(TEST_CONTROL_MODE)))
    {
        if (get_uval(TEST_BTN_TIME) == 0)
        {
            switch (get_uval(TEST_CONTROL_MODE))
            {
            case 0x01:
            {
                // 主加热按钮
                set_val(CONTROL_MAIN_HEAT, OFF);
                set_val(TEST_MAIN_HEAT_MODE, OFF);
                set_test_mode(0);
                set_val(TEST_CONTROL_MODE, OFF);
                zlog_debug(g_zlog_zc, "%-40s触发关闭 主加热按钮", "[test_control_mode]");
                break;
            }
            case 0x02:
            {
                // 附加热按钮
                set_val(CONTROL_AUX_HEAT, OFF);
                set_val(TEST_AUX_HEAT_MODE, OFF);
                set_test_mode(0);
                set_val(TEST_CONTROL_MODE, OFF);
                zlog_debug(g_zlog_zc, "%-40s触发关闭 附加热按钮", "[test_control_mode]");
                break;
            }
            case 0x04:
            {
                // 开门按钮
                control_open_damper(OFF);
                set_val(TEST_DAMPER_OPEN_MODE, OFF);
                set_test_mode(0);
                set_val(TEST_CONTROL_MODE, OFF);
                zlog_debug(g_zlog_zc, "%-40st触发关闭 开门按钮", "[test_control_mode]");
                break;
            }
            case 0x08:
            {
                // 关门按钮
                control_close_damper(OFF, 0);
                set_val(TEST_DAMPER_CLOSE_MODE, OFF);
                set_test_mode(0);
                set_val(TEST_CONTROL_MODE, OFF);
                zlog_debug(g_zlog_zc, "%-40s触发关闭 关门按钮", "[test_control_mode]");
                break;
            }
            case 0x10:
            {
                // 加湿按钮
                set_val(CONTROL_SPRAY, OFF);
                set_val(TEST_SPRAY_MODE, OFF);
                set_test_mode(0);
                set_val(TEST_CONTROL_MODE, OFF);
                zlog_debug(g_zlog_zc, "%-40s触发关闭 加湿按钮", "[test_control_mode]");
                break;
            }
            case 0x20:
            {
                // 翻蛋按钮
                set_val(P_EGG_FLIPPING_EXECUTION_STATUS, OFF);
                set_val(TEST_FILP_EGG_MODE, OFF);
                set_test_mode(0);
                set_val(TEST_CONTROL_MODE, OFF);
                zlog_debug(g_zlog_zc, "%-40s触发关闭 翻蛋控制", "[test_control_mode]");
                break;
            }
            case 0x40:
            {
                // 水冷按钮
                set_val(CONTROL_COOL, OFF);
                set_val(TEST_COOL_MODE, OFF);
                set_test_mode(0);
                set_val(TEST_CONTROL_MODE, OFF);
                zlog_debug(g_zlog_zc, "%-40s触发关闭 水冷按钮", "[test_control_mode]");
                break;
            }
            case 0x80:
            {
                // 辅助水冷1按钮
                // control_cool2_1(OFF);
                set_val(CONTROL_COOL2_1, OFF);
                set_val(TEST_COOL2_1_MODE, OFF);
                set_test_mode(0);
                set_val(TEST_CONTROL_MODE, OFF);
                zlog_debug(g_zlog_zc, "%-40s触发关闭 辅助水冷1按钮", "[test_control_mode]");
                break;
            }
            case 0x100:
            {
                // 辅助水冷2按钮
                // control_cool2_2(OFF);
                set_val(CONTROL_COOL2_2, OFF);
                set_val(TEST_COOL2_2_MODE, OFF);
                set_test_mode(0);
                set_val(TEST_CONTROL_MODE, OFF);
                zlog_debug(g_zlog_zc, "%-40s触发关闭 辅助水冷2按钮", "[test_control_mode]");
                break;
            }
            case 0x200:
            {
                // 辅助水冷3按钮
                // control_cool2_3(OFF);
                set_val(CONTROL_COOL2_3, OFF);
                set_val(TEST_COOL2_3_MODE, OFF);
                set_test_mode(0);
                set_val(TEST_CONTROL_MODE, OFF);
                zlog_debug(g_zlog_zc, "%-40s触发关闭 辅助水冷3按钮", "[test_control_mode]");
                break;
            }
            case 0x400:
            {
                // 辅助水冷4按钮
                // control_cool2_4(OFF);
                set_val(CONTROL_COOL2_4, OFF);
                set_val(TEST_COOL2_4_MODE, OFF);
                set_test_mode(0);
                set_val(TEST_CONTROL_MODE, OFF);
                zlog_debug(g_zlog_zc, "%-40s触发关闭 辅助水冷4按钮", "[test_control_mode]");
                break;
            }
            case 0x800:
            {
                // 压板升按钮
                set_val(CONTROL_BLOWER, OFF);
                set_val(TEST_BLOWER_MODE, OFF);
                set_test_mode(0);
                set_val(TEST_CONTROL_MODE, OFF);
                zlog_debug(g_zlog_zc, "%-40s触发关闭 鼓风机", "[test_control_mode]");
                break;
            }
            default:
            {
                set_test_mode(0);
            }
            }
        }
    }
    else if ((status_test_control > 0) && (get_val(TEST_CONTROL_MODE) > 0) && (status_test_control != get_val(TEST_CONTROL_MODE)))
    {
        set_uval(TEST_BTN_TIME, 0);

        // 关闭主加热
        set_val(CONTROL_MAIN_HEAT, OFF);
        set_val(TEST_MAIN_HEAT_MODE, OFF);

        // 关闭附加热
        set_val(CONTROL_AUX_HEAT, OFF);
        set_val(TEST_AUX_HEAT_MODE, OFF);

        // 关闭开门
        control_open_damper(OFF);
        set_val(TEST_DAMPER_OPEN_MODE, OFF);

        // 关闭关门
        control_close_damper(OFF, 0);
        set_val(TEST_DAMPER_CLOSE_MODE, OFF);

        // 关闭加湿
        set_val(CONTROL_SPRAY, OFF);
        set_val(TEST_SPRAY_MODE, OFF);

        // 关闭翻蛋
        set_val(P_EGG_FLIPPING_EXECUTION_STATUS, OFF);
        set_val(TEST_FILP_EGG_MODE, OFF);

        // 关闭水冷
        set_val(CONTROL_COOL, OFF);
        set_val(TEST_COOL_MODE, OFF);

        // 关闭 辅助水冷1
        // control_cool2_1(OFF);
        set_val(CONTROL_COOL2_1, OFF);
        set_val(TEST_COOL2_1_MODE, OFF);

        // 关闭 辅助水冷2
        // control_cool2_2(OFF);
        set_val(CONTROL_COOL2_2, OFF);
        set_val(TEST_COOL2_2_MODE, OFF);

        // 关闭 辅助水冷3
        // control_cool2_3(OFF);
        set_val(CONTROL_COOL2_3, OFF);
        set_val(TEST_COOL2_3_MODE, OFF);

        // 关闭 辅助水冷4
        // control_cool2_4(OFF);
        set_val(CONTROL_COOL2_4, OFF);
        set_val(TEST_COOL2_4_MODE, OFF);

        // 关闭  压板升
        set_val(CONTROL_BLOWER, OFF);
        set_val(TEST_BLOWER_MODE, OFF);

        // 清空测试
        set_test_mode(0);

        // 清空
        set_val(TEST_CONTROL_MODE, OFF);
    }
    else if ((status_test_control == 0))
    {
        set_uval(TEST_BTN_TIME, 0);

        // 关闭主加热
        set_val(TEST_MAIN_HEAT_MODE, OFF);

        // 关闭附加热
        set_val(TEST_AUX_HEAT_MODE, OFF);

        // 关闭开门
        set_val(TEST_DAMPER_OPEN_MODE, OFF);

        // 关闭关门
        set_val(TEST_DAMPER_CLOSE_MODE, OFF);

        // 关闭加湿
        set_val(TEST_SPRAY_MODE, OFF);

        // 关闭翻蛋
        set_val(TEST_FILP_EGG_MODE, OFF);

        // 关闭水冷
        set_val(TEST_COOL_MODE, OFF);

        // 关闭 辅助水冷1
        set_val(TEST_COOL2_1_MODE, OFF);

        // 关闭 辅助水冷2
        set_val(TEST_COOL2_2_MODE, OFF);

        // 关闭 辅助水冷3
        set_val(TEST_COOL2_3_MODE, OFF);

        // 关闭 辅助水冷4
        set_val(TEST_COOL2_4_MODE, OFF);

        // 关闭  压板升
        set_val(TEST_BLOWER_MODE, OFF);
        // 清空
        set_val(TEST_CONTROL_MODE, OFF);
    }
}

//----------------------------------------------------------------------------------------------------------
// 风门控制函数
//----------------------------------------------------------------------------------------------------------
void damper_control_func()
{
    zlog_debug(g_zlog_zc, "%-40s进入到风门控制函数", "[damper_control_func]");
    // 正在校正风门，到全关位置，校正完成变量显示0FF

    // 获取风门当前值
    unsigned short current_fan_value = get_val(R_AI_AD);
    // 获取风门设定值
    unsigned short setting_fan_value = get_val(P_AO_AD);

    // 风门校验
    if (get_val(STATUS_DAMPER_CHECK))
    {
        // 先将风门关到位
        if (!get_val(CONTROL_DB_OPEN_CALIBRATION_STATUS) && !get_val(CONTROL_DB_CLISE_CALIBRATION_STATUS))
        {
            // 当风门关到位
            if (get_val(DETECT_DAMPER_FULLY_CLOSE))
            {
                // 关闭风门
                control_damper(1, OFF, 0);

                // 改变状态
                set_val(CONTROL_DB_OPEN_CALIBRATION_STATUS, ON);
                zlog_debug(g_zlog_zc, "%-40s风门校验 关门到开门 开始计时", "[damper_control_func]");
            }
            else
            {
                zlog_debug(g_zlog_zc, "%-40s风门校验 风门关", "[damper_control_func]");
                control_damper(2, ON, 0);
            }
        }
        // 从风门关到位，移到开到位
        else if (get_val(CONTROL_DB_OPEN_CALIBRATION_STATUS) && !get_val(CONTROL_DB_CLISE_CALIBRATION_STATUS))
        {
            // 当风门关到位
            if (current_fan_value >= setting_fan_value || get_val(DETECT_DAMPER_FULLY_OPEN))
            {
                // 关闭风门
                control_damper(1, OFF, 0);

                // 清除风门校准状态
                set_val(STATUS_DAMPER_CHECK, 0);

                // 改变状态
                set_val(CONTROL_DB_CLISE_CALIBRATION_STATUS, ON);
                zlog_debug(g_zlog_zc, "%-40s风门校验 开门到关门 开始计时", "[damper_control_func]");
            }
            else
            {
                zlog_debug(g_zlog_zc, "%-40s风门校验 风门关到风门开", "[damper_control_func]");
                control_damper(1, ON, 0);
            }
        }
    }
    else
    {
        // 清空风门校准状态
        set_val(CONTROL_DB_OPEN_CALIBRATION_STATUS, 0);
        set_val(CONTROL_DB_CLISE_CALIBRATION_STATUS, 0);
        set_val(CONTROL_DB_CLOSE_TO_OPEN_TIME_STATUS, 0);
        set_val(CONTROL_DB_OPEN_TO_CLOSE_TIME_STATUS, 0);

        // 风门设定值为0
        if (setting_fan_value == 0)
        {
            // 高温报警
            if (get_val(HIGH_TEMP_ALARM_STATUS))
            {
                // 开启风门
                control_damper(1, ON, 0);
                zlog_debug(g_zlog_zc, "%-40s风门设定值0 高温报警 风门开", "[damper_control_func]");
            }

            // 二氧化碳报警
            else if (get_val(CARBON_DIOXIDE_WARNING_LIGHT))
            {
                if (get_uval(CARBON_DIOXIDE_OPERATING_TIME) > 0)
                {
                    // 开启风门
                    control_damper(1, ON, 0);
                    zlog_debug(g_zlog_zc, "%-40s风门设定值0 二氧化碳报警 风门开", "[damper_control_func]");
                }
                else
                {
                    if (!get_val(CARBON_DIOXIDE_INIT))
                    {
                        // 设置控制风门的停止时间
                        unsigned short fan_stop_time = get_uval(P_CO2_ZQC) - get_uval(P_CO2_GZS);
                        set_uval(CARBON_DIOXIDE_OPERATING_TIME, 0);
                        set_uval(CARBON_DIOXIDE_STOP_TIME, fan_stop_time);
                        set_val(CARBON_DIOXIDE_INIT, 1);
                    }
                }

                if (get_uval(CARBON_DIOXIDE_STOP_TIME) > 0)
                {
                    // 风门停止
                    control_damper(1, OFF, 0);
                    zlog_debug(g_zlog_zc, "%-40s风门设定值0 二氧化碳报警 风门停止", "[damper_control_func]");
                }
                else
                {
                    if (get_val(CARBON_DIOXIDE_INIT) == 1)
                    {
                        // 设置控制风门的运行时间
                        set_uval(CARBON_DIOXIDE_OPERATING_TIME, get_uval(P_CO2_GZS));
                        set_uval(CARBON_DIOXIDE_STOP_TIME, 0);
                        set_val(CARBON_DIOXIDE_INIT, 0);
                    }
                }

                if (!get_uval(CARBON_DIOXIDE_STOP_TIME) && !get_uval(CARBON_DIOXIDE_OPERATING_TIME))
                {
                    // 风门停止
                    control_damper(1, OFF, 0);
                    zlog_debug(g_zlog_zc, "%-40s风门设定值0 二氧化碳报警 风门停止", "[damper_control_func]");
                }
            }

            // 触发高湿时，开始开风门。开到DBO位置。
            else if (get_val(HIGH_HUMI_ALARM_STATUS) && (get_val(R_AI_TP_MAIN) > get_val(LOW_TEMP_ALARM_VALUE)))
            {
                if (current_fan_value == get_val(P_HUM_DBO))
                {
                    // 风门停止
                    control_damper(1, OFF, 0);
                    zlog_debug(g_zlog_zc, "%-40s风门设定值0 高湿报警 风门停止", "[damper_control_func]");
                }

                // 风门当前值小于DBO设定值
                if (current_fan_value < get_val(P_HUM_DBO))
                {
                    // 开启风门
                    control_damper(1, ON, 0);
                    zlog_debug(g_zlog_zc, "%-40s风门设定值0 高湿报警 风门开", "[damper_control_func]");
                }

                if (current_fan_value > get_val(P_HUM_DBO))
                {
                    // 关闭风门
                    control_damper(2, ON, 0);
                    zlog_debug(g_zlog_zc, "%-40s风门设定值0 高湿报警 风门关", "[damper_control_func]");
                }
            }

            // 实际湿度在（设定湿度+SPO）值以上时，开始开风门。开到DBO位置。
            else if ((get_val(R_AI_HM) >= (get_val(P_AO_HM) + get_val(P_HUM_SPO))) && (get_val(R_AI_TP_MAIN) > get_val(LOW_TEMP_ALARM_VALUE)))
            {
                // 风门当前值小于DBO设定值
                if (current_fan_value == get_val(P_HUM_DBO))
                {
                    // 开启风门
                    control_damper(1, OFF, 0);
                    zlog_debug(g_zlog_zc, "%-40s风门设定值0 设定湿度+SPO 风门停止", "[damper_control_func]");
                }

                // 风门当前值小于DBO设定值
                if (current_fan_value < get_val(P_HUM_DBO))
                {
                    // 开启风门
                    control_damper(1, ON, 0);
                    zlog_debug(g_zlog_zc, "%-40s风门设定值0 设定湿度+SPO 风门开", "[damper_control_func]");
                }

                if (current_fan_value > get_val(P_HUM_DBO))
                {
                    // 关闭风门
                    control_damper(2, ON, 0);
                    zlog_debug(g_zlog_zc, "%-40s风门设定值0 设定湿度+SPO 风门关", "[damper_control_func]");
                }
            }
            // 没有其他控制当前值等于设定值时，关闭风门
            else
            {
                control_damper(2, ON, 0);
                zlog_debug(g_zlog_zc, "%-40s风门设定值0 风门关", "[damper_control_func]");
            }
        }
        else
        {
            // 风门最小开度
            // 当风门测量值小于风门设定值，风门才会开
            if (current_fan_value < setting_fan_value)
            {
                // 开启风门
                control_damper(1, ON, 0);
                zlog_debug(g_zlog_zc, "%-40s风门设定值>0 风门最小开度 风门开", "[damper_control_func]");
            }
            else
            {
                // 高温报警
                if (get_val(HIGH_TEMP_ALARM_STATUS))
                {
                    // 开启风门
                    control_damper(1, ON, 0);
                    zlog_debug(g_zlog_zc, "%-40s风门设定值>0 高温报警 风门开", "[damper_control_func]");
                }

                // 二氧化碳报警
                else if (get_val(CARBON_DIOXIDE_WARNING_LIGHT))
                {
                    if (get_uval(CARBON_DIOXIDE_OPERATING_TIME) > 0)
                    {
                        // 开启风门
                        control_damper(1, ON, 0);
                        zlog_debug(g_zlog_zc, "%-40s风门设定值>0 二氧化碳报警 风门开", "[damper_control_func]");
                    }
                    else
                    {
                        if (!get_val(CARBON_DIOXIDE_INIT))
                        {
                            // 设置控制风门的停止时间
                            unsigned short fan_stop_time = get_uval(P_CO2_ZQC) - get_uval(P_CO2_GZS);
                            set_uval(CARBON_DIOXIDE_OPERATING_TIME, 0);
                            set_uval(CARBON_DIOXIDE_STOP_TIME, fan_stop_time);
                            set_val(CARBON_DIOXIDE_INIT, 1);
                        }
                    }

                    if (get_uval(CARBON_DIOXIDE_STOP_TIME) > 0)
                    {
                        // 开启风门
                        control_damper(1, OFF, 0);
                        zlog_debug(g_zlog_zc, "%-40s风门设定值>0 二氧化碳报警 风门停止", "[damper_control_func]");
                    }
                    else
                    {
                        if (get_val(CARBON_DIOXIDE_INIT) == 1)
                        {
                            // 设置控制风门的运行时间
                            set_uval(CARBON_DIOXIDE_OPERATING_TIME, get_uval(P_CO2_GZS));
                            set_uval(CARBON_DIOXIDE_STOP_TIME, 0);
                            set_val(CARBON_DIOXIDE_INIT, 0);
                        }
                    }

                    if (!get_uval(CARBON_DIOXIDE_STOP_TIME) && !get_uval(CARBON_DIOXIDE_OPERATING_TIME))
                    {
                        // 风门停止
                        control_damper(1, OFF, 0);
                        zlog_debug(g_zlog_zc, "%-40s风门设定值>0 二氧化碳报警 风门停止", "[damper_control_func]");
                    }
                }

                // 触发高湿时，开始开风门。开到DBO位置。
                else if (get_val(HIGH_HUMI_ALARM_STATUS) && (get_val(R_AI_TP_MAIN) > get_val(LOW_TEMP_ALARM_VALUE)))
                {
                    // 风门当前值小于DBO设定值
                    if (current_fan_value < get_val(P_HUM_DBO))
                    {
                        // 开启风门
                        control_damper(1, ON, 0);
                        zlog_debug(g_zlog_zc, "%-40s风门设定值>0 高湿报警 风门开", "[damper_control_func]");
                    }
                    else
                    {
                        // 关闭风门
                        control_damper(1, OFF, 0);
                        zlog_debug(g_zlog_zc, "%-40s风门设定值>0 高湿报警 风门停止", "[damper_control_func]");
                    }
                }

                // 实际湿度在（设定湿度+SPO）值以上时，开始开风门。开到DBO位置。
                else if ((get_val(R_AI_HM) >= (get_val(P_AO_HM) + get_val(P_HUM_SPO))) && (get_val(R_AI_TP_MAIN) >= get_val(LOW_TEMP_ALARM_VALUE)))
                {
                    // 风门当前值小于DBO设定值
                    if (current_fan_value < get_val(P_HUM_DBO))
                    {
                        // 开启风门
                        control_damper(1, ON, 0);
                        zlog_debug(g_zlog_zc, "%-40s风门设定值>0 设定湿度+SPO 风门开", "[damper_control_func]");
                    }
                    else
                    {
                        // 关闭风门
                        control_damper(1, OFF, 0);
                        zlog_debug(g_zlog_zc, "%-40s风门设定值>0 设定湿度+SPO 风门停止", "[damper_control_func]");
                    }
                }

                // 温度低关风门
                else if (get_val(R_AI_TP_MAIN) < (get_val(P_AO_TP_MAIN) - get_x10_val(P_TEMP_P) / 2))
                {
                    // 关风门
                    control_damper(2, ON, 1);
                    zlog_debug(g_zlog_zc, "%-40s风门设定值>0 温度低关风门 风门关", "[damper_control_func]");
                }

                // PID控制风门
                else if ((get_val(R_AI_TP_MAIN) >= (get_val(P_AO_TP_MAIN) - get_x10_val(P_TEMP_P) / 2)) && (get_val(R_AI_TP_MAIN) <= (get_val(P_AO_TP_MAIN) + get_x10_val(P_TEMP_P) / 2)))
                {
                    if (get_val(PID_TEMP_MODE_STATUS) == OFF)
                    {
                        set_val(PID_AD_VALUE, get_val(R_AI_AD));

                        set_val(PID_TEMP_MODE_STATUS, ON);
                    }

                    if ((get_uval(PID_ON_COUNT_DB_OPEN) > 0) && (get_uval(PID_ON_COUNT_DB_OPEN) >= get_uval(PID_ON_COUNT_DB_OPEN + 1)) && get_val(PID_AD_OPEN_STATUS))
                    {
                        control_damper(1, ON, 0);
                        zlog_debug(g_zlog_zc, "%-40s风门设定值>0 PID 风门开", "[damper_control_func]");
                    }
                    else if ((get_uval(PID_ON_COUNT_DB_CLOSE) > 0) && (get_uval(PID_ON_COUNT_DB_CLOSE) >= get_uval(PID_ON_COUNT_DB_CLOSE + 1)))
                    {
                        control_damper(2, ON, 1);
                        zlog_debug(g_zlog_zc, "%-40s风门设定值>0 PID 风门关", "[damper_control_func]");
                    }
                    else
                    {
                        control_damper(1, OFF, 0);
                        zlog_debug(g_zlog_zc, "%-40s风门设定值>0 PID 风门停止", "[damper_control_func]");
                    }
                }

                // 温度高关风门
                else if (get_val(R_AI_TP_MAIN) > (get_val(P_AO_TP_MAIN) + get_x10_val(P_TEMP_P) / 2))
                {
                    // 开启风门
                    control_damper(1, ON, 0);
                    zlog_debug(g_zlog_zc, "%-40s风门设定值>0 温度高关风门 风门开", "[damper_control_func]");
                }

                // 没有其他控制当前值等于设定值时，关闭风门
                else if (current_fan_value == setting_fan_value)
                {
                    control_damper(2, OFF, 0);
                    zlog_debug(g_zlog_zc, "%-40s风门设定值>0 测量值==设定值 风门停止", "[damper_control_func]");
                }
                else
                {
                    control_damper(2, OFF, 0);
                    zlog_debug(g_zlog_zc, "%-40s风门设定值>0 风门停止", "[damper_control_func]");
                }

                // 当设置值等于实际值关闭风门
            }
        }
    }
}

//----------------------------------------------------------------------------------------------------------
// PID风门开度
//----------------------------------------------------------------------------------------------------------
void pid_air_door_open()
{
    // 风门开
    short difference_in_damper_open_setting = get_val(R_AI_TP_MAIN) - get_val(P_AO_TP_MAIN);
    if (difference_in_damper_open_setting > 0)
    {
        if (((difference_in_damper_open_setting) <= (get_x10_val(P_TEMP_P) / 6)) && (difference_in_damper_open_setting > 0))
        {
            if (get_val(R_AI_AD) >= (get_val(PID_AD_VALUE) + 10))
            {
                set_val(PID_AD_OPEN_STATUS, 0);
            }
            else
            {
                set_val(PID_AD_OPEN_STATUS, 1);
            }
        }

        else if (((difference_in_damper_open_setting) <= (get_x10_val(P_TEMP_P) / 3)) && ((difference_in_damper_open_setting) > (get_x10_val(P_TEMP_P) / 6)))
        {
            if (get_val(R_AI_AD) >= (get_val(PID_AD_VALUE) + 20))
            {
                set_val(PID_AD_OPEN_STATUS, 0);
            }
            else
            {
                set_val(PID_AD_OPEN_STATUS, 1);
            }
        }

        else if (((difference_in_damper_open_setting) <= (get_x10_val(P_TEMP_P) / 2)) && ((difference_in_damper_open_setting) > (get_x10_val(P_TEMP_P) / 3)))
        {
            if (get_val(R_AI_AD) >= (get_val(PID_AD_VALUE) + 30))
            {
                set_val(PID_AD_OPEN_STATUS, 0);
            }
            else
            {
                set_val(PID_AD_OPEN_STATUS, 1);
            }
        }
    }
}

//----------------------------------------------------------------------------------------------------------
// 逻辑处理线程-主温度
//----------------------------------------------------------------------------------------------------------
void logic_process_temp_func()
{
    static int pid_run_wait = 1;

    // 获取温度当前值
    short current_temp_value = get_val(R_AI_TP_MAIN);
    // 获取温度设定值
    short setting_temp_value = get_val(P_AO_TP_MAIN);
    // 获取辅助加热动作点
    short setting_temp_auh_value = get_val(P_AUH);
    // 获取温度PID参数的P/2值
    short setting_temp_pid_p_value = get_x10_val(P_TEMP_P) / 2;

    // 获取高温报警状态
    short high_temp_alarm_status = get_val(HIGH_TEMP_ALARM_STATUS);

    static int old_pid_main_heating_reriod_count = 0;
    static int temp_mode_status = 0;
    static int temp_mode_value = 0;

    zlog_debug(g_zlog_zc, "%-40s进入到逻辑处理线程-主温度", "[logic_process_temp_func]");

    // 辅助加热依靠AUH辅助加热动作点（设定温度的相对值）。进行控制
    if (current_temp_value < (setting_temp_value - setting_temp_auh_value))
    {
        zlog_debug(g_zlog_zc, "%-40s[PV2<(SP2-AUH)]开启辅助加热", "[logic_process_temp_func]");
        // 开启辅助加热
        control_aux_heat(ON);
    }

    // 辅助加热依靠AUH辅助加热动作点（设定温度的相对值）。进行控制
    else if (current_temp_value >= (setting_temp_value - setting_temp_auh_value))
    {
        zlog_debug(g_zlog_zc, "%-40s[PV2>=(SP2-AUH)]关闭辅助加热", "[logic_process_temp_func]");
        // 关闭辅助加热
        control_aux_heat(OFF);
    }
    else
    {
    }

    // 当PV2 < SP2-P/2时,风门开始关闭（输出关闭驱动信号,指示灯）,直到设定值。主加热同时动作（输出主温度加热指示灯）
    if (current_temp_value < (setting_temp_value - setting_temp_pid_p_value))
    {
        zlog_debug(g_zlog_zc, "%-40s[PV2<SP2-P/2]关闭水冷和开启主加热", "[logic_process_temp_func]");

        // 主加热-开启
        control_main_heat(ON);

        // 水冷关闭
        control_cool(OFF);

        // control_blower(OFF);

        set_time(PID_TEMP_SUM, 0);
        set_time(PID_PERIOD_COUNT_MAIN_HEATER, 0);
        set_val(PID_TEMP_MODE_STATUS, 0);

        for (size_t i = 0; i < 10; i++)
        {
            int *pout = (int *)&shm_out[FILTER_PID_TEMP_ARR];
            *(pout + i) = 10000;
        }
    }

    // PID范围
    else if ((current_temp_value >= (setting_temp_value - setting_temp_pid_p_value)) && (current_temp_value <= (setting_temp_value + setting_temp_pid_p_value)))
    {
        zlog_debug(g_zlog_zc, "%-40s[(PV2<SP2-P/2)<=(PV2<=SP2+P/2)]PID范围", "[logic_process_temp_func]");
        pid_temp_init();
        pid_air_door_open();

        int pid_filter_old = get_int(PID_TEMP_OLD_FILTER_VALUE);
        int pid_filter_new = get_int(PID_TEMP_NEW_FILTER_VALUE);
        int pid_filter_status = (((pid_filter_old <= 5000) && (pid_filter_new >= 5000)) || ((pid_filter_old >= 5000) && (pid_filter_new <= 5000)) || (abs(pid_filter_new - pid_filter_old) >= 500)) ? 1 : 0;

        // 主加热电磁阀
        if (get_uval(PID_TEMP_MAIN_HEATER) >= 0)
        {
            if (get_uval(PID_ON_COUNT_MAIN_HEATER) == 0 || pid_filter_status)
            {
                float scale = get_uval(PID_TEMP_MAIN_HEATER) / 1000.0;
                short cycle = get_uval(P_TEMP_TO);
                short pid_main_heating_reriod_count = cycle * scale;
                if (pid_main_heating_reriod_count > 0)
                {
                    // 获取主加热的时间
                    set_uval(PID_ON_COUNT_MAIN_HEATER + 1, cycle - pid_main_heating_reriod_count);
                    set_uval(PID_ON_COUNT_MAIN_HEATER, cycle);
                }
                else
                {
                    set_uval(PID_ON_COUNT_MAIN_HEATER, 0);
                }
            }
        }
        else
        {
            set_uval(PID_ON_COUNT_MAIN_HEATER, 0);
        }

        // 获取水冷工作时间
        if (get_uval(PID_TEMP_COOL))
        {
            if (get_uval(PID_ON_COUNT_COOL) == 0 || pid_filter_status)
            {
                float scale = (5000 - pid_filter_new) / 5000.0;
                short cycle = get_uval(P_TEMP_TO);
                short pid_cool_reriod_count = cycle * scale;
                if (pid_cool_reriod_count > 0)
                {
                    set_uval(PID_ON_COUNT_COOL + 1, cycle - pid_cool_reriod_count);
                    set_uval(PID_ON_COUNT_COOL, cycle);
                }
                else
                {
                    set_uval(PID_ON_COUNT_COOL, 0);
                }
            }
        }
        else
        {
            set_uval(PID_ON_COUNT_COOL, 0);
        }

// 获取鼓风机工作时间
#if (0)
        if (get_uval(PID_TEMP_BLOWER))
        {
            if (get_uval(PID_ON_COUNT_BLOWER) == 0 || pid_filter_status)
            {
                float scale = (5000 - pid_filter_new) / 5000.0;
                short cycle = get_uval(P_TEMP_TO);
                short pid_blower_reriod_count = cycle * scale;
                if (pid_blower_reriod_count > 0)
                {
                    set_uval(PID_ON_COUNT_BLOWER + 1, cycle - pid_blower_reriod_count);
                    set_uval(PID_ON_COUNT_BLOWER, cycle);
                }
                else
                {
                    set_uval(PID_ON_COUNT_BLOWER, 0);
                }
            }
        }
        else
        {
            set_uval(PID_ON_COUNT_BLOWER, 0);
        }
#endif

        // 获取风门开时间
        if (get_uval(PID_TEMP_DB_OPEN))
        {
            if (get_uval(PID_ON_COUNT_DB_OPEN) == 0 || pid_filter_status)
            {
                float scale = (5000 - pid_filter_new) / 5000.0;
                short cycle = get_uval(P_TEMP_TO);
                short pid_db_open_reriod_count = cycle * scale;
                if (pid_db_open_reriod_count > 0)
                {
                    set_uval(PID_ON_COUNT_DB_OPEN + 1, cycle - pid_db_open_reriod_count);
                    set_uval(PID_ON_COUNT_DB_OPEN, cycle);
                }
                else
                {
                    set_uval(PID_ON_COUNT_DB_OPEN, 0);
                }
            }
        }
        else
        {
            set_uval(PID_ON_COUNT_DB_OPEN, 0);
        }

        // 获取风门关时间
        if (get_uval(PID_TEMP_DB_CLOSE))
        {
            if (get_uval(PID_ON_COUNT_DB_CLOSE) == 0 || pid_filter_status)
            {
                float scale = (pid_filter_new - 5000) / 5000.0;
                short cycle = get_uval(P_TEMP_TO);
                short pid_db_close_reriod_count = cycle * scale;
                if (pid_db_close_reriod_count > 0)
                {
                    set_uval(PID_ON_COUNT_DB_CLOSE + 1, cycle - pid_db_close_reriod_count);
                    set_uval(PID_ON_COUNT_DB_CLOSE, cycle);
                }
                else
                {
                    set_uval(PID_ON_COUNT_DB_CLOSE, 0);
                }
            }
        }
        else
        {
            set_uval(PID_ON_COUNT_DB_CLOSE, 0);
        }

        // 过滤PID 旧值
        set_int(PID_TEMP_OLD_FILTER_VALUE, pid_filter_new);

        // 主加热
        if ((get_uval(PID_ON_COUNT_MAIN_HEATER) > 0) && (get_uval(PID_ON_COUNT_MAIN_HEATER) >= get_uval(PID_ON_COUNT_MAIN_HEATER + 1)))
        {
            zlog_debug(g_zlog_zc, "%-40s[(PV2<SP2-P/2)<=(PV2<=SP2+P/2)]PID范围 主加热 启动", "[logic_process_temp_func]");
            // 主加热-开启
            control_main_heat(ON);
        }
        else
        {
            zlog_debug(g_zlog_zc, "%-40s[(PV2<SP2-P/2)<=(PV2<=SP2+P/2)]PID范围 主加热 关闭", "[logic_process_temp_func]");
            control_main_heat(OFF);
        }

        // 水冷
        if ((get_uval(PID_ON_COUNT_COOL) > 0) && (get_uval(PID_ON_COUNT_COOL) >= get_uval(PID_ON_COUNT_COOL + 1)))
        {
            zlog_debug(g_zlog_zc, "%-40s[(PV2<SP2-P/2)<=(PV2<=SP2+P/2)]PID范围 水冷 启动", "[logic_process_temp_func]");
            // 水冷电磁阀
            control_cool(ON);
        }
        else
        {
            zlog_debug(g_zlog_zc, "%-40s[(PV2<SP2-P/2)<=(PV2<=SP2+P/2)]PID范围 水冷 关闭", "[logic_process_temp_func]");
            control_cool(OFF);
        }

#if 0
        // 鼓风机
        if ((get_uval(PID_ON_COUNT_BLOWER) > 0) && (get_uval(PID_ON_COUNT_BLOWER) >= get_uval(PID_ON_COUNT_BLOWER + 1)))
        {
            zlog_debug(g_zlog_zc, "%-40s[(PV2<SP2-P/2)<=(PV2<=SP2+P/2)]PID范围 鼓风机 启动", "[logic_process_temp_func]");
            // 鼓风机电磁阀
            control_blower(ON);
        }
        else
        {
            zlog_debug(g_zlog_zc, "%-40s[(PV2<SP2-P/2)<=(PV2<=SP2+P/2)]PID范围 鼓风机 关闭", "[logic_process_temp_func]");
            control_blower(OFF);
        }
#endif
    }

    // 当PV2>SP2+P/2,水冷电磁阀打开,风门等待20秒后打开（同时输出水冷指示灯,20秒后开门指示灯）。风门打开直到2公分（监测到20mm开关信号输入闭合）时,水冷和风门同时停止。等待30秒（HH9）后再进行比较,再按照1,2,3步输出动作,如果还是PV2>SP2,继续水冷电磁阀打开,风门打开直到风门全部打开,（风门灯闪烁表示全关）
    else if (current_temp_value > (setting_temp_value + setting_temp_pid_p_value))
    {
        zlog_debug(g_zlog_zc, "%-40s[PV2>SP2+P/2]开启水冷和关闭主加热", "[logic_process_temp_func]");

        // 水冷-开启
        control_cool(ON);

        // 鼓风机开启
        // control_blower(ON);

        // 主加热-关闭
        control_main_heat(OFF);

        // pid_temp_sum_en = 0;
        set_time(PID_TEMP_SUM, 0);
        set_time(PID_PERIOD_COUNT_MAIN_HEATER, 0);
        set_val(PID_TEMP_MODE_STATUS, 0);

        for (size_t i = 0; i < 10; i++)
        {
            int *pout = (int *)&shm_out[FILTER_PID_TEMP_ARR];
            *(pout + i) = 5000;
        }
    }
    else
    {
    }

    // 当PV2>SP2时，比如温度高于设定0.1度频率50%、高于0.2度频率51%
    if (g_fan_status == 1)
    {
        if (current_temp_value >= setting_temp_value)
        {
            // 当高温报警的时候
            if (high_temp_alarm_status)
            {
                // 将上限频率赋值给当前频率
                shm_out[S_SYNC_MOTOR_CONTROLRUN_FREQ] = shm_out[S_SYNC_MOTOR_UPPER_LIMIT_FREQ];
                zlog_debug(g_zlog_zc, "%-40s[PV2>SP2] 开启上限频率 %d%", "[logic_process_temp_func]", get_val(S_SYNC_MOTOR_CONTROLRUN_FREQ) / 100);
            }
            else
            {
                short temp_difference = (current_temp_value - setting_temp_value) / get_val(S_SYNC_MOTOR_STAGE);
                int current_freq = get_val(S_SYNC_MOTOR_LOWER_LIMIT_FREQ) + temp_difference * get_val(S_SYNC_MOTOR_BASICS);
                if (current_freq >= get_val(S_SYNC_MOTOR_UPPER_LIMIT_FREQ))
                {
                    current_freq = get_val(S_SYNC_MOTOR_UPPER_LIMIT_FREQ);
                }
                set_val(S_SYNC_MOTOR_CONTROLRUN_FREQ, current_freq);
                zlog_debug(g_zlog_zc, "%-40s[PV2>SP2] 开启当前频率 %d%", "[logic_process_temp_func]", current_freq / 100);
            }
        }
        else if (current_temp_value < setting_temp_value)
        {
            // 将下限频率赋值给当前频率
            shm_out[S_SYNC_MOTOR_CONTROLRUN_FREQ] = shm_out[S_SYNC_MOTOR_LOWER_LIMIT_FREQ];
            zlog_debug(g_zlog_zc, "%-40s[PV2>SP2] 开启下限频率 %d%", "[logic_process_temp_func]", get_val(S_SYNC_MOTOR_CONTROLRUN_FREQ) / 100);
        }
        else
        {
        }
    }
}

//----------------------------------------------------------------------------------------------------------
// 逻辑处理线程-湿度
//----------------------------------------------------------------------------------------------------------
void logic_process_humi_func()
{
    zlog_debug(g_zlog_zc, "%-40s进入到逻辑处理线程-湿度", "[logic_process_humi_func]");
    // 获取湿度当前值
    short current_humi_value = get_val(R_AI_HM);
    // 获取湿度设定值
    short setting_humi_value = get_val(P_AO_HM);
    // 获取湿度PID参数的P/2值
    short setting_humi_pid_p_value = get_x10_val(P_HUM_P) / 2;

    short setting_humi_spr_value = get_val(P_HUM_SPR);

    // 获取温度当前值
    short current_temp_value = get_val(R_AI_TP_MAIN);
    // 获取温度最小值
    short setting_temp_min_value = get_val(LOW_TEMP_ALARM_VALUE);

    static int old_pid_reriod_count = 0;

    short point_stp = 0;

    if (get_val(P_HUM_SPR) < (get_x10_val(P_HUM_P) / 2))
    {
        point_stp = get_val(P_HUM_SPR);
    }
    else
    {
        point_stp = get_x10_val(P_HUM_P) / 2;
    }

    // 当PV2>AL2。时，
    if ((current_temp_value > setting_temp_min_value))
    {

        // 当PV1<SP1-P/2,加湿电磁阀启动,同时输出加湿状态指示灯
        if (current_humi_value < (setting_humi_value - setting_humi_pid_p_value))
        {
            zlog_debug(g_zlog_zc, "%-40s[%d<(%d-%d)]开启加湿电磁阀", "[logic_process_humi_func]", current_humi_value, setting_humi_value, setting_humi_pid_p_value);
            // 开启加湿电磁阀
            control_spray(ON);
            // pid_humi_sum_en = 0;
            set_int(PID_HUMI_SUM, 0);
            set_int(PID_PERIOD_COUNT_HUMI, 0);
        }

        // PID范围
        else if ((current_humi_value >= (setting_humi_value - setting_humi_pid_p_value)) && (current_humi_value <= (setting_humi_value + point_stp)))
        {
            zlog_debug(g_zlog_zc, "%-40s[(PV1<SP1-P/2)<=(PV1<=SP1+P/2)]PID范围", "[logic_process_humi_func]");

            pid_humi_init();

            // unsigned int pid_reriod_count = (get_int(PID_HUMI_HUMIDIFICATION) * (get_uval(P_HUM_TO))) / 1000;

            int pid_filter_old = get_int(PID_HUMI_OLD_HUMIDIFICATION);
            int pid_filter_new = get_int(PID_HUMI_NEW_HUMIDIFICATION);
            int pid_filter_status = ((pid_filter_old == 0) && (pid_filter_new != 0) || (pid_filter_old == 0) && (pid_filter_new != 0) || (abs(pid_filter_new - pid_filter_old) >= 500)) ? 1 : 0;

            unsigned int pid_reriod_count = 0;
            pid_reriod_count = (pid_filter_new * get_uval(P_HUM_TO)) / 1000;

            if (get_time(PID_PERIOD_COUNT_HUMI) == 0 || pid_filter_status)
            {
                int hum_to = get_uval(P_HUM_TO) - pid_reriod_count;
                set_time(PID_ON_COUNT_HUMI, get_uval(P_HUM_TO) - pid_reriod_count);
                set_time(PID_PERIOD_COUNT_HUMI, get_uval(P_HUM_TO));
            }
            else
            {
                if ((get_time(PID_PERIOD_COUNT_HUMI) > 0) && (get_time(PID_PERIOD_COUNT_HUMI) >= get_time(PID_ON_COUNT_HUMI)) && current_humi_value < (setting_humi_value + setting_humi_spr_value))
                {
                    // 加湿-开启
                    control_spray(ON);
                }
                else
                {
                    control_spray(OFF);
                }
            }

            set_int(PID_HUMI_OLD_HUMIDIFICATION, pid_filter_new);
        }

        // PV1>SP1+SPR 时,强制关闭加湿动作。
        else if (current_humi_value >= (setting_humi_value + setting_humi_spr_value))
        {
            zlog_debug(g_zlog_zc, "%-40s[PV1>(SP1+SPR)]关闭加湿电磁阀", "[logic_process_humi_func]");
            // 关闭加湿电磁阀
            control_spray(OFF);
            // pid_humi_sum_en = 0;
            set_int(PID_HUMI_SUM, 0);
            set_int(PID_PERIOD_COUNT_HUMI, 0);
        }

        // PV1>SP1+P/2 时,强制关闭加湿动作。
        // else if (current_humi_value > (setting_humi_value + setting_humi_pid_p_value))
        // {
        //     zlog_debug(g_zlog_zc, "%-40s[PV1>(SP1+P/2)]关闭加湿电磁阀", "[logic_process_humi_func]");
        //     // 关闭加湿电磁阀
        //     control_spray(OFF);
        //     // pid_humi_sum_en = 0;
        //     set_int(PID_HUMI_SUM, 0);
        //     set_int(PID_PERIOD_COUNT_HUMI, 0);

        //     humi_mode_status = 0;
        //     humi_mode_value = 0;
        // }
    }
    else
    {
        zlog_debug(g_zlog_zc, "%-40sPV2<AL2", "[logic_process_humi_func]");
        control_spray(OFF);
        set_int(PID_HUMI_SUM, 0);
        set_int(PID_PERIOD_COUNT_HUMI, 0);
    }
}

//----------------------------------------------------------------------------------------------------------
// 逻辑处理线程-回流温度1-正常模式
//----------------------------------------------------------------------------------------------------------
void normal_logic_process_retemp1_func()
{
    zlog_debug(g_zlog_zc, "%-40s进入到回流温度1-正常模式", "[normal_logic_process_retemp1_func]");
    if (get_uval(REFLUX_TEMPERATURE) & 0x01)
    {
        // 获取当前天数和小时
        short sys_day_hour_time = get_val(R_RUN_DAY);
        // 获取开启冷水的天数和小数
        short cool_day_hour_time = get_val(P_PRE_TP_RF1_DAY);

        // 当系统时间大于等于开启冷水的时间时，开启冷水
        if (sys_day_hour_time >= cool_day_hour_time)
        {
            // 热水模式初始化为0
            set_uval(RETURN_TEMP_1_HOT_WATER_OPERAT_INIT, 0);
            // 开启冷水
            control_cold_hot_water(OFF);

            // 回流温度延迟时间为0即可进入
            if (get_uval(REFLUX_TEMP_1_DELAY_TIME) == 0)
            {
                // 获取当前值
                unsigned short current_ref_temp_value = get_uval(R_AI_TP_RF1);
                // 获取设定值
                unsigned short setting_ref_temp_value = get_val(P_AO_TP_RF1);

                // 冷水模式
                if (current_ref_temp_value > setting_ref_temp_value)
                {
                    // 冷水工作模式初始化为0
                    if (!get_uval(REFLOW_TEMP_1_COLD_WATER_OPERAT_INIT))
                    {
                        // 设置回流温度电磁阀的工作时间
                        set_uval(REFLUX_TEMP_1_WORK_TIME, get_uval(P_RTEMP_GZS));

                        // 设置实时工作周期为
                        set_uval(RETURN_TEMP_1_WORK_CYCLE, 0);

                        // 冷水工作模式初始化为1
                        set_uval(REFLOW_TEMP_1_COLD_WATER_OPERAT_INIT, 1);
                    }

                    // 当前冷水工作周期小于工作周期设定值
                    if (get_uval(RETURN_TEMP_1_WORK_CYCLE) < get_uval(P_RTEMP_ZQC))
                    {
                        // 工作时间大于0
                        if (get_uval(REFLUX_TEMP_1_WORK_TIME) > 0)
                        {
                            // 开启回流温度电磁阀
                            control_cool2(1, ON);
                        }
                        else
                        {
                            // 冷水工作模式为0
                            if (get_uval(REFLOW_TEMP_1_COLD_WATER_OPERAT_INIT) == 1)
                            {
                                // 设置回流温度电磁阀的停止时间
                                unsigned short setting_ref_temp_stop_time = get_uval(P_RTEMP_ZQS) - get_uval(P_RTEMP_GZS);
                                set_uval(RETURN_TEMP_1_STOP_TIME, setting_ref_temp_stop_time);

                                // 设置冷水工作模式为1
                                set_uval(REFLOW_TEMP_1_COLD_WATER_OPERAT_INIT, 2);
                            }

                            // 停止时间大于0
                            if (get_uval(RETURN_TEMP_1_STOP_TIME) > 0)
                            {
                                // 关闭启回流温度电磁阀
                                control_cool2(1, OFF);
                            }
                            else
                            {
                                if (get_uval(REFLOW_TEMP_1_COLD_WATER_OPERAT_INIT) == 2)
                                {
                                    // 设置回流温度电磁阀的工作时间
                                    set_uval(REFLUX_TEMP_1_WORK_TIME, get_uval(P_RTEMP_GZS));
                                    // 回流温度工作周期加1
                                    set_uval(RETURN_TEMP_1_WORK_CYCLE, get_uval(RETURN_TEMP_1_WORK_CYCLE) + 1);
                                    // 设置冷水工作模式为1
                                    set_uval(REFLOW_TEMP_1_COLD_WATER_OPERAT_INIT, 1);
                                }
                            }
                        }
                    }
                    else
                    {
                        // 开启回流温度电磁阀
                        control_cool2(1, ON);
                    }
                }
                // 当前值大于等于设定值
                else
                {
                    // 关闭启回流温度电磁阀
                    control_cool2(1, OFF);
                    // 设置热水工作模式为0
                    set_uval(REFLOW_TEMP_1_COLD_WATER_OPERAT_INIT, 0);
                }
            }
        }
        // 热水模式
        else
        {
            // 冷水模式初始化为0
            set_uval(REFLOW_TEMP_1_COLD_WATER_OPERAT_INIT, 0);
            // 开启热水
            control_cold_hot_water(ON);

            // 回流温度延迟时间为0即可进入
            if (get_uval(REFLUX_TEMP_1_DELAY_TIME) == 0)
            {
                // 获取当前值
                unsigned short current_ref_temp_value = get_uval(R_AI_TP_RF1);
                // 获取设定值
                unsigned short setting_ref_temp_value = get_val(P_AO_TP_RF1);

                // 热水模式
                if (current_ref_temp_value < setting_ref_temp_value)
                {
                    // 热水工作模式初始化为0
                    if (!get_uval(RETURN_TEMP_1_HOT_WATER_OPERAT_INIT))
                    {
                        // 设置回流温度电磁阀的工作时间
                        set_uval(REFLUX_TEMP_1_WORK_TIME, get_uval(P_RTEMP_GZS));

                        // 设置实时工作周期为
                        set_uval(RETURN_TEMP_1_WORK_CYCLE, 0);

                        // 热水工作模式初始化为1
                        set_uval(RETURN_TEMP_1_HOT_WATER_OPERAT_INIT, 1);
                    }

                    // 当前热水工作周期小于工作周期设定值
                    if (get_uval(RETURN_TEMP_1_WORK_CYCLE) < get_uval(P_RTEMP_ZQC))
                    {
                        // 工作时间大于0
                        if (get_uval(REFLUX_TEMP_1_WORK_TIME) > 0)
                        {
                            // 开启回流温度电磁阀
                            control_cool2(1, ON);
                        }
                        else
                        {
                            // 热水工作模式为0
                            if (get_uval(RETURN_TEMP_1_HOT_WATER_OPERAT_INIT) == 1)
                            {
                                // 设置回流温度电磁阀的停止时间
                                unsigned short setting_ref_temp_stop_time = get_uval(P_RTEMP_ZQS) - get_uval(P_RTEMP_GZS);
                                set_uval(RETURN_TEMP_1_STOP_TIME, setting_ref_temp_stop_time);

                                // 设置热水工作模式为1
                                set_uval(RETURN_TEMP_1_HOT_WATER_OPERAT_INIT, 2);
                            }

                            // 停止时间大于0
                            if (get_uval(RETURN_TEMP_1_STOP_TIME) > 0)
                            {
                                // 关闭启回流温度电磁阀
                                control_cool2(1, OFF);
                            }
                            else
                            {
                                if (get_uval(RETURN_TEMP_1_HOT_WATER_OPERAT_INIT) == 2)
                                {
                                    // 设置回流温度电磁阀的工作时间
                                    set_uval(REFLUX_TEMP_1_WORK_TIME, get_uval(P_RTEMP_GZS));
                                    // 回流温度工作周期加1
                                    set_uval(RETURN_TEMP_1_WORK_CYCLE, get_uval(RETURN_TEMP_1_WORK_CYCLE) + 1);
                                    // 设置热水工作模式为1
                                    set_uval(RETURN_TEMP_1_HOT_WATER_OPERAT_INIT, 1);
                                }
                            }
                        }
                    }
                    else
                    {
                        // 开启回流温度电磁阀
                        control_cool2(1, ON);
                    }
                }
                // 当前值大于等于设定值
                else
                {
                    // 关闭启回流温度电磁阀
                    control_cool2(1, OFF);
                    // 设置热水工作模式为0
                    set_uval(RETURN_TEMP_1_HOT_WATER_OPERAT_INIT, 0);
                }
            }
        }
    }
    else
    {
        control_cool2(1, OFF);
    }
}

//----------------------------------------------------------------------------------------------------------
// 逻辑处理线程-回流温度2-正常模式
//----------------------------------------------------------------------------------------------------------
void normal_logic_process_retemp2_func()
{
    zlog_debug(g_zlog_zc, "%-40s进入到回流温度2-正常模式", "[normal_logic_process_retemp2_func]");
    if (get_uval(REFLUX_TEMPERATURE) & 0x02)
    {
        // 获取当前天数和小时
        short sys_day_hour_time = get_val(R_RUN_DAY);
        // 获取开启冷水的天数和小数
        short cool_day_hour_time = get_val(P_PRE_TP_RF1_DAY);

        // 当系统时间大于等于开启冷水的时间时，开启冷水
        if (sys_day_hour_time >= cool_day_hour_time)
        {
            // 热水模式初始化为0
            set_uval(RETURN_TEMP_2_HOT_WATER_OPERAT_INIT, 0);
            // 开启冷水
            control_cold_hot_water(OFF);

            // 回流温度延迟时间为0即可进入
            if (get_uval(REFLUX_TEMP_2_DELAY_TIME) == 0)
            {
                // 获取当前值
                unsigned short current_ref_temp_value = get_uval(R_AI_TP_RF2);
                // 获取设定值
                unsigned short setting_ref_temp_value = get_val(P_AO_TP_RF2);

                // 冷水模式
                if (current_ref_temp_value > setting_ref_temp_value)
                {
                    // 冷水工作模式初始化为0
                    if (!get_uval(REFLOW_TEMP_2_COLD_WATER_OPERAT_INIT))
                    {
                        // 设置回流温度电磁阀的工作时间
                        set_uval(REFLUX_TEMP_2_WORK_TIME, get_uval(P_RTEMP_GZS));

                        // 设置实时工作周期为
                        set_uval(RETURN_TEMP_2_WORK_CYCLE, 0);

                        // 冷水工作模式初始化为1
                        set_uval(REFLOW_TEMP_2_COLD_WATER_OPERAT_INIT, 1);
                    }

                    // 当前冷水工作周期小于工作周期设定值
                    if (get_uval(RETURN_TEMP_2_WORK_CYCLE) < get_uval(P_RTEMP_ZQC))
                    {
                        // 工作时间大于0
                        if (get_uval(REFLUX_TEMP_2_WORK_TIME) > 0)
                        {
                            // 开启回流温度电磁阀
                            control_cool2(2, ON);
                        }
                        else
                        {
                            // 冷水工作模式为0
                            if (get_uval(REFLOW_TEMP_2_COLD_WATER_OPERAT_INIT) == 1)
                            {
                                // 设置回流温度电磁阀的停止时间
                                unsigned short setting_ref_temp_stop_time = get_uval(P_RTEMP_ZQS) - get_uval(P_RTEMP_GZS);
                                set_uval(RETURN_TEMP_2_STOP_TIME, setting_ref_temp_stop_time);

                                // 设置冷水工作模式为1
                                set_uval(REFLOW_TEMP_2_COLD_WATER_OPERAT_INIT, 2);
                            }

                            // 停止时间大于0
                            if (get_uval(RETURN_TEMP_2_STOP_TIME) > 0)
                            {
                                // 关闭启回流温度电磁阀
                                control_cool2(2, OFF);
                            }
                            else
                            {
                                if (get_uval(REFLOW_TEMP_2_COLD_WATER_OPERAT_INIT) == 2)
                                {
                                    // 设置回流温度电磁阀的工作时间
                                    set_uval(REFLUX_TEMP_2_WORK_TIME, get_uval(P_RTEMP_GZS));
                                    // 回流温度工作周期加1
                                    set_uval(RETURN_TEMP_2_WORK_CYCLE, get_uval(RETURN_TEMP_2_WORK_CYCLE) + 1);
                                    // 设置冷水工作模式为1
                                    set_uval(REFLOW_TEMP_2_COLD_WATER_OPERAT_INIT, 1);
                                }
                            }
                        }
                    }
                    else
                    {
                        // 开启回流温度电磁阀
                        control_cool2(2, ON);
                    }
                }
                // 当前值大于等于设定值
                else
                {
                    // 关闭启回流温度电磁阀
                    control_cool2(2, OFF);
                    // 设置热水工作模式为0
                    set_uval(REFLOW_TEMP_2_COLD_WATER_OPERAT_INIT, 0);
                }
            }
        }
        // 热水模式
        else
        {
            // 冷水模式初始化为0
            set_uval(REFLOW_TEMP_2_COLD_WATER_OPERAT_INIT, 0);
            // 开启热水
            control_cold_hot_water(ON);

            // 回流温度延迟时间为0即可进入
            if (get_uval(REFLUX_TEMP_2_DELAY_TIME) == 0)
            {
                // 获取当前值
                unsigned short current_ref_temp_value = get_uval(R_AI_TP_RF2);
                // 获取设定值
                unsigned short setting_ref_temp_value = get_val(P_AO_TP_RF2);

                // 热水模式
                if (current_ref_temp_value < setting_ref_temp_value)
                {
                    // 热水工作模式初始化为0
                    if (!get_uval(RETURN_TEMP_2_HOT_WATER_OPERAT_INIT))
                    {
                        // 设置回流温度电磁阀的工作时间
                        set_uval(REFLUX_TEMP_2_WORK_TIME, get_uval(P_RTEMP_GZS));

                        // 设置实时工作周期为
                        set_uval(RETURN_TEMP_2_WORK_CYCLE, 0);

                        // 热水工作模式初始化为1
                        set_uval(RETURN_TEMP_2_HOT_WATER_OPERAT_INIT, 1);
                    }

                    // 当前热水工作周期小于工作周期设定值
                    if (get_uval(RETURN_TEMP_2_WORK_CYCLE) < get_uval(P_RTEMP_ZQC))
                    {
                        // 工作时间大于0
                        if (get_uval(REFLUX_TEMP_2_WORK_TIME) > 0)
                        {
                            // 开启回流温度电磁阀
                            control_cool2(2, ON);
                        }
                        else
                        {
                            // 热水工作模式为0
                            if (get_uval(RETURN_TEMP_2_HOT_WATER_OPERAT_INIT) == 1)
                            {
                                // 设置回流温度电磁阀的停止时间
                                unsigned short setting_ref_temp_stop_time = get_uval(P_RTEMP_ZQS) - get_uval(P_RTEMP_GZS);
                                set_uval(RETURN_TEMP_2_STOP_TIME, setting_ref_temp_stop_time);

                                // 设置热水工作模式为1
                                set_uval(RETURN_TEMP_2_HOT_WATER_OPERAT_INIT, 2);
                            }

                            // 停止时间大于0
                            if (get_uval(RETURN_TEMP_2_STOP_TIME) > 0)
                            {
                                // 关闭启回流温度电磁阀
                                control_cool2(2, OFF);
                            }
                            else
                            {
                                if (get_uval(RETURN_TEMP_2_HOT_WATER_OPERAT_INIT) == 2)
                                {
                                    // 设置回流温度电磁阀的工作时间
                                    set_uval(REFLUX_TEMP_2_WORK_TIME, get_uval(P_RTEMP_GZS));
                                    // 回流温度工作周期加1
                                    set_uval(RETURN_TEMP_2_WORK_CYCLE, get_uval(RETURN_TEMP_2_WORK_CYCLE) + 1);
                                    // 设置热水工作模式为1
                                    set_uval(RETURN_TEMP_2_HOT_WATER_OPERAT_INIT, 1);
                                }
                            }
                        }
                    }
                    else
                    {
                        // 开启回流温度电磁阀
                        control_cool2(2, ON);
                    }
                }
                // 当前值大于等于设定值
                else
                {
                    // 关闭启回流温度电磁阀
                    control_cool2(2, OFF);
                    // 设置热水工作模式为0
                    set_uval(RETURN_TEMP_2_HOT_WATER_OPERAT_INIT, 0);
                }
            }
        }
    }
    else
    {
        control_cool2(2, OFF);
    }
}

//----------------------------------------------------------------------------------------------------------
// 逻辑处理线程-回流温度3-正常模式
//----------------------------------------------------------------------------------------------------------
void normal_logic_process_retemp3_func()
{
    zlog_debug(g_zlog_zc, "%-40s进入到回流温度3-正常模式", "[normal_logic_process_retemp3_func]");
    if (get_uval(REFLUX_TEMPERATURE) & 0x04)
    {

        // 获取当前天数和小时
        short sys_day_hour_time = get_val(R_RUN_DAY);
        // 获取开启冷水的天数和小数
        short cool_day_hour_time = get_val(P_PRE_TP_RF1_DAY);

        // 当系统时间大于等于开启冷水的时间时，开启冷水
        if (sys_day_hour_time >= cool_day_hour_time)
        {
            // 热水模式初始化为0
            set_uval(RETURN_TEMP_3_HOT_WATER_OPERAT_INIT, 0);
            // 开启冷水
            control_cold_hot_water(OFF);

            // 回流温度延迟时间为0即可进入
            if (get_uval(REFLUX_TEMP_3_DELAY_TIME) == 0)
            {
                // 获取当前值
                unsigned short current_ref_temp_value = get_uval(R_AI_TP_RF3);
                // 获取设定值
                unsigned short setting_ref_temp_value = get_val(P_AO_TP_RF3);

                // 冷水模式
                if (current_ref_temp_value > setting_ref_temp_value)
                {
                    // 冷水工作模式初始化为0
                    if (!get_uval(REFLOW_TEMP_3_COLD_WATER_OPERAT_INIT))
                    {
                        // 设置回流温度电磁阀的工作时间
                        set_uval(REFLUX_TEMP_3_WORK_TIME, get_uval(P_RTEMP_GZS));

                        // 设置实时工作周期为
                        set_uval(RETURN_TEMP_3_WORK_CYCLE, 0);

                        // 冷水工作模式初始化为1
                        set_uval(REFLOW_TEMP_3_COLD_WATER_OPERAT_INIT, 1);
                    }

                    // 当前冷水工作周期小于工作周期设定值
                    if (get_uval(RETURN_TEMP_3_WORK_CYCLE) < get_uval(P_RTEMP_ZQC))
                    {
                        // 工作时间大于0
                        if (get_uval(REFLUX_TEMP_3_WORK_TIME) > 0)
                        {
                            // 开启回流温度电磁阀
                            control_cool2(3, ON);
                        }
                        else
                        {
                            // 冷水工作模式为0
                            if (get_uval(REFLOW_TEMP_3_COLD_WATER_OPERAT_INIT) == 1)
                            {
                                // 设置回流温度电磁阀的停止时间
                                unsigned short setting_ref_temp_stop_time = get_uval(P_RTEMP_ZQS) - get_uval(P_RTEMP_GZS);
                                set_uval(RETURN_TEMP_3_STOP_TIME, setting_ref_temp_stop_time);

                                // 设置冷水工作模式为1
                                set_uval(REFLOW_TEMP_3_COLD_WATER_OPERAT_INIT, 2);
                            }

                            // 停止时间大于0
                            if (get_uval(RETURN_TEMP_3_STOP_TIME) > 0)
                            {
                                // 关闭启回流温度电磁阀
                                control_cool2(3, OFF);
                            }
                            else
                            {
                                if (get_uval(REFLOW_TEMP_3_COLD_WATER_OPERAT_INIT) == 2)
                                {
                                    // 设置回流温度电磁阀的工作时间
                                    set_uval(REFLUX_TEMP_3_WORK_TIME, get_uval(P_RTEMP_GZS));
                                    // 回流温度工作周期加1
                                    set_uval(RETURN_TEMP_3_WORK_CYCLE, get_uval(RETURN_TEMP_3_WORK_CYCLE) + 1);
                                    // 设置冷水工作模式为1
                                    set_uval(REFLOW_TEMP_3_COLD_WATER_OPERAT_INIT, 1);
                                }
                            }
                        }
                    }
                    else
                    {
                        // 开启回流温度电磁阀
                        control_cool2(3, ON);
                    }
                }
                // 当前值大于等于设定值
                else
                {
                    // 关闭启回流温度电磁阀
                    control_cool2(3, OFF);
                    // 设置热水工作模式为0
                    set_uval(REFLOW_TEMP_3_COLD_WATER_OPERAT_INIT, 0);
                }
            }
        }
        // 热水模式
        else
        {
            // 冷水模式初始化为0
            set_uval(REFLOW_TEMP_3_COLD_WATER_OPERAT_INIT, 0);
            // 开启热水
            control_cold_hot_water(ON);

            // 回流温度延迟时间为0即可进入
            if (get_uval(REFLUX_TEMP_3_DELAY_TIME) == 0)
            {
                // 获取当前值
                unsigned short current_ref_temp_value = get_uval(R_AI_TP_RF3);
                // 获取设定值
                unsigned short setting_ref_temp_value = get_val(P_AO_TP_RF3);

                // 热水模式
                if (current_ref_temp_value < setting_ref_temp_value)
                {
                    // 热水工作模式初始化为0
                    if (!get_uval(RETURN_TEMP_3_HOT_WATER_OPERAT_INIT))
                    {
                        // 设置回流温度电磁阀的工作时间
                        set_uval(REFLUX_TEMP_3_WORK_TIME, get_uval(P_RTEMP_GZS));

                        // 设置实时工作周期为
                        set_uval(RETURN_TEMP_3_WORK_CYCLE, 0);

                        // 热水工作模式初始化为1
                        set_uval(RETURN_TEMP_3_HOT_WATER_OPERAT_INIT, 1);
                    }

                    // 当前热水工作周期小于工作周期设定值
                    if (get_uval(RETURN_TEMP_3_WORK_CYCLE) < get_uval(P_RTEMP_ZQC))
                    {
                        // 工作时间大于0
                        if (get_uval(REFLUX_TEMP_3_WORK_TIME) > 0)
                        {
                            // 开启回流温度电磁阀
                            control_cool2(3, ON);
                        }
                        else
                        {
                            // 热水工作模式为0
                            if (get_uval(RETURN_TEMP_3_HOT_WATER_OPERAT_INIT) == 1)
                            {
                                // 设置回流温度电磁阀的停止时间
                                unsigned short setting_ref_temp_stop_time = get_uval(P_RTEMP_ZQS) - get_uval(P_RTEMP_GZS);
                                set_uval(RETURN_TEMP_3_STOP_TIME, setting_ref_temp_stop_time);

                                // 设置热水工作模式为1
                                set_uval(RETURN_TEMP_3_HOT_WATER_OPERAT_INIT, 2);
                            }

                            // 停止时间大于0
                            if (get_uval(RETURN_TEMP_3_STOP_TIME) > 0)
                            {
                                // 关闭启回流温度电磁阀
                                control_cool2(3, OFF);
                            }
                            else
                            {
                                if (get_uval(RETURN_TEMP_3_HOT_WATER_OPERAT_INIT) == 2)
                                {
                                    // 设置回流温度电磁阀的工作时间
                                    set_uval(REFLUX_TEMP_3_WORK_TIME, get_uval(P_RTEMP_GZS));
                                    // 回流温度工作周期加1
                                    set_uval(RETURN_TEMP_3_WORK_CYCLE, get_uval(RETURN_TEMP_3_WORK_CYCLE) + 1);
                                    // 设置热水工作模式为1
                                    set_uval(RETURN_TEMP_3_HOT_WATER_OPERAT_INIT, 1);
                                }
                            }
                        }
                    }
                    else
                    {
                        // 开启回流温度电磁阀
                        control_cool2(3, ON);
                    }
                }
                // 当前值大于等于设定值
                else
                {
                    // 关闭启回流温度电磁阀
                    control_cool2(3, OFF);
                    // 设置热水工作模式为0
                    set_uval(RETURN_TEMP_3_HOT_WATER_OPERAT_INIT, 0);
                }
            }
        }
    }
    else
    {
        control_cool2(3, OFF);
    }
}

//----------------------------------------------------------------------------------------------------------
// 逻辑处理线程-回流温度4-正常模式
//----------------------------------------------------------------------------------------------------------
void normal_logic_process_retemp4_func()
{
    zlog_debug(g_zlog_zc, "%-40s进入到回流温度4-正常模式", "[normal_logic_process_retemp4_func]");
    if (get_uval(REFLUX_TEMPERATURE) & 0x08)
    {
        // 获取当前天数和小时
        short sys_day_hour_time = get_val(R_RUN_DAY);
        // 获取开启冷水的天数和小数
        short cool_day_hour_time = get_val(P_PRE_TP_RF1_DAY);

        // 当系统时间大于等于开启冷水的时间时，开启冷水
        if (sys_day_hour_time >= cool_day_hour_time)
        {
            // 热水模式初始化为0
            set_uval(RETURN_TEMP_4_HOT_WATER_OPERAT_INIT, 0);
            // 开启冷水
            control_cold_hot_water(OFF);

            // 回流温度延迟时间为0即可进入
            if (get_uval(REFLUX_TEMP_4_DELAY_TIME) == 0)
            {
                // 获取当前值
                unsigned short current_ref_temp_value = get_uval(R_AI_TP_RF4);
                // 获取设定值
                unsigned short setting_ref_temp_value = get_val(P_AO_TP_RF4);

                // 冷水模式
                if (current_ref_temp_value > setting_ref_temp_value)
                {
                    // 冷水工作模式初始化为0
                    if (!get_uval(REFLOW_TEMP_4_COLD_WATER_OPERAT_INIT))
                    {
                        // 设置回流温度电磁阀的工作时间
                        set_uval(REFLUX_TEMP_4_WORK_TIME, get_uval(P_RTEMP_GZS));

                        // 设置实时工作周期为
                        set_uval(RETURN_TEMP_4_WORK_CYCLE, 0);

                        // 冷水工作模式初始化为1
                        set_uval(REFLOW_TEMP_4_COLD_WATER_OPERAT_INIT, 1);
                    }

                    // 当前冷水工作周期小于工作周期设定值
                    if (get_uval(RETURN_TEMP_4_WORK_CYCLE) < get_uval(P_RTEMP_ZQC))
                    {
                        // 工作时间大于0
                        if (get_uval(REFLUX_TEMP_4_WORK_TIME) > 0)
                        {
                            // 开启回流温度电磁阀
                            control_cool2(1, ON);
                        }
                        else
                        {
                            // 冷水工作模式为0
                            if (get_uval(REFLOW_TEMP_4_COLD_WATER_OPERAT_INIT) == 1)
                            {
                                // 设置回流温度电磁阀的停止时间
                                unsigned short setting_ref_temp_stop_time = get_uval(P_RTEMP_ZQS) - get_uval(P_RTEMP_GZS);
                                set_uval(RETURN_TEMP_4_STOP_TIME, setting_ref_temp_stop_time);

                                // 设置冷水工作模式为1
                                set_uval(REFLOW_TEMP_4_COLD_WATER_OPERAT_INIT, 2);
                            }

                            // 停止时间大于0
                            if (get_uval(RETURN_TEMP_4_STOP_TIME) > 0)
                            {
                                // 关闭启回流温度电磁阀
                                control_cool2(4, OFF);
                            }
                            else
                            {
                                if (get_uval(REFLOW_TEMP_4_COLD_WATER_OPERAT_INIT) == 2)
                                {
                                    // 设置回流温度电磁阀的工作时间
                                    set_uval(REFLUX_TEMP_4_WORK_TIME, get_uval(P_RTEMP_GZS));
                                    // 回流温度工作周期加1
                                    set_uval(RETURN_TEMP_4_WORK_CYCLE, get_uval(RETURN_TEMP_4_WORK_CYCLE) + 1);
                                    // 设置冷水工作模式为1
                                    set_uval(REFLOW_TEMP_4_COLD_WATER_OPERAT_INIT, 1);
                                }
                            }
                        }
                    }
                    else
                    {
                        // 开启回流温度电磁阀
                        control_cool2(4, ON);
                    }
                }
                // 当前值大于等于设定值
                else
                {
                    // 关闭启回流温度电磁阀
                    control_cool2(4, OFF);
                    // 设置热水工作模式为0
                    set_uval(REFLOW_TEMP_4_COLD_WATER_OPERAT_INIT, 0);
                }
            }
        }
        // 热水模式
        else
        {
            // 冷水模式初始化为0
            set_uval(REFLOW_TEMP_4_COLD_WATER_OPERAT_INIT, 0);
            // 开启热水
            control_cold_hot_water(ON);

            // 回流温度延迟时间为0即可进入
            if (get_uval(REFLUX_TEMP_4_DELAY_TIME) == 0)
            {
                // 获取当前值
                unsigned short current_ref_temp_value = get_uval(R_AI_TP_RF4);
                // 获取设定值
                unsigned short setting_ref_temp_value = get_val(P_AO_TP_RF4);

                // 热水模式
                if (current_ref_temp_value < setting_ref_temp_value)
                {
                    // 热水工作模式初始化为0
                    if (!get_uval(RETURN_TEMP_4_HOT_WATER_OPERAT_INIT))
                    {
                        // 设置回流温度电磁阀的工作时间
                        set_uval(REFLUX_TEMP_4_WORK_TIME, get_uval(P_RTEMP_GZS));

                        // 设置实时工作周期为
                        set_uval(RETURN_TEMP_4_WORK_CYCLE, 0);

                        // 热水工作模式初始化为1
                        set_uval(RETURN_TEMP_4_HOT_WATER_OPERAT_INIT, 1);
                    }

                    // 当前热水工作周期小于工作周期设定值
                    if (get_uval(RETURN_TEMP_4_WORK_CYCLE) < get_uval(P_RTEMP_ZQC))
                    {
                        // 工作时间大于0
                        if (get_uval(REFLUX_TEMP_4_WORK_TIME) > 0)
                        {
                            // 开启回流温度电磁阀
                            control_cool2(4, ON);
                        }
                        else
                        {
                            // 热水工作模式为0
                            if (get_uval(RETURN_TEMP_4_HOT_WATER_OPERAT_INIT) == 1)
                            {
                                // 设置回流温度电磁阀的停止时间
                                unsigned short setting_ref_temp_stop_time = get_uval(P_RTEMP_ZQS) - get_uval(P_RTEMP_GZS);
                                set_uval(RETURN_TEMP_4_STOP_TIME, setting_ref_temp_stop_time);

                                // 设置热水工作模式为1
                                set_uval(RETURN_TEMP_4_HOT_WATER_OPERAT_INIT, 2);
                            }

                            // 停止时间大于0
                            if (get_uval(RETURN_TEMP_4_STOP_TIME) > 0)
                            {
                                // 关闭启回流温度电磁阀
                                control_cool2(4, OFF);
                            }
                            else
                            {
                                if (get_uval(RETURN_TEMP_4_HOT_WATER_OPERAT_INIT) == 2)
                                {
                                    // 设置回流温度电磁阀的工作时间
                                    set_uval(REFLUX_TEMP_4_WORK_TIME, get_uval(P_RTEMP_GZS));
                                    // 回流温度工作周期加1
                                    set_uval(RETURN_TEMP_4_WORK_CYCLE, get_uval(RETURN_TEMP_4_WORK_CYCLE) + 1);
                                    // 设置热水工作模式为1
                                    set_uval(RETURN_TEMP_4_HOT_WATER_OPERAT_INIT, 1);
                                }
                            }
                        }
                    }
                    else
                    {
                        // 开启回流温度电磁阀
                        control_cool2(4, ON);
                    }
                }
                // 当前值大于等于设定值
                else
                {
                    // 关闭启回流温度电磁阀
                    control_cool2(4, OFF);
                    // 设置热水工作模式为0
                    set_uval(RETURN_TEMP_4_HOT_WATER_OPERAT_INIT, 0);
                }
            }
        }
    }
    else
    {
        control_cool2(4, OFF);
    }
}

//----------------------------------------------------------------------------------------------------------
// 逻辑处理线程-回流温度1-预热模式
//----------------------------------------------------------------------------------------------------------
void pre_logic_process_retemp1_func()
{
    zlog_debug(g_zlog_zc, "%-40s进入到回流温度1-预热模式", "[pre_logic_process_retemp1_func]");
    if (get_uval(REFLUX_TEMPERATURE) & 0x01)
    {
        // 获取当前天数和小时
        short sys_day_hour_time = get_val(R_RUN_DAY);
        // 获取预热回流温度值
        float setting_pre_ref_temp_value = get_fval(P_PRE_TP_RF1);

        // 当预热回流温度值大于0时，开启冷水
        if (setting_pre_ref_temp_value > 0)
        {
            // 热水模式初始化为0
            set_uval(RETURN_TEMP_1_HOT_WATER_OPERAT_INIT, 0);
            // 开启冷水
            control_cold_hot_water(OFF);

            // 回流温度延迟时间为0即可进入
            if (get_uval(REFLUX_TEMP_1_DELAY_TIME) == 0)
            {
                // 获取当前值
                unsigned short current_ref_temp_value = get_uval(R_AI_TP_RF1);
                // 获取设定值
                unsigned short setting_ref_temp_value = get_val(P_AO_TP_RF1);

                // 冷水模式
                if (current_ref_temp_value > setting_ref_temp_value)
                {
                    // 冷水工作模式初始化为0
                    if (!get_uval(REFLOW_TEMP_1_COLD_WATER_OPERAT_INIT))
                    {
                        // 设置回流温度电磁阀的工作时间
                        set_uval(REFLUX_TEMP_1_WORK_TIME, get_uval(P_RTEMP_GZS));

                        // 设置实时工作周期为
                        set_uval(RETURN_TEMP_1_WORK_CYCLE, 0);

                        // 冷水工作模式初始化为1
                        set_uval(REFLOW_TEMP_1_COLD_WATER_OPERAT_INIT, 1);
                    }

                    // 当前冷水工作周期小于工作周期设定值
                    if (get_uval(RETURN_TEMP_1_WORK_CYCLE) < get_uval(P_RTEMP_ZQC))
                    {
                        // 工作时间大于0
                        if (get_uval(REFLUX_TEMP_1_WORK_TIME) > 0)
                        {
                            // 开启回流温度电磁阀
                            control_cool2(1, ON);
                        }
                        else
                        {
                            // 冷水工作模式为0
                            if (get_uval(REFLOW_TEMP_1_COLD_WATER_OPERAT_INIT) == 1)
                            {
                                // 设置回流温度电磁阀的停止时间
                                unsigned short setting_ref_temp_stop_time = get_uval(P_RTEMP_ZQS) - get_uval(P_RTEMP_GZS);
                                set_uval(RETURN_TEMP_1_STOP_TIME, setting_ref_temp_stop_time);

                                // 设置冷水工作模式为1
                                set_uval(REFLOW_TEMP_1_COLD_WATER_OPERAT_INIT, 2);
                            }

                            // 停止时间大于0
                            if (get_uval(RETURN_TEMP_1_STOP_TIME) > 0)
                            {
                                // 关闭启回流温度电磁阀
                                control_cool2(1, OFF);
                            }
                            else
                            {
                                if (get_uval(REFLOW_TEMP_1_COLD_WATER_OPERAT_INIT) == 2)
                                {
                                    // 设置回流温度电磁阀的工作时间
                                    set_uval(REFLUX_TEMP_1_WORK_TIME, get_uval(P_RTEMP_GZS));
                                    // 回流温度工作周期加1
                                    set_uval(RETURN_TEMP_1_WORK_CYCLE, get_uval(RETURN_TEMP_1_WORK_CYCLE) + 1);
                                    // 设置冷水工作模式为1
                                    set_uval(REFLOW_TEMP_1_COLD_WATER_OPERAT_INIT, 1);
                                }
                            }
                        }
                    }
                    else
                    {
                        // 开启回流温度电磁阀
                        control_cool2(1, ON);
                    }
                }
                // 当前值大于等于设定值
                else
                {
                    // 关闭启回流温度电磁阀
                    control_cool2(1, OFF);
                    // 设置热水工作模式为0
                    set_uval(REFLOW_TEMP_1_COLD_WATER_OPERAT_INIT, 0);
                }
            }
        }
        // 热水模式
        else
        {
            // 冷水模式初始化为0
            set_uval(REFLOW_TEMP_1_COLD_WATER_OPERAT_INIT, 0);
            // 开启热水
            control_cold_hot_water(ON);

            // 回流温度延迟时间为0即可进入
            if (get_uval(REFLUX_TEMP_1_DELAY_TIME) == 0)
            {
                // 获取当前值
                unsigned short current_ref_temp_value = get_uval(R_AI_TP_RF1);
                // 获取设定值
                unsigned short setting_ref_temp_value = get_val(P_AO_TP_RF1);

                // 热水模式
                if (current_ref_temp_value < setting_ref_temp_value)
                {
                    // 热水工作模式初始化为0
                    if (!get_uval(RETURN_TEMP_1_HOT_WATER_OPERAT_INIT))
                    {
                        // 设置回流温度电磁阀的工作时间
                        set_uval(REFLUX_TEMP_1_WORK_TIME, get_uval(P_RTEMP_GZS));

                        // 设置实时工作周期为
                        set_uval(RETURN_TEMP_1_WORK_CYCLE, 0);

                        // 热水工作模式初始化为1
                        set_uval(RETURN_TEMP_1_HOT_WATER_OPERAT_INIT, 1);
                    }

                    // 当前热水工作周期小于工作周期设定值
                    if (get_uval(RETURN_TEMP_1_WORK_CYCLE) < get_uval(P_RTEMP_ZQC))
                    {
                        // 工作时间大于0
                        if (get_uval(REFLUX_TEMP_1_WORK_TIME) > 0)
                        {
                            // 开启回流温度电磁阀
                            control_cool2(1, ON);
                        }
                        else
                        {
                            // 热水工作模式为0
                            if (get_uval(RETURN_TEMP_1_HOT_WATER_OPERAT_INIT) == 1)
                            {
                                // 设置回流温度电磁阀的停止时间
                                unsigned short setting_ref_temp_stop_time = get_uval(P_RTEMP_ZQS) - get_uval(P_RTEMP_GZS);
                                set_uval(RETURN_TEMP_1_STOP_TIME, setting_ref_temp_stop_time);

                                // 设置热水工作模式为1
                                set_uval(RETURN_TEMP_1_HOT_WATER_OPERAT_INIT, 2);
                            }

                            // 停止时间大于0
                            if (get_uval(RETURN_TEMP_1_STOP_TIME) > 0)
                            {
                                // 关闭启回流温度电磁阀
                                control_cool2(1, OFF);
                            }
                            else
                            {
                                if (get_uval(RETURN_TEMP_1_HOT_WATER_OPERAT_INIT) == 2)
                                {
                                    // 设置回流温度电磁阀的工作时间
                                    set_uval(REFLUX_TEMP_1_WORK_TIME, get_uval(P_RTEMP_GZS));
                                    // 回流温度工作周期加1
                                    set_uval(RETURN_TEMP_1_WORK_CYCLE, get_uval(RETURN_TEMP_1_WORK_CYCLE) + 1);
                                    // 设置热水工作模式为1
                                    set_uval(RETURN_TEMP_1_HOT_WATER_OPERAT_INIT, 1);
                                }
                            }
                        }
                    }
                    else
                    {
                        // 开启回流温度电磁阀
                        control_cool2(1, ON);
                    }
                }
                // 当前值大于等于设定值
                else
                {
                    // 关闭启回流温度电磁阀
                    control_cool2(1, OFF);
                    // 设置热水工作模式为0
                    set_uval(RETURN_TEMP_1_HOT_WATER_OPERAT_INIT, 0);
                }
            }
        }
    }
    else
    {
        control_cool2(1, OFF);
    }
}

//----------------------------------------------------------------------------------------------------------
// 逻辑处理线程-回流温度2-预热模式
//----------------------------------------------------------------------------------------------------------
void pre_logic_process_retemp2_func()
{
    zlog_debug(g_zlog_zc, "%-40s进入到回流温度2-预热模式", "[pre_logic_process_retemp2_func]");
    if (get_uval(REFLUX_TEMPERATURE) & 0x02)
    {
        // 获取当前天数和小时
        short sys_day_hour_time = get_val(R_RUN_DAY);
        // 获取预热回流温度值
        float setting_pre_ref_temp_value = get_fval(P_PRE_TP_RF2);

        // 当预热回流温度值大于0时，开启冷水
        if (setting_pre_ref_temp_value > 0)
        {
            // 热水模式初始化为0
            set_uval(RETURN_TEMP_2_HOT_WATER_OPERAT_INIT, 0);
            // 开启冷水
            control_cold_hot_water(OFF);

            // 回流温度延迟时间为0即可进入
            if (get_uval(REFLUX_TEMP_2_DELAY_TIME) == 0)
            {
                // 获取当前值
                unsigned short current_ref_temp_value = get_uval(R_AI_TP_RF2);
                // 获取设定值
                unsigned short setting_ref_temp_value = get_val(P_AO_TP_RF2);

                // 冷水模式
                if (current_ref_temp_value > setting_ref_temp_value)
                {
                    // 冷水工作模式初始化为0
                    if (!get_uval(REFLOW_TEMP_2_COLD_WATER_OPERAT_INIT))
                    {
                        // 设置回流温度电磁阀的工作时间
                        set_uval(REFLUX_TEMP_2_WORK_TIME, get_uval(P_RTEMP_GZS));

                        // 设置实时工作周期为
                        set_uval(RETURN_TEMP_2_WORK_CYCLE, 0);

                        // 冷水工作模式初始化为1
                        set_uval(REFLOW_TEMP_2_COLD_WATER_OPERAT_INIT, 1);
                    }

                    // 当前冷水工作周期小于工作周期设定值
                    if (get_uval(RETURN_TEMP_2_WORK_CYCLE) < get_uval(P_RTEMP_ZQC))
                    {
                        // 工作时间大于0
                        if (get_uval(REFLUX_TEMP_2_WORK_TIME) > 0)
                        {
                            // 开启回流温度电磁阀
                            control_cool2(2, ON);
                        }
                        else
                        {
                            // 冷水工作模式为0
                            if (get_uval(REFLOW_TEMP_2_COLD_WATER_OPERAT_INIT) == 1)
                            {
                                // 设置回流温度电磁阀的停止时间
                                unsigned short setting_ref_temp_stop_time = get_uval(P_RTEMP_ZQS) - get_uval(P_RTEMP_GZS);
                                set_uval(RETURN_TEMP_2_STOP_TIME, setting_ref_temp_stop_time);

                                // 设置冷水工作模式为1
                                set_uval(REFLOW_TEMP_2_COLD_WATER_OPERAT_INIT, 2);
                            }

                            // 停止时间大于0
                            if (get_uval(RETURN_TEMP_2_STOP_TIME) > 0)
                            {
                                // 关闭启回流温度电磁阀
                                control_cool2(2, OFF);
                            }
                            else
                            {
                                if (get_uval(REFLOW_TEMP_2_COLD_WATER_OPERAT_INIT) == 2)
                                {
                                    // 设置回流温度电磁阀的工作时间
                                    set_uval(REFLUX_TEMP_2_WORK_TIME, get_uval(P_RTEMP_GZS));
                                    // 回流温度工作周期加1
                                    set_uval(RETURN_TEMP_2_WORK_CYCLE, get_uval(RETURN_TEMP_2_WORK_CYCLE) + 1);
                                    // 设置冷水工作模式为1
                                    set_uval(REFLOW_TEMP_2_COLD_WATER_OPERAT_INIT, 1);
                                }
                            }
                        }
                    }
                    else
                    {
                        // 开启回流温度电磁阀
                        control_cool2(2, ON);
                    }
                }
                // 当前值大于等于设定值
                else
                {
                    // 关闭启回流温度电磁阀
                    control_cool2(2, OFF);
                    // 设置热水工作模式为0
                    set_uval(REFLOW_TEMP_2_COLD_WATER_OPERAT_INIT, 0);
                }
            }
        }
        // 热水模式
        else
        {
            // 冷水模式初始化为0
            set_uval(REFLOW_TEMP_2_COLD_WATER_OPERAT_INIT, 0);
            // 开启热水
            control_cold_hot_water(ON);

            // 回流温度延迟时间为0即可进入
            if (get_uval(REFLUX_TEMP_2_DELAY_TIME) == 0)
            {
                // 获取当前值
                unsigned short current_ref_temp_value = get_uval(R_AI_TP_RF2);
                // 获取设定值
                unsigned short setting_ref_temp_value = get_val(P_AO_TP_RF2);

                // 热水模式
                if (current_ref_temp_value < setting_ref_temp_value)
                {
                    // 热水工作模式初始化为0
                    if (!get_uval(RETURN_TEMP_2_HOT_WATER_OPERAT_INIT))
                    {
                        // 设置回流温度电磁阀的工作时间
                        set_uval(REFLUX_TEMP_2_WORK_TIME, get_uval(P_RTEMP_GZS));

                        // 设置实时工作周期为
                        set_uval(RETURN_TEMP_2_WORK_CYCLE, 0);

                        // 热水工作模式初始化为1
                        set_uval(RETURN_TEMP_2_HOT_WATER_OPERAT_INIT, 1);
                    }

                    // 当前热水工作周期小于工作周期设定值
                    if (get_uval(RETURN_TEMP_2_WORK_CYCLE) < get_uval(P_RTEMP_ZQC))
                    {
                        // 工作时间大于0
                        if (get_uval(REFLUX_TEMP_2_WORK_TIME) > 0)
                        {
                            // 开启回流温度电磁阀
                            control_cool2(2, ON);
                        }
                        else
                        {
                            // 热水工作模式为0
                            if (get_uval(RETURN_TEMP_2_HOT_WATER_OPERAT_INIT) == 1)
                            {
                                // 设置回流温度电磁阀的停止时间
                                unsigned short setting_ref_temp_stop_time = get_uval(P_RTEMP_ZQS) - get_uval(P_RTEMP_GZS);
                                set_uval(RETURN_TEMP_2_STOP_TIME, setting_ref_temp_stop_time);

                                // 设置热水工作模式为1
                                set_uval(RETURN_TEMP_2_HOT_WATER_OPERAT_INIT, 2);
                            }

                            // 停止时间大于0
                            if (get_uval(RETURN_TEMP_2_STOP_TIME) > 0)
                            {
                                // 关闭启回流温度电磁阀
                                control_cool2(2, OFF);
                            }
                            else
                            {
                                if (get_uval(RETURN_TEMP_2_HOT_WATER_OPERAT_INIT) == 2)
                                {
                                    // 设置回流温度电磁阀的工作时间
                                    set_uval(REFLUX_TEMP_2_WORK_TIME, get_uval(P_RTEMP_GZS));
                                    // 回流温度工作周期加1
                                    set_uval(RETURN_TEMP_2_WORK_CYCLE, get_uval(RETURN_TEMP_2_WORK_CYCLE) + 1);
                                    // 设置热水工作模式为1
                                    set_uval(RETURN_TEMP_2_HOT_WATER_OPERAT_INIT, 1);
                                }
                            }
                        }
                    }
                    else
                    {
                        // 开启回流温度电磁阀
                        control_cool2(2, ON);
                    }
                }
                // 当前值大于等于设定值
                else
                {
                    // 关闭启回流温度电磁阀
                    control_cool2(2, OFF);
                    // 设置热水工作模式为0
                    set_uval(RETURN_TEMP_2_HOT_WATER_OPERAT_INIT, 0);
                }
            }
        }
    }
    else
    {
        control_cool2(2, OFF);
    }
}

//----------------------------------------------------------------------------------------------------------
// 逻辑处理线程-回流温度3-预热模式
//----------------------------------------------------------------------------------------------------------
void pre_logic_process_retemp3_func()
{
    zlog_debug(g_zlog_zc, "%-40s进入到回流温度3-预热模式", "[pre_logic_process_retemp3_func]");
    if (get_uval(REFLUX_TEMPERATURE) & 0x04)
    {

        // 获取当前天数和小时
        short sys_day_hour_time = get_val(R_RUN_DAY);
        // 获取预热回流温度值
        float setting_pre_ref_temp_value = get_fval(P_PRE_TP_RF3);

        // 当预热回流温度值大于0时，开启冷水
        if (setting_pre_ref_temp_value > 0)
        {
            // 热水模式初始化为0
            set_uval(RETURN_TEMP_3_HOT_WATER_OPERAT_INIT, 0);
            // 开启冷水
            control_cold_hot_water(OFF);

            // 回流温度延迟时间为0即可进入
            if (get_uval(REFLUX_TEMP_3_DELAY_TIME) == 0)
            {
                // 获取当前值
                unsigned short current_ref_temp_value = get_uval(R_AI_TP_RF3);
                // 获取设定值
                unsigned short setting_ref_temp_value = get_val(P_AO_TP_RF3);

                // 冷水模式
                if (current_ref_temp_value > setting_ref_temp_value)
                {
                    // 冷水工作模式初始化为0
                    if (!get_uval(REFLOW_TEMP_3_COLD_WATER_OPERAT_INIT))
                    {
                        // 设置回流温度电磁阀的工作时间
                        set_uval(REFLUX_TEMP_3_WORK_TIME, get_uval(P_RTEMP_GZS));

                        // 设置实时工作周期为
                        set_uval(RETURN_TEMP_3_WORK_CYCLE, 0);

                        // 冷水工作模式初始化为1
                        set_uval(REFLOW_TEMP_3_COLD_WATER_OPERAT_INIT, 1);
                    }

                    // 当前冷水工作周期小于工作周期设定值
                    if (get_uval(RETURN_TEMP_3_WORK_CYCLE) < get_uval(P_RTEMP_ZQC))
                    {
                        // 工作时间大于0
                        if (get_uval(REFLUX_TEMP_3_WORK_TIME) > 0)
                        {
                            // 开启回流温度电磁阀
                            control_cool2(3, ON);
                        }
                        else
                        {
                            // 冷水工作模式为0
                            if (get_uval(REFLOW_TEMP_3_COLD_WATER_OPERAT_INIT) == 1)
                            {
                                // 设置回流温度电磁阀的停止时间
                                unsigned short setting_ref_temp_stop_time = get_uval(P_RTEMP_ZQS) - get_uval(P_RTEMP_GZS);
                                set_uval(RETURN_TEMP_3_STOP_TIME, setting_ref_temp_stop_time);

                                // 设置冷水工作模式为1
                                set_uval(REFLOW_TEMP_3_COLD_WATER_OPERAT_INIT, 2);
                            }

                            // 停止时间大于0
                            if (get_uval(RETURN_TEMP_3_STOP_TIME) > 0)
                            {
                                // 关闭启回流温度电磁阀
                                control_cool2(3, OFF);
                            }
                            else
                            {
                                if (get_uval(REFLOW_TEMP_3_COLD_WATER_OPERAT_INIT) == 2)
                                {
                                    // 设置回流温度电磁阀的工作时间
                                    set_uval(REFLUX_TEMP_3_WORK_TIME, get_uval(P_RTEMP_GZS));
                                    // 回流温度工作周期加1
                                    set_uval(RETURN_TEMP_3_WORK_CYCLE, get_uval(RETURN_TEMP_3_WORK_CYCLE) + 1);
                                    // 设置冷水工作模式为1
                                    set_uval(REFLOW_TEMP_3_COLD_WATER_OPERAT_INIT, 1);
                                }
                            }
                        }
                    }
                    else
                    {
                        // 开启回流温度电磁阀
                        control_cool2(3, ON);
                    }
                }
                // 当前值大于等于设定值
                else
                {
                    // 关闭启回流温度电磁阀
                    control_cool2(3, OFF);
                    // 设置热水工作模式为0
                    set_uval(REFLOW_TEMP_3_COLD_WATER_OPERAT_INIT, 0);
                }
            }
        }
        // 热水模式
        else
        {
            // 冷水模式初始化为0
            set_uval(REFLOW_TEMP_3_COLD_WATER_OPERAT_INIT, 0);
            // 开启热水
            control_cold_hot_water(ON);

            // 回流温度延迟时间为0即可进入
            if (get_uval(REFLUX_TEMP_3_DELAY_TIME) == 0)
            {
                // 获取当前值
                unsigned short current_ref_temp_value = get_uval(R_AI_TP_RF3);
                // 获取设定值
                unsigned short setting_ref_temp_value = get_val(P_AO_TP_RF3);

                // 热水模式
                if (current_ref_temp_value < setting_ref_temp_value)
                {
                    // 热水工作模式初始化为0
                    if (!get_uval(RETURN_TEMP_3_HOT_WATER_OPERAT_INIT))
                    {
                        // 设置回流温度电磁阀的工作时间
                        set_uval(REFLUX_TEMP_3_WORK_TIME, get_uval(P_RTEMP_GZS));

                        // 设置实时工作周期为
                        set_uval(RETURN_TEMP_3_WORK_CYCLE, 0);

                        // 热水工作模式初始化为1
                        set_uval(RETURN_TEMP_3_HOT_WATER_OPERAT_INIT, 1);
                    }

                    // 当前热水工作周期小于工作周期设定值
                    if (get_uval(RETURN_TEMP_3_WORK_CYCLE) < get_uval(P_RTEMP_ZQC))
                    {
                        // 工作时间大于0
                        if (get_uval(REFLUX_TEMP_3_WORK_TIME) > 0)
                        {
                            // 开启回流温度电磁阀
                            control_cool2(3, ON);
                        }
                        else
                        {
                            // 热水工作模式为0
                            if (get_uval(RETURN_TEMP_3_HOT_WATER_OPERAT_INIT) == 1)
                            {
                                // 设置回流温度电磁阀的停止时间
                                unsigned short setting_ref_temp_stop_time = get_uval(P_RTEMP_ZQS) - get_uval(P_RTEMP_GZS);
                                set_uval(RETURN_TEMP_3_STOP_TIME, setting_ref_temp_stop_time);

                                // 设置热水工作模式为1
                                set_uval(RETURN_TEMP_3_HOT_WATER_OPERAT_INIT, 2);
                            }

                            // 停止时间大于0
                            if (get_uval(RETURN_TEMP_3_STOP_TIME) > 0)
                            {
                                // 关闭启回流温度电磁阀
                                control_cool2(3, OFF);
                            }
                            else
                            {
                                if (get_uval(RETURN_TEMP_3_HOT_WATER_OPERAT_INIT) == 2)
                                {
                                    // 设置回流温度电磁阀的工作时间
                                    set_uval(REFLUX_TEMP_3_WORK_TIME, get_uval(P_RTEMP_GZS));
                                    // 回流温度工作周期加1
                                    set_uval(RETURN_TEMP_3_WORK_CYCLE, get_uval(RETURN_TEMP_3_WORK_CYCLE) + 1);
                                    // 设置热水工作模式为1
                                    set_uval(RETURN_TEMP_3_HOT_WATER_OPERAT_INIT, 1);
                                }
                            }
                        }
                    }
                    else
                    {
                        // 开启回流温度电磁阀
                        control_cool2(3, ON);
                    }
                }
                // 当前值大于等于设定值
                else
                {
                    // 关闭启回流温度电磁阀
                    control_cool2(3, OFF);
                    // 设置热水工作模式为0
                    set_uval(RETURN_TEMP_3_HOT_WATER_OPERAT_INIT, 0);
                }
            }
        }
    }
    else
    {
        control_cool2(3, OFF);
    }
}

//----------------------------------------------------------------------------------------------------------
// 逻辑处理线程-回流温度4-预热模式
//----------------------------------------------------------------------------------------------------------
void pre_logic_process_retemp4_func()
{
    zlog_debug(g_zlog_zc, "%-40s进入到回流温度4-预热模式", "[pre_logic_process_retemp4_func]");
    if (get_uval(REFLUX_TEMPERATURE) & 0x08)
    {
        // 获取当前天数和小时
        short sys_day_hour_time = get_val(R_RUN_DAY);
        // 获取预热回流温度值
        float setting_pre_ref_temp_value = get_fval(P_PRE_TP_RF4);

        // 当预热回流温度值大于0时，开启冷水
        if (setting_pre_ref_temp_value > 0)
        {
            // 热水模式初始化为0
            set_uval(RETURN_TEMP_4_HOT_WATER_OPERAT_INIT, 0);
            // 开启冷水
            control_cold_hot_water(OFF);

            // 回流温度延迟时间为0即可进入
            if (get_uval(REFLUX_TEMP_4_DELAY_TIME) == 0)
            {
                // 获取当前值
                unsigned short current_ref_temp_value = get_uval(R_AI_TP_RF4);
                // 获取设定值
                unsigned short setting_ref_temp_value = get_uval(P_AO_TP_RF4);

                // 冷水模式
                if (current_ref_temp_value > setting_ref_temp_value)
                {
                    // 冷水工作模式初始化为0
                    if (!get_uval(REFLOW_TEMP_4_COLD_WATER_OPERAT_INIT))
                    {
                        // 设置回流温度电磁阀的工作时间
                        set_uval(REFLUX_TEMP_4_WORK_TIME, get_uval(P_RTEMP_GZS));

                        // 设置实时工作周期为
                        set_uval(RETURN_TEMP_4_WORK_CYCLE, 0);

                        // 冷水工作模式初始化为1
                        set_uval(REFLOW_TEMP_4_COLD_WATER_OPERAT_INIT, 1);
                    }

                    // 当前冷水工作周期小于工作周期设定值
                    if (get_uval(RETURN_TEMP_4_WORK_CYCLE) < get_uval(P_RTEMP_ZQC))
                    {
                        // 工作时间大于0
                        if (get_uval(REFLUX_TEMP_4_WORK_TIME) > 0)
                        {
                            // 开启回流温度电磁阀
                            control_cool2(1, ON);
                        }
                        else
                        {
                            // 冷水工作模式为0
                            if (get_uval(REFLOW_TEMP_4_COLD_WATER_OPERAT_INIT) == 1)
                            {
                                // 设置回流温度电磁阀的停止时间
                                unsigned short setting_ref_temp_stop_time = get_uval(P_RTEMP_ZQS) - get_uval(P_RTEMP_GZS);
                                set_uval(RETURN_TEMP_4_STOP_TIME, setting_ref_temp_stop_time);

                                // 设置冷水工作模式为1
                                set_uval(REFLOW_TEMP_4_COLD_WATER_OPERAT_INIT, 2);
                            }

                            // 停止时间大于0
                            if (get_uval(RETURN_TEMP_4_STOP_TIME) > 0)
                            {
                                // 关闭启回流温度电磁阀
                                control_cool2(4, OFF);
                            }
                            else
                            {
                                if (get_uval(REFLOW_TEMP_4_COLD_WATER_OPERAT_INIT) == 2)
                                {
                                    // 设置回流温度电磁阀的工作时间
                                    set_uval(REFLUX_TEMP_4_WORK_TIME, get_uval(P_RTEMP_GZS));
                                    // 回流温度工作周期加1
                                    set_uval(RETURN_TEMP_4_WORK_CYCLE, get_uval(RETURN_TEMP_4_WORK_CYCLE) + 1);
                                    // 设置冷水工作模式为1
                                    set_uval(REFLOW_TEMP_4_COLD_WATER_OPERAT_INIT, 1);
                                }
                            }
                        }
                    }
                    else
                    {
                        // 开启回流温度电磁阀
                        control_cool2(4, ON);
                    }
                }
                // 当前值大于等于设定值
                else
                {
                    // 关闭启回流温度电磁阀
                    control_cool2(4, OFF);
                    // 设置热水工作模式为0
                    set_uval(REFLOW_TEMP_4_COLD_WATER_OPERAT_INIT, 0);
                }
            }
        }
        // 热水模式
        else
        {
            // 冷水模式初始化为0
            set_uval(REFLOW_TEMP_4_COLD_WATER_OPERAT_INIT, 0);
            // 开启热水
            control_cold_hot_water(ON);

            // 回流温度延迟时间为0即可进入
            if (get_uval(REFLUX_TEMP_4_DELAY_TIME) == 0)
            {
                // 获取当前值
                unsigned short current_ref_temp_value = get_uval(R_AI_TP_RF4);
                // 获取设定值
                unsigned short setting_ref_temp_value = get_val(P_AO_TP_RF4);

                // 热水模式
                if (current_ref_temp_value < setting_ref_temp_value)
                {
                    // 热水工作模式初始化为0
                    if (!get_uval(RETURN_TEMP_4_HOT_WATER_OPERAT_INIT))
                    {
                        // 设置回流温度电磁阀的工作时间
                        set_uval(REFLUX_TEMP_4_WORK_TIME, get_uval(P_RTEMP_GZS));

                        // 设置实时工作周期为
                        set_uval(RETURN_TEMP_4_WORK_CYCLE, 0);

                        // 热水工作模式初始化为1
                        set_uval(RETURN_TEMP_4_HOT_WATER_OPERAT_INIT, 1);
                    }

                    // 当前热水工作周期小于工作周期设定值
                    if (get_uval(RETURN_TEMP_4_WORK_CYCLE) < get_uval(P_RTEMP_ZQC))
                    {
                        // 工作时间大于0
                        if (get_uval(REFLUX_TEMP_4_WORK_TIME) > 0)
                        {
                            // 开启回流温度电磁阀
                            control_cool2(4, ON);
                        }
                        else
                        {
                            // 热水工作模式为0
                            if (get_uval(RETURN_TEMP_4_HOT_WATER_OPERAT_INIT) == 1)
                            {
                                // 设置回流温度电磁阀的停止时间
                                unsigned short setting_ref_temp_stop_time = get_uval(P_RTEMP_ZQS) - get_uval(P_RTEMP_GZS);
                                set_uval(RETURN_TEMP_4_STOP_TIME, setting_ref_temp_stop_time);

                                // 设置热水工作模式为1
                                set_uval(RETURN_TEMP_4_HOT_WATER_OPERAT_INIT, 2);
                            }

                            // 停止时间大于0
                            if (get_uval(RETURN_TEMP_4_STOP_TIME) > 0)
                            {
                                // 关闭启回流温度电磁阀
                                control_cool2(4, OFF);
                            }
                            else
                            {
                                if (get_uval(RETURN_TEMP_4_HOT_WATER_OPERAT_INIT) == 2)
                                {
                                    // 设置回流温度电磁阀的工作时间
                                    set_uval(REFLUX_TEMP_4_WORK_TIME, get_uval(P_RTEMP_GZS));
                                    // 回流温度工作周期加1
                                    set_uval(RETURN_TEMP_4_WORK_CYCLE, get_uval(RETURN_TEMP_4_WORK_CYCLE) + 1);
                                    // 设置热水工作模式为1
                                    set_uval(RETURN_TEMP_4_HOT_WATER_OPERAT_INIT, 1);
                                }
                            }
                        }
                    }
                    else
                    {
                        // 开启回流温度电磁阀
                        control_cool2(4, ON);
                    }
                }
                // 当前值大于等于设定值
                else
                {
                    // 关闭启回流温度电磁阀
                    control_cool2(4, OFF);
                    // 设置热水工作模式为0
                    set_uval(RETURN_TEMP_4_HOT_WATER_OPERAT_INIT, 0);
                }
            }
        }
    }
    else
    {
        control_cool2(4, OFF);
    }
}

//----------------------------------------------------------------------------------------------------------
// 获取运行模式
//----------------------------------------------------------------------------------------------------------
unsigned short get_run_mode(int type)
{
    // 将14个模式状态汇总到一个变量中
    unsigned short mode = 0;
    for (size_t i = 0; i < 14; i++)
    {
        if (type)
        {
            // 排除预热状态
            if ((i == 6) ||
                // 消除报警发送地址
                (i == 7) ||
                // 消除报警接收地址
                (i == 8) ||
                // 消除报警时间
                (i == 9) ||
                // 消除校准模式发送地址
                (i == 12) ||
                // 消除校准模式接收地址
                (i == 13))
            {
                continue;
            }
        }
        else
        {
            // 排除预热状态
            if ((i == 6) ||
                // 消除报警发送地址
                (i == 7) ||
                // 消除报警接收地址
                (i == 8) ||
                // 消除报警时间
                (i == 9))
            {
                continue;
            }
        }

        mode += (get_uval(SYSTEM_INIT + i) << i);
    }
    return mode;
}

//----------------------------------------------------------------------------------------------------------
// 设置运行模式
//----------------------------------------------------------------------------------------------------------
void set_run_mode(unsigned short mode)
{
    // 将一个变量更新到14个模式中
    for (size_t i = 0; i < 14; i++)
    {
        // 排除预热状态
        if ((i == 6) ||
            // 消除报警发送地址
            (i == 7) ||
            // 消除报警接收地址
            (i == 8) ||
            // 消除报警时间
            (i == 9) ||
            // 初始化发送地址
            (i == 11) ||
            // 初始化接收地址
            (i == 12))
        {
            continue;
        }
        // 更新模式
        set_uval(SYSTEM_INIT + i, (mode >> i) & 0x01);
    }
}

//----------------------------------------------------------------------------------------------------------
// 获取测试模式
//----------------------------------------------------------------------------------------------------------
unsigned short get_test_mode()
{
    // 将12个模式状态汇总到一个变量中
    unsigned short mode = 0;
    // 将16个模式状态汇总到一个变量中
    for (size_t i = 0; i < 12; i++)
    {
        mode += (get_uval(R_BTN_TP_MAIN + i) << i);
    }
    return mode;
}

//----------------------------------------------------------------------------------------------------------
// 设置测试模式
//----------------------------------------------------------------------------------------------------------
void set_test_mode(unsigned short mode)
{
    // 将一个变量更新到12个模式中
    for (size_t i = 0; i < 12; i++)
    {
        // 更新模式
        set_uval(R_BTN_TP_MAIN + i, (mode >> i) & 0x01);
    }
}

//----------------------------------------------------------------------------------------------------------
// 初始化启动模式
//----------------------------------------------------------------------------------------------------------
void init_start_mode()
{
}

//----------------------------------------------------------------------------------------------------------
// 初始化停止模式
//----------------------------------------------------------------------------------------------------------
void init_stop_mode()
{
}

//----------------------------------------------------------------------------------------------------------
// 初始化预热模式
//----------------------------------------------------------------------------------------------------------
void init_preheat_mode()
{
}

//----------------------------------------------------------------------------------------------------------
// 温度预警计算
//----------------------------------------------------------------------------------------------------------
void temp_update_para_data()
{
    if ((get_uval(P_TEMP_ALO) == ON))
    {
        // 高温模式-相对值
        short v_f_tp_high_val = get_val(P_AO_TP_MAIN) + get_val(P_TEMP_AH);
        // 低温模式-相对值
        short v_f_tp_low_val = get_val(P_AO_TP_MAIN) - get_val(P_TEMP_AL);

        // 设置高温报警值
        set_val(HIGH_TEMP_ALARM_VALUE, v_f_tp_high_val);
        // 设置低温报警值
        set_val(LOW_TEMP_ALARM_VALUE, v_f_tp_low_val);
    }

    if ((get_uval(P_TEMP_ALO) == OFF))
    {
        // 设置高温报警值
        set_val(HIGH_TEMP_ALARM_VALUE, get_val(P_TEMP_AH));
        // 设置低温报警值
        set_val(LOW_TEMP_ALARM_VALUE, get_val(P_TEMP_AL));
    }

    if ((get_uval(P_HUM_ALO) == ON))
    {
        // 高湿数值-相对值
        short v_f_hm_high_val = get_val(P_AO_HM) + get_val(P_HUM_AH);
        // 低湿数值-相对值
        short v_f_hm_low_val = get_val(P_AO_HM) - get_val(P_HUM_AL);

        // 设置高湿报警值
        set_val(HIGH_HUMI_ALARM_VALUE, v_f_hm_high_val);
        // 设置低湿报警值
        set_val(LOW_HUMI_ALARM_VALUE, v_f_hm_low_val);
    }

    if ((get_uval(P_HUM_ALO) == OFF))
    {
        // 设置高湿报警值
        set_val(HIGH_HUMI_ALARM_VALUE, get_val(P_HUM_AH));
        // 设置低湿报警值
        set_val(LOW_HUMI_ALARM_VALUE, get_val(P_HUM_AL));
    }

    if ((get_uval(P_RTEMP_ALO) == ON))
    {
        // 回流模式1-高温
        short v_f_retp1_high_val = get_val(P_AO_TP_RF1) + get_val(P_RTEMP_AH);
        // 回流模式1-低温
        short v_f_retp1_low_val = get_val(P_AO_TP_RF1) - get_val(P_RTEMP_AL);

        // 设置回流模式1-高温报警值
        set_val(HIGH_TEMP_ALARM_VALUE_FOR_REFLUX_TEMP_1, v_f_retp1_high_val);
        // 设置回流模式1-低温报警值
        set_val(LOW_TEMP_ALARM_VALUE_FOR_REFLUX_TEMP_1, v_f_retp1_low_val);

        // 回流模式2-高温
        short v_f_retp2_high_val = get_val(P_AO_TP_RF2) + get_val(P_RTEMP_AH);
        // 回流模式2-低温
        short v_f_retp2_low_val = get_val(P_AO_TP_RF2) - get_val(P_RTEMP_AL);

        // 设置回流模式2-高温报警值
        set_val(HIGH_TEMP_ALARM_VALUE_FOR_REFLUX_TEMP_2, v_f_retp2_high_val);
        // 设置回流模式2-低温报警值
        set_val(LOW_TEMP_ALARM_VALUE_FOR_REFLUX_TEMP_2, v_f_retp2_low_val);

        // 回流模式3-高温
        short v_f_retp3_high_val = get_val(P_AO_TP_RF3) + get_val(P_RTEMP_AH);
        // 回流模式3-低温
        short v_f_retp3_low_val = get_val(P_AO_TP_RF3) - get_val(P_RTEMP_AL);

        // 设置回流模式3-高温报警值
        set_val(HIGH_TEMP_ALARM_VALUE_FOR_REFLUX_TEMP_3, v_f_retp3_high_val);
        // 设置回流模式3-低温报警值
        set_val(LOW_TEMP_ALARM_VALUE_FOR_REFLUX_TEMP_3, v_f_retp3_low_val);

        // 回流模式4-高温
        short v_f_retp4_high_val = get_val(P_AO_TP_RF4) + get_val(P_RTEMP_AH);
        // 回流模式4-低温
        short v_f_retp4_low_val = get_val(P_AO_TP_RF4) - get_val(P_RTEMP_AL);

        // 设置回流模式4-高温报警值
        set_val(HIGH_TEMP_ALARM_VALUE_FOR_REFLUX_TEMP_4, v_f_retp4_high_val);
        // 设置回流模式3-低温报警值
        set_val(LOW_TEMP_ALARM_VALUE_FOR_REFLUX_TEMP_4, v_f_retp4_low_val);
    }

    if ((get_uval(P_RTEMP_ALO) == OFF))
    {
        // 设置回流模式1-高温报警值
        set_val(HIGH_TEMP_ALARM_VALUE_FOR_REFLUX_TEMP_1, get_val(P_RTEMP_AH));
        // 设置回流模式1-低温报警值
        set_val(LOW_TEMP_ALARM_VALUE_FOR_REFLUX_TEMP_1, get_val(P_RTEMP_AL));

        // 设置回流模式2-高温报警值
        set_val(HIGH_TEMP_ALARM_VALUE_FOR_REFLUX_TEMP_2, get_val(P_RTEMP_AH));
        // 设置回流模式2-低温报警值
        set_val(LOW_TEMP_ALARM_VALUE_FOR_REFLUX_TEMP_2, get_val(P_RTEMP_AL));

        // 设置回流模式3-高温报警值
        set_val(HIGH_TEMP_ALARM_VALUE_FOR_REFLUX_TEMP_3, get_val(P_RTEMP_AH));
        // 设置回流模式3-低温报警值
        set_val(LOW_TEMP_ALARM_VALUE_FOR_REFLUX_TEMP_3, get_val(P_RTEMP_AL));

        // 设置回流模式4-高温报警值
        set_val(HIGH_TEMP_ALARM_VALUE_FOR_REFLUX_TEMP_4, get_val(P_RTEMP_AH));
        // 设置回流模式4-低温报警值
        set_val(LOW_TEMP_ALARM_VALUE_FOR_REFLUX_TEMP_4, get_val(P_RTEMP_AL));
    }
}

//----------------------------------------------------------------------------------------------------------
// LED灯控制函数
//----------------------------------------------------------------------------------------------------------
void direction_func()
{
    zlog_debug(g_zlog_zc, "%-40s进入LED灯控制函数!", "[direction_func]");

    if (g_fan_status == 1)
    {
        // 风机
        if (((get_val(R_SYNC_MOTOR_CONTROL_PARA) == 1) || (get_val(R_SYNC_MOTOR_CONTROL_PARA) == 2)))
        {
            set_uval(OUT_ST_FAN_STATUS, ON);
        }
        else
        {
            set_uval(OUT_ST_FAN_STATUS, OFF);
        }
    }
    else
    {
        // 风机
        if (get_uval(CONTROL_FAN))
        {
            set_uval(OUT_ST_FAN_STATUS, ON);
        }
        else
        {
            set_uval(OUT_ST_FAN_STATUS, OFF);
        }
    }

    // 主加热
    if (get_uval(CONTROL_MAIN_HEAT))
    {
        set_uval(OUT_ST_TP_MAIN_STATUS, ON);
    }
    else
    {
        set_uval(OUT_ST_TP_MAIN_STATUS, OFF);
    }

    // 辅助加热
    if (get_uval(CONTROL_AUX_HEAT))
    {
        set_uval(OUT_ST_TP_AID_STATUS, ON);
    }
    else
    {
        set_uval(OUT_ST_TP_AID_STATUS, OFF);
    }

    // 加湿
    if (get_uval(CONTROL_SPRAY))
    {
        set_uval(OUT_ST_HM_STATUS, ON);
    }
    else
    {
        set_uval(OUT_ST_HM_STATUS, OFF);
    }

    // 翻蛋
    if (get_uval(DETECT_FLIP_EGG))
    {
        set_uval(OUT_ST_EGG_STATUS, ON);
    }
    else
    {
        set_uval(OUT_ST_EGG_STATUS, OFF);
    }

    // 风门开启
    if (get_uval(CONTROL_DAMPER_OPEN))
    {
        set_uval(OUT_ST_AD_OPEN_STATUS, ON);
    }
    else
    {
        set_uval(OUT_ST_AD_OPEN_STATUS, OFF);
    }

    // 风门关闭
    if (get_uval(CONTROL_DAMPER_CLOSE))
    {
        set_uval(OUT_ST_AD_CLOSE_STATUS, ON);
    }
    else
    {
        set_uval(OUT_ST_AD_CLOSE_STATUS, OFF);
    }

    // 水冷
    if (get_uval(CONTROL_COOL))
    {
        set_uval(OUT_ST_WT_CL_STATUS, ON);
    }
    else
    {
        set_uval(OUT_ST_WT_CL_STATUS, OFF);
    }

    // 辅助水冷2-1
    if (get_uval(CONTROL_COOL2_1))
    {
        set_uval(OUT_ST_TP_RF1_STATUS, ON);
    }
    else
    {
        set_uval(OUT_ST_TP_RF1_STATUS, OFF);
    }

    // 辅助水冷2-2
    if (get_uval(CONTROL_COOL2_2))
    {
        set_uval(OUT_ST_TP_RF2_STATUS, ON);
    }
    else
    {
        set_uval(OUT_ST_TP_RF2_STATUS, OFF);
    }

    // 辅助水冷2-3
    if (get_uval(CONTROL_COOL2_3))
    {
        set_uval(OUT_ST_TP_RF3_STATUS, ON);
    }
    else
    {
        set_uval(OUT_ST_TP_RF3_STATUS, OFF);
    }

    // 辅助水冷2-4
    if (get_uval(CONTROL_COOL2_4))
    {
        set_uval(OUT_ST_TP_RF4_STATUS, ON);
    }
    else
    {
        set_uval(OUT_ST_TP_RF4_STATUS, OFF);
    }
}

//----------------------------------------------------------------------------------------------------------
// 定时器
//----------------------------------------------------------------------------------------------------------
void timer_callback_func(int signo)
{
    static int ui_timer_1_minute_count = 0;
    switch (signo)
    {
    case SIGALRM:
    {
        //----------------------------------------------------------------------------------------------------------
        // 风门控制时间处理模块
        //----------------------------------------------------------------------------------------------------------
        // 获取到风门全开监测停止控制风门
        if (get_uval(DETECT_DAMPER_FULLY_OPEN) > 0)
        {
            set_uval(CONTROL_DAMPER_OPEN, OFF);
            set_uval(R_AI_AD, get_uval(DAMPER_MAX_LEN));
            set_val(TEST_DAMPER_OPEN_MODE, OFF);
            set_val(R_BTN_DR_ON, OFF);
        }
        else
        {
            // 风门开计时
            if (get_uval(CONTROL_DAMPER_OPEN) > 0)
            {
                // 清空风门关闭状态
                set_uval(AIR_DOOR_CLOSING_RUNNING_STATUS, 0);

                // 当风门开的状态为0时
                if (!get_uval(AIR_DOOR_OPEN_RUNNING_STATUS))
                {
                    // 设置风门的开时间的倒计时
                    set_uval(AIR_DOOR_OPEN_RUNNING_TIME, get_uval(DAMPER_TIME_OPEN) + get_uval(AIR_DOOR_CLOSING_RUNNING_TIME));

                    // 设置风门关时间的倒计时为0
                    set_uval(AIR_DOOR_CLOSING_RUNNING_TIME, 0);

                    // 设置风门开的状态为1时
                    set_uval(AIR_DOOR_OPEN_RUNNING_STATUS, 1);
                }

                // 当风门开的状态为1时和风门的开时间的倒计时为0时
                if ((get_uval(AIR_DOOR_OPEN_RUNNING_TIME) == 0) && (get_uval(AIR_DOOR_OPEN_RUNNING_STATUS) == 1))
                {
                    // 风门当前值小于最大值时
                    if ((get_val(R_AI_AD) < get_val(DAMPER_MAX_LEN)))
                    {
                        // 风门当前值加1
                        set_uval(R_AI_AD, get_val(R_AI_AD) + 1);

                        // 设置风门开的状态为0时
                        set_uval(AIR_DOOR_OPEN_RUNNING_STATUS, 0);
                    }
                }

                // 进行倒计时
                if (get_uval(AIR_DOOR_OPEN_RUNNING_TIME) > 0)
                {
                    set_uval(AIR_DOOR_OPEN_RUNNING_TIME, get_uval(AIR_DOOR_OPEN_RUNNING_TIME) - 1);
                }
            }
        }

        // 获取风门全关检测停止控制风门
        if (get_uval(DETECT_DAMPER_FULLY_CLOSE) > 0)
        {
            set_uval(CONTROL_DAMPER_CLOSE, OFF);
            set_uval(R_AI_AD, get_uval(DAMPER_MIN_LEN));
            set_val(TEST_DAMPER_CLOSE_MODE, OFF);
            set_val(R_BTN_DR_OFF, OFF);
        }
        else
        {
            if (get_uval(CONTROL_DAMPER_CLOSE) > 0)
            {
                // 清空风门开闭状态
                set_uval(AIR_DOOR_OPEN_RUNNING_STATUS, 0);

                // 当风门关的状态为0时
                if (!get_uval(AIR_DOOR_CLOSING_RUNNING_STATUS))
                {
                    // 设置风门的关时间的倒计时
                    set_uval(AIR_DOOR_CLOSING_RUNNING_TIME, get_uval(DAMPER_TIME_CLOSE) + get_uval(AIR_DOOR_OPEN_RUNNING_TIME));

                    // 设置风门开时间的倒计时为0
                    set_uval(AIR_DOOR_OPEN_RUNNING_TIME, 0);

                    // 设置风门关的状态为1时
                    set_uval(AIR_DOOR_CLOSING_RUNNING_STATUS, 1);
                }

                // 当风门关的状态为1时和风门的关时间的倒计时为0时
                if ((get_uval(AIR_DOOR_CLOSING_RUNNING_TIME) == 0) && (get_uval(AIR_DOOR_CLOSING_RUNNING_STATUS) == 1))
                {
                    // 当前值大于等于风门最小值时
                    if ((get_val(R_AI_AD) > get_val(DAMPER_MIN_LEN)))
                    {
                        // 风门当前值减一
                        set_val(R_AI_AD, get_val(R_AI_AD) - 1);

                        // 设置风门关的状态为0时
                        set_uval(AIR_DOOR_CLOSING_RUNNING_STATUS, 0);
                    }
                }

                // 进行倒计时
                if (get_uval(AIR_DOOR_CLOSING_RUNNING_TIME) > 0)
                {
                    set_uval(AIR_DOOR_CLOSING_RUNNING_TIME, get_uval(AIR_DOOR_CLOSING_RUNNING_TIME) - 1);
                }
            }
        }

        // zlog_debug(g_zlog_zc, "%-40s进入定时器!", "[timer_callback_func]");
        // 一秒处理
        if ((++ui_timer_1_minute_count) >= 10)
        {
            ui_timer_1_minute_count = 0;
            zlog_debug(g_zlog_zc, "%-40s进入定时器!", "[timer_callback_func]");
            //----------------------------------------------------------------------------------------------------------
            // 程序时间处理模块
            //----------------------------------------------------------------------------------------------------------
            // 孵蛋运行时间累加
            if (get_val(R_RUN_SECOND_STATUS))
            {
                set_time(R_RUN_SECOND_HIGH, get_time(R_RUN_SECOND_HIGH) + 1);
            }

            // // 系统时间
            // if (R_RUN_SECOND_STATUS)
            // {
            //     unsigned int v_t_time_sum = get_time(R_RUN_SECOND_HIGH);
            //     if ((v_t_time_count >= 120) || (v_t_time_count <= 0))
            //     {
            //         v_t_time_sum = v_t_time_sum + 60;
            //         set_time(R_RUN_SECOND_HIGH, v_t_time_sum);
            //     }
            //     else
            //     {
            //         v_t_time_sum = v_t_time_sum + v_t_time_count;
            //         set_time(R_RUN_SECOND_HIGH, v_t_time_sum);
            //     }
            // }
            // set_time(S_RUN_SECOND_HIGH, v_t_now_time);

            // 程序运行时间累加
            if (get_val(R_RUN_SYSTEM_SECOND_STATUS))
            {
                set_time(R_RUN_SYSTEM_SECOND_HIGH, get_time(R_RUN_SYSTEM_SECOND_HIGH) + 1);
            }

            //----------------------------------------------------------------------------------------------------------
            // 测试控制时间处理模块
            //----------------------------------------------------------------------------------------------------------
            if (get_uval(TEST_BTN_TIME) > 0)
            {
                set_uval(TEST_BTN_TIME, get_uval(TEST_BTN_TIME) - 1);
            }

            //----------------------------------------------------------------------------------------------------------
            // 翻蛋控制时间处理模块
            //----------------------------------------------------------------------------------------------------------
            // 翻蛋时间
            if (get_uval(P_EGG_FLIPPING_TIME) > 0)
            {
                set_uval(P_EGG_FLIPPING_TIME, get_uval(P_EGG_FLIPPING_TIME) - 1);
            }

            // 有翻蛋信号报警时间
            if (get_uval(EGG_FLIPPING_SIGNAL_ALARM_TIME) > 0)
            {
                set_uval(EGG_FLIPPING_SIGNAL_ALARM_TIME, get_uval(EGG_FLIPPING_SIGNAL_ALARM_TIME) - 1);
            }

            // 没有翻蛋信号报警时间
            if (get_uval(NO_EGG_FLIPPING_SIGNAL_ALARM_TIME) > 0)
            {
                set_uval(NO_EGG_FLIPPING_SIGNAL_ALARM_TIME, get_uval(NO_EGG_FLIPPING_SIGNAL_ALARM_TIME) - 1);
            }

            //----------------------------------------------------------------------------------------------------------
            // 风扇控制时间处理模块
            //----------------------------------------------------------------------------------------------------------
            if (get_uval(FAN_ALARM_TIME) > 0)
            {
                set_uval(FAN_ALARM_TIME, get_uval(FAN_ALARM_TIME) - 1);
            }

            //----------------------------------------------------------------------------------------------------------
            // 消警模式处理模块
            //----------------------------------------------------------------------------------------------------------
            if (get_uval(ALARM_SUPPRESSION_TIME) > 0)
            {
                set_uval(ALARM_SUPPRESSION_TIME, get_uval(ALARM_SUPPRESSION_TIME) - 1);
            }

            //----------------------------------------------------------------------------------------------------------
            // 校准模式处理模块
            //----------------------------------------------------------------------------------------------------------
            if (get_uval(CALIBRATION_RUN_TIME) > 0)
            {
                set_uval(CALIBRATION_RUN_TIME, get_uval(CALIBRATION_RUN_TIME) - 1);
            }

            //----------------------------------------------------------------------------------------------------------
            // 二氧化碳时间处理模块
            //----------------------------------------------------------------------------------------------------------
            // 二氧化碳工作时间
            if (get_uval(CARBON_DIOXIDE_OPERATING_TIME) > 0)
            {
                set_uval(CARBON_DIOXIDE_OPERATING_TIME, get_uval(CARBON_DIOXIDE_OPERATING_TIME) - 1);
            }

            // 二氧化碳停止时间
            if (get_uval(CARBON_DIOXIDE_STOP_TIME) > 0)
            {
                set_uval(CARBON_DIOXIDE_STOP_TIME, get_uval(CARBON_DIOXIDE_STOP_TIME) - 1);
            }

            //----------------------------------------------------------------------------------------------------------
            // 湿度报警模块
            //----------------------------------------------------------------------------------------------------------
            // 高湿报警延迟
            if (get_uval(HIGH_HUMIDITY_ALARM_DELAY_TIME) > 0)
            {
                set_uval(HIGH_HUMIDITY_ALARM_DELAY_TIME, get_uval(HIGH_HUMIDITY_ALARM_DELAY_TIME) - 1);
            }

            //----------------------------------------------------------------------------------------------------------
            // 回流温度时间处理模块
            //----------------------------------------------------------------------------------------------------------
            // 回流温度1
            if (get_uval(REFLUX_TEMP_1_WORK_TIME) > 0)
            {
                set_uval(REFLUX_TEMP_1_WORK_TIME, get_uval(REFLUX_TEMP_1_WORK_TIME) - 1);
            }

            if (get_uval(RETURN_TEMP_1_STOP_TIME) > 0)
            {
                set_uval(RETURN_TEMP_1_STOP_TIME, get_uval(RETURN_TEMP_1_STOP_TIME) - 1);
            }

            if (get_uval(REFLUX_TEMP_1_DELAY_TIME) > 0)
            {
                set_uval(REFLUX_TEMP_1_DELAY_TIME, get_uval(REFLUX_TEMP_1_DELAY_TIME) - 1);
            }

            // 回流温度2
            if (get_uval(REFLUX_TEMP_2_WORK_TIME) > 0)
            {
                set_uval(REFLUX_TEMP_2_WORK_TIME, get_uval(REFLUX_TEMP_2_WORK_TIME) - 1);
            }

            if (get_uval(RETURN_TEMP_2_STOP_TIME) > 0)
            {
                set_uval(RETURN_TEMP_2_STOP_TIME, get_uval(RETURN_TEMP_2_STOP_TIME) - 1);
            }

            if (get_uval(REFLUX_TEMP_2_DELAY_TIME) > 0)
            {
                set_uval(REFLUX_TEMP_2_DELAY_TIME, get_uval(REFLUX_TEMP_2_DELAY_TIME) - 1);
            }

            // 回流温度3
            if (get_uval(REFLUX_TEMP_3_WORK_TIME) > 0)
            {
                set_uval(REFLUX_TEMP_3_WORK_TIME, get_uval(REFLUX_TEMP_3_WORK_TIME) - 1);
            }

            if (get_uval(RETURN_TEMP_3_STOP_TIME) > 0)
            {
                set_uval(RETURN_TEMP_3_STOP_TIME, get_uval(RETURN_TEMP_3_STOP_TIME) - 1);
            }

            if (get_uval(REFLUX_TEMP_3_DELAY_TIME) > 0)
            {
                set_uval(REFLUX_TEMP_3_DELAY_TIME, get_uval(REFLUX_TEMP_3_DELAY_TIME) - 1);
            }

            // 回流温度4
            if (get_uval(REFLUX_TEMP_4_WORK_TIME) > 0)
            {
                set_uval(REFLUX_TEMP_4_WORK_TIME, get_uval(REFLUX_TEMP_4_WORK_TIME) - 1);
            }

            if (get_uval(RETURN_TEMP_4_STOP_TIME) > 0)
            {
                set_uval(RETURN_TEMP_4_STOP_TIME, get_uval(RETURN_TEMP_4_STOP_TIME) - 1);
            }

            if (get_uval(REFLUX_TEMP_4_DELAY_TIME) > 0)
            {
                set_uval(REFLUX_TEMP_4_DELAY_TIME, get_uval(REFLUX_TEMP_4_DELAY_TIME) - 1);
            }

            // 回流温度高温报警时间
            if (get_uval(RETURN_TEMP_HIGH_TEMP_ALARM_TIME) > 0)
            {
                set_uval(RETURN_TEMP_HIGH_TEMP_ALARM_TIME, get_uval(RETURN_TEMP_HIGH_TEMP_ALARM_TIME) - 1);
            }

            // 回流温度低温报警时间
            if (get_uval(RETURN_TEMP_LOW_TEMP_ALARM_TIME) > 0)
            {
                set_uval(RETURN_TEMP_LOW_TEMP_ALARM_TIME, get_uval(RETURN_TEMP_LOW_TEMP_ALARM_TIME) - 1);
            }

            //----------------------------------------------------------------------------------------------------------
            // PID处理时间
            //----------------------------------------------------------------------------------------------------------
            // 温度周期时间
            // if (get_time(PID_PERIOD_COUNT_MAIN_HEATER) > 0)
            // {
            //     set_time(PID_PERIOD_COUNT_MAIN_HEATER, get_time(PID_PERIOD_COUNT_MAIN_HEATER) - 1);
            // }

            // 主加热时间
            if (get_uval(PID_ON_COUNT_MAIN_HEATER) > 0)
            {
                set_uval(PID_ON_COUNT_MAIN_HEATER, get_uval(PID_ON_COUNT_MAIN_HEATER) - 1);
            }

            // 水冷时间
            if (get_uval(PID_ON_COUNT_COOL) > 0)
            {
                set_uval(PID_ON_COUNT_COOL, get_uval(PID_ON_COUNT_COOL) - 1);
            }

            // 鼓风时间
            if (get_uval(PID_ON_COUNT_BLOWER) > 0)
            {
                set_uval(PID_ON_COUNT_BLOWER, get_uval(PID_ON_COUNT_BLOWER) - 1);
            }

            // 风门开时间
            if (get_uval(PID_ON_COUNT_DB_OPEN) > 0)
            {
                set_uval(PID_ON_COUNT_DB_OPEN, get_uval(PID_ON_COUNT_DB_OPEN) - 1);
            }

            // 风门关时间
            if (get_uval(PID_ON_COUNT_DB_CLOSE) > 0)
            {
                set_uval(PID_ON_COUNT_DB_CLOSE, get_uval(PID_ON_COUNT_DB_CLOSE) - 1);
            }

            // 湿度周期时间
            if (get_time(PID_PERIOD_COUNT_HUMI) > 0)
            {
                set_time(PID_PERIOD_COUNT_HUMI, get_time(PID_PERIOD_COUNT_HUMI) - 1);
            }

            // 加湿时间
            // if (get_time(PID_ON_COUNT_HUMI) > 0)
            // {
            //     set_time(PID_ON_COUNT_HUMI, get_time(PID_ON_COUNT_HUMI) - 1);
            // }

            //----------------------------------------------------------------------------------------------------------
            // 风门校准时间
            //----------------------------------------------------------------------------------------------------------
            // 风门从关到开
            // if (get_val(CONTROL_DB_CLOSE_TO_OPEN_TIME_STATUS) > 0)
            // {
            //     if (get_uval(CONTROL_DB_CLOSE_TO_OPEN_TIME) < 0xffff)
            //     {
            //         set_uval(CONTROL_DB_CLOSE_TO_OPEN_TIME, get_uval(CONTROL_DB_CLOSE_TO_OPEN_TIME) + 1);
            //     }
            // }

            // // 风门从开到关
            // if (get_val(CONTROL_DB_OPEN_TO_CLOSE_TIME_STATUS) > 0)
            // {
            //     if (get_uval(CONTROL_DB_OPEN_TO_CLOSE_TIME) < 0xffff)
            //     {
            //         set_uval(CONTROL_DB_OPEN_TO_CLOSE_TIME, get_uval(CONTROL_DB_OPEN_TO_CLOSE_TIME) + 1);
            //     }
            // }
        }
    }
    break;
    }
}

//----------------------------------------------------------------------------------------------------------
// 线程
//----------------------------------------------------------------------------------------------------------
// 线程 同步线程
//----------------------------------------------------------------------------------------------------------
void *sysn_thread_func(void *pv)
{

    zlog_debug(g_zlog_zc, "%-40s进入同步线程!", "[sysn_thread_func]");
    // 旧PT100参数
    unsigned short old_pt100_para[100] = {0};
    // 当前备份文件
    char sync_backup_file[512] = "";
    // 当前备份文件序号
    int sync_backup_index = 1;
    // 备份数据起始地址
    int sync_data_start = 0;
    // 备份数据长度
    int sync_data_len = 5000;

    // 赋值给旧PT100参数
    memcpy(old_pt100_para, &shm_out[P_PT1_AO1], sizeof(short) * 30);

    while (1)
    {
        if (memcmp(old_pt100_para, &shm_out[P_PT1_AO1], sizeof(short) * 30) != 0)
        {
            // INI配置文件
            xini_file_t st_filenaem_ini(g_filenaem_ini);

            //----------------------------------------------------------------------------------------------------------
            // PT100 检测参数
            //----------------------------------------------------------------------------------------------------------
            // PT100-1 第一个温度
            if (get_uval(P_PT1_AO1) != g_st_jincubator.pt1.pt_0_temp)
            {
                st_filenaem_ini["GENERAL SETTING"]["PT1_0_TEMP"] = get_uval(P_PT1_AO1);
                g_st_jincubator.pt1.pt_0_temp = get_uval(P_PT1_AO1);
            }

            // PT100-1 第二个温度
            if ((get_uval(P_PT1_AO2) != 0) && (get_uval(P_PT1_AO2) != g_st_jincubator.pt1.pt_40_temp))
            {
                st_filenaem_ini["GENERAL SETTING"]["PT1_40_TEMP"] = get_uval(P_PT1_AO2);
                g_st_jincubator.pt1.pt_40_temp = get_uval(P_PT1_AO2);
            }

            // PT100-1 第一个通道值
            if ((get_uval(P_PT1_AO3) != 0) && (get_uval(P_PT1_AO3) != g_st_jincubator.pt1.pt_0_channel))
            {
                st_filenaem_ini["GENERAL SETTING"]["PT1_0_CHANNEL"] = get_uval(P_PT1_AO3);
                g_st_jincubator.pt1.pt_0_channel = get_uval(P_PT1_AO3);
            }

            // PT100-1 第二个通道值
            if ((get_uval(P_PT1_AO4) != 0) && (get_uval(P_PT1_AO4) != g_st_jincubator.pt1.pt_40_channel))
            {
                st_filenaem_ini["GENERAL SETTING"]["PT1_40_CHANNEL"] = get_uval(P_PT1_AO4);
                g_st_jincubator.pt1.pt_40_channel = get_uval(P_PT1_AO4);
            }

            // PT100-2 第一个温度
            if (get_uval(P_PT2_AO1) != g_st_jincubator.pt2.pt_0_temp)
            {
                st_filenaem_ini["GENERAL SETTING"]["PT2_0_TEMP"] = get_uval(P_PT2_AO1);
                g_st_jincubator.pt2.pt_0_temp = get_uval(P_PT2_AO1);
            }

            // PT100-2 第二个温度
            if ((get_uval(P_PT2_AO2) != 0) && (get_uval(P_PT2_AO2) != g_st_jincubator.pt2.pt_40_temp))
            {
                st_filenaem_ini["GENERAL SETTING"]["PT2_40_TEMP"] = get_uval(P_PT2_AO2);
                g_st_jincubator.pt2.pt_40_temp = get_uval(P_PT2_AO2);
            }

            // PT100-2 第一个通道值
            if ((get_uval(P_PT2_AO3) != 0) && (get_uval(P_PT2_AO3) != g_st_jincubator.pt2.pt_0_channel))
            {
                st_filenaem_ini["GENERAL SETTING"]["PT2_0_CHANNEL"] = get_uval(P_PT2_AO3);
                g_st_jincubator.pt2.pt_0_channel = get_uval(P_PT2_AO3);
            }

            // PT100-2 第二个通道值
            if ((get_uval(P_PT2_AO4) != 0) && (get_uval(P_PT2_AO4) != g_st_jincubator.pt2.pt_40_channel))
            {
                st_filenaem_ini["GENERAL SETTING"]["PT2_40_CHANNEL"] = get_uval(P_PT2_AO4);
                g_st_jincubator.pt2.pt_40_channel = get_uval(P_PT2_AO4);
            }

            // PT100-3 第一个温度
            if (get_uval(P_PT3_AO1) != g_st_jincubator.pt3.pt_0_temp)
            {
                st_filenaem_ini["GENERAL SETTING"]["PT3_0_TEMP"] = get_uval(P_PT3_AO1);
                g_st_jincubator.pt3.pt_0_temp = get_uval(P_PT3_AO1);
            }

            // PT100-3 第二个温度
            if ((get_uval(P_PT3_AO2) != 0) && (get_uval(P_PT3_AO2) != g_st_jincubator.pt3.pt_40_temp))
            {
                st_filenaem_ini["GENERAL SETTING"]["PT3_40_TEMP"] = get_uval(P_PT3_AO2);
                g_st_jincubator.pt3.pt_40_temp = get_uval(P_PT3_AO2);
            }

            // PT100-3 第一个通道值
            if ((get_uval(P_PT3_AO3) != 0) && (get_uval(P_PT3_AO3) != g_st_jincubator.pt3.pt_0_channel))
            {
                st_filenaem_ini["GENERAL SETTING"]["PT3_0_CHANNEL"] = get_uval(P_PT3_AO3);
                g_st_jincubator.pt3.pt_0_channel = get_uval(P_PT3_AO3);
            }

            // PT100-3 第二个通道值
            if ((get_uval(P_PT3_AO4) != 0) && (get_uval(P_PT3_AO4) != g_st_jincubator.pt3.pt_40_channel))
            {
                st_filenaem_ini["GENERAL SETTING"]["PT3_40_CHANNEL"] = get_uval(P_PT3_AO4);
                g_st_jincubator.pt3.pt_40_channel = get_uval(P_PT3_AO4);
            }

            // PT100-4 第一个温度
            if (get_uval(P_PT4_AO1) != g_st_jincubator.pt4.pt_0_temp)
            {
                st_filenaem_ini["GENERAL SETTING"]["PT4_0_TEMP"] = get_uval(P_PT4_AO1);
                g_st_jincubator.pt4.pt_0_temp = get_uval(P_PT4_AO1);
            }

            // PT100-4 第二个温度
            if ((get_uval(P_PT4_AO2) != 0) && (get_uval(P_PT4_AO2) != g_st_jincubator.pt4.pt_40_temp))
            {
                st_filenaem_ini["GENERAL SETTING"]["PT4_40_TEMP"] = get_uval(P_PT4_AO2);
                g_st_jincubator.pt4.pt_40_temp = get_uval(P_PT4_AO2);
            }

            // PT100-4 第一个通道值
            if ((get_uval(P_PT4_AO3) != 0) && (get_uval(P_PT4_AO3) != g_st_jincubator.pt4.pt_0_channel))
            {
                st_filenaem_ini["GENERAL SETTING"]["PT4_0_CHANNEL"] = get_uval(P_PT4_AO3);
                g_st_jincubator.pt4.pt_0_channel = get_uval(P_PT4_AO3);
            }

            // PT100-4 第二个通道值
            if ((get_uval(P_PT4_AO4) != 0) && (get_uval(P_PT4_AO4) != g_st_jincubator.pt4.pt_40_channel))
            {
                st_filenaem_ini["GENERAL SETTING"]["PT4_40_CHANNEL"] = get_uval(P_PT4_AO4);
                g_st_jincubator.pt4.pt_40_channel = get_uval(P_PT4_AO4);
            }

            // PT100-5 第一个温度
            if (get_uval(P_PT5_AO1) != g_st_jincubator.pt5.pt_0_temp)
            {
                st_filenaem_ini["GENERAL SETTING"]["PT5_0_TEMP"] = get_uval(P_PT5_AO1);
                g_st_jincubator.pt5.pt_0_temp = get_uval(P_PT5_AO1);
            }

            // PT100-5 第二个温度
            if ((get_uval(P_PT5_AO2) != 0) && (get_uval(P_PT5_AO2) != g_st_jincubator.pt5.pt_40_temp))
            {
                st_filenaem_ini["GENERAL SETTING"]["PT5_40_TEMP"] = get_uval(P_PT5_AO2);
                g_st_jincubator.pt5.pt_40_temp = get_uval(P_PT5_AO2);
            }

            // PT100-5 第一个通道值
            if ((get_uval(P_PT5_AO3) != 0) && (get_uval(P_PT5_AO3) != g_st_jincubator.pt5.pt_0_channel))
            {
                st_filenaem_ini["GENERAL SETTING"]["PT5_0_CHANNEL"] = get_uval(P_PT5_AO3);
                g_st_jincubator.pt5.pt_0_channel = get_uval(P_PT5_AO3);
            }

            // PT100-5 第二个通道值
            if ((get_uval(P_PT5_AO4) != 0) && (get_uval(P_PT5_AO4) != g_st_jincubator.pt5.pt_40_channel))
            {
                st_filenaem_ini["GENERAL SETTING"]["PT5_40_CHANNEL"] = get_uval(P_PT5_AO4);
                g_st_jincubator.pt5.pt_40_channel = get_uval(P_PT5_AO4);
            }

            // PT100-6 第一个温度
            if (get_uval(P_PT6_AO1) != g_st_jincubator.pt6.pt_0_temp)
            {
                st_filenaem_ini["GENERAL SETTING"]["PT6_0_TEMP"] = get_uval(P_PT6_AO1);
                g_st_jincubator.pt6.pt_0_temp = get_uval(P_PT6_AO1);
            }

            // PT100-6 第二个温度
            if ((get_uval(P_PT6_AO2) != 0) && (get_uval(P_PT6_AO2) != g_st_jincubator.pt6.pt_40_temp))
            {
                st_filenaem_ini["GENERAL SETTING"]["PT6_40_TEMP"] = get_uval(P_PT6_AO2);
                g_st_jincubator.pt6.pt_40_temp = get_uval(P_PT6_AO2);
            }

            // PT100-6 第一个通道值
            if ((get_uval(P_PT6_AO3) != 0) && (get_uval(P_PT6_AO3) != g_st_jincubator.pt6.pt_0_channel))
            {
                st_filenaem_ini["GENERAL SETTING"]["PT6_0_CHANNEL"] = get_uval(P_PT6_AO3);
                g_st_jincubator.pt6.pt_0_channel = get_uval(P_PT6_AO3);
            }

            // PT100-6 第二个通道值
            if ((get_uval(P_PT6_AO4) != 0) && (get_uval(P_PT6_AO4) != g_st_jincubator.pt6.pt_40_channel))
            {
                st_filenaem_ini["GENERAL SETTING"]["PT6_40_CHANNEL"] = get_uval(P_PT6_AO4);
                g_st_jincubator.pt6.pt_40_channel = get_uval(P_PT6_AO4);
            }

            memcpy(old_pt100_para, &shm_out[P_PT1_AO1], sizeof(short) * 30);
        }

        snprintf(sync_backup_file, 512, "%s_1", g_st_jincubator.conf_sysc_file);
        int i_ret = syn_shm_to_ini(sync_backup_file, sync_data_start, sync_data_len);
        zlog_debug(g_zlog_zc, "%-40s文件名称: %s 数据起始地址：%d 数据长度：%d 备份状态: %s", "[sysn_thread_func]", sync_backup_file, sync_data_start, sync_data_len, (!i_ret) ? "备份成功" : "备份失败");
        snprintf(sync_backup_file, 512, "%s_2", g_st_jincubator.conf_sysc_file);
        i_ret = syn_shm_to_ini(sync_backup_file, sync_data_start, sync_data_len);
        zlog_debug(g_zlog_zc, "%-40s文件名称: %s 数据起始地址：%d 数据长度：%d 备份状态: %s", "[sysn_thread_func]", sync_backup_file, sync_data_start, sync_data_len, (!i_ret) ? "备份成功" : "备份失败");
        snprintf(sync_backup_file, 512, "%s_3", g_st_jincubator.conf_sysc_file);
        i_ret = syn_shm_to_ini(sync_backup_file, sync_data_start, sync_data_len);
        zlog_debug(g_zlog_zc, "%-40s文件名称: %s 数据起始地址：%d 数据长度：%d 备份状态: %s", "[sysn_thread_func]", sync_backup_file, sync_data_start, sync_data_len, (!i_ret) ? "备份成功" : "备份失败");
        snprintf(sync_backup_file, 512, "%s_4", g_st_jincubator.conf_sysc_file);
        i_ret = syn_shm_to_ini(sync_backup_file, sync_data_start, sync_data_len);
        zlog_debug(g_zlog_zc, "%-40s文件名称: %s 数据起始地址：%d 数据长度：%d 备份状态: %s", "[sysn_thread_func]", sync_backup_file, sync_data_start, sync_data_len, (!i_ret) ? "备份成功" : "备份失败");
        snprintf(sync_backup_file, 512, "%s_5", g_st_jincubator.conf_sysc_file);
        i_ret = syn_shm_to_ini(sync_backup_file, sync_data_start, sync_data_len);
        zlog_debug(g_zlog_zc, "%-40s文件名称: %s 数据起始地址：%d 数据长度：%d 备份状态: %s", "[sysn_thread_func]", sync_backup_file, sync_data_start, sync_data_len, (!i_ret) ? "备份成功" : "备份失败");
        snprintf(sync_backup_file, 512, "%s_6", g_st_jincubator.conf_sysc_file);
        i_ret = syn_shm_to_ini(sync_backup_file, sync_data_start, sync_data_len);
        zlog_debug(g_zlog_zc, "%-40s文件名称: %s 数据起始地址：%d 数据长度：%d 备份状态: %s", "[sysn_thread_func]", sync_backup_file, sync_data_start, sync_data_len, (!i_ret) ? "备份成功" : "备份失败");
        snprintf(sync_backup_file, 512, "%s_7", g_st_jincubator.conf_sysc_file);
        i_ret = syn_shm_to_ini(sync_backup_file, sync_data_start, sync_data_len);
        zlog_debug(g_zlog_zc, "%-40s文件名称: %s 数据起始地址：%d 数据长度：%d 备份状态: %s", "[sysn_thread_func]", sync_backup_file, sync_data_start, sync_data_len, (!i_ret) ? "备份成功" : "备份失败");
        snprintf(sync_backup_file, 512, "%s_8", g_st_jincubator.conf_sysc_file);
        i_ret = syn_shm_to_ini(sync_backup_file, sync_data_start, sync_data_len);
        zlog_debug(g_zlog_zc, "%-40s文件名称: %s 数据起始地址：%d 数据长度：%d 备份状态: %s", "[sysn_thread_func]", sync_backup_file, sync_data_start, sync_data_len, (!i_ret) ? "备份成功" : "备份失败");
        snprintf(sync_backup_file, 512, "%s_9", g_st_jincubator.conf_sysc_file);
        i_ret = syn_shm_to_ini(sync_backup_file, sync_data_start, sync_data_len);
        zlog_debug(g_zlog_zc, "%-40s文件名称: %s 数据起始地址：%d 数据长度：%d 备份状态: %s", "[sysn_thread_func]", sync_backup_file, sync_data_start, sync_data_len, (!i_ret) ? "备份成功" : "备份失败");
        snprintf(sync_backup_file, 512, "%s_10", g_st_jincubator.conf_sysc_file);
        i_ret = syn_shm_to_ini(sync_backup_file, sync_data_start, sync_data_len);
        zlog_debug(g_zlog_zc, "%-40s文件名称: %s 数据起始地址：%d 数据长度：%d 备份状态: %s", "[sysn_thread_func]", sync_backup_file, sync_data_start, sync_data_len, (!i_ret) ? "备份成功" : "备份失败");

        // 间隔1秒
        wait_ms(5000);
    }
}

//----------------------------------------------------------------------------------------------------------
// 数据采集线程-PT100
//----------------------------------------------------------------------------------------------------------
void *data_collection_pt100_func(void *pv)
{
    zlog_debug(g_zlog_zc, "%-40s数据采集线程-PT100启动", "[data_collection_pt100_func]");
    // 滑动窗口长度
    int filter_len = 15;
    int run_ai_tp_len[6];
    unsigned short run_ai_tp_oid[6];
    double run_ai_tp_data[6][filter_len];
    double run_ai_tp_sum = 0.0;
    while (1)
    {
        // 获取六个个PT100的通道值
        for (int i = 0; i < 6; i++)
        {
            // 第一个温度
            float run_ao_r_low = get_fval(P_PT1_AO1 + (i * 5));

            // 第二个温度
            float run_ao_r_high = get_fval(P_PT1_AO1 + (i * 5) + 1);

            // 第一个通道值
            unsigned short run_ao_c_low = get_uval(P_PT1_AO1 + (i * 5) + 2);

            // 第二个通道值
            unsigned short run_ao_c_high = get_uval(P_PT1_AO1 + (i * 5) + 3);

            // 差值
            float run_ao_offiset = get_uval(P_PT1_AO1 + (i * 5) + 4);

            // 当前通道值
            unsigned short run_ai_c_tp = get_uval(PT_AI1 + i);

            if (run_ai_tp_len[i] == filter_len)
            {
                int v_abs = abs(run_ai_c_tp - run_ai_tp_oid[i]);
                if (v_abs > get_uval(P_PT_PIC))
                {
                    if (v_abs < 0)
                    {
                        run_ai_c_tp += get_uval(P_PT_PIC);
                    }
                    else
                    {
                        run_ai_c_tp -= get_uval(P_PT_PIC);
                    }
                    run_ai_tp_oid[i] = run_ai_c_tp;
                }
                else
                {
                    run_ai_c_tp = run_ai_tp_oid[i];
                }
            }
            else
            {
                run_ai_tp_oid[i] = run_ai_c_tp;
            }

            // 计算初始值
            double run_ai_tp_val = (run_ai_c_tp - run_ao_c_low) * (run_ao_r_high - run_ao_r_low) / (run_ao_c_high - run_ao_c_low) + run_ao_r_low + run_ao_offiset;
            if (run_ai_tp_len[i] < filter_len)
            {
                // 当滑动窗口数量小于15进行填充数据
                run_ai_tp_data[i][run_ai_tp_len[i]] = run_ai_tp_val;
                // 并累加数据数量
                ++run_ai_tp_len[i];
            }
            else
            {
                double tmp_ai_tp_data[filter_len];
                memset(tmp_ai_tp_data, 0, sizeof(tmp_ai_tp_data));
                // 将滑动窗口的第一个剔除，
                memcpy(&tmp_ai_tp_data, &run_ai_tp_data[i][1], sizeof(double) * (filter_len - 1));
                // 将最新的数据保存到最后的位置
                tmp_ai_tp_data[filter_len - 1] = run_ai_tp_val;
                // 将整理后的数据从新保存到数组中
                memcpy(&run_ai_tp_data[i][0], &tmp_ai_tp_data, sizeof(double) * (filter_len));
            }

            if (run_ai_tp_len[i] == filter_len)
            {
                // 初始化实际数据的序号
                unsigned short init_index = 0;
                // 实际数据的序号
                unsigned short index = 0;
                // 修正值的序号
                unsigned short shm_index = 0;
                // 当前值
                double run_ai_tp_aver = 0.0;
                // 过滤浮点数
                char d_tp_data[256] = "";

                for (int j = 0; j < filter_len; j++)
                {
                    run_ai_tp_sum += run_ai_tp_data[i][j];
                }

                if (i > 1)
                {
                    init_index = R_INIT_AI_TP_MAIN + i + 2;
                    index = R_AI_TP_MAIN + i + 2;
                    shm_index = P_EDIT_TP_MAIN + i + 2;
                }
                else
                {
                    init_index = R_INIT_AI_TP_MAIN + i;
                    index = R_AI_TP_MAIN + i;
                    shm_index = P_EDIT_TP_MAIN + i;
                }

                // 使用滑动窗口获取通道平均值
                run_ai_tp_aver = run_ai_tp_sum / filter_len;

                // 设置浮点精度
                sprintf(d_tp_data, "%0.2f", run_ai_tp_aver);

                // 原始值
                float f_tp_data_init = atof(d_tp_data);
                int run_ai_i_tp_init = f_tp_data_init * 100;

                // 传输实际值
                set_uval(init_index, run_ai_i_tp_init);

                // 校准模式标志位
                float f_tp_data = atof(d_tp_data) + get_fval(shm_index);

                // 将浮点数转为无符号整数数据
                int run_ai_i_tp = f_tp_data * 100;

                // 过滤异常数据
                unsigned short run_ao_c_shm = ((run_ai_i_tp > 32767) || (run_ai_i_tp < -32767)) ? 0 : run_ai_i_tp;
                // zlog_debug(g_zlog_zc,
                //            "%-40s通道值 [%d] Val-Offset [%0.4f] Val-Divi [%0.4f] Val-slope [%u]  Ref-Std0 [%u] 当前通道值 [%u] 当前温度 [%0.4f] 当前差值[%d] 最终温度 [%0.4f] 最终温度 [%d]",
                //            "[data_collection_pt100_func]", i + 1, run_ao_r_low, run_ao_r_high, run_ao_c_low,
                //            run_ao_c_high, run_ai_c_tp,
                //            atof(d_tp_data), get_val(shm_index), f_tp_data, run_ao_c_shm);

                // 无符号温度大于50摄氏度时，触发报警
                if (run_ao_c_shm > 0x1388)
                {
                    switch (i)
                    {
                    case 0:
                    {
                        // 设置报警变量为ON
                        set_uval(SENSOR_1_ABNORMAL_ALARM, ON);
                        set_val(P_EDIT_TP_MAIN, 0);
                        // set_val(CONTROL_MAIN_HEAT, OFF);
                        // set_val(TEST_MAIN_HEAT_MODE, OFF);
                        // set_val(R_BTN_TP_MAIN, OFF);

                        break;
                    }
                    case 1:
                    {
                        // 设置报警变量为ON
                        set_uval(SENSOR_2_ABNORMAL_ALARM, ON);
                        set_val(P_EDIT_HM, 0);
                        // set_val(R_BTN_HM, OFF);
                        // set_val(CONTROL_SPRAY, OFF);
                        // set_val(TEST_SPRAY_MODE, OFF);
                        break;
                    }
                    case 2:
                    {
                        if (get_uval(REFLUX_TEMPERATURE) & 0x01)
                        {
                            // 设置报警变量为ON
                            set_uval(SENSOR_3_ABNORMAL_ALARM, ON);
                        }
                        else
                        {
                            // 设置报警变量为OFF
                            set_uval(SENSOR_3_ABNORMAL_ALARM, OFF);
                        }
                        set_val(P_EDIT_TP_RF1, 0);
                        // set_val(R_BTN_WC_RF1, OFF);
                        // set_val(CONTROL_COOL2_1, OFF);
                        // set_val(TEST_COOL2_1_MODE, OFF);
                        break;
                    }
                    case 3:
                    {
                        if (get_uval(REFLUX_TEMPERATURE) & 0x02)
                        {
                            // 设置报警变量为ON
                            set_uval(SENSOR_4_ABNORMAL_ALARM, ON);
                        }
                        else
                        {
                            // 设置报警变量为OFF
                            set_uval(SENSOR_4_ABNORMAL_ALARM, OFF);
                        }
                        set_val(P_EDIT_TP_RF2, 0);
                        // set_val(R_BTN_WC_RF2, OFF);
                        // set_val(CONTROL_COOL2_2, OFF);
                        // set_val(TEST_COOL2_2_MODE, OFF);
                        break;
                    }
                    case 4:
                    {
                        if (get_uval(REFLUX_TEMPERATURE) & 0x04)
                        {
                            // 设置报警变量为ON
                            set_uval(SENSOR_5_ABNORMAL_ALARM, ON);
                        }
                        else
                        {
                            // 设置报警变量为OFF
                            set_uval(SENSOR_5_ABNORMAL_ALARM, OFF);
                        }
                        set_val(P_EDIT_TP_RF3, 0);
                        // set_val(R_BTN_WC_RF3, OFF);
                        // set_val(CONTROL_COOL2_3, OFF);
                        // set_val(TEST_COOL2_3_MODE, OFF);
                        break;
                    }
                    case 5:
                    {
                        if (get_uval(REFLUX_TEMPERATURE) & 0x08)
                        {
                            // 设置报警变量为OFF
                            set_uval(SENSOR_6_ABNORMAL_ALARM, ON);
                        }
                        else
                        {
                            // 设置报警变量为OFF
                            set_uval(SENSOR_6_ABNORMAL_ALARM, OFF);
                        }
                        set_val(P_EDIT_TP_RF4, 0);
                        // set_val(R_BTN_WC_RF4, OFF);
                        // set_val(CONTROL_COOL2_4, OFF);
                        // set_val(TEST_COOL2_4_MODE, OFF);
                        break;
                    }
                    }

                    // 将实时数据设置为-1
                    set_uval(index, 0xffff);
                }
                else
                {
                    // 设置报警变量为OFF
                    switch (i)
                    {
                    case 0:
                    {
                        // 设置报警变量为OFF
                        set_uval(SENSOR_1_ABNORMAL_ALARM, OFF);
                        break;
                    }
                    case 1:
                    {
                        // 设置报警变量为OFF
                        set_uval(SENSOR_2_ABNORMAL_ALARM, OFF);
                        break;
                    }
                    case 2:
                    {
                        // 设置报警变量为OFF
                        set_uval(SENSOR_3_ABNORMAL_ALARM, OFF);
                        break;
                    }
                    case 3:
                    {
                        // 设置报警变量为OFF
                        set_uval(SENSOR_4_ABNORMAL_ALARM, OFF);
                        break;
                    }
                    case 4:
                    {
                        // 设置报警变量为OFF
                        set_uval(SENSOR_5_ABNORMAL_ALARM, OFF);
                        break;
                    }
                    case 5:
                    {
                        // 设置报警变量为OFF
                        set_uval(SENSOR_6_ABNORMAL_ALARM, OFF);
                        break;
                    }
                    }

                    // 传输实际值
                    set_uval(index, run_ao_c_shm);
                }

                run_ai_tp_sum = 0;
            }
        }
        //  数据采集时间
        wait_ms(get_uval(P_PT_WAIT));
    }
}

//----------------------------------------------------------------------------------------------------------
// 数据采集线程-CO2
//----------------------------------------------------------------------------------------------------------
void *data_collection_co2_func(void *pv)
{
    zlog_debug(g_zlog_zc, "%-40sCO2采集线程启动", "[data_collection_co2_func]");
    unsigned char b_cmd_check[] = {0xff, 0x86, 0x02};
    unsigned char b_cmd_sendbuf[1024] = {0xff, 0x01, 0x86, 0x00, 0x00, 0x00, 0x00, 0x00, 0x79};
    unsigned char b_cmd_sendcheck[1024] = {0xff, 0x01, 0x87, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf4};
    unsigned char b_cmd_recvbuf[1024];

    unsigned short v_us_co2_curr_val = 0;
    unsigned short v_us_co2_old_val = 0;

    // 打开串口
    int i_bandrate = 9600;
    int i_parity = 0;
    int i_databits = 8;
    int i_stopbit = 1;

    int i_cmd_wait_time_out = 200;
    int i_cmd_read_time_out = 50;

    int i_recv_len = 0;
    int i_data_count = 0;
    int i_data_value = 0;

    unsigned short i_recv_data = 0;
    char *c_seral_name = g_st_jincubator.co2_serial_device;
    zlog_info(g_zlog_zc, "%-40s正在打开串口!", "[data_collection_co2_func]");
    int i_serial_fd = SerialOpen(c_seral_name);
    if (i_serial_fd < 0)
    {
        zlog_error(g_zlog_zc, "%-40s串口打开失败", "[data_collection_co2_func]");
        pthread_exit(NULL);
    }
    zlog_info(g_zlog_zc, "%-40s串口打开成功!", "[data_collection_co2_func]");
    // 设置波特率
    SerialSetSpeed(i_serial_fd, i_bandrate);

    // 设置校验、数据位、停止位
    SerialSetParam(i_serial_fd, i_parity, i_databits, i_stopbit);
    zlog_info(g_zlog_zc, "%-40s[%s] bandrate=%d parity=%d databits=%d stopbit=%d  open success!!", "[data_collection_co2_func]", c_seral_name, i_bandrate, i_parity, i_databits, i_stopbit);

    while (ON)
    {
        // b_cmd_sendbuf[8] = co2_check(&b_cmd_sendbuf[1], 7);
        // 发送数据
        SerialFlush(i_serial_fd);
        SerialWrite(i_serial_fd, b_cmd_sendbuf, 9);
        // zlog_debug(g_zlog_zc, "%-40s[%s] 发送读取数据[9]:", "[data_collection_co2_func]", c_seral_name);
        // hzlog_debug(g_zlog_zc, b_cmd_sendbuf, 9);
        // 接收数据
        wait_ms(1000);
        i_recv_len = SerialRead(i_serial_fd, b_cmd_recvbuf, 1024, i_cmd_wait_time_out, i_cmd_read_time_out);
        if (i_recv_len > 0)
        {
            // zlog_debug(g_zlog_zc, "%-40s[%s] 接收读取数据[%d]:", "[data_collection_co2_func]", c_seral_name, i_recv_len);
            // hzlog_debug(g_zlog_zc, b_cmd_recvbuf, i_recv_len);
            if (i_recv_len == 9)
            {
                i_data_count = 0;
                memcpy(&i_recv_data, &b_cmd_recvbuf[2], 2);
                // 校准模式标志位
                i_data_value = i_recv_data;
                ba((char *)&i_data_value, 2);

                if (get_val(CARBON_DIOXIDE_CALIBRATION_STATUS))
                {
                    short co2_difference = i_data_value - 400;
                    co2_difference = (co2_difference >= 0) ? co2_difference : 0;
                    set_val(P_EDIT_CO2_VALUE, co2_difference);
                    set_val(CARBON_DIOXIDE_CALIBRATION_STATUS, 0);
                }

                int i_out_data_value = i_data_value - get_val(P_EDIT_CO2_VALUE);

                if (i_data_value >= 0 && i_data_value <= 20000)
                {
                    // 传感器正常
                    set_val(SENSOR_CO2_ALARM, OFF);
                    // 将数据传输到实时位置
                    set_val(R_AI_CO, i_out_data_value);
                    // 将数据传输到实时位置
                    set_val(R_INIT_AI_CO, i_data_value);
                }
                else
                {
                    i_data_count = 10;
                }

                // printf("%d\n", (&shm_out[P_EDIT_CO]));
                // zlog_debug(g_zlog_zc, "%-40s[%s] 接收到CO2 [%u]", "[data_collection_co2_func]", c_seral_name, i_data_value);
            }
            else
            {
                ++i_data_count;
            }
        }
        else
        {
            if (i_data_count >= 10)
            {
                // P_CO2_STATUS 为1时消警
                if (get_val(P_CO2_STATUS) == OFF)
                {
                    set_val(SENSOR_CO2_ALARM, OFF);
                }
                else
                {
                    set_val(SENSOR_CO2_ALARM, ON);
                }
                set_val(R_AI_CO, 0xffff);
            }

            // zlog_error(g_zlog_zc, "[%s] 接收数据超时 超时时间[%u]", c_seral_name, i_cmd_wait_time_out);
            // zlog_error(g_zlog_zc, "%-40s[%s] 发送读取数据[%d]:", "[data_collection_co2_func]", c_seral_name, i_recv_len);
            // hzlog_error(g_zlog_zc, b_cmd_sendbuf, 9);
            ++i_data_count;
        }

        if (get_val(P_EDIT_CO) != v_us_co2_old_val)
        {
            // 发送数据
            SerialFlush(i_serial_fd);
            SerialWrite(i_serial_fd, b_cmd_sendcheck, 9);
            zlog_debug(g_zlog_zc, "%-40s[%s] 发送校准数据[9]:", "[data_collection_co2_func]", c_seral_name);
            hzlog_debug(g_zlog_zc, b_cmd_sendcheck, 9);
            wait_ms(1000);
            v_us_co2_old_val = 0;
            set_val(P_EDIT_CO, 0);
            set_val(CARBON_DIOXIDE_CALIBRATION_STATUS, 1);
        }
    }
}

//----------------------------------------------------------------------------------------------------------
// 同步电机线程
//----------------------------------------------------------------------------------------------------------
void *synchronous_motor_func(void *pv)
{
    zlog_debug(g_zlog_zc, "%-40s同步电机控制线程启动", "[synchronous_motor_func]");

    unsigned char b_cmd_sendbuf[1024] = {0x00};
    unsigned char b_cmd_recvbuf[1024] = {0x00};

    // 打开串口
    int i_cmd_meterid = 0x01;
    int i_cmd_func = 0x03;

    int i_bandrate = 19200;
    int i_parity = 2;
    int i_databits = 8;
    int i_stopbit = 1;

    int i_cmd_wait_time_out = 1000;
    int i_cmd_read_time_out = 50;

    int i_send_len = 0;
    int i_recv_len = 0;
    int i_data_len = 0;
    int i_data_count = 0;
    int i_data_value = 0;

    int i_wait_time = 100;

    int i_ret = 0;

    int i_alarm_status = 0;

    int i_modbus_err_count = 0;
    int i_modbus_err_status = 1;

    unsigned char g_sync_motor_mode = 0xff;
    // int bak_sync_motor_mode = -1;

    unsigned short i_recv_data = 0;
    const char *c_seral_name = "ttyS2";
    zlog_info(g_zlog_zc, "%-40s正在打开串口!", "[synchronous_motor_func]");
    int i_serial_fd = SerialOpen(c_seral_name);
    if (i_serial_fd < 0)
    {
        zlog_error(g_zlog_zc, "%-40s%s串口打开失败", "[synchronous_motor_func]", c_seral_name);
        pthread_exit(NULL);
    }
    zlog_info(g_zlog_zc, "%-40s%s串口打开成功!", "[synchronous_motor_func]", c_seral_name);
    // 设置波特率
    SerialSetSpeed(i_serial_fd, i_bandrate);

    // 设置校验、数据位、停止位
    SerialSetParam(i_serial_fd, i_parity, i_databits, i_stopbit);
    zlog_info(g_zlog_zc, "%-40s[%s] bandrate=%d parity=%d databits=%d stopbit=%d  open success!!", "[synchronous_motor_func]", c_seral_name, i_bandrate, i_parity, i_databits, i_stopbit);

    // 重置 设置频率
    shm_old[S_SYNC_MOTOR_CONTROLRUN_FREQ] = 0;
    while (ON)
    {
        char sync_motor_mode = get_val(R_SYNC_MOTOR_MODE);
        zlog_debug(g_zlog_zc, "%-40s模式：%02x, 状态:%02x", "[synchronous_motor_func]", sync_motor_mode, g_fan_status);

        // if((bak_sync_motor_mode != -1) && (bak_sync_motor_mode != sync_motor_mode))
        // {
        //     exit(EXIT_SUCCESS);
        // }

        // bak_sync_motor_mode = sync_motor_mode;

        if (sync_motor_mode)
        {
            g_fan_status = 1;
            set_val(R_SYNC_VERSION, R_SYNC_FREQ_NAME);
        }
        else
        {
            g_fan_status = 0;
            set_val(R_SYNC_VERSION, R_SYNC_FIXED_NAME);
        }

        if (g_fan_status)
        {
            // 启动或者停止同步电机
            if (get_val(S_SYNC_MOTOR_CONTROL_CMD) == 1)
            {
                // 读取控制器故障地址
                i_cmd_func = 0x06;
                i_data_len = 0x01;
                i_send_len = 0x08;

                b_cmd_sendbuf[0] = i_cmd_meterid;                         // 设备号
                b_cmd_sendbuf[1] = i_cmd_func;                            // 功能码
                b_cmd_sendbuf[2] = SYNC_MOTOR_MB_CONTROL_CMD_ADDR >> 8;   // 读取控制状态地址高位
                b_cmd_sendbuf[3] = SYNC_MOTOR_MB_CONTROL_CMD_ADDR & 0xff; // 读取控制状态地址低位
                b_cmd_sendbuf[4] = SYNC_MOTOR_MB_CONTROL_JUST_RUN >> 8;   // 读取长度高位
                b_cmd_sendbuf[5] = SYNC_MOTOR_MB_CONTROL_JUST_RUN & 0xff; // 读取长度高位

                // 计算校验码
                unsigned short check = crc_chk_value(b_cmd_sendbuf, i_send_len - 2);
                memcpy(&b_cmd_sendbuf[6], &check, 2);
                // b_cmd_sendbuf[6] = check >> 8;
                // b_cmd_sendbuf[7] = check & 0xff;

                zlog_debug(g_zlog_zc, "%-40s发送读取数据[8]:", "[synchronous_motor_func]");
                hzlog_debug(g_zlog_zc, b_cmd_sendbuf, i_send_len);

                // 刷新串口
                SerialFlush(i_serial_fd);
                // 发送数据
                SerialWrite(i_serial_fd, b_cmd_sendbuf, i_send_len);
                // 等待数据
                i_recv_len = SerialRead(i_serial_fd, b_cmd_recvbuf, 8, i_cmd_wait_time_out, i_cmd_read_time_out);
                if (i_recv_len < 8)
                {
                    if (i_recv_len < 0)
                        i_recv_len = 0;
                    i_ret = SerialRead(i_serial_fd, &b_cmd_recvbuf[i_recv_len], 8 - i_recv_len, i_cmd_wait_time_out, i_cmd_read_time_out);
                    i_recv_len = i_recv_len + i_ret;
                }

                // 打印数据
                zlog_debug(g_zlog_zc, "%-40s接收读取数据[%d]:", "[synchronous_motor_func]", i_recv_len);
                hzlog_debug(g_zlog_zc, b_cmd_recvbuf, i_recv_len);

                if (i_recv_len >= 8)
                {
                    // 计算校验码
                    check = crc_chk_value(b_cmd_recvbuf, 6);
                    zlog_debug(g_zlog_zc, "%-40s校验码[%02x %02x]", "[synchronous_motor_func]", check & 0xff, (check >> 8) & 0xff);
                    if ((b_cmd_recvbuf[0] == i_cmd_meterid) && (b_cmd_recvbuf[1] == i_cmd_func) && (memcmp(&b_cmd_recvbuf[6], &check, 2) == 0))
                    {
                        set_uval(R_SYNC_MOTOR_CONNECT_STATUS, get_uval(R_SYNC_MOTOR_CONNECT_STATUS) & ~(0x01));
                        set_uval(S_SYNC_MOTOR_CONTROL_CMD, 0);
                    }
                    else
                    {
                        set_uval(R_SYNC_MOTOR_CONNECT_STATUS, get_uval(R_SYNC_MOTOR_CONNECT_STATUS) | 0x01);
                    }
                }
                else
                {
                    set_uval(R_SYNC_MOTOR_CONNECT_STATUS, get_uval(R_SYNC_MOTOR_CONNECT_STATUS) | 0x01);
                }

                wait_ms(i_wait_time);
            }

            // 关闭电机
            if (get_val(S_SYNC_MOTOR_CONTROL_CMD) == 2)
            {
                // 读取控制器故障地址
                i_cmd_func = 0x06;
                i_data_len = 0x01;
                i_send_len = 0x08;

                b_cmd_sendbuf[0] = i_cmd_meterid;                           // 设备号
                b_cmd_sendbuf[1] = i_cmd_func;                              // 功能码
                b_cmd_sendbuf[2] = SYNC_MOTOR_MB_CONTROL_CMD_ADDR >> 8;     // 读取控制状态地址高位
                b_cmd_sendbuf[3] = SYNC_MOTOR_MB_CONTROL_CMD_ADDR & 0xff;   // 读取控制状态地址低位
                b_cmd_sendbuf[4] = SYNC_MOTOR_MB_CONTROL_DECEL_STOP >> 8;   // 读取长度高位
                b_cmd_sendbuf[5] = SYNC_MOTOR_MB_CONTROL_DECEL_STOP & 0xff; // 读取长度高位

                // 计算校验码
                unsigned short check = crc_chk_value(b_cmd_sendbuf, i_send_len - 2);
                memcpy(&b_cmd_sendbuf[6], &check, 2);
                // b_cmd_sendbuf[6] = check >> 8;
                // b_cmd_sendbuf[7] = check & 0xff;

                zlog_debug(g_zlog_zc, "%-40s发送读取数据[8]:", "[synchronous_motor_func]");
                hzlog_debug(g_zlog_zc, b_cmd_sendbuf, i_send_len);

                // 刷新串口
                SerialFlush(i_serial_fd);

                // 发送数据
                SerialWrite(i_serial_fd, b_cmd_sendbuf, i_send_len);

                // 等待数据
                i_recv_len = SerialRead(i_serial_fd, b_cmd_recvbuf, 8, i_cmd_wait_time_out, i_cmd_read_time_out);
                if (i_recv_len < 8)
                {
                    if (i_recv_len < 0)
                        i_recv_len = 0;
                    i_ret = SerialRead(i_serial_fd, &b_cmd_recvbuf[i_recv_len], 8 - i_recv_len, i_cmd_wait_time_out, i_cmd_read_time_out);
                    i_recv_len = i_recv_len + i_ret;
                }

                // 打印数据
                zlog_debug(g_zlog_zc, "%-40s接收读取数据[%d]:", "[synchronous_motor_func]", i_recv_len);
                hzlog_debug(g_zlog_zc, b_cmd_recvbuf, i_recv_len);

                if (i_recv_len >= 8)
                {
                    // 计算校验码
                    check = crc_chk_value(b_cmd_recvbuf, 6);
                    zlog_debug(g_zlog_zc, "%-40s校验码[%02x %02x]", "[synchronous_motor_func]", check & 0xff, (check >> 8) & 0xff);
                    if ((b_cmd_recvbuf[0] == i_cmd_meterid) && (b_cmd_recvbuf[1] == i_cmd_func) && (memcmp(&b_cmd_recvbuf[6], &check, 2) == 0))
                    {
                        set_uval(R_SYNC_MOTOR_CONNECT_STATUS, get_uval(R_SYNC_MOTOR_CONNECT_STATUS) & ~(0x02));
                        set_uval(S_SYNC_MOTOR_CONTROL_CMD, 0);
                    }
                    else
                    {
                        set_uval(R_SYNC_MOTOR_CONNECT_STATUS, get_uval(R_SYNC_MOTOR_CONNECT_STATUS) | 0x02);
                    }
                }
                else
                {
                    set_uval(R_SYNC_MOTOR_CONNECT_STATUS, get_uval(R_SYNC_MOTOR_CONNECT_STATUS) | 0x02);
                }
                wait_ms(i_wait_time);
            }

            // 读取控制器状态
            {
                i_cmd_func = 0x03;
                i_data_len = 0x01;
                i_send_len = 0x08;

                b_cmd_sendbuf[0] = i_cmd_meterid;                          // 设备号
                b_cmd_sendbuf[1] = i_cmd_func;                             // 功能码
                b_cmd_sendbuf[2] = SYNC_MOTOR_MB_CONTROL_PARA_ADDR >> 8;   // 读取控制状态地址高位
                b_cmd_sendbuf[3] = SYNC_MOTOR_MB_CONTROL_PARA_ADDR & 0xff; // 读取控制状态地址低位
                b_cmd_sendbuf[4] = i_data_len >> 8;                        // 读取长度高位
                b_cmd_sendbuf[5] = i_data_len & 0xff;                      // 读取长度高位

                // 计算校验码
                unsigned short check = crc_chk_value(b_cmd_sendbuf, i_send_len - 2);
                memcpy(&b_cmd_sendbuf[6], &check, 2);
                // b_cmd_sendbuf[6] = check >> 8;
                // b_cmd_sendbuf[7] = check & 0xff;

                zlog_debug(g_zlog_zc, "%-40s发送读取数据[8]:", "[synchronous_motor_func]");
                hzlog_debug(g_zlog_zc, b_cmd_sendbuf, i_send_len);

                // 刷新串口
                SerialFlush(i_serial_fd);
                // 发送数据
                SerialWrite(i_serial_fd, b_cmd_sendbuf, i_send_len);

                // 等待数据
                i_recv_len = SerialRead(i_serial_fd, b_cmd_recvbuf, 8, i_cmd_wait_time_out, i_cmd_read_time_out);
                if (i_recv_len < 7)
                {
                    if (i_recv_len < 0)
                        i_recv_len = 0;
                    i_ret = SerialRead(i_serial_fd, &b_cmd_recvbuf[i_recv_len], 8 - i_recv_len, i_cmd_wait_time_out, i_cmd_read_time_out);
                    i_recv_len = i_recv_len + i_ret;
                }

                // 打印数据
                zlog_debug(g_zlog_zc, "%-40s接收读取数据[%d]:", "[synchronous_motor_func]", i_recv_len);
                hzlog_debug(g_zlog_zc, b_cmd_recvbuf, i_recv_len);

                if (i_recv_len >= 7)
                {
                    // 计算校验码
                    check = crc_chk_value(b_cmd_recvbuf, 5);
                    zlog_debug(g_zlog_zc, "%-40s校验码[%02x %02x]", "[synchronous_motor_func]", check & 0xff, (check >> 8) & 0xff);
                    if ((b_cmd_recvbuf[0] == i_cmd_meterid) && (b_cmd_recvbuf[1] == i_cmd_func) && (memcmp(&b_cmd_recvbuf[5], &check, 2) == 0))
                    {
                        memcpy(&shm_out[R_SYNC_MOTOR_CONTROL_PARA], &b_cmd_recvbuf[3], 2);
                        set_uval(R_SYNC_MOTOR_CONNECT_STATUS, get_uval(R_SYNC_MOTOR_CONNECT_STATUS) & ~(0x04));
                        i_alarm_status = 0;
                    }
                    else
                    {
                        if (++i_alarm_status >= UINT16_MAX)
                        {
                            i_alarm_status = 10;
                        }

                        if (i_alarm_status >= 10)
                        {
                            set_uval(R_SYNC_MOTOR_CONNECT_STATUS, get_uval(R_SYNC_MOTOR_CONNECT_STATUS) | 0x04);
                        }
                        else
                        {
                            set_uval(R_SYNC_MOTOR_CONNECT_STATUS, get_uval(R_SYNC_MOTOR_CONNECT_STATUS) & ~(0x04));
                        }
                    }
                }
                else
                {
                    if (++i_alarm_status >= UINT16_MAX)
                    {
                        i_alarm_status = 10;
                    }

                    if (i_alarm_status >= 10)
                    {
                        set_uval(R_SYNC_MOTOR_CONNECT_STATUS, get_uval(R_SYNC_MOTOR_CONNECT_STATUS) | 0x04);
                    }
                    else
                    {
                        set_uval(R_SYNC_MOTOR_CONNECT_STATUS, get_uval(R_SYNC_MOTOR_CONNECT_STATUS) & ~(0x04));
                    }
                }
                wait_ms(i_wait_time);
            }

            // 修改设置频率
            if ((get_val(S_SYNC_MOTOR_CONTROLRUN_FREQ) != get_old(S_SYNC_MOTOR_CONTROLRUN_FREQ)) && (get_uval(S_SYNC_MOTOR_CONTROLRUN_FREQ) != 0))
            {
                // memcpy(&shm_old[S_SYNC_MOTOR_CONTROLRUN_FREQ], &shm_out[S_SYNC_MOTOR_CONTROLRUN_FREQ], 2);
                shm_old[S_SYNC_MOTOR_CONTROLRUN_FREQ] = shm_out[S_SYNC_MOTOR_CONTROLRUN_FREQ];

                // 控制频率
                // unsigned short i_run_freq = get_uval(S_SYNC_MOTOR_CONTROLRUN_FREQ);

                // 读取控制器故障地址
                i_cmd_func = 0x06;
                i_data_len = 0x01;
                i_send_len = 0x08;

                b_cmd_sendbuf[0] = i_cmd_meterid;                      // 设备号
                b_cmd_sendbuf[1] = i_cmd_func;                         // 功能码
                b_cmd_sendbuf[2] = SYNC_MOTOR_MB_RUN_FREQ_ADDR >> 8;   // 读取控制状态地址高位
                b_cmd_sendbuf[3] = SYNC_MOTOR_MB_RUN_FREQ_ADDR & 0xff; // 读取控制状态地址低位
                // b_cmd_sendbuf[4] = i_run_freq >> 8;                    // 读取长度高位
                // b_cmd_sendbuf[5] = i_run_freq & 0xff;                  // 读取长度高位
                memcpy(&b_cmd_sendbuf[4], &shm_out[S_SYNC_MOTOR_CONTROLRUN_FREQ], 2);

                // 计算校验码
                unsigned short check = crc_chk_value(b_cmd_sendbuf, i_send_len - 2);
                memcpy(&b_cmd_sendbuf[6], &check, 2);
                // b_cmd_sendbuf[6] = check >> 8;
                // b_cmd_sendbuf[7] = check & 0xff;

                zlog_debug(g_zlog_zc, "%-40s发送读取数据[8]:", "[synchronous_motor_func]");
                hzlog_debug(g_zlog_zc, b_cmd_sendbuf, i_send_len);

                // 刷新串口
                SerialFlush(i_serial_fd);
                // 发送数据
                SerialWrite(i_serial_fd, b_cmd_sendbuf, i_send_len);
                // 等待数据
                i_recv_len = SerialRead(i_serial_fd, b_cmd_recvbuf, 8, i_cmd_wait_time_out, i_cmd_read_time_out);
                if (i_recv_len < 8)
                {
                    if (i_recv_len < 0)
                        i_recv_len = 0;
                    i_ret = SerialRead(i_serial_fd, &b_cmd_recvbuf[i_recv_len], 8 - i_recv_len, i_cmd_wait_time_out, i_cmd_read_time_out);
                    i_recv_len = i_recv_len + i_ret;
                }

                // 打印数据
                zlog_debug(g_zlog_zc, "%-40s接收读取数据[%d]:", "[synchronous_motor_func]", i_recv_len);
                hzlog_debug(g_zlog_zc, b_cmd_recvbuf, i_recv_len);

                if (i_recv_len >= 8)
                {
                    // 计算校验码
                    check = crc_chk_value(b_cmd_recvbuf, 6);
                    zlog_debug(g_zlog_zc, "%-40s校验码[%02x %02x]", "[synchronous_motor_func]", check & 0xff, (check >> 8) & 0xff);
                    if ((b_cmd_recvbuf[0] == i_cmd_meterid) && (b_cmd_recvbuf[1] == i_cmd_func) && (memcmp(&b_cmd_recvbuf[6], &check, 2) == 0))
                    {
                        set_uval(R_SYNC_MOTOR_CONNECT_STATUS, get_uval(R_SYNC_MOTOR_CONNECT_STATUS) & ~(0x08));
                        // set_uval(S_SYNC_MOTOR_CONTROLRUN_FREQ, 0);
                    }
                    else
                    {
                        set_uval(R_SYNC_MOTOR_CONNECT_STATUS, get_uval(R_SYNC_MOTOR_CONNECT_STATUS) | 0x08);
                    }
                }
                else
                {
                    set_uval(R_SYNC_MOTOR_CONNECT_STATUS, get_uval(R_SYNC_MOTOR_CONNECT_STATUS) | 0x08);
                }

                wait_ms(i_wait_time);
            }

            // // 读取设置频率
            // {
            //     i_cmd_func = 0x03;
            //     i_data_len = 0x01;
            //     i_send_len = 0x08;

            //     b_cmd_sendbuf[0] = i_cmd_meterid;                      // 设备号
            //     b_cmd_sendbuf[1] = i_cmd_func;                         // 功能码
            //     b_cmd_sendbuf[2] = SYNC_MOTOR_MB_RUN_FREQ_ADDR >> 8;   // 读取运行频率地址高位
            //     b_cmd_sendbuf[3] = SYNC_MOTOR_MB_RUN_FREQ_ADDR & 0xff; // 读取运行频率地址低位
            //     b_cmd_sendbuf[4] = i_data_len >> 8;                    // 读取长度高位
            //     b_cmd_sendbuf[5] = i_data_len & 0xff;                  // 读取长度高位

            //     // 计算校验码
            //     unsigned short check = crc_chk_value(b_cmd_sendbuf, i_send_len - 2);
            //     memcpy(&b_cmd_sendbuf[6], &check, 2);
            //     // b_cmd_sendbuf[6] = check >> 8;
            //     // b_cmd_sendbuf[7] = check & 0xff;

            //     zlog_debug(g_zlog_zc, "%-40s发送读取数据[8]:", "[synchronous_motor_func]");
            //     hzlog_debug(g_zlog_zc, b_cmd_sendbuf, i_send_len);

            //     // 刷新串口
            //     SerialFlush(i_serial_fd);
            //     // 发送数据
            //     SerialWrite(i_serial_fd, b_cmd_sendbuf, i_send_len);
            //     // 等待数据
            //     i_recv_len = SerialRead(i_serial_fd, b_cmd_recvbuf, 8, i_cmd_wait_time_out, i_cmd_read_time_out);
            //     if (i_recv_len < 7)
            //     {
            //         if (i_recv_len < 0)
            //             i_recv_len = 0;
            //         i_ret = SerialRead(i_serial_fd, &b_cmd_recvbuf[i_recv_len], 8 - i_recv_len, i_cmd_wait_time_out, i_cmd_read_time_out);
            //         i_recv_len = i_recv_len + i_ret;
            //     }

            //     // 打印数据
            //     zlog_debug(g_zlog_zc, "%-40s接收读取数据[%d]:", "[synchronous_motor_func]", i_recv_len);
            //     hzlog_debug(g_zlog_zc, b_cmd_recvbuf, i_recv_len);

            //     if (i_recv_len >= 7)
            //     {
            //         // 计算校验码
            //         check = crc_chk_value(b_cmd_recvbuf, 5);
            //         zlog_debug(g_zlog_zc, "%-40s校验码[%02x %02x]", "[synchronous_motor_func]", check & 0xff, (check >> 8) & 0xff);
            //         if ((b_cmd_recvbuf[0] == i_cmd_meterid) && (b_cmd_recvbuf[1] == i_cmd_func) && (memcmp(&b_cmd_recvbuf[5], &check, 2) == 0))
            //         {
            //             memcpy(&shm_old[S_SYNC_MOTOR_CONTROLRUN_FREQ], &b_cmd_recvbuf[3], 2);
            //             set_uval(R_SYNC_MOTOR_CONNECT_STATUS, get_uval(R_SYNC_MOTOR_CONNECT_STATUS) & ~(0x10));
            //         }
            //         else
            //         {
            //             set_uval(R_SYNC_MOTOR_CONNECT_STATUS, get_uval(R_SYNC_MOTOR_CONNECT_STATUS) | 0x10);
            //         }
            //     }
            //     else
            //     {
            //         set_uval(R_SYNC_MOTOR_CONNECT_STATUS, get_uval(R_SYNC_MOTOR_CONNECT_STATUS) | 0x10);
            //     }
            //     wait_ms(500);
            // }

            // 读取当前频率
            {
                i_cmd_func = 0x03;
                i_data_len = 0x01;
                i_send_len = 0x08;

                b_cmd_sendbuf[0] = i_cmd_meterid;                          // 设备号
                b_cmd_sendbuf[1] = i_cmd_func;                             // 功能码
                b_cmd_sendbuf[2] = SYNC_MOTOR_MB_CURRENT_FREQ_ADDR >> 8;   // 读取控制状态地址高位
                b_cmd_sendbuf[3] = SYNC_MOTOR_MB_CURRENT_FREQ_ADDR & 0xff; // 读取控制状态地址低位
                b_cmd_sendbuf[4] = i_data_len >> 8;                        // 读取长度高位
                b_cmd_sendbuf[5] = i_data_len & 0xff;                      // 读取长度高位

                // 计算校验码
                unsigned short check = crc_chk_value(b_cmd_sendbuf, i_send_len - 2);
                memcpy(&b_cmd_sendbuf[6], &check, 2);
                // b_cmd_sendbuf[6] = check >> 8;
                // b_cmd_sendbuf[7] = check & 0xff;

                zlog_debug(g_zlog_zc, "%-40s发送读取数据[8]:", "[synchronous_motor_func]");
                hzlog_debug(g_zlog_zc, b_cmd_sendbuf, i_send_len);

                // 刷新串口
                SerialFlush(i_serial_fd);
                // 发送数据
                SerialWrite(i_serial_fd, b_cmd_sendbuf, i_send_len);
                // 等待数据
                i_recv_len = SerialRead(i_serial_fd, b_cmd_recvbuf, 8, i_cmd_wait_time_out, i_cmd_read_time_out);
                if (i_recv_len < 7)
                {
                    if (i_recv_len < 0)
                        i_recv_len = 0;
                    i_ret = SerialRead(i_serial_fd, &b_cmd_recvbuf[i_recv_len], 8 - i_recv_len, i_cmd_wait_time_out, i_cmd_read_time_out);
                    i_recv_len = i_recv_len + i_ret;
                }

                // 打印数据
                zlog_debug(g_zlog_zc, "%-40s接收读取数据[%d]:", "[synchronous_motor_func]", i_recv_len);
                hzlog_debug(g_zlog_zc, b_cmd_recvbuf, i_recv_len);

                if (i_recv_len >= 7)
                {
                    // 计算校验码
                    check = crc_chk_value(b_cmd_recvbuf, 5);
                    zlog_debug(g_zlog_zc, "%-40s校验码[%02x %02x]", "[synchronous_motor_func]", check & 0xff, (check >> 8) & 0xff);
                    if ((b_cmd_recvbuf[0] == i_cmd_meterid) && (b_cmd_recvbuf[1] == i_cmd_func) && (memcmp(&b_cmd_recvbuf[5], &check, 2) == 0))
                    {
                        int current = b_cmd_recvbuf[3] * 256 + b_cmd_recvbuf[4];
                        zlog_debug(g_zlog_zc, "%-40s当前频率:[%d]:", "[synchronous_motor_func]", current);
                        memcpy(&shm_out[R_SYNC_MOTOR_CURRENT_FREQ], &b_cmd_recvbuf[3], 2);

                        set_uval(R_SYNC_MOTOR_CONNECT_STATUS, get_uval(R_SYNC_MOTOR_CONNECT_STATUS) & ~(0x20));
                    }
                    else
                    {
                        set_uval(R_SYNC_MOTOR_CONNECT_STATUS, get_uval(R_SYNC_MOTOR_CONNECT_STATUS) | 0x20);
                    }
                }
                else
                {
                    set_uval(R_SYNC_MOTOR_CONNECT_STATUS, get_uval(R_SYNC_MOTOR_CONNECT_STATUS) | 0x20);
                }
                wait_ms(i_wait_time);
            }

            // // 读取控制器故障
            // {
            //     i_cmd_func = 0x03;
            //     i_data_len = 0x01;
            //     i_send_len = 0x08;

            //     b_cmd_sendbuf[0] = i_cmd_meterid;                           // 设备号
            //     b_cmd_sendbuf[1] = i_cmd_func;                              // 功能码
            //     b_cmd_sendbuf[2] = SYNC_MOTOR_MB_CONTROL_FAULT_ADDR >> 8;   // 读取控制状态地址高位
            //     b_cmd_sendbuf[3] = SYNC_MOTOR_MB_CONTROL_FAULT_ADDR & 0xff; // 读取控制状态地址低位
            //     b_cmd_sendbuf[4] = i_data_len >> 8;                         // 读取长度高位
            //     b_cmd_sendbuf[5] = i_data_len & 0xff;                       // 读取长度高位

            //     // 计算校验码
            //     unsigned short check = crc_chk_value(b_cmd_sendbuf, i_send_len - 2);
            //     memcpy(&b_cmd_sendbuf[6], &check, 2);
            //     // b_cmd_sendbuf[6] = check >> 8;
            //     // b_cmd_sendbuf[7] = check & 0xff;

            //     zlog_debug(g_zlog_zc, "%-40s发送读取数据[8]:", "[synchronous_motor_func]");
            //     hzlog_debug(g_zlog_zc, b_cmd_sendbuf, i_send_len);

            //     // 刷新串口
            //     SerialFlush(i_serial_fd);
            //     // 发送数据
            //     SerialWrite(i_serial_fd, b_cmd_sendbuf, i_send_len);
            //     // 等待数据
            //     i_recv_len = SerialRead(i_serial_fd, b_cmd_recvbuf, 8, i_cmd_wait_time_out, i_cmd_read_time_out);
            //     if (i_recv_len < 7)
            //     {
            //         if (i_recv_len < 0)
            //             i_recv_len = 0;
            //         i_ret = SerialRead(i_serial_fd, &b_cmd_recvbuf[i_recv_len], 8 - i_recv_len, i_cmd_wait_time_out, i_cmd_read_time_out);
            //         i_recv_len = i_recv_len + i_ret;
            //     }

            //     // 打印数据
            //     zlog_debug(g_zlog_zc, "%-40s接收读取数据[%d]:", "[synchronous_motor_func]", i_recv_len);
            //     hzlog_debug(g_zlog_zc, b_cmd_recvbuf, i_recv_len);

            //     if (i_recv_len >= 7)
            //     {
            //         // 计算校验码
            //         check = crc_chk_value(b_cmd_recvbuf, 5);
            //         zlog_debug(g_zlog_zc, "%-40s校验码[%02x %02x]", "[synchronous_motor_func]", check & 0xff, (check >> 8) & 0xff);
            //         if ((b_cmd_recvbuf[0] == i_cmd_meterid) && (b_cmd_recvbuf[1] == i_cmd_func) && (memcmp(&b_cmd_recvbuf[5], &check, 2) == 0))
            //         {
            //             memcpy(&shm_out[R_SYNC_MOTOR_CONTROL_FAULT], &b_cmd_recvbuf[3], 2);
            //             set_uval(R_SYNC_MOTOR_CONNECT_STATUS, get_uval(R_SYNC_MOTOR_CONNECT_STATUS) & ~(0x40));
            //         }
            //         else
            //         {
            //             set_uval(R_SYNC_MOTOR_CONNECT_STATUS, get_uval(R_SYNC_MOTOR_CONNECT_STATUS) | 0x40);
            //         }
            //     }
            //     else
            //     {
            //         set_uval(R_SYNC_MOTOR_CONNECT_STATUS, get_uval(R_SYNC_MOTOR_CONNECT_STATUS) | 0x40);
            //     }
            //     wait_ms(i_wait_time);
            // }
        }
        else
        {
            wait_ms(500);
        }
    }
}

//----------------------------------------------------------------------------------------------------------
// 翻蛋线程
//----------------------------------------------------------------------------------------------------------
void *flip_egg_func(void *pv)
{
    zlog_debug(g_zlog_zc, "%-40s翻蛋线程启动", "[flip_egg_func]");

    // 翻蛋计时状态
    unsigned char sys_now_time_status = ON;

    // 翻蛋控制时间
    struct timespec start, end;
    while (ON)
    {
        // 赋值翻蛋值
        set_uval(P_EGG_TIME, get_val(P_EGG_FLIPPING_TIME));

        // 当翻蛋时间等于0时，触发翻蛋
        if (get_uval(P_EGG_FLIPPING_TIME) == 0)
        {
            // 翻蛋控制
            control_egg();
        }

        // 获取翻蛋执行状态
        if (get_uval(P_EGG_FLIPPING_EXECUTION_STATUS))
        {
            if (sys_now_time_status)
            {
                // 获取开始时间
                clock_gettime(CLOCK_MONOTONIC, &start);
                sys_now_time_status = OFF;
            }

            if (!sys_now_time_status)
            {
                // 获取当前时间
                clock_gettime(CLOCK_MONOTONIC, &end);

                // 计算时间差
                double elapsed_time = (end.tv_sec - start.tv_sec) +
                                      (end.tv_nsec - start.tv_nsec) / 1e9;

                // 将浮点数转为整型，只获取秒数
                int sys_sub_time = elapsed_time;
                if ((sys_sub_time >= 0) && (sys_sub_time <= get_val(EGG_FLIPP_INGINTERVAL)))
                {
                    // 翻蛋方向-左
                    if (get_uval(P_EGG_FLIPPING_DIRECTION))
                    {
                        set_uval(FILP_EGG_ON, ON);
                        set_uval(FILP_EGG_OFF, OFF);
                    }

                    // 翻蛋方向-右
                    else
                    {
                        set_uval(FILP_EGG_ON, OFF);
                        set_uval(FILP_EGG_OFF, ON);
                    }
                }
                else
                {
                    sys_now_time_status = ON;
                    set_uval(FILP_EGG_ON, OFF);
                    set_uval(FILP_EGG_OFF, OFF);
                    set_val(P_EGG_FLIPPING_EXECUTION_STATUS, OFF);
                }
            }
        }
        else
        {
            sys_now_time_status = ON;
            set_uval(FILP_EGG_ON, OFF);
            set_uval(FILP_EGG_OFF, OFF);
        }

        // if (get_val(DETECT_FLIP_EGG) == ON)
        // {
        //     // 翻蛋控制状态为ON时，将翻蛋时间清空
        //     if (get_val(EGG_FLIPPING_CONTROL_STATUS) == ON)
        //     {
        //         // 将翻蛋时间清空
        //         set_uval(P_EGG_FLIPPING_TIME, 0);

        //         // 将翻蛋控制状态为OFF
        //         set_val(EGG_FLIPPING_CONTROL_STATUS, OFF);
        //     }
        // }
        // else
        // {
        //     // 未触发翻蛋时，将 翻蛋控制状态为ON
        //     set_val(EGG_FLIPPING_CONTROL_STATUS, ON);
        // }

        // 翻蛋触发状态为ON
        if (get_val(EGG_FLIPPING_TRIGGER_STATUS))
        {
            // 未监测到翻蛋信号时，翻蛋累加状态为ON
            if (!get_val(DETECT_FLIP_EGG))
            {
                // 翻蛋累加状态为ON
                set_val(EGG_FLIPPING_ACCUMULATION_STATE, ON);
            }

            // 当翻蛋累加状态为ON 且 翻蛋信号为ON
            if ((get_val(EGG_FLIPPING_ACCUMULATION_STATE) == ON) && (get_val(DETECT_FLIP_EGG) == ON))
            {
                // 翻蛋累加
                set_val(R_AI_TP_EGG, get_val(R_AI_TP_EGG) + 1);

                // 清空状态值
                set_val(EGG_FLIPPING_ACCUMULATION_STATE, OFF);
                set_val(EGG_FLIPPING_TRIGGER_STATUS, OFF);
            }
        }
        else
        {
            // 未触发翻蛋时，将 翻蛋触发状态为OFF
            set_val(EGG_FLIPPING_ACCUMULATION_STATE, OFF);
        }

        // 延迟1秒
        wait_ms(1000);
    }
}

//----------------------------------------------------------------------------------------------------------
// 报警线程
//----------------------------------------------------------------------------------------------------------
void *thread_alarm_func(void *pv)
{
    zlog_debug(g_zlog_zc, "%-40s进入到报警线程", "[thread_alarm_func]");
    // 传感器突变总和
    unsigned short us_pt100_val = 0;

    // 回流温度报警状态
    int ref_temp_mode_old = 0;
    int ref_temp_mode_high_old = 0;
    int ref_temp_mode_low_old = 0;
    int di_fan_mode = ON;
    int status_fan_mode = ON;

    // 门开关报警
    int alarm_door_switch = 0;

    //----------------------------------------------------------------------------------------------------------
    // 有翻蛋信号初始化
    //----------------------------------------------------------------------------------------------------------
    set_uval(EGG_FLIPPING_SIGNAL_RINGING_STATUS, ON);
    set_uval(NO_EGG_FLIPPING_SIGNAL_RINGING_STATUS, ON);

    while (ON)
    {
        // 更新报警点数值
        temp_update_para_data();

        //----------------------------------------------------------------------------------------------------------
        // 高温报警
        // 当PV2>AH2+HYS (AH2为高温报警点,由程序可以修改)
        //----------------------------------------------------------------------------------------------------------
        if ((get_val(R_AI_TP_MAIN) >= (get_val(HIGH_TEMP_ALARM_VALUE) + get_val(P_HYS))) || (get_val(DETECT_AUX_HIGH_TEMP) == ON))
        {
            // 启动高温报警灯
            set_val(OUT_ST_TP_HIGH_ALARM, ON);
            set_val(HIGH_TEMP_ALARM_STATUS, ON);
        }

        if ((get_val(R_AI_TP_MAIN) < get_val(HIGH_TEMP_ALARM_VALUE)) && (get_val(DETECT_AUX_HIGH_TEMP) == OFF))
        {
            // 关闭高温报警灯
            set_val(OUT_ST_TP_HIGH_ALARM, OFF);
            set_val(HIGH_TEMP_ALARM_STATUS, OFF);
        }

        //----------------------------------------------------------------------------------------------------------
        // 低温 报警
        // PV2<=AL2-HYS,(AL2为低温报警点,由程序可以修改)时,辅助加热动作（输出辅助加热）,输出低温警报指示灯
        //----------------------------------------------------------------------------------------------------------
        if ((get_val(R_AI_TP_MAIN) <= (get_val(LOW_TEMP_ALARM_VALUE) - get_val(P_HYS))) && (get_val(DETECT_AUX_HIGH_TEMP) == OFF))
        {
            // 主温度低值报警指示灯为ON
            set_val(OUT_ST_TP_LOW_ALARM, ON);
            set_val(LOW_TEMP_ALARM_STATUS, ON);
        }

        if ((get_val(R_AI_TP_MAIN) > get_val(LOW_TEMP_ALARM_VALUE)) && (get_val(DETECT_AUX_HIGH_TEMP) == OFF))
        {
            // 主温度低值报警指示灯为OFF
            set_val(OUT_ST_TP_LOW_ALARM, OFF);
            set_val(LOW_TEMP_ALARM_STATUS, OFF);
        }

        //----------------------------------------------------------------------------------------------------------
        // 高湿 报警
        // 当PV1>=AH1+HYS,输出高湿警报指示灯。
        //----------------------------------------------------------------------------------------------------------
        if (get_val(R_AI_HM) >= (get_val(HIGH_HUMI_ALARM_VALUE) + get_val(P_HYS)))
        {
            if (get_val(HIGH_HUMIDITY_ALARM_DELAY_STATUS) == 0)
            {
                // 设置时间
                set_val(HIGH_HUMIDITY_ALARM_DELAY_TIME, get_val(P_HH11) * 60);
                set_val(HIGH_HUMIDITY_ALARM_DELAY_STATUS, ON);
            }
            else
            {
                if (get_val(HIGH_HUMIDITY_ALARM_DELAY_TIME) == 0)
                {
                    // 开启高湿警报状态灯
                    set_val(OUT_ST_HM_HIGH_ALARM, ON);
                    set_val(HIGH_HUMI_ALARM_STATUS, ON);
                }
                else
                {
                    // 关闭启高湿警报状态灯
                    set_val(OUT_ST_HM_HIGH_ALARM, OFF);
                    set_val(HIGH_HUMI_ALARM_STATUS, OFF);
                }
            }
        }

        if ((get_val(R_AI_HM) < get_val(HIGH_HUMI_ALARM_VALUE)))
        {
            // 清除高湿报警状态
            set_val(HIGH_HUMIDITY_ALARM_DELAY_STATUS, OFF);

            // 关闭高湿警报状态灯
            set_val(OUT_ST_HM_HIGH_ALARM, OFF);
            set_val(HIGH_HUMI_ALARM_STATUS, OFF);
        }

        //----------------------------------------------------------------------------------------------------------
        // 低湿报警
        // 当PV1 ≤ AL - HYS, 输出低湿警报指示灯
        //----------------------------------------------------------------------------------------------------------
        if (get_val(R_AI_HM) <= (get_val(LOW_HUMI_ALARM_VALUE) - get_val(P_HYS)))
        {
            // 开启低湿警报状态灯
            set_val(OUT_ST_HM_LOW_ALARM, ON);
            set_val(LOW_HUMI_ALARM_STATUS, ON);
        }

        if (get_val(R_AI_HM) > get_val(LOW_HUMI_ALARM_VALUE))
        {
            // 关闭低湿警报状态灯
            set_val(OUT_ST_HM_LOW_ALARM, OFF);
            set_val(LOW_HUMI_ALARM_STATUS, OFF);
        }

        //----------------------------------------------------------------------------------------------------------
        // 回流温度模式
        //----------------------------------------------------------------------------------------------------------
        if (get_uval(REFLUX_TEMPERATURE) & 0x01)
        {
            //----------------------------------------------------------------------------------------------------------
            // 回流高温1 报警
            //----------------------------------------------------------------------------------------------------------
            if (get_val(R_AI_TP_RF1) >= (get_val(HIGH_TEMP_ALARM_VALUE_FOR_REFLUX_TEMP_1) + get_val(P_RTEMP_HYS)))
            {
                // 启动响铃
                set_val(RETURN_TEMPE_HIGH_TEMP_ALARM_STATUS_1, ON);
            }

            if (get_val(R_AI_TP_RF1) < get_val(HIGH_TEMP_ALARM_VALUE_FOR_REFLUX_TEMP_1))
            {
                // 关闭响铃
                set_val(RETURN_TEMPE_HIGH_TEMP_ALARM_STATUS_1, OFF);
            }

            //----------------------------------------------------------------------------------------------------------
            // 回流低温1 报警
            //----------------------------------------------------------------------------------------------------------
            if (get_val(R_AI_TP_RF1) <= (get_val(LOW_TEMP_ALARM_VALUE_FOR_REFLUX_TEMP_1) - get_val(P_RTEMP_HYS)))
            {
                // 开启报警灯
                set_val(RETURN_TEMP_LOW_TEMP_ALARM_STATUS_1, ON);
            }

            if (get_val(R_AI_TP_RF1) > get_val(LOW_TEMP_ALARM_VALUE_FOR_REFLUX_TEMP_1))
            {
                // 关闭报警灯
                set_val(RETURN_TEMP_LOW_TEMP_ALARM_STATUS_1, OFF);
            }
        }
        else
        {
            set_val(RETURN_TEMPE_HIGH_TEMP_ALARM_STATUS_1, OFF); // 初始化 回流温度 高温报警
            set_val(RETURN_TEMP_LOW_TEMP_ALARM_STATUS_1, OFF);   // 初始化 回流温度 低温报警
            set_val(SENSOR_3_ABNORMAL_ALARM, OFF);
        }

        if (get_uval(REFLUX_TEMPERATURE) & 0x02)
        {
            //----------------------------------------------------------------------------------------------------------
            // 回流高温2 报警
            //----------------------------------------------------------------------------------------------------------
            if (get_val(R_AI_TP_RF2) >= (get_val(HIGH_TEMP_ALARM_VALUE_FOR_REFLUX_TEMP_2) + get_val(P_RTEMP_HYS)))
            {
                // 开启报警灯
                set_val(RETURN_TEMPE_HIGH_TEMP_ALARM_STATUS_2, ON);
            }

            if (get_val(R_AI_TP_RF2) < get_val(HIGH_TEMP_ALARM_VALUE_FOR_REFLUX_TEMP_2))
            {
                // 关闭报警灯
                set_val(RETURN_TEMPE_HIGH_TEMP_ALARM_STATUS_2, OFF);
            }

            //----------------------------------------------------------------------------------------------------------
            // 回流低温2 报警
            //----------------------------------------------------------------------------------------------------------
            if (get_val(R_AI_TP_RF2) <= (get_val(LOW_TEMP_ALARM_VALUE_FOR_REFLUX_TEMP_2) - get_val(P_RTEMP_HYS)))
            {
                // 开启报警灯
                set_val(RETURN_TEMP_LOW_TEMP_ALARM_STATUS_2, ON);
            }

            if (get_val(R_AI_TP_RF2) > get_val(LOW_TEMP_ALARM_VALUE_FOR_REFLUX_TEMP_2))
            {
                // 关闭报警灯
                set_val(RETURN_TEMP_LOW_TEMP_ALARM_STATUS_2, OFF);
            }
        }
        else
        {
            set_val(RETURN_TEMPE_HIGH_TEMP_ALARM_STATUS_2, OFF); // 初始化 回流温度 高温报警
            set_val(RETURN_TEMP_LOW_TEMP_ALARM_STATUS_2, OFF);   // 初始化 回流温度 低温报警
            set_val(SENSOR_4_ABNORMAL_ALARM, OFF);
        }

        if (get_uval(REFLUX_TEMPERATURE) & 0x04)
        {
            //----------------------------------------------------------------------------------------------------------
            // 回流高温3 报警
            //----------------------------------------------------------------------------------------------------------
            if (get_val(R_AI_TP_RF3) >= (get_val(HIGH_TEMP_ALARM_VALUE_FOR_REFLUX_TEMP_3) + get_val(P_RTEMP_HYS)))
            {
                // 开启报警灯
                set_val(RETURN_TEMPE_HIGH_TEMP_ALARM_STATUS_3, ON);
            }

            if (get_val(R_AI_TP_RF3) < get_val(HIGH_TEMP_ALARM_VALUE_FOR_REFLUX_TEMP_3))
            {
                // 关闭报警灯
                set_val(RETURN_TEMPE_HIGH_TEMP_ALARM_STATUS_3, OFF);
            }

            //----------------------------------------------------------------------------------------------------------
            // 回流低温3 报警
            //----------------------------------------------------------------------------------------------------------
            if (get_val(R_AI_TP_RF3) <= (get_val(LOW_TEMP_ALARM_VALUE_FOR_REFLUX_TEMP_3) - get_val(P_RTEMP_HYS)))
            {
                // 一直输出报警灯
                set_val(RETURN_TEMP_LOW_TEMP_ALARM_STATUS_3, ON);
            }

            if (get_val(R_AI_TP_RF3) > get_val(LOW_TEMP_ALARM_VALUE_FOR_REFLUX_TEMP_3))
            {
                // 一直输出报警灯
                set_val(RETURN_TEMP_LOW_TEMP_ALARM_STATUS_3, OFF);
            }
        }
        else
        {
            set_val(RETURN_TEMPE_HIGH_TEMP_ALARM_STATUS_3, OFF); // 初始化 回流温度 高温报警
            set_val(RETURN_TEMP_LOW_TEMP_ALARM_STATUS_3, OFF);   // 初始化 回流温度 低温报警
            set_val(SENSOR_5_ABNORMAL_ALARM, OFF);
        }

        if (get_uval(REFLUX_TEMPERATURE) & 0x08)
        {
            //----------------------------------------------------------------------------------------------------------
            // 回流高温4 报警
            //----------------------------------------------------------------------------------------------------------
            if (get_val(R_AI_TP_RF4) >= (get_val(HIGH_TEMP_ALARM_VALUE_FOR_REFLUX_TEMP_4) + get_val(P_RTEMP_HYS)))
            {
                // 开启报警灯
                set_val(RETURN_TEMPE_HIGH_TEMP_ALARM_STATUS_4, ON);
            }

            if (get_val(R_AI_TP_RF4) < get_val(HIGH_TEMP_ALARM_VALUE_FOR_REFLUX_TEMP_4))
            {
                // 关闭报警灯
                set_val(RETURN_TEMPE_HIGH_TEMP_ALARM_STATUS_4, OFF);
            }

            //----------------------------------------------------------------------------------------------------------
            // 回流低温4 报警
            //----------------------------------------------------------------------------------------------------------
            if (get_val(R_AI_TP_RF4) <= (get_val(LOW_TEMP_ALARM_VALUE_FOR_REFLUX_TEMP_4) - get_val(P_RTEMP_HYS)))
            {
                // 开启报警灯
                set_val(RETURN_TEMP_LOW_TEMP_ALARM_STATUS_4, ON);
            }

            if (get_val(R_AI_TP_RF4) > get_val(LOW_TEMP_ALARM_VALUE_FOR_REFLUX_TEMP_4))
            {
                // 关闭报警灯
                set_val(RETURN_TEMP_LOW_TEMP_ALARM_STATUS_4, OFF);
            }
        }
        else
        {
            set_val(RETURN_TEMPE_HIGH_TEMP_ALARM_STATUS_4, OFF); // 初始化 回流温度 高温报警
            set_val(RETURN_TEMP_LOW_TEMP_ALARM_STATUS_4, OFF);   // 初始化 回流温度 低温报警
            set_val(SENSOR_6_ABNORMAL_ALARM, OFF);
        }

        //----------------------------------------------------------------------------------------------------------
        // 回流温度 报警灯
        //----------------------------------------------------------------------------------------------------------
        int ref_temp_mode_high = get_val(RETURN_TEMPE_HIGH_TEMP_ALARM_STATUS_1) +
                                 // 回流温度1 高温报警
                                 (get_val(RETURN_TEMPE_HIGH_TEMP_ALARM_STATUS_2)
                                  << 1) + // 回流温度2 高温报警
                                 (get_val(RETURN_TEMPE_HIGH_TEMP_ALARM_STATUS_3)
                                  << 2) + // 回流温度3 高温报警
                                 (get_val(RETURN_TEMPE_HIGH_TEMP_ALARM_STATUS_4)
                                  << 3); // 回流温度4 高温报警

        int ref_temp_mode_low = (get_val(RETURN_TEMP_LOW_TEMP_ALARM_STATUS_1)
                                 << 4) + // 回流温度1 低温报警
                                (get_val(RETURN_TEMP_LOW_TEMP_ALARM_STATUS_2)
                                 << 5) + // 回流温度2 低温报警
                                (get_val(RETURN_TEMP_LOW_TEMP_ALARM_STATUS_3)
                                 << 6) + // 回流温度3 低温报警
                                (get_val(RETURN_TEMP_LOW_TEMP_ALARM_STATUS_4)
                                 << 7); // 回流温度4 低温报警

        int ref_temp_mode = ref_temp_mode_high + ref_temp_mode_low;
        set_uval(RETURN_TEMP_HIGH_TEMP_ALARM_STATUS, ref_temp_mode_high);
        set_uval(RETURN_TEMP_LOW_TEMP_ALARM_STATUS, ref_temp_mode_low);
        set_uval(RETURN_TEMP_TOTAL_TEMP_ALARM_STATUS, ref_temp_mode);
        if (ref_temp_mode > 0)
        {
            if (ref_temp_mode != ref_temp_mode_old)
            {
                if (ref_temp_mode_high != ref_temp_mode_high_old)
                {
                    // 设置 回流温度 高温报警时间
                    set_uval(RETURN_TEMP_HIGH_TEMP_ALARM_TIME, get_val(P_HH5) * 60);

                    ref_temp_mode_high_old = ref_temp_mode_high;
                }

                if (ref_temp_mode_low != ref_temp_mode_low_old)
                { // 设置回流温度低温报警时间
                    set_uval(RETURN_TEMP_LOW_TEMP_ALARM_TIME, get_val(P_HH0) * 60);

                    ref_temp_mode_low_old = ref_temp_mode_low;
                }

                ref_temp_mode_old = ref_temp_mode;
            }

            // 处理回流温度高温报警
            if ((ref_temp_mode_high) > 0)
            {
                // 开启 回流温度 报警灯
                set_val(OUT_ST_REF_TEMP_ALARM, ON);
            }

            // 处理回流温度低温报警
            if ((ref_temp_mode_low) > 0)
            {
                // 开启 回流温度 报警灯
                set_val(OUT_ST_REF_TEMP_ALARM, ON);
            }
        }
        else
        {
            // 关闭 回流温度 报警灯
            set_val(OUT_ST_REF_TEMP_ALARM, OFF);
            ref_temp_mode_old = 0;
            ref_temp_mode_high_old = 0;
            ref_temp_mode_low_old = 0;

            set_uval(RETURN_TEMP_LOW_TEMP_ALARM_TIME, 0);
            set_uval(RETURN_TEMP_HIGH_TEMP_ALARM_TIME, 0);
        }

        //----------------------------------------------------------------------------------------------------------
        // 翻蛋 指示灯
        //----------------------------------------------------------------------------------------------------------
        // 接收到翻蛋信号
        if (get_uval(DETECT_FLIP_EGG) == ON)
        {
            // 翻蛋响铃赋值状态为ON
            if (get_val(EGG_FLIPPING_SIGNAL_RINGING_STATUS) == ON)
            {
                // 设置有翻蛋信号报警时间为 HH1_VAL * 60
                set_uval(EGG_FLIPPING_SIGNAL_ALARM_TIME, get_uval(P_HH1) * 60);

                // 翻蛋响铃赋值状态为OFF
                set_uval(EGG_FLIPPING_SIGNAL_RINGING_STATUS, OFF);
            }
            else
            {
                // 当翻蛋赋值状态为OFF 翻蛋响铃时间为0
                if (
                    // 有翻蛋信号报警时间 <= 0 时
                    (get_uval(EGG_FLIPPING_SIGNAL_ALARM_TIME) == OFF))
                {
                    // 开启 翻蛋报警灯
                    set_uval(OUT_ST_EGG_ALARM, ON);
                }
                else
                {
                    // 关闭 翻蛋报警
                    set_uval(OUT_ST_EGG_ALARM, OFF);
                }
            }
            set_uval(NO_EGG_FLIPPING_SIGNAL_RINGING_STATUS, ON);
        }
        else
        {
            // 在固定时间内 没有使没有翻蛋信号报警灯 初始化为ON 会触发报警
            if (get_val(NO_EGG_FLIPPING_SIGNAL_RINGING_STATUS) == ON)
            {
                // 设置 没有 翻蛋报警信号的时间为  (HH8_VAL * 60)
                set_uval(NO_EGG_FLIPPING_SIGNAL_ALARM_TIME, get_uval(P_HH8) * 60);

                // 设置 没有使没有翻蛋信号报警灯 状态为OFF 防止重复 赋值
                set_uval(NO_EGG_FLIPPING_SIGNAL_RINGING_STATUS, OFF);
            }
            // 开启 翻蛋报警灯
            set_uval(OUT_ST_EGG_ALARM, ON);

            // 翻蛋信号
            set_uval(EGG_FLIPPING_SIGNAL_RINGING_STATUS, ON);
        }

        //----------------------------------------------------------------------------------------------------------
        // CO2报警
        //----------------------------------------------------------------------------------------------------------
        if (get_uval(P_CO2_STATUS))
        {
            if ((get_uval(R_AI_CO) >= (
                                          // 二氧化碳设定值
                                          get_uval(P_AO_CO) +
                                          // 二氧化碳 最低报警值
                                          get_uval(P_CO2_AH) +
                                          // 二氧化碳 报警差值
                                          get_uval(P_CO2_HYS))))
            {
                // 设置报警灯
                set_uval(OUT_ST_CO2_ALARM, ON);

                // 一直输出报警灯
                set_uval(CARBON_DIOXIDE_WARNING_LIGHT, ON);
            }

            if (get_uval(R_AI_CO) < (get_uval(P_AO_CO) + get_uval(P_CO2_AH)))
            {
                // 清除报警灯
                set_uval(OUT_ST_CO2_ALARM, OFF);

                // 一直输出报警灯
                set_uval(CARBON_DIOXIDE_WARNING_LIGHT, OFF);
            }
        }
        else
        {
            // 清除报警灯
            set_uval(OUT_ST_CO2_ALARM, OFF);

            // 一直输出报警灯
            set_uval(CARBON_DIOXIDE_WARNING_LIGHT, OFF);
        }

        //----------------------------------------------------------------------------------------------------------
        // 风扇停止 报警
        //----------------------------------------------------------------------------------------------------------

#if 0
        // 先判断是否接通电源
        if (get_val(DETECT_FAN))
        {
            status_fan_mode = ON;
            set_uval(FAN_ALARM_TIME, 0);
            di_fan_mode = 0;
            set_uval(OUT_ST_FAN_ALARM, OFF);
            // 接收到电流互感和风机停止信号立即报警
            if ((get_val(DETECT_FAN_STOP) == ON) || (get_val(NANOPI_GPIO_PG_11) == ON))
            {
                // 接收到风扇停止后
                di_fan_mode = 1;
                set_uval(OUT_ST_FAN_ALARM, ON);
            }
        }
        else
        {
            // 立即报警
            set_uval(OUT_ST_FAN_ALARM, ON);
            // 如果是停止模式则10分钟后报警
            if ((get_val(R_STOP_RECV) == ON))
            {
                if (status_fan_mode)
                {
                    // 风机停止报警时间
                    set_uval(FAN_ALARM_TIME, get_val(P_HH7) * 60);
                    di_fan_mode = 1;
                    status_fan_mode = OFF;
                }
            }
            else
            {
                // 风机停止报警时间
                status_fan_mode = ON;
                // 接收到风扇停止后
                set_uval(FAN_ALARM_TIME, 0);
                di_fan_mode = 1;
            }
        }
#endif
        if (g_fan_status == 1)
        {
            set_uval(REFLUX_TEMPERATURE, 0x00);
        }

        if (get_val(DETECT_FAN))
        {
            if (g_fan_status == 1)
            {
                // zlog_debug(g_zlog_zc, "%-40sR_SYNC_MOTOR_CONTROL_PARA [%d]", "[thread_alarm_func]", get_val(R_SYNC_MOTOR_CONTROL_PARA));
                switch (get_val(R_SYNC_MOTOR_CONTROL_PARA))
                {
                case 2:
                case 1:
                    // zlog_debug(g_zlog_zc, "%-40sDETECT_DOOR_SWITCH 1 [%d]", "[thread_alarm_func]", get_val(DETECT_DOOR_SWITCH));
                    // zlog_debug(g_zlog_zc, "%-40sOUT_ST_DOOR_SWITCH_ALARM 1 [%d]", "[thread_alarm_func]", get_val(OUT_ST_DOOR_SWITCH_ALARM));
                    if (get_val(DETECT_DOOR_SWITCH) == OFF)
                    {
                        // 开启状态灯
                        set_uval(OUT_ST_DOOR_SWITCH_ALARM, ON);
                        // 触发报警
                        alarm_door_switch = 1;
                    }
                    else
                    {
                        // 关闭状态灯
                        set_uval(OUT_ST_DOOR_SWITCH_ALARM, OFF);
                        // 关闭报警
                        alarm_door_switch = 0;
                    }

                    set_uval(FAN_ALARM_TIME, 0);
                    di_fan_mode = 0;
                    set_uval(OUT_ST_FAN_ALARM, OFF);
                    break;
                default:
                    // zlog_debug(g_zlog_zc, "%-40sOUT_ST_DOOR_SWITCH_ALARM 2 [%d]", "[thread_alarm_func]", get_val(OUT_ST_DOOR_SWITCH_ALARM));
                    // 关闭门开关状态灯
                    set_uval(OUT_ST_DOOR_SWITCH_ALARM, OFF);
                    // 关闭门开关报警
                    alarm_door_switch = 0;

                    // 接收到风扇停止后
                    di_fan_mode = 1;
                    set_uval(OUT_ST_FAN_ALARM, ON);
                    break;
                }
            }
            else
            {
                // zlog_debug(g_zlog_zc, "%-40sOUT_ST_DOOR_SWITCH_ALARM 3 [%d]", "[thread_alarm_func]", get_val(OUT_ST_DOOR_SWITCH_ALARM));
                set_uval(FAN_ALARM_TIME, 0);
                di_fan_mode = 0;
                set_uval(OUT_ST_FAN_ALARM, OFF);

                // 关闭门开关状态灯
                set_uval(OUT_ST_DOOR_SWITCH_ALARM, OFF);
                // 关闭门开关报警
                alarm_door_switch = 0;
            }

            status_fan_mode = ON;

            // 接收到电流互感和风机停止信号立即报警
            if ((get_uval(R_SYNC_MOTOR_CONNECT_STATUS) & 0x04) && (g_fan_status == 1) ||
                (get_val(DETECT_FAN_STOP) == ON) ||
                (get_val(NANOPI_GPIO_PG_11) == ON))
            {
                // zlog_debug(g_zlog_zc, "%-40sOUT_ST_DOOR_SWITCH_ALARM 4 [%d]", "[thread_alarm_func]", get_val(OUT_ST_DOOR_SWITCH_ALARM));
                // 接收到风扇停止后
                // 如果是停止模式则10分钟后报警
                if ((get_val(R_STOP_RECV) == ON))
                {
                    if (status_fan_mode)
                    {
                        // 风机停止报警时间
                        set_uval(FAN_ALARM_TIME, get_val(P_HH7) * 60);
                        di_fan_mode = 1;
                        status_fan_mode = OFF;
                    }
                }
                else
                {
                    status_fan_mode = ON;
                    di_fan_mode = 1;
                    set_uval(OUT_ST_FAN_ALARM, ON);
                    set_uval(FAN_ALARM_TIME, 0);
                }
            }
        }
        else
        {
            // 立即报警
            set_uval(OUT_ST_FAN_ALARM, ON);

            // 关闭门开关状态灯
            set_uval(OUT_ST_DOOR_SWITCH_ALARM, OFF);
            // 关闭门开关报警
            alarm_door_switch = 0;

            // 如果是停止模式则10分钟后报警
            if ((get_val(R_STOP_RECV) == ON))
            {
                if (status_fan_mode)
                {
                    // 风机停止报警时间
                    set_uval(FAN_ALARM_TIME, get_val(P_HH7) * 60);
                    di_fan_mode = 1;
                    status_fan_mode = OFF;
                }
            }
            else
            {
                // 风机停止报警时间
                status_fan_mode = ON;
                // 接收到风扇停止后
                set_uval(FAN_ALARM_TIME, 0);
                di_fan_mode = 1;
            }
        }

        //-----------------------------------------------------------------------------------------------------------
        // 上位机清除操作 当40084为1时，触发传感器报警
        //----------------------------------------------------------------------------------------------------------
        if (get_val(SENSOR_MUTATION_STATUS) == ON)
        {
            us_pt100_val = ON;
        }
        else
        {
            us_pt100_val = OFF;
        }

        //----------------------------------------------------------------------------------------------------------
        // 传感器异常 指示灯
        //----------------------------------------------------------------------------------------------------------
        unsigned short status_ref_temp_alarm = get_val(SENSOR_1_ABNORMAL_ALARM) +
                                               (get_val(SENSOR_2_ABNORMAL_ALARM) << 1) +
                                               ((get_uval(REFLUX_TEMPERATURE) & 0x01) ? (get_val(SENSOR_3_ABNORMAL_ALARM) << 2) : 0) +
                                               ((get_uval(REFLUX_TEMPERATURE) & 0x02) ? (get_val(SENSOR_4_ABNORMAL_ALARM) << 3) : 0) +
                                               ((get_uval(REFLUX_TEMPERATURE) & 0x03) ? (get_val(SENSOR_5_ABNORMAL_ALARM) << 4) : 0) +
                                               ((get_uval(REFLUX_TEMPERATURE) & 0x08) ? (get_val(SENSOR_6_ABNORMAL_ALARM) << 5) : 0) +
                                               (get_val(SENSOR_CO2_ALARM) << 6) +
                                               (us_pt100_val << 7) +
                                               (get_val(CARBON_DIOXIDE_WARNING_LIGHT) << 8);

        set_uval(ALARM_PT100_TOTAL, status_ref_temp_alarm);

        if ((status_ref_temp_alarm) > 0)
        {
            // zlog_info(g_zlog_zc, "[报警线程]\t传感器异常 指示灯 报警 [%d]", ALARM_PV_VAL);
            set_uval(OUT_ST_SENSOR_ABNORMAL_ALARM, ON);
        }
        else
        {
            // 传感器异常
            set_uval(OUT_ST_SENSOR_ABNORMAL_ALARM, OFF);
        }

        // 报警值
        unsigned short g_us_temp_mode = 0;

        // 启动报警灯
        if ((
                //----------------------------------------------------------------------------------------------------------
                // 翻蛋 报警
                //----------------------------------------------------------------------------------------------------------
                get_val(OUT_ST_EGG_ALARM) +
                //----------------------------------------------------------------------------------------------------------
                // 高温 报警
                //----------------------------------------------------------------------------------------------------------
                get_val(OUT_ST_TP_HIGH_ALARM) +
                //----------------------------------------------------------------------------------------------------------
                // 低温 报警
                //----------------------------------------------------------------------------------------------------------
                get_val(OUT_ST_TP_LOW_ALARM) +
                //----------------------------------------------------------------------------------------------------------
                // 高湿 报警
                //----------------------------------------------------------------------------------------------------------
                get_val(OUT_ST_HM_HIGH_ALARM) +
                //----------------------------------------------------------------------------------------------------------
                // 低湿 报警
                //----------------------------------------------------------------------------------------------------------
                get_val(OUT_ST_HM_LOW_ALARM) +
                //----------------------------------------------------------------------------------------------------------
                // 回流温度 报警
                //----------------------------------------------------------------------------------------------------------
                get_val(OUT_ST_REF_TEMP_ALARM) +
                //----------------------------------------------------------------------------------------------------------
                // 二氧化碳 报警
                //----------------------------------------------------------------------------------------------------------
                get_val(OUT_ST_CO2_ALARM) +
                //----------------------------------------------------------------------------------------------------------
                // 传感器异常 报警
                //----------------------------------------------------------------------------------------------------------
                get_val(OUT_ST_SENSOR_ABNORMAL_ALARM) +
                //----------------------------------------------------------------------------------------------------------
                // 风扇 报警
                //----------------------------------------------------------------------------------------------------------
                get_val(OUT_ST_FAN_ALARM) +

                //----------------------------------------------------------------------------------------------------------
                // 门开关 报警
                //----------------------------------------------------------------------------------------------------------
                alarm_door_switch) > 0)

        {

            //----------------------------------------------------------------------------------------------------------
            // 高温 报警
            //----------------------------------------------------------------------------------------------------------
            if ((get_val(OUT_ST_TP_HIGH_ALARM)) > 0)
            {
                g_us_temp_mode |= 0x01;
            }
            else
            {
                g_us_temp_mode &= ~(0x01);
            }

            //----------------------------------------------------------------------------------------------------------
            // 风扇 报警
            //----------------------------------------------------------------------------------------------------------
            if ((get_uval(FAN_ALARM_TIME) == 0) && (di_fan_mode == 1))
            {
                g_us_temp_mode |= 0x02;
            }
            else
            {
                g_us_temp_mode &= ~(0x02);
            }

            //----------------------------------------------------------------------------------------------------------
            // 传感器异常 报警
            //----------------------------------------------------------------------------------------------------------
            if (get_val(OUT_ST_SENSOR_ABNORMAL_ALARM) > 0)
            {
                g_us_temp_mode |= 0x04;
            }
            else
            {
                g_us_temp_mode &= ~(0x04);
            }

            //----------------------------------------------------------------------------------------------------------
            // 回流温度 高温 报警
            //----------------------------------------------------------------------------------------------------------
            if ((ref_temp_mode_high > 0) && (get_uval(RETURN_TEMP_HIGH_TEMP_ALARM_TIME) == 0))
            {
                g_us_temp_mode |= 0x08;
            }
            else
            {
                g_us_temp_mode &= ~(0x08);
            }

            //----------------------------------------------------------------------------------------------------------
            // 回流温度 低温 报警
            //----------------------------------------------------------------------------------------------------------
            if ((ref_temp_mode_low > 0) && (get_uval(RETURN_TEMP_LOW_TEMP_ALARM_TIME) == 0))
            {
                g_us_temp_mode |= 0x10;
            }
            else
            {
                g_us_temp_mode &= ~(0x10);
            }

            //----------------------------------------------------------------------------------------------------------
            // 翻蛋有信号 报警
            //----------------------------------------------------------------------------------------------------------
            if ((get_val(EGG_FLIPPING_SIGNAL_RINGING_STATUS) == OFF) && (get_uval(EGG_FLIPPING_SIGNAL_ALARM_TIME) == OFF) && (get_val(P_FAN_TIME) == ON))
            {
                g_us_temp_mode |= 0x40;
            }
            else
            {
                g_us_temp_mode &= ~(0x40);
            }

            //----------------------------------------------------------------------------------------------------------
            // 翻蛋无信号 报警
            //----------------------------------------------------------------------------------------------------------
            if ((get_val(NO_EGG_FLIPPING_SIGNAL_RINGING_STATUS) == OFF) && (get_uval(NO_EGG_FLIPPING_SIGNAL_ALARM_TIME) == OFF) && (get_val(P_FAN_TIME) == ON))
            {
                // zlog_error(g_zlog_zc, "[报警线程]\t 翻蛋无信号 [%0.2x]", g_us_eggfilp_alarm_count);
                g_us_temp_mode |= 0x80;
            }
            else
            {
                g_us_temp_mode &= ~(0x80);
            }

            //----------------------------------------------------------------------------------------------------------
            // 门开关报警
            //----------------------------------------------------------------------------------------------------------

            if ((alarm_door_switch == ON) && (g_fan_status == 1))
            {
                // zlog_error(g_zlog_zc, "[报警线程]\t 翻蛋无信号 [%0.2x]", g_us_eggfilp_alarm_count);
                g_us_temp_mode |= 0x100;
            }
            else
            {
                g_us_temp_mode &= ~(0x100);
            }
        }

        // 将报警状态
        set_uval(ALARM_MODULE, g_us_temp_mode);
        //----------------------------------------------------------------------------------------------------------
        // 报警灯 常亮 低温报警 高湿报警 低湿报警 翻蛋报警 风门报警
        //----------------------------------------------------------------------------------------------------------
        int v_i_alarm_status = get_val(LOW_TEMP_ALARM_STATUS) + get_val(HIGH_HUMI_ALARM_STATUS) + get_val(LOW_HUMI_ALARM_STATUS) + get_val(OUT_ST_EGG_ALARM) + get_val(OUT_ST_FAN_ALARM) + get_val(OUT_ST_REF_TEMP_ALARM);

        //----------------------------------------------------------------------------------------------------------
        // 报警灯 闪烁 响铃
        //----------------------------------------------------------------------------------------------------------
        // zlog_error(g_zlog_zc, "[报警线程]\t 闪烁报警值 [%0.2x]", g_us_temp_mode);
        if ((g_us_temp_mode > 0))
        {
            // 消警时间为0 时 ，触发 报警灯闪烁
            if (get_uval(ALARM_SUPPRESSION_TIME) == 0)
            {
                // 报警灯闪烁
                control_warn_light(!get_val(CONTROL_WARN_LIGHT));
                control_electric_light(OFF);
            }
            else
            {
                control_warn_light(ON);
                control_electric_light(ON);
            }
        }

        // zlog_error(g_zlog_zc, "[报警线程]\t 常量报警值 [%0.2x]", v_i_alarm_status);
        else if (v_i_alarm_status > 0)
        {
            // 报警灯常亮
            control_warn_light(ON);
            control_electric_light(ON);
        }

        //----------------------------------------------------------------------------------------------------------
        // 报警灯 停止
        //----------------------------------------------------------------------------------------------------------
        else if ((g_us_temp_mode == 0) && ((get_val(LOW_TEMP_ALARM_STATUS) + get_val(HIGH_HUMI_ALARM_STATUS) + get_val(LOW_HUMI_ALARM_STATUS) + get_val(OUT_ST_EGG_ALARM) + get_val(OUT_ST_FAN_ALARM) + get_val(OUT_ST_REF_TEMP_ALARM)) == 0))
        {
            control_warn_light(OFF);
            control_electric_light(ON);
        }

        wait_ms(1000);
    }

    return NULL;
}

// // 主线程
// void *thread_pid_func(void *pv)
// {
//     while (1)
//     {
//         // 温度PID
//         if ((get_val(R_AI_TP_MAIN) >= (get_val(P_AO_TP_MAIN) - get_x10_val(P_TEMP_P) / 2)) && (get_val(R_AI_TP_MAIN) <= (get_val(P_AO_TP_MAIN) + get_x10_val(P_TEMP_P) / 2)))
//         {
//             pid_temp_init();
//         }

//         // 湿度PID
//         if ((get_val(R_AI_HM) >= (get_val(P_AO_HM) - get_x10_val(P_HUM_P) / 2)) && (get_val(R_AI_HM) <= (get_val(P_AO_HM) + get_x10_val(P_HUM_P) / 2)))
//         {
//             pid_humi_init();
//         }

//         wait_ms(250);
//     }
// }

// 主线程
void *thread_main_func(void *pv)
{
    int run_mode_time = 0;
    while (1)
    {
        //----------------------------------------------------------------------------------------------------------
        // 实时执行区域
        //----------------------------------------------------------------------------------------------------------
        // 时间函数
        run_sys_time();

        //----------------------------------------------------------------------------------------------------------
        // 启动模式-初始化
        //----------------------------------------------------------------------------------------------------------
        if (get_val(R_START_SEND) == ON)
        {
            zlog_info(g_zlog_zc, "%-40s初始化启动模式!", "[main]");

            // 开启启动模式
            set_run_mode(STATUS_START_RECV);

            // 初始化PID参数
            set_time(PID_TEMP_SUM, 0);
            set_time(PID_HUMI_SUM, 0);
            set_time(PID_PERIOD_COUNT_MAIN_HEATER, 0);
            set_time(PID_PERIOD_COUNT_HUMI, 0);
            set_val(PID_TEMP_MODE_STATUS, 0);

            // 设置回流温度的初始化为0
            set_uval(RETURN_TEMP_1_HOT_WATER_OPERAT_INIT, 0);
            set_uval(RETURN_TEMP_2_HOT_WATER_OPERAT_INIT, 0);
            set_uval(RETURN_TEMP_3_HOT_WATER_OPERAT_INIT, 0);
            set_uval(RETURN_TEMP_4_HOT_WATER_OPERAT_INIT, 0);
            set_uval(REFLOW_TEMP_1_COLD_WATER_OPERAT_INIT, 0);
            set_uval(REFLOW_TEMP_2_COLD_WATER_OPERAT_INIT, 0);
            set_uval(REFLOW_TEMP_3_COLD_WATER_OPERAT_INIT, 0);
            set_uval(REFLOW_TEMP_4_COLD_WATER_OPERAT_INIT, 0);

            // 设置回流温度的延迟为30秒
            set_uval(REFLUX_TEMP_1_DELAY_TIME, 30);
            set_uval(REFLUX_TEMP_2_DELAY_TIME, 30);
            set_uval(REFLUX_TEMP_3_DELAY_TIME, 30);
            set_uval(REFLUX_TEMP_4_DELAY_TIME, 30);

            // 风机关闭
            // control_fan(OFF);

            // 主加热-关闭
            control_main_heat(OFF);

            // 关闭辅助加热
            control_aux_heat(OFF);

            // 水冷关闭
            control_cool(OFF);

            // 加湿关闭
            control_spray(OFF);

            // 回流温度关闭
            control_cool2(1, OFF);
            control_cool2(2, OFF);
            control_cool2(3, OFF);
            control_cool2(4, OFF);

            // 鼓风机关闭
            // control_blower(OFF);

            // 风门关闭
            control_damper(1, OFF, 0);
            control_damper(2, OFF, 0);

            // 开启入孵计时
            set_val(R_RUN_SECOND_STATUS, ON);

            // 预热状态为1
            set_uval(R_PRE_STATUS, 1);

            // 设置预热时间为0
            set_time(R_PRE_START_TIME_HIGH, 0);
            set_time(R_PRE_STOP_TIME_HIGH, 0);
        }

        //----------------------------------------------------------------------------------------------------------
        // 启动模式-运行
        //----------------------------------------------------------------------------------------------------------
        if (get_val(R_START_RECV) == ON)
        {
            zlog_info(g_zlog_zc, "%-40s执行启动模式!", "[main]");

            // 开启运行灯
            control_run_light(ON);

            // 开启入孵计时
            set_val(R_RUN_SECOND_STATUS, ON);

            // 预热状态为1
            set_uval(R_PRE_STATUS, 1);

            // 更新配置
            update_set_para();

            // 启动风机
            if (g_fan_status == 1)
            {
                if (get_val(DETECT_DOOR_SWITCH) == ON)
                {
                    // 门开有信号
                    if ((get_val(R_SYNC_MOTOR_CONTROL_PARA) == 3) && (get_val(S_SYNC_MOTOR_CONTROL_CMD) == 0) && ((get_val(R_SYNC_MOTOR_CONTROL_FAULT) & 0x01) == 0))
                    {
                        zlog_info(g_zlog_zc, "%-40s启动电机", "[main]");
                        set_val(S_SYNC_MOTOR_CONTROL_CMD, 0x01);
                    }
                }
                else
                {
                    // 门开关没有信号
                    if ((get_val(R_SYNC_MOTOR_CONTROL_PARA) != 3) && (get_val(S_SYNC_MOTOR_CONTROL_CMD) == 0) && ((get_val(R_SYNC_MOTOR_CONTROL_FAULT) & 0x02) == 0))
                    {
                        zlog_info(g_zlog_zc, "%-40s关闭电机", "[main]");
                        set_val(S_SYNC_MOTOR_CONTROL_CMD, 0x02);
                    }
                }
            }

            // 启动风机
            control_fan(ON);

            // 启动鼓风机
            control_blower(ON);

            // 主温度
            logic_process_temp_func();

            // 湿度
            logic_process_humi_func();

            // 风门
            damper_control_func();

            // 回流温度
            normal_logic_process_retemp1_func();
            normal_logic_process_retemp2_func();
            normal_logic_process_retemp3_func();
            normal_logic_process_retemp4_func();
        }

        //----------------------------------------------------------------------------------------------------------
        // 停止模式-初始化
        //----------------------------------------------------------------------------------------------------------
        if (get_val(R_STOP_SEND) == ON)
        {
            zlog_info(g_zlog_zc, "%-40s初始化停止模式!", "[main]");

            // 开启停止模式
            set_run_mode(STATUS_STOP_RECV);

            // 开启入孵计时
            set_val(R_RUN_SECOND_STATUS, ON);

            // 预热状态为1
            set_uval(R_PRE_STATUS, 1);

            // 设置预热时间为0
            set_time(R_PRE_START_TIME_HIGH, 0);
            set_time(R_PRE_STOP_TIME_HIGH, 0);
        }

        //----------------------------------------------------------------------------------------------------------
        // 停止模式-运行
        //----------------------------------------------------------------------------------------------------------
        if (get_val(R_STOP_RECV) == ON)
        {
            zlog_info(g_zlog_zc, "%-40s执行停止模式!", "[main]");

            // 开启运行灯
            control_run_light(OFF);

            // 开启入孵计时
            set_val(R_RUN_SECOND_STATUS, ON);

            // 预热状态为1
            set_uval(R_PRE_STATUS, 1);

            // 更新配置
            update_set_para();

            // 风机关闭
            // control_fan(OFF);

            if (g_fan_status == 1)
            {
                // 门开关没有信号
                if ((get_val(R_SYNC_MOTOR_CONTROL_PARA) != 3) && (get_val(S_SYNC_MOTOR_CONTROL_CMD) == 0) && ((get_val(R_SYNC_MOTOR_CONTROL_FAULT) & 0x02) == 0))
                {
                    zlog_info(g_zlog_zc, "%-40s关闭电机", "[main]");
                    set_val(S_SYNC_MOTOR_CONTROL_CMD, 0x02);
                }
            }

            // 启动风机
            control_fan(OFF);

            // 主加热-关闭
            control_main_heat(OFF);

            // 关闭辅助加热
            control_aux_heat(OFF);

            // 水冷关闭
            control_cool(OFF);

            // 加湿关闭
            control_spray(OFF);

            // 回流温度关闭
            control_cool2(1, OFF);
            control_cool2(2, OFF);
            control_cool2(3, OFF);
            control_cool2(4, OFF);

            // 冷热水切换阀关闭
            control_cold_hot_water(OFF);

            // 鼓风机关闭
            control_blower(OFF);

            // 风门关闭
            control_damper(1, OFF, 0);
            control_damper(2, OFF, 0);
        }

        //----------------------------------------------------------------------------------------------------------
        // 预热模式-初始化
        //----------------------------------------------------------------------------------------------------------
        // 当预热开启时间和停止时间不为0
        if ((get_time(R_PRE_START_TIME_HIGH) != 0) && (get_time(R_PRE_STOP_TIME_HIGH) != 0))
        {
            // 预热开启时间小于预热停止时间
            if ((get_time(R_PRE_START_TIME_HIGH) < get_time(R_PRE_STOP_TIME_HIGH)) &&
                // 预热时间不能超过2天
                ((get_time(R_PRE_STOP_TIME_HIGH) - get_time(R_PRE_START_TIME_HIGH)) <= TWO_DAY_SECONDS))
            {
                // 获取系统时间
                unsigned int sys_now_time = time(NULL);
                // 当预热时间大于等于系统时间，并系统时间小于停止时间 进入到预热时间
                if ((sys_now_time >= get_time(R_PRE_START_TIME_HIGH)) && (sys_now_time <= get_time(R_PRE_STOP_TIME_HIGH)))
                {
                    zlog_info(g_zlog_zc, "%-40s正在进行预热模式!", "[main]");
                    if (get_uval(R_PRE_STATUS))
                    {
                        // 开启预热模式
                        set_run_mode(STARTS_PRE_RECV);

                        // 初始化PID参数
                        set_time(PID_TEMP_SUM, 0);
                        set_time(PID_HUMI_SUM, 0);
                        set_time(PID_PERIOD_COUNT_MAIN_HEATER, 0);
                        set_time(PID_PERIOD_COUNT_HUMI, 0);
                        set_val(PID_TEMP_MODE_STATUS, 0);

                        // 设置回流温度的初始化为0
                        set_uval(RETURN_TEMP_1_HOT_WATER_OPERAT_INIT, 0);
                        set_uval(RETURN_TEMP_2_HOT_WATER_OPERAT_INIT, 0);
                        set_uval(RETURN_TEMP_3_HOT_WATER_OPERAT_INIT, 0);
                        set_uval(RETURN_TEMP_4_HOT_WATER_OPERAT_INIT, 0);
                        set_uval(REFLOW_TEMP_1_COLD_WATER_OPERAT_INIT, 0);
                        set_uval(REFLOW_TEMP_2_COLD_WATER_OPERAT_INIT, 0);
                        set_uval(REFLOW_TEMP_3_COLD_WATER_OPERAT_INIT, 0);
                        set_uval(REFLOW_TEMP_4_COLD_WATER_OPERAT_INIT, 0);

                        // 设置回流温度的延迟为30秒
                        set_uval(REFLUX_TEMP_1_DELAY_TIME, 30);
                        set_uval(REFLUX_TEMP_2_DELAY_TIME, 30);
                        set_uval(REFLUX_TEMP_3_DELAY_TIME, 30);
                        set_uval(REFLUX_TEMP_4_DELAY_TIME, 30);
                        set_uval(R_PRE_STATUS, 0);

                        // 风机关闭
                        // control_fan(OFF);

                        // 主加热-关闭
                        control_main_heat(OFF);

                        // 关闭辅助加热
                        control_aux_heat(OFF);

                        // 水冷关闭
                        control_cool(OFF);

                        // 加湿关闭
                        control_spray(OFF);

                        // 回流温度关闭
                        control_cool2(1, OFF);
                        control_cool2(2, OFF);
                        control_cool2(3, OFF);
                        control_cool2(4, OFF);

                        // 鼓风机关闭
                        // control_blower(OFF);

                        // 风门关闭
                        control_damper(1, OFF, 0);
                        control_damper(2, OFF, 0);
                    }

                    // 停止入孵天数
                    set_val(R_RUN_SECOND_STATUS, OFF);

                    if (g_fan_status == 1)
                    {
                        if (get_val(DETECT_DOOR_SWITCH) == ON)
                        {
                            // 门开有信号
                            if ((get_val(R_SYNC_MOTOR_CONTROL_PARA) == 3) && (get_val(S_SYNC_MOTOR_CONTROL_CMD) == 0) && ((get_val(R_SYNC_MOTOR_CONTROL_FAULT) & 0x01) == 0))
                            {
                                zlog_info(g_zlog_zc, "%-40s启动电机", "[main]");
                                set_val(S_SYNC_MOTOR_CONTROL_CMD, 0x01);
                            }
                        }
                        else
                        {
                            // 门开关没有信号
                            if ((get_val(R_SYNC_MOTOR_CONTROL_PARA) != 3) && (get_val(S_SYNC_MOTOR_CONTROL_CMD) == 0) && ((get_val(R_SYNC_MOTOR_CONTROL_FAULT) & 0x02) == 0))
                            {
                                zlog_info(g_zlog_zc, "%-40s关闭电机", "[main]");
                                set_val(S_SYNC_MOTOR_CONTROL_CMD, 0x02);
                            }
                        }
                    }

                    // 启动风机
                    control_fan(ON);

                    // 开启运行灯
                    control_run_light(ON);

                    // 启动鼓风机
                    control_blower(ON);

                    // 设置预热参数
                    preheat_set_para();

                    // 主温度
                    logic_process_temp_func();

                    // 湿度
                    logic_process_humi_func();

                    // 风门
                    damper_control_func();

                    // 回流温度
                    pre_logic_process_retemp1_func();
                    pre_logic_process_retemp2_func();
                    pre_logic_process_retemp3_func();
                    pre_logic_process_retemp4_func();
                }

                // 系统时间大于预热停止模式，自动退出预热模式，进行入到启动模式
                else if (sys_now_time > get_time(R_PRE_STOP_TIME_HIGH))
                {
                    // 停止入孵时间计时
                    set_val(R_RUN_SECOND_STATUS, OFF);

                    // 清空入孵时间
                    set_time(R_RUN_SECOND_HIGH, 0);

                    // 获取运行秒数
                    unsigned int sys_second_time = get_time(R_RUN_SECOND_HIGH);

                    // 获取运行时间的天数和小数
                    struct Time sys_time = timer(sys_second_time);
                    unsigned short para_time = sys_time.days * 100 + sys_time.hours;

                    // 设置天数和小时
                    set_uval(R_RUN_DAY, para_time);

                    // 获取系统时间
                    unsigned int sys_new_time = time(NULL);

                    // 获取入孵时间
                    unsigned int init_new_time = sys_new_time - sys_second_time;

                    // 设置入孵时间
                    set_time(R_INIT_TIME_HIGH, init_new_time);

                    // 开始入孵时间计时
                    set_val(R_RUN_SECOND_STATUS, ON);

                    // 开启启动模式
                    set_run_mode(STATUS_START_SEND);

                    // 设置预热时间为0
                    set_time(R_PRE_START_TIME_HIGH, 0);
                    set_time(R_PRE_STOP_TIME_HIGH, 0);
                }

                // 突然断开进入到停止模式
                // else
                // {
                //     // 开启停止模式
                //     set_run_mode(STATUS_STOP_SENT);

                //     // 开启入孵计时
                //     set_val(R_RUN_SECOND_STATUS, ON);

                //     // 设置预热时间为0
                //     set_time(R_PRE_START_TIME_HIGH, 0);
                //     set_time(R_PRE_STOP_TIME_HIGH, 0);
                // }
            }

            // 当预热开启时间等于预热停止时间，则进入到停止模式
            else if (get_time(R_PRE_START_TIME_HIGH) == get_time(R_PRE_STOP_TIME_HIGH))
            {
                // 开启停止模式
                set_run_mode(STATUS_STOP_SENT);

                // 开启入孵计时
                set_val(R_RUN_SECOND_STATUS, ON);

                // 设置预热时间为0
                set_time(R_PRE_START_TIME_HIGH, 0);
                set_time(R_PRE_STOP_TIME_HIGH, 0);
            }

            // 其他条件也进入到停止模式
            // else
            // {
            //     // 开启停止模式
            //     set_run_mode(STATUS_STOP_SENT);

            //     // 设置预热时间为0
            //     set_time(R_PRE_START_TIME_HIGH, 0);
            //     set_time(R_PRE_STOP_TIME_HIGH, 0);
            // }
        }

        //----------------------------------------------------------------------------------------------------------
        // 消警模式-启动
        //----------------------------------------------------------------------------------------------------------
        if ((get_val(R_CLOSE_ALA_SEND) == ON) && (get_val(R_CLOSE_ALA_RECV) == OFF))
        {
            zlog_info(g_zlog_zc, "%-40s消警模式!", "[main]");

            set_val(R_CLOSE_ALA_SEND, ON);
            set_val(R_CLOSE_ALA_RECV, ON);

            // 设置消警时间
            set_uval(ALARM_SUPPRESSION_TIME, get_uval(R_CLOSE_ALA_TIME));
        }

        //----------------------------------------------------------------------------------------------------------
        // 消警模式-时间触发解除
        //----------------------------------------------------------------------------------------------------------
        if ((get_val(R_CLOSE_ALA_SEND) == ON) && (get_val(R_CLOSE_ALA_RECV) == ON))
        {
            // 消警时间为0时，退出消警模式
            if (get_uval(ALARM_SUPPRESSION_TIME) == 0)
            {
                set_val(R_CLOSE_ALA_SEND, OFF);
                set_val(R_CLOSE_ALA_RECV, OFF);
            }
        }

        //----------------------------------------------------------------------------------------------------------
        // 消警模式-点击触发解除
        //----------------------------------------------------------------------------------------------------------
        if ((get_val(R_CLOSE_ALA_SEND) == OFF) && (get_val(R_CLOSE_ALA_RECV) == ON))
        {
            // 手动清除消警模式
            set_uval(ALARM_SUPPRESSION_TIME, 0);
            set_val(R_CLOSE_ALA_SEND, OFF);
            set_val(R_CLOSE_ALA_RECV, OFF);
        }

        //----------------------------------------------------------------------------------------------------------
        // 初始化模式
        //----------------------------------------------------------------------------------------------------------
        if (get_val(R_INIT_SEND) == ON)
        {
            // 停止初始化
            set_val(R_INIT_SEND, OFF);

            zlog_info(g_zlog_zc, "%-40s初始化模式!", "[main]");

            // 停止入孵时间计时
            set_val(R_RUN_SECOND_STATUS, OFF);

            // 清空入孵时间
            set_time(R_RUN_SECOND_HIGH, 0);

            // 获取运行秒数
            unsigned int sys_second_time = get_time(R_RUN_SECOND_HIGH);

            // 设置天数和小时
            set_uval(R_RUN_DAY, 0);

            // 获取系统时间
            unsigned int sys_new_time = time(NULL);

            // 获取入孵时间
            unsigned int init_new_time = sys_new_time - sys_second_time;

            // 设置入孵时间
            set_time(R_INIT_TIME_HIGH, init_new_time);

            // 开始入孵时间计时
            set_val(R_RUN_SECOND_STATUS, ON);

            // 初始化 翻蛋次数（朱工在2025年6月22日）提出需要需求。
            set_val(R_AI_TP_EGG, 0);
        }

        //----------------------------------------------------------------------------------------------------------
        // 校准模式-启动
        //----------------------------------------------------------------------------------------------------------
        if ((get_val(R_CHECK_SEND) == ON) && (get_val(R_CHECK_RECV) == OFF))
        {
            zlog_info(g_zlog_zc, "%-40s校准模式!", "[main]");

            // 备份运行模式
            set_uval(BACKUP_OPERATION_MODE, get_run_mode(1));

            // 备份预热初始化
            set_uval(BACKUP_PRE_STATUS, get_uval(R_PRE_STATUS));

            // 备份预热开始时间
            set_time(BACKUP_PREHEATING_START_TIME_HIGH, get_time(R_PRE_START_TIME_HIGH));

            // 备份预热结束时间
            set_time(BACKUP_PREHEATING_SHUTDOWN_TIME_HIGH, get_time(R_PRE_STOP_TIME_HIGH));

            // 运行模式
            set_run_mode(STATUS_CHECK_SEND | STATUS_CHECK_REVV);

            if (g_fan_status == 1)
            {
                if (get_val(DETECT_DOOR_SWITCH) == ON)
                {
                    // 门开有信号
                    if ((get_val(R_SYNC_MOTOR_CONTROL_PARA) == 3) && (get_val(S_SYNC_MOTOR_CONTROL_CMD) == 0) && ((get_val(R_SYNC_MOTOR_CONTROL_FAULT) & 0x01) == 0))
                    {
                        zlog_info(g_zlog_zc, "%-40s启动电机", "[main]");
                        set_val(S_SYNC_MOTOR_CONTROL_CMD, 0x01);
                    }
                }
                else
                {
                    // 门开关没有信号
                    if ((get_val(R_SYNC_MOTOR_CONTROL_PARA) != 3) && (get_val(S_SYNC_MOTOR_CONTROL_CMD) == 0) && ((get_val(R_SYNC_MOTOR_CONTROL_FAULT) & 0x02) == 0))
                    {
                        zlog_info(g_zlog_zc, "%-40s关闭电机", "[main]");
                        set_val(S_SYNC_MOTOR_CONTROL_CMD, 0x02);
                    }
                }
            }

            // 启动风机
            control_fan(ON);

            // 主加热-关闭
            control_main_heat(OFF);

            // 关闭辅助加热
            control_aux_heat(OFF);

            // 水冷关闭
            control_cool(OFF);

            // 加湿关闭
            control_spray(OFF);

            // 回流温度关闭
            control_cool2(1, OFF);
            control_cool2(2, OFF);
            control_cool2(3, OFF);
            control_cool2(4, OFF);

            // 冷热水切换阀关闭
            control_cold_hot_water(OFF);

            // 风门关闭
            control_damper(1, OFF, 0);
            control_damper(2, OFF, 0);

            // 设置预热时间为0
            set_time(R_PRE_START_TIME_HIGH, 0);
            set_time(R_PRE_STOP_TIME_HIGH, 0);

            // 设置校准时间
            set_uval(CALIBRATION_RUN_TIME, get_uval(P_HH9) * 60);
        }

        //----------------------------------------------------------------------------------------------------------
        // 校准模式-时间触发解除
        //----------------------------------------------------------------------------------------------------------
        if ((get_val(R_CHECK_SEND) == ON) && (get_val(R_CHECK_RECV) == ON))
        {
            if (get_uval(CALIBRATION_RUN_TIME) == 0)
            {
                // 恢复运行模式
                set_run_mode(get_uval(BACKUP_OPERATION_MODE));

                // 恢复预热初始化
                set_uval(R_PRE_STATUS, get_uval(BACKUP_PRE_STATUS));

                // 恢复预热开始时间
                set_time(R_PRE_START_TIME_HIGH, get_time(BACKUP_PREHEATING_START_TIME_HIGH));

                // 恢复预热结束时间
                set_time(R_PRE_STOP_TIME_HIGH, get_time(BACKUP_PREHEATING_SHUTDOWN_TIME_HIGH));
            }
        }

        //----------------------------------------------------------------------------------------------------------
        // 校准模式-点击触发解除
        //----------------------------------------------------------------------------------------------------------
        if ((get_val(R_CHECK_SEND) == OFF) && (get_val(R_CHECK_RECV) == ON))
        {
            // 恢复运行模式
            set_run_mode(get_uval(BACKUP_OPERATION_MODE));

            // 恢复预热初始化
            set_uval(R_PRE_STATUS, get_uval(BACKUP_PRE_STATUS));

            // 恢复预热开始时间
            set_time(R_PRE_START_TIME_HIGH, get_time(BACKUP_PREHEATING_START_TIME_HIGH));

            // 恢复预热结束时间
            set_time(R_PRE_STOP_TIME_HIGH, get_time(BACKUP_PREHEATING_SHUTDOWN_TIME_HIGH));

            // 清空校准时间为0
            set_uval(CALIBRATION_RUN_TIME, 0);
        }

        // 默认模式为停止模式
        unsigned short run_mode_value = get_run_mode(0);
        if (!run_mode_value)
        {
            // 进入停止模式
            set_run_mode(STATUS_STOP_SENT);
        }
        run_mode_time = 0;

        //----------------------------------------------------------------------------------------------------------
        // 测试控制模式
        //----------------------------------------------------------------------------------------------------------
        test_control_mode();

        //----------------------------------------------------------------------------------------------------------
        // LED灯控制函数
        //----------------------------------------------------------------------------------------------------------
        direction_func();

        // 延迟为250毫秒
        wait_ms(250);
    }
}

int main(int argc, char *argv[])
{
    // 防止启动文件异常和日志异常
    int reboot_state = 0;

    // 初始化
    create_directories();
    if (check_rc_local_contains_update() != 1)
    {
        ++reboot_state;
        write_rc_local();
    }

    if (check_update_script_contains_jlog() != 1)
    {
        ++reboot_state;
        write_update_script();
    }

    if (check_jlog_contains_jincubator() != 1)
    {
        ++reboot_state;
        write_jlog_config();
    }

    if (reboot_state > 0)
    {
        // 删除 日志
        remove_log_file();
        system("/sbin/reboot");
    }

    //----------------------------------------------------------------------------------------------------------
    // 定义变量
    //----------------------------------------------------------------------------------------------------------
    // 定义返回值
    int i_ret = 0;

    // 定义申请共享内存字节数
    unsigned short shm_addr_size = 0;

    // 定义共享内存
    jshmem_t shm_input_addr = {0};
    jshmem_t shm_output_addr = {0};

    //----------------------------------------------------------------------------------------------------------
    // 解析程序名称
    //----------------------------------------------------------------------------------------------------------
    char date_str[256];
    print_current_time(date_str, sizeof(date_str));
    parse_file_path(argv[0], &g_filename);
    if (strlen(g_filename) > 256)
    {
        fprintf(stderr, "%s %-40s程序名称已超过256个字节\n", date_str, "[main]");
        return -1;
    }

    print_current_time(date_str, sizeof(date_str));
    if (g_filename == NULL)
    {
        fprintf(stderr, "%s %-40s程序名称为空\n", date_str, "[main]");
        return -1;
    }

    sprintf(g_filenaem_ini, "%s.ini", g_filename);
    print_current_time(date_str, sizeof(date_str));
    fprintf(stdout, "%s %-40s程序名称:%s 程序配置:%s\n", date_str, "[main]", g_filename, g_filenaem_ini);

    //----------------------------------------------------------------------------------------------------------
    // 读取INI配置
    //----------------------------------------------------------------------------------------------------------
    init_ini_file(g_filenaem_ini, &g_st_jincubator);

    //----------------------------------------------------------------------------------------------------------
    // ZLOG日志初始化
    //----------------------------------------------------------------------------------------------------------
    print_current_time(date_str, sizeof(date_str));
    fprintf(stdout, "%s %-40sZLOG日志:%s\n", date_str, "[main]", g_st_jincubator.conf_zlog_file);
    i_ret = init_zlog_file(g_st_jincubator.conf_zlog_file, g_filename);
    if (i_ret != 0)
    {
        fprintf(stderr, "%-40sZLOG日志初始化失败!\n", "[main]");
        return -1;
    }

    //----------------------------------------------------------------------------------------------------------
    // 共享内存初始化
    //----------------------------------------------------------------------------------------------------------
    // 获取申请字节数
    shm_addr_size = g_st_jincubator.shmem_record_len * g_st_jincubator.shmem_record_num;

    // 共享内存输出
    i_ret = j_shmem_init(&shm_output_addr, g_st_jincubator.shmem_output_key, shm_addr_size);
    if (i_ret != 0)
    {
        zlog_error(g_zlog_zc, "%-40s申请共享内存输出地址失败!", "[main]");
        return -1;
    }

    shm_out = shm_output_addr.uptr_shmem_addr;
    zlog_info(g_zlog_zc, "%-40s申请共享内存输出地址成功, 内存地址：%p", "[main]", shm_out);

    // 共享内存输入
    i_ret = j_shmem_init(&shm_input_addr, g_st_jincubator.shmem_input_key, shm_addr_size);
    if (i_ret != 0)
    {
        zlog_error(g_zlog_zc, "%-40s申请共享内存输入地址失败!", "[main]");
        return -1;
    }

    shm_old = shm_input_addr.uptr_shmem_addr;
    zlog_info(g_zlog_zc, "%-40s申请共享内存输入地址成功, 内存地址：%p", "[main]", shm_old);

    //----------------------------------------------------------------------------------------------------------
    // 初始化参数
    //----------------------------------------------------------------------------------------------------------
    init_para();
    zlog_info(g_zlog_zc, "%-40s参数初始化成功!", "[main]");

    wait_ms(200);
    //----------------------------------------------------------------------------------------------------------
    // 定时器
    //----------------------------------------------------------------------------------------------------------
    int timer_value = 100; // 定时器的时间间隔为1秒
    set_custom_timer_ms(timer_value, timer_callback_func);
    wait_ms(200);

    //----------------------------------------------------------------------------------------------------------
    // 备份线程
    //----------------------------------------------------------------------------------------------------------
    pthread_t sync_client_thread;
    pthread_create(&sync_client_thread, NULL, sysn_thread_func, NULL);
    wait_ms(200);
    //----------------------------------------------------------------------------------------------------------
    // 数据采集线程-PT100
    //----------------------------------------------------------------------------------------------------------
    pthread_t data_collection_thread;
    pthread_create(&data_collection_thread, NULL, data_collection_pt100_func, NULL);
    wait_ms(200);
    //----------------------------------------------------------------------------------------------------------
    // 数据采集线程-CO2
    //----------------------------------------------------------------------------------------------------------
    pthread_t co2_collection_thread;
    pthread_create(&co2_collection_thread, NULL, data_collection_co2_func, NULL);
    wait_ms(200);

    //----------------------------------------------------------------------------------------------------------
    // 同步电机线程
    //----------------------------------------------------------------------------------------------------------
    pthread_t synchronous_motor_thread;
    pthread_create(&synchronous_motor_thread, NULL, synchronous_motor_func, NULL);
    wait_ms(200);

    //----------------------------------------------------------------------------------------------------------
    // 翻蛋线程
    //----------------------------------------------------------------------------------------------------------
    pthread_t flip_egg_thread;
    pthread_create(&flip_egg_thread, NULL, flip_egg_func, NULL);
    wait_ms(200);
    //----------------------------------------------------------------------------------------------------------
    // 报警线程
    //----------------------------------------------------------------------------------------------------------
    pthread_t alarm_thread;
    pthread_create(&alarm_thread, NULL, thread_alarm_func, NULL);
    wait_ms(200);
    //----------------------------------------------------------------------------------------------------------
    // PID线程
    //----------------------------------------------------------------------------------------------------------
    // pthread_t pid_thread;
    // pthread_create(&pid_thread, NULL, thread_pid_func, NULL);

    //----------------------------------------------------------------------------------------------------------
    // 主线程
    //----------------------------------------------------------------------------------------------------------
    pthread_t main_thread;
    pthread_create(&main_thread, NULL, thread_main_func, NULL);
    wait_ms(1000);

    // 备份线程
    pthread_join(sync_client_thread, NULL);
    // 数据采集线程
    pthread_join(data_collection_thread, NULL);
    // 二氧化碳采集线程
    pthread_join(co2_collection_thread, NULL);
    // 同步电机线程
    pthread_join(synchronous_motor_thread, NULL);
    // 翻蛋线程
    pthread_join(flip_egg_thread, NULL);
    // 报警线程
    pthread_join(alarm_thread, NULL);
    // PID线程
    // pthread_join(pid_thread, NULL);
    // 主线线程
    pthread_join(main_thread, NULL);
    // 注销ZLOG
    zlog_fini();
    return 0;
}
