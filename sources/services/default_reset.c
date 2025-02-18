//******************************************************************************
// ��� �����    :       default_reset.c
// ���������    :       
// �����        :       ���������� �.�.
// ����         :       13.12.2019
//
//------------------------------------------------------------------------------
/**
�������������� ��������� ��������

**/

#include "default_reset.h"

#include "stm32f4xx_rcc.h"

void DefResetInit() {
  //Enable the clock for the GPIOs
  RCC_AHB1PeriphClockCmd(GPIO_DEFRES_PERIPH, ENABLE);
  //Configure these pins using GPIO_Init();
  GPIO_InitTypeDef cGPIO_InitStruct;
  cGPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
  cGPIO_InitStruct.GPIO_Pin = GPIO_DEFRES_PIN;
  cGPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_DOWN;
  cGPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_Init(GPIO_DEFRES_PORT, &cGPIO_InitStruct);
}

uint16_t ProcessDefResetValue() {
  uint16_t cValue = 0;
  uint16_t cReg = GPIO_DEFRES_PORT->IDR;
  cReg &= GPIO_DEFRES_PIN;
  if (cReg != 0) {
    cValue = 1;
  }
  return cValue;
}

static int csResetCount = 0;
uint16_t gDefResValue = 0;

#include "info.h"
#include <timeouts.h>
#include "log.h"
#include "settings.h"
void ProcessDefaultReset(void * arg) {
  //�������� ������� ������ �� PA4
  uint16_t cDefResValue = ProcessDefResetValue(); //��������� ������� �������
  gDefResValue = cDefResValue;
  if (cDefResValue > 0) {
    if (csResetCount >= DEFAULT_RESET_TIMEOUT) { //���� >= 5 ���
      csResetCount = 0;
      settings_default(true); //������ ��������� ��������
#ifdef USE_SWITCH //������������ ����� kmz8895
      switch_off(0); switch_off(1);
#endif
      SaveEvent(0, etCount, ecCount); //����. �������: ������� �� ���������
      return;
    } else {
      ++csResetCount;
    }
  } else {
    if (csResetCount >= 1) { //���� >= 1 ��� � < 5 ���
      //������ ������������
#ifdef USE_SWITCH //������������ ����� kmz8895
      switch_off(0); switch_off(1);
#endif
      SaveEvent(0, etCount, ecCount); //����. �������: ������� �� ���������
    }
    csResetCount = 0;
  }
  sys_timeout(DEFAULT_RESET_CHECK_INTERVAL, ProcessDefaultReset, NULL);
}


