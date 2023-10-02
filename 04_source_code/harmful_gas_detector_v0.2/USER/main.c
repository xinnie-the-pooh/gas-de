#include "gs_api.h"
#include "gs_bsp.h"
#include "gs_harmful_gas_detector.h"
#include "gs_global_para.h"

/*
 * Function Name: main
 * Function Prototype: int main(void);
 * Function Description: ������.
 * ÿ��һ���޸ķ���, ��ע���޸���Ӳ���汾�źͱ���ʱ�����Ϣ,
 * �Ա����Ų��������.
 * �ⲿ����Ϊ16MHz, ��Ҫ��HSE_VALUE�޸�Ϊ16000000.
 *      AHB  = 24 MHz;
 *      APB2 = 24 MHz;
 *      APB1 = 12 MHz.
 *      �޸�<header_files.h>ͷ�ļ��µ�AGV_TYPE��,
 *      �ɱ�����Բ�ͬ�泵���źŲɼ������.
 * Entrance Parameters: None.
 * Export Parameters: None.
 * Return Value: None.
 * Version: V0.01
 * Modification Time: 2020.02.06 17:02
 * Author: WYP
 * Status: �Ѳ���
 */
int main(void)
{
    //! ϵͳʱ�ӳ�ʼ��.
    gs_sysclock_init();
    //! ϵͳ��શ�ʱ����ʼ��.
    gs_systick_init(1000);
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    //! LED״ָ̬ʾ�Ƴ�ʼ��.
    gs_work_led_init(&ledpara, WORK_LED_FLASH_CYCLE);
    //! LED����ָʾ�Ƴ�ʼ��.
    gs_soc_led_init();
	gs_debug_uart_check_init();
    //! ������������ʼ��.
    gs_alarm_beep_init();
    //! E22����LoRaģʽ���ƶ˿ڳ�ʼ��.
    gs_e32_m0m1_port_init();
    //! E22����LoRaģʽ����Ĭ��Ϊ����ģʽ.
    gs_e32_set_mode(E32_NORMAL_MODE);
    //! E22����LoRa���ڳ�ʼ��.
    gs_uart_init(LORA_UART, 9600, NONE_PARITY);
    //! ADC��ʼ��, �ɼ���ص�ѹ��MCU�ڲ��¶ȴ�����.
    gs_adc_init();
    //! ȫ�ֱ�����ʼ��.
//    global_para_init();
    //! ����LoRa��������Ϊ1s.
    gs_s_msleep_start_and_setting(&lora_msleep, 2000);
    //! ALARM�������Ϊ100ms.
    gs_s_msleep_start_and_setting(&alarm_msleep, 100);
    //! Ĭ�ϱ���Ϊ�ޱ���.
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

