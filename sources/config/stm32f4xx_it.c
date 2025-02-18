/**
  ******************************************************************************
  * @file    stm32f4xx_it.c 
  * @author  MCD Application Team
  * @version V1.1.0
  * @date    31-July-2013
  * @brief   Main Interrupt Service Routines.
  *          This file provides template for all exceptions handler and 
  *          peripherals interrupt service routine.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2013 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_it.h"
#include "main.h"
//#include "stm32f4x7_eth.h"

/* Scheduler includes */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

/* lwip includes */
//#include "lwip/sys.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
//extern xSemaphoreHandle s_xSemaphore;
//extern xSemaphoreHandle ETH_link_xSemaphore;
/* Private function prototypes -----------------------------------------------*/
extern void xPortSysTickHandler(void); 
/* Private functions ---------------------------------------------------------*/

/******************************************************************************/
/*            Cortex-M4 Processor Exceptions Handlers                         */
/******************************************************************************/

extern void __reboot__();

/**
  * @brief   This function handles NMI exception.
  * @param  None
  * @retval None
  */
void NMI_Handler(void)
{
  __reboot__();
}

/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */
void HardFault_Handler(void)
{
  /* Go to infinite loop when Hard Fault exception occurs */
  __reboot__();
}

/**
  * @brief  This function handles Memory Manage exception.
  * @param  None
  * @retval None
  */
void MemManage_Handler(void)
{
  /* Go to infinite loop when Memory Manage exception occurs */
  __reboot__();
}

/**
  * @brief  This function handles Bus Fault exception.
  * @param  None
  * @retval None
  */
void BusFault_Handler(void)
{
  /* Go to infinite loop when Bus Fault exception occurs */
  __reboot__();
}

/**
  * @brief  This function handles Usage Fault exception.
  * @param  None
  * @retval None
  */
void UsageFault_Handler(void)
{
  /* Go to infinite loop when Usage Fault exception occurs */
  __reboot__();
}

/**
  * @brief  This function handles Debug Monitor exception.
  * @param  None
  * @retval None
  */
void DebugMon_Handler(void)
{
  __reboot__();
}

/**
  * @brief  This function handles SysTick Handler.
  * @param  None
  * @retval None
  */
volatile static uint32_t reset_time = 5*60*1000; // 5Минут
volatile static uint32_t reset_timer[2] = {0,0}; // 5Минут
void ResetRestartTimer(int timer)
{
  if(timer<0 || timer > 1) return;
  //reset_timer[timer] = 0;
  reset_timer[0] = 0;
  reset_timer[1] = 0;
}

#if (IIP != 0)
void RecalcFreqCounts();
#endif
void SysTick_Handler(void)
{
//  reset_timer[0]++;
//  reset_timer[1]++;
//  if((reset_timer[0]>reset_time) 
//     || 
//     (reset_timer[1]>reset_time))
//  {
//    while(1){
//      NVIC_SystemReset();
//    }
//  }
  xPortSysTickHandler();
#if (IIP != 0)
  uint32_t cTick = xTaskGetTickCount();
  if ((cTick % 1000) == 0) {
    RecalcFreqCounts();
  }
#endif
}

#if (IIP != 0)
//Прерывания EXTI для вычисления частоты / периода сигналов I1-I4, U1-U4
#include "power_src.h"

void RecalcFreqCounts() {
  memcpy((uint8_t *)gaFreqValue, (uint8_t *)gaFreqCount, sizeof(gaFreqCount));
  memset((uint8_t *)gaFreqCount, 0, sizeof(gaFreqCount));
}

/**
  * @brief  This function handles External line 2 interrupt request.
  * @param  None
  * @retval None
  */
void EXTI2_IRQHandler(void)
{
  /* Clear interrupt pending bit */
  EXTI_ClearITPendingBit(EXTI_Line2);
  ++gaFreqCount[0][0];
}

/**
  * @brief  This function handles External line 3 interrupt request.
  * @param  None
  * @retval None
  */
void EXTI3_IRQHandler(void)
{
  /* Clear interrupt pending bit */
  EXTI_ClearITPendingBit(EXTI_Line3);
  ++gaFreqCount[0][1];
}

/**
  * @brief  This function handles External line 4 interrupt request.
  * @param  None
  * @retval None
  */
void EXTI4_IRQHandler(void)
{
  /* Clear interrupt pending bit */
  EXTI_ClearITPendingBit(EXTI_Line4);
  ++gaFreqCount[0][2];
}

/**
  * @brief  This function handles External line 5,6,8,9 interrupt request.
  * @param  None
  * @retval None
  */
void EXTI9_5_IRQHandler(void)
{
  if (EXTI_GetITStatus(EXTI_Line5) == SET) {
    /* Clear interrupt pending bit */
    EXTI_ClearITPendingBit(EXTI_Line5);
    ++gaFreqCount[0][3];
  }
  if (EXTI_GetITStatus(EXTI_Line6) == SET) {
    /* Clear interrupt pending bit */
    EXTI_ClearITPendingBit(EXTI_Line6);
    ++gaFreqCount[1][0];
  }
  if (EXTI_GetITStatus(EXTI_Line7) == SET) {
    /* Clear interrupt pending bit */
    EXTI_ClearITPendingBit(EXTI_Line7);
    //
  }
  if (EXTI_GetITStatus(EXTI_Line8) == SET) {
    /* Clear interrupt pending bit */
    EXTI_ClearITPendingBit(EXTI_Line8);
    ++gaFreqCount[1][1];
  }
  if (EXTI_GetITStatus(EXTI_Line9) == SET) {
    /* Clear interrupt pending bit */
    EXTI_ClearITPendingBit(EXTI_Line9);
    ++gaFreqCount[1][2];
  }
}

/**
  * @brief  This function handles External line 10 interrupt request.
  * @param  None
  * @retval None
  */
void EXTI15_10_IRQHandler(void)
{
  if (EXTI_GetITStatus(EXTI_Line10) == SET) {
    /* Clear interrupt pending bit */
    EXTI_ClearITPendingBit(EXTI_Line10);
    ++gaFreqCount[1][3];
  }
  EXTI_ClearITPendingBit(EXTI_Line11 | EXTI_Line12 | EXTI_Line13 | EXTI_Line14 | EXTI_Line14);
}
#endif

///**
//  * @brief  This function handles ethernet DMA interrupt request.
//  * @param  None
//  * @retval None
//  */
//void ETH_IRQHandler(void)
//{
//  portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
//
//  /* Frame received */
//  if ( ETH_GetDMAFlagStatus(ETH_DMA_FLAG_R) == SET) 
//  {
//    /* Give the semaphore to wakeup LwIP task */
//    xSemaphoreGiveFromISR( s_xSemaphore, &xHigherPriorityTaskWoken );   
//  }
//	
//  /* Clear the interrupt flags. */
//  /* Clear the Eth DMA Rx IT pending bits */
//  ETH_DMAClearITPendingBit(ETH_DMA_IT_R);
//  ETH_DMAClearITPendingBit(ETH_DMA_IT_NIS);
//	
//  /* Switch tasks if necessary. */	
//  if( xHigherPriorityTaskWoken != pdFALSE )
//  {
//    portEND_SWITCHING_ISR( xHigherPriorityTaskWoken );
//  }
//}

/******************************************************************************/
/*                 STM32F4xx Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f4xx.s).                                               */
/******************************************************************************/
/**
  * @brief  This function handles PPP interrupt request.
  * @param  None
  * @retval None
  */
/*void PPP_IRQHandler(void)
{
}*/

#include "switch.h"
#if (MKPSH10 != 0)
//TIM4 Handler
void TIM4_IRQHandler(void) {
  if (TIM_GetITStatus(POWER_ON_TIMER, TIM_IT_CC1) != RESET)  {
    TIM_ClearITPendingBit(POWER_ON_TIMER, TIM_IT_CC1);
    SetPWM_Pulse(1);    
  }
}
#endif

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
