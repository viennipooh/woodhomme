//******************************************************************************
// Имя файла    :       default_reset.c
// Заголовок    :       
// Автор        :       Сосновских А.А.
// Дата         :       13.12.2019
//
//------------------------------------------------------------------------------
/**
Восстановление заводских настроек

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
  //Проверка сигнала сброса на PA4
  uint16_t cDefResValue = ProcessDefResetValue(); //Прочитать уровень сигнала
  gDefResValue = cDefResValue;
  if (cDefResValue > 0) {
    if (csResetCount >= DEFAULT_RESET_TIMEOUT) { //Если >= 5 сек
      csResetCount = 0;
      settings_default(true); //Запись заводских настроек
#ifdef USE_SWITCH //Использовать свитч kmz8895
      switch_off(0); switch_off(1);
#endif
      SaveEvent(0, etCount, ecCount); //Спец. событие: Переход на загрузчик
      return;
    } else {
      ++csResetCount;
    }
  } else {
    if (csResetCount >= 1) { //Если >= 1 сек и < 5 сек
      //Просто перезагрузка
#ifdef USE_SWITCH //Использовать свитч kmz8895
      switch_off(0); switch_off(1);
#endif
      SaveEvent(0, etCount, ecCount); //Спец. событие: Переход на загрузчик
    }
    csResetCount = 0;
  }
  sys_timeout(DEFAULT_RESET_CHECK_INTERVAL, ProcessDefaultReset, NULL);
}


