#ifndef __GS_BSP_H__
#define __GS_BSP_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f10x.h"
#include "gs_typedef.h"

//! 系统时钟初始化函数声明.
void gs_sysclock_init(void);
//! 系统嘀嗒定时器初始化函数声明.
void gs_systick_init(uint32_t freq);

#define WORK_LED_RCC_PORT			(RCC_APB2Periph_GPIOA)
#define WORK_LED_GPIO_PORT			(GPIOA)
#define WORK_LED_GPIO_PIN			(GPIO_Pin_15)
#define WORK_LED_FLASH_CYCLE		(100)

void gs_work_led_init(LED_PARA *ledpara, uint16_t timesout_ms);
void gs_work_led_reverse(void);
void gs_work_led_flash(LED_PARA *ledpara);

#define ALARM_LED_RCC_PORT			(RCC_APB2Periph_GPIOA)
#define ALARM_LED_GPIO_PORT			(GPIOA)
#define ALARM_LED_GPIO_PIN			(GPIO_Pin_7)
#define ALARM_LED_FLASH_CYCLE		(100)

void gs_alarm_led_init(LED_PARA *ledpara, uint16_t timesout_ms);
void gs_alarm_led_reverse(void);
void gs_alarm_beep_on(void);
void gs_alarm_beep_off(void);
void gs_alarm_led_flash(LED_PARA *ledpara);

#define ALARM_BEEP_RCC_PORT			(RCC_APB2Periph_GPIOA)
#define ALARM_BEEP_GPIO_PORT		(GPIOA)
#define ALARM_BEEP_GPIO_PIN			(GPIO_Pin_4)

void gs_alarm_beep_init(void);
void gs_alarm_beep_reverse(void);
void gs_alarm_beep_on(void);
void gs_alarm_beep_off(void);

#define JXM_S0_RCC_PORT				(RCC_APB2Periph_GPIOA)
#define JXM_S0_GPIO_PORT		    (GPIOA)
#define JXM_S0_GPIO_PIN				(GPIO_Pin_11)

#define JXM_S1_RCC_PORT				(RCC_APB2Periph_GPIOA)
#define JXM_S1_GPIO_PORT			(GPIOA)
#define JXM_S1_GPIO_PIN				(GPIO_Pin_12)

/** 
  * @brief  继电器控制枚举.
  */
typedef enum
{
	JXM_SWITCH_O2  = 0,	/*!< 继电器不吸合. */
	JXM_SWITCH_CO  = 1,	/*!< 继电器吸合. */
    JXM_SWITCH_H2S = 2,	/*!< 继电器吸合. */
    JXM_SWITCH_LEL = 3,	/*!< 继电器吸合. */
    JXM_SWITCH_MAX = 4,	/*!< 继电器吸合. */
}JXM_SWITCH;

void gs_jxm_switch_init(void);
void gs_jxm_switch_ctrl(JXM_SWITCH jxm);

typedef enum
{
	E32_NORMAL_MODE    = 0, 
	E32_WAKEUP_MODE    = 1,
	E32_POWERDOWN_MODE = 2,
	E32_SLEEP_MODE     = 3,
}E32_MODE;

#define E32_M0_RCC_PORT						(RCC_APB2Periph_GPIOB)
#define E32_M0_GPIO_PORT					(GPIOB)
#define E32_M0_GPIO_PIN						(GPIO_Pin_12)

#define E32_M1_RCC_PORT						(RCC_APB2Periph_GPIOB)
#define E32_M1_GPIO_PORT					(GPIOB)
#define E32_M1_GPIO_PIN						(GPIO_Pin_13)

void gs_e32_m0m1_port_init(void);
void gs_e32_set_mode(E32_MODE e32mode);

/** 
  * @brief  串口号枚举.
  */
typedef enum
{
	JXM_UART     = 1,	/*!< 调试RS232串口. */
	LORA_UART    = 2,	/*!< 通讯RS232串口. */
}UART_NUM;

/**
  * @brief  UART波特率.
  */
typedef enum
{
	NONE_PARITY = 0x01,	/*!< 无校验. */
	ODD_PARITY  = 0x02,	/*!< 奇校验. */
	EVEN_PARITY = 0x03,	/*!< 偶校验. */
}UART_PARITY;

//! 串口发送缓冲区大小.
#define UART_TX_BUFF_SIZE		(512)
//! 串口接收缓冲区大小.
#define UART_RX_BUFF_SIZE		(512)

/**
  * @brief  UART结构体定义.
  */
typedef struct
{
	uint8_t rxdata[UART_RX_BUFF_SIZE];	/*!< 接收缓冲区. */
	uint8_t txdata[UART_TX_BUFF_SIZE];	/*!< 发送缓冲区. */
	uint8_t flag_rx;		/*!< 接收完成标志. */
	uint8_t flag_tx;		/*!< 发送完成标志. */
//	uint8_t flag_rxtimesout;/*!< 接收超时标志. */
//	uint8_t flag_txtimesout;/*!< 发送超时标志. */
//	uint16_t cnt_rx;		/*!< 接收超时计数, 单位为ms. */
//	uint16_t cnt_tx;		/*!< 发送超时计数, 单位为ms. */
//	uint16_t maxcnt_rx;		/*!< 接收超时时间, 单位为ms. */
//	uint16_t maxcnt_tx;		/*!< 发送超时时间, 单位为ms. */
	uint16_t rxlen;			/*!< 接收数据个数. */
	uint16_t rxmaxlen;		/*!< 接收数据最大个数. */
}UART_PARA;

//! JXM TTL UART.
#define JXM_UART_IRQ_CHN					(USART1_IRQn)
#define JXM_UART_RCC_PORT				    (RCC_APB2Periph_USART1)
#define JXM_UART_DMA_RCC_PORT			    (RCC_AHBPeriph_DMA1)
#define JXM_UART_PORT					    (USART1)
#define JXM_UART_TX_DMA_IRQ_CHN			    (DMA1_Channel4_IRQn)
#define JXM_UART_RX_DMA_IRQ_CHN			    (DMA1_Channel5_IRQn)
#define JXM_UART_TX_DMA_CHANNEL			    (DMA1_Channel4)
#define JXM_UART_RX_DMA_CHANNEL			    (DMA1_Channel5)
#define JXM_UART_TX_DMA_IT_TC			    (DMA1_IT_TC4)
#define JXM_UART_RX_DMA_FLAG				(DMA1_FLAG_TC5)

#define JXM_UART_TX_RCC_PORT				(RCC_APB2Periph_GPIOA)
#define JXM_UART_TX_GPIO_PORT			    (GPIOA)
#define JXM_UART_TX_GPIO_PIN				(GPIO_Pin_9)

#define JXM_UART_RX_RCC_PORT				(RCC_APB2Periph_GPIOA)
#define JXM_UART_RX_GPIO_PORT			    (GPIOA)
#define JXM_UART_RX_GPIO_PIN				(GPIO_Pin_10)

//! LORA TTL UART.
#define LORA_UART_IRQ_CHN					(USART3_IRQn)
#define LORA_UART_RCC_PORT					(RCC_APB1Periph_USART3)
#define LORA_UART_DMA_RCC_PORT				(RCC_AHBPeriph_DMA1)
#define LORA_UART_PORT						(USART3)
#define LORA_UART_TX_DMA_IRQ_CHN			(DMA1_Channel2_IRQn)
#define LORA_UART_RX_DMA_IRQ_CHN			(DMA1_Channel3_IRQn)
#define LORA_UART_TX_DMA_CHANNEL			(DMA1_Channel2)
#define LORA_UART_RX_DMA_CHANNEL			(DMA1_Channel3)
#define LORA_UART_TX_DMA_IT_TC				(DMA1_IT_TC2)
#define LORA_UART_RX_DMA_FLAG				(DMA1_FLAG_TC3)

#define LORA_UART_TX_RCC_PORT				(RCC_APB2Periph_GPIOB)
#define LORA_UART_TX_GPIO_PORT				(GPIOB)
#define LORA_UART_TX_GPIO_PIN				(GPIO_Pin_10)

#define LORA_UART_RX_RCC_PORT				(RCC_APB2Periph_GPIOB)
#define LORA_UART_RX_GPIO_PORT				(GPIOB)
#define LORA_UART_RX_GPIO_PIN				(GPIO_Pin_11)

void gs_uart_init(UART_NUM uart_num, uint32_t baudrate, uint8_t parity);
void gs_uart_send(UART_NUM uart_num, uint8_t *buff, uint16_t len);
int gs_uart_recv(UART_NUM uart_num, uint8_t *buff, uint16_t *len);
void gs_uart_test(UART_NUM uart_num);
void gs_printf(char *fmt, ...);

//! B_VOLT_ADC.
#define B_VOLT_ADC_IRQ_CHN					(ADC1_IRQn)
#define B_VOLT_ADC_RCC_PORT					(RCC_APB2Periph_ADC1)
#define B_VOLT_ADC_DMA_RCC_PORT				(RCC_AHBPeriph_DMA1)
#define B_VOLT_ADC_PORT						(ADC1)
#define B_VOLT_ADC_DMA_IRQ_CHN				(DMA1_Channel1_IRQn)
#define B_VOLT_ADC_DMA_CHANNEL				(DMA1_Channel1)
#define B_VOLT_ADC_DMA_IT_TC				(DMA1_IT_TC1)

#define B_VOLT_RCC_PORT						(RCC_APB2Periph_GPIOC)
#define B_VOLT_GPIO_PORT					(GPIOC)
#define B_VOLT_GPIO_PIN						(GPIO_Pin_4)

#define ADC1_DR_ADDR						((uint32_t)0x4001244C)

//! ADC采样通道数
#define ADC_CHANNEL_NUM						(2)
//! ADC每通道采样次数
#define ADC_SAMPLE_NUM						(15)

typedef struct
{
	uint8_t  flag_adc_sample;						//! ADC采样完成标志, 置1, 采样完.
	uint8_t  flag_adc_filtered;						//! ADC滤波完.
	uint16_t data[ADC_CHANNEL_NUM];					//! DMA方式下的A/D采样值.
	uint16_t buff[ADC_CHANNEL_NUM][ADC_SAMPLE_NUM];	//! ADC采样值.
	uint16_t filtered_data[ADC_CHANNEL_NUM];		//! ADC滤波后的数据.
}ADC_PARA;

void gs_adc_init(void);
void gs_adc_dma_restart(void);
void gs_calc_adc_data(void);

void WFI_SET(void);
void INTX_DISABLE(void);
void INTX_ENABLE(void);
__asm void MSR_MSP(u32 addr);
void qu_delay(void);
void gs_usleep(uint16_t us);
void gs_msleep(uint16_t ms);

#ifdef __cplusplus
}
#endif

#endif

