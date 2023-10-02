#ifndef __GS_HARMFUL_GAS_DETECTOR_H__
#define __GS_HARMFUL_GAS_DETECTOR_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "stdint.h"
#include "gs_typedef.h"

#define BL_ADDRESS				(0x08000000)	//! Flash Size is 32KB.
#define BL_ENV_ADDRESS			(0x08008000)	//! Flash Size is 2KB.
#define FLASH2EEPROM_ADDRESS	(0x08008800)	//! Flash Size is 2KB.
#define APPLICATION_ADDRESS		(0x08000000)	//! Flash Size is 220KB.
#define VERSION_OFFSET			(0x00002000)	//! Offset is 8KB.
#define E2PROM_INFO_ADDRESS		(0x0803F800)	//! Offset is 2KB.

extern const VERSION_INFO version;

void gs_send_jxm_data_by_lora(void);
void gs_send_jxm_data_periodic(void);

/** 
  * @brief  报警等级枚举.
  */
typedef enum
{
    GS_NONE_ALARM       = 0,    /*!< 无报警. */
    GS_LOW_LEVEL_ALARM  = 1,    /*!< 低限报警. */
    GS_HIGH_LEVEL_ALARM = 2,    /*!< 高限报警. */
} GS_ALARM_LEVEL;

typedef struct
{
    float val;
    uint32_t alarm;
	int16_t low_cnt;
	int16_t high_cnt;
}GS_SENSOR_DATA;

void gs_alarm_ctrl(GS_ALARM_LEVEL level);
void gs_jxm_alarm_ctrl(void);
void gs_calc_battery_soc(void);
void gs_debug_uart_init(void);
void gs_debug_uart_process(void);
void gs_debug_uart_logcat(void);
int gs_e32_set_configpara(uint16_t addr);

/** 
  * @brief  .
  */
typedef enum
{
	GS_SENSOR_CHN_O2  = 0,	/*!< . */
	GS_SENSOR_CHN_CO  = 1,	/*!< . */
    GS_SENSOR_CHN_H2S = 2,	/*!< . */
    GS_SENSOR_CHN_LEL = 3,	/*!< . */
    GS_SENSOR_CHN_MAX = 4,	/*!< . */
}GS_SENSOR_CHN;

typedef struct
{
	uint32_t val;
	float a;
    float b;
}GS_SENSOR_PARA;

typedef struct
{
    uint32_t id;
	GS_SENSOR_PARA para[GS_SENSOR_CHN_MAX];
}GS_CONFIG_DATA;

typedef union
{
	uint32_t val;
	float    fval;
}GS_UNION_FLOAT;

int gs_read_e2prom_info(GS_CONFIG_DATA *configdata);
int gs_write_e2prom_info(GS_CONFIG_DATA *configdata);
int gs_config_data_init(GS_CONFIG_DATA *configdata);

#ifdef __cplusplus
}
#endif

#endif
