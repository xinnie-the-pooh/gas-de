#include "gs_api.h"
#include <string.h>
#include "gs_global_para.h"

void gs_s_msleep_start(S_MSLEEP *s)
{
	s->enable_timer = 1;
}

void gs_s_msleep_stop(S_MSLEEP *s)
{
	s->enable_timer = 0;
}

void gs_s_msleep_clear(S_MSLEEP *s)
{
	memset(s, 0x00, sizeof(S_MSLEEP));
}

uint8_t gs_s_msleep_is_timesup(S_MSLEEP *s)
{
	if ((1 == s->enable_timer) && ((syspara.heartbeat - s->cnt) >= s->maxcnt))
	{
		return (1);
	}
	return (0);
}

void gs_s_msleep_setting(S_MSLEEP *s, uint16_t ms)
{
	s->maxcnt = ms;
	s->cnt = syspara.heartbeat;
}

void gs_s_msleep_start_and_setting(S_MSLEEP *s, uint16_t ms)
{
	s->enable_timer = 1;
	s->maxcnt = ms;
	s->cnt = syspara.heartbeat;
}

int gs_is_data_all_xx(uint8_t *buff, uint8_t len, uint8_t xx)
{
	uint8_t i;

	for (i = 0; i < len; i++)
	{
		if (buff[i] != xx)
		{
			return (-1);
		}
	}
	return (0);
}

uint16_t gs_moving_average_filter_uint16_t(uint16_t input, uint16_t *buff, int len)
{
	int i;
	uint32_t sum = 0x00;

	for (i = 0; i < len - 1; i++)
	{
		buff[i] = buff[i + 1];
        sum += buff[i];
	}
	buff[len - 1] = input;
    sum += input;

	return (sum / len);
}

void gs_print_hex_data(char *fmt, uint8_t *buff, uint16_t len)
{
	uint16_t i;
	gs_printf(fmt);
	for (i = 0; i < len; i++)
	{
		gs_printf("%02X ", buff[i]);
	}
	gs_printf("\r\n\r\n");
}

uint16_t gs_calc_cs(uint8_t *buff, int len)
{
	int i;
	uint16_t cs = 0;

	for (i = 0; i < len; i++)
	{
		cs += *(buff + i);
	}
	return (cs);
}

float gs_pow(float val, int n)
{
	int i;
	float ret = 1.0f;

	for (i = 0; i < n; i++)
	{
		ret *= val;
	}
	return (ret);
}

