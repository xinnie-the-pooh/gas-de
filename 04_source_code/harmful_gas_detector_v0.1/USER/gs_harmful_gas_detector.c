#include "gs_harmful_gas_detector.h"
#include "gs_global_para.h"
#include <string.h>
#include <stdlib.h>
#include "gs_api.h"
#include <stdio.h>

const VERSION_INFO version __attribute__((at(APPLICATION_ADDRESS + VERSION_OFFSET))) = 
{
	0x01,			//! device_type.
	0x01,			//! hw_version.
	{0x00, 0x01},	//! sw_version.
	{0x20, 0x20, 0x04, 0x06},
	{0x23, 0x29, 0x45},
//	{BIN2BCD(BUILD_YEAR / 100), BIN2BCD(BUILD_YEAR % 100), BIN2BCD(BUILD_MONTH), BIN2BCD(BUILD_DAY),},//! build_date.
//	{BIN2BCD(BUILD_HOUR), BIN2BCD(BUILD_MINUTE), BIN2BCD(BUILD_SECOND),},//! build_time.
	0x00, 			//! reverse1.
	0x00000001,		//! device_id.
	0x0001,			//! build_num.
	0x0000,			//! reverse2.
};

void gs_jxm_set_work_mode(JXM_WORK_MODE mode)
{
    uint8_t buff[32];
    uint8_t len = 0;
    
    buff[len++] = 0xFF;
    buff[len++] = 0x01;
    buff[len++] = 0x03;
    buff[len++] = mode;
    buff[len++] = 0x00;
    buff[len++] = 0x00;
    buff[len++] = 0x00;
    buff[len++] = 0x00;
    buff[len++] = (uint8_t)gs_calc_cs(&buff[2], 6);
    gs_uart_send(JXM_UART, buff, len);
//    gs_printf("[CHN=%d] ", syspara.jxm_chn);
//    gs_print_hex_data("[JXM Send] ", buff, len);
}

void gs_jxm_send_inquiry(void)
{
    uint8_t buff[32];
    uint8_t len = 0;
    
    buff[len++] = 0xFF;
    buff[len++] = 0x01;
    buff[len++] = 0x07;
    buff[len++] = 0x00;
    buff[len++] = 0x00;
    buff[len++] = 0x00;
    buff[len++] = 0x00;
    buff[len++] = 0x00;
    buff[len++] = (uint8_t)gs_calc_cs(&buff[2], 6);
    gs_uart_send(JXM_UART, buff, len);
//    gs_printf("[CHN=%d] ", syspara.jxm_chn);
//    gs_print_hex_data("[JXM Send] ", buff, len);
}

void gs_jxm_send_inquiry1(void)
{
    uint8_t buff[32];
    uint8_t len = 0;
    
    buff[len++] = 0xFF;
    buff[len++] = 0x01;
    buff[len++] = 0x01;
    buff[len++] = 0x01;
    buff[len++] = 0x00;
    buff[len++] = 0x00;
    buff[len++] = 0x00;
    buff[len++] = 0x00;
    buff[len++] = (uint8_t)gs_calc_cs(&buff[2], 6);
    gs_uart_send(JXM_UART, buff, len);
//    gs_printf("[CHN=%d] ", syspara.jxm_chn);
//    gs_print_hex_data("[JXM Send] ", buff, len);
}

#define JXM_ADDR        (0x01)
int gs_jxm_paser_data(uint8_t *buff, uint16_t len, JXM_DATA *jxmdata)
{
    uint16_t i;
    uint16_t cs;
    
    for (i = 0; i < len; i++)
    {
        if ((buff[i]  == 0xFF) 
            || (buff[i + 1] == JXM_ADDR))
        {
            cs = gs_calc_cs(&buff[i + 2], 7);
            if ((cs & 0xFF) == buff[i + 9])
            {
                if (buff[i + 2] == 0x03)
                {
                    jxmdata->status = buff[i + 3];
                    return (0);
                }
                else if (buff[i + 2] == 0x07)
                {
                    jxmdata->resolution = buff[i + 3];
                    jxmdata->sensor = ((buff[i + 4] << 8) | buff[i + 5]);
                    jxmdata->val = (float)jxmdata->sensor * gs_pow(0.1f, jxmdata->resolution);
                    return (0);
                }
            }
        }
    }
    return (-1);
}

#define MAX_TIMESOUT    (100)
void gs_jxm_get_sensor_data(void)
{
    uint8_t buff[128];
    uint16_t len = 0;
    static uint8_t status = 0;
    static uint8_t num = 0;
    static S_MSLEEP s;

    switch(status)
    {
    case 0:
        gs_jxm_set_work_mode(JXM_WORK_INQUIRY_MODE);
        gs_s_msleep_start_and_setting(&s, MAX_TIMESOUT);
        status = 1;
        break;
    case 1:
        if (gs_s_msleep_is_timesup(&s))
        {
            gs_s_msleep_stop(&s);
            syspara.jxm_chn++;
            syspara.jxm_chn %= JXM_SWITCH_MAX;
            gs_jxm_switch_ctrl((JXM_SWITCH)syspara.jxm_chn);
            status = 0;
        }
        else
        {
            if (0 == gs_uart_recv(JXM_UART, buff, &len))
            {
//                gs_printf("[CHN=%d] ", syspara.jxm_chn);
//                gs_print_hex_data("[JXM Recv] ", buff, len);
                if (0 == gs_jxm_paser_data(buff, len, &jxmdata[syspara.jxm_chn]))
                {
                    if ((jxmdata[0].status == 1) 
                        && (jxmdata[1].status == 1) 
                        && (jxmdata[2].status == 1) 
                        && (jxmdata[3].status == 1))
                    {
                        status = 2;
                    }
                    else if ((++num) >= 20)
                    {
                        status = 2;
                    }
                }
                else
                {
                    status = 0;
                }
            }
        }
        break;
    case 2:
//        if (syspara.jxm_chn == 0)
        {
//            gs_jxm_send_inquiry1();
        }
//        else
        {
            gs_jxm_send_inquiry();
        }
        
        gs_s_msleep_start_and_setting(&s, MAX_TIMESOUT);
        status = 3;
        break;
    case 3:
        if (gs_s_msleep_is_timesup(&s))
        {
            gs_s_msleep_stop(&s);
            syspara.jxm_chn++;
            syspara.jxm_chn %= JXM_SWITCH_MAX;
            gs_jxm_switch_ctrl((JXM_SWITCH)syspara.jxm_chn);
            status = 2;
        }
        else
        {
            memset(buff,'\0', sizeof(buff));
            if (0 == gs_uart_recv(JXM_UART, buff, &len))
            {
//                gs_printf("[CHN=%d] ", syspara.jxm_chn);
//                gs_print_hex_data("[JXM Recv] ", buff, len);
                if (0 == gs_jxm_paser_data(buff, len, &jxmdata[syspara.jxm_chn]))
                {
                    syspara.jxm_status |= (1 << syspara.jxm_chn);
//                    gs_printf("[CHN=%d] volt=%f, mcu_temp=%f, o2=%f, co=%f, h2s=%f, lel=%f.\r\n", syspara.jxm_chn, syspara.battery_volt, syspara.mcu_temp, jxmdata[0].val, jxmdata[1].val, jxmdata[2].val, jxmdata[3].val);
                }
                else
                {
                    status = 2;
                }
            }
        }
        break;
    default:
        break;
    }
}

void gs_send_jxm_data_by_lora(void)
{
    uint8_t buff[64];
    uint16_t len = 0;
    uint8_t dlen = 0;
    FLOAT_UNION fu;
    
    buff[len++] = 0x68;
    buff[len++] = 0x01;
    buff[len++] = 0x80;
    buff[len++] = 0x00;
    
    buff[len++] = syspara.jxm_status;
    dlen += 1;
    
    buff[len++] = ((syspara.framenum >> 0) & 0xFF);
    buff[len++] = ((syspara.framenum >> 8) & 0xFF);
    buff[len++] = ((syspara.framenum >> 16) & 0xFF);
    dlen += 3;
    syspara.framenum++;
    
    fu.val = syspara.battery_volt;
    buff[len++] = fu.bytes.buff[0];
    buff[len++] = fu.bytes.buff[1];
    buff[len++] = fu.bytes.buff[2];
    buff[len++] = fu.bytes.buff[3];
    dlen += 4;
    
    fu.val = jxmdata[JXM_SWITCH_O2].val;
    buff[len++] = fu.bytes.buff[0];
    buff[len++] = fu.bytes.buff[1];
    buff[len++] = fu.bytes.buff[2];
    buff[len++] = fu.bytes.buff[3];
    dlen += 4;
    
    fu.val = jxmdata[JXM_SWITCH_CO].val;
    buff[len++] = fu.bytes.buff[0];
    buff[len++] = fu.bytes.buff[1];
    buff[len++] = fu.bytes.buff[2];
    buff[len++] = fu.bytes.buff[3];
    dlen += 4;
    
    fu.val = jxmdata[JXM_SWITCH_H2S].val;
    buff[len++] = fu.bytes.buff[0];
    buff[len++] = fu.bytes.buff[1];
    buff[len++] = fu.bytes.buff[2];
    buff[len++] = fu.bytes.buff[3];
    dlen += 4;
    
    fu.val = jxmdata[JXM_SWITCH_LEL].val;
    buff[len++] = fu.bytes.buff[0];
    buff[len++] = fu.bytes.buff[1];
    buff[len++] = fu.bytes.buff[2];
    buff[len++] = fu.bytes.buff[3];
    dlen += 4;
    
    buff[3] = dlen;
    
    buff[len++] = gs_calc_cs(&buff[4], dlen);
    buff[len++] = 0x16;
    
    gs_uart_send(LORA_UART, buff, len);
}

void gs_send_jxm_data_periodic(void)
{
    if (syspara.heartbeat < 10 * 1000)
    {
        return;
    }

    if (gs_s_msleep_is_timesup(&lora_msleep))
    {
        gs_send_jxm_data_by_lora();
        gs_s_msleep_stop(&lora_msleep);
        gs_s_msleep_start_and_setting(&lora_msleep, lora_msleep.maxcnt);
    }
}

void gs_alarm_ctrl(GS_ALARM_LEVEL level)
{
    if (GS_NONE_ALARM == level)
    {
        syspara.alarm_ctrl = 0;
        syspara.alarm_ctrl_cycle = 200;
    }
    else if (GS_LOW_LEVEL_ALARM == level)
    {
        syspara.alarm_ctrl = 1;
        syspara.alarm_ctrl_cycle = 250;
    }
    else if (GS_HIGH_LEVEL_ALARM == level)
    {
        syspara.alarm_ctrl = 1;
        syspara.alarm_ctrl_cycle = 100;
    }
}

//! 单位为%, 量程为0.0%-100.0%.
#define GS_O2_LOW_LEVEL_VAL         (19.5f)
#define GS_O2_HIGH_LEVEL_VAL        (23.0f)
//! 单位为PPM, 量程为0.0PPM-1000.0PPM.
#define GS_CO_LOW_LEVEL_VAL         (24.0f)
#define GS_CO_HIGH_LEVEL_VAL        (160.0f)
//! 单位为PPM, 量程为0.0PPM-1000.0PPM.
#define GS_H2S_LOW_LEVEL_VAL        (6.0f)
#define GS_H2S_HIGH_LEVEL_VAL       (20.0f)
//! 单位为%, 量程为0.0PPM-100.0%.
#define GS_LEL_LOW_LEVEL_VAL        (10.0f)
#define GS_LEL_HIGH_LEVEL_VAL       (50.0f)

void gs_jxm_alarm_ctrl(void)
{
    if (syspara.heartbeat < 10 * 1000)
    {
        return;
    }

    if (0 == gs_s_msleep_is_timesup(&alarm_msleep))
    {
        return;
    }
    gs_s_msleep_start_and_setting(&alarm_msleep, alarm_msleep.maxcnt);

    //! O2报警处理.
    if (1 == jxmdata[JXM_SWITCH_O2].status)
    {
        if (jxmdata[JXM_SWITCH_O2].val <= GS_O2_LOW_LEVEL_VAL)
        {
            //! O2异常值, 低限报警.
            jxmdata[JXM_SWITCH_O2].alarm = GS_LOW_LEVEL_ALARM;
//            gs_alarm_ctrl(GS_LOW_LEVEL_ALARM);
        }
        else if(jxmdata[JXM_SWITCH_O2].val >= GS_O2_HIGH_LEVEL_VAL)
        {
            //! O2异常值, 高限报警.
            jxmdata[JXM_SWITCH_O2].alarm = GS_HIGH_LEVEL_ALARM;
//            gs_alarm_ctrl(GS_HIGH_LEVEL_ALARM);
        }
        else
        {
            //! O2正常值, 无报警.
            jxmdata[JXM_SWITCH_O2].alarm = GS_NONE_ALARM;
//            gs_alarm_ctrl(GS_NONE_ALARM);
        }
    }
    else
    {
        //! O2传感器异常, 低限报警.
        jxmdata[JXM_SWITCH_O2].alarm = GS_LOW_LEVEL_ALARM;
//        gs_alarm_ctrl(GS_LOW_LEVEL_ALARM);
    }

    //! CO报警处理.
    if (1 == jxmdata[JXM_SWITCH_CO].status)
    {
        if (jxmdata[JXM_SWITCH_CO].val >= GS_CO_HIGH_LEVEL_VAL)
        {
            //! CO异常值, 高限报警.
            jxmdata[JXM_SWITCH_CO].alarm = GS_HIGH_LEVEL_ALARM;
        }
        else if(jxmdata[JXM_SWITCH_CO].val >= GS_O2_LOW_LEVEL_VAL)
        {
            //! CO异常值, 低限报警.
            jxmdata[JXM_SWITCH_CO].alarm = GS_LOW_LEVEL_ALARM;
        }
        else
        {
            //! CO正常值, 无报警.
            jxmdata[JXM_SWITCH_CO].alarm = GS_NONE_ALARM;
        }
    }
    else
    {
        //! CO传感器异常, 低限报警.
        jxmdata[JXM_SWITCH_CO].alarm = GS_LOW_LEVEL_ALARM;
    }

    //! H2S报警处理.
    if (1 == jxmdata[JXM_SWITCH_H2S].status)
    {
        if (jxmdata[JXM_SWITCH_H2S].val >= GS_H2S_HIGH_LEVEL_VAL)
        {
            //! H2S异常值, 高限报警.
            jxmdata[JXM_SWITCH_H2S].alarm = GS_HIGH_LEVEL_ALARM;
        }
        else if(jxmdata[JXM_SWITCH_H2S].val >= GS_H2S_LOW_LEVEL_VAL)
        {
            //! H2S异常值, 低限报警.
            jxmdata[JXM_SWITCH_H2S].alarm = GS_LOW_LEVEL_ALARM;
        }
        else
        {
            //! H2S正常值, 无报警.
            jxmdata[JXM_SWITCH_H2S].alarm = GS_NONE_ALARM;
        }
    }
    else
    {
        //! H2S传感器异常, 低限报警.
        jxmdata[JXM_SWITCH_H2S].alarm = GS_LOW_LEVEL_ALARM;
    }

    //! LEL报警处理.
    if (1 == jxmdata[JXM_SWITCH_LEL].status)
    {
        if (jxmdata[JXM_SWITCH_LEL].val >= GS_LEL_HIGH_LEVEL_VAL)
        {
            //! LEL异常值, 高限报警.
            jxmdata[JXM_SWITCH_LEL].alarm = GS_HIGH_LEVEL_ALARM;
        }
        else if(jxmdata[JXM_SWITCH_LEL].val >= GS_LEL_LOW_LEVEL_VAL)
        {
            //! LEL异常值, 低限报警.
            jxmdata[JXM_SWITCH_LEL].alarm = GS_LOW_LEVEL_ALARM;
        }
        else
        {
            //! LEL正常值, 无报警.
            jxmdata[JXM_SWITCH_LEL].alarm = GS_NONE_ALARM;
        }
    }
    else
    {
        //! LEL传感器异常, 低限报警.
        jxmdata[JXM_SWITCH_LEL].alarm = GS_LOW_LEVEL_ALARM;
    }

    if ((jxmdata[JXM_SWITCH_O2].alarm == GS_HIGH_LEVEL_ALARM) 
        || (jxmdata[JXM_SWITCH_CO].alarm == GS_HIGH_LEVEL_ALARM) 
        || (jxmdata[JXM_SWITCH_H2S].alarm == GS_HIGH_LEVEL_ALARM) 
        || (jxmdata[JXM_SWITCH_LEL].alarm == GS_HIGH_LEVEL_ALARM))
    {
        gs_alarm_ctrl(GS_HIGH_LEVEL_ALARM);
    }
    else if ((jxmdata[JXM_SWITCH_O2].alarm == GS_LOW_LEVEL_ALARM) 
        || (jxmdata[JXM_SWITCH_CO].alarm == GS_LOW_LEVEL_ALARM) 
        || (jxmdata[JXM_SWITCH_H2S].alarm == GS_LOW_LEVEL_ALARM) 
        || (jxmdata[JXM_SWITCH_LEL].alarm == GS_LOW_LEVEL_ALARM))
    {
        gs_alarm_ctrl(GS_LOW_LEVEL_ALARM);
    }
    else
    {
        gs_alarm_ctrl(GS_NONE_ALARM);
    }
}

