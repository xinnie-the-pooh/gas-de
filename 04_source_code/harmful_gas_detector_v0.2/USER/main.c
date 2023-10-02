#include "gs_api.h"
#include "gs_bsp.h"
#include "gs_harmful_gas_detector.h"
#include "gs_global_para.h"

/*
 * Function Name: main
 * Function Prototype: int main(void);
 * Function Description: 主程序.
 * 每做一次修改发布, 请注意修改软硬件版本号和编译时间等信息,
 * 以便于排查软件问题.
 * 外部晶振为16MHz, 需要将HSE_VALUE修改为16000000.
 *      AHB  = 24 MHz;
 *      APB2 = 24 MHz;
 *      APB1 = 12 MHz.
 *      修改<header_files.h>头文件下的AGV_TYPE宏,
 *      可编译针对不同叉车的信号采集板程序.
 * Entrance Parameters: None.
 * Export Parameters: None.
 * Return Value: None.
 * Version: V0.01
 * Modification Time: 2020.02.06 17:02
 * Author: WYP
 * Status: 已测试
 */
int main(void)
{
    //! 系统时钟初始化.
    gs_sysclock_init();
    //! 系统嘀嗒定时器初始化.
    gs_systick_init(1000);
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    //! LED状态指示灯初始化.
    gs_work_led_init(&ledpara, WORK_LED_FLASH_CYCLE);
    //! LED报警指示灯初始化.
    gs_soc_led_init();
	gs_debug_uart_check_init();
    //! 报警蜂鸣器初始化.
    gs_alarm_beep_init();
    //! E22无线LoRa模式控制端口初始化.
    gs_e32_m0m1_port_init();
    //! E22无线LoRa模式控制默认为正常模式.
    gs_e32_set_mode(E32_NORMAL_MODE);
    //! E22无线LoRa串口初始化.
    gs_uart_init(LORA_UART, 9600, NONE_PARITY);
    //! ADC初始化, 采集电池电压和MCU内部温度传感器.
    gs_adc_init();
    //! 全局变量初始化.
//    global_para_init();
    //! 无线LoRa发送周期为1s.
    gs_s_msleep_start_and_setting(&lora_msleep, 2000);
    //! ALARM检测周期为100ms.
    gs_s_msleep_start_and_setting(&alarm_msleep, 100);
    //! 默认报警为无报警.
    gs_alarm_ctrl(GS_NONE_ALARM);
	gs_soc_led_output_ctrl(SOC_100_GREEN);
	gs_uart_init(DEBUG_UART, 115200, NONE_PARITY);
    for (;;)
    {
        gs_work_led_flash(&ledpara);
		gs_debug_uart_init();
		gs_calc_adc_data();
		gs_calc_battery_soc();
        gs_alarm_led_flash(&alarm_msleep);
        gs_send_jxm_data_periodic();
		gs_jxm_alarm_ctrl();
    }
}

