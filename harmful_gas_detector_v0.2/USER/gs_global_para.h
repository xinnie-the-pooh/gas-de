#ifndef __GS_GLOBAL_PARA_H__
#define __GS_GLOBAL_PARA_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f10x.h"
#include "gs_bsp.h"
#include "gs_typedef.h"
#include "gs_harmful_gas_detector.h"

extern LED_PARA ledpara;
extern UART_PARA debuguart;
extern UART_PARA lorauart;
extern ADC_PARA adcpara;
extern SYS_PARA syspara;
extern S_MSLEEP adc_msleep;
extern S_MSLEEP lora_msleep;
extern S_MSLEEP alarm_msleep;
extern S_MSLEEP buttom_msleep;
extern S_MSLEEP logcat_msleep;
extern GS_SENSOR_DATA sensordata[GS_SENSOR_CHN_MAX];
extern GS_CONFIG_DATA configdata;
extern void global_para_init(void);

#ifdef __cplusplus
}
#endif

#endif

