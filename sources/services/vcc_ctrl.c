//******************************************************************************
// Имя файла    :       vcc_ctrl.c
// Заголовок    :       
// Автор        :       Сосновских А.А.
// Дата         :       21.11.2018
//
//------------------------------------------------------------------------------
/**
Контроль Uпит МКПШ

**/

#include "vcc_ctrl.h"
#include "stm32f4xx_rcc.h"

//#ifdef USE_VCC_PD_PIN
//void SetPdPin(bool iPull) {
//  //Пин подтяжки к 0В
//  GPIO_InitTypeDef cGPIO_InitStruct;
//  cGPIO_InitStruct.GPIO_Pin = GPIO_VCC_PD_PIN;
//  cGPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
//  if (iPull) {
//    cGPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_DOWN; //GPIO_PuPd_UP;
//  } else {
//    cGPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
//  }
//  cGPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_DOWN;
//  cGPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
//  GPIO_Init(GPIO_VCC_PORT, &cGPIO_InitStruct);
//}
//#endif

void VccInitADC() {
  //Enable the ADC interface clock using 
  RCC_APB2PeriphClockCmd(VCC_ADC_PERIPH, ENABLE);
  //Enable the clock for the ADC GPIOs
  RCC_AHB1PeriphClockCmd(GPIO_VCC_PERIPH, ENABLE);
  //Configure these ADC pins in analog mode using GPIO_Init();
  GPIO_InitTypeDef cGPIO_InitStruct;
  cGPIO_InitStruct.GPIO_Pin = GPIO_VCC_PIN;
  cGPIO_InitStruct.GPIO_Mode = GPIO_Mode_AN;
  cGPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
  cGPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_Init(GPIO_VCC_PORT, &cGPIO_InitStruct);

//#ifdef USE_VCC_PD_PIN
//  SetPdPin(true);
//#endif
  
  ADC_CommonInitTypeDef  ADC_CommonInitStructure; 
  ADC_CommonInitStructure.ADC_Mode = ADC_Mode_Independent; 
  ADC_CommonInitStructure.ADC_Prescaler = ADC_Prescaler_Div4; 
  ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled; 
  ADC_CommonInitStructure.ADC_TwoSamplingDelay = 0; 
  ADC_CommonInit(&ADC_CommonInitStructure); 

  ADC_AutoInjectedConvCmd(GPIO_VCC_ADC, DISABLE);
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
  ADC_Init(GPIO_VCC_ADC, &cADC_InitStruct);
  //Activate the ADC peripheral using ADC_Cmd() function.
  ADC_RegularChannelConfig(GPIO_VCC_ADC, GPIO_VCC_ADC_CHANNEL, 1, ADC_SampleTime_480Cycles);
  ADC_Cmd(GPIO_VCC_ADC, ENABLE);
  
  ADC_ContinuousModeCmd(GPIO_VCC_ADC, ENABLE);
  ADC_SoftwareStartConv(GPIO_VCC_ADC);
#if (UTD_M != 0)
  //Светодиод питания (зеленый/красный)
  GPIO_INIT(GPIOD, 14, MODE_OUT, OTYPE_PP, OSPEED_VHS, PUPD_N, 0, 0); // Red
#endif
#if (IIP != 0)
  //Светодиод питания (зеленый/красный)
  GPIO_INIT(GPIOC,  0, MODE_OUT, OTYPE_PP, OSPEED_VHS, PUPD_N, 0, 0); // Red
#endif
}

#include "extio.h"

float CalcVccValue(uint16_t iValue) {
  float cfVD, cfValue = iValue;
  cfValue = cfValue / LEVEL_DIV_COEF; //LEVEL_COEF;
  cfVD = cfValue * LEVEL_VD_A + LEVEL_VD_B;
#if ((UTD_M != 0) || (IIP != 0))
  cfValue = cfVD;
#else
  cfValue += cfVD;
#endif
  return cfValue;
}

//Блок усреднения показаний
struct sAdcBuff gVccBuff = {
  .CurrIdx = 0,
  .Full = false,
#ifdef NO_USE_VCC
  .Disabled = true,
#else
  .Disabled = false,
#endif
  .State = vsNone,
  .CalcValueCB = CalcVccValue
};

//=== Дескрипторы состояний блоков контроля напряжения
//Блок контроля диапазонов
struct sValLevelDesc gVccVolt[] = { //По вольтам
//  {vsWarningLow,  8.5,  0.0, 0.5}, //U <  8.5;
//  {vs09V       ,  9.5,  9.0, 0.5}, //U <  9.5; U > 10.0
  {vsWarningLow,  9.5,  0.0, 0.5}, //U <  9.5;
  
  {vs10V       , 10.5, 10.0, 0.5}, //U < 10.5; U > 11.0
  {vs11V       , 11.5, 11.0, 0.5}, //U < 11.5; U > 12.0
  {vs12V       , 12.5, 12.0, 0.5}, //U < 12.5; U > 13.0
  {vs13V       , 13.5, 13.0, 0.5}, //U < 13.5; U > 14.0
  {vs14V       , 14.5, 14.0, 0.5}, //U < 14.5; U > 15.0
  {vs15V       , 15.5, 15.0, 0.5}, //U < 15.5; U > 16.0
  {vsFaultHigh , 30.0, 16.0, 0.5}
};

struct sValLevelDesc gVccZone[] = { //По зонам
//  {vsFaultLow  ,  8.0,  0.0, 0.5}, //1  Красный; Откл / Вкл портов 1, 2
//  {vsWarningLow, 10.5,  8.5, 0.5}, //2  Желтый ; Откл / Вкл портов 3, 4 и свитча 2
  {vsFaultLow  ,  9.0,  0.0, 0.5}, //1  Красный; Откл / Вкл портов 1, 2
  
  {vsWarningLow, 10.5,  9.5, 0.5}, //2  Желтый ; Откл / Вкл портов 3, 4 и свитча 2
  {vsNormal    , 15.5, 11.0, 0.5}, //10 Зеленый;
  {vsFaultHigh , 30.0, 16.0, 0.5}  //12 Красный;
};

#include <stddef.h>
#include "log.h"

bool UseVccValue() {
  if (gVccBuff.Disabled)
    return false;
  float cVccValueAverage = GetAverageVcc();
  if (cVccValueAverage < VOLT_MIN_CTRL_LEVEL) //Не действовать по Vcc
    return false;
  return true;
}

void ProcessVccEvent(const struct sValLevelDesc * ipVccLevelDesc, int iCode) {
  if (!UseVccValue()) //Не действовать по Vcc
    return;
  float cfValue = 0;
  if (iCode == ecVoltageMore)
    cfValue = ipVccLevelDesc->InLevelUp;
  if (iCode == ecVoltageLess)
    cfValue = ipVccLevelDesc->InLevelDown;
  int8_t cValue = cfValue;
  SaveFloatEvent(etPower, (enum eEventCode)iCode, cfValue, cValue);
}

uint32_t gOnCount = 0, gOffCount = 0;

#ifdef USE_SWITCH //Использовать свитч kmz8895
  extern int sw_restart( int iSwIdx );
#endif
void ProcessVccZoneEvent(const struct sValLevelDesc * ipVccLevelDesc, int iCode) {
#ifdef USE_SWITCH //Использовать свитч kmz8895
  enum eEventCode cCode = (enum eEventCode)iCode;
  if (!UseVccValue()) //Не действовать по Vcc
    return;
  if (cCode == ecVoltageMore) {
    if (ipVccLevelDesc->ValState == vsWarningLow) {
      //Включить порты 1, 2
      switch_port_power( 0, 1 );
      vTaskDelay(10);
      switch_port_power( 1, 1 );
      vTaskDelay(10);
    }
    if (ipVccLevelDesc->ValState == vsNormal) {
      //Включить свитч 2 и порты 3, 4
  #ifdef TWO_SWITCH_ITEMS
      ++gOnCount;
      switch_on(1);
      vTaskDelay(200);
      sw_restart( 1 );
      vTaskDelay(10);
      switch_port_power( 2, 1 );
      vTaskDelay(10);
      switch_port_power( 3, 1 );
      vTaskDelay(10);
  #endif
    }
  }
  if (cCode == ecVoltageLess) {
    if (ipVccLevelDesc->ValState == vsFaultLow) {
      //Выключить порты 1, 2
      switch_port_power( 0, 0 );
      vTaskDelay(10);
      switch_port_power( 1, 0 );
      vTaskDelay(10);
    }
    if (ipVccLevelDesc->ValState == vsWarningLow) {
      //Выключить свитч 2 и порты 3, 4
  #ifdef TWO_SWITCH_ITEMS
      ++gOffCount;
      switch_off(1);
      vTaskDelay(10);
      switch_port_power( 2, 0 );
      vTaskDelay(10);
      switch_port_power( 3, 0 );
      vTaskDelay(10);
  #endif
    }
  }
#endif
}

struct sAdcBlock  VccBlockVolt = {
  .aValLevelDesc = &gVccVolt[0],
  .LevelDescNumber = sizeof(gVccVolt) / sizeof(struct sValLevelDesc),
  .CurrLevelDescIdx = -1,
  .EventFunc = ProcessVccEvent
};

struct sAdcBlock  VccBlockZone = {
  .aValLevelDesc = &gVccZone[0],
  .LevelDescNumber = sizeof(gVccZone) / sizeof(struct sValLevelDesc),
  .CurrLevelDescIdx = -1,
  .EventFunc = ProcessVccZoneEvent

};

//Обработка полученного напряжения
void ProcessVccValue(struct sAdcBlock * ipVccBlock, float iValue) {
  if (ipVccBlock->CurrLevelDescIdx < 0) {
     //Просто найти состояние по InValue, и установить
    for (int i = 0; i < ipVccBlock->LevelDescNumber; ++i) {
      if (iValue < ipVccBlock->aValLevelDesc[i].InLevelDown) {
        ipVccBlock->CurrLevelDescIdx = i;
        break;
      }
    }
    if (ipVccBlock->CurrLevelDescIdx < 0) {
      ipVccBlock->CurrLevelDescIdx = ipVccBlock->LevelDescNumber - 1;
    }
  } else {
    const struct sValLevelDesc * cpPrevDesc = NULL, * cpNextDesc = NULL;
    if (ipVccBlock->CurrLevelDescIdx >= 0) {
      if (ipVccBlock->CurrLevelDescIdx > 0) { //Не первый
        cpPrevDesc = &ipVccBlock->aValLevelDesc[ipVccBlock->CurrLevelDescIdx - 1];
        if (iValue < cpPrevDesc->InLevelDown) {
          ipVccBlock->CurrLevelDescIdx -= 1;
          if (ipVccBlock->EventFunc != NULL)
            ipVccBlock->EventFunc(cpPrevDesc, ecVoltageLess);
          return;
        }
      }
      if (ipVccBlock->CurrLevelDescIdx < (ipVccBlock->LevelDescNumber - 1)) { //Не последний
        cpNextDesc = &ipVccBlock->aValLevelDesc[ipVccBlock->CurrLevelDescIdx + 1];
        if (iValue > cpNextDesc->InLevelUp) {
          ipVccBlock->CurrLevelDescIdx += 1;
          if (ipVccBlock->EventFunc != NULL)
            ipVccBlock->EventFunc(cpNextDesc, ecVoltageMore);
          return;
        }
      }
    }
  }
}

#include "portmacro.h"
portTickType xTaskGetTickCount( void );

#if ((UTD_M != 0) || (IIP != 0))
  #include "dev_param.h"
#endif
void SetSrcPowerLedState() {
#if ((UTD_M != 0) || (IIP != 0))
  if (IsVccNormal()) {
 #if (UTD_M != 0)
    GPIO_SetBits(GPIOD, GPIO_Pin_14); //Зелёный
    SetPwrMicrotic(true);
 #else
    GPIO_ResetBits(VCC_LED_GPIO, VCC_LED_PIN); //Зелёный
 #endif
  } else {
 #if (UTD_M != 0)
    GPIO_ResetBits(GPIOD, GPIO_Pin_14); //Красный
    SetPwrMicrotic(false);
 #else
    GPIO_SetBits(VCC_LED_GPIO, VCC_LED_PIN); //Красный
 #endif
  }
#else
  if (IsVccNormal()) {
    extio_led(EXTIO_RS232, EXTIO_LED_OFF);
  } else {
    uint32_t cTickCount = xTaskGetTickCount();
    cTickCount %= 2000;
    if (cTickCount < 1000) {
      extio_led(EXTIO_RS232, EXTIO_LED_ON);
    } else {
      extio_led(EXTIO_RS232, EXTIO_LED_OFF);
    }
  }
#endif
}

#if (MKPSH10 != 0)
  #include "settings.h"
#endif
void TestVccCurcuit() {
  uint16_t cCount;
#if (MKPSH10 != 0)
  if (IsHwVersion2()) {
    gVccBuff.Disabled = true;
    return;
  }
#endif
  for (int i = 0; i < 5; ++i) {
    cCount = 10000;
    while ((ADC_GetFlagStatus(GPIO_VCC_ADC, ADC_FLAG_EOC) == RESET) && (--cCount > 0)) {}
    if (ADC_GetFlagStatus(GPIO_VCC_ADC, ADC_FLAG_EOC) != RESET) { 
      uint16_t cValue = ADC_GetConversionValue(GPIO_VCC_ADC);
      ADC_ClearFlag(GPIO_VCC_ADC, ADC_FLAG_EOC);
      BuffNext(&gVccBuff, cValue);
    } else {
      gVccBuff.Disabled = true;
    }
  }
  float cVccValueAverage = GetAverageVcc();
  if (cVccValueAverage < VOLT_MIN_CTRL_LEVEL) {
    gVccBuff.Disabled = true;
  }
}

void ProcessVccADCValue() {
  uint16_t cValue = 0;
  if (ADC_GetFlagStatus(GPIO_VCC_ADC, ADC_FLAG_EOC) != RESET) { 
    cValue = ADC_GetConversionValue(GPIO_VCC_ADC);
    ADC_ClearFlag(GPIO_VCC_ADC, ADC_FLAG_EOC);
    BuffNext(&gVccBuff, cValue);
    float cVccValueAverage = GetAverageVcc();
    switch (gVccBuff.State) {
    case vsNone:
    case vsFaultLow:
      if (cVccValueAverage > VOLT_HIGH_LEVEL) {
        gVccBuff.State = vsNormal;
        SetSrcPowerLedState();
      } else {
        if (gVccBuff.State != vsFaultLow) {
          gVccBuff.State = vsFaultLow;
          if (UseVccValue()) { //Действовать по Vcc
            SetSrcPowerLedState();
          }
        }
      }
      break;
    case vsNormal:
      if (cVccValueAverage < VOLT_LOW_LEVEL) {
        gVccBuff.State = vsFaultLow;
        if (UseVccValue()) { //Действовать по Vcc
          SetSrcPowerLedState();
        }
      }
#ifdef USE_HIGH_BORDER //Использовать верхнюю границу напряжения
      if (cVccValueAverage > VOLT_UP_HIGH_LEVEL) {
        gVccBuff.State = vsFaultHigh;
        if (UseVccValue()) { //Действовать по Vcc
          SetSrcPowerLedState();
        }
      }
#endif
      break;
#ifdef USE_HIGH_BORDER //Использовать верхнюю границу напряжения
    case vsFaultHigh:
      if (cVccValueAverage < VOLT_UP_LOW_LEVEL) {
        gVccBuff.State = vsNormal;
        SetSrcPowerLedState();
      } else {
        if (gVccBuff.State != vsFaultHigh) {
          gVccBuff.State = vsFaultHigh;
          if (UseVccValue()) { //Действовать по Vcc
            SetSrcPowerLedState();
          }
        }
      }
      break;
#endif
    default:
      break;
    }
    //
    ProcessVccValue(&VccBlockVolt, cVccValueAverage);
    ProcessVccValue(&VccBlockZone, cVccValueAverage);
  }
}

float gVccValueAverage = 0;

float GetAverageVcc() {
  if (gVccBuff.Disabled)
    return 0.0;
  float cfValue = fBuffAverage(&gVccBuff);
  gVccValueAverage = cfValue;
  if (cfValue < VOLT_MIN_CTRL_LEVEL) //Не действовать по Vcc
    return 0.0;
  return cfValue;
}

enum eValState GetVccZone() {
  return VccBlockZone.aValLevelDesc[VccBlockZone.CurrLevelDescIdx].ValState;
}

bool IsVccNormal() {
  if (!UseVccValue()) //Не действовать по Vcc
    return true;
  return ((gVccBuff.State == vsNormal) || (gVccBuff.State == vsFaultHigh));
}

//Контроль Uпит
#ifdef USE_STP
  #include "stp.h"
#endif
enum eValState cCurrVccState = vsNone;
bool CheckVccRight(bool iSetPortEnable) {
  ProcessVccADCValue();
  if (gVccBuff.State != cCurrVccState) {
    if (iSetPortEnable) {
#ifdef USE_STP
      setPortsEnable(IsVccNormal());
#endif
      cCurrVccState = gVccBuff.State;
    } else {
      return (IsVccNormal());
    }
  }
  if (!IsVccNormal()) {
    if (!UseVccValue()) //Не действовать по Vcc
      return true;
    return false;
  }
  return true;
}

