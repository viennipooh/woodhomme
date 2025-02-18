//******************************************************************************
// Имя файла    :       vcc_ctrl.h
// Заголовок    :       
// Автор        :       Сосновских А.А.
// Дата         :       21.11.2018
//
//------------------------------------------------------------------------------
/**
Контроль Uпит МКПШ

**/

#ifndef __VCC_CTRL_H
#define __VCC_CTRL_H

#include "stm32f4xx_adc.h"
#include "stdbool.h"
#include "info.h"
#ifdef USE_SWITCH //Использовать свитч kmz8895
  #include "switch.h"
#endif

#include "adc_ctrl.h"

//Подключение контроля Uпит (с версии 4.1 - автоматически) !!! не получается
#if (IMC_FTX_MC != 0)
//  #define NO_USE_VCC
#endif

//Версия для МКПШ без контроля питания
#if (MKPSH10 != 0)
//  #define NO_USE_VCC
#endif

//#ifndef NO_USE_VCC
// #if ((UTD_M == 0) && (IIP == 0))
//  #define USE_VCC_PD_PIN
// #endif
//#endif

//Коэффициент делителя
#define LEVEL_DIV_COEF (1630.0 / 12.05) //

//Коэффициенты входных диодов
#if (MKPSH10 != 0)
  #define LEVEL_VD_A (-0.047) //
  #define LEVEL_VD_B (1.925) //
#endif
#if (IMC_FTX_MC != 0)
  #define LEVEL_VD_A (0.043) //
  #define LEVEL_VD_B (0.416) //
#endif
#if ((UTD_M != 0) || (IIP != 0))
  // !!! Требуется настройка (независимая цепь измерения)
  #define LEVEL_VD_A (1.01) //
  #define LEVEL_VD_B (1.567) //
 #if (UTD_M != 0)
  #define VCC_LED_GPIO GPIOD //
  #define VCC_LED_PIN  GPIO_Pin_14 //
 #endif
 #if (IIP != 0)
  #define VCC_LED_GPIO GPIOC //
  #define VCC_LED_PIN  GPIO_Pin_0 //
 #endif

  #define VOLT_LOW_LEVEL  (9.0) //
  #define VOLT_HIGH_LEVEL (9.4) //
 #if (UTD_M != 0)
  #define VOLT_UP_LOW_LEVEL  (15.5) //
  #define VOLT_UP_HIGH_LEVEL (16.0) //
  #define USE_HIGH_BORDER //Использовать верхнюю границу напряжения
 #endif
#else
  #define VOLT_LOW_LEVEL  (9.0) //
  #define VOLT_HIGH_LEVEL (9.3) //
#endif

#define VOLT_MIN_CTRL_LEVEL  (3.0) //Уровень принятия решения о контроле

#if (MKPSH10 != 0)
  #define GPIO_VCC_PIN    GPIO_Pin_9
  #define GPIO_VCC_PD_PIN GPIO_Pin_10
  #define GPIO_VCC_PORT   GPIOF
  #define GPIO_VCC_ADC    ADC3
  #define GPIO_VCC_PERIPH RCC_AHB1Periph_GPIOF
  #define GPIO_VCC_ADC_CHANNEL ADC_Channel_7
  #define VCC_ADC_PERIPH  RCC_APB2Periph_ADC3
#endif
#if (IMC_FTX_MC != 0)
  #define GPIO_VCC_PIN  GPIO_Pin_2
  #define GPIO_VCC_PD_PIN GPIO_Pin_1
  #define GPIO_VCC_PORT GPIOA
  #define GPIO_VCC_ADC  ADC3
  #define GPIO_VCC_PERIPH RCC_AHB1Periph_GPIOA
  #define GPIO_VCC_ADC_CHANNEL ADC_Channel_2
  #define VCC_ADC_PERIPH  RCC_APB2Periph_ADC3
#endif
#if ((UTD_M != 0) || (IIP != 0))
  #define GPIO_VCC_PIN  GPIO_Pin_5
  #define GPIO_VCC_PORT GPIOA
  #define GPIO_VCC_ADC  ADC2
  #define GPIO_VCC_PERIPH RCC_AHB1Periph_GPIOA
  #define GPIO_VCC_ADC_CHANNEL ADC_Channel_5
  #define VCC_ADC_PERIPH  RCC_APB2Periph_ADC2
#endif

extern struct sAdcBuff gVccBuff;

extern void VccInitADC();

extern void TestVccCurcuit();
extern float GetAverageVcc();
extern enum eValState GetVccZone();

extern bool CheckVccRight(bool iSetPortEnable);
extern bool IsVccNormal();
extern bool UseVccValue();

#endif //__VCC_CTRL_H
