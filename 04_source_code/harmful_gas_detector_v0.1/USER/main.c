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
    uint8_t i;
    //! ϵͳʱ�ӳ�ʼ��.
    gs_sysclock_init();
    //! ϵͳ��શ�ʱ����ʼ��.
    gs_systick_init(1000);
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    //! LED״ָ̬ʾ�Ƴ�ʼ��.
    gs_work_led_init(&ledpara, WORK_LED_FLASH_CYCLE);
    //! LED����ָʾ�Ƴ�ʼ��.
    gs_alarm_led_init(&alarmled, ALARM_LED_FLASH_CYCLE);
    //! ������������ʼ��.
    gs_alarm_beep_init();
    //! JXM����������ͨ���л����Ƴ�ʼ��.
    gs_jxm_switch_init();
    //! JXM����������ͨ���л�����Ĭ���л���O2ͨ��.
    gs_jxm_switch_ctrl(JXM_SWITCH_O2);
    //! JXM���������ڳ�ʼ��.
    gs_uart_init(JXM_UART, 9600, NONE_PARITY);
    //! E22����LoRaģʽ���ƶ˿ڳ�ʼ��.
    gs_e32_m0m1_port_init();
    //! E22����LoRaģʽ����Ĭ��Ϊ����ģʽ.
    gs_e32_set_mode(E32_NORMAL_MODE);
    //! E22����LoRa���ڳ�ʼ��.
    gs_uart_init(LORA_UART, 115200, NONE_PARITY);
    //! ADC��ʼ��, �ɼ���ص�ѹ��MCU�ڲ��¶ȴ�����.
    gs_adc_init();
    //! ȫ�ֱ�����ʼ��.
    global_para_init();
    //! ����LoRa��������Ϊ1s.
    gs_s_msleep_start_and_setting(&lora_msleep, 1000);
    //! ALARM�������Ϊ100ms.
    gs_s_msleep_start_and_setting(&alarm_msleep, 100);
    //! Ĭ�ϱ���Ϊ�ޱ���.
    gs_alarm_ctrl(GS_NONE_ALARM);
//    gs_jxm_send_inquiry1();
//    gs_msleep(1000);
    
    for (i = 0; i < 20; i++)
    {
//        gs_msleep(1000);
    }
//    gs_alarm_ctrl(GS_HIGH_LEVEL_ALARM);
//    gs_alarm_ctrl(GS_LOW_LEVEL_ALARM);

    for (;;)
    {
        //! LEDָʾ��1s��˸.
        gs_work_led_flash(&ledpara);
        gs_alarm_led_flash(&alarmled);
        gs_calc_adc_data();
        gs_jxm_get_sensor_data();
        gs_send_jxm_data_periodic();
//        gs_jxm_alarm_ctrl();
    }
}

