//******************************************************************************
// Имя файла    :       default_reset.h
// Заголовок    :       
// Автор        :       Сосновских А.А.
// Дата         :       13.12.2019
//
//------------------------------------------------------------------------------
/**
Восстановление заводских настроек

**/

#ifndef __DEFAULT_RESET_H
#define __DEFAULT_RESET_H

#include <stdint.h>
#include <stdbool.h>

#include "extio.h"

#if (MKPSH10 != 0)
  #define GPIO_DEFRES_PIN         GPIO_Pin_4
  #define GPIO_DEFRES_PORT        GPIOA
  #define GPIO_DEFRES_PERIPH      RCC_AHB1Periph_GPIOA
#endif
#if (IMC_FTX_MC != 0)
  #define GPIO_DEFRES_PIN         GPIO_Pin_6
  #define GPIO_DEFRES_PORT        GPIOB
  #define GPIO_DEFRES_PERIPH      RCC_AHB1Periph_GPIOB
#endif
#if (UTD_M != 0)
  #define GPIO_DEFRES_PIN         GPIO_Pin_7
  #define GPIO_DEFRES_PORT        GPIOD
  #define GPIO_DEFRES_PERIPH      RCC_AHB1Periph_GPIOD
#endif

#if (IIP != 0)
  #define GPIO_DEFRES_PIN         GPIO_Pin_3
  #define GPIO_DEFRES_PORT        GPIOB
  #define GPIO_DEFRES_PERIPH      RCC_AHB1Periph_GPIOB
#endif

#if ((UTD_M != 0) || (IIP != 0))
  #define DEFAULT_RESET_CHECK_INTERVAL  ( 700) //~= 1 сек
#else
  #define DEFAULT_RESET_CHECK_INTERVAL  (1000) //~= 1 сек
#endif
#define DEFAULT_RESET_TIMEOUT            (5)

extern void DefResetInit();
extern void ProcessDefaultReset(void * arg);


#endif //__DEFAULT_RESET_H

