//******************************************************************************
// Имя файла    :       can.h
// Заголовок    :       
// Автор        :       Сосновских А.А.
// Дата         :       13.06.2019
//
//------------------------------------------------------------------------------
/**
Драйвер CAN

**/
#ifndef __CAN_H__
#define __CAN_H__

#include "stm32f4xx_can.h"

#define USE_CAN

#define CAN_CMD_Test_StdId  (0x101)
#define CAN_THREAD_PRIO ( tskIDLE_PRIORITY + 2 )


extern uint8_t InitCAN(CAN_TypeDef * ipCAN, uint16_t iBaudrateKbod);
extern int CAN_mode_init();
extern void ProcessCAN();


#endif //__CAN_H__
