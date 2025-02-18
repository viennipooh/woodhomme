//******************************************************************************
// ��� �����    :       dev_param.h
// ���������    :
// �����        :       ���������� �.�.
// ����         :       18.03.2020
//
//------------------------------------------------------------------------------
/**
��������� ���������� ����������
**/

#ifndef __DEV_PARAM_H
#define __DEV_PARAM_H

#include <stdint.h>
#include <stdbool.h>
#include "stm32f4xx_gpio.h"



#define WD_INTERVAL  ( 1000)
#define WD_PULSE_LEN (  500)

#include <time.h>


//����������� ���
struct sCtrlPin {
  GPIO_TypeDef  * GPIO_Port;
  uint16_t        PinIdx;
};
typedef struct sCtrlPin tCtrlPin;


extern uint16_t ToGPIO_Pin(uint16_t iPin);
extern void InitDevParam();
extern void SetWD();


#endif //__DEV_PARAM_H
