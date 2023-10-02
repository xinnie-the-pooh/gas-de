/**
  ******************************************************************************
  * @file    Project/STM32F10x_StdPeriph_Template/stm32f10x_it.c 
  * @author  MCD Application Team
  * @version V3.5.0
  * @date    08-April-2011
  * @brief   Main Interrupt Service Routines.
  *          This file provides template for all exceptions handler and 
  *          peripherals interrupt service routine.
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2011 STMicroelectronics</center></h2>
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x_it.h"
#include "gs_global_para.h"
#include "gs_bsp.h"

/** @addtogroup STM32F10x_StdPeriph_Template
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/******************************************************************************/
/*            Cortex-M3 Processor Exceptions Handlers                         */
/******************************************************************************/

/**
  * @brief  This function handles NMI exception.
  * @param  None
  * @retval None
  */
void NMI_Handler(void)
{
}

/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */
void HardFault_Handler(void)
{
	/* Go to infinite loop when Hard Fault exception occurs */
	while (1)
	{}
}

/**
  * @brief  This function handles Memory Manage exception.
  * @param  None
  * @retval None
  */
void MemManage_Handler(void)
{
	/* Go to infinite loop when Memory Manage exception occurs */
	while (1)
	{}
}

/**
  * @brief  This function handles Bus Fault exception.
  * @param  None
  * @retval None
  */
void BusFault_Handler(void)
{
	/* Go to infinite loop when Bus Fault exception occurs */
	while (1)
	{}
}

/**
  * @brief  This function handles Usage Fault exception.
  * @param  None
  * @retval None
  */
void UsageFault_Handler(void)
{
	/* Go to infinite loop when Usage Fault exception occurs */
	while (1)
	{}
}

/**
  * @brief  This function handles SVCall exception.
  * @param  None
  * @retval None
  */
void SVC_Handler(void)
{
}

/**
  * @brief  This function handles Debug Monitor exception.
  * @param  None
  * @retval None
  */
void DebugMon_Handler(void)
{
}

/**
  * @brief  This function handles PendSVC exception.
  * @param  None
  * @retval None
  */
void PendSV_Handler(void)
{
}

/**
  * @brief  This function handles SysTick Handler.
  * @param  None
  * @retval None
  */
void SysTick_Handler(void)
{
	//! System Heartbeat.
	//! 1000ms*30days=1000*86400*30=2592000000u.
	if ((++syspara.heartbeat) >= 2592000000u)
	{
		syspara.heartbeat = 0x00;
	}
}

/******************************************************************************/
/*                 STM32F10x Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f10x_xx.s).                                            */
/******************************************************************************/

/**
  * @brief  This function handles CAN1_RX0_IRQHandler interrupt 
  *         request.
  * @param  None
  * @retval None
  */
//! JXM TTL USART_IDLE
void USART1_IRQHandler(void)
{
	if (USART_GetITStatus(JXM_UART_PORT, USART_IT_IDLE) == SET)
	{
		//! 读取数据注意: 这句必须要, 否则不能够清除中断标志位.
		USART_ReceiveData(JXM_UART_PORT);
		//! 清除中断标志.
		USART_ClearITPendingBit(JXM_UART_PORT, USART_IT_IDLE);
		DMA_Cmd(JXM_UART_RX_DMA_CHANNEL, DISABLE);
		DMA_ClearFlag(JXM_UART_RX_DMA_FLAG);
		//! 算出接本帧数据长度.
		jxmuart.rxlen = UART_RX_BUFF_SIZE - DMA_GetCurrDataCounter(JXM_UART_RX_DMA_CHANNEL);

		if (jxmuart.rxlen > 0)
		{
			jxmuart.flag_rx = 0x01;
		}

		DMA_SetCurrDataCounter(JXM_UART_RX_DMA_CHANNEL, UART_RX_BUFF_SIZE);
		DMA_Cmd(JXM_UART_RX_DMA_CHANNEL, ENABLE);
	}
}

//! JXM TTL USART_TX_DMA
void DMA1_Channel4_IRQHandler(void)
{
	//! 等待DMA1_Stream7传输完成.
	if (DMA_GetITStatus(JXM_UART_TX_DMA_IT_TC) != RESET)
	{
		//! 清除DMA1_Stream7传输完成标志.
		DMA_ClearITPendingBit(JXM_UART_TX_DMA_IT_TC);
		//! 关闭DMA.
		DMA_Cmd(JXM_UART_TX_DMA_CHANNEL, DISABLE);
	}
}

//! LORA TTL USART3_IDLE
void USART3_IRQHandler(void)
{
	if (USART_GetITStatus(LORA_UART_PORT, USART_IT_IDLE) == SET)
	{
		//! 读取数据注意: 这句必须要, 否则不能够清除中断标志位.
		USART_ReceiveData(LORA_UART_PORT);
		USART_ClearITPendingBit(LORA_UART_PORT, USART_IT_IDLE);//! 清除中断标志.
		DMA_Cmd(LORA_UART_RX_DMA_CHANNEL, DISABLE);
		DMA_ClearFlag(LORA_UART_RX_DMA_FLAG);
		//! 算出接本帧数据长度.
		lorauart.rxlen = UART_RX_BUFF_SIZE - DMA_GetCurrDataCounter(LORA_UART_RX_DMA_CHANNEL);

		if (lorauart.rxlen > 0)
		{
			lorauart.flag_rx = 0x01;
		}

		DMA_SetCurrDataCounter(LORA_UART_RX_DMA_CHANNEL, UART_RX_BUFF_SIZE);
		DMA_Cmd(LORA_UART_RX_DMA_CHANNEL, ENABLE);
	}
}

//! LORA TTL USART3_TX_DMA
void DMA1_Channel2_IRQHandler(void)
{
	//! 等待DMA1_Stream2传输完成.
	if (DMA_GetITStatus(LORA_UART_TX_DMA_IT_TC) != RESET)
	{
		//! 清除DMA1_Stream2传输完成标志.
		DMA_ClearITPendingBit(LORA_UART_TX_DMA_IT_TC);
		//! 关闭DMA.
		DMA_Cmd(LORA_UART_TX_DMA_CHANNEL, DISABLE);
	}
}

void DMA1_Channel1_IRQHandler(void)
{
	if (DMA_GetITStatus(B_VOLT_ADC_DMA_IT_TC) == SET)
	{
		adcpara.flag_adc_sample = 0x01;		//! A/D采样完成.
		DMA_ClearITPendingBit(B_VOLT_ADC_DMA_IT_TC);
	}
}

/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
