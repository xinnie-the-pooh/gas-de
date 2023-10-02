#include "gs_bsp.h"
#include "gs_typedef.h"
#include "gs_global_para.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include "gs_api.h"

/*
 * Function Name: gs_sysclock_init
 * Function Prototype: void gs_sysclock_init(void);
 * Function Description: ���ÿ�����ϵͳʱ��, ����AHB, APB2��APB1ʱ��.
 *                       ���16MHz�ľ���;
 *                       AHB  = 24 MHz;
 *                       APB2 = 24 MHz;
 *                       APB1 = 12 MHz.
 * Entrance Parameters: None.
 * Export Parameters: None.
 * Return Value: None.
 * Version: V0.01
 * Modification Time: 2020.02.06 17:11
 * Author: WYP
 * Status: �Ѳ���
 */
void gs_sysclock_init(void)
{
	ErrorStatus HSEStartUpStatus;

	//! RCC system reset(for debug purpose).
	RCC_DeInit();

	//! Enable HSE.
	RCC_HSEConfig(RCC_HSE_ON);

	//! Wait till HSE is ready.
	HSEStartUpStatus = RCC_WaitForHSEStartUp();

	if (HSEStartUpStatus == SUCCESS)
	{
		//! Enable Prefetch Buffer.
		FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable);

		//! zero wait state, if 0 < SYSCLK <= 24 MHz.
		//! one wait state, if 24 MHz < SYSCLK <= 48 MHz.
		//! two wait states, if 48 MHz < SYSCLK <= 72 MHz.
		//! Flash 2 wait state.
		FLASH_SetLatency(FLASH_Latency_0);

		//! HCLK=SYSCLK/1=24MHz.
		RCC_HCLKConfig(RCC_SYSCLK_Div1);

		//! PCLK2=HCLK=24MHz.
		RCC_PCLK2Config(RCC_HCLK_Div1);

		//! PCLK1=HCLK/2=12MHz.
		RCC_PCLK1Config(RCC_HCLK_Div2);

#ifdef STM32F10X_CL
		//! 3MHz <= PLL2IN <= 5MHz.
		//! PLL2IN=16MHz/4=4MHz.
		RCC_PREDIV2Config(RCC_PREDIV2_Div4);
		//! 40MHz <= PLL2OUT <= 74MHz.
		//! PLL2OUT=4MHz*10=40MHz.
		RCC_PLL2Config(RCC_PLL2Mul_10);
		//! 3MHz <= PLLIN <= 12MHz.
		//! PLLIN=40MHz/10=4MHz.
		RCC_PREDIV1Config(RCC_PREDIV1_Source_PLL2, RCC_PREDIV1_Div10);
		RCC_PLL2Cmd(ENABLE);
		while (RCC_GetFlagStatus(RCC_FLAG_PLL2RDY) == RESET);
		//! 18MHz <= PLLOUT <= 72MHz.
		//! PLLOUT=4MHz*6=72MHz.
		//! PLLCLK=4MHz*6=72MHz.
		RCC_PLLConfig(RCC_PLLSource_PREDIV1, RCC_PLLMul_6);
#else
		//! 1MHz <= PLLIN <= 25MHz.
		//! PLLIN=16MHz/2=8MHz.
		//! 16MHz <= PLLOUT <= 72MHz.
		//! PLLOUT=8MHz*9=72MHz.
		//! PLLCLK=8MHz*9=72MHz
		RCC_PLLConfig(RCC_PLLSource_HSE_Div2, RCC_PLLMul_9);
#endif

		//! Enable PLL.
		RCC_PLLCmd(ENABLE);

		//! Wait till PLL is ready.
		while (RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET);

		//! Select PLL as system clock source.
		RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);

		//! Wait till PLL is used as system clock source.
		while (RCC_GetSYSCLKSource() != 0x08);
	}
}

/*
 * Function Name: gs_systick_init
 * Function Prototype: void gs_systick_init(void);
 * Function Description: ����SysTick��ʱ��, ʹ�����1ms�ж�.
 *                       HCLK = AHB =72 MHz.
 * Entrance Parameters: None.
 * Export Parameters: None.
 * Return Value: None.
 * Version: V0.01
 * Modification Time: 2020.02.06 17:18
 * Author: WYP
 * Status: �Ѳ���
 */
void gs_systick_init(uint32_t freq)
{
#if 0
	//! ʹ��SysTick�⺯��.
	//! SysTickʱ��ԴΪAHBʱ��.
	SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK);

	//! ����SysTick��װ��ֵ.
	SysTick_SetReload(SystemCoreClock/1000);

	//! set Priority for Cortex-M0 System Interrupts.
	NVIC_SetPriority (SysTick_IRQn, (1 << __NVIC_PRIO_BITS) - 1);

	//! ʹ��SysTick�ж�.
	SysTick_ITConfig(ENABLE);

	//! ���������ֵΪ0.
	SysTick_CounterCmd(SysTick_Counter_Clear);

	//! ʹ��SysTick������.
	SysTick_CounterCmd(SysTick_Counter_Enable);
#else
	//! ��ʱʱ��Ϊ1ms.
	SysTick_Config(SystemCoreClock / freq);
#endif
}

/*
 * Function Name: gs_led_init
 * Function Prototype: void gs_led_init(void);
 * Function Description: LEDָʾ�ƶ˿ڳ�ʼ��.
 * Entrance Parameters: None.
 * Export Parameters: None.
 * Return Value: None.
 * Version: V00.01
 * Modification Time: 2020.02.06 18:28
 * Author: WYP
 * Status: �Ѳ���
 */
void gs_work_led_init(LED_PARA *ledpara, uint16_t timesout_ms)
{
	GPIO_InitTypeDef GPIO_InitStruct;

	//! ʹ��GPIO��ʱ��.
	RCC_APB2PeriphClockCmd(WORK_LED_RCC_PORT, ENABLE);

	GPIO_InitStruct.GPIO_Pin   = WORK_LED_GPIO_PIN;	//! �ܽ�
	GPIO_InitStruct.GPIO_Mode  = GPIO_Mode_Out_PP;	//! �������ģʽ.
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;	//! ������ָʾϵͳ����״̬, �ٶȲ���̫��.
	GPIO_Init(WORK_LED_GPIO_PORT, &GPIO_InitStruct);

	GPIO_SetBits(WORK_LED_GPIO_PORT, WORK_LED_GPIO_PIN);//! LEDָʾ��Ϩ��.

	gs_s_msleep_start_and_setting(ledpara, timesout_ms);
}

void gs_work_led_reverse(void)
{
	static uint8_t status = 0x00;

	if (status == 0x00)
	{
		GPIO_ResetBits(WORK_LED_GPIO_PORT, WORK_LED_GPIO_PIN);
		status = 0x01;
	}
	else if (status == 0x01)
	{
		GPIO_SetBits(WORK_LED_GPIO_PORT, WORK_LED_GPIO_PIN);
		status = 0x00;
	}
}

void gs_work_led_flash(LED_PARA *ledpara)
{
	if (gs_s_msleep_is_timesup(ledpara))
	{
		gs_work_led_reverse();
		gs_s_msleep_start_and_setting(ledpara, ledpara->maxcnt);
	}
}

void gs_soc_led_init(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_Disable, ENABLE);
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);

	//! ʹ��GPIO��ʱ��.
	RCC_APB2PeriphClockCmd(SOC0_LED_RCC_PORT, ENABLE);
	GPIO_InitStruct.GPIO_Pin   = SOC0_LED_GPIO_PIN;	//! �ܽ�
	GPIO_InitStruct.GPIO_Mode  = GPIO_Mode_Out_PP;	//! �������ģʽ.
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;	//! ������ָʾϵͳ����״̬, �ٶȲ���̫��.
	GPIO_Init(SOC0_LED_GPIO_PORT, &GPIO_InitStruct);
	GPIO_SetBits(SOC0_LED_GPIO_PORT, SOC0_LED_GPIO_PIN);//! LEDָʾ��Ϩ��.

	//! ʹ��GPIO��ʱ��.
	RCC_APB2PeriphClockCmd(SOC1_LED_RCC_PORT, ENABLE);
	GPIO_InitStruct.GPIO_Pin   = SOC1_LED_GPIO_PIN;	//! �ܽ�
	GPIO_InitStruct.GPIO_Mode  = GPIO_Mode_Out_PP;	//! �������ģʽ.
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;	//! ������ָʾϵͳ����״̬, �ٶȲ���̫��.
	GPIO_Init(SOC1_LED_GPIO_PORT, &GPIO_InitStruct);
	GPIO_SetBits(SOC1_LED_GPIO_PORT, SOC1_LED_GPIO_PIN);//! LEDָʾ��Ϩ��.
}

void gs_soc_led_output_ctrl(SOC_LED_STATUS status)
{
	static uint8_t flag = 0;
	switch (status)
	{
	case SOC_100_GREEN:
	case SOC_075_GREEN:
		GPIO_SetBits(SOC0_LED_GPIO_PORT, SOC0_LED_GPIO_PIN);
		GPIO_ResetBits(SOC1_LED_GPIO_PORT, SOC1_LED_GPIO_PIN);
		break;
	case SOC_050_GREEN_FLASH:
		GPIO_SetBits(SOC0_LED_GPIO_PORT, SOC0_LED_GPIO_PIN);
		if (0 == flag)
		{
			GPIO_ResetBits(SOC1_LED_GPIO_PORT, SOC1_LED_GPIO_PIN);
			flag = 1;
		}
		else if (1 == flag)
		{
			GPIO_SetBits(SOC1_LED_GPIO_PORT, SOC1_LED_GPIO_PIN);
			flag = 0;
		}
		break;
	case SOC_025_RED:
		GPIO_ResetBits(SOC0_LED_GPIO_PORT, SOC0_LED_GPIO_PIN);
		GPIO_SetBits(SOC1_LED_GPIO_PORT, SOC1_LED_GPIO_PIN);
		break;
	case SOC_000_RED_FLASH_BEEP:
		if (0 == flag)
		{
			GPIO_ResetBits(SOC0_LED_GPIO_PORT, SOC0_LED_GPIO_PIN);
			flag = 1;
		}
		else if (1 == flag)
		{
			GPIO_SetBits(SOC0_LED_GPIO_PORT, SOC0_LED_GPIO_PIN);
			flag = 0;
		}
		GPIO_SetBits(SOC1_LED_GPIO_PORT, SOC1_LED_GPIO_PIN);
		gs_alarm_ctrl(GS_LOW_LEVEL_ALARM);
		break;
	default:
		break;
	}
}

void gs_debug_uart_check_init(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;
    
	//! ʹ��GPIO��ʱ��.
	RCC_APB2PeriphClockCmd(DEBUG_UART_CHECK_RCC_PORT, ENABLE);

	GPIO_InitStruct.GPIO_Pin   = DEBUG_UART_CHECK_GPIO_PIN;	//! �ܽ�
	GPIO_InitStruct.GPIO_Mode  = GPIO_Mode_IPU;	//! �������ģʽ.
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;	//! ������ָʾϵͳ����״̬, �ٶȲ���̫��.
	GPIO_Init(DEBUG_UART_CHECK_GPIO_PORT, &GPIO_InitStruct);
}

uint8_t gs_debug_uart_check(void)
{
	if (Bit_SET == GPIO_ReadInputDataBit(DEBUG_UART_CHECK_GPIO_PORT, DEBUG_UART_CHECK_GPIO_PIN))
	{
		return (1);
	}
	return (0);
}

void gs_alarm_beep_init(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;
    
	//! ʹ��GPIO��ʱ��.
	RCC_APB2PeriphClockCmd(ALARM_BEEP_RCC_PORT, ENABLE);

	GPIO_InitStruct.GPIO_Pin   = ALARM_BEEP_GPIO_PIN;	//! �ܽ�
	GPIO_InitStruct.GPIO_Mode  = GPIO_Mode_Out_PP;	//! �������ģʽ.
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;	//! ������ָʾϵͳ����״̬, �ٶȲ���̫��.
	GPIO_Init(ALARM_BEEP_GPIO_PORT, &GPIO_InitStruct);

	GPIO_ResetBits(ALARM_BEEP_GPIO_PORT, ALARM_BEEP_GPIO_PIN);//! LEDָʾ��Ϩ��.
}

void gs_alarm_beep_reverse(void)
{
	static uint8_t status = 0x00;

	if (status == 0x00)
	{
		GPIO_ResetBits(ALARM_BEEP_GPIO_PORT, ALARM_BEEP_GPIO_PIN);
		status = 0x01;
	}
	else if (status == 0x01)
	{
		GPIO_SetBits(ALARM_BEEP_GPIO_PORT, ALARM_BEEP_GPIO_PIN);
		status = 0x00;
	}
}

void gs_alarm_beep_on(void)
{
	GPIO_SetBits(ALARM_BEEP_GPIO_PORT, ALARM_BEEP_GPIO_PIN);
}

void gs_alarm_beep_off(void)
{
	GPIO_ResetBits(ALARM_BEEP_GPIO_PORT, ALARM_BEEP_GPIO_PIN);
}

void gs_alarm_led_flash(LED_PARA *ledpara)
{
	if (gs_s_msleep_is_timesup(ledpara))
	{
        if (syspara.alarm_ctrl)
        {
            ledpara->maxcnt = syspara.alarm_ctrl_cycle;
            gs_alarm_beep_reverse();
        }
        else
        {
            gs_alarm_beep_off();
        }
		gs_s_msleep_start_and_setting(ledpara, ledpara->maxcnt);
	}
}

void gs_e32_m0m1_port_init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;

	//! ʹ��GPIO��ʱ��.
	RCC_APB2PeriphClockCmd(E32_M0_RCC_PORT, ENABLE);
    RCC_APB2PeriphClockCmd(E32_M1_RCC_PORT, ENABLE);

	GPIO_InitStruct.GPIO_Pin   = E32_M0_GPIO_PIN;
	GPIO_InitStruct.GPIO_Mode  = GPIO_Mode_Out_PP;	//! �������ģʽ.
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;	//! ������ָʾϵͳ����״̬, �ٶȲ���̫��.
	GPIO_Init(E32_M0_GPIO_PORT, &GPIO_InitStruct);
    
    GPIO_InitStruct.GPIO_Pin   = E32_M1_GPIO_PIN;
	GPIO_InitStruct.GPIO_Mode  = GPIO_Mode_Out_PP;	//! �������ģʽ.
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;	//! ������ָʾϵͳ����״̬, �ٶȲ���̫��.
	GPIO_Init(E32_M1_GPIO_PORT, &GPIO_InitStruct);

	GPIO_ResetBits(E32_M0_GPIO_PORT, E32_M0_GPIO_PIN);//! Ĭ�ϲ�����.
    GPIO_ResetBits(E32_M1_GPIO_PORT, E32_M1_GPIO_PIN);//! Ĭ�ϲ�����.
}

void gs_e32_set_mode(E32_MODE e32mode)
{
	if (e32mode == E32_NORMAL_MODE)
	{
		GPIO_ResetBits(E32_M0_GPIO_PORT, E32_M0_GPIO_PIN);
		GPIO_ResetBits(E32_M1_GPIO_PORT, E32_M1_GPIO_PIN);
	}
	else if (e32mode == E32_WAKEUP_MODE)
	{
		GPIO_SetBits(E32_M0_GPIO_PORT, E32_M0_GPIO_PIN);
		GPIO_ResetBits(E32_M1_GPIO_PORT, E32_M1_GPIO_PIN);
	}
	else if (e32mode == E32_POWERDOWN_MODE)
	{
		GPIO_ResetBits(E32_M0_GPIO_PORT, E32_M0_GPIO_PIN);
		GPIO_SetBits(E32_M1_GPIO_PORT, E32_M1_GPIO_PIN);
	}
	else if (e32mode == E32_SLEEP_MODE)
	{
		GPIO_SetBits(E32_M0_GPIO_PORT, E32_M0_GPIO_PIN);
		GPIO_SetBits(E32_M1_GPIO_PORT, E32_M1_GPIO_PIN);
	}
}

void gs_uart_init(UART_NUM uart_num, uint32_t baudrate, uint8_t parity)
{
	GPIO_InitTypeDef  GPIO_InitStruct;
	NVIC_InitTypeDef  NVIC_InitStruct;
	USART_InitTypeDef USART_InitStruct;
	DMA_InitTypeDef   DMA_InitStruct;

	if (uart_num == DEBUG_UART)
	{
		NVIC_InitStruct.NVIC_IRQChannel = DEBUG_UART_IRQ_CHN;
		NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0x00;
		NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0x01;
		NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
		NVIC_Init(&NVIC_InitStruct);

		//! ʹ��GPIO��ʱ��.
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
		RCC_APB1PeriphClockCmd(DEBUG_UART_RCC_PORT, ENABLE);
		RCC_AHBPeriphClockCmd(DEBUG_UART_DMA_RCC_PORT, ENABLE);

		RCC_APB2PeriphClockCmd(DEBUG_UART_TX_RCC_PORT, ENABLE);
		GPIO_InitStruct.GPIO_Pin   = DEBUG_UART_TX_GPIO_PIN;
		GPIO_InitStruct.GPIO_Mode  = GPIO_Mode_AF_PP;
		GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(DEBUG_UART_TX_GPIO_PORT, &GPIO_InitStruct);

		RCC_APB2PeriphClockCmd(DEBUG_UART_RX_RCC_PORT, ENABLE);
		GPIO_InitStruct.GPIO_Pin   = DEBUG_UART_RX_GPIO_PIN;
		GPIO_InitStruct.GPIO_Mode  = GPIO_Mode_IN_FLOATING;
		GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(DEBUG_UART_RX_GPIO_PORT, &GPIO_InitStruct);

		USART_DeInit(DEBUG_UART_PORT);

		USART_InitStruct.USART_BaudRate = baudrate;
		USART_InitStruct.USART_WordLength = USART_WordLength_8b;

		if (parity == NONE_PARITY)
		{
			USART_InitStruct.USART_Parity = USART_Parity_No;
		}
		else if (parity == ODD_PARITY)
		{
			USART_InitStruct.USART_Parity = USART_Parity_Odd;
		}
		else if (parity == EVEN_PARITY)
		{
			USART_InitStruct.USART_Parity = USART_Parity_Even;
		}

		USART_InitStruct.USART_StopBits = USART_StopBits_1;

		USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
		USART_InitStruct.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
		USART_Init(DEBUG_UART_PORT, &USART_InitStruct);

		USART_ITConfig(DEBUG_UART_PORT, USART_IT_IDLE, ENABLE);
//		USART_DMACmd(DEBUG_UART_PORT, USART_DMAReq_Tx, ENABLE);
		USART_DMACmd(DEBUG_UART_PORT, USART_DMAReq_Rx, ENABLE);

		USART_Cmd(DEBUG_UART_PORT, ENABLE);

        #if 0
		//! USART_TX_DMA
		NVIC_InitStruct.NVIC_IRQChannel = DEBUG_UART_TX_DMA_IRQ_CHN;
		NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0x00;
		NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0x00;
		NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
		NVIC_Init(&NVIC_InitStruct);
        #endif

		#if 0
		//! USART_RX_DMA
		NVIC_InitStruct.NVIC_IRQChannel = DEBUG_UART_RX_DMA_IRQ_CHN;
		NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0x00;
		NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0x00;
		NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
		NVIC_Init(&NVIC_InitStruct);
		#endif

		#if 0
		//! USART_TX_DMA
		DMA_DeInit(DEBUG_UART_TX_DMA_CHANNEL);
		DMA_InitStruct.DMA_PeripheralBaseAddr = (uint32_t)&(DEBUG_UART_PORT->DR);
		DMA_InitStruct.DMA_MemoryBaseAddr     = (uint32_t)&debuguart.txdata[0];
		DMA_InitStruct.DMA_DIR                = DMA_DIR_PeripheralDST;
		DMA_InitStruct.DMA_BufferSize         = UART_TX_BUFF_SIZE;
		DMA_InitStruct.DMA_PeripheralInc      = DMA_PeripheralInc_Disable;
		DMA_InitStruct.DMA_MemoryInc          = DMA_MemoryInc_Enable;
		DMA_InitStruct.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
		DMA_InitStruct.DMA_MemoryDataSize     = DMA_MemoryDataSize_Byte;
		DMA_InitStruct.DMA_Mode               = DMA_Mode_Normal;
		DMA_InitStruct.DMA_Priority           = DMA_Priority_VeryHigh;
		DMA_InitStruct.DMA_M2M                = DMA_M2M_Disable;
		DMA_Init(DEBUG_UART_TX_DMA_CHANNEL, &DMA_InitStruct);
		DMA_ITConfig(DEBUG_UART_TX_DMA_CHANNEL, DMA_IT_TC, ENABLE);
		#endif

		//! USART_RX_DMA
		DMA_DeInit(DEBUG_UART_RX_DMA_CHANNEL);
		DMA_InitStruct.DMA_PeripheralBaseAddr = (uint32_t)&(DEBUG_UART_PORT->DR);
		DMA_InitStruct.DMA_MemoryBaseAddr     = (uint32_t)&debuguart.rxdata[0];
		DMA_InitStruct.DMA_DIR                = DMA_DIR_PeripheralSRC;
		DMA_InitStruct.DMA_BufferSize         = UART_RX_BUFF_SIZE;
		DMA_InitStruct.DMA_PeripheralInc      = DMA_PeripheralInc_Disable;
		DMA_InitStruct.DMA_MemoryInc          = DMA_MemoryInc_Enable;
		DMA_InitStruct.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
		DMA_InitStruct.DMA_MemoryDataSize     = DMA_MemoryDataSize_Byte;
		DMA_InitStruct.DMA_Mode               = DMA_Mode_Normal;
		DMA_InitStruct.DMA_Priority           = DMA_Priority_VeryHigh;
		DMA_InitStruct.DMA_M2M                = DMA_M2M_Disable;
		DMA_Init(DEBUG_UART_RX_DMA_CHANNEL, &DMA_InitStruct);
		DMA_Cmd(DEBUG_UART_RX_DMA_CHANNEL, ENABLE);
	}
	else if (uart_num == LORA_UART)
	{
		NVIC_InitStruct.NVIC_IRQChannel = LORA_UART_IRQ_CHN;
		NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0x00;
		NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0x01;
		NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
		NVIC_Init(&NVIC_InitStruct);

		//! ʹ��GPIO��ʱ��.
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
		RCC_APB2PeriphClockCmd(LORA_UART_RCC_PORT, ENABLE);
		RCC_AHBPeriphClockCmd(LORA_UART_DMA_RCC_PORT, ENABLE);
		
		GPIO_PinRemapConfig(GPIO_Remap_USART1, ENABLE);

		RCC_APB2PeriphClockCmd(LORA_UART_TX_RCC_PORT, ENABLE);
		GPIO_InitStruct.GPIO_Pin   = LORA_UART_TX_GPIO_PIN;
		GPIO_InitStruct.GPIO_Mode  = GPIO_Mode_AF_PP;
		GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(LORA_UART_TX_GPIO_PORT, &GPIO_InitStruct);

		RCC_APB2PeriphClockCmd(LORA_UART_RX_RCC_PORT, ENABLE);
		GPIO_InitStruct.GPIO_Pin   = LORA_UART_RX_GPIO_PIN;
		GPIO_InitStruct.GPIO_Mode  = GPIO_Mode_IN_FLOATING;
		GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(LORA_UART_RX_GPIO_PORT, &GPIO_InitStruct);

		USART_DeInit(LORA_UART_PORT);

		USART_InitStruct.USART_BaudRate = baudrate;

		USART_InitStruct.USART_WordLength = USART_WordLength_8b;

		if (parity == NONE_PARITY)
		{
			USART_InitStruct.USART_Parity = USART_Parity_No;
		}
		else if (parity == ODD_PARITY)
		{
			USART_InitStruct.USART_Parity = USART_Parity_Odd;
		}
		else if (parity == EVEN_PARITY)
		{
			USART_InitStruct.USART_Parity = USART_Parity_Even;
		}

		USART_InitStruct.USART_StopBits = USART_StopBits_1;

		USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
		USART_InitStruct.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
		USART_Init(LORA_UART_PORT, &USART_InitStruct);

		USART_ITConfig(LORA_UART_PORT, USART_IT_IDLE, ENABLE);
//		USART_DMACmd(LORA_UART_PORT, USART_DMAReq_Tx, ENABLE);
		USART_DMACmd(LORA_UART_PORT, USART_DMAReq_Rx, ENABLE);

		USART_Cmd(LORA_UART_PORT, ENABLE);

        #if 0
		//! USART_TX_DMA
		NVIC_InitStruct.NVIC_IRQChannel = LORA_UART_TX_DMA_IRQ_CHN;
		NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0x00;
		NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0x00;
		NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
		NVIC_Init(&NVIC_InitStruct);
        #endif

		#if 0
		// USART_RX_DMA
		NVIC_InitStruct.NVIC_IRQChannel = LORA_UART_RX_DMA_IRQ_CHN;
		NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0x00;
		NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0x00;
		NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
		NVIC_Init(&NVIC_InitStruct);
		#endif

		#if 0
		//! USART_TX_DMA
		DMA_DeInit(LORA_UART_TX_DMA_CHANNEL);
		DMA_InitStruct.DMA_PeripheralBaseAddr = (uint32_t)&(LORA_UART_PORT->DR);
		DMA_InitStruct.DMA_MemoryBaseAddr     = (uint32_t)&dsffoduart.txdata[0];
		DMA_InitStruct.DMA_DIR                = DMA_DIR_PeripheralDST;
		DMA_InitStruct.DMA_BufferSize         = UART_TX_BUFF_SIZE;
		DMA_InitStruct.DMA_PeripheralInc      = DMA_PeripheralInc_Disable;
		DMA_InitStruct.DMA_MemoryInc          = DMA_MemoryInc_Enable;
		DMA_InitStruct.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
		DMA_InitStruct.DMA_MemoryDataSize     = DMA_MemoryDataSize_Byte;
		DMA_InitStruct.DMA_Mode               = DMA_Mode_Normal;
		DMA_InitStruct.DMA_Priority           = DMA_Priority_VeryHigh;
		DMA_InitStruct.DMA_M2M                = DMA_M2M_Disable;
		DMA_Init(LORA_UART_TX_DMA_CHANNEL, &DMA_InitStruct);
		DMA_ITConfig(LORA_UART_TX_DMA_CHANNEL, DMA_IT_TC, ENABLE);
		#endif

		//! USART_RX_DMA
		DMA_DeInit(LORA_UART_RX_DMA_CHANNEL);
		DMA_InitStruct.DMA_PeripheralBaseAddr = (uint32_t)&(LORA_UART_PORT->DR);
		DMA_InitStruct.DMA_MemoryBaseAddr     = (uint32_t)&lorauart.rxdata[0];
		DMA_InitStruct.DMA_DIR                = DMA_DIR_PeripheralSRC;
		DMA_InitStruct.DMA_BufferSize         = UART_RX_BUFF_SIZE;
		DMA_InitStruct.DMA_PeripheralInc      = DMA_PeripheralInc_Disable;
		DMA_InitStruct.DMA_MemoryInc          = DMA_MemoryInc_Enable;
		DMA_InitStruct.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
		DMA_InitStruct.DMA_MemoryDataSize     = DMA_MemoryDataSize_Byte;
		DMA_InitStruct.DMA_Mode               = DMA_Mode_Normal;
		DMA_InitStruct.DMA_Priority           = DMA_Priority_VeryHigh;
		DMA_InitStruct.DMA_M2M                = DMA_M2M_Disable;
		DMA_Init(LORA_UART_RX_DMA_CHANNEL, &DMA_InitStruct);
		DMA_Cmd(LORA_UART_RX_DMA_CHANNEL, ENABLE);
	}
}

void gs_uart_close(UART_NUM uart_num)
{
	if (uart_num == DEBUG_UART)
	{
		RCC_APB1PeriphClockCmd(DEBUG_UART_RCC_PORT, DISABLE);
	}
	else if (uart_num == LORA_UART)
	{
	}
}

void gs_uart_send(UART_NUM uart_num, uint8_t *buff, uint16_t len)
{
	uint16_t i;

	if (uart_num == DEBUG_UART)
	{
		memcpy(debuguart.txdata, buff, len);

		#if 0
		//! �ر�DMA����.
		DMA_Cmd(DMA1_Channel7, DISABLE);
		//! ���ô������ݳ���.
		DMA_SetCurrDataCounter(DMA1_Channel7, len);
		//! ��DMA, ��ʼ����.
		DMA_Cmd(DMA1_Channel7, ENABLE);
		#else
		for (i = 0; i < len; i++)
		{
			while (USART_GetFlagStatus(DEBUG_UART_PORT, USART_FLAG_TC) != SET);
			USART_SendData(DEBUG_UART_PORT, (unsigned char)buff[i]);
		}
		while (USART_GetFlagStatus(DEBUG_UART_PORT, USART_FLAG_TC) != SET);
		#endif
	}
	else if (uart_num == LORA_UART)
	{
		memcpy(lorauart.txdata, buff, len);

		#if 0
		//! �ر�DMA����.
		DMA_Cmd(DMA1_Channel2, DISABLE);
		//! ���ô������ݳ���.
		DMA_SetCurrDataCounter(DMA1_Channel2, len);
		//! ��DMA, ��ʼ����.
		DMA_Cmd(DMA1_Channel2, ENABLE);
		#else
		for (i = 0; i < len; i++)
		{
			while (USART_GetFlagStatus(LORA_UART_PORT, USART_FLAG_TC) != SET);
			USART_SendData(LORA_UART_PORT, (unsigned char)buff[i]);
		}
		while (USART_GetFlagStatus(LORA_UART_PORT, USART_FLAG_TC) != SET);
		#endif
	}
}

int gs_uart_recv(UART_NUM uart_num, uint8_t *buff, uint16_t *len)
{
	if (uart_num == DEBUG_UART)
	{
		if (debuguart.flag_rx == 1)
		{
			memcpy(buff, debuguart.rxdata, debuguart.rxlen);
			*len = debuguart.rxlen;
			debuguart.flag_rx = 0;
			return (0);
		}
	}
	else if (uart_num == LORA_UART)
	{
		if (lorauart.flag_rx == 1)
		{
			memcpy(buff, lorauart.rxdata, lorauart.rxlen);
			*len = lorauart.rxlen;
			lorauart.flag_rx = 0;
			return (0);
		}
	}
	return (-1);
}

void gs_uart_test(UART_NUM uart_num)
{
	uint8_t buff[UART_RX_BUFF_SIZE];
	uint16_t len;

	if (0 == gs_uart_recv(uart_num, buff, &len))
	{
		gs_uart_send(uart_num, buff, len);
	}
}

#if 0
/*
 * Function Name: send_char
 * Function Prototype: int send_char(int ch);
 * Function Description: �����ض�����. 
 * Entrance Parameters: (1) ch, Ҫ���͵�����.
 * Export Parameters: None.
 * Return Value: None.
 * Version: V00.01
 * Modification Time: 2019.12.17 20:13
 * Author: WYP
 * Status: �Ѳ���
 */
int send_char(int ch)
{
	USART_SendData(DEBUG_UART_PORT, (unsigned char)ch);
	while (USART_GetFlagStatus(DEBUG_UART_PORT, USART_FLAG_TC) != SET);
	return (ch);
}

/*
 * Function Name: fputc
 * Function Prototype: int fputc(int ch, FILE *f);
 * Function Description: fputc�����ض���, �ض��򴮿�Ϊ����1. 
 * ��Ҫ����<stdio.h>ͷ�ļ�.
 * �ú���������ͷ�ļ�������.
 * ��keil������, ���"Target Options...",
 * ��"Target"��, ѡ��"Use MicroLIB", �����,
 * ����ʹ��printf����, ����Ϣ������������ض���Ĵ���.
 * Entrance Parameters: (1) ch, Ҫ���͵�����;
 * 						(2) *f, �ض����ļ�ָ��.
 * Export Parameters: None.
 * Return Value: None.
 * Version: V00.01
 * Modification Time: 2019.12.17 20:43
 * Author: WYP
 * Status: �Ѳ���
 */
int fputc(int ch, FILE *f)
{
	return (send_char(ch));
}
#else
void gs_printf(char *fmt, ...)
{
    #if 0
    #else
	char buff[512];
	uint16_t i = 0;
	va_list arg_ptr;
	va_start(arg_ptr, fmt);
	vsnprintf(buff, sizeof(buff), fmt, arg_ptr);

	while ((i < (sizeof(buff) - 1) && buff[i]))
	{
		while (USART_GetFlagStatus(DEBUG_UART_PORT, USART_FLAG_TC) != SET);
		USART_SendData(DEBUG_UART_PORT, (unsigned char) buff[i]);
		i++;
	}
	va_end(arg_ptr);
    #endif
}
#endif

void gs_adc_init(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	NVIC_InitTypeDef NVIC_InitStruct;
	ADC_InitTypeDef  ADC_InitStruct;
	DMA_InitTypeDef  DMA_InitStruct;

	RCC_APB2PeriphClockCmd(B_VOLT_ADC_RCC_PORT, ENABLE);
    RCC_ADCCLKConfig(RCC_PCLK2_Div8);

	//! ����DMA1_Channel1���ж�.
	NVIC_InitStruct.NVIC_IRQChannel = B_VOLT_ADC_DMA_IRQ_CHN;
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0x00;
	NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0x00;
	NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStruct);

	RCC_APB2PeriphClockCmd(B_VOLT_RCC_PORT, ENABLE);
	GPIO_InitStruct.GPIO_Pin   = B_VOLT_GPIO_PIN;
	GPIO_InitStruct.GPIO_Mode  = GPIO_Mode_AIN;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(B_VOLT_GPIO_PORT, &GPIO_InitStruct);
	#if 1
	RCC_APB2PeriphClockCmd(O2_RCC_PORT, ENABLE);
	GPIO_InitStruct.GPIO_Pin   = O2_GPIO_PIN;
	GPIO_InitStruct.GPIO_Mode  = GPIO_Mode_AIN;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(O2_GPIO_PORT, &GPIO_InitStruct);
	
	RCC_APB2PeriphClockCmd(CO_RCC_PORT, ENABLE);
	GPIO_InitStruct.GPIO_Pin   = CO_GPIO_PIN;
	GPIO_InitStruct.GPIO_Mode  = GPIO_Mode_AIN;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(CO_GPIO_PORT, &GPIO_InitStruct);
	
	RCC_APB2PeriphClockCmd(H2S_RCC_PORT, ENABLE);
	GPIO_InitStruct.GPIO_Pin   = H2S_GPIO_PIN;
	GPIO_InitStruct.GPIO_Mode  = GPIO_Mode_AIN;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(H2S_GPIO_PORT, &GPIO_InitStruct);
	
	RCC_APB2PeriphClockCmd(LEL_RCC_PORT, ENABLE);
	GPIO_InitStruct.GPIO_Pin   = LEL_GPIO_PIN;
	GPIO_InitStruct.GPIO_Mode  = GPIO_Mode_AIN;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(LEL_GPIO_PORT, &GPIO_InitStruct);
#endif
	//! ʹ��DMA1��ʱ��.
	RCC_AHBPeriphClockCmd(B_VOLT_ADC_DMA_RCC_PORT, ENABLE);

	//! ����DMA_Channel1.
	DMA_DeInit(B_VOLT_ADC_DMA_CHANNEL);

	DMA_InitStruct.DMA_PeripheralBaseAddr = (uint32_t)ADC1_DR_ADDR;		//! �������ַΪADC1�����ݼĴ�����ַ.
	DMA_InitStruct.DMA_MemoryBaseAddr     = (uint32_t)&adcpara.data[0];	//! �ڴ����ַ.
	DMA_InitStruct.DMA_DIR                = DMA_DIR_PeripheralSRC;		//! ������Ϊ���ݴ������Դ.
	DMA_InitStruct.DMA_BufferSize         = ADC_CHANNEL_NUM;			//! ����buffer��С.
	DMA_InitStruct.DMA_PeripheralInc      = DMA_PeripheralInc_Disable;	//! �����ַ�Ĵ�������.
	DMA_InitStruct.DMA_MemoryInc          = DMA_MemoryInc_Enable;		//! �ڴ��ַ�Ĵ�������.
	DMA_InitStruct.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;//! �������ݿ��Ϊ16λ.
	DMA_InitStruct.DMA_MemoryDataSize     = DMA_MemoryDataSize_HalfWord;	//! �ڴ����ݿ��Ϊ16λ.
	DMA_InitStruct.DMA_Mode               = DMA_Mode_Normal;			//! ����������ģʽ.
	DMA_InitStruct.DMA_Priority           = DMA_Priority_High;			//! DMAͨ��ӵ�зǳ������ȼ�.
	DMA_InitStruct.DMA_M2M                = DMA_M2M_Disable;			//! DMAͨ��û������Ϊ�ڴ浽�ڴ洫��.
	DMA_Init(B_VOLT_ADC_DMA_CHANNEL, &DMA_InitStruct);

	DMA_ClearITPendingBit(B_VOLT_ADC_DMA_IT_TC);
	//! DMA1_Channel1ʹ�ܴ�������ж�.
	DMA_ITConfig(B_VOLT_ADC_DMA_CHANNEL, DMA_IT_TC, ENABLE);

	//! ʹ��DMA1_Channel1.
	DMA_Cmd(B_VOLT_ADC_DMA_CHANNEL, ENABLE);

	//! ADC1����.
	ADC_InitStruct.ADC_ScanConvMode       = ENABLE;		//! ʹ�ܶ�ͨ����ɨ��ģʽ.
	ADC_InitStruct.ADC_ContinuousConvMode = DISABLE;	//! ��ֹ��������ģʽ.
	ADC_InitStruct.ADC_DataAlign          = ADC_DataAlign_Right;		//! ADC�����Ҷ���.
	ADC_InitStruct.ADC_ExternalTrigConv   = ADC_ExternalTrigConv_None;	//! ת��������������ⲿ��������.
	ADC_InitStruct.ADC_Mode               = ADC_Mode_Independent;		//! ADC1��ADC2�����ڶ���ģʽ.
	ADC_InitStruct.ADC_NbrOfChannel       = ADC_CHANNEL_NUM;			//! ˳����й���ת����ADCͨ������Ŀ.
	ADC_Init(B_VOLT_ADC_PORT, &ADC_InitStruct);

	//! ADC1��ͨ���Ĳ���˳������.
	//! ADC1 regular channel0 configuration.
	ADC_RegularChannelConfig(B_VOLT_ADC_PORT, ADC_Channel_16, 1, ADC_SampleTime_239Cycles5);

	//! ADC1 regular channel16 configuration.
	ADC_RegularChannelConfig(B_VOLT_ADC_PORT, ADC_Channel_1, 2, ADC_SampleTime_239Cycles5);
	#if 1
	//! ADC1 regular channel16 configuration.
	ADC_RegularChannelConfig(B_VOLT_ADC_PORT, ADC_Channel_4, 3, ADC_SampleTime_239Cycles5);
	
	//! ADC1 regular channel16 configuration.
	ADC_RegularChannelConfig(B_VOLT_ADC_PORT, ADC_Channel_5, 4, ADC_SampleTime_239Cycles5);
	
	//! ADC1 regular channel16 configuration.
	ADC_RegularChannelConfig(B_VOLT_ADC_PORT, ADC_Channel_6, 5, ADC_SampleTime_239Cycles5);
	
	//! ADC1 regular channel16 configuration.
	ADC_RegularChannelConfig(B_VOLT_ADC_PORT, ADC_Channel_7, 6, ADC_SampleTime_239Cycles5);
#endif
	ADC_TempSensorVrefintCmd(ENABLE);

	//! ʹ��ADC1��DMA.
	ADC_DMACmd(B_VOLT_ADC_PORT, ENABLE);

	//! ʹ��ADC1.
	ADC_Cmd(B_VOLT_ADC_PORT, ENABLE);

	//! Enable ADC1 reset calibration register.
	ADC_ResetCalibration(B_VOLT_ADC_PORT);

	//! Check the end of ADC1 reset calibration register.
	while(ADC_GetResetCalibrationStatus(B_VOLT_ADC_PORT));

	//! Start ADC1 calibration.
	ADC_StartCalibration(B_VOLT_ADC_PORT);

	//! Check the end of ADC1 calibration.
	while(ADC_GetCalibrationStatus(B_VOLT_ADC_PORT));

	//! Start ADC1 Software Conversion. 
	ADC_SoftwareStartConvCmd(B_VOLT_ADC_PORT, ENABLE);
}

void gs_adc_dma_restart(void)
{
	DMA_InitTypeDef DMA_InitStruct;

	//! ��ֹDMA1_Channel1.
	DMA_Cmd(B_VOLT_ADC_DMA_CHANNEL, DISABLE);

	DMA_InitStruct.DMA_PeripheralBaseAddr = (uint32_t)ADC1_DR_ADDR;		//! �������ַΪADC1�����ݼĴ�����ַ.
	DMA_InitStruct.DMA_MemoryBaseAddr     = (uint32_t)&adcpara.data[0];	//! �ڴ����ַ.
	DMA_InitStruct.DMA_DIR                = DMA_DIR_PeripheralSRC;		//! ������Ϊ���ݴ������Դ.
	DMA_InitStruct.DMA_BufferSize         = ADC_CHANNEL_NUM;			//! ����buffer��С.
	DMA_InitStruct.DMA_PeripheralInc      = DMA_PeripheralInc_Disable;	//! �����ַ�Ĵ�������.
	DMA_InitStruct.DMA_MemoryInc          = DMA_MemoryInc_Enable;		//! �ڴ��ַ�Ĵ�������.
	DMA_InitStruct.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;	//! �������ݿ��Ϊ16λ.
	DMA_InitStruct.DMA_MemoryDataSize     = DMA_MemoryDataSize_HalfWord;		//! �ڴ����ݿ��Ϊ16λ.
	DMA_InitStruct.DMA_Mode               = DMA_Mode_Normal;			//! ������ѭ������ģʽ.
	DMA_InitStruct.DMA_Priority           = DMA_Priority_High;			//! DMAͨ��ӵ�зǳ������ȼ�.
	DMA_InitStruct.DMA_M2M                = DMA_M2M_Disable;			//! DMAͨ��û������Ϊ�ڴ浽�ڴ洫��.
	DMA_Init(B_VOLT_ADC_DMA_CHANNEL, &DMA_InitStruct);

	//! DMA1_Channel1ʹ�ܴ�������ж�.
	DMA_ITConfig(B_VOLT_ADC_DMA_CHANNEL, DMA_IT_TC, ENABLE);

	//! ʹ��DMA1_Channel1.
	DMA_Cmd(B_VOLT_ADC_DMA_CHANNEL, ENABLE);

	//! Start ADC1 Software Conversion. 
	ADC_SoftwareStartConvCmd(B_VOLT_ADC_PORT, ENABLE);
}

void gs_calc_adc_data(void)
{
	int i;

	//! A/D�������, �����˲�����.
	if (adcpara.flag_adc_sample == 0x01)
	{
		//! ��ÿһͨ�����л���ƽ���˲�����.
		for (i = 0; i < ADC_CHANNEL_NUM; i++)
		{
			adcpara.filtered_data[i] = gs_moving_average_filter_uint16_t(adcpara.data[i], &adcpara.buff[i][0], ADC_SAMPLE_NUM);
		}
		syspara.mcu_temp     = (float)adcpara.filtered_data[0] * 3.3f / 4095.0f;
		syspara.mcu_temp     = (1.43f - syspara.mcu_temp) / 0.0043f + 25.0f;
		syspara.battery_volt = (float)adcpara.filtered_data[1] * 3.3f / 4095.0f * 20.0f / 10.0f;
		
		jxmdata[JXM_SWITCH_O2].a = 0
		jxmdata[JXM_SWITCH_O2].val  = jxmdata[JXM_SWITCH_O2].a * (float)adcpara.data[2] + jxmdata[JXM_SWITCH_O2].b;
		jxmdata[JXM_SWITCH_CO].val  = jxmdata[JXM_SWITCH_CO].a * (float)adcpara.data[3] + jxmdata[JXM_SWITCH_CO].b;
		jxmdata[JXM_SWITCH_H2S].val = jxmdata[JXM_SWITCH_H2S].a * (float)adcpara.data[4] + jxmdata[JXM_SWITCH_H2S].b;
		jxmdata[JXM_SWITCH_LEL].val = jxmdata[JXM_SWITCH_LEL].a * (float)adcpara.data[5] + jxmdata[JXM_SWITCH_LEL].b;
		
		for(i = 0; i < 100000; i++);
		gs_printf("volt: %f V, temp: %f ��, ", syspara.battery_volt, syspara.mcu_temp);
		gs_printf("O2: %d, CO: %d, H2S: %d, LEL: %d.\r\n", adcpara.data[2], adcpara.data[3], adcpara.data[4], adcpara.data[5]);
		gs_adc_dma_restart();
		adcpara.flag_adc_sample = 0x00;
	}
}

void gs_software_reboot(void)
{
	__disable_fault_irq();
	NVIC_SystemReset();
}

void WFI_SET(void)
{
	__ASM volatile("wfi");
}

//�ر������ж�
void INTX_DISABLE(void)
{
	__ASM volatile("cpsid i");
}

//���������ж�
void INTX_ENABLE(void)
{
	__ASM volatile("cpsie i");
}

//����ջ����ַ
//addr:ջ����ַ
__asm void MSR_MSP(u32 addr)
{
	MSR MSP, r0//set Main Stack value
	BX r14
}

/**
  * @brief  1/4us��ʱ�Ӻ���, ��ʱʱ���ʵ�ʴ�СδУ��.
  * @param  None.
  * @retval None
  */
void qu_delay(void)
{
	#if (IDE_TYPE == IAR_IDE)
	//! IAR�����µĿղ���.
	__NOP();
	__NOP();
	__NOP();
	__NOP();

	__NOP();
	__NOP();
	__NOP();
	__NOP();
	#elif (IDE_TYPE == KEIL_MDK_IDE)
	//! Keil MDK�����µĿղ���.
	__nop();
	__nop();
	__nop();
	__nop();
	
	__nop();
	__nop();
	__nop();
	__nop();
	#endif
}

/**
  * @brief  us��ʱ�Ӻ���, ��ʱʱ���ʵ�ʴ�СδУ��.
  * @param  uint16_t us: ��ʱʱ��ֵ.
  * @retval None
  */
void gs_usleep(uint16_t us)
{
	while(us--)
	{
		qu_delay();
		qu_delay();
		qu_delay();
		qu_delay();
	}
}

/**
  * @brief  ms��ʱ�Ӻ���, ��ʱʱ���ʵ�ʴ�СδУ��.
  * @param  uint16_t ms: ��ʱʱ��ֵ.
  * @retval None
  */
void gs_msleep(uint16_t ms)
{
	while(ms--)
	{
		gs_usleep(1000);
	}
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{
    /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

    /* Infinite loop */
    while (1)
    {
    }
}
#endif

