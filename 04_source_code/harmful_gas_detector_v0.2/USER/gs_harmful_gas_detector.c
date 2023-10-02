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

	#if 1
    //! O2报警处理.
	if (jxmdata[JXM_SWITCH_O2].val <= GS_O2_LOW_LEVEL_VAL)
	{
		//! O2异常值, 低限报警.
		jxmdata[JXM_SWITCH_O2].alarm = GS_LOW_LEVEL_ALARM;
	}
	else if(jxmdata[JXM_SWITCH_O2].val >= GS_O2_HIGH_LEVEL_VAL)
	{
		//! O2异常值, 高限报警.
		jxmdata[JXM_SWITCH_O2].alarm = GS_HIGH_LEVEL_ALARM;
	}
	else
	{
		//! O2正常值, 无报警.
		jxmdata[JXM_SWITCH_O2].alarm = GS_NONE_ALARM;
	}

    //! CO报警处理.
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

    //! H2S报警处理.
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

    //! LEL报警处理.
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
	#endif
}

void gs_calc_battery_soc(void)
{
	static S_MSLEEP s;
	static uint8_t first = 0;
//	syspara.battery_volt = 3.5f;
	
	if (syspara.heartbeat < 1000L)
	{
		return;
	}
	
	if (0 == first)
	{
		gs_s_msleep_start_and_setting(&s, 1000);
		first = 1;
	}
	if (gs_s_msleep_is_timesup(&s))
	{
		if (syspara.battery_volt >= 4.2f)
		{
			syspara.soc = 100.0f;
			gs_soc_led_output_ctrl(SOC_100_GREEN);
		}
		else if (syspara.battery_volt >= 3.9f)
		{
			syspara.soc = 75.0f;
			gs_soc_led_output_ctrl(SOC_075_GREEN);
		}
		else if (syspara.battery_volt >= 3.6f)
		{
			syspara.soc = 50.0f;
			gs_soc_led_output_ctrl(SOC_050_GREEN_FLASH);
		}
		else if (syspara.battery_volt >= 3.3f)
		{
			syspara.soc = 25.0f;
			gs_soc_led_output_ctrl(SOC_025_RED);
		}
		else if (syspara.battery_volt >= 3.0f)
		{
			syspara.soc = 0.0f;
			gs_soc_led_output_ctrl(SOC_000_RED_FLASH_BEEP);
		}
		gs_s_msleep_stop(&s);
		gs_s_msleep_start_and_setting(&s, s.maxcnt);
	}
}

void gs_debug_uart_init(void)
{
	static uint8_t first  = 0;
	static uint8_t debug_uart = 0;
	debug_uart = gs_debug_uart_check();
		
	if (0 == debug_uart)
	{
		if (0 == first)
		{
			gs_uart_init(DEBUG_UART, 115200, NONE_PARITY);
			first = 1;
		}
	}
}
