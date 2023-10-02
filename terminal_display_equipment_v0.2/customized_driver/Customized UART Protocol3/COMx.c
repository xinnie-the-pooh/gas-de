#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>
#include <sys/time.h>
#include <time.h>
#include <stdbool.h>
#include <sys/sem.h>
#include <string.h>
#include <stdint.h>

void Delay(int ms);
int ReadData(int fd, unsigned char *read_buff, int count, int ms);
int WriteData(int fd, unsigned char *read_buff, int count);
unsigned short Read_LW(unsigned int n);
void Write_LW(unsigned int n, unsigned short val);
void CopyToLW(unsigned int offset, const void *src, int n);
void CopyFromLW(unsigned int offset, const void *src, int n);
void SetLBON(unsigned int n);
void SetLBOFF(unsigned int n);
int GetLB(unsigned int n);
int SetLWBON(unsigned int n, unsigned int offset);
int SetLWBOFF(unsigned int n, unsigned int offset);
int GetLWB(unsigned int n, unsigned int offset);
int Read_Rw(void *buf, int addr, int nWords);
int Write_Rw(const void *buf, int addr, int nWords);
int GetRB(unsigned int addr);
int SetLRBON(unsigned int addr);
int SetLRBOFF(unsigned int addr);

#define SMSEND_RESPONSE1					"usr.cn#\r\nOK\r\n"
#define SMSEND_RESPONSE2					"\r\nSMSEND OK\r\n"

int gs_uart_send_message(int fd, const unsigned char *phone_number, const unsigned char *msg)
{
	unsigned char buff[1024];
	uint8_t ret = 0;
	int len;
	char *p = NULL;

	memset(buff, '\0' , sizeof(buff));
	sprintf(buff, "usr.cn#AT+SMSEND=\"%s\",3,\"%s\"\r", phone_number, msg);
	WriteData(fd, buff, strlen(buff));

	Delay(1000);
	memset(buff, '\0' , sizeof(buff));
	len = ReadData(fd, buff, strlen(SMSEND_RESPONSE1), 2000);
	if (len >= strlen(SMSEND_RESPONSE1))
	{
		p = strstr(buff, SMSEND_RESPONSE1);
		if (NULL == p)
		{
			ret |= 0x01;
		}
	}
	else
	{
		ret |= 0x02;
	}

	Delay(5000);
	memset(buff, '\0' , sizeof(buff));
	len = ReadData(fd, buff, strlen(SMSEND_RESPONSE2), 5000);
	if (len >= strlen(SMSEND_RESPONSE2))
	{
		p = strstr(buff, SMSEND_RESPONSE2);
		if (NULL == p)
		{
			ret |= 0x04;
		}
	}
	else
	{
		ret |= 0x08;
	}

	if (ret == 0)
	{
		return (0);
	}
	return (-1);
}

int gs_find_special_byte(uint8_t *buff, int len, uint8_t byte)
{
	int i;
	for (i = 0; i < len; i++)
	{
		if (buff[i] == byte)
		{
			return (i);
		}
	}
	return (-1);
}

void ProcessComx(int fd)
{
	char tmp[16];
	char chn_msg[16];
	char phonenum[16];
	char msg[1024];
	char tmp1[32];
	int len;
	int index;
	uint8_t warning_chn = 0;
	static uint8_t last_warning_chn = 0;
	static uint8_t status = 0;
	static unsigned short retry_cnt = 0;

	while (1)
	{
		Delay(100);
		memset(tmp, '\0', sizeof(tmp));
		CopyFromLW(220, tmp, 12);
		if (tmp[0] != '1')
		{
			SetLBON(45);
			WriteData(fd, tmp1, strlen(tmp1));
		}
		else
		{
			index = gs_find_special_byte(tmp, 12, 0x20);
			if (index != 11)
			{
				SetLBON(45);
			}
			else
			{
				tmp[11] = '\0';
				memset(phonenum, '\0', sizeof(phonenum));
				sprintf(phonenum, "%s", tmp);
				SetLBOFF(45);
			}
		}

		//! 启用短信功能.
		if (1 == GetLWB(1, 0))
		{
			warning_chn = 0;
			memset(chn_msg, '\0', sizeof(chn_msg));
			if ((0 != Read_LW(250)) && (1 == GetLWB(0, 0)))
			{
				warning_chn |= 0x01;
				memcpy(&chn_msg[strlen(chn_msg)], "1 ", strlen("1 "));
			}
			if ((0 != Read_LW(251)) && (1 == GetLWB(0, 1)))
			{
				warning_chn |= 0x02;
				memcpy(&chn_msg[strlen(chn_msg)], "2 ", strlen("2 "));
			}
			if ((0 != Read_LW(252)) && (1 == GetLWB(0, 2)))
			{
				warning_chn |= 0x04;
				memcpy(&chn_msg[strlen(chn_msg)], "3 ", strlen("3 "));
			}
			if ((0 != Read_LW(253)) && (1 == GetLWB(0, 3)))
			{
				warning_chn |= 0x08;
				memcpy(&chn_msg[strlen(chn_msg)], "4 ", strlen("4 "));
			}

			if ((0 != warning_chn) && (warning_chn != last_warning_chn))
			{
				if ((retry_cnt >= Read_LW(211)) || (retry_cnt >= 5))
				{
//					retry_cnt = 0;
					last_warning_chn = warning_chn;
				}
				else
				{
					memset(msg, '\0', sizeof(msg));
					sprintf(msg, "工作站号：%d，当前通道(%s)气体浓度处于危险值，请注意！", Read_LW(210), chn_msg);
					if (0 == gs_uart_send_message(fd, (unsigned char *)phonenum, (unsigned char *)msg))
					{
//						retry_cnt++;
					}
					else
					{

					}
					retry_cnt++;
				}
//				memset(msg, '\0', sizeof(msg));
//				sprintf(msg, "Read_LW(211)=%d, retry_cnt=%d, warning_chn=%02X, last_warning_chn=%02X\r\n", 
//						Read_LW(211), retry_cnt, warning_chn, last_warning_chn);
//				WriteData(fd, msg, strlen(msg));
			}
			else
			{
				retry_cnt = 0;
			}
		}
		else
		{
			retry_cnt = 0;
		}
	}
}

