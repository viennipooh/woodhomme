//******************************************************************************
// Имя файла    :       log.h
// Заголовок    :       
// Автор        :       Сосновских А.А.
// Дата         :       10.12.2018
//
//------------------------------------------------------------------------------
/**
Логирование событий МКПШ

**/

#ifndef __LOG_H
#define __LOG_H

#include "stdint.h"
#include "stm32f4xx_flash.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"


#include <time.h>

#include "extio.h"

//Обнуление памяти событий при старте программы
//#define EVENT_MEMORY_ERASE

//Часовой пояс (Томск, Кемерово)
#define TIME_ZONE       (7)

//Тестовый режим блока времени / событий (для отладки)
//#define TIMEBLOCK_TEST

#define EVENT_SECTORS_NUMBER    (3) //3 сектора по 16 кБайт
#define EVENT_SECTOR_SIZE       (0x4000)
#define EVENT_BLOCK_SIZE        (EVENT_SECTOR_SIZE * EVENT_SECTORS_NUMBER)

#if ((MKPSH10 != 0) || (IMC_FTX_MC != 0))
  #define EVENT_START_SECTOR      (12)
#endif
#if ((UTD_M != 0) || (IIP != 0))
  #define EVENT_START_SECTOR      ( 1)
#endif

#define FLASH_WORD_SIZE (32)
#ifdef TIMEBLOCK_TEST
  #define FLASH_TIMEBLOCK_SIZE  (0x10000) //64 кБайт
  #define FLASH_TIMEBLOCK_START CCMDATARAM_BASE
#else
  #define FLASH_TIMEBLOCK_SIZE  (0x20000) //128 кБайт
  #define FLASH_TIMEBLOCK_START (0x8120000)
  #define FLASH_SYSBLOCK_START  (0x8100000) //Первые 3 сектора 2-го банка
#endif

#define CODE_ADDRESS_UPL_FLASH  (0x08140000)

enum eEventType { //Тип (класс) события
  etCommon,   //Общие события
  etSys,      //Системное событие. События температуры кристалла
  etDevice,   //On / Off (Вкл/Выкл устройства)
  etPower,    //События питания
  etSetting,  //События изменения значения уставки
  etCount     //Количество типов
};

enum eEventCode { //Код события
  ecDeviceStart     ,  //Пуск устройства                        0
  ecDeviceStop      ,  //Останов устройства
  ecVoltageLess     ,  //Напряжение меньше                      2
  ecVoltageMore     ,  //Напряжение больше
  ecSetSysTime      ,  //Установка системного времени
  ecSetSysTimeNtp   ,  //Установка системного времени NTP протоколом
  ecParamChange     ,  //Изменение значения уставки
  ecParamChangePort ,  //Изменение значения уставки порта [имя, значение]
  ecTestSysTime     ,  //Проверка системного времени NTP протоколом
  ecUploadFwCode    ,  //Загрузка обновления ПО (версия, дата)
  ecTempLess        ,  //Температура меньше
  ecTempMore        ,  //Температура больше
  ecLinkErrPort     ,  //Пересброс порта по потере связи
  ecLinkErrSwitch   ,  //Пересброс свитча по потере связи
  ecLinkErrReboot   ,  //Перезагрузка по потере связи
  ecLinkOn          ,  //Порт х: Есть связь
  ecLinkOff         ,  //Порт х: Нет связи
  ecRstpOn          ,  //Порт х: RSTP Вкл.
  ecRstpOff         ,  //Порт х: RSTP Выкл.
  ecCount
};

//Перечисления для записи событий изменения уставок
enum ePortName { //Имена портов (Eth и RS485)
  pnF_Eth1  = 0,
  pnF_Eth2  = 1,
  pnF_Eth3  = 2,
  pnF_Eth4  = 3,
  pnEth5    = 4,
  pnEth6    = 5,
  pnEth7    = 6,
  pnRS485_1 = 10,
  pnRS485_2 = 11,
  pnCount
};

enum eSettingName { //Имена уставок (Eth и RS485)
  //Включен 	Авто 	Скорость 	Дуплекс 	Упр. потоком 	MDI/MDIX
  //Для каналов Eth
  snIsOn        =  0,
  snSpeedAuto   =  1, //Вкл/Выкл
  snSpeed       =  2, //100/10
  snDuplex      =  3, //FDX/HDX
  snFlowCtrl    =  4, //Вкл/Выкл
  snMdiMdx      =  5, //Auto/Mdi/Mdx
  //Для каналов RS485
  snParity      = 10, //Паритет
  snStopBits    = 11, //Число стоп бит
  snCount
};

typedef struct sEventDef {
  enum eEventCode   Code;
  uint8_t   IsValue8  :1; //Есть значение Value8
  uint8_t   IsValue   :1; //Есть значение Value (32)
  uint8_t   IsValue1  :1; //Есть значение Value(0-15) (16)
  uint8_t   IsValue2  :1; //Есть значение Value(16-31) (16)
} SEventDef;

#include <ip_addr.h>

#pragma pack(1)
//Структура события (12 байт)
#define WORDS_IN_EVENT  (3)
typedef struct sEvent {
  union {
    uint32_t      aEvent[WORDS_IN_EVENT];  //Событие как массив слов
    struct {
      uint32_t    Time;      //Время события
      uint16_t    Code;      //Код сообщения
      uint8_t     Type;      //Тип сообщения
      int8_t      Value8;    //Значение параметра в диапазоне -127 .. +127
      int32_t     Value;     //Значение параметра события
    };
  };
} SEvent;

enum eOper { //Код операции с Flash
  ocWriteEvent  ,  //Запись события
  ocWriteWord   ,  //Запись слова
  ocEraseEvent  ,  //Очистка сектора событий
  ocErase       ,  //Очистка сектора (абсолютного, в виде FLASH_Sector_XX)
  ocSaveSettings,  //Сохранение уставок во Flash
  ocCount
};

typedef struct sEventItem { //Элемент очереди операций с Flash
  enum eOper  Oper;
  uint32_t    Addr;
  struct sEvent Event;
} SEventItem;

#pragma pack()

//Для проверки NTP-коррекции
typedef struct sCorrParam { //Уставки службы времени
  int32_t  Int;
  int32_t  CorrNew;
  int32_t  Dt;
  int32_t  TimeCorr;
  int32_t  CorrToSave;
} SCorrParam;

#include "ntp_settings.h"
extern int aPeriodKindValue[pkCount];

extern xQueueHandle gQueueFlash;
extern void ProcessTick();
extern time_t GetCurrTime();
extern void CreateTaskLog();
extern void EraseLogFlash();
extern void SaveEvent(time_t iTime, enum eEventType iType, enum eEventCode iCode);
extern void SaveFloatEvent(enum eEventType iType, enum eEventCode iCode,
                           float iValue, int8_t iThreshold);
extern void SaveIntEvent(enum eEventType iType, enum eEventCode iCode,
                         int8_t iValue8, int32_t iValue);
extern void SaveSettingEvent(enum eEventType iType, enum eEventCode iCode,
                             enum ePortName iPortName,
                             enum eSettingName iSettingName, int iValue);
extern void SaveDateEvent(enum eEventType iType, enum eEventCode iCode,
                          int8_t iValueH, int16_t iValueL,
                          int8_t iYear, int8_t iMonth, int8_t iDay);
extern void RestartSystem(uint32_t iStartAddr);
extern time_t GetCurrMsecs();

#endif //__LOG_H
