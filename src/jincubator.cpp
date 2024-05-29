#include "jincubator.h"

//----------------------------------------------------------------------------------------------------------
// 预定义
//----------------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------------
// 全局变量-ZLOG变量
//----------------------------------------------------------------------------------------------------------
static zlog_category_t *g_zlog_zc = NULL;

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

short get_X10_val(unsigned short index)
{
    return pidshm2short(&shm_out[index]);
}

short get_E10_val(unsigned short index)
{
    return shm2shortpid(&shm_out[index]);
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

    strcpy(st_incubator->serial_device, ini["GENERAL SETTING"]["SERIAL_DEVICE"]);
    fprintf(stdout, "%s %-40s串口设备:%s\n", date_str, "[init_ini_file]", st_incubator->serial_device);

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
}

//----------------------------------------------------------------------------------------------------------
// ZLOG日志初始化
//----------------------------------------------------------------------------------------------------------
int init_zlog_file(zlog_category_t **zlog_zc, const char *conf_zlog_file, const char *process_name)
{
    
    char date_str[256];
    print_current_time(date_str, sizeof(date_str));
    fprintf(stderr, "%s %-40s配置文件: %s, 程序名称：%s\n",date_str, "[init_zlog_file]", conf_zlog_file, process_name);
    int i_ret = zlog_init(conf_zlog_file);
    if (i_ret)
    {
        fprintf(stderr, "%s %-40s打开日志配置文件: ./jlog.conf失败!\n",date_str, "[init_zlog_file]");
        return -1;
    }

    fprintf(stdout, "%s %-40s打开日志配置文件: ./jlog.conf成功!\n",date_str, "[init_zlog_file]");
    *zlog_zc = zlog_get_category(process_name);
    if (!(*zlog_zc))
    {
        fprintf(stderr, "%s %-40s日志初始化失败!\n",date_str, "[init_zlog_file]");
        zlog_fini();
        return -2;
    }
    zlog_info(*zlog_zc, "%-40s日志初始化成功!", "[init_zlog_file]");
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
void control_main_heat(int mode, int status)
{
    // 测试模式优先
    if (mode)
    {
        set_val(CONTROL_MAIN_HEAT, status);
    }
    else
    {
        set_val(CONTROL_MAIN_HEAT, status);
    }
}

//----------------------------------------------------------------------------------------------------------
// 辅助加热控制
//----------------------------------------------------------------------------------------------------------
void control_aux_heat(int mode, int status)
{
    if (mode)
    {
        set_val(CONTROL_AUX_HEAT, status);
    }
    else
    {
        set_val(CONTROL_AUX_HEAT, status);
    }
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
void control_cool2_1(int mode, int status)
{
    if (mode)
    {
        set_val(CONTROL_COOL2_1, status);
    }
    else
    {
        set_val(CONTROL_COOL2_1, status);
    }
}

//----------------------------------------------------------------------------------------------------------
// 水冷电磁阀2-2
//----------------------------------------------------------------------------------------------------------
void control_cool2_2(int mode, int status)
{
    if (mode)
    {
        set_val(CONTROL_COOL2_2, status);
    }
    else
    {
        set_val(CONTROL_COOL2_2, status);
    }
}

//----------------------------------------------------------------------------------------------------------
// 水冷电磁阀2-3
//----------------------------------------------------------------------------------------------------------
void control_cool2_3(int mode, int status)
{
    if (mode)
    {
        set_val(CONTROL_COOL2_3, status);
    }
    else
    {
        set_val(CONTROL_COOL2_3, status);
    }
}

//----------------------------------------------------------------------------------------------------------
// 水冷电磁阀2-4
//----------------------------------------------------------------------------------------------------------
void control_cool2_4(int mode, int status)
{
    if (mode)
    {
        set_val(CONTROL_COOL2_4, status);
    }
    else
    {
        set_val(CONTROL_COOL2_4, status);
    }
}

//----------------------------------------------------------------------------------------------------------
// 水冷电磁阀2
//----------------------------------------------------------------------------------------------------------
void control_cool2(int mode, int type, int status)
{

    switch (type)
    {
    case 1:
        control_cool2_1(mode, status);
        break;
    case 2:
        control_cool2_2(mode, status);
        break;
    case 3:
        control_cool2_3(mode, status);
        break;
    case 4:
        control_cool2_4(mode, status);
        break;
    default:
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
void control_blower(int mode, int status)
{
    if (mode)
    {
        set_val(CONTROL_BLOWER, status);
    }
    else
    {
        set_val(CONTROL_BLOWER, status);
    }
}

//----------------------------------------------------------------------------------------------------------
// 风门开控制
//----------------------------------------------------------------------------------------------------------
void control_open_damper(int mode, int status)
{
    // 检测到开到位停止控制
    if (get_val(DETECT_DAMPER_FULLY_OPEN))
    {
        set_val(CONTROL_DAMPER_OPEN, OFF);
    }
    else
    {
        set_val(CONTROL_DAMPER_CLOSE, OFF);
        if (mode)
        {
            set_val(CONTROL_DAMPER_OPEN, status);
        }
        else
        {
            set_val(CONTROL_DAMPER_OPEN, status);
        }
    }
}

//----------------------------------------------------------------------------------------------------------
// 风门关控制
//----------------------------------------------------------------------------------------------------------
void control_close_damper(int mode, int status, int schedule)
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
        // 优先处理 测试模式
        if (mode)
        {
            set_val(CONTROL_DAMPER_CLOSE, status);
        }
        else
        {
            // 当schedule为1时，风门小于设定值不会操作
            if (1 == schedule)
            {
                // 当风门测量值大于风门设定值，风门才会操作
                if (get_val(R_AI_AD) > get_val(P_AO_AD))
                {
                    set_val(CONTROL_DAMPER_CLOSE, status);
                }
            }
            // 当schedule为2时，风门会从最小值开到设定值
            else if (2 == schedule)
            {
                // 当风门测量值小于风门设定值，风门才会开
                if (get_val(R_AI_AD) < get_val(P_AO_AD))
                {
                    set_val(CONTROL_DAMPER_CLOSE, ON);
                }
            }
            // 当schedule为0时，风门关到位
            else
            {
                set_val(CONTROL_DAMPER_CLOSE, status);
            }
        }
    }
}

//----------------------------------------------------------------------------------------------------------
// 风门控制
//----------------------------------------------------------------------------------------------------------
void control_damper(int mode, int type, int status, int schedule)
{

    // 测试模式优先级最高
    switch (type)
    {
    case 1:
        control_open_damper(mode, status);
        break;
    case 2:
        control_close_damper(mode, status, schedule);
        break;
    default:
        break;
    }
}

//----------------------------------------------------------------------------------------------------------
// 水冷控制
//----------------------------------------------------------------------------------------------------------
void control_cool(int mode, int status)
{
    if (mode)
    {
        set_val(CONTROL_COOL, status);
    }
    else
    {
        set_val(CONTROL_COOL, status);
    }
}

//----------------------------------------------------------------------------------------------------------
// 加湿电磁阀控制
//----------------------------------------------------------------------------------------------------------
void control_spray(int mode, int status)
{
    if (mode)
    {
        set_val(CONTROL_SPRAY, status);
    }
    else
    {
        set_val(CONTROL_SPRAY, status);
    }
}

//----------------------------------------------------------------------------------------------------------
// 翻蛋控制
//----------------------------------------------------------------------------------------------------------
void control_egg(int mode)
{
    // 翻蛋方向
    unsigned short v_uc_egg_filpping_direction = get_uval(P_EGG_FLIPPING_DIRECTION);
    // 翻蛋时间
    unsigned short v_i_egg_filping_time = get_uval(P_HH10) * 60;

    zlog_debug(g_zlog_zc, "%-40s翻动方向:[%d]", "[翻蛋控制]", v_uc_egg_filpping_direction);
    if (mode)
    {
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
    }
    else
    {
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
    // 初始化DO输出为空
    //----------------------------------------------------------------------------------------------------------
    memset(&shm_out[DO_INIT], 0, sizeof(short) * 24);

    //----------------------------------------------------------------------------------------------------------
    // 初始化状态灯为空
    //----------------------------------------------------------------------------------------------------------
    memset(&shm_out[OUT_ST_FAN_STATUS], OFF, sizeof(short) * 100);

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
            int v_i_status = (get_time(R_RUN_SECOND_HIGH) > 0) && (get_time(R_RUN_SYSTEM_SECOND_HIGH) > 0) &&
                             (get_uval(P_PT1_AO2) > 0) && (get_uval(P_PT1_AO3) > 0) && (get_uval(P_PT1_AO4) > 0) &&
                             (get_uval(P_PT2_AO2) > 0) && (get_uval(P_PT2_AO3) > 0) && (get_uval(P_PT2_AO4) > 0) &&
                             (get_uval(P_PT3_AO2) > 0) && (get_uval(P_PT3_AO3) > 0) && (get_uval(P_PT3_AO4) > 0) &&
                             (get_uval(P_PT4_AO2) > 0) && (get_uval(P_PT4_AO3) > 0) && (get_uval(P_PT4_AO4) > 0) &&
                             (get_uval(P_PT5_AO2) > 0) && (get_uval(P_PT5_AO3) > 0) && (get_uval(P_PT5_AO4) > 0) &&
                             (get_uval(P_PT6_AO2) > 0) && (get_uval(P_PT6_AO3) > 0) && (get_uval(P_PT6_AO4) > 0);
            // zlog_info(g_zlog_zc, "[init_para]%-40sget_time(R_RUN_SECOND_HIGH):%ld\n", get_time(R_RUN_SECOND_HIGH));
            // zlog_info(g_zlog_zc, "[init_para]%-40sget_time(R_RUN_SYSTEM_SECOND_HIGH):%ld\n", get_time(R_RUN_SYSTEM_SECOND_HIGH));
            // zlog_info(g_zlog_zc, "[init_para]%-40sget_uval(P_PT1_AO2):%d\n", get_uval(P_PT1_AO2));
            // zlog_info(g_zlog_zc, "[init_para]%-40sget_uval(P_PT2_AO2):%d\n", get_uval(P_PT2_AO2));
            // zlog_info(g_zlog_zc, "[init_para]%-40sget_uval(P_PT3_AO2):%d\n", get_uval(P_PT3_AO2));
            // zlog_info(g_zlog_zc, "[init_para]%-40sget_uval(P_PT4_AO2):%d\n", get_uval(P_PT4_AO2));
            // zlog_info(g_zlog_zc, "[init_para]%-40sget_uval(P_PT5_AO2):%d\n", get_uval(P_PT5_AO2));
            // zlog_info(g_zlog_zc, "[init_para]%-40sget_uval(P_PT6_AO2):%d\n", get_uval(P_PT6_AO2));
            // zlog_info(g_zlog_zc, "[init_para]%-40sget_uval(P_PT1_AO3):%d\n", get_uval(P_PT1_AO3));
            // zlog_info(g_zlog_zc, "[init_para]%-40sget_uval(P_PT2_AO3):%d\n", get_uval(P_PT2_AO3));
            // zlog_info(g_zlog_zc, "[init_para]%-40sget_uval(P_PT4_AO3):%d\n", get_uval(P_PT4_AO3));
            // zlog_info(g_zlog_zc, "[init_para]%-40sget_uval(P_PT5_AO3):%d\n", get_uval(P_PT5_AO3));
            // zlog_info(g_zlog_zc, "[init_para]%-40sget_uval(P_PT6_AO3):%d\n", get_uval(P_PT6_AO3));
            // zlog_info(g_zlog_zc, "[init_para]%-40sget_uval(P_PT1_AO4):%d\n", get_uval(P_PT1_AO4));
            // zlog_info(g_zlog_zc, "[init_para]%-40sget_uval(P_PT2_AO4):%d\n", get_uval(P_PT2_AO4));
            // zlog_info(g_zlog_zc, "[init_para]%-40sget_uval(P_PT3_AO4):%d\n", get_uval(P_PT3_AO4));
            // zlog_info(g_zlog_zc, "[init_para]%-40sget_uval(P_PT4_AO4):%d\n", get_uval(P_PT4_AO4));
            // zlog_info(g_zlog_zc, "[init_para]%-40sget_uval(P_PT5_AO4):%d\n", get_uval(P_PT5_AO4));
            // zlog_info(g_zlog_zc, "[init_para]%-40sget_uval(P_PT6_AO4):%d\n", get_uval(P_PT6_AO4));
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
            int v_i_status = (get_time(R_RUN_SECOND_HIGH) > 0) && (get_time(R_RUN_SYSTEM_SECOND_HIGH) > 0) &&
                             (get_uval(P_PT1_AO2) > 0) && (get_uval(P_PT1_AO3) > 0) && (get_uval(P_PT1_AO4) > 0) &&
                             (get_uval(P_PT2_AO2) > 0) && (get_uval(P_PT2_AO3) > 0) && (get_uval(P_PT2_AO4) > 0) &&
                             (get_uval(P_PT3_AO2) > 0) && (get_uval(P_PT3_AO3) > 0) && (get_uval(P_PT3_AO4) > 0) &&
                             (get_uval(P_PT4_AO2) > 0) && (get_uval(P_PT4_AO3) > 0) && (get_uval(P_PT4_AO4) > 0) &&
                             (get_uval(P_PT5_AO2) > 0) && (get_uval(P_PT5_AO3) > 0) && (get_uval(P_PT5_AO4) > 0) &&
                             (get_uval(P_PT6_AO2) > 0) && (get_uval(P_PT6_AO3) > 0) && (get_uval(P_PT6_AO4) > 0);
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
            int v_i_status = (get_time(R_RUN_SECOND_HIGH) > 0) && (get_time(R_RUN_SYSTEM_SECOND_HIGH) > 0) &&
                             (get_uval(P_PT1_AO2) > 0) && (get_uval(P_PT1_AO3) > 0) && (get_uval(P_PT1_AO4) > 0) &&
                             (get_uval(P_PT2_AO2) > 0) && (get_uval(P_PT2_AO3) > 0) && (get_uval(P_PT2_AO4) > 0) &&
                             (get_uval(P_PT3_AO2) > 0) && (get_uval(P_PT3_AO3) > 0) && (get_uval(P_PT3_AO4) > 0) &&
                             (get_uval(P_PT4_AO2) > 0) && (get_uval(P_PT4_AO3) > 0) && (get_uval(P_PT4_AO4) > 0) &&
                             (get_uval(P_PT5_AO2) > 0) && (get_uval(P_PT5_AO3) > 0) && (get_uval(P_PT5_AO4) > 0) &&
                             (get_uval(P_PT6_AO2) > 0) && (get_uval(P_PT6_AO3) > 0) && (get_uval(P_PT6_AO4) > 0);
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
            int v_i_status = (get_time(R_RUN_SECOND_HIGH) > 0) && (get_time(R_RUN_SYSTEM_SECOND_HIGH) > 0) &&
                             (get_uval(P_PT1_AO2) > 0) && (get_uval(P_PT1_AO3) > 0) && (get_uval(P_PT1_AO4) > 0) &&
                             (get_uval(P_PT2_AO2) > 0) && (get_uval(P_PT2_AO3) > 0) && (get_uval(P_PT2_AO4) > 0) &&
                             (get_uval(P_PT3_AO2) > 0) && (get_uval(P_PT3_AO3) > 0) && (get_uval(P_PT3_AO4) > 0) &&
                             (get_uval(P_PT4_AO2) > 0) && (get_uval(P_PT4_AO3) > 0) && (get_uval(P_PT4_AO4) > 0) &&
                             (get_uval(P_PT5_AO2) > 0) && (get_uval(P_PT5_AO3) > 0) && (get_uval(P_PT5_AO4) > 0) &&
                             (get_uval(P_PT6_AO2) > 0) && (get_uval(P_PT6_AO3) > 0) && (get_uval(P_PT6_AO4) > 0);
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
            int v_i_status = (get_time(R_RUN_SECOND_HIGH) > 0) && (get_time(R_RUN_SYSTEM_SECOND_HIGH) > 0) &&
                             (get_uval(P_PT1_AO2) > 0) && (get_uval(P_PT1_AO3) > 0) && (get_uval(P_PT1_AO4) > 0) &&
                             (get_uval(P_PT2_AO2) > 0) && (get_uval(P_PT2_AO3) > 0) && (get_uval(P_PT2_AO4) > 0) &&
                             (get_uval(P_PT3_AO2) > 0) && (get_uval(P_PT3_AO3) > 0) && (get_uval(P_PT3_AO4) > 0) &&
                             (get_uval(P_PT4_AO2) > 0) && (get_uval(P_PT4_AO3) > 0) && (get_uval(P_PT4_AO4) > 0) &&
                             (get_uval(P_PT5_AO2) > 0) && (get_uval(P_PT5_AO3) > 0) && (get_uval(P_PT5_AO4) > 0) &&
                             (get_uval(P_PT6_AO2) > 0) && (get_uval(P_PT6_AO3) > 0) && (get_uval(P_PT6_AO4) > 0);
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
            int v_i_status = (get_time(R_RUN_SECOND_HIGH) > 0) && (get_time(R_RUN_SYSTEM_SECOND_HIGH) > 0) &&
                             (get_uval(P_PT1_AO2) > 0) && (get_uval(P_PT1_AO3) > 0) && (get_uval(P_PT1_AO4) > 0) &&
                             (get_uval(P_PT2_AO2) > 0) && (get_uval(P_PT2_AO3) > 0) && (get_uval(P_PT2_AO4) > 0) &&
                             (get_uval(P_PT3_AO2) > 0) && (get_uval(P_PT3_AO3) > 0) && (get_uval(P_PT3_AO4) > 0) &&
                             (get_uval(P_PT4_AO2) > 0) && (get_uval(P_PT4_AO3) > 0) && (get_uval(P_PT4_AO4) > 0) &&
                             (get_uval(P_PT5_AO2) > 0) && (get_uval(P_PT5_AO3) > 0) && (get_uval(P_PT5_AO4) > 0) &&
                             (get_uval(P_PT6_AO2) > 0) && (get_uval(P_PT6_AO3) > 0) && (get_uval(P_PT6_AO4) > 0);
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
            int v_i_status = (get_time(R_RUN_SECOND_HIGH) > 0) && (get_time(R_RUN_SYSTEM_SECOND_HIGH) > 0) &&
                             (get_uval(P_PT1_AO2) > 0) && (get_uval(P_PT1_AO3) > 0) && (get_uval(P_PT1_AO4) > 0) &&
                             (get_uval(P_PT2_AO2) > 0) && (get_uval(P_PT2_AO3) > 0) && (get_uval(P_PT2_AO4) > 0) &&
                             (get_uval(P_PT3_AO2) > 0) && (get_uval(P_PT3_AO3) > 0) && (get_uval(P_PT3_AO4) > 0) &&
                             (get_uval(P_PT4_AO2) > 0) && (get_uval(P_PT4_AO3) > 0) && (get_uval(P_PT4_AO4) > 0) &&
                             (get_uval(P_PT5_AO2) > 0) && (get_uval(P_PT5_AO3) > 0) && (get_uval(P_PT5_AO4) > 0) &&
                             (get_uval(P_PT6_AO2) > 0) && (get_uval(P_PT6_AO3) > 0) && (get_uval(P_PT6_AO4) > 0);
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
            int v_i_status = (get_time(R_RUN_SECOND_HIGH) > 0) && (get_time(R_RUN_SYSTEM_SECOND_HIGH) > 0) &&
                             (get_uval(P_PT1_AO2) > 0) && (get_uval(P_PT1_AO3) > 0) && (get_uval(P_PT1_AO4) > 0) &&
                             (get_uval(P_PT2_AO2) > 0) && (get_uval(P_PT2_AO3) > 0) && (get_uval(P_PT2_AO4) > 0) &&
                             (get_uval(P_PT3_AO2) > 0) && (get_uval(P_PT3_AO3) > 0) && (get_uval(P_PT3_AO4) > 0) &&
                             (get_uval(P_PT4_AO2) > 0) && (get_uval(P_PT4_AO3) > 0) && (get_uval(P_PT4_AO4) > 0) &&
                             (get_uval(P_PT5_AO2) > 0) && (get_uval(P_PT5_AO3) > 0) && (get_uval(P_PT5_AO4) > 0) &&
                             (get_uval(P_PT6_AO2) > 0) && (get_uval(P_PT6_AO3) > 0) && (get_uval(P_PT6_AO4) > 0);
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
            int v_i_status = (get_time(R_RUN_SECOND_HIGH) > 0) && (get_time(R_RUN_SYSTEM_SECOND_HIGH) > 0) &&
                             (get_uval(P_PT1_AO2) > 0) && (get_uval(P_PT1_AO3) > 0) && (get_uval(P_PT1_AO4) > 0) &&
                             (get_uval(P_PT2_AO2) > 0) && (get_uval(P_PT2_AO3) > 0) && (get_uval(P_PT2_AO4) > 0) &&
                             (get_uval(P_PT3_AO2) > 0) && (get_uval(P_PT3_AO3) > 0) && (get_uval(P_PT3_AO4) > 0) &&
                             (get_uval(P_PT4_AO2) > 0) && (get_uval(P_PT4_AO3) > 0) && (get_uval(P_PT4_AO4) > 0) &&
                             (get_uval(P_PT5_AO2) > 0) && (get_uval(P_PT5_AO3) > 0) && (get_uval(P_PT5_AO4) > 0) &&
                             (get_uval(P_PT6_AO2) > 0) && (get_uval(P_PT6_AO3) > 0) && (get_uval(P_PT6_AO4) > 0);
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
            int v_i_status = (get_time(R_RUN_SECOND_HIGH) > 0) && (get_time(R_RUN_SYSTEM_SECOND_HIGH) > 0) &&
                             (get_uval(P_PT1_AO2) > 0) && (get_uval(P_PT1_AO3) > 0) && (get_uval(P_PT1_AO4) > 0) &&
                             (get_uval(P_PT2_AO2) > 0) && (get_uval(P_PT2_AO3) > 0) && (get_uval(P_PT2_AO4) > 0) &&
                             (get_uval(P_PT3_AO2) > 0) && (get_uval(P_PT3_AO3) > 0) && (get_uval(P_PT3_AO4) > 0) &&
                             (get_uval(P_PT4_AO2) > 0) && (get_uval(P_PT4_AO3) > 0) && (get_uval(P_PT4_AO4) > 0) &&
                             (get_uval(P_PT5_AO2) > 0) && (get_uval(P_PT5_AO3) > 0) && (get_uval(P_PT5_AO4) > 0) &&
                             (get_uval(P_PT6_AO2) > 0) && (get_uval(P_PT6_AO3) > 0) && (get_uval(P_PT6_AO4) > 0);
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
    // 超过10次以上不恢复数据
    //----------------------------------------------------------------------------------------------------------
    if (sync_file_index > 10)
    {
        zlog_error(g_zlog_zc, "%-40s数据恢复失败!", "[init_para]");
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
    if (get_uval(REFLUX_TEMPERATURE) == 0)
    {
        set_uval(REFLUX_TEMPERATURE, 15);
    }

    //----------------------------------------------------------------------------------------------------------
    // 风门校准状态
    //----------------------------------------------------------------------------------------------------------
    set_uval(STATUS_DAMPER_CHECK, OFF);

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
        set_uval(P_HH9, 1200);
    }

    //----------------------------------------------------------------------------------------------------------
    // 最大运行天数
    //----------------------------------------------------------------------------------------------------------
    if (get_uval(R_RUN_MAX_DAY) == 0)
    {
        set_uval(R_RUN_MAX_DAY, 3500);
    }

    //----------------------------------------------------------------------------------------------------------
    // 风门开单位时间内所需要的时间
    //----------------------------------------------------------------------------------------------------------
    int v_i_fan_open_time = get_uval(DAMPER_TIME_LEN_ALL_OPEN) / (get_uval(DAMPER_MAX_LEN) - get_uval(DAMPER_MIN_LEN));
    set_uval(DAMPER_TIME_OPEN, v_i_fan_open_time);

    //----------------------------------------------------------------------------------------------------------
    // 风门关单位时间内所需要的时间
    //----------------------------------------------------------------------------------------------------------
    int v_i_fan_close_time = get_uval(DAMPER_TIME_LEN_ALL_CLOSE) / (get_uval(DAMPER_MAX_LEN) - get_uval(DAMPER_MIN_LEN));
    set_uval(DAMPER_TIME_CLOSE, v_i_fan_close_time);

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
    // 时间改变时 触发数据变更
    //----------------------------------------------------------------------------------------------------------
    // 回流温度传感器忘挂
    //----------------------------------------------------------------------------------------------------------
    if (get_uold(P_HH0) == 0)
    {
        unsigned short v_i_hh0_time = get_uval(P_HH0);
        set_uval(P_HH0, v_i_hh0_time);
    }

    //----------------------------------------------------------------------------------------------------------
    // 翻蛋异常延时
    //----------------------------------------------------------------------------------------------------------
    if (get_uold(P_HH1) == 0)
    {
        unsigned short v_i_hh1_time = get_uval(P_HH1);
        set_uval(P_HH1, v_i_hh1_time);
    }

    //----------------------------------------------------------------------------------------------------------
    // 出雏器高温报警延时
    //----------------------------------------------------------------------------------------------------------
    if (get_uold(P_HH2) == 0)
    {
        unsigned short v_i_hh2_time = get_uval(P_HH2);
        set_uval(P_HH2, v_i_hh2_time);
    }

    //----------------------------------------------------------------------------------------------------------
    // 测试运转时间
    //----------------------------------------------------------------------------------------------------------
    if (get_uold(P_HH3) == 0)
    {
        unsigned short v_i_hh3_time = get_uval(P_HH3);
        set_uval(P_HH3, v_i_hh3_time);
    }

    //----------------------------------------------------------------------------------------------------------
    // 消毒时间
    //----------------------------------------------------------------------------------------------------------
    if (get_uold(P_HH4) == 0)
    {
        unsigned short v_i_hh4_time = get_uval(P_HH4);
        set_uval(P_HH4, v_i_hh4_time);
    }

    //----------------------------------------------------------------------------------------------------------
    // 回流温度控制检测时间
    //----------------------------------------------------------------------------------------------------------
    if (get_uold(P_HH5) == 0)
    {
        unsigned short v_i_hh5_time = get_uval(P_HH5);
        set_uval(P_HH5, v_i_hh5_time);
    }

    //----------------------------------------------------------------------------------------------------------
    // 干燥开关最长运行时间
    //----------------------------------------------------------------------------------------------------------
    if (get_uold(P_HH6) == 0)
    {
        unsigned short v_i_hh6_time = get_uval(P_HH6);
        set_uval(P_HH6, v_i_hh6_time);
    }

    //----------------------------------------------------------------------------------------------------------
    // 风扇停止警报时间长度
    //----------------------------------------------------------------------------------------------------------
    if (get_uold(P_HH7) == 0)
    {
        unsigned short v_i_hh7_time = get_uval(P_HH7);
        set_uval(P_HH7, v_i_hh7_time);
    }

    //----------------------------------------------------------------------------------------------------------
    // 翻蛋警报检测时间
    //----------------------------------------------------------------------------------------------------------
    if (get_uold(P_HH8) == 0)
    {
        unsigned short v_i_hh8_time = get_uval(P_HH8);
        set_uval(P_HH8, v_i_hh8_time);
    }

    //----------------------------------------------------------------------------------------------------------
    // 风门开10MM后 再启动的延时时间
    //----------------------------------------------------------------------------------------------------------
    if (get_uold(P_HH9) == 0)
    {
        unsigned short v_i_hh9_time = get_uval(P_HH9);
        set_uval(P_HH9, v_i_hh9_time);
    }

    //----------------------------------------------------------------------------------------------------------
    // 翻蛋时间
    //----------------------------------------------------------------------------------------------------------
    if (get_uold(P_HH10) == 0)
    {
        unsigned short v_i_hh10_time = get_uval(P_HH10);
        set_uval(P_HH10, v_i_hh10_time);
    }

    //----------------------------------------------------------------------------------------------------------
    // 高湿报警延时时间
    //----------------------------------------------------------------------------------------------------------
    if (get_uold(P_HH11) == 0)
    {
        unsigned short v_i_hh11_time = get_uval(P_HH11);
        set_uval(P_HH11, v_i_hh11_time);
    }

    //----------------------------------------------------------------------------------------------------------
    // 初始化风门校准
    //----------------------------------------------------------------------------------------------------------
    set_uval(STATUS_DAMPER_ALL_CLOSE_CAL, ON);
}

//----------------------------------------------------------------------------------------------------------
// 将共享内存写入INI文化中
//----------------------------------------------------------------------------------------------------------
int syn_shm_to_ini(const char *file_name, unsigned short index, int size)
{
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
    }
    fclose(file);
    return 0;
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

//----------------------------------------------------------------------------------------------------------
// 将当前时间保存到共享内存中
//----------------------------------------------------------------------------------------------------------
void set_time(unsigned short addr, unsigned int time)
{
    dcba(&time, sizeof(unsigned int));
    memcpy(&shm_out[addr], &time, sizeof(unsigned int));
}

//----------------------------------------------------------------------------------------------------------
// 线程
//----------------------------------------------------------------------------------------------------------
// 线程 同步线程
//----------------------------------------------------------------------------------------------------------
void *sysn_thread_func(void *pv)
{
    zlog_debug(g_zlog_zc, "%-40s进入同步线程!", "[sysn_thread_func]");

    // 当前备份文件
    char sync_backup_file[512] = "";
    // 当前备份文件序号
    int sync_backup_index = 1;
    // 备份数据起始地址
    int sync_data_start = 0;
    // 备份数据长度
    int sync_data_len = 5000;

    while (1)
    {
        // 当前备份文件序号 大于 10 重新赋值
        if (sync_backup_index > 10)
        {
            sync_backup_index = 1;
        }

        snprintf(sync_backup_file, 512, "%s_%d", g_st_jincubator.conf_sysc_file, sync_backup_index);
        int i_ret = syn_shm_to_ini(sync_backup_file, sync_data_start, sync_data_len);
        zlog_debug(g_zlog_zc, "%-40s文件名称: %s 数据起始地址：%d 数据长度：%d 备份状态: %s", "[sysn_thread_func]", sync_backup_file, sync_data_start, sync_data_len, (!i_ret) ? "备份成功" : "备份失败");

        // 当前备份文件序号 自增
        ++sync_backup_index;

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
                double tmp_ai_tp_data[filter_len] = {0.0};
                // 将滑动窗口的第一个剔除，
                memcpy(&tmp_ai_tp_data, &run_ai_tp_data[i][1], sizeof(double) * (filter_len - 1));
                // 将最新的数据保存到最后的位置
                tmp_ai_tp_data[filter_len - 1] = run_ai_tp_val;
                // 将整理后的数据从新保存到数组中
                memcpy(&run_ai_tp_data[i][0], &tmp_ai_tp_data, sizeof(double) * (filter_len));
            }

            if (run_ai_tp_len[i] == filter_len)
            {
                // 实际数据的序号
                unsigned short index = 0;
                // 修正值的序号
                unsigned short shm_index = 0;
                // 传感器异常报警序号
                unsigned short alarm_index = 0;
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
                    index = R_AI_TP_MAIN + i + 2;
                    shm_index = P_EDIT_TP_MAIN + i + 2;
                }
                else
                {
                    index = R_AI_TP_MAIN + i;
                    shm_index = P_EDIT_TP_MAIN + i;
                }
                // 获取当前传感器报警变量
                alarm_index = SENSOR_1_ABNORMAL_ALARM + i;
                // 使用滑动窗口获取通道平均值
                run_ai_tp_aver = run_ai_tp_sum / filter_len;
                // 设置浮点精度
                sprintf(d_tp_data, "%0.2f", run_ai_tp_aver);

                // 校准模式标志位
                float f_tp_data = atof(d_tp_data) + get_val(shm_index);
                // 将浮点数转为无符号整数数据
                int run_ai_i_tp = f_tp_data * 100;
                // 过滤异常数据
                unsigned short run_ao_c_shm = ((run_ai_i_tp > 32767) || (run_ai_i_tp < -32767)) ? 0 : run_ai_i_tp;
                // zlog_debug(g_zlog_zc,
                //            "%-40s通道值 [%d] Val-Offset [%0.4f] Val-Divi [%0.4f] Val-slope [%u]  Ref-Std0 [%u] 当前通道值 [%u] 当前温度 [%0.4f] 当前差值[%0.4f] 最终温度 [%0.4f] 最终温度 [%d]",
                //            "[data_collection_pt100_func]",i + 1, run_ao_r_low, run_ao_r_high, run_ao_c_low,
                //            run_ao_c_high, run_ai_c_tp,
                //            atof(d_tp_data), get_val(shm_index), f_tp_data, run_ao_c_shm);

                // 无符号温度大于50摄氏度时，触发报警
                if (run_ao_c_shm > 0x1388)
                {
                    // 设置报警变量为ON
                    set_uval(alarm_index, ON);
                    // 将实时数据设置为-1
                    set_uval(index, 0xffff);
                }
                else
                {
                    // 设置报警变量为OFF
                    set_uval(alarm_index, OFF);
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
    char *c_seral_name = g_st_jincubator.serial_device;

    int i_serial_fd = SerialOpen(c_seral_name);
    if (i_serial_fd < 0)
    {
        zlog_error(g_zlog_zc, "%-40s串口打开失败", "[data_collection_co2_func]");
        pthread_exit(NULL);
    }
    zlog_info(g_zlog_zc, "%-40s创建CO2串口线程成功!", "[data_collection_co2_func]");
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
        zlog_debug(g_zlog_zc, "%-40s[%s] 发送读取数据[9]:", "[data_collection_co2_func]", c_seral_name);
        hzlog_debug(g_zlog_zc, b_cmd_sendbuf, 9);
        // 接收数据
        wait_ms(1000);
        i_recv_len = SerialRead(i_serial_fd, b_cmd_recvbuf, 1024, i_cmd_wait_time_out, i_cmd_read_time_out);

        if (i_recv_len > 0)
        {
            zlog_debug(g_zlog_zc, "%-40s[%s] 接收读取数据[%d]:", "[data_collection_co2_func]", c_seral_name, i_recv_len);
            hzlog_debug(g_zlog_zc, b_cmd_recvbuf, i_recv_len);
            if (i_recv_len == 9)
            {
                i_data_count = 0;
                memcpy(&i_recv_data, &b_cmd_recvbuf[2], 2);
                // 校准模式标志位
                i_data_value = i_recv_data;
                ba((char *)&i_data_value, 2);

                if (i_data_value >= 0 && i_data_value <= 20000)
                {
                    // 传感器正常
                    set_val(SENSOR_CO2_ALARM, OFF);
                    // 将数据传输到实时位置
                    set_val(R_AI_CO, i_data_value);
                }
                else
                {
                    i_data_count = 10;
                }

                // printf("%d\n", (&shm_out[P_EDIT_CO]));

                zlog_debug(g_zlog_zc, "%-40s[%s] 接收到CO2 [%u]", "[data_collection_co2_func]", c_seral_name, i_data_value);
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
                if (get_val(P_CO2_STATUS))
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
            zlog_error(g_zlog_zc, "%-40s[%s] 发送读取数据[%d]:", "[data_collection_co2_func]", c_seral_name, i_recv_len);
            hzlog_error(g_zlog_zc, b_cmd_sendbuf, 9);
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
        }
    }
}

int main(int argc, char *argv[])
{
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

    // 定义程序名称
    char *filename = NULL;

    //----------------------------------------------------------------------------------------------------------
    // 解析程序名称
    //----------------------------------------------------------------------------------------------------------
    char date_str[256];
    print_current_time(date_str, sizeof(date_str));
    parse_file_path(argv[0], &filename);
    if (strlen(filename) > 256)
    {
        fprintf(stderr, "%s %-40s程序名称已超过256个字节\n", date_str, "[main]");
        return -1;
    }
    char filenaem_ini[256] = {0};
    print_current_time(date_str, sizeof(date_str));
    if (filename == NULL)
    {
        fprintf(stderr, "%s %-40s程序名称为空\n", date_str, "[main]");
        return -1;
    }

    sprintf(filenaem_ini, "%s.ini", filename);
    print_current_time(date_str, sizeof(date_str));
    fprintf(stdout, "%s %-40s程序名称:%s 程序配置:%s\n", date_str, "[main]", filename, filenaem_ini);

    //----------------------------------------------------------------------------------------------------------
    // 读取INI配置
    //----------------------------------------------------------------------------------------------------------

    init_ini_file(filenaem_ini, &g_st_jincubator);

    //----------------------------------------------------------------------------------------------------------
    // ZLOG日志初始化
    //----------------------------------------------------------------------------------------------------------
    print_current_time(date_str, sizeof(date_str));
    fprintf(stdout, "%s %-40sZLOG日志:%s\n", date_str, "[main]", g_st_jincubator.conf_zlog_file);
    i_ret = init_zlog_file(&g_zlog_zc, g_st_jincubator.conf_zlog_file, filename);
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

    //----------------------------------------------------------------------------------------------------------
    // 备份线程
    //----------------------------------------------------------------------------------------------------------
    pthread_t sync_client_thread;
    pthread_create(&sync_client_thread, NULL, sysn_thread_func, NULL);
    pthread_detach(sync_client_thread);

    //----------------------------------------------------------------------------------------------------------
    // 数据采集线程-PT100
    //----------------------------------------------------------------------------------------------------------
    pthread_t data_collection_thread;
    pthread_create(&data_collection_thread, NULL, data_collection_pt100_func, NULL);
    pthread_detach(data_collection_thread);

    //----------------------------------------------------------------------------------------------------------
    // 数据采集线程-CO2
    //----------------------------------------------------------------------------------------------------------
    pthread_t co2_collection_thread;
    pthread_create(&co2_collection_thread, NULL, data_collection_co2_func, NULL);
    pthread_detach(co2_collection_thread);
    while (1)
    {
        //----------------------------------------------------------------------------------------------------------
        // 启动模式
        //----------------------------------------------------------------------------------------------------------

        //----------------------------------------------------------------------------------------------------------
        // 停止模式
        //----------------------------------------------------------------------------------------------------------

        //----------------------------------------------------------------------------------------------------------
        // 消警模式
        //----------------------------------------------------------------------------------------------------------

        //----------------------------------------------------------------------------------------------------------
        // 校准模式
        //----------------------------------------------------------------------------------------------------------

        //----------------------------------------------------------------------------------------------------------
        // 初始化模式
        //----------------------------------------------------------------------------------------------------------

        wait_ms(100);
    }

    return 0;
}
