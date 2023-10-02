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
#define CMD_INFO_ADDRESS		(0x0803F800)	//! Offset is 8KB.

extern const VERSION_INFO version;

typedef enum
{
    JXM_WORK_ACTIVE_REPORTING_MODE = 1,
    JXM_WORK_INQUIRY_MODE = 2,
}JXM_WORK_MODE;

#pragma pack(1)
typedef struct
{
    uint8_t status;
    uint8_t resolution;
    uint16_t sensor;
    float val;
    uint8_t alarm;
}JXM_DATA;
#pragma pack()

void gs_jxm_get_sensor_data(void);
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

void gs_alarm_ctrl(GS_ALARM_LEVEL level);
void gs_jxm_alarm_ctrl(void);

#ifdef __cplusplus
}
#endif

#endif
