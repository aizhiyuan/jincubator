#include "job.h"
#include "jincubator.h"
#include "define.h"
#include "../zlog/zlog.h"

// 全局变量
extern unsigned short *shm_out;
extern zlog_category_t *g_zlog_zc;

// 常量定义
const int MAX_PID_VALUE = 10000;
const int MIN_PID_VALUE = 0;

//================================================================================================================================
//	Out Range:[0,10000]
//================================================================================================================================
int pid_calc1(int pv, int sp)
{
    int en;

    int xp;
    int xi;
    int xd;

    int vn;
    int wn;
    int yn;

    int pidv;

    static int vn_1 = 0L;
    static int wn_1 = 0L;
    static int yn_1 = 0L;

    int iv, jv;

    // if(ERR_PT2 || pv<0) return(0);	// (*Outer[1]) * 100;
    en = (sp - pv); // Difference
    zlog_error(g_zlog_zc, "[PID温度]\t TEMP[EN]:[%d]", en);

    //--------------------------------------------------------------------
    //	P:
    //--------------------------------------------------------------------
    if ((get_x10_val(P_TEMP_P)) > 0)
    {
        zlog_error(g_zlog_zc, "[PID温度]\t TEMP[P]:[%d]", get_x10_val(P_TEMP_P));
        if (abs(en) <= (get_x10_val(P_TEMP_P) / 2))
        {
            //
            xp = (10000L) * en / (get_x10_val(P_TEMP_P)) + (5000L);
        }
        else if (sp > pv)
        {
            xp = 10000L;
        }
        else
        {
            xp = 0L;
        }
    }
    else
    {
        xp = 0L;
    }

    if (xp < (0L))
        xp = 0L;
    if (xp > (10000L))
        xp = (10000L);

    // XP[0]=xp;
    zlog_error(g_zlog_zc, "[PID温度]\t TEMP[XP]:[%d]", xp);

    //--------------------------------------------------------------------
    // I:
    //--------------------------------------------------------------------
    zlog_error(g_zlog_zc, "[PID温度]\t TEMP[I]:[%d]", get_e10_val(P_TEMP_I));
    if ((get_e10_val(P_TEMP_I)) > 0 && abs(en) <= (get_x10_val(P_TEMP_P) / 2))
    {
        iv = (5000L); // iv=(10000L)*(SAMPLE/SCONST)/(1000L);

        // pid_temp_sum_en += en;
        set_int(PID_TEMP_SUM, get_int(PID_TEMP_SUM) + en);            // -200000L ~ 300000L
        xi = (50L) * get_int(PID_TEMP_SUM) / (get_e10_val(P_TEMP_I)); // xi=iv*pid_temp_sum_en/gMain_I/ 100L;
    }
    else
    {
        xi = 0L;
        // pid_temp_sum_en = 0L;
        set_int(PID_TEMP_SUM, 0);
    }

    if (xi < (-10000L))
        xi = (-10000L);
    if (xi > (10000L))
        xi = (10000L);

    //------------------------------------------------
    //	Note:
    //------------------------------------------------
    vn = xp + xi; // -10000L ~ 20000L
    if ((vn > (15000L)) || (vn < (-5000L)))
    {
        // pid_temp_sum_en -= en;
        set_int(PID_TEMP_SUM, get_int(PID_TEMP_SUM) - en);
    }

    zlog_error(g_zlog_zc, "[PID温度]\t TEMP[XI]:[%d]", xi);
    zlog_error(g_zlog_zc, "[PID温度]\t TEMP[SUM]:[%d]", get_int(PID_TEMP_SUM));
    zlog_error(g_zlog_zc, "[PID温度]\t TEMP[VN]:[%d]", vn);

    // XI[0]=xi;
    // SUM[0]=pid_temp_sum_en;
    // VN[0]=vn;

    //--------------------------------------------------------------------
    //	D:
    //--------------------------------------------------------------------
    zlog_error(g_zlog_zc, "[PID温度]\t TEMP[D]:[%d]", get_e10_val(P_TEMP_D));
    if ((get_e10_val(P_TEMP_D)) > 0 && abs(en) <= (get_x10_val(P_TEMP_P) / 2))
    {

        xd = (2L) * (get_e10_val(P_TEMP_D)) * (vn - vn_1); //	-30000 ~ 30000	// xd=1000L*gMain_D*(vn - vn_1)/(SAMPLE/SCONST);
    }
    else
    {
        xd = 0L;
    }

    //------------------------------------------------
    //	result:
    //------------------------------------------------
    wn = vn + xd; // -1820000 ~ 1820000

    zlog_error(g_zlog_zc, "[PID温度]\t TEMP[XD]:[%d]", xd);
    zlog_error(g_zlog_zc, "[PID温度]\t TEMP[WN]:[%d]", wn);
    // XD[0]=xd;
    // WN[0]=wn;

    iv = wn + ((get_e10_val(P_TEMP_D)) * 100 * wn_1) / 500; // PID����//iv = wn + (gMain_D * wn_1 * 1000) / 10 / (SAMPLE/SCONST);
    jv = 1L + ((get_e10_val(P_TEMP_D)) * 100) / 500;        // PID��ĸ//jv = 1L + gMain_D * 1000 / 10 / (SAMPLE/SCONST);
    yn = iv / jv;

    //------------------------------------------------
    //
    //------------------------------------------------
    vn_1 = vn;
    // VN_1[0]=vn_1;
    wn_1 = wn;
    yn_1 = yn;

    //------------------------------------------------
    //	limits:
    //------------------------------------------------
    if (yn > (10000L))
    {
        pidv = 10000;
    }
    else if (yn < 0)
    {
        pidv = 0;
    }
    else
    {
        pidv = yn;
    }

    //------------------------------------------------
    //
    //------------------------------------------------
    // pidv=(10000)-pidv;
    // pidv = (pidv - MAIN_PIDVAL_OFFSET) * 2;// 2010-05-26
    if (pidv > (get_val(P_TEMP_OHI) * 100))
    {
        pidv = (get_val(P_TEMP_OHI) * 100);
    }
    if (pidv < (get_val(P_TEMP_OLO) * 100))
    {
        pidv = (get_val(P_TEMP_OLO) * 100);
    }
    return (pidv);
}

//================================================================================================================================
//
//================================================================================================================================

//--------------------------------------------------------------------------------------------------
//	MH & AUXH:
//--------------------------------------------------------------------------------------------------

int main_heater(int pid)
{
    zlog_debug(g_zlog_zc, "[PID温度]\t pid:[%d]", pid);
    int duty = 0, DUTY_HI = (10L) * (get_val(P_TEMP_OHI)), DUTY_LO = (10L) * (get_val(P_TEMP_OLO));
    // Valid Range: 5500 ~ 8000
    zlog_debug(g_zlog_zc, "[PID温度]\t DUTY_HI:[%d]", DUTY_HI);
    duty = (100L) * ((pid / 10) - get_val(P_HLO) * 10) / (get_val(P_HHI) - get_val(P_HLO)); // Range: -2200 ~ 1800

    // duty = (100L) * ((pid / 10) - get_val(P_TEMP_OLO) * 10) / (get_val(P_TEMP_OHI) - get_val(P_TEMP_OLO) * 10); // Range: -2200 ~ 1800

    zlog_debug(g_zlog_zc, "[PID温度]\t duty1:[%d]", duty);
    if (duty >= DUTY_HI)
    {
        duty = DUTY_HI;
    }
    else if (duty <= DUTY_LO)
    {
        duty = DUTY_LO;
    }
    zlog_debug(g_zlog_zc, "[PID温度]\t duty2:[%d]", duty);

    //--------------------------------------------------------------------
    //
    //--------------------------------------------------------------------
    if ((get_val(R_AI_TP_MAIN)) >= (get_val(P_AO_TP_MAIN)))
    {
        zlog_debug(g_zlog_zc, "[PID温度]\t (get_val(R_AI_TP_MAIN)) >= (get_val(P_AO_TP_MAIN))");
        return (0);
    }
    else if (get_val(R_AI_TP_MAIN) < (get_val(P_AO_TP_MAIN) - (get_x10_val(P_TEMP_P) / 2)))
    {
        zlog_debug(g_zlog_zc, "[PID温度]\t get_val(R_AI_TP_MAIN) < (get_val(P_AO_TP_MAIN) - (get_x10_val(P_TEMP_P) / 2))");
        return (DUTY_HI);
    }
    else
    {
        zlog_debug(g_zlog_zc, "[PID温度]\t else");
        return (duty);
    }
}

//--------------------------------------------------------------------------------------------------
//	COOLER & BLOWER:
//--------------------------------------------------------------------------------------------------
char cool(int pid)
{

    static int nn = 0;
    static char retf = 0;
    int sp, op;

    //--------------------------------------------------------------------
    //
    //--------------------------------------------------------------------
    sp = get_val(P_COL) * 100;
    if (pid <= sp)
        op = 1000;
    else
        op = 0;

    //--------------------------------------------------------------------
    //
    //--------------------------------------------------------------------
    if (retf)
    {
        if (op < 500)
            nn--;
    }
    else
    {
        if (op > 500)
            nn++;
        else
            nn--;
    }

    if (nn > get_val(P_TCO))
        retf = 1;
    if (nn <= 0)
    {
        nn = 0;
        retf = 0;
    }

    //--------------------------------------------------------------------
    //
    //--------------------------------------------------------------------
    if ((get_val(R_AI_TP_MAIN)) <= (get_val(P_AO_TP_MAIN)))
    {
        return (0);
    }
    else
    {
        return (retf);
    }
}

//-----------------------------------------------------------------------------
//	鼓风机
//----------------------------------------------------------------------------
char blower(int pid)
{
    static int nn = 0;
    static char retf = 0;
    int sp, op;

    //--------------------------------------------------------------------
    //
    //--------------------------------------------------------------------

    sp = get_val(P_BLO) * 100;
    if (pid < sp)
        op = 1000;
    else
        op = 0;

    //--------------------------------------------------------------------
    //
    //--------------------------------------------------------------------

    if (retf)
    {
        if (op < 500)
            nn--;
    }
    else
    {
        if (op > 500)
            nn++;
        else
            nn--;
    }

    // if(Setter_sw) nn=0;	// �������޴˿���
    if (nn > get_val(P_TBL))
        retf = 1;
    if (nn <= 0)
    {
        nn = 0;
        retf = 0;
    }

    //--------------------------------------------------------------------
    //
    //--------------------------------------------------------------------
    if ((get_val(R_AI_TP_MAIN)) <= (get_val(P_AO_TP_MAIN)))
    {
        return (0);
    }
    else
    {
        return (retf);
    }
}

//-----------------------------------------------------------------------------
//	风门开
//----------------------------------------------------------------------------

unsigned int db_stoke = 10000; // No control

char db_open(int pid)
{

    static int nn = 0;    //
    static char retf = 0; //
    int sp, op, tmin;

    //--------------------------------------------------------------------
    //
    //--------------------------------------------------------------------
    sp = get_val(P_OPE) * 100;
    if (pid < sp)
        op = 1000;
    else
        op = 0;

    //--------------------------------------------------------------------
    //
    //--------------------------------------------------------------------

    if (retf)
    {
        if (op < 500)
            nn--;
    }
    else
    {
        if (op > 500)
            nn++;
        else
            nn--;
    }

    if (get_val(PID_TEMP_DB_CLOSE))
        nn = 0; // Interlock

    if (db_stoke > 10050)
        tmin = get_val(P_TCL);
    // No use
    else
        tmin = get_val(P_TOP);
    if (nn > tmin)
        retf = 1;
    if (nn <= 0)
    {
        nn = 0;
        retf = 0;
    }

    //--------------------------------------------------------------------
    //
    //--------------------------------------------------------------------
    if ((get_val(R_AI_TP_MAIN)) <= (get_val(P_AO_TP_MAIN)))
    {
        return (0);
    }
    else
    {
        return (retf);
    }
}

//-----------------------------------------------------------------------------
//	风门关
//----------------------------------------------------------------------------
char db_close(int pid)
{
    static int nn = 0;    //
    static char retf = 0; //
    int sp, op, tmin;

    //--------------------------------------------------------------------
    //
    //--------------------------------------------------------------------

    sp = get_val(P_OPE) * 100;
    if (pid > sp)
        op = 1000;
    else
        op = 0;

    //--------------------------------------------------------------------
    //
    //--------------------------------------------------------------------

    if (retf)
    {
        if (op < 500)
            nn--;
    }
    else
    {
        if (op > 500)
            nn++;
        else
            nn--;
    }

    if (get_val(PID_TEMP_DB_OPEN))
        nn = 0; // Interlock

    if (db_stoke < 9950)
        tmin = get_val(P_TCL);
    else
        tmin = get_val(P_TOP);

    if (nn > tmin)
        retf = 1;
    if (nn <= 0)
    {
        nn = 0;
        retf = 0;
    }

    //--------------------------------------------------------------------
    //
    //--------------------------------------------------------------------
    if ((get_val(R_AI_TP_MAIN)) >= (get_val(P_AO_TP_MAIN)))
    {
        return (0);
    }
    else
    {
        return (retf);
    }
}

//-----------------------------------------------------------------------------
//	湿度PID计算
//----------------------------------------------------------------------------

int pid_calc2(int pv, int sp)
{
    int en;
    // int en;

    int xp;
    int xi;
    int xd;

    int vn;
    int wn;
    int yn;

    int pidv;

    static int vn_1 = 0L;
    static int wn_1 = 0L;
    static int yn_1 = 0L;

    static int sum_en = 0L;

    int iv, jv;

    // if (ERR_PT3 || pv < 0)
    // return (0);		// (*Outer[2]) * 100;
    en = (sp - pv); // Difference
    zlog_error(g_zlog_zc, "[PID湿度]\t HUMI[EN]:[%d]", en);

    //------------------------------------------------
    //	P:
    //------------------------------------------------
    zlog_error(g_zlog_zc, "[PID湿度]\t HUMI[P]:[%d]", get_x10_val(P_HUM_P));

    if (get_x10_val(P_HUM_P) > 0)
    {
        if (abs(en) <= get_x10_val(P_HUM_P) / 2)
        {
            xp = (10000L) * en / get_x10_val(P_HUM_P) + (5000L);
        }
        else if (sp > pv)
        {
            xp = 10000L;
        }
        else
        {
            xp = 0L;
        }
    }
    else
    {
        xp = 0L;
    }

    if (xp < (0L))
        xp = 0L;
    if (xp > (10000L))
        xp = (10000L);

    zlog_error(g_zlog_zc, "[PID湿度]\t HUMI[XP]:[%d]", xp);

    // XP[1] = xp;

    //------------------------------------------------
    //	I:
    //------------------------------------------------
    zlog_error(g_zlog_zc, "[PID湿度]\t HUMI[I]:[%d]", get_e10_val(P_HUM_I));
    if (get_e10_val(P_HUM_I) > 0 && abs(en) <= get_x10_val(P_HUM_P) / 2)
    {
        iv = (5000L); // iv=(10000L)*(SAMPLE/SCONST)/(1000L);

        // pid_humi_sum_en += en;
        set_int(PID_HUMI_SUM, get_int(PID_HUMI_SUM) + en);         // -200000L ~ 300000L
        xi = (50L) * get_int(PID_HUMI_SUM) / get_e10_val(P_HUM_I); // xi=iv*sum_en/gMain_I/ 100L;
    }
    else
    {
        xi = 0L;
        // pid_humi_sum_en = 0L;
        set_int(PID_HUMI_SUM, 0);
    }

    if (xi < (-10000L))
        xi = (-10000L);
    if (xi > (10000L))
        xi = (10000L);

    //------------------------------------------------
    //	Note:
    //------------------------------------------------
    vn = xp + xi; // -10000L ~ 20000L
    if ((vn > (15000L)) || (vn < (-5000L)))
    {
        // pid_humi_sum_en -= en;
        set_int(PID_HUMI_SUM, get_int(PID_HUMI_SUM) - en);
    }
    zlog_error(g_zlog_zc, "[PID湿度]\t HUMI[XI]:[%d]", xi);
    zlog_error(g_zlog_zc, "[PID湿度]\t HUMI[SUM]:[%d]", get_int(PID_HUMI_SUM));
    zlog_error(g_zlog_zc, "[PID湿度]\t HUMI[VN]:[%d]", vn);

    // XI[1] = xi;
    // SUM[1]=pid_humi_sum_en;
    // VN[1] = vn;

    //------------------------------------------------
    //	D:
    //------------------------------------------------
    zlog_error(g_zlog_zc, "[PID湿度]\t HUMI[D]:[%d]", get_e10_val(P_HUM_D));
    if ((get_e10_val(P_HUM_D)) > 0 && abs(en) <= get_x10_val(P_HUM_P) / 2)
    {

        xd = (2L) * (get_e10_val(P_HUM_D)) * (vn - vn_1); //	-30000 ~ 30000	// xd=1000L*gMain_D*(vn - vn_1)/(SAMPLE/SCONST);
    }
    else
    {
        xd = 0L;
    }

    //------------------------------------------------
    //	result:
    //------------------------------------------------
    wn = vn + xd; // -1820000 ~ 1820000

    zlog_error(g_zlog_zc, "[PID湿度]\t HUMI[XD]:[%d]", xd);
    zlog_error(g_zlog_zc, "[PID湿度]\t HUMI[WN]:[%d]", wn);
    // XD[1] = xd;
    // WN[1] = wn;

    iv = wn + ((get_e10_val(P_HUM_D)) * 100 * wn_1) / 500; // PID����//iv = wn + (gWet_D * 100 / (SAMPLE/SCONST)) * wn_1;
    jv = 1L + ((get_e10_val(P_HUM_D)) * 100) / 500;        // PID��ĸ//jv = 1L + (gWet_D * 100 / (SAMPLE/SCONST));
    yn = iv / jv;

    //------------------------------------------------
    //
    //------------------------------------------------
    vn_1 = vn;
    // VN_1[1] = vn_1;
    wn_1 = wn;
    yn_1 = yn;

    //------------------------------------------------
    //	limits:
    //------------------------------------------------
    if (yn > (10000L))
    {
        pidv = 10000;
    }
    else if (yn < 0)
    {
        pidv = 0;
    }
    else
    {
        pidv = yn;
    }

    //------------------------------------------------
    //
    //------------------------------------------------
    pidv = (pidv - 5000) * 2; // -10000 ~ 10000

    if (pidv > ((get_val(P_HUM_OHI)) * 100))
    { // 6000
        pidv = ((get_val(P_HUM_OHI)) * 100);
    }
    if (pidv < ((get_val(P_HUM_OLO)) * 100))
    { // 0
        pidv = ((get_val(P_HUM_OLO)) * 100);
    }

    return (pidv);
}

int spray_duty(int pid)
{
    int duty;
    short point_stp = 0;

    unsigned int pid_humi_curr_main = pid_filter(pid, (int *)&shm_out[FILTER_PID_HUMI_ARR], 50);
    duty = (pid_humi_curr_main / 10);

    if (duty > (10L) * (get_val(P_HUM_OHI)))
    {
        duty = (10L) * (get_val(P_HUM_OHI)); // 600
    }
    else if (duty < (10L) * (get_val(P_HUM_OLO)))
    {
        duty = (10L) * (get_val(P_HUM_OLO)); // 0
    }

    //--------------------------------------------------------------------
    //
    //--------------------------------------------------------------------
    if (get_val(P_HUM_SPR) < (get_x10_val(P_HUM_P) / 2))
    {
        point_stp = get_val(P_HUM_SPR);
    }
    else
    {
        point_stp = get_x10_val(P_HUM_P) / 2;
    }

    if (get_val(R_AI_HM) < (get_val(P_AO_HM) - (get_x10_val(P_HUM_P) / 2)))
    {
        return ((10L) * (get_val(P_HUM_OHI))); // 600
    }

    if (get_val(R_AI_HM) > (get_val(P_AO_HM)))
    {
        return (0); // (10L)*(*Outlo[2])
    }

    if ((get_val(R_AI_HM) >= (get_val(P_AO_HM) - (get_x10_val(P_HUM_P) / 2))) && (get_val(R_AI_HM) <= (get_val(P_AO_HM) + point_stp)))
    {
        return (duty);
    }
    return (0);
}
