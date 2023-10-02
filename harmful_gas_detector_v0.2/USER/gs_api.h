#ifndef __GS_API_H__
#define __GS_API_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f10x.h"
#include "gs_typedef.h"

void gs_s_msleep_start(S_MSLEEP *s);
void gs_s_msleep_stop(S_MSLEEP *s);
void gs_s_msleep_clear(S_MSLEEP *s);
uint8_t gs_s_msleep_is_timesup(S_MSLEEP *s);
void gs_s_msleep_setting(S_MSLEEP *s, uint16_t ms);
void gs_s_msleep_start_and_setting(S_MSLEEP *s, uint16_t ms);

int gs_is_data_all_xx(uint8_t *buff, uint8_t len, uint8_t xx);
uint16_t gs_moving_average_filter_uint16_t(uint16_t input, uint16_t *buff, int len);
void gs_print_hex_data(char *fmt, uint8_t *buff, uint16_t len);
uint16_t gs_calc_cs(uint8_t *buff, int len);
float gs_pow(float val, int n);

#ifdef __cplusplus
}
#endif

#endif

