//******************************************************************************
// ��� �����    :       log.c
// ���������    :       
// �����        :       ���������� �.�.
// ����         :       10.12.2018
//
//------------------------------------------------------------------------------
/**
����������� ������� �����������
**/

/* Scheduler includes */

#include "log.h"
#include "string.h"
#include "stdbool.h"

#include "settings.h"
#include "ntp_conv.h"

#define LOG_TASK_DELAY (200)

int aPeriodKindValue[pkCount] = {
  60, 3600, 86400
};

//������� ����� � ���.
portTickType xTaskGetTickCount( void );
uint64_t xTaskGetTickCountL( void );

xQueueHandle gQueueFlash;
//������������� ����� LOG
void log_task(void * pvParameters);

void CreateTaskLog() {
  //��������� ������, ��� RSTP
  xTaskCreate(log_task, (int8_t *) "Log", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 3, NULL);
  
  gQueueFlash = xQueueCreate(10, sizeof(struct sEventItem));
}

#include <stdio.h>

#include "log_flash.h"
bool IsSectorEmpty(uint8_t iSectIdx);

void EraseLogFlash() { //��������� ������ �������
  for (int i = 0; i < EVENT_SECTORS_NUMBER; ++i) {
    if (!IsSectorEmpty(i)) {
      EraseSectorArea(EVENT_START_SECTOR + i, 1);
      vTaskDelay(1000);
    }
  }
}

volatile int8_t gEraseSectorIdx = -1; //���� >= 0, ��: ������� � ������� ��� �� ���������� ������� �������� �������
volatile int8_t gEventCmdCount = 0;  //���������� � ������� ��� �� ���������� ������ ������ �������

//!!! ���������� �������� ��� ������ � gEraseSectorIdx, gEventCmdCount
/*		if( xSemaphoreTake( *sem, timeout / portTICK_RATE_MS ) == pdTRUE )
		{
xSemaphoreGive(*sem);
*/
void ProgramWord(uint32_t iAddr, uint32_t iWriteWord) {
  uint32_t * cpWordAddr = (uint32_t *)iAddr;
  *cpWordAddr = iWriteWord;
}

time_t gStartTime; //����� ������ ����������
time_t gStartSeconds = 0; //������ ������ � ������ gStartTime
uint64_t gStartMsecs = 0; //������ ���������� � ������ gStartTime
//�������: ����� ����������� (Web, SNMP, ��� ����������)
bool gbTimeIsAssigned = false;

//��������� ��� ��������� ������� �������
#define CORR_BASE (1000000) //���� ���������

uint64_t CurrTimeInMsecs() { //����� ������ ���������� � ������������
  uint32_t cTicks = xTaskGetTickCount();
  uint64_t cTicksL = xTaskGetTickCountL();
//  cTicksL = 285; //��� �������� ������� �������� (> 49 �����)
  cTicksL <<= 32;
  cTicksL |= cTicks;
  //��������� �� ������������
  cTicksL *= CORR_BASE;
  cTicksL /= (CORR_BASE - rsettings->TimeCorr);
  return cTicksL;
}

//������� ����� � ��������
uint32_t CurrTimeInSecs() { //����� ������ ���������� � ��������
  uint64_t cTicksL = CurrTimeInMsecs();
  cTicksL /= 1000;
  uint32_t cTicks = (cTicksL & 0xffffffff);
  return cTicks;
}


time_t GetCurrSeconds() { //����� �� ���������� ��������� (�����, ���.�������)
  return CurrTimeInSecs() - gStartSeconds;
}

time_t GetCurrMsecs() { //����� �� ���������� ��������� (�����, ���.�������)
  return CurrTimeInMsecs() - gStartMsecs;
}

time_t GetCurrTime() { //������� ���������� ����� �� UNIX-���������
  return gStartTime + GetCurrSeconds();
}

uint64_t GetCurrMsecsTime() { //������� ���������� ����� �� UNIX-��������� � ����
  if (gStartTime > 0) {
    uint64_t cMsecs = gStartTime;
    cMsecs *= 1000;
    cMsecs += GetCurrMsecs();
    return cMsecs;
  } else {
    return 0;
  }
}

uint64_t GetRefMsecsTime() { //��������� ���������� ����� �� UNIX-��������� � ����
  if (gStartTime > 0) { // (��������� ����� ������� �������)
    uint64_t cMsecs = gStartTime;
    cMsecs *= 1000;
    return cMsecs;
  } else {
    return 0;
  }
}

// !!! ��������� ������� � ���������.

uint32_t gStartSector = 0;  //������ ���������� �������

uint32_t gCurrEvent = 0;    //������ �������� ������� (����������, �� ������ �����)
uint32_t gStartEvent = 0;   //������ ���������� ������� (����������, �� ������ �����)

struct sEventDef aEventDef[ecCount] = { //����������� �������
  {ecDeviceStart    , 0, 0, 0, 0},  //���� ����������
  {ecDeviceStop     , 0, 0, 0, 0},  //������� ����������
  {ecVoltageLess    , 0, 1, 0, 0},  //���������� ������
  {ecVoltageMore    , 0, 1, 0, 0},  //���������� ������
  {ecSetSysTime     , 0, 1, 0, 0},  //��������� ���������� �������
  {ecSetSysTimeNtp  , 0, 1, 0, 0},  //��������� ���������� ������� NTP ����������
  {ecParamChange    , 1, 0, 1, 1},  //��������� �������� ������� (�������, ����, �����)
  {ecParamChangePort, 1, 0, 1, 1},  //��������� �������� ������� ����� [���, ��������]
  {ecTestSysTime    , 1, 0, 0, 0},  //�������� �������� ������� NTP ����������
};

#ifdef TIMEBLOCK_TEST
  #undef FLASH_TIMEBLOCK_START
  #undef FLASH_TIMEBLOCK_SIZE
  #define FLASH_TIMEBLOCK_SIZE  (0x200)
#endif

#define FLASH_TIMEBLOCK_ITEMSIZE  (FLASH_TIMEBLOCK_SIZE >> 2)

#ifdef TIMEBLOCK_TEST
  uint32_t taTimeBuff[FLASH_TIMEBLOCK_ITEMSIZE] @ ".sram";
  #define FLASH_TIMEBLOCK_START (uint32_t)(&taTimeBuff);
#endif

struct sEvent gClearEvent = { CLEAR_WORD, CLEAR_WORD, CLEAR_WORD };
struct sEvent gTimedEvent = {
  .Time   = 0x00ffffff,   //����� �������
  .Code   = ecDeviceStop, //��� ���������
  .Type   = etSys,        //��� ���������
  .Value8 = 0,            //�������� ��������� � ��������� 0 .. 255
  .Value  = 0             //�������� ��������� �������
};

#ifdef TIMEBLOCK_TEST
void FillTimeBuff(int32_t iCurrWord, int32_t iCurrValue) {
  for (int i = 0; i < FLASH_TIMEBLOCK_ITEMSIZE; ++i) {
    if (i < iCurrWord) {
      taTimeBuff[i] = 0;
    } else if (i > iCurrWord) {
      taTimeBuff[i] = CLEAR_WORD;
    } else {
      taTimeBuff[i] = iCurrValue;
    }
  }
}

  #undef FLASH_SYSBLOCK_START
  #undef EVENT_BLOCK_SIZE
  #define EVENT_BLOCK_SIZE  (0x80 * sizeof(struct sEvent)) //16 �������
#endif

#define FLASH_SYS_SECT_SIZE  (EVENT_BLOCK_SIZE / 3) //128 * 12 / 3 = 512 ���� = 128 ����
//
#define FLASH_SYSBLOCK_ITEMSIZE  (EVENT_BLOCK_SIZE / sizeof(struct sEvent))
#define FLASH_SYSBLOCK_WORDSIZE  (EVENT_BLOCK_SIZE >> 2)

#ifdef TIMEBLOCK_TEST
  struct sEvent taSysEventBuff[FLASH_SYSBLOCK_ITEMSIZE] @ ".sram";
  #define FLASH_SYSBLOCK_START  (uint32_t)(taSysEventBuff);

void FillEventBuff(int32_t iCurrEvent, bool iFillRest) {
  enum eEventCode cEventCode = ecVoltageLess; //ecDeviceStart;
  float cValue = 8.5;
  int32_t * cpValue = (int32_t *)&cValue;
  time_t cTime = GetCurrTime();
  if (iFillRest)
    gTimedEvent.Time = cTime;
  for (int i = 0; i < FLASH_SYSBLOCK_ITEMSIZE; ++i) {
    if (i < iCurrEvent) { // <
      memcpy((void *)&taSysEventBuff[i], (void *)&gTimedEvent, sizeof(struct sEvent));
      taSysEventBuff[i].Code = cEventCode;
      if ((cEventCode == ecVoltageLess) || (cEventCode == ecVoltageMore)) {
        taSysEventBuff[i].Type = etPower;
        if (cValue >= 16.0)
          cValue = 8.5;
        else
          cValue += 1.0;
        taSysEventBuff[i].Value = *cpValue;
      }
      if ((cEventCode == ecDeviceStart) || (cEventCode == ecDeviceStop)) {
        taSysEventBuff[i].Type = etDevice;
      }
      if ((cEventCode == ecParamChange) || (cEventCode == ecParamChangePort)) {
        taSysEventBuff[i].Type = etSetting;
      }
    } else if (i > iCurrEvent) { // >
      if (iFillRest)
        memcpy((void *)&taSysEventBuff[i], (void *)&gClearEvent, sizeof(struct sEvent));
    } else { // =
      if (iFillRest)
        memcpy((void *)&taSysEventBuff[i], (void *)&gClearEvent, sizeof(struct sEvent));
    }
    ++cEventCode;
    gTimedEvent.Time += 90500;
    if (cEventCode == ecCount)
      cEventCode = ecDeviceStart;
  }
}
#endif

void EventSectorDirectErase(int8_t iSectIdx) {
#ifdef TIMEBLOCK_TEST
  uint32_t cStart = iSectIdx;
  cStart *= FLASH_SYS_SECT_SIZE;
  cStart += FLASH_SYSBLOCK_START;
  memset((void *)cStart, 0xff, FLASH_SYS_SECT_SIZE);
#else
  EraseSectorArea(EVENT_START_SECTOR + iSectIdx, 1); //
#endif
}

void EventDirectWrite(uint32_t iAddr, struct sEvent * ipEvent) {
#ifdef TIMEBLOCK_TEST
  for (int i = 0; i < 3; ++i) {
    ProgramWord(iAddr + (i * sizeof(uint32_t)), ipEvent->aEvent[i]);
    if (i < 2)
      vTaskDelay(300);
    else
      vTaskDelay(100);
  }
#else
  Flash_Program_Event(iAddr, ipEvent);
#endif
  if (gEventCmdCount > 0)
    --gEventCmdCount;
}

#include "fw.h"
#include "switch.h"
#include "eth.h"
#include "eth_mac.h"
extern eth_mac ETH_MAC_(0);
#include "info.h"
#ifdef USE_SWITCH //������������ ����� kmz8895
  extern int switch_off(int n);
#endif
#if ((UTD_M != 0) || (IIP != 0))
  #include "dev_param.h"
#endif
extern void dma_dealloc_nvic_all();
extern void settings_save_to_flash();
extern void mode_timer_deinit();
extern void usart_irq_disable_all();
extern int at45db_deinit( int dev );
extern void IWDG_CounterRefresh(uint16_t iCounterIn_mSecond);
void RestartSystem(uint32_t iStartAddr) {
#if (UTD_M != 0)
  SetPwrMicrotic(false);
#endif
  mode_timer_deinit(); //����. �������: ������� ���������� ��
  usart_irq_disable_all();
  dma_dealloc_nvic_all();
  ETH_MAC_(0).uninitialize();
#ifdef USE_SWITCH //������������ ����� kmz8895
  switch_off(0);
 #ifdef TWO_SWITCH_ITEMS
  switch_off(1);
 #endif
  switch_deinit();
 #if ((UTD_M == 0) && (IIP == 0)) //��� ����������� �� UTD_M
  at45db_deinit(1);
 #endif
#endif
  vTaskEndScheduler(); //���������� ������ FreeRTOS
  IWDG_CounterRefresh(32700);
  //����� ����������
  boot_code(iStartAddr);
}

extern bool gSwitchStarted;
//������� � ����� ������� �����
uint32_t gLastTick = 0;     //�������� ���������� ���� (������)
uint32_t gCurrWord = 0;     //������ �������� ����� � ����� �������
uint8_t gCurrBit = 0;       //����� �������� ���� � ������� ����� (gCurrWord)

extern void __reboot__();
void log_task(void * pvParameters) {
  struct sEventItem cEventItem;
  vTaskDelay(2000);
//  Flash_Program_Init(); //������������� Mutex (?)
#ifdef EVENT_MEMORY_ERASE
  EraseLogFlash(); //��������� ������ �������
#endif
  while (1) {
    vTaskDelay(LOG_TASK_DELAY);
    if (pdTRUE == xQueueReceive(gQueueFlash, &cEventItem, 5)) {
      switch (cEventItem.Oper) {
      case ocWriteEvent:
        if ((cEventItem.Event.Type == etCount) || (cEventItem.Event.Code == ecCount)) {
          vTaskDelay(LOG_TASK_DELAY);
          IWDG_CounterRefresh(32700);
#ifdef USE_SDRAM
          RestartSystem(FW_LOADER_START);
#else
          __reboot__();
#endif
        } else { //������ �������
          EventDirectWrite(cEventItem.Addr, &cEventItem.Event);
        }
        break;
      case ocWriteWord:
#ifdef TIMEBLOCK_TEST
        ProgramWord(cEventItem.Addr, cEventItem.Event.aEvent[0]);
#else
        Flash_Program_Word(cEventItem.Addr, cEventItem.Event.aEvent[0]);
#endif
        break;
      case ocEraseEvent:
        EventSectorDirectErase(cEventItem.Addr);
        break;
      case ocErase: //�������� ������ (FLASH_Sector_XX)
#ifdef TIMEBLOCK_TEST
        FillTimeBuff(-1, CLEAR_WORD);
#else
        Flash_Erase_Sector(cEventItem.Addr);
#endif
        if (cEventItem.Addr == FLASH_Sector_17) { //��� ������ ������� �����
#ifdef TIMEBLOCK_TEST
          gLastTick = GetCurrSeconds() / 5;
#else
          gLastTick = GetCurrSeconds() / 60;
#endif
          gCurrBit  = 0;
          gCurrWord = 0;
        }
        if (gEraseSectorIdx >= 0)
          gEraseSectorIdx = -1;
        break;
      case ocSaveSettings: //���������� ������� �� Flash
        settings_save_to_flash();
        gSwitchStarted = true;
        break;
      default:
        break;
      }
    }
  }
}

struct sEvent * GetSysBlockAddr() {
  return (struct sEvent *)FLASH_SYSBLOCK_START;
}

int32_t GetStartEvent() { //��������� ������� (������)
  return gStartEvent;
}

int32_t GetCurrEvent() { //������� ������� (������)
  return gCurrEvent;
}

int32_t GetEventCount() { //���������� �������
  int32_t cEventCount = gCurrEvent - gStartEvent;
  if (cEventCount < 0)
    cEventCount += FLASH_SYSBLOCK_ITEMSIZE;
  return cEventCount;
}

struct sEvent * GetEventByIdx(int32_t iEventIdx) {
  if (iEventIdx < 0)
    return NULL;
  int32_t cEvent = gStartEvent + iEventIdx;
  struct sEvent * cpCurrBase = GetSysBlockAddr();
  if (cEvent < FLASH_SYSBLOCK_ITEMSIZE) {
    return &cpCurrBase[cEvent];
  } else {
    cEvent -= FLASH_SYSBLOCK_ITEMSIZE;
    if (cEvent >= 0)
      return &cpCurrBase[cEvent];
  }
  return NULL;
}

void Int2ToStr(char * oStr, int8_t iValue) {
  if (iValue < 10)
    sprintf(oStr, "0%d", iValue);
  else
    sprintf(oStr, "%d", iValue);
}

void GetDateString(char * oStr, uint8_t iYear, uint8_t iMonth, uint8_t iDay) {
  char cStr[2][5];
  Int2ToStr(cStr[0], iMonth);
  Int2ToStr(cStr[1], iDay);
  sprintf(oStr, "%d.%s.%s", iYear + 1900, cStr[0], cStr[1]);
}

void GetDateTimeString(char * oStr, time_t iTime) {
  struct tm * cpTmTime = localtime(&iTime);
  char cStr[4][5];
  Int2ToStr(cStr[0], cpTmTime->tm_mon + 1);
  Int2ToStr(cStr[1], cpTmTime->tm_mday);
  Int2ToStr(cStr[2], cpTmTime->tm_hour);
  Int2ToStr(cStr[3], cpTmTime->tm_min);
  sprintf(oStr, "%d.%s.%s %s:%s",
          cpTmTime->tm_year + 1900,
          cStr[0],
          cStr[1],
          cStr[2],
          cStr[3] );
}

bool IsSectorEmpty(uint8_t iSectIdx) {
  if (iSectIdx < 3) {
    uint32_t cStart = (iSectIdx * FLASH_SYS_SECT_SIZE) + FLASH_SYSBLOCK_START;
    uint32_t * cpStart = (uint32_t *)cStart;
    bool cIsEmpty = (*cpStart == CLEAR_WORD);
    if (!cIsEmpty) {
      if (gEraseSectorIdx == iSectIdx) 
        cIsEmpty = true;
    }
    return cIsEmpty;
  }
  return false;
}

//���������� ������� �� Flash
void SaveSettingsToFlash() {
  struct sEventItem cEventItem;
  cEventItem.Oper = ocSaveSettings;
  cEventItem.Addr = 0;
  if( xQueueSend( gQueueFlash, &cEventItem, 50 ) == pdPASS ) {
  }
}

//������� ������� �������: ������ 0, 1, ��� 2
void EraseEventSector(uint8_t iSectIdx) {
  if (iSectIdx < 3) {
    struct sEventItem cEventItem;
    cEventItem.Oper = ocEraseEvent;
    cEventItem.Addr = iSectIdx;
    if( xQueueSend( gQueueFlash, &cEventItem, 50 ) == pdPASS ) {
      gEraseSectorIdx = iSectIdx;
    }
  }
}

void EraseTimeSector() { //�������� ������ �������
  struct sEventItem cEventItem;
  cEventItem.Oper = ocErase;
  cEventItem.Addr = FLASH_Sector_17;
  if( xQueueSend( gQueueFlash, &cEventItem, 50 ) == pdPASS ) {

  }
}

// !!! ��������� �������. �����

int32_t FindSetLastMinute() { //����� � ��������� ��������� ������
  uint32_t * cCurrWord, * cCurrBase, cCurrDiv;
  uint32_t cCurrValue = CLEAR_WORD;
  cCurrWord = (uint32_t *)FLASH_TIMEBLOCK_START;
  if (*cCurrWord == CLEAR_WORD) {
    gCurrWord = 0;
    gCurrBit  = 0;
    return 0;
  }
  cCurrBase = cCurrWord;
  cCurrDiv = (FLASH_TIMEBLOCK_ITEMSIZE >> 1);
  cCurrWord += (cCurrDiv);
  cCurrValue = *cCurrWord;
  while ((cCurrValue == CLEAR_WORD) || (cCurrValue == 0)) { //����� �������� �����
    if (*cCurrWord == CLEAR_WORD) { //� ������ ��������
    } else {
      cCurrBase += (cCurrDiv);
    }
    cCurrDiv >>= 1;
    cCurrWord = cCurrBase + (cCurrDiv);
    cCurrValue = *cCurrWord;
    if (cCurrDiv < 1) {
      break;
    }
  }
  //���� cCurrValue == 0, ��: ����� ����� (������� ����� �� ��� ���������) == ������������ �����
  //������ �������� ����� � �������
  int32_t cWordsNum = cCurrWord - (uint32_t *)FLASH_TIMEBLOCK_START;
  gCurrWord = cWordsNum;
  cWordsNum *= FLASH_WORD_SIZE;
  if (cCurrValue == 0) {
    cWordsNum += FLASH_WORD_SIZE;
    gCurrBit = 32;
  } else {
    for (int i = 0; i < FLASH_WORD_SIZE; ++i) {
      if (cCurrValue & 0x01) {
        cWordsNum += i;
        gCurrBit = i;
        break;
      }
      cCurrValue >>= 1;
    }
  }
  return cWordsNum;
}

int CalcStartEvent();
int32_t FindSetLastEvent() { //����� � ��������� ���������� �������
  struct sEvent * cpCurrEvent, * cpCurrBase;
  int32_t cStartIdx = 0, cEndIdx = (2 * FLASH_SYS_SECT_SIZE) / sizeof(struct sEvent) + 1;
  int32_t cMiddleIdx;
  if (cStartIdx > 0)
    cStartIdx -= 1;
  int8_t cEraseSector = CalcStartEvent();
  switch (cEraseSector) {
  case 0:
    cStartIdx = (1 * FLASH_SYS_SECT_SIZE) / sizeof(struct sEvent) + 1;
    cEndIdx = (3 * FLASH_SYS_SECT_SIZE) / sizeof(struct sEvent) - 1;
    break;
  case 1: //���� 0 ������ ������?
    cStartIdx = 0;
    cEndIdx = (1 * FLASH_SYS_SECT_SIZE) / sizeof(struct sEvent) + 1;
    break;
  case 2:
    cStartIdx = 0;
    cEndIdx = (2 * FLASH_SYS_SECT_SIZE) / sizeof(struct sEvent) + 1;
    break;
  default:
    break;
  }
  cMiddleIdx = (cStartIdx + cEndIdx) >> 1;
  cpCurrEvent = GetSysBlockAddr();
  cpCurrBase = cpCurrEvent;
  while (true) { //����� ������� ������� �������
    if (memcmp(&cpCurrBase[cMiddleIdx], (void *)&gClearEvent, sizeof(struct sEvent)) == 0) { //� ������ ��������
      cEndIdx = cMiddleIdx;
    } else {
      cStartIdx = cMiddleIdx;
    }
    if ((cEndIdx - cStartIdx) > 4) {
      cMiddleIdx = (cStartIdx + cEndIdx) >> 1;
    } else {
      bool cIsMiddle = false;
      for (int i = cStartIdx; i <= cEndIdx; ++i) {
        if (memcmp(&cpCurrBase[i], (void *)&gClearEvent, sizeof(struct sEvent)) == 0) {
          cMiddleIdx = i;
          cIsMiddle = true;
          break;
        }
      }
      if (!cIsMiddle) //��� ������ ������� �� ����� ���������
        cMiddleIdx = cEndIdx + 1;
      break;
    }
  }
  gCurrEvent = cMiddleIdx;
  if (gCurrEvent >= FLASH_SYSBLOCK_ITEMSIZE)
    gCurrEvent = 0;
  return cMiddleIdx;
}

struct tm gBaseTime_tm = {
  .tm_sec    = 0,   //0-61
  .tm_min    = 0,   //0-59
  .tm_hour   = 0,   //0-23
  .tm_mday   = 1,   //1-31
  .tm_mon    = 0,   //0-11
  .tm_year   = 119, //��� - 1900
  .tm_wday   = 1,   //0-6, � �����������
  .tm_yday   = 0,   //0-365
};

int32_t FindLastOnOffEvent() { //����� ��������� ������� ���/����
  struct sEvent * cpCurrEvent, * cpStartEvent = GetSysBlockAddr();
  int32_t cEventCount = GetEventCount();
  for (int i = cEventCount - 1; i >= 0; --i) {
    cpCurrEvent = GetEventByIdx(i);
    if ((cpCurrEvent->Code == ecDeviceStart) ||
        (cpCurrEvent->Code == ecDeviceStop) ||
        (cpCurrEvent->Code == ecSetSysTime) ||
        (cpCurrEvent->Code == ecSetSysTimeNtp)) {
      int cCurrEventIdx = cpCurrEvent - cpStartEvent;
      return cCurrEventIdx;
    }
  }
  return -1;
}

int32_t FindLastOnOffSetTimeEvent() { //����� ��������� ������� ���/����
  struct sEvent * cpCurrEvent, * cpStartEvent = GetSysBlockAddr();
  int32_t cEventCount = GetEventCount();
  for (int i = cEventCount - 1; i >= 0; --i) {
    cpCurrEvent = GetEventByIdx(i);
    if ((cpCurrEvent->Code == ecDeviceStart) ||
        (cpCurrEvent->Code == ecDeviceStop) ||
        (cpCurrEvent->Code == ecSetSysTime) ||
        (cpCurrEvent->Code == ecTestSysTime) ||
        (cpCurrEvent->Code == ecSetSysTimeNtp)) {
      int cCurrEventIdx = cpCurrEvent - cpStartEvent;
      return cCurrEventIdx;
    }
  }
  return -1;
}

void SaveEvent(time_t iTime, enum eEventType iType, enum eEventCode iCode);
void SaveTimeEvent(time_t iNewTime, enum eEventCode iEventCode);

void SetStartTime(time_t iNewCurrTime, enum eEventCode iEventCode) {
  //���������� ���������� �������
  time_t cStartTime = iNewCurrTime - GetCurrSeconds();
  int32_t cEventIdx = FindLastOnOffEvent();
  struct sEvent * cpCurrEvent = GetSysBlockAddr();
  struct tm * cTmTime; //��� �������� ������� (��� �������)
  cTmTime = localtime(&iNewCurrTime); //�������� ������� (������� ����� ������� ���������)
  if (cEventIdx >= 0) {
    //���� ��� ������� ������, ��:
    if (cpCurrEvent[cEventIdx].Code == ecDeviceStop) {
      //  ������ ������� ������
      SaveEvent(cStartTime, etDevice, ecDeviceStart);
      vTaskDelay(200);
    }
    SaveTimeEvent(iNewCurrTime, iEventCode); //������ ������� ��������� �������
    vTaskDelay(200);
    EraseTimeSector();
    gStartTime = iNewCurrTime;
    cTmTime = localtime(&gStartTime); //�������� ������� (������� ����� ������� ���������)
    if (cTmTime->tm_sec > 0) { } //����� cTmTime �� ������ Warning
    gStartMsecs = CurrTimeInMsecs();
    gStartSeconds = CurrTimeInSecs();
  //
    gbTimeIsAssigned = true;
  }
}

int8_t CalcSectorFillCode() { //��� ���������� ��������
  int8_t cCode = 0;
  uint32_t * cpSect = (uint32_t *)FLASH_SYSBLOCK_START;
  for (int i = 0; i < 3; ++i) {
    if (cpSect[i * (FLASH_SYS_SECT_SIZE >> 2)] != CLEAR_WORD) {
      if ((gEraseSectorIdx < 0) || (gEraseSectorIdx != i)) {
        cCode |= (1 << i);
      }
    } else if (gEventCmdCount > 0) {
      uint32_t * cpStart = &cpSect[i * (FLASH_SYS_SECT_SIZE >> 2)];
      uint32_t * cpEnd = &cpSect[i + 1 * (FLASH_SYS_SECT_SIZE >> 2)];
      
      uint32_t * cpCurr = cpSect + (gCurrEvent * WORDS_IN_EVENT);
      if (cpCurr >= (cpSect + FLASH_SYSBLOCK_WORDSIZE))
        cpCurr -= FLASH_SYSBLOCK_WORDSIZE;
      if ((cpCurr > cpStart) && (cpCurr < cpEnd))
        cCode |= (1 << i);
    }
  }
  return cCode;
}

int8_t CalcEraseSector() {
  int8_t cCode = CalcSectorFillCode(), cSect = -1;
  bool cIsDefined = false;
  uint32_t * cpSect = (uint32_t *)FLASH_SYSBLOCK_START;
  if (cCode == 7) {
    for (int i = 0; i < 3; ++i) {
      if (cpSect[i * (FLASH_SYS_SECT_SIZE >> 2) + sizeof(struct sEvent)] == CLEAR_WORD) {
        cSect = (i + 1) % 3;
        cIsDefined = true;
        break;
      } else {
        if (/*(gEraseSectorIdx >= 0) && */(gEraseSectorIdx == i)) {
          cSect = (i + 1) % 3;
          cIsDefined = true;
          break;
        }
      }
    }
    if (!cIsDefined) //��� ������ ��������, ��������� = 0
      cSect = 0;
  }
  return cSect;
}


void WriteNextTick();

int CalcStartEvent() {
  //���������� ������������� �������� �������, � �������� ���
  int8_t cCode = CalcSectorFillCode();
  int8_t cEraseSect = CalcEraseSector();
  if (cEraseSect < 0) {
    switch (cCode) {
    case 0: //������
    case 1: //0 ������
    case 3: //0,1 �������
      cEraseSect = 2;
      break;
    case 2: //1 ������
      cEraseSect = 0;
      break;
    case 6: //1,2 �������
//    case 7: //0,1,2 �������
      cEraseSect = 0;
      break;
    case 4: //2 ������
    case 5: //2, 0 �������
      cEraseSect = 1;
      break;
    default:
      break;
    }
  }
  int8_t cStartSect = (cEraseSect + 1) % 3;
  gStartSector = cStartSect;
  gStartEvent = (cStartSect * FLASH_SYS_SECT_SIZE) / sizeof(struct sEvent);
  if (cStartSect > 0)
    ++gStartEvent;
  return cEraseSect;
}

void WriteEvent(struct sEvent * ipEvent) {
  uint32_t cAddr = FLASH_SYSBLOCK_START;
  cAddr += gCurrEvent * sizeof(struct sEvent);
  struct sEventItem cEventItem;
  memcpy((void *)&cEventItem.Event, (void *)ipEvent, sizeof(struct sEvent));
  cEventItem.Oper = ocWriteEvent;
  cEventItem.Addr = cAddr;
  if( xQueueSend( gQueueFlash, &cEventItem, 50 ) == pdPASS ) {
    ++gEventCmdCount;
    ++gCurrEvent;
    if (gCurrEvent >= FLASH_SYSBLOCK_ITEMSIZE) {
      gCurrEvent = 0;
    }
    //���������� ������������� �������� �������, � �������� ���
    int8_t cEraseSect = CalcEraseSector();
    if (cEraseSect >= 0) {
      if (!IsSectorEmpty(cEraseSect)) {
        EraseEventSector(cEraseSect);
        CalcStartEvent();
      }
    }
  }
}

void SaveEventDirect(time_t iTime, enum eEventType iType, enum eEventCode iCode) {
  struct sEvent cEvent;
  cEvent.Time = iTime;
  cEvent.Type = iType;
  cEvent.Code = iCode;
  if (iCode == ecDeviceStart) { //� ������� ������ - ���� IP �����
    cEvent.Value = fsettings->ip.addr.addr;
    if (cEvent.Value == CLEAR_WORD)
      cEvent.Value = rsettings->ip.addr.addr;
  } else {
    cEvent.Value = 0;
  }
  cEvent.Value8 = 0;
  uint32_t cAddr = FLASH_SYSBLOCK_START;
  cAddr += gCurrEvent * sizeof(struct sEvent);
  EventDirectWrite(cAddr, &cEvent);
  ++gCurrEvent;
  if (gCurrEvent >= FLASH_SYSBLOCK_ITEMSIZE) {
    gCurrEvent = 0;
  }
  //���������� ������������� �������� �������, � �������� ���
  int8_t cEraseSect = CalcEraseSector();
  if (cEraseSect >= 0) {
    if (!IsSectorEmpty(cEraseSect)) {
      EventSectorDirectErase(cEraseSect);
      CalcStartEvent();
    }
  }
}

void SetEventAttributes(struct sEvent * ipEvent,
                        enum eEventType iType, enum eEventCode iCode) {
  ipEvent->Time = GetCurrTime();
  ipEvent->Type = iType;
  ipEvent->Code = iCode;
}

void SaveEvent(time_t iTime, enum eEventType iType, enum eEventCode iCode) {
  struct sEvent cEvent;
  cEvent.Time = iTime;
  cEvent.Type = iType;
  cEvent.Code = iCode;
  cEvent.Value = 0;
  cEvent.Value8 = 0;
  WriteEvent(&cEvent);
}

#define CORR_PARAM_SIZE (5)
#define MAX_CORR_VALUE (127)
void SetCorrToValue8(struct sEvent * ipEvent, int iCorrValue) {
  //��������� - � �������, � 1/10 000 (= 0,01%)
  int32_t cCorrToSave = iCorrValue / 10;
  if ((cCorrToSave > -MAX_CORR_VALUE) && (cCorrToSave < MAX_CORR_VALUE)) {
    ipEvent->Value8 = cCorrToSave;
  } else {
    if (cCorrToSave > 0)
      ipEvent->Value8 = MAX_CORR_VALUE;
    else
      ipEvent->Value8 = -MAX_CORR_VALUE;
  }
}

#define CORR_SAVE_MAX (50000)
struct sCorrParam aCorrParam[CORR_PARAM_SIZE];
int8_t gCorrParamIdx = 0;
void SaveTimeEvent(time_t iNewTime, enum eEventCode iEventCode) {
  struct sEvent cEvent, * cpLastEvent;
  SetEventAttributes(&cEvent, etSys, iEventCode);
  cEvent.Value = iNewTime;
  cEvent.Value8 = 0;
  
  int32_t cLastEventIdx = FindLastOnOffSetTimeEvent();
  if (cLastEventIdx >= 0) {
    cpLastEvent = &GetSysBlockAddr()[cLastEventIdx];
    if ((cpLastEvent->Code == ecSetSysTime) ||
        (cpLastEvent->Code == ecTestSysTime) ||
        (cpLastEvent->Code == ecSetSysTimeNtp)) {
      //���� ���������� ����� �������: ��������� ��������� � �������
      uint32_t cLastTime = cpLastEvent->Value;
      uint32_t cLocalInt;
      cLocalInt = cEvent.Time - cpLastEvent->Value;
      uint32_t cServerInt = cEvent.Value - cpLastEvent->Value;
      int32_t cInt = cServerInt;
      int32_t cCorrNew = cServerInt - cLocalInt;
      int64_t cTimeCorr = rsettings->TimeCorr;
      int64_t cDt = cCorrNew;
      cDt *= CORR_BASE;
      cDt /= cInt;
      int32_t cCorrToSave = cTimeCorr + cDt;
      if (gCorrParamIdx < CORR_PARAM_SIZE) {
        aCorrParam[gCorrParamIdx].Int = cInt;
        aCorrParam[gCorrParamIdx].CorrNew = cCorrNew;
        aCorrParam[gCorrParamIdx].Dt = cDt;
        aCorrParam[gCorrParamIdx].TimeCorr = rsettings->TimeCorr;
        aCorrParam[gCorrParamIdx].CorrToSave = cCorrToSave;
        ++gCorrParamIdx;
      }
      if (cCorrToSave != 0) {
        if ((cCorrToSave >= -CORR_SAVE_MAX) && (cCorrToSave <= CORR_SAVE_MAX)) {
          cLastTime = (rsettings->IntervalCorr * 90) / 100;
          if ((cLastTime <= cInt) && (cCorrToSave != rsettings->TimeCorr)) {
            //��������� - ���� ������ �������� ������ 0,90 �� �����������
            // (����� - �������� ����� ������), � ���� ��������� �����
            rsettings->TimeCorr = cCorrToSave;
            rsettings->IntervalCorr = cInt;
            settings_save();
          }
        }
        //��������� ������� - � �������, � 1/10 000 (= 0,01%)
        SetCorrToValue8(&cEvent, rsettings->TimeCorr);
      }
    }
  }
  WriteEvent(&cEvent);
}

#include "snmp_trap.h"
//������� � ���������� float
void SaveFloatEvent(enum eEventType iType, enum eEventCode iCode,
                    float iValue, int8_t iThreshold) {
  struct sEvent cEvent;
  SetEventAttributes(&cEvent, iType, iCode);
  float cValue = iValue;
  int32_t * cpValue = (int32_t *)&cValue;
  cEvent.Value = *cpValue;
  cEvent.Value8 = iThreshold;
  WriteEvent(&cEvent);
}

//������� � ���������� int
void SaveIntEvent(enum eEventType iType, enum eEventCode iCode,
                  int8_t iValue8, int32_t iValue) {
  struct sEvent cEvent;
  SetEventAttributes(&cEvent, iType, iCode);
  cEvent.Value  = iValue;
  cEvent.Value8 = iValue8;
  WriteEvent(&cEvent);
  if ((iCode == ecLinkOn) || (iCode == ecLinkOff)) {
    SendSnmpTrapPort(iValue8, iCode); //������� ����������� �����
  }
}

//������� ��������� ������� �����
void SaveSettingEvent(enum eEventType iType, enum eEventCode iCode,
                      enum ePortName iPortName,
                      enum eSettingName iSettingName, int iValue) {
  struct sEvent cEvent;
  SetEventAttributes(&cEvent, iType, iCode);
  uint32_t cpValue;
  if ((iPortName >= pnRS485_1) && (iPortName <= pnRS485_2)) {
    cpValue = iSettingName | (iValue << 8);
  } else {
    cpValue = iSettingName | (iValue << 16);
  }
  cEvent.Value = cpValue;
  cEvent.Value8 = iPortName;
  WriteEvent(&cEvent);
}

//������� � ����� � 2 �����������
void SaveDateEvent(enum eEventType iType, enum eEventCode iCode,
                   int8_t iValueH, int16_t iValueL,
                   int8_t iYear, int8_t iMonth, int8_t iDay) {
  //�������� ��:
  struct sEvent cEvent;
  SetEventAttributes(&cEvent, iType, iCode);
  uint32_t cpValue = (iYear << 24) | (iMonth << 16) | (iDay << 8) | iValueL;
  cEvent.Value = cpValue;
  cEvent.Value8 = iValueH;
  WriteEvent(&cEvent);
}

time_t GetLastStartTime() {
  time_t cLastStartTime = 0;
  if (gCurrEvent > 0) {
    //����� ���������� ���������� �������
    struct sEvent * cpEvent;
    int32_t cEventCount = GetEventCount();
    for (int i = cEventCount - 1; i >= 0; --i) {
      cpEvent = GetEventByIdx(i);
      if ((cpEvent->Code == ecDeviceStart) ||
          (cpEvent->Code == ecSetSysTime) ||
          (cpEvent->Code == ecSetSysTimeNtp)) {
        if (cpEvent->Code == ecDeviceStart) {
          cLastStartTime = cpEvent->Time;
        } else {
          cLastStartTime = (time_t)cpEvent->Value;
        }
        gStartTime = cLastStartTime;
        break;
      }
    }
  }
  return cLastStartTime;
}

time_t GetLastEventTime() {
  int32_t cEventIdx = gCurrEvent - 1;
  if (cEventIdx < 0)
    cEventIdx += FLASH_SYSBLOCK_ITEMSIZE;
  struct sEvent * cpEvent = GetSysBlockAddr();
  return cpEvent[cEventIdx].Time;
}

void InitLog() {
  gStartTime = mktime(&gBaseTime_tm);
#ifdef TIMEBLOCK_TEST
  FillTimeBuff(FLASH_TIMEBLOCK_ITEMSIZE >> 1, 0xffff0000);
  FillEventBuff(FLASH_SYSBLOCK_ITEMSIZE * 1 /*/ 3*/ - 2, true);

  EventSectorDirectErase(0); //�������� ������ ������
//  EventSectorDirectErase(1); //�������� ������ ������
//  FillEventBuff(FLASH_SYSBLOCK_ITEMSIZE * 1 / 3 - 41/**/, false);
#endif
  //����� ��������� ������
  int32_t cMinutes = FindSetLastMinute();
  int32_t cEvents = FindSetLastEvent();
  CalcStartEvent();
  //���������� ������ �� ������ �� �������� (� ��������� �� �����)
  time_t cTime = cMinutes * 60;
  gStartTime += cTime;
  if (cEvents == 0) { //�������� ������� ����� �� �������� = gStartTime
    vTaskDelay(200);
    SaveEventDirect(gStartTime, etDevice, ecDeviceStart);
  } else {
    //����� ��������� ������� ������. cTime = StartEvent.Time + cMinutes * 60
    //�������� cTime � gStartTime
    cTime += GetLastStartTime();
    time_t cLastEventTime = GetLastEventTime();
    if (cLastEventTime > cTime) {
      cTime = cLastEventTime;
    }
    //�������� ������� ���������� ���������� � cTime
    SaveEventDirect(cTime, etDevice, ecDeviceStop);
    cTime += 10;
    SaveEventDirect(cTime, etDevice, ecDeviceStart);
    gStartTime = cTime;
    gStartSeconds = CurrTimeInSecs();
    gStartMsecs = CurrTimeInMsecs();
  }
  //������� ������� ����� �������
  uint32_t * cpWordAddr = (uint32_t *)FLASH_TIMEBLOCK_START;
  if (*cpWordAddr != CLEAR_WORD) {
#ifdef TIMEBLOCK_TEST
    FillTimeBuff(-1, CLEAR_WORD);
#else
    EraseSectorArea(17, 1);
#endif
  }
  //��������� ������� gCurrWord � gCurrBit
  gCurrWord = 0;
  gCurrBit = 0;
}

void WriteNextTick() { //������ ���������� ���� (������)
  if (gCurrWord >= FLASH_TIMEBLOCK_ITEMSIZE) {
    gCurrBit = 0;
    return;
  }
  ++gCurrBit;
  if (gCurrBit > FLASH_WORD_SIZE) {
    gCurrBit = 1;
    ++gCurrWord;
  }
  uint32_t cWord = (0x01 << gCurrBit) - 1;
  cWord ^= CLEAR_WORD;
  uint32_t cWordAddr = FLASH_TIMEBLOCK_START;
  cWordAddr += (gCurrWord * 4);
#ifdef TIMEBLOCK_TEST
  uint32_t * cpWordAddr = (uint32_t *)cWordAddr;
  *cpWordAddr = cWord;
#else
  struct sEventItem cEventItem;
  cEventItem.Oper = ocWriteWord;
  cEventItem.Addr = cWordAddr;
  cEventItem.Event.aEvent[0] = cWord;
  if( xQueueSend( gQueueFlash, &cEventItem, 50 ) != pdPASS ) {
  }
#endif
}

void ProcessLog() {
#ifdef TIMEBLOCK_TEST
  uint32_t cTick = GetCurrSeconds() / 5;
#else
  uint32_t cTick = GetCurrSeconds() / 60;
#endif
  if (cTick > gLastTick) {
    WriteNextTick();
    gLastTick = cTick;
  }
}

