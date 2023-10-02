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
void Write_LW(unsigned int n,unsigned short val);
void CopyToLW(unsigned int offset,const void *src, int n);
void CopyFromLW(unsigned int offset,const void *src, int n);
void SetLBON(unsigned int n);
void SetLBOFF(unsigned int n);
int GetLB(unsigned int n);
int SetLWBON(unsigned int n, unsigned int offset);
int SetLWBOFF(unsigned int n, unsigned int offset);
int GetLWB(unsigned int n, unsigned int offset);
int set_sp_speed(int fd,int nSpeed);

typedef enum
{
    GS_WARNING_CTRL_OFF = 0, 
    GS_WARNING_CTRL_ON  = 1, 
}GS_WARNING_CTRL;

typedef enum
{
    GS_WARNING_CHN1 = 0, 
    GS_WARNING_CHN2 = 1, 
    GS_WARNING_CHN3 = 2, 
    GS_WARNING_CHN4 = 3, 
    GS_WARNING_CHN5 = 4, 
    GS_WARNING_CHN6 = 5, 
    GS_WARNING_CHN7 = 6, 
    GS_WARNING_CHN8 = 7, 
    GS_WARNING_CHN_MAX = 8, 
}GS_WARNING_CHN;

typedef union
{
	struct
	{
		uint8_t buff[4];
	} bytes;
	float val;
}FLOAT_UNION;

uint16_t gs_calc_crc16_modbus(uint8_t *buff, int len)
{
	uint8_t i;
	uint16_t crc = 0xFFFF;

	while(len--)
	{
		crc ^= *(buff++);// crc ^= *buff; buff++;
		for (i = 0; i < 8; i++)
		{
			if (crc & 1)
			{
				crc = (crc >> 1) ^ 0xA001;
			}
			else
			{
				crc = (crc >> 1);
			}
		}
	}
	return crc;
}

int gs_warning_ctrl(int fd, GS_WARNING_CHN chn, GS_WARNING_CTRL on_off)
{
    unsigned char buff[16];
    int len = 0;
    uint16_t crc = 0;

    if (chn >= GS_WARNING_CHN_MAX)
    {
        return (-1);
    }

    memset(buff, '\0', sizeof(buff));
    buff[len++] = 0xFE;
    buff[len++] = 0x05;
    buff[len++] = 0x00;
    buff[len++] = chn;

    //! 开.
    if (GS_WARNING_CTRL_ON == on_off)
    {
        buff[len++] = 0xFF;
        buff[len++] = 0x00;
    }
    else if (GS_WARNING_CTRL_OFF == on_off)
    {
        buff[len++] = 0x00;
        buff[len++] = 0x00;
    }
    crc = gs_calc_crc16_modbus(buff, len);
	buff[len++] = ((crc >> 0) & 0xFF);
	buff[len++] = ((crc >> 8) & 0xFF);

    WriteData(fd, buff, len);
    return (0);
}

//! 单位为%, 量程为0.0%-100.0%.
#define GS_BATTERY_VOLT_LOW_LEVEL_VAL       (3.45f)
#define GS_BATTERY_VOLT_HIGH_LEVEL_VAL      (4.3f)
//! 单位为%, 量程为0.0%-100.0%.
#define GS_O2_LOW_LEVEL_VAL                 (19.5f)
#define GS_O2_HIGH_LEVEL_VAL                (23.0f)
//! 单位为PPM, 量程为0.0PPM-1000.0PPM.
#define GS_CO_LOW_LEVEL_VAL                 (24.0f)
#define GS_CO_HIGH_LEVEL_VAL                (160.0f)
//! 单位为PPM, 量程为0.0PPM-1000.0PPM.
#define GS_H2S_LOW_LEVEL_VAL                (6.0f)
#define GS_H2S_HIGH_LEVEL_VAL               (20.0f)
//! 单位为%, 量程为0.0PPM-100.0%.
#define GS_LEL_LOW_LEVEL_VAL                (10.0f)
#define GS_LEL_HIGH_LEVEL_VAL               (50.0f)

#define JXM_HISTORY_ADDR			(500)
#define JXM_CH1_BASE_ADDR			(300)
#define JXM_CH2_BASE_ADDR			(313)
#define JXM_CH3_BASE_ADDR			(326)
#define JXM_CH4_BASE_ADDR			(339)

GS_WARNING_CTRL gs_calc_warning_ctrl(void)
{
    FLOAT_UNION fu;
    uint8_t warning_status[4] = {0};
    uint8_t tmp[32];
	int i;
	static uint16_t cnt = 0;
	static uint8_t cur_chn = 0;

    if (1 == GetLWB(0, 0))
    {
        memset(fu.bytes.buff, '\0', 4);
        CopyFromLW((JXM_CH1_BASE_ADDR + 3), fu.bytes.buff, 4);
        if (fu.val < GS_BATTERY_VOLT_LOW_LEVEL_VAL)
        {
            warning_status[0] |= 0x01;
        }

        memset(fu.bytes.buff, '\0', 4);
        CopyFromLW((JXM_CH1_BASE_ADDR + 5), fu.bytes.buff, 4);
        if ((fu.val < GS_O2_LOW_LEVEL_VAL) || (fu.val > GS_O2_HIGH_LEVEL_VAL))
        {
            warning_status[0] |= 0x02;
        }

        memset(fu.bytes.buff, '\0', 4);
        CopyFromLW((JXM_CH1_BASE_ADDR + 7), fu.bytes.buff, 4);
        if (fu.val > GS_CO_LOW_LEVEL_VAL)
        {
            warning_status[0] |= 0x04;
        }

        memset(fu.bytes.buff, '\0', 4);
        CopyFromLW((JXM_CH1_BASE_ADDR + 9), fu.bytes.buff, 4);
        if (fu.val > GS_H2S_LOW_LEVEL_VAL)
        {
            warning_status[0] |= 0x08;
        }

        memset(fu.bytes.buff, '\0', 4);
        CopyFromLW((JXM_CH1_BASE_ADDR + 11), fu.bytes.buff, 4);
        if (fu.val > GS_LEL_LOW_LEVEL_VAL)
        {
            warning_status[0] |= 0x10;
        }
        Write_LW(250, warning_status[0]);
    }
    else
    {
        memset(tmp, '\0', sizeof(tmp));
        CopyToLW(JXM_CH1_BASE_ADDR, tmp, 13 * 2);
        Write_LW(250, 0);
    }

    if (1 == GetLWB(0, 1))
    {
        memset(fu.bytes.buff, '\0', 4);
        CopyFromLW((JXM_CH2_BASE_ADDR + 3), fu.bytes.buff, 4);
        if (fu.val < GS_BATTERY_VOLT_LOW_LEVEL_VAL)
        {
            warning_status[1] |= 0x01;
        }

        memset(fu.bytes.buff, '\0', 4);
        CopyFromLW((JXM_CH2_BASE_ADDR + 5), fu.bytes.buff, 4);
        if ((fu.val < GS_O2_LOW_LEVEL_VAL) || (fu.val > GS_O2_HIGH_LEVEL_VAL))
        {
            warning_status[1] |= 0x02;
        }

        memset(fu.bytes.buff, '\0', 4);
        CopyFromLW((JXM_CH2_BASE_ADDR + 7), fu.bytes.buff, 4);
        if (fu.val > GS_CO_LOW_LEVEL_VAL)
        {
            warning_status[1] |= 0x04;
        }

        memset(fu.bytes.buff, '\0', 4);
        CopyFromLW((JXM_CH2_BASE_ADDR + 9), fu.bytes.buff, 4);
        if (fu.val > GS_H2S_LOW_LEVEL_VAL)
        {
            warning_status[1] |= 0x08;
        }

        memset(fu.bytes.buff, '\0', 4);
        CopyFromLW((JXM_CH2_BASE_ADDR + 11), fu.bytes.buff, 4);
        if (fu.val > GS_LEL_LOW_LEVEL_VAL)
        {
            warning_status[1] |= 0x10;
        }
        Write_LW(251, warning_status[1]);
    }
    else
    {
        memset(tmp, '\0', sizeof(tmp));
        CopyToLW(JXM_CH2_BASE_ADDR, tmp, 13 * 2);
        Write_LW(251, 0);
    }

    if (1 == GetLWB(0, 2))
    {
        memset(fu.bytes.buff, '\0', 4);
        CopyFromLW((JXM_CH3_BASE_ADDR + 3), fu.bytes.buff, 4);
        if (fu.val < GS_BATTERY_VOLT_LOW_LEVEL_VAL)
        {
            warning_status[2] |= 0x01;
        }

        memset(fu.bytes.buff, '\0', 4);
        CopyFromLW((JXM_CH3_BASE_ADDR + 5), fu.bytes.buff, 4);
        if ((fu.val < GS_O2_LOW_LEVEL_VAL) || (fu.val > GS_O2_HIGH_LEVEL_VAL))
        {
            warning_status[2] |= 0x02;
        }

        memset(fu.bytes.buff, '\0', 4);
        CopyFromLW((JXM_CH3_BASE_ADDR + 7), fu.bytes.buff, 4);
        if (fu.val > GS_CO_LOW_LEVEL_VAL)
        {
            warning_status[2] |= 0x04;
        }

        memset(fu.bytes.buff, '\0', 4);
        CopyFromLW((JXM_CH3_BASE_ADDR + 9), fu.bytes.buff, 4);
        if (fu.val > GS_H2S_LOW_LEVEL_VAL)
        {
            warning_status[2] |= 0x08;
        }

        memset(fu.bytes.buff, '\0', 4);
        CopyFromLW((JXM_CH3_BASE_ADDR + 11), fu.bytes.buff, 4);
        if (fu.val > GS_LEL_LOW_LEVEL_VAL)
        {
            warning_status[2] |= 0x10;
        }
        Write_LW(252, warning_status[2]);
    }
    else
    {
        memset(tmp, '\0', sizeof(tmp));
        CopyToLW(JXM_CH3_BASE_ADDR, tmp, 13 * 2);
        Write_LW(252, 0);
    }

    if (1 == GetLWB(0, 3))
    {
        memset(fu.bytes.buff, '\0', 4);
        CopyFromLW((JXM_CH4_BASE_ADDR + 3), fu.bytes.buff, 4);
        if (fu.val < GS_BATTERY_VOLT_LOW_LEVEL_VAL)
        {
            warning_status[3] |= 0x01;
        }

        memset(fu.bytes.buff, '\0', 4);
        CopyFromLW((JXM_CH4_BASE_ADDR + 5), fu.bytes.buff, 4);
        if ((fu.val < GS_O2_LOW_LEVEL_VAL) || (fu.val > GS_O2_HIGH_LEVEL_VAL))
        {
            warning_status[3] |= 0x02;
        }

        memset(fu.bytes.buff, '\0', 4);
        CopyFromLW((JXM_CH4_BASE_ADDR + 7), fu.bytes.buff, 4);
        if (fu.val > GS_CO_LOW_LEVEL_VAL)
        {
            warning_status[3] |= 0x04;
        }

        memset(fu.bytes.buff, '\0', 4);
        CopyFromLW((JXM_CH4_BASE_ADDR + 9), fu.bytes.buff, 4);
        if (fu.val > GS_H2S_LOW_LEVEL_VAL)
        {
            warning_status[3] |= 0x08;
        }

        memset(fu.bytes.buff, '\0', 4);
        CopyFromLW((JXM_CH4_BASE_ADDR + 11), fu.bytes.buff, 4);
        if (fu.val > GS_LEL_LOW_LEVEL_VAL)
        {
            warning_status[3] |= 0x10;
        }
        Write_LW(253, warning_status[3]);
    }
    else
    {
        memset(tmp, '\0', sizeof(tmp));
        CopyToLW(JXM_CH4_BASE_ADDR, tmp, 13 * 2);
        Write_LW(253, 0);
    }

	#if 0
	if (0 != warning_status[0])
	{
		memset(tmp, '\0', sizeof(tmp));
		CopyFromLW((JXM_CH1_BASE_ADDR + 0), tmp, 13 * 2);
		CopyToLW(JXM_HISTORY_ADDR, tmp, 13 * 2);
		
		SetLBON(100);
	}
	
	if (0 != warning_status[1])
	{
		memset(tmp, '\0', sizeof(tmp));
		CopyFromLW((JXM_CH2_BASE_ADDR + 0), tmp, 13 * 2);
		CopyToLW(JXM_HISTORY_ADDR, tmp, 13 * 2);
		SetLBON(100);
	}
	
	if (0 != warning_status[2])
	{
		memset(tmp, '\0', sizeof(tmp));
		CopyFromLW((JXM_CH3_BASE_ADDR + 0), tmp, 13 * 2);
		CopyToLW(JXM_HISTORY_ADDR, tmp, 13 * 2);
		SetLBON(100);
	}
	
	if (0 != warning_status[3])
	{
		memset(tmp, '\0', sizeof(tmp));
		CopyFromLW((JXM_CH4_BASE_ADDR + 0), tmp, 13 * 2);
		CopyToLW(JXM_HISTORY_ADDR, tmp, 13 * 2);
		SetLBON(100);
	}
	#endif
	
	for (i = 0; i < 4; i++)
	{
		if (0 != warning_status[cur_chn])
		{
			if (0 != Read_LW(JXM_CH1_BASE_ADDR + 13 * cur_chn))
			{
				memset(tmp, '\0', sizeof(tmp));
				CopyFromLW((JXM_CH1_BASE_ADDR + 13 * cur_chn), tmp, 13 * 2);
				CopyToLW(JXM_HISTORY_ADDR, tmp, 13 * 2);
				SetLBON(100);
			}
			cur_chn++;
			cur_chn %= 4;
			break;
		}
		else
		{
			cur_chn++;
			cur_chn %= 4;
		}
	}
		
    if ((0 == warning_status[0]) 
        && (0 == warning_status[1]) 
        && (0 == warning_status[2]) 
        && (0 == warning_status[3]) 
		&& (0 == GetLWB(2, 4))
		&& (0 == GetLWB(3, 0))
		&& (0 == GetLWB(3, 1))
		&& (0 == GetLWB(3, 2))
		&& (0 == GetLWB(3, 3)))
    {
        SetLWBOFF(0, 5);
        return (GS_WARNING_CTRL_OFF);
    }
	
	if (0 == GetLWB(0, 4))
	{
		SetLWBON(0, 5);
		cnt = 0;
		return (GS_WARNING_CTRL_ON);
	}
	
	SetLWBOFF(0, 5);
	if ((++cnt) >= 30 * 1)
	{
		SetLWBOFF(0, 4);
		cnt = 0;
	}
    return (GS_WARNING_CTRL_OFF);
}

GS_WARNING_CTRL gs_calc_liandong_ctrl(void)
{
	if (1 == GetLWB(0, 6))
	{
		return (GS_WARNING_CTRL_ON);
	}
    return (GS_WARNING_CTRL_OFF);
}

void ProcessComx(int fd)
{
    while (1)
    {
        gs_warning_ctrl(fd, GS_WARNING_CHN1, gs_calc_warning_ctrl());
        Delay(500);
		gs_warning_ctrl(fd, GS_WARNING_CHN2, gs_calc_liandong_ctrl());
        Delay(500);
	}
}

