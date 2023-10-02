#ifndef __GS_TYPEDEF_H__
#define __GS_TYPEDEF_H__

#ifdef __cplusplus
extern "C" {
#endif

// 包含使用StdPeriph_Lib库的头文件.
#include "stm32f10x.h"
#include "stm32f10x_conf.h"

#pragma pack(1)
/**
  * @brief  软件版本号结构体信息.
  */
typedef struct
{
	uint8_t  device_type;	/*!< 设备类型. */
	uint8_t  hw_version;	/*!< 硬件版本号. */
	uint8_t  sw_version[2];	/*!< 软件版本号. */
	uint8_t  build_date[4];	/*!< 软件编译日期. */
	uint8_t  build_time[3];	/*!< 软件编译时间. */
	uint8_t  reverse1;		/*!< 预留1. */
	uint32_t device_id;		/*!< 设备ID编号. */
	uint16_t build_num;		/*!< build号, 即第几次build. */
	uint16_t reverse2;		/*!< 预留2. */
}VERSION_INFO;
#pragma pack()

#pragma pack(1)
/** 
  * @brief  软件非阻塞延时结构体定义.
  */
typedef struct
{
	uint8_t  enable_timer;	/*!< 延时时间启动标志. 1, 启动; 0, 暂停. */
	uint8_t  reverse;		/*!< reverse. */
	uint16_t maxcnt;		/*!< 设定软件延时值, 最大为65535ms. */
	uint32_t cnt;			/*!< 当前计数值. */
}S_MSLEEP;
#pragma pack()

typedef S_MSLEEP LED_PARA;		/*!< LED_PARA typedef. */
typedef S_MSLEEP BEEP_PARA;		/*!< BEEP_PARA typedef. */

#pragma pack(1)
/** 
  * @brief  软件非阻塞延时结构体定义.
  */
typedef struct
{
	uint8_t  enable_timer;	/*!< 延时时间启动标志. 1, 启动; 0, 暂停. */
	uint8_t  reverse;		/*!< reverse. */
	uint16_t maxcnt;		/*!< 设定软件延时值, 最大为65535ms. */
	uint32_t cnt;			/*!< 当前计数值. */
}DT_CONFIG;
#pragma pack()

#pragma pack(1)
/** 
  * @brief  系统参数结构体定义.
  */
typedef struct
{
	uint32_t heartbeat;			/*!< 系统心跳(单位为ms). */
	uint32_t running_time;		/*!< 设备当前的运行时间, 单位为秒. */
//	uint32_t device_id;		/*!< 设备当前的运行时间, 单位为秒. */

	float    mcu_temp;			/*!< MCU内部温度. */
	float    battery_volt;		/*!< 电池电压. */
	float    soc;				/*!< 电池SOC. */

	uint8_t  logcat;	/*!< 设备工作标志. */
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
  * @brief  DSFFOD配置文件结构体定义.
  */
typedef struct
{
	uint32_t starttime;			/*!< 开始时间(单位为s). */
	uint32_t endtime;			/*!< 结束时间(单位为s). */
	uint32_t intervaltime;		/*!< 间隔时间(单位为s). */
	uint32_t worktime;			/*!< 设备通电运行时间(单位为s). */
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

