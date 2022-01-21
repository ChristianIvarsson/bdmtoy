/**
  ******************************************************************************
  * @file    stm32_it.h
  * @author  MCD Application Team
  * @version V4.1.0
  * @date    26-May-2017
  * @brief   This file contains the headers of the interrupt handlers.
  ****************************************************************************** */

#ifndef __STM32_IT_H
#define __STM32_IT_H

/* Includes ------------------------------------------------------------------*/
#include "platform_config.h"

void NMI_Handler(void);
void HardFault_Handler(void);
void MemManage_Handler(void);
void BusFault_Handler(void);
void UsageFault_Handler(void);
void SVC_Handler(void);
void DebugMon_Handler(void);
void PendSV_Handler(void);
void SysTick_Handler(void);
void USB_LP_CAN1_RX0_IRQHandler(void);


#endif

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
