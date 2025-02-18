//******************************************************************************
// Имя файла    :       temp_ctrl.c
// Заголовок    :       
// Автор        :       Сосновских А.А.
// Дата         :       20.06.2019
//
//------------------------------------------------------------------------------
/**
Контроль температуры кристалла МКПШ

**/

#include "temp_ctrl.h"
#include "stm32f4xx_rcc.h"

void TempInitADC() {
  //Enable the ADC interface clock using 
  RCC_APB2PeriphClockCmd(TEMP_ADC_PERIPH, ENABLE);
//  //Enable the clock for the ADC GPIOs
//  RCC_AHB1PeriphClockCmd(TEMP_GPIO_PERIPH, ENABLE);
//  //Configure these ADC pins in analog mode using GPIO_Init();
//  GPIO_InitTypeDef cGPIO_InitStruct;
//  cGPIO_InitStruct.GPIO_Mode = GPIO_Mode_AN;
//  cGPIO_InitStruct.GPIO_Pin = TEMP_GPIO_PIN;
//  cGPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
//  cGPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
//  GPIO_Init(TEMP_GPIO_PORT, &cGPIO_InitStruct);
  
//  ADC_CommonInitTypeDef  ADC_CommonInitStructure; 
//  ADC_CommonInitStructure.ADC_Mode = ADC_Mode_Independent; 
//  ADC_CommonInitStructure.ADC_Prescaler = ADC_Prescaler_Div4; 
//  ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled; 
//  ADC_CommonInitStructure.ADC_TwoSamplingDelay = 0; 
//  ADC_CommonInit(&ADC_CommonInitStructure); 

  ADC_AutoInjectedConvCmd(TEMP_ADC, DISABLE);
  //Configure the ADC Prescaler, conversion resolution and data
  ADC_InitTypeDef cADC_InitStruct;
  ADC_StructInit(&cADC_InitStruct);
  cADC_InitStruct.ADC_ContinuousConvMode = ENABLE;
  cADC_InitStruct.ADC_DataAlign = ADC_DataAlign_Right;
  cADC_InitStruct.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T1_CC1;
  cADC_InitStruct.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;
  cADC_InitStruct.ADC_NbrOfConversion = 1;
  cADC_InitStruct.ADC_Resolution = ADC_Resolution_12b;
  cADC_InitStruct.ADC_ScanConvMode = DISABLE;
  ADC_Init(TEMP_ADC, &cADC_InitStruct);
  //Activate the ADC peripheral using ADC_Cmd() function.
  ADC_RegularChannelConfig(TEMP_ADC, TEMP_ADC_CHANNEL, 1, ADC_SampleTime_480Cycles);
  ADC_Cmd(TEMP_ADC, ENABLE);
  ADC_TempSensorVrefintCmd(ENABLE);
  ADC_ContinuousModeCmd(TEMP_ADC, ENABLE);
  ADC_SoftwareStartConv(TEMP_ADC);
}

#include "extio.h"

uint16_t gTempValue = 0;

#define TEMP_30_GRAD  0x1fff7a2c
#define TEMP_110_GRAD 0x1fff7a2e
#define TEMP_BASE     (30)

float gTempCoef = 1;
float gTempBase = 30;

//Блок усреднения показаний
float CalcTempValue(uint16_t iValue) {
  float cfValue = iValue;
  cfValue = (cfValue - gTempBase) / gTempCoef;
  cfValue += TEMP_BASE;
  return cfValue;
}

struct sAdcBuff gTempBuff = {
  .CurrIdx = 0,
  .Full = false,
  .Disabled = false,
  .State = vsNone,
  .CalcValueCB = CalcTempValue
};

void TempInit() {
  TempInitADC();
  BuffInit(&gTempBuff);
  uint16_t * cp30 = (uint16_t *)TEMP_30_GRAD;
  uint16_t * cp110 = (uint16_t *)TEMP_110_GRAD;
  float cf30 = *cp30;
  float cf110 = *cp110;
  gTempBase = cf30;
  gTempCoef = (cf110 - cf30)/(110 - 30);
}

#include <stddef.h>
#include <math.h>
#include "log.h"
#include "info.h"

extern int sw_restart( int iSwIdx );
void ProcessTempZoneEvent(const struct sValLevelDesc * ipTempLevelDesc, int iCode) {
  enum eEventCode cCode = (enum eEventCode)iCode;
  float cfValue = fBuffAverage(&gTempBuff);
  int8_t cValue = floor(ipTempLevelDesc->InLevelDown);
  if (cCode == ecTempLess) {
#ifdef USE_SWITCH //Использовать свитч kmz8895
    if (ipTempLevelDesc->ValState == vsWarningHigh) {
      //Включить порты 1, 2
      switch_port_power( 0, 1 );
      vTaskDelay(10);
      switch_port_power( 1, 1 );
      //Включить свитч 2 и порты 3, 4
  #ifdef TWO_SWITCH_ITEMS
      vTaskDelay(50);
      switch_on(1);
      vTaskDelay(50);
      sw_restart( 1 );
      vTaskDelay(10);
      switch_port_power( 2, 1 );
      vTaskDelay(10);
      switch_port_power( 3, 1 );
  #endif
    }
#endif
    if (ipTempLevelDesc->ValState == vsNormal) {
    }
    SaveFloatEvent(etSys, cCode, cfValue, cValue);
  }
  if (cCode == ecTempMore) {
#ifdef USE_SWITCH //Использовать свитч kmz8895
    if (ipTempLevelDesc->ValState == vsFaultHigh) {
      //Выключить порты 1, 2
      switch_port_power( 0, 0 );
      vTaskDelay(10);
      switch_port_power( 1, 0 );
      //Выключить свитч 2 и порты 3, 4
  #ifdef TWO_SWITCH_ITEMS
      vTaskDelay(10);
      switch_off(1);
      vTaskDelay(10);
      switch_port_power( 2, 0 );
      vTaskDelay(10);
      switch_port_power( 3, 0 );
  #endif
    }
#endif
    if (ipTempLevelDesc->ValState == vsWarningHigh) {
    }
    cValue = floor(ipTempLevelDesc->InLevelUp);
    SaveFloatEvent(etSys, cCode, cfValue, cValue);
  }
}


//=== Дескрипторы состояний блоков контроля напряжения
//Блок контроля диапазонов
#include "settings.h"
struct sValLevelDesc gTempZone[] = { //По зонам
  {vsNormal     ,  72.0,   0.0, 3.0}, //10 Зеленый;
  {vsWarningHigh,  82.0,  75.0, 3.0}, //10 Желтый;
  {vsFaultHigh  , 200.0,  85.0, 3.0}  //12 Красный; Откл / Вкл портов 1, 2; Откл / Вкл портов 3, 4 и свитча 2
};

struct sAdcBlock  TempBlockZone = {
  .aValLevelDesc = &gTempZone[0],
  .LevelDescNumber = sizeof(gTempZone) / sizeof(struct sValLevelDesc),
  .CurrLevelDescIdx = -1,
  .EventFunc = ProcessTempZoneEvent

};

void SetupZoneCtrl() { //Из rsettings->TempXxxLevel в gTempZone
  struct sValLevelDesc * cpDesc;
  for (int i = 0; i < TempBlockZone.LevelDescNumber; ++i) {
    cpDesc = &TempBlockZone.aValLevelDesc[i];
    if (cpDesc->ValState == vsNormal) {
      cpDesc->InLevelDown = rsettings->TempWarningLevel - cpDesc->LevelHyst;
      cpDesc->InLevelUp   = 0;
    }
    if (cpDesc->ValState == vsWarningHigh) {
      cpDesc->InLevelDown = rsettings->TempFaultLevel - cpDesc->LevelHyst;
      cpDesc->InLevelUp   = rsettings->TempWarningLevel;
    }
    if (cpDesc->ValState == vsFaultHigh) {
      cpDesc->InLevelDown = 200.0;
      cpDesc->InLevelUp   = rsettings->TempFaultLevel;
    }
  }
}

//Обработка полученного значения
void ProcessTempValue(struct sAdcBlock * ipTempBlock, float iValue) {
  if (ipTempBlock->CurrLevelDescIdx < 0) {
     //Просто найти состояние по InValue, и установить
    for (int i = 0; i < ipTempBlock->LevelDescNumber; ++i) {
      if (iValue < ipTempBlock->aValLevelDesc[i].InLevelDown) {
        ipTempBlock->CurrLevelDescIdx = i;
        break;
      }
    }
    if (ipTempBlock->CurrLevelDescIdx < 0) {
      ipTempBlock->CurrLevelDescIdx = ipTempBlock->LevelDescNumber - 1;
    }
  } else {
    const struct sValLevelDesc * cpPrevDesc = NULL, * cpNextDesc = NULL;
    if (ipTempBlock->CurrLevelDescIdx >= 0) {
      if (ipTempBlock->CurrLevelDescIdx > 0) { //Не первый
        cpPrevDesc = &ipTempBlock->aValLevelDesc[ipTempBlock->CurrLevelDescIdx - 1];
        if (iValue < cpPrevDesc->InLevelDown) {
          ipTempBlock->CurrLevelDescIdx -= 1;
          if (ipTempBlock->EventFunc != NULL)
            ipTempBlock->EventFunc(cpPrevDesc, ecTempLess);
          return;
        }
      }
      if (ipTempBlock->CurrLevelDescIdx < (ipTempBlock->LevelDescNumber - 1)) { //Не последний
        cpNextDesc = &ipTempBlock->aValLevelDesc[ipTempBlock->CurrLevelDescIdx + 1];
        if (iValue > cpNextDesc->InLevelUp) {
          ipTempBlock->CurrLevelDescIdx += 1;
          if (ipTempBlock->EventFunc != NULL)
            ipTempBlock->EventFunc(cpNextDesc, ecTempMore);
          return;
        }
      }
    }
  }
}

#include "portmacro.h"
portTickType xTaskGetTickCount( void );

float GetAverageTemp() {
  return fBuffAverage(&gTempBuff);
}

void ProcessTempADCValue() {
  uint16_t cValue = 0;
  if (ADC_GetFlagStatus(TEMP_ADC, ADC_FLAG_EOC) != RESET) { 
    cValue = ADC_GetConversionValue(TEMP_ADC);
    ADC_ClearFlag(TEMP_ADC, ADC_FLAG_EOC);
    gTempValue = cValue;
    BuffNext(&gTempBuff, cValue);
    float cTempValueAverage = GetAverageTemp();
    switch (gTempBuff.State) {
    case vsNone:
    case vsFaultHigh:
    case vsWarningHigh:
      if (cTempValueAverage <= rsettings->TempWarningLevel) {
        gTempBuff.State = vsNormal;
        extio_led(EXTIO_RS232, EXTIO_LED_OFF);
      } else {
        if (cTempValueAverage <= rsettings->TempFaultLevel) {
          gTempBuff.State = vsWarningHigh;
        } else {
          gTempBuff.State = vsFaultHigh;
        }
      }
      break;
    case vsNormal:
      if (cTempValueAverage > rsettings->TempWarningLevel) {
        if (cTempValueAverage > rsettings->TempFaultLevel) {
          gTempBuff.State = vsFaultHigh;
        } else {
          gTempBuff.State = vsWarningHigh;
        }
      }
      break;
    default:
      break;
    }
    //
    ProcessTempValue(&TempBlockZone, cTempValueAverage);
  }
  if (gTempBuff.State == vsFaultHigh) {
    uint32_t cTickCount = xTaskGetTickCount();
    cTickCount %= 2000;
    if (cTickCount < 1000) {
      extio_led(EXTIO_RS232, EXTIO_LED_ON);
    } else {
      extio_led(EXTIO_RS232, EXTIO_LED_OFF);
    }
  }
}

enum eValState GetTempZone() {
  return TempBlockZone.aValLevelDesc[TempBlockZone.CurrLevelDescIdx].ValState;
}

bool IsTempNormal() {
  return (gTempBuff.State == vsNormal);
}

#ifdef USE_STP
  #include "stp.h"
#endif
enum eValState cCurrTempState = vsNone;
bool CheckTempRight() {
  //Контроль температуры
  ProcessTempADCValue();
  if (gTempBuff.State != cCurrTempState) {
#ifdef USE_STP
    // !!! Пересечение с USE_SWITCH
    // Следовало бы поставить USE_SWITCH, но setPortsEnable определена в stp.c
    if (cCurrTempState != vsNone)
      setPortsEnable(gTempBuff.State != vsFaultHigh);
#endif
    cCurrTempState = gTempBuff.State;
  }
  if (cCurrTempState == vsFaultHigh) {
    return false;
  }
  return true;
}

