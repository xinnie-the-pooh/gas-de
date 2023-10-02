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

//	buff[len++] = 0x00;
//	buff[len++] = 0x00;
//	buff[len++] = 0x17;
	
    buff[len++] = 0x68;
    buff[len++] = (configdata.id & 0xFF);
    buff[len++] = 0x80;
    buff[len++] = 0x00;

	buff[len++] = syspara.buttom;
    dlen += 1;
    
	syspara.framenum = syspara.heartbeat / 1000;
    buff[len++] = ((syspara.framenum >> 0) & 0xFF);
    buff[len++] = ((syspara.framenum >> 8) & 0xFF);
    buff[len++] = ((syspara.framenum >> 16) & 0xFF);
    dlen += 3;
//    syspara.framenum++;
    
    fu.val = syspara.battery_volt;
    buff[len++] = fu.bytes.buff[0];
    buff[len++] = fu.bytes.buff[1];
    buff[len++] = fu.bytes.buff[2];
    buff[len++] = fu.bytes.buff[3];
    dlen += 4;
    
    fu.val = sensordata[GS_SENSOR_CHN_O2].val;
    buff[len++] = fu.bytes.buff[0];
    buff[len++] = fu.bytes.buff[1];
    buff[len++] = fu.bytes.buff[2];
    buff[len++] = fu.bytes.buff[3];
    dlen += 4;
    
    fu.val = sensordata[GS_SENSOR_CHN_CO].val;
    buff[len++] = fu.bytes.buff[0];
    buff[len++] = fu.bytes.buff[1];
    buff[len++] = fu.bytes.buff[2];
    buff[len++] = fu.bytes.buff[3];
    dlen += 4;
    
    fu.val = sensordata[GS_SENSOR_CHN_H2S].val;
    buff[len++] = fu.bytes.buff[0];
    buff[len++] = fu.bytes.buff[1];
    buff[len++] = fu.bytes.buff[2];
    buff[len++] = fu.bytes.buff[3];
    dlen += 4;
    
    fu.val = sensordata[GS_SENSOR_CHN_LEL].val;
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
//		syspara.flag_filter_out_adc = 1;
//		syspara.filter_out_adc_cnt = syspara.heartbeat;
        gs_send_jxm_data_by_lora();
//		gs_adc_dma_restart();
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
//! 单位为PPM, 量程为0.0PPM-100.0PPM.
#define GS_H2S_LOW_LEVEL_VAL        (6.0f)
#define GS_H2S_HIGH_LEVEL_VAL       (20.0f)
//! 单位为%, 量程为0.0PPM-100.0%.
#define GS_LEL_LOW_LEVEL_VAL        (10.0f)
#define GS_LEL_HIGH_LEVEL_VAL       (50.0f)

#define GS_MAX_ALARM_CNT			(3)
#define GS_MIN_ALARM_CNT			(0)

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
	if (sensordata[GS_SENSOR_CHN_O2].val <= GS_O2_LOW_LEVEL_VAL)
	{
		if ((++sensordata[GS_SENSOR_CHN_O2].low_cnt) >= GS_MAX_ALARM_CNT)
		{
			sensordata[GS_SENSOR_CHN_O2].low_cnt = GS_MAX_ALARM_CNT;
			//! O2异常值, 低限报警.
			sensordata[GS_SENSOR_CHN_O2].alarm = GS_LOW_LEVEL_ALARM;
		}
		sensordata[GS_SENSOR_CHN_O2].high_cnt = 0;
	}
	else if(sensordata[GS_SENSOR_CHN_O2].val >= GS_O2_HIGH_LEVEL_VAL)
	{
		if ((++sensordata[GS_SENSOR_CHN_O2].high_cnt) >= GS_MAX_ALARM_CNT)
		{
			sensordata[GS_SENSOR_CHN_O2].high_cnt = GS_MAX_ALARM_CNT;
			//! O2异常值, 高限报警.
			sensordata[GS_SENSOR_CHN_O2].alarm = GS_HIGH_LEVEL_ALARM;
		}
		sensordata[GS_SENSOR_CHN_O2].low_cnt = 0;
	}
	else
	{
		sensordata[GS_SENSOR_CHN_O2].low_cnt = 0;
		sensordata[GS_SENSOR_CHN_O2].high_cnt = 0;
		//! O2正常值, 无报警.
		sensordata[GS_SENSOR_CHN_O2].alarm = GS_NONE_ALARM;
	}

    //! CO报警处理.
	if (sensordata[GS_SENSOR_CHN_CO].val >= GS_CO_HIGH_LEVEL_VAL)
	{
		if ((++sensordata[GS_SENSOR_CHN_CO].low_cnt) >= GS_MAX_ALARM_CNT)
		{
			sensordata[GS_SENSOR_CHN_CO].low_cnt = GS_MAX_ALARM_CNT;
			//! CO异常值, 高限报警.
			sensordata[GS_SENSOR_CHN_CO].alarm = GS_HIGH_LEVEL_ALARM;
		}
		sensordata[GS_SENSOR_CHN_CO].high_cnt = 0;
	}
	else if(sensordata[GS_SENSOR_CHN_CO].val >= GS_O2_LOW_LEVEL_VAL)
	{
		if ((++sensordata[GS_SENSOR_CHN_CO].high_cnt) >= GS_MAX_ALARM_CNT)
		{
			sensordata[GS_SENSOR_CHN_CO].high_cnt = GS_MAX_ALARM_CNT;
			//! CO异常值, 低限报警.
			sensordata[GS_SENSOR_CHN_CO].alarm = GS_LOW_LEVEL_ALARM;
		}
		sensordata[GS_SENSOR_CHN_CO].low_cnt = 0;
	}
	else
	{
		sensordata[GS_SENSOR_CHN_CO].low_cnt = 0;
		sensordata[GS_SENSOR_CHN_CO].high_cnt = 0;
		//! CO正常值, 无报警.
		sensordata[GS_SENSOR_CHN_CO].alarm = GS_NONE_ALARM;
	}

    //! H2S报警处理.
	if (sensordata[GS_SENSOR_CHN_H2S].val >= GS_H2S_HIGH_LEVEL_VAL)
	{
		if ((++sensordata[GS_SENSOR_CHN_H2S].low_cnt) >= GS_MAX_ALARM_CNT)
		{
			sensordata[GS_SENSOR_CHN_H2S].low_cnt = GS_MAX_ALARM_CNT;
			//! H2S异常值, 高限报警.
			sensordata[GS_SENSOR_CHN_H2S].alarm = GS_HIGH_LEVEL_ALARM;
		}
		sensordata[GS_SENSOR_CHN_H2S].high_cnt = 0;
	}
	else if(sensordata[GS_SENSOR_CHN_H2S].val >= GS_H2S_LOW_LEVEL_VAL)
	{
		if ((++sensordata[GS_SENSOR_CHN_H2S].high_cnt) >= GS_MAX_ALARM_CNT)
		{
			sensordata[GS_SENSOR_CHN_H2S].high_cnt = GS_MAX_ALARM_CNT;
			//! H2S异常值, 低限报警.
			sensordata[GS_SENSOR_CHN_H2S].alarm = GS_LOW_LEVEL_ALARM;
		}
		sensordata[GS_SENSOR_CHN_H2S].low_cnt = 0;
	}
	else
	{
		sensordata[GS_SENSOR_CHN_H2S].low_cnt = 0;
		sensordata[GS_SENSOR_CHN_H2S].high_cnt = 0;
		//! H2S正常值, 无报警.
		sensordata[GS_SENSOR_CHN_H2S].alarm = GS_NONE_ALARM;
	}

    //! LEL报警处理.
	if (sensordata[GS_SENSOR_CHN_LEL].val >= GS_LEL_HIGH_LEVEL_VAL)
	{
		if ((++sensordata[GS_SENSOR_CHN_LEL].low_cnt) >= GS_MAX_ALARM_CNT)
		{
			sensordata[GS_SENSOR_CHN_LEL].low_cnt = GS_MAX_ALARM_CNT;
			//! LEL异常值, 高限报警.
			sensordata[GS_SENSOR_CHN_LEL].alarm = GS_HIGH_LEVEL_ALARM;
		}
		sensordata[GS_SENSOR_CHN_LEL].high_cnt = 0;
	}
	else if(sensordata[GS_SENSOR_CHN_LEL].val >= GS_LEL_LOW_LEVEL_VAL)
	{
		if ((++sensordata[GS_SENSOR_CHN_LEL].high_cnt) >= GS_MAX_ALARM_CNT)
		{
			sensordata[GS_SENSOR_CHN_LEL].high_cnt = GS_MAX_ALARM_CNT;
			//! LEL异常值, 低限报警.
			sensordata[GS_SENSOR_CHN_LEL].alarm = GS_LOW_LEVEL_ALARM;
		}
		sensordata[GS_SENSOR_CHN_LEL].low_cnt = 0;
	}
	else
	{
		sensordata[GS_SENSOR_CHN_LEL].low_cnt = 0;
		sensordata[GS_SENSOR_CHN_LEL].high_cnt = 0;
		//! LEL正常值, 无报警.
		sensordata[GS_SENSOR_CHN_LEL].alarm = GS_NONE_ALARM;
	}

    if ((sensordata[GS_SENSOR_CHN_O2].alarm == GS_HIGH_LEVEL_ALARM) 
        || (sensordata[GS_SENSOR_CHN_CO].alarm == GS_HIGH_LEVEL_ALARM) 
        || (sensordata[GS_SENSOR_CHN_H2S].alarm == GS_HIGH_LEVEL_ALARM) 
        || (sensordata[GS_SENSOR_CHN_LEL].alarm == GS_HIGH_LEVEL_ALARM))
    {
        gs_alarm_ctrl(GS_HIGH_LEVEL_ALARM);
    }
    else if ((sensordata[GS_SENSOR_CHN_O2].alarm == GS_LOW_LEVEL_ALARM) 
        || (sensordata[GS_SENSOR_CHN_CO].alarm == GS_LOW_LEVEL_ALARM) 
        || (sensordata[GS_SENSOR_CHN_H2S].alarm == GS_LOW_LEVEL_ALARM) 
        || (sensordata[GS_SENSOR_CHN_LEL].alarm == GS_LOW_LEVEL_ALARM))
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
		else if (syspara.battery_volt >= 3.95f)
		{
			syspara.soc = 75.0f;
			gs_soc_led_output_ctrl(SOC_075_GREEN);
		}
		else if (syspara.battery_volt >= 3.7f)
		{
			syspara.soc = 50.0f;
			gs_soc_led_output_ctrl(SOC_050_GREEN);
		}
		else if (syspara.battery_volt >= 3.45f)
		{
			syspara.soc = 25.0f;
			gs_soc_led_output_ctrl(SOC_025_GREEN_FLASH);
		}
		#if 0
		else if (syspara.battery_volt >= 3.2f)
		{
			syspara.soc = 0.0f;
			gs_soc_led_output_ctrl(SOC_000_GREEN_FLASH_BEEP);
		}
		#endif
		else
		{
			syspara.soc = 0.0f;
			gs_soc_led_output_ctrl(SOC_000_GREEN_FLASH_BEEP);
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

void gs_debug_uart_process(void)
{
	uint8_t buff[64] = {0};
	uint16_t len = 0;
	char *p = NULL;
	
	if (0 == gs_uart_recv(DEBUG_UART, buff, &len))
	{
		if (0 == strcmp((char *)buff, "getid\r\n"))
		{
			gs_printf("get the device id is %d.\r\n", configdata.id);
		}
		else if (NULL != (p = strstr((char *)buff, "setid")))
		{
			sscanf(p, "setid %d\r\n", &configdata.id);
			gs_printf("set the device id is %d.\r\n", configdata.id);
			gs_write_e2prom_info(&configdata);
		}
		else if (0 == strcmp((char *)buff, "logcat on\r\n"))
		{
			syspara.logcat = 1;
		}
		else if (0 == strcmp((char *)buff, "logcat off\r\n"))
		{
			syspara.logcat = 0;
		}
		else if (0 == strcmp((char *)buff, "getpara o2\r\n"))
		{
			gs_printf("get the o2 para is %f, %f, o2 %f%%.\r\n", 
			configdata.para[GS_SENSOR_CHN_O2].a, configdata.para[GS_SENSOR_CHN_O2].b, sensordata[GS_SENSOR_CHN_O2].val);
		}
		else if (NULL != (p = strstr((char *)buff, "setpara o2")))
		{
			sscanf(p, "setpara o2 %f, %f\r\n", &configdata.para[GS_SENSOR_CHN_O2].a, &configdata.para[GS_SENSOR_CHN_O2].b);
			gs_printf("set the o2 para is %f, %f.\r\n", configdata.para[GS_SENSOR_CHN_O2].a, configdata.para[GS_SENSOR_CHN_O2].b);
			gs_write_e2prom_info(&configdata);
		}
		else if (0 == strcmp((char *)buff, "getpara co\r\n"))
		{
			gs_printf("get the co para is %f, %f, co %f%%.\r\n", 
			configdata.para[GS_SENSOR_CHN_CO].a, configdata.para[GS_SENSOR_CHN_CO].b, sensordata[GS_SENSOR_CHN_CO].val);
		}
		else if (NULL != (p = strstr((char *)buff, "setpara co")))
		{
			sscanf(p, "setpara co %f, %f\r\n", &configdata.para[GS_SENSOR_CHN_CO].a, &configdata.para[GS_SENSOR_CHN_CO].b);
			gs_printf("set the co para is %f, %f.\r\n", configdata.para[GS_SENSOR_CHN_CO].a, configdata.para[GS_SENSOR_CHN_CO].b);
			gs_write_e2prom_info(&configdata);
		}
		else if (0 == strcmp((char *)buff, "getpara h2s\r\n"))
		{
			gs_printf("get the h2s para is %f, %f, h2s %f%%.\r\n", 
			configdata.para[GS_SENSOR_CHN_H2S].a, configdata.para[GS_SENSOR_CHN_H2S].b, sensordata[GS_SENSOR_CHN_H2S].val);
		}
		else if (NULL != (p = strstr((char *)buff, "setpara h2s")))
		{
			sscanf(p, "setpara h2s %f, %f\r\n", &configdata.para[GS_SENSOR_CHN_H2S].a, &configdata.para[GS_SENSOR_CHN_H2S].b);
			gs_printf("set the h2s para is %f, %f.\r\n", configdata.para[GS_SENSOR_CHN_H2S].a, configdata.para[GS_SENSOR_CHN_H2S].b);
			gs_write_e2prom_info(&configdata);
		}
		else if (0 == strcmp((char *)buff, "getpara lel\r\n"))
		{
			gs_printf("get the lel para is %f, %f, lel %f%%.\r\n", 
			configdata.para[GS_SENSOR_CHN_LEL].a, configdata.para[GS_SENSOR_CHN_LEL].b, sensordata[GS_SENSOR_CHN_LEL].val);
		}
		else if (NULL != (p = strstr((char *)buff, "setpara lel")))
		{
			sscanf(p, "setpara lel %f, %f\r\n", &configdata.para[GS_SENSOR_CHN_LEL].a, &configdata.para[GS_SENSOR_CHN_LEL].b);
			gs_printf("set the lel para is %f, %f.\r\n", configdata.para[GS_SENSOR_CHN_LEL].a, configdata.para[GS_SENSOR_CHN_LEL].b);
			gs_write_e2prom_info(&configdata);
		}
	}
}

void gs_debug_uart_logcat(void)
{
	if (gs_s_msleep_is_timesup(&logcat_msleep))
	{
		if (1 == syspara.logcat)
		{
			gs_printf("T: %lu, %d, volt: %f V, temp: %f ℃, ", syspara.heartbeat, syspara.flag_filter_out_adc, syspara.battery_volt, syspara.mcu_temp);
			gs_printf("O2: %d, CO: %d, H2S: %d, LEL: %d, BT: %d, %f, %f, %f, %f.\r\n", 
			adcpara.filtered_data[2], adcpara.filtered_data[3], adcpara.filtered_data[4], adcpara.filtered_data[5], syspara.buttom, 
			sensordata[GS_SENSOR_CHN_O2].val, sensordata[GS_SENSOR_CHN_CO].val, sensordata[GS_SENSOR_CHN_H2S].val, sensordata[GS_SENSOR_CHN_LEL].val);
		}
		gs_s_msleep_start_and_setting(&logcat_msleep, logcat_msleep.maxcnt);
	}
}

int gs_e32_set_configpara(uint16_t addr)
{
	char cmd[32];
	uint16_t len = 0;

    gs_e32_set_mode(E32_POWERDOWN_MODE);
	gs_msleep(1000);
	memset(cmd, 0, sizeof(cmd));
	cmd[len++] = 0xC0;
	cmd[len++] = 0x00;
	cmd[len++] = 0x09;
	cmd[len++] = ((addr >> 8) & 0xFF);//! ADDH.
	cmd[len++] = ((addr >> 0) & 0xFF);//! ADDL.
	cmd[len++] = 0x00;//! netid.
	cmd[len++] = 0xE4;//! 9600, 8N1, 9.6KBPS.
	cmd[len++] = 0xC0;//! 
	cmd[len++] = 23;//addr % 84;//! CHN.
	cmd[len++] = 0x10;
	cmd[len++] = 0x00;
	cmd[len++] = 0x00;
	gs_uart_send(LORA_UART, (uint8_t *)cmd, len);
	gs_msleep(200);
	memset(cmd, 0, sizeof(cmd));
	len = 0;
	gs_uart_recv(LORA_UART, (uint8_t *)cmd, &len);
	//! E22无线LoRa模式控制默认为正常模式.
    gs_e32_set_mode(E32_NORMAL_MODE);

	if (len != 0)
	{
		return (0);
	}
	return (-1);
}

int gs_read_e2prom_info(GS_CONFIG_DATA *configdata)
{
	configdata->id = *(__IO uint32_t *)(E2PROM_INFO_ADDRESS + 0);
	configdata->para[GS_SENSOR_CHN_O2].a  = *(__IO float *)(E2PROM_INFO_ADDRESS + 4);
	configdata->para[GS_SENSOR_CHN_O2].b  = *(__IO float *)(E2PROM_INFO_ADDRESS + 8);
	configdata->para[GS_SENSOR_CHN_CO].a  = *(__IO float *)(E2PROM_INFO_ADDRESS + 12);
	configdata->para[GS_SENSOR_CHN_CO].b  = *(__IO float *)(E2PROM_INFO_ADDRESS + 16);
	configdata->para[GS_SENSOR_CHN_H2S].a = *(__IO float *)(E2PROM_INFO_ADDRESS + 20);
	configdata->para[GS_SENSOR_CHN_H2S].b = *(__IO float *)(E2PROM_INFO_ADDRESS + 24);
	configdata->para[GS_SENSOR_CHN_LEL].a = *(__IO float *)(E2PROM_INFO_ADDRESS + 28);
	configdata->para[GS_SENSOR_CHN_LEL].b = *(__IO float *)(E2PROM_INFO_ADDRESS + 32);
	
	return (0);
}

int gs_write_e2prom_info(GS_CONFIG_DATA *configdata)
{
	GS_UNION_FLOAT uf;
	FLASH_Unlock();
	FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);
	FLASH_ErasePage(E2PROM_INFO_ADDRESS);
	FLASH_ProgramWord((E2PROM_INFO_ADDRESS + 0),  configdata->id);
	
	uf.fval = configdata->para[GS_SENSOR_CHN_O2].a;
	FLASH_ProgramWord((E2PROM_INFO_ADDRESS + 4),  uf.val);
	uf.fval = configdata->para[GS_SENSOR_CHN_O2].b;
	FLASH_ProgramWord((E2PROM_INFO_ADDRESS + 8),  uf.val);
	
	uf.fval = configdata->para[GS_SENSOR_CHN_CO].a;
	FLASH_ProgramWord((E2PROM_INFO_ADDRESS + 12), uf.val);
	uf.fval = configdata->para[GS_SENSOR_CHN_CO].b;
	FLASH_ProgramWord((E2PROM_INFO_ADDRESS + 16), uf.val);
	
	uf.fval = configdata->para[GS_SENSOR_CHN_H2S].a;
	FLASH_ProgramWord((E2PROM_INFO_ADDRESS + 20), uf.val);
	uf.fval = configdata->para[GS_SENSOR_CHN_H2S].b;
	FLASH_ProgramWord((E2PROM_INFO_ADDRESS + 24), uf.val);
	
	uf.fval = configdata->para[GS_SENSOR_CHN_LEL].a;
	FLASH_ProgramWord((E2PROM_INFO_ADDRESS + 28), uf.val);
	uf.fval = configdata->para[GS_SENSOR_CHN_LEL].b;
	FLASH_ProgramWord((E2PROM_INFO_ADDRESS + 32), uf.val);
	FLASH_Lock();
	return (0);
}

int gs_config_data_init(GS_CONFIG_DATA *configdata)
{
	GS_UNION_FLOAT uf;
	gs_read_e2prom_info(configdata);
	
	if (configdata->id == 0xFFFFFFFF)
	{
		configdata->id = 1;
	}
	
	uf.fval = configdata->para[GS_SENSOR_CHN_O2].a;
	if (uf.val == 0xFFFFFFFF)
	{
		configdata->para[GS_SENSOR_CHN_O2].a = 1.50f;
	}
	uf.fval = configdata->para[GS_SENSOR_CHN_O2].b;
	if (uf.val == 0xFFFFFFFF)
	{
		configdata->para[GS_SENSOR_CHN_O2].b = -2.545f;
	}
	
	uf.fval = configdata->para[GS_SENSOR_CHN_CO].a;
	if (uf.val == 0xFFFFFFFF)
	{
		configdata->para[GS_SENSOR_CHN_CO].a = 787.61904762f;
	}
	uf.fval = configdata->para[GS_SENSOR_CHN_CO].b;
	if (uf.val == 0xFFFFFFFF)
	{
		configdata->para[GS_SENSOR_CHN_CO].b = -1331.0f;
	}
	
	uf.fval = configdata->para[GS_SENSOR_CHN_H2S].a;
	if (uf.val == 0xFFFFFFFF)
	{
		configdata->para[GS_SENSOR_CHN_H2S].a = 211.0f;
	}
	uf.fval = configdata->para[GS_SENSOR_CHN_H2S].b;
	if (uf.val == 0xFFFFFFFF)
	{
		configdata->para[GS_SENSOR_CHN_H2S].b = -354.0f;
	}
	
	uf.fval = configdata->para[GS_SENSOR_CHN_LEL].a;
	if (uf.val == 0xFFFFFFFF)
	{
		configdata->para[GS_SENSOR_CHN_LEL].a = 1.03f;
	}
	uf.fval = configdata->para[GS_SENSOR_CHN_LEL].b;
	if (uf.val == 0xFFFFFFFF)
	{
		configdata->para[GS_SENSOR_CHN_LEL].b = -1.8576f;
	}
	return (0);
}
