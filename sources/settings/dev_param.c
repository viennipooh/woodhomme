//******************************************************************************
// Имя файла    :       dev_param.c
// Заголовок    :
// Автор        :       Сосновских А.А./ Дмитриев И.В.
// Дата         :       18.03.2020 / 2024
//
//------------------------------------------------------------------------------
/**
Настройка параметров для WD
**/

#include "dev_param.h"
#include "settings.h"

#if (UTD_M != 0)
struct sCtrlPin gCtrlPinWdRestart = {
  .GPIO_Port = GPIOE,
  .PinIdx = 4
};
#endif
#if (IIP != 0)
struct sCtrlPin gCtrlPinWdRestart = {
  .GPIO_Port = GPIOE,
  .PinIdx = 0
};
#endif

#if (PIXEL != 0)
struct sCtrlPin gCtrlPinWdRestart = {
  .GPIO_Port = GPIOE,
  .PinIdx = 3
};
#endif


struct sCtrlPin gCtrlPinPwrMicrotic = {
  .GPIO_Port = GPIOE,
  .PinIdx = 5
};

#if (UTD_M != 0)
struct sCtrlPin gCtrlPinWifiPwr[2] = {
  {
    .GPIO_Port = GPIOE,
    .PinIdx = 9
  },
  {
    .GPIO_Port = GPIOE,
    .PinIdx = 10
  },
};
#endif

void InitCtrlPin(struct sCtrlPin * ipCtrlPin) {
  GPIO_INIT(ipCtrlPin->GPIO_Port, ipCtrlPin->PinIdx, MODE_OUT, OTYPE_PP, OSPEED_VHS, PUPD_N, 0, 0);
}

void SetCtrlPin(struct sCtrlPin * ipCtrlPin, bool iOn) {
  if(iOn) {
    GPIO_SetBits(ipCtrlPin->GPIO_Port, ToGPIO_Pin(ipCtrlPin->PinIdx));
  } else {
    GPIO_ResetBits(ipCtrlPin->GPIO_Port, ToGPIO_Pin(ipCtrlPin->PinIdx));
  }
}


void InitDevParam() {

  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);
#if (PIXEL != 0)
  GPIO_INIT(GPIOE, 3, MODE_OUT, OTYPE_PP, OSPEED_VHS, PUPD_N, 0, 0); // ANT_SW1

#endif

  InitCtrlPin(&gCtrlPinWdRestart); // WD_Restart

  SetCtrlPin(&gCtrlPinWdRestart, true);
}

//Управление WD
#include "log.h"
void SetWD() {
  static bool cIsSet = true;
  static time_t sSysTick = 0;
  time_t cSysTick = GetCurrMsecs();
  int cDiff = cSysTick - sSysTick;
  if (cIsSet) {
    if (cDiff >= WD_PULSE_LEN) {
      SetCtrlPin(&gCtrlPinWdRestart, false);
      cIsSet = false;
      sSysTick = cSysTick;
    }
  } else {
    if (cDiff >= (WD_INTERVAL - WD_PULSE_LEN)) {
      SetCtrlPin(&gCtrlPinWdRestart, true);
      cIsSet = true;
      sSysTick = cSysTick;
    }
  }
}
