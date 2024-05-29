/*
 * @Author: chiyuan shaome@foxmail.com
 * @Date: 2023-07-21 15:00:00
 * @LastEditors: chiyuan shaome@foxmail.com
 * @LastEditTime: 2023-07-21 15:21:17
 * @FilePath: /c/jpearl/src/serial.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
/*---------------------------------------------------------------------------*/
/**
  @file		serial.h
  @brief	Serial API header file

  Serial utility functions, it helps programmer easily operating serial port.
  It provides mostly standard functions we usually use, and SerialSetMode()
  is the private function can be use in UC box.
 */
/*---------------------------------------------------------------------------*/

#ifndef SERIAL_H
#define SERIAL_H

#include "jlib.h"
#include <asm/ioctls.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h> //g++ compiler need
#include <termios.h>
#include <unistd.h>


#define PORT1 0
#define PORT2 1
#define PORT3 2
#define PORT4 3
#define PORT5 4
#define PORT6 5
#define PORT7 6
#define PORT8 7

#define MAX_PORT_NUM 16

#define NO_FLOW_CONTROL 0
#define HW_FLOW_CONTROL 1
#define SW_FLOW_CONTROL 2

#define SERIAL_OK 0
#define SERIAL_ERROR_FD                                                        \
  -1 ///< Could not find the fd in the map, device not opened
#define SERIAL_ERROR_OPEN                                                      \
  -2 ///< Could not open the port or port has been opened
#define SERIAL_PARAMETER_ERROR -3 ///< Not available parameter

#define RS232 232
#define RS422 422
#define RS485H 4852
#define RS485F 4854

int SerialOpen(const char *cDeviceName);
int SerialFlush(int fd);
int SerialWrite(int fd, void *pvData, int iLen);
int SerialNonBlockRead(int fd, char *buf, int len);
int SerialBlockRead(int fd, char *buf, int len);
int SerialRead(int fd, unsigned char *buf, int len, int waitms, int timeoutms);
int SerialClose(int fd);
int SerialDataInInputQueue(int fd);
int SerialDataInOutputQueue(int fd);
int SerialFlowControl(int fd, int control);
int SerialSetSpeed(int fd, unsigned int speed);
int SerialSetParam(int fd, int parity, int databits, int stopbit);
int SerialSetInterface(int fd, unsigned int interface);
int SerailSetRts(int fd, int set);

#endif
