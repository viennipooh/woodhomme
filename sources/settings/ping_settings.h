//******************************************************************************
// Имя файла    :       ping_settings.h
// Заголовок    :       
// Автор        :       Сосновских А.А.
// Дата         :       16.07.2019
//
//------------------------------------------------------------------------------
/**
Структура уставок контроля связи

**/

#ifndef __PING_SETTINGS_H
#define __PING_SETTINGS_H

#include <stdbool.h>
#if !NO_SYS
  #include "semphr.h"
#endif

// - ping.htm : INPUT name=period_num

typedef struct sPingSettings { //Уставки контроля связи
  bool        CtrlIsOn;       //Режим контроля включен
  ip_addr_t   IpAddr;         //IP-адрес для пингования
  int         StartAfterBoot; //Интервал запуска контроля после загрузки (мин)
  int         CtrlInterval;   //Интервал между проверками (сек)
  int         PingNum;        //Количество пингов в пакете проверки
  int         PingErrNum;     //Количество ошибочных проверок для перезагрузки
} SPingSettings;

//Объект WD связи
typedef struct sPingCtrl { //
  int       StartTimeout;
  int       StepTimeout;
  int8_t    PingCount;        //Количество пингов в пакете проверки
  int8_t    PingResultCount;
  int8_t    PingErrorCount;
  bool      PingOn;         //Контроль связи включен
  bool      Checked;        //Проверка была выполнена как минимум 1 раз
  uint8_t   MAC[6];         //MAC-адрес устройства пингования
  uint16_t  ping_seq_num;
#if !NO_SYS
  xSemaphoreHandle Mutex; //
#endif
  struct sPingSettings * pPingSettings;
  uint32_t  PortEventCount;
  uint32_t  SwitchEventCount;
} SPingCtrl;

#endif //__PING_SETTINGS_H
