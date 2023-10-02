#include "gs_global_para.h"

LED_PARA ledpara;
LED_PARA alarmled;
UART_PARA jxmuart;
UART_PARA lorauart;
ADC_PARA adcpara;
JXM_DATA jxmdata[JXM_SWITCH_MAX];
SYS_PARA syspara;
S_MSLEEP lora_msleep;
S_MSLEEP alarm_msleep;

void global_para_init(void)
{
}
