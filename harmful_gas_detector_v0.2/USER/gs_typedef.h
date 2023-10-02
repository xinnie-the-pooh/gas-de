#ifndef __GS_TYPEDEF_H__
#define __GS_TYPEDEF_H__

#ifdef __cplusplus
extern "C" {
#endif

// ����ʹ��StdPeriph_Lib���ͷ�ļ�.
#include "stm32f10x.h"
#include "stm32f10x_conf.h"

#pragma pack(1)
/**
  * @brief  ����汾�Žṹ����Ϣ.
  */
typedef struct
{
	uint8_t  device_type;	/*!< �豸����. */
	uint8_t  hw_version;	/*!< Ӳ���汾��. */
	uint8_t  sw_version[2];	/*!< ����汾��. */
	uint8_t  build_date[4];	/*!< �����������. */
	uint8_t  build_time[3];	/*!< �������ʱ��. */
	uint8_t  reverse1;		/*!< Ԥ��1. */
	uint32_t device_id;		/*!< �豸ID���. */
	uint16_t build_num;		/*!< build��, ���ڼ���build. */
	uint16_t reverse2;		/*!< Ԥ��2. */
}VERSION_INFO;
#pragma pack()

#pragma pack(1)
/** 
  * @brief  �����������ʱ�ṹ�嶨��.
  */
typedef struct
{
	uint8_t  enable_timer;	/*!< ��ʱʱ��������־. 1, ����; 0, ��ͣ. */
	uint8_t  reverse;		/*!< reverse. */
	uint16_t maxcnt;		/*!< �趨�����ʱֵ, ���Ϊ65535ms. */
	uint32_t cnt;			/*!< ��ǰ����ֵ. */
}S_MSLEEP;
#pragma pack()

typedef S_MSLEEP LED_PARA;		/*!< LED_PARA typedef. */
typedef S_MSLEEP BEEP_PARA;		/*!< BEEP_PARA typedef. */

#pragma pack(1)
/** 
  * @brief  �����������ʱ�ṹ�嶨��.
  */
typedef struct
{
	uint8_t  enable_timer;	/*!< ��ʱʱ��������־. 1, ����; 0, ��ͣ. */
	uint8_t  reverse;		/*!< reverse. */
	uint16_t maxcnt;		/*!< �趨�����ʱֵ, ���Ϊ65535ms. */
	uint32_t cnt;			/*!< ��ǰ����ֵ. */
}DT_CONFIG;
#pragma pack()

#pragma pack(1)
/** 
  * @brief  ϵͳ�����ṹ�嶨��.
  */
typedef struct
{
	uint32_t heartbeat;			/*!< ϵͳ����(��λΪms). */
	uint32_t running_time;		/*!< �豸��ǰ������ʱ��, ��λΪ��. */
//	uint32_t device_id;		/*!< �豸��ǰ������ʱ��, ��λΪ��. */

	float    mcu_temp;			/*!< MCU�ڲ��¶�. */
	float    battery_volt;		/*!< ��ص�ѹ. */
	float    soc;				/*!< ���SOC. */

	uint8_t  logcat;	/*!< �豸������־. */
    uint8_t  buttom;
    uint8_t  reverse;
    uint8_t  alarm_ctrl;
    uint32_t framenum;
    uint16_t alarm_ctrl_cycle;
	uint8_t  flag_filter_out_adc;
	uint32_t  filter_out_adc_cnt;
}SYS_PARA;
#pragma pack()

#pragma pack(1)
/** 
  * @brief  DSFFOD�����ļ��ṹ�嶨��.
  */
typedef struct
{
	uint32_t starttime;			/*!< ��ʼʱ��(��λΪs). */
	uint32_t endtime;			/*!< ����ʱ��(��λΪs). */
	uint32_t intervaltime;		/*!< ���ʱ��(��λΪs). */
	uint32_t worktime;			/*!< �豸ͨ������ʱ��(��λΪs). */
}DSFFOD_CONFIG;
#pragma pack()

typedef union
{
	struct
	{
		uint8_t buff[4];
	} bytes;
	float val;
}FLOAT_UNION;

#ifdef __cplusplus
}
#endif

#endif

