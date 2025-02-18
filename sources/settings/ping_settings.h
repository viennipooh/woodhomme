//******************************************************************************
// ��� �����    :       ping_settings.h
// ���������    :       
// �����        :       ���������� �.�.
// ����         :       16.07.2019
//
//------------------------------------------------------------------------------
/**
��������� ������� �������� �����

**/

#ifndef __PING_SETTINGS_H
#define __PING_SETTINGS_H

#include <stdbool.h>
#if !NO_SYS
  #include "semphr.h"
#endif

// - ping.htm : INPUT name=period_num

typedef struct sPingSettings { //������� �������� �����
  bool        CtrlIsOn;       //����� �������� �������
  ip_addr_t   IpAddr;         //IP-����� ��� ����������
  int         StartAfterBoot; //�������� ������� �������� ����� �������� (���)
  int         CtrlInterval;   //�������� ����� ���������� (���)
  int         PingNum;        //���������� ������ � ������ ��������
  int         PingErrNum;     //���������� ��������� �������� ��� ������������
} SPingSettings;

//������ WD �����
typedef struct sPingCtrl { //
  int       StartTimeout;
  int       StepTimeout;
  int8_t    PingCount;        //���������� ������ � ������ ��������
  int8_t    PingResultCount;
  int8_t    PingErrorCount;
  bool      PingOn;         //�������� ����� �������
  bool      Checked;        //�������� ���� ��������� ��� ������� 1 ���
  uint8_t   MAC[6];         //MAC-����� ���������� ����������
  uint16_t  ping_seq_num;
#if !NO_SYS
  xSemaphoreHandle Mutex; //
#endif
  struct sPingSettings * pPingSettings;
  uint32_t  PortEventCount;
  uint32_t  SwitchEventCount;
} SPingCtrl;

#endif //__PING_SETTINGS_H
