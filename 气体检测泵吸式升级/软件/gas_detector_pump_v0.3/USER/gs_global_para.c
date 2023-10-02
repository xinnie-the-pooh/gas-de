#include "gs_global_para.h"

LED_PARA ledpara;
UART_PARA debuguart;
UART_PARA lorauart;
ADC_PARA adcpara;
SYS_PARA syspara;
S_MSLEEP adc_msleep;
S_MSLEEP lora_msleep;
S_MSLEEP alarm_msleep;
S_MSLEEP buttom_msleep;
S_MSLEEP logcat_msleep;
GS_SENSOR_DATA sensordata[GS_SENSOR_CHN_MAX];
GS_CONFIG_DATA configdata;

void global_para_init(void)
{
}
