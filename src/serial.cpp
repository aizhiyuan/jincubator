/*---------------------------------------------------------------------------*/
/**
  @file		serial.c
  @brief	Serial API define file

  Serial utility functions, it helps programmer easily operating serial port.
  It provides mostly standard functions we usually use, and SerialSetMode()
  is the private function can be use in UC box.

 */
/*---------------------------------------------------------------------------*/

#include "serial.h"

/*---------------------------------------------------------------------------*/
/**
  @brief	open serial port
  @param	cDeviceName		serial port name
  @return	return fd for success, on error return error code
 */
/*---------------------------------------------------------------------------*/
int	SerialOpen(const char * cDeviceName) 
{
	int fd;
	struct termios tio;

	char c_device[100];
	sprintf(c_device,"/dev/%s", cDeviceName);

	fd = open(c_device, O_RDWR|O_NOCTTY);
	if(fd <0)
		return SERIAL_ERROR_OPEN;

	//bzero( &tio, sizeof(tio));		///< clear struct for new port settings
        memset(&tio, 0, sizeof(tio));	

	tio.c_iflag = 0;
	tio.c_oflag = 0;
	tio.c_cflag = B9600|CS8|CREAD|CLOCAL;
	tio.c_lflag = 0;

	tio.c_cc[VTIME] = 0;			///< inter-character timer unused
	tio.c_cc[VMIN] = 1;			///< blocking read until 1 character arrives

	tcflush(fd, TCIFLUSH);
	tcsetattr(fd, TCSANOW, &tio);

	return fd;
}

int	SerialFlush(int fd) 
{
	int iRet;

	if(fd < 0)			///< error
		return fd;

    iRet = tcflush(fd, TCIFLUSH);
	//if(iRet < 0)  iRet = write( fd, str, len);  //200805 microlee  try again  
	return iRet;
}

/*---------------------------------------------------------------------------*/
/**
  @brief	write to serial port
  @param	fd		port file
  @param	str		string to write
  @param	len		length of str
  @return	return length of str for success, on error return error code
 */
/*---------------------------------------------------------------------------*/
int	SerialWrite(int fd, void* pvData, int iLen) 
{
	int iRet;

	if(fd < 0)			///< error
		return SERIAL_ERROR_FD;

    char * pcTmp = (char*) pvData;
    iRet = write(fd, pvData, iLen);
	return iRet;
}

/*---------------------------------------------------------------------------*/
/**
  @brief	non-block read from serial port
  @param	fd		port file
  @param	buf		input buffer
  @param	len		buffer length
  @return	return length of read str for success,
  		on error return error code
 */
/*---------------------------------------------------------------------------*/
int	SerialNonBlockRead(int fd, char* buf, int len) 
{
	int res = 0;
	int bytes = 0;

	if(fd < 0)			///< error
		return SERIAL_ERROR_FD;

	fcntl(fd, F_SETFL, FNDELAY);
	res = read(fd, buf, len);
	return res;
}

/*---------------------------------------------------------------------------*/
/**
  @brief	block read from serial port
  @param	fd		port number
  @param	buf		input buffer
  @param	len		buffer length
  @return	return length of read str for success,
  		on error return error code
 */
/*---------------------------------------------------------------------------*/
int	SerialBlockRead(int fd, char* buf, int len) 
{
	int res = 0;
	int bytes = 0;

	if(fd < 0)			///< error
		return SERIAL_ERROR_FD;

	fcntl(fd, F_SETFL, 0);
	res = read(fd, buf, len);
	return res;
}

/*---------------------------------------------------------------------------*/
/**
  @brief	read from serial port
  @param	fd		port number
  @param	buf		input buffer
  @param	len		buffer length
  @param	waitms		waiting ms then bigen to read
  @param	timeoutms		
  @return	return length of read str for success,
  		on error return error code
 */
/*---------------------------------------------------------------------------*/
int SerialRead(int fd, unsigned char* buf, int len, int waitms, int timeoutms) 
{
	int retval;
	fd_set rfds;
	struct timeval tv;
	int ret;
	int pos;

	if (fd < 0)			///< error
		return SERIAL_ERROR_FD;

	fcntl(fd, F_SETFL, FNDELAY);

	tv.tv_sec = waitms / 1000; // wait 2.5s
    tv.tv_usec = waitms % 1000;
	tv.tv_usec = tv.tv_usec * 1000;
	pos = 0; // point to rceeive buf

    while (1) 
    {
		FD_ZERO(&rfds);
        FD_SET(fd, &rfds);
		retval = select(fd + 1, &rfds, NULL, NULL, &tv);
        if(retval == -1) 
        {
			//perror("select()");
			//printf("select error\n");
			break;
        }
		else if (retval) 
		{
            // pan duan shi fou hai you shu ju
			ret = read(fd, buf + pos, len - pos);
			//printf("receive data:%d\n",ret);
            pos += ret;
            if (pos < len) 
            {
                tv.tv_sec = timeoutms / 1000; // waiting frame timeout
				tv.tv_usec = timeoutms % 1000;
				tv.tv_usec = tv.tv_usec * 1000;
	            //printf("waiting timeout %d us \n",tv.tv_usec);
				continue;
			}
			else 
			{
				//printf("%d data\n",pos);
				break;
			}
		}
		else 
		{
			//printf("No data\n");
            break;
        }  
	}
	return pos;
}




/*---------------------------------------------------------------------------*/
/**
  @brief	close serial port
  @param	fd		port number
  @return	return SERIAL_OK for success, on error return error code
 */
/*---------------------------------------------------------------------------*/
int	SerialClose(int fd) 
{

	if (fd < 0)			///< error
		return SERIAL_ERROR_FD;
	close(fd);

	return SERIAL_OK;
}

/*---------------------------------------------------------------------------*/
/**
  @brief	test how much data in input queue
  @param	fd		port number
  @return	return number of data to be read for success,
  		on error return error code
 */
/*---------------------------------------------------------------------------*/
int	SerialDataInInputQueue(int fd) 
{
	int bytes = 0;

	if (fd < 0)			///< error
		return SERIAL_ERROR_FD;

	ioctl(fd, FIONREAD, &bytes);
	return bytes;
}

/*---------------------------------------------------------------------------*/
/**
  @brief	test how much data in output queue
  @param	fd		port number
  @return	return number of data to be write for success,
  		on error return error code
 */
/*---------------------------------------------------------------------------*/
int	SerialDataInOutputQueue(int fd) 
{
	int bytes = 0;

	if (fd < 0)			///< error
		return SERIAL_ERROR_FD;

	ioctl(fd, TIOCOUTQ, &bytes);
	return bytes;
}
/*---------------------------------------------------------------------------*/
/**
  @brief	set flow control
  @param	fd		port number
  @param	control		NO_FLOW_CONTROL/HW_FLOW_CONTROL/SW_FLOW_CONTROL
  @return	return SERIAL_OK for success, on error return error code
 */
/*---------------------------------------------------------------------------*/
int	SerialFlowControl(int fd, int control) 
{
	struct termios tio;
	
	if (fd < 0)			///< error
		return SERIAL_ERROR_FD;

	tcgetattr(fd, &tio);	///< save current serial port settings

	if (control == NO_FLOW_CONTROL) 
	{
		tio.c_cflag &= !CRTSCTS;
		tio.c_iflag &= !(IXON | IXOFF | IXANY);
	}
	else if (control == HW_FLOW_CONTROL)
		tio.c_cflag |= CRTSCTS;
	else if (control == SW_FLOW_CONTROL)
		tio.c_iflag |= (IXON | IXOFF | IXANY);
	else
		return SERIAL_PARAMETER_ERROR;

	tcflush(fd, TCIFLUSH);
	tcsetattr(fd, TCSANOW, &tio);

	return SERIAL_OK;
}

/*---------------------------------------------------------------------------*/
/**
  @brief	set serial speed and make changes now
  @param	fd		port number
  @param	speed		unsigned integer for new speed
  @return	return SERIAL_OK for success, on error return error code
 */
/*---------------------------------------------------------------------------*/
int	SerialSetSpeed(int fd, unsigned int speed)
{
	struct termios tio;
	int i, table_size= 23;
	int speed_table1[]={ 0, 50, 75, 110, 134, 150, 200, 300,
			     600, 1200, 1800, 2400, 4800, 9600,
			     19200, 38400, 57600, 115200};/*, 230400,
			     460800, 500000, 576000, 921600};*/
	int speed_table2[]={ B0, B50, B75, B110, B134, B150, B200, B300,
			     B600, B1200, B1800, B2400, B4800, B9600,
			     B19200, B38400, B57600, B115200};/*, B230400,
			     B460800, B500000, B576000, B921600};*/
	

	if (fd < 0)			///< error
		return SERIAL_ERROR_FD;

	for (i = 1; i < table_size; i++)	///< i start from 1, bellow 50 will be set to B0
		if (speed_table1[i] >= speed)
            break;
	
	tcgetattr(fd, &tio);	///< save current serial port settings
	cfsetispeed(&tio, speed_table2[i]);
	cfsetospeed(&tio, speed_table2[i]);
	tcsetattr(fd, TCSANOW, &tio);

	return SERIAL_OK;
}


/*---------------------------------------------------------------------------*/
/**
  @brief	set serial port parameter
  @param	fd		port number
  @param	parity		parity check, 0: none, 1: odd, 2: even, 3: space, 4: mark
  @param	databits	data bits
  @param	stopbit		stop bit
  @return	return fd for success, on error return error code
 */
/*---------------------------------------------------------------------------*/
int	SerialSetParam(int fd, int parity, int databits, int stopbit)
{
	struct termios tio;
	if (fd < 0)			///< error
		return SERIAL_ERROR_FD;
	
	tcgetattr(fd, &tio);	///< save current serial port settings
	if (parity == 0) {
		tio.c_cflag &= ~PARENB;
		tio.c_iflag &= ~INPCK;
	}
	else if (parity == 1) {
		tio.c_cflag |= PARENB;
		tio.c_cflag |= PARODD;
		tio.c_iflag |= INPCK;
	}
	else if (parity == 2) {
		tio.c_cflag |= PARENB;
		tio.c_cflag &= ~PARODD;
	}
	else if (parity == 3) {
		tio.c_cflag &= ~PARENB;
		tio.c_cflag &= ~CSTOPB;
	}
	else if (parity == 4) {
		tio.c_cflag |= CSTOPB;
		tio.c_cflag &= ~PARENB;
		tio.c_iflag &= ~INPCK;
	}

	if (databits == 5)
		tio.c_cflag |= CS5;
	else if (databits == 6)
		tio.c_cflag |= CS6;
	else if (databits == 7)
		tio.c_cflag |= CS7;
	else if (databits == 8)
		tio.c_cflag |= CS8;

	if (stopbit == 1)
		tio.c_cflag &= ~CSTOPB;
	else if (stopbit == 2)
		tio.c_cflag |= CSTOPB;

	tcflush(fd, TCIFLUSH);
	tcsetattr(fd, TCSANOW, &tio);

	return SERIAL_OK;
}

///*---------------------------------------------------------------------------*/
///**
//  @brief	set serial port mode for RS232/RS422/RS485
//  @param	port		port number
//  @param	mode		serial port mode
//  		{RS232_MODE/RS485_2WIRE_MODE/RS422_MODE/RS485_4WIRE_MODE}
//  @return	return SERIAL_OK for success, on error return error code
// */
///*---------------------------------------------------------------------------*/
int SerialSetInterface( int fd, unsigned int interface)
{
	int ret;
    /*int fd = FindFD(port);
	if (fd < 0)			///< error
		return fd;

	switch (interface)
    {
    case RS422:
        printf("Set serial port%d RS422\n",port+1);
        ioctl(fd, TCSET422, port);
        ioctl(fd, TCSET422, port);
        ioctl(fd, TCSET422, port);
		ret = 422;
        break;
    case RS485H:
        printf("Set serial port%d RS485 2wire\n",port+1);
        ioctl(fd, TCSET485, port);
        ioctl(fd, TCSET485, port);
        ioctl(fd, TCSET485, port);
		ret = 4852;
        break;

    case RS485F:
        printf("Set serial port%d RS485 4wire\n",port+1);
		ioctl(fd, TCSET422, port);
        ioctl(fd, TCSET422, port);
        ioctl(fd, TCSET422, port);
		ret = 4854;
        break;
    default:
        printf("Set serial port%d RS232\n",port+1);
		ioctl(fd, TCSET550, port);
        ioctl(fd, TCSET550, port);
        ioctl(fd, TCSET550, port);
		ret = 232;
        break;
    }*/
		
	return ret;
}

int SerailSetRts(int fd, int set) 
{
	int status;
	
	if (fd < 0)			///< error
		return SERIAL_ERROR_FD;  
	    
    ioctl(fd, TIOCMGET, &status);
    if (set) {
		status|=TIOCM_RTS;
    }
    else {
		status&=~TIOCM_RTS;
    }
	ioctl(fd, TIOCMSET, &status);
	return 1;
}
