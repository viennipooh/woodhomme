//******************************************************************************
// ��� �����    :       ntp_settings.h
// ���������    :       
// �����        :       ���������� �.�.
// ����         :       25.04.2019
//
//------------------------------------------------------------------------------
/**
��������� ������� NTP

**/

#ifndef __NTP_SETTINGS_H
#define __NTP_SETTINGS_H

enum eTimeCorrMode { //����� ��������� �������
  tcmManual,  //������ (����� Web-���������)
  tcmNtpAuto, //�������������� (NTP ����������)
  tcmCount
};

//��� ������� ���������: �����, �����, �����, ������
// ��� ���������: ��������������:
// - time.htm : INPUT name=period_num
// - log.c : aPeriodKindValue
enum ePeriodKind {
  pkMinute, pkHour, pkDay, pkCount
};

typedef struct sNtpSettings { //������� ������ �������
  ip_addr_t           IpAddr;
  enum eTimeCorrMode  TimeCorrMode;
  int8_t              TimeZone;
  int                 PeriodNum;
  enum ePeriodKind    PeriodKind;
  
} SNtpSettings;


#endif //__NTP_SETTINGS_H
