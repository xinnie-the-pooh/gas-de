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

typedef struct
{
	uint8_t id;
	uint8_t status;
	uint32_t framenum;
	float battery_volt;
	float jxm_o2;
	float jxm_co;
	float jxm_h2s;
	float jxm_lel;
} JXM_DATA;

typedef union
{
	struct
	{
		uint8_t buff[4];
	} bytes;
	float val;
}FLOAT_UNION;

static uint8_t buttom[4] = { 0 };
static uint16_t timesout[4] = { 0 };

uint16_t gs_calc_cs(uint8_t *buff, int len)
{
	int i;
	uint16_t cs = 0;

	for (i = 0; i < len; i++)
	{
		cs += *(buff + i);
	}
	return (cs);
}

#define JXM_CH1_BASE_ADDR			(300)
#define JXM_CH2_BASE_ADDR			(313)
#define JXM_CH3_BASE_ADDR			(326)
#define JXM_CH4_BASE_ADDR			(339)
unsigned int num = 0; 
int gs_uart_recv_message(int fd, JXM_DATA *jxmdata)
{
	int i;
	int ret;
	int dlen;
	uint8_t cs;
	uint8_t flag = 0;
	FLOAT_UNION fu;
	unsigned int baseaddr;
	unsigned char tmp[256];
	static unsigned char buff[1024];
	static int len = 0;

	memset(tmp, '\0', sizeof(tmp));
	ret = ReadData(fd, tmp, 32, 10);

	if (ret > 0)
	{
		memcpy(&buff[len], tmp, ret);
		len += ret;
	}

	if (len >= 30)
	{
		for (i = 0; i < len; i++)
		{
			if ((buff[i] == 0x68) && (buff[i + 2] == 0x80))
			{
				jxmdata->id = buff[i + 1];
				dlen = buff[i + 3];

				if (dlen > 0)
				{
					cs = (uint8_t)gs_calc_cs(&buff[i + 4], dlen);
				}
				if ((cs == buff[i + 4 + dlen]) && (0x16 == buff[i + 5 + dlen]))
				{
					if ((1 == GetLWB(0, 0)) && (jxmdata->id == Read_LW(200)))
					{
						//! CHN1.
						baseaddr = JXM_CH1_BASE_ADDR;
						flag = 1;
						
						jxmdata->status = buff[i + 4];
						buttom[0] = jxmdata->status;
						timesout[0] = 0;
					}
					else if ((1 == GetLWB(0, 1)) && (jxmdata->id == Read_LW(201)))
					{
						//! CHN2.
						baseaddr = JXM_CH2_BASE_ADDR;
						flag = 1;
						jxmdata->status = buff[i + 4];
						buttom[1] = jxmdata->status;
						timesout[1] = 0;
					}
					else if ((1 == GetLWB(0, 2)) && (jxmdata->id == Read_LW(202)))
					{
						//! CHN3.
						baseaddr = JXM_CH3_BASE_ADDR;
						flag = 1;
						jxmdata->status = buff[i + 4];
						buttom[2] = jxmdata->status;
						timesout[2] = 0;
					}
					else if ((1 == GetLWB(0, 3)) && (jxmdata->id == Read_LW(203)))
					{
						//! CHN4.
						baseaddr = JXM_CH4_BASE_ADDR;
						flag = 1;
						jxmdata->status = buff[i + 4];
						buttom[3] = jxmdata->status;
						timesout[3] = 0;
					}
					else
					{
						flag = 0;
					}

					if (1 == flag)
					{
						Write_LW(baseaddr, jxmdata->id);

						jxmdata->framenum = buff[i + 5] | (buff[i + 6] << 8) | (buff[i + 7] << 16);
						CopyToLW(((baseaddr + 1)), &buff[i + 5], 3);

						memcpy(&fu.bytes, &buff[i + 8], 4);
						jxmdata->battery_volt = fu.val;
						CopyToLW(((baseaddr + 3)), fu.bytes.buff, 4);

						memcpy(&fu.bytes, &buff[i + 12], 4);
						jxmdata->jxm_o2 = fu.val;
						CopyToLW((baseaddr + 5), fu.bytes.buff, 4);

						memcpy(&fu.bytes, &buff[i + 16], 4);
						jxmdata->jxm_co = fu.val;
						CopyToLW((baseaddr + 7), fu.bytes.buff, 4);

						memcpy(&fu.bytes, &buff[i + 20], 4);
						jxmdata->jxm_h2s = fu.val;
						CopyToLW((baseaddr + 9), fu.bytes.buff, 4);

						memcpy(&fu.bytes, &buff[i + 24], 4);
						jxmdata->jxm_lel = fu.val;
						CopyToLW((baseaddr + 11), fu.bytes.buff, 4);
					}

					memcpy(buff, &buff[i + 6 + dlen], len - (i + 6 + dlen));

					len -= (i + 6 + dlen);
					return (0);
				}
			}
		}
	}
	return (-1);
}

void gs_process_display_alarm(void)
{
	FLOAT_UNION fu;

	//! 通道1, 电压和O2浓度上下限报警值设置.
	if (1 == GetLWB(0, 0))
	{
		//! 电池下限报警值.
		fu.val = 3.45f;
		CopyToLW(100, fu.bytes.buff, 4);

		//! 电池上限报警值.
		fu.val = 4.3f;
		CopyToLW(102, fu.bytes.buff, 4);

		//! O2下限报警值.
		fu.val = 19.5f;
		CopyToLW(104, fu.bytes.buff, 4);

		//! O2上限报警值.
		fu.val = 23.0f;
		CopyToLW(106, fu.bytes.buff, 4);
	}
	else
	{
		//! 电池下限报警值.
		fu.val = 0.0f;
		CopyToLW(100, fu.bytes.buff, 4);

		//! 电池上限报警值.
		fu.val = 4.3f;
		CopyToLW(102, fu.bytes.buff, 4);

		//! O2下限报警值.
		fu.val = 0.0f;
		CopyToLW(104, fu.bytes.buff, 4);

		//! O2上限报警值.
		fu.val = 23.0f;
		CopyToLW(106, fu.bytes.buff, 4);
	}

	//! 通道2, 电压和O2浓度上下限报警值设置.
	if (1 == GetLWB(0, 1))
	{
		//! 电池下限报警值.
		fu.val = 3.45f;
		CopyToLW(108, fu.bytes.buff, 4);

		//! 电池上限报警值.
		fu.val = 4.3f;
		CopyToLW(110, fu.bytes.buff, 4);

		//! O2下限报警值.
		fu.val = 19.5f;
		CopyToLW(112, fu.bytes.buff, 4);

		//! O2上限报警值.
		fu.val = 23.0f;
		CopyToLW(114, fu.bytes.buff, 4);
	}
	else
	{
		//! 电池下限报警值.
		fu.val = 0.0f;
		CopyToLW(108, fu.bytes.buff, 4);

		//! 电池上限报警值.
		fu.val = 4.3f;
		CopyToLW(110, fu.bytes.buff, 4);

		//! O2下限报警值.
		fu.val = 0.0f;
		CopyToLW(112, fu.bytes.buff, 4);

		//! O2上限报警值.
		fu.val = 23.0f;
		CopyToLW(114, fu.bytes.buff, 4);
	}

	//! 通道3, 电压和O2浓度上下限报警值设置.
	if (1 == GetLWB(0, 2))
	{
		//! 电池下限报警值.
		fu.val = 3.45f;
		CopyToLW(116, fu.bytes.buff, 4);

		//! 电池上限报警值.
		fu.val = 4.3f;
		CopyToLW(118, fu.bytes.buff, 4);

		//! O2下限报警值.
		fu.val = 19.5f;
		CopyToLW(120, fu.bytes.buff, 4);

		//! O2上限报警值.
		fu.val = 23.0f;
		CopyToLW(122, fu.bytes.buff, 4);
	}
	else
	{
		//! 电池下限报警值.
		fu.val = 0.0f;
		CopyToLW(116, fu.bytes.buff, 4);

		//! 电池上限报警值.
		fu.val = 4.3f;
		CopyToLW(118, fu.bytes.buff, 4);

		//! O2下限报警值.
		fu.val = 0.0f;
		CopyToLW(120, fu.bytes.buff, 4);

		//! O2上限报警值.
		fu.val = 23.0f;
		CopyToLW(122, fu.bytes.buff, 4);
	}

	//! 通道4, 电压和O2浓度上下限报警值设置.
	if (1 == GetLWB(0, 3))
	{
		//! 电池下限报警值.
		fu.val = 3.45f;
		CopyToLW(124, fu.bytes.buff, 4);

		//! 电池上限报警值.
		fu.val = 4.3f;
		CopyToLW(126, fu.bytes.buff, 4);

		//! O2下限报警值.
		fu.val = 19.5f;
		CopyToLW(128, fu.bytes.buff, 4);

		//! O2上限报警值.
		fu.val = 23.0f;
		CopyToLW(130, fu.bytes.buff, 4);
	}
	else
	{
		//! 电池下限报警值.
		fu.val = 0.0f;
		CopyToLW(124, fu.bytes.buff, 4);

		//! 电池上限报警值.
		fu.val = 4.3f;
		CopyToLW(126, fu.bytes.buff, 4);

		//! O2下限报警值.
		fu.val = 0.0f;
		CopyToLW(128, fu.bytes.buff, 4);

		//! O2上限报警值.
		fu.val = 23.0f;
		CopyToLW(130, fu.bytes.buff, 4);
	}
}

void ProcessComx(int fd)
{
	JXM_DATA jxmdata;

	while (1)
	{
		Delay(10);
		gs_process_display_alarm();

		if (0 == gs_uart_recv_message(fd, &jxmdata))
		{
			if (1 == buttom[0])
			{
				SetLWBON(2, 0);
			}
			else
			{
				SetLWBOFF(2, 0);
			}
			
			if (1 == buttom[1])
			{
				SetLWBON(2, 1);
			}
			else
			{
				SetLWBOFF(2, 1);
			}
			
			if (1 == buttom[2])
			{
				SetLWBON(2, 2);
			}
			else
			{
				SetLWBOFF(2, 2);
			}
			
			if (1 == buttom[3])
			{
				SetLWBON(2, 3);
			}
			else
			{
				SetLWBOFF(2, 3);
			}
			
			if ((1 == buttom[0]) || (1 == buttom[1]) || (1 == buttom[2]) || (1 == buttom[3]))
			{
				SetLWBON(2, 4);
			}
			else
			{
				SetLWBOFF(2, 4);
			}
		}
		timesout[0]++;
		timesout[1]++;
		timesout[2]++;
		timesout[3]++;
		
		if (timesout[0] >= 120 * 50)
		{			
			if (1 == GetLWB(0, 0))
			{
				timesout[0] = 120 * 50;
				SetLWBON(3, 0);
			}
			else
			{
				timesout[0] = 0;
				SetLWBOFF(3, 0);
			}
		}
		else
		{
			SetLWBOFF(3, 0);
		}
		
		if (timesout[1] >= 120 * 50)
		{
			if (1 == GetLWB(0, 1))
			{
				timesout[1] = 120 * 50;
				SetLWBON(3, 1);
			}
			else
			{
				timesout[1] = 0;
				SetLWBOFF(3, 1);
			}
		}
		else
		{
			SetLWBOFF(3, 1);
		}
		
		if (timesout[2] >= 120 * 50)
		{
			if (1 == GetLWB(0, 2))
			{
				timesout[2] = 120 * 50;
				SetLWBON(3, 2);
			}
			else
			{
				timesout[2] = 0;
				SetLWBOFF(3, 2);
			}
		}
		else
		{
			SetLWBOFF(3, 2);
		}
		
		if (timesout[3] >= 120 * 50)
		{
			if (1 == GetLWB(0, 3))
			{
				timesout[3] = 120 * 50;
				SetLWBON(3, 3);
			}
			else
			{
				timesout[3] = 0;
				SetLWBOFF(3, 3);
			}
		}
		else
		{
			SetLWBOFF(3, 3);
		}
	}
}

