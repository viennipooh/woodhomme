//******************************************************************************
// Имя файла    :       adc_ctrl.c
// Заголовок    :       
// Автор        :       Сосновских А.А.
// Дата         :       24.06.2019
//
//------------------------------------------------------------------------------
/**
Контроль значения АЦП (ADC)

**/

#include "stddef.h"
#include "adc_ctrl.h"

float fBuffAverage(struct sAdcBuff * ipAdcBuff) {
  float cfValue = 0;
  uint8_t cMaxIdx = ipAdcBuff->CurrIdx;
  if (ipAdcBuff->Full)
    cMaxIdx = BUFF_SIZE;
  for (int i = 0; i < cMaxIdx; ++i) {
    cfValue += ipAdcBuff->afAdcValue[i];
  }
  if (cMaxIdx > 0)
    cfValue /= cMaxIdx;
  return cfValue;
}

uint16_t uBuffAverage(struct sAdcBuff * ipAdcBuff) {
  uint32_t cValue = 0;
  uint8_t cMaxIdx = ipAdcBuff->CurrIdx;
  if (ipAdcBuff->Full)
    cMaxIdx = BUFF_SIZE;
  for (int i = 0; i < cMaxIdx; ++i) {
    cValue += ipAdcBuff->aAdcValue[i];
  }
  if (cMaxIdx > 0)
    cValue /= cMaxIdx;
  return cValue;
}

void BuffInit(struct sAdcBuff * ipAdcBuff) {
  ipAdcBuff->CurrIdx = 0;
  ipAdcBuff->Full = false;
  ipAdcBuff->State = vsNone;
}

void BuffNext(struct sAdcBuff * ipAdcBuff, uint16_t iValue) {
  float cfValue = iValue;
  if (ipAdcBuff->CalcValueCB != NULL) {
    cfValue = ipAdcBuff->CalcValueCB(iValue);
  }
  ipAdcBuff->afAdcValue[ipAdcBuff->CurrIdx] = cfValue;
  ipAdcBuff->aAdcValue[ipAdcBuff->CurrIdx++] = iValue;
  if (ipAdcBuff->CurrIdx >= BUFF_SIZE) {
    ipAdcBuff->CurrIdx = 0;
    ipAdcBuff->Full = true;
  }
}


