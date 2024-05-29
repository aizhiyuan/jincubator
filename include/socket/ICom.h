/*
 * ICom.h
 *
 *  
 *     
 */

#ifndef ICOM_H_
#define ICOM_H_
#include "stdio.h"
#include "stdlib.h"
#include "unistd.h"
#include "sys/types.h"
#include "sys/stat.h"
#include "fcntl.h"
#include <termios.h>
#include "errno.h"
#include "string.h"
#include <pthread.h>
#include "sys/time.h"
#include "sys/epoll.h"
#include <sys/ioctl.h>  
#include <asm/ioctls.h>

#define MAXEPOLLEVENTS 20
#define MAXLEN         1024
#define MAXCOMNUM      4

#define RS232 		232
#define RS422		422
#define RS485H		4852
#define RS485F		4854


class ICom 
{
public:
	ICom();
	virtual ~ICom();

	bool ComOpen(const int v_i_com_port);//根据端口号打开com口，并创建接收线程
	bool ComSet(const int v_i_baud_rate, const int v_i_data_bits, const int v_i_parity, const int v_i_stop_bits);

    int ComWrite(char *v_pch_writebuff,const int v_i_writelen);
    int ComRead(char * v_pch_readbuff,const int v_i_readlen,const int v_i_timeoutms);
    int ComRead(char * v_pch_readbuff,const int v_i_readlen,const int v_i_timeoutms,const int v_i_intervalms); 
    int ComNonBlockRead(char * v_pch_readbuff,const int v_i_readlen);
    int ComBlockRead(char * v_pch_readbuff,const int v_i_readlen);
    int ComFlush(void);
    int ComSetRTS(const int v_i_status);
    int ComSetInterface(const int v_i_interface);

    virtual void ReadDataProc(void);//数据的处理函数。

    int m_i_port;

private:
	int m_i_fd;//文件描述符
	int m_i_epid; //epoll标识符
	epoll_event m_event;
	epoll_event m_events[MAXEPOLLEVENTS];//事件集合
	char m_ch_recvbuff[MAXLEN];//接受到的数据
	pthread_t m_pid;//接受数据线程的Id
	static void * ReadThreadFunction(void * v_p_arg);//接受数据的线程函数
	void ComClose();//关闭com口

};

#endif /* ICOM_H_ */
