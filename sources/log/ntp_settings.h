//******************************************************************************
// Имя файла    :       ntp_settings.h
// Заголовок    :       
// Автор        :       Сосновских А.А.
// Дата         :       25.04.2019
//
//------------------------------------------------------------------------------
/**
Структура уставок NTP

**/

#ifndef __NTP_SETTINGS_H
#define __NTP_SETTINGS_H

enum eTimeCorrMode { //Режим коррекции времени
  tcmManual,  //Ручной (через Web-интерфейс)
  tcmNtpAuto, //Автоматический (NTP протоколом)
  tcmCount
};

//Вид периода коррекции: Минут, Часов, Суток, Недель
// При изменении: корректировать:
// - time.htm : INPUT name=period_num
// - log.c : aPeriodKindValue
enum ePeriodKind {
  pkMinute, pkHour, pkDay, pkCount
};

typedef struct sNtpSettings { //Уставки службы времени
  ip_addr_t           IpAddr;
  enum eTimeCorrMode  TimeCorrMode;
  int8_t              TimeZone;
  int                 PeriodNum;
  enum ePeriodKind    PeriodKind;
  
} SNtpSettings;


#endif //__NTP_SETTINGS_H
