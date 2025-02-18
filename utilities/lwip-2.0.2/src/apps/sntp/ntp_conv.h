//******************************************************************************
// ��� �����    :       ntp_conv.h
// ���������    :       
// �����        :       ���������� �.�.
// ����         :       24.01.2019
//
//------------------------------------------------------------------------------
/**
�������������� �������� NTP <-> timeval, time_t, msecs

**/

#ifndef __NTP_CONV_H
#define __NTP_CONV_H

#include <time.h> 
#include <stdint.h> 

 //����������� ���������� ����������� ����� � �������� (������)
#define MAX_TIME_DIFF   (30)


enum eTimestampKind { //��� Timestamp � ����� NTP
  tkReference,
  tkOrigin,
  tkReceive,
  tkTransmit,
  tkCount
};

struct ntp_time_t {
  union {
    uint8_t buff[8];
    struct {
      uint32_t  second;
      uint32_t  fraction;
    } ntp;
  };
};
 

extern uint64_t NtpToMsec(uint8_t ntp[8]);
extern void MsecsToNtp(uint64_t iMsecs, uint8_t ntp[8]);
extern void MsecsToNtpTimeval(uint64_t iMsecs, struct timeval * ipTimeval);


#endif //__NTP_CONV_H
