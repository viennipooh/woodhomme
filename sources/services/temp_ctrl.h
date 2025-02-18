//******************************************************************************
// Имя файла    :       temp_ctrl.h
// Заголовок    :       
// Автор        :       Сосновских А.А.
// Дата         :       20.06.2019
//
//------------------------------------------------------------------------------
/**
Контроль температуры кристалла МКПШ

**/

#ifndef __TEMP_CTRL_H
#define __TEMP_CTRL_H

#include "stm32f4xx_adc.h"
#include "stdbool.h"
#include "extio.h"
#include "info.h"
#ifdef USE_SWITCH //Использовать свитч kmz8895
  #include "switch.h"
#endif

#include "adc_ctrl.h"

//Подключение контроля Uпит
//#define USE_TEMP_CTRL

#if (MKPSH10 != 0)
  #define TEMP_ADC  ADC1
  #define TEMP_ADC_CHANNEL ADC_Channel_18
  #define TEMP_ADC_PERIPH  RCC_APB2Periph_ADC1
#endif
#if ((IMC_FTX_MC != 0) || (UTD_M != 0) || (IIP != 0))
  #define TEMP_ADC  ADC1
  #define TEMP_ADC_CHANNEL ADC_Channel_18
  #define TEMP_ADC_PERIPH  RCC_APB2Periph_ADC1
#endif

enum eTempState {
  tsNone,         //0
  tsFaultLow,     //1
  tsWarningLow,   //2
  tsNormal,       //10
  tsWarningHigh,  //11
  tsFaultHigh,    //12
  tsCount
};

#define BUFF_SIZE (10)

extern struct sAdcBuff gTempBuff;

extern void ProcessADCValue();
extern bool IsTempNormal();
#ifdef USE_TEMP_CTRL
extern void TempInit();

extern float GetAverageTemp();
extern enum eValState GetTempZone();

extern bool CheckTempRight();
extern void SetupZoneCtrl();
#endif


#endif //__TEMP_CTRL_H
