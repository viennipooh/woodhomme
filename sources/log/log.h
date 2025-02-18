//******************************************************************************
// ��� �����    :       log.h
// ���������    :       
// �����        :       ���������� �.�.
// ����         :       10.12.2018
//
//------------------------------------------------------------------------------
/**
����������� ������� ����

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

//��������� ������ ������� ��� ������ ���������
//#define EVENT_MEMORY_ERASE

//������� ���� (�����, ��������)
#define TIME_ZONE       (7)

//�������� ����� ����� ������� / ������� (��� �������)
//#define TIMEBLOCK_TEST

#define EVENT_SECTORS_NUMBER    (3) //3 ������� �� 16 �����
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
  #define FLASH_TIMEBLOCK_SIZE  (0x10000) //64 �����
  #define FLASH_TIMEBLOCK_START CCMDATARAM_BASE
#else
  #define FLASH_TIMEBLOCK_SIZE  (0x20000) //128 �����
  #define FLASH_TIMEBLOCK_START (0x8120000)
  #define FLASH_SYSBLOCK_START  (0x8100000) //������ 3 ������� 2-�� �����
#endif

#define CODE_ADDRESS_UPL_FLASH  (0x08140000)

enum eEventType { //��� (�����) �������
  etCommon,   //����� �������
  etSys,      //��������� �������. ������� ����������� ���������
  etDevice,   //On / Off (���/���� ����������)
  etPower,    //������� �������
  etSetting,  //������� ��������� �������� �������
  etCount     //���������� �����
};

enum eEventCode { //��� �������
  ecDeviceStart     ,  //���� ����������                        0
  ecDeviceStop      ,  //������� ����������
  ecVoltageLess     ,  //���������� ������                      2
  ecVoltageMore     ,  //���������� ������
  ecSetSysTime      ,  //��������� ���������� �������
  ecSetSysTimeNtp   ,  //��������� ���������� ������� NTP ����������
  ecParamChange     ,  //��������� �������� �������
  ecParamChangePort ,  //��������� �������� ������� ����� [���, ��������]
  ecTestSysTime     ,  //�������� ���������� ������� NTP ����������
  ecUploadFwCode    ,  //�������� ���������� �� (������, ����)
  ecTempLess        ,  //����������� ������
  ecTempMore        ,  //����������� ������
  ecLinkErrPort     ,  //��������� ����� �� ������ �����
  ecLinkErrSwitch   ,  //��������� ������ �� ������ �����
  ecLinkErrReboot   ,  //������������ �� ������ �����
  ecLinkOn          ,  //���� �: ���� �����
  ecLinkOff         ,  //���� �: ��� �����
  ecRstpOn          ,  //���� �: RSTP ���.
  ecRstpOff         ,  //���� �: RSTP ����.
  ecCount
};

//������������ ��� ������ ������� ��������� �������
enum ePortName { //����� ������ (Eth � RS485)
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

enum eSettingName { //����� ������� (Eth � RS485)
  //������� 	���� 	�������� 	������� 	���. ������� 	MDI/MDIX
  //��� ������� Eth
  snIsOn        =  0,
  snSpeedAuto   =  1, //���/����
  snSpeed       =  2, //100/10
  snDuplex      =  3, //FDX/HDX
  snFlowCtrl    =  4, //���/����
  snMdiMdx      =  5, //Auto/Mdi/Mdx
  //��� ������� RS485
  snParity      = 10, //�������
  snStopBits    = 11, //����� ���� ���
  snCount
};

typedef struct sEventDef {
  enum eEventCode   Code;
  uint8_t   IsValue8  :1; //���� �������� Value8
  uint8_t   IsValue   :1; //���� �������� Value (32)
  uint8_t   IsValue1  :1; //���� �������� Value(0-15) (16)
  uint8_t   IsValue2  :1; //���� �������� Value(16-31) (16)
} SEventDef;

#include <ip_addr.h>

#pragma pack(1)
//��������� ������� (12 ����)
#define WORDS_IN_EVENT  (3)
typedef struct sEvent {
  union {
    uint32_t      aEvent[WORDS_IN_EVENT];  //������� ��� ������ ����
    struct {
      uint32_t    Time;      //����� �������
      uint16_t    Code;      //��� ���������
      uint8_t     Type;      //��� ���������
      int8_t      Value8;    //�������� ��������� � ��������� -127 .. +127
      int32_t     Value;     //�������� ��������� �������
    };
  };
} SEvent;

enum eOper { //��� �������� � Flash
  ocWriteEvent  ,  //������ �������
  ocWriteWord   ,  //������ �����
  ocEraseEvent  ,  //������� ������� �������
  ocErase       ,  //������� ������� (�����������, � ���� FLASH_Sector_XX)
  ocSaveSettings,  //���������� ������� �� Flash
  ocCount
};

typedef struct sEventItem { //������� ������� �������� � Flash
  enum eOper  Oper;
  uint32_t    Addr;
  struct sEvent Event;
} SEventItem;

#pragma pack()

//��� �������� NTP-���������
typedef struct sCorrParam { //������� ������ �������
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
