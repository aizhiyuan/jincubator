#include "jshmem.h"

//----------------------------------------------------------------------------------------------------------
// 共享内存初始化
//----------------------------------------------------------------------------------------------------------
int j_shmem_init(jshmem_t *pjshmem, key_t v_i_key, size_t v_ul_size)
{
    if ((pjshmem->v_shmem_id = shmget(v_i_key, v_ul_size, IPC_CREAT|0666)) == -1)
    {
        return -1;
    }
    else
    {
        pjshmem->uptr_shmem_addr = (unsigned short *)shmat(pjshmem->v_shmem_id, NULL, 0);
        if (pjshmem->uptr_shmem_addr == (unsigned short *)-1)
        {
            return -1;
        }
    }


    return 0;
}

//----------------------------------------------------------------------------------------------------------
// 释放共享内存指针
//----------------------------------------------------------------------------------------------------------
int j_shmem_destroy(jshmem_t *pjshmem)
{
    return shmdt(pjshmem->uptr_shmem_addr);
}

//----------------------------------------------------------------------------------------------------------
// 删除共享内存
//----------------------------------------------------------------------------------------------------------
int j_shmem_del(jshmem_t *pjshmem)
{
    return shmctl(pjshmem->v_shmem_id, IPC_RMID, NULL);
}

//----------------------------------------------------------------------------------------------------------
// 共享内存转无符号整型
//----------------------------------------------------------------------------------------------------------
unsigned short shm2ushort(unsigned short *shm_addr)
{
    unsigned short us_shm_data;
    memcpy(&us_shm_data, shm_addr, sizeof(unsigned short));
    ba(&us_shm_data, sizeof(unsigned short));
    return us_shm_data;
}

//----------------------------------------------------------------------------------------------------------
// 共享内存转整型
//----------------------------------------------------------------------------------------------------------
short shm2short(unsigned short *shm_addr)
{
    short us_shm_data;
    memcpy(&us_shm_data, shm_addr, sizeof(unsigned short));
    ba(&us_shm_data, sizeof(unsigned short));
    return us_shm_data;
}

//----------------------------------------------------------------------------------------------------------
// 共享内存转浮点数
//----------------------------------------------------------------------------------------------------------
float ushm2float(unsigned short *shm_addr)
{
    unsigned short us_shm_data;
    float f_shm_data;
    us_shm_data = shm2ushort(shm_addr);
    f_shm_data = (float)us_shm_data / 100.0;
    return f_shm_data;
}

//----------------------------------------------------------------------------------------------------------
// 共享内存转浮点数
//----------------------------------------------------------------------------------------------------------
float shm2float(unsigned short *shm_addr)
{
    short us_shm_data;
    float f_shm_data;
    us_shm_data = shm2short(shm_addr);
    f_shm_data = (float)us_shm_data / 100.0;
    return f_shm_data;
}

//----------------------------------------------------------------------------------------------------------
// 共享内存转浮点数(只除于10)
//----------------------------------------------------------------------------------------------------------
float shm2floatpid(unsigned short *shm_addr)
{
    short us_shm_data;
    float f_shm_data;
    us_shm_data = shm2ushort(shm_addr);
    f_shm_data = (float)us_shm_data / 10.0;
    return f_shm_data;
}

//----------------------------------------------------------------------------------------------------------
// 共享内存转整数(只除于10)
//----------------------------------------------------------------------------------------------------------
short shm2shortpid(unsigned short *shm_addr)
{
    short s_shm_data;
    s_shm_data = shm2ushort(shm_addr);
    s_shm_data = s_shm_data / 10.0;
    return s_shm_data;
}

//----------------------------------------------------------------------------------------------------------
// 整数转共享内存（只乘于10）
//----------------------------------------------------------------------------------------------------------
short pidshm2short(unsigned short *shm_addr)
{
    short s_shm_data;
    s_shm_data = shm2ushort(shm_addr);
    s_shm_data = s_shm_data * 10.0;
    return s_shm_data;
}

//----------------------------------------------------------------------------------------------------------
// 无符号整型转为共享内存
//----------------------------------------------------------------------------------------------------------
unsigned short ushort2shm(unsigned short us_data)
{
    ba(&us_data, sizeof(unsigned short));
    return us_data;
}

//----------------------------------------------------------------------------------------------------------
// 浮点数转为共享内存
//----------------------------------------------------------------------------------------------------------
unsigned short float2shm(float f_data)
{
    unsigned short us_data;
    us_data = (unsigned short)((float)f_data * 100.0);
    ba(&us_data, sizeof(unsigned short));
    return us_data;
}

//----------------------------------------------------------------------------------------------------------
// 双精度浮点数转为共享内存
//----------------------------------------------------------------------------------------------------------
void double2shm(unsigned short *addr, double data)
{
    ba(&data, sizeof(double));
    memcpy(addr, &data, sizeof(double));
}

//----------------------------------------------------------------------------------------------------------
// 整型转为共享内存
//----------------------------------------------------------------------------------------------------------
unsigned short short2shm(short us_data)
{
    ba(&us_data, sizeof(unsigned short));
    return us_data;
}

//----------------------------------------------------------------------------------------------------------
// 共享内存转双精度浮点
//----------------------------------------------------------------------------------------------------------
double shm2double(unsigned short *shm_addr)
{
    double d_shm_data;
    memcpy(&d_shm_data, shm_addr, sizeof(double));
    ba(&d_shm_data, sizeof(double));
    return d_shm_data;
}