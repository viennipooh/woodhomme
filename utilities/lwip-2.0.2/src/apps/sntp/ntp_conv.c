//******************************************************************************
// Имя файла    :       ntp_conv.c
// Заголовок    :       
// Автор        :       Сосновских А.А.
// Дата         :       24.01.2019
//
//------------------------------------------------------------------------------
/**
Преобразования значений NTP <-> timeval, time_t, msecs

**/

#include <stdio.h> 
#include <stdlib.h> 
#include <sockets.h> 

#include "ntp_conv.h"
#include "def.h"

#define OFFSET 2208988800ULL 

void ntp2tv(uint8_t ntp[8], struct timeval *tv) { 
    uint64_t aux = 0; 
    uint8_t *p = ntp; 
    int i; 
    /* we get the ntp in network byte order, so we must convert it to host byte order. */ 
    for (i = 0; i < 4; i++) { 
     aux <<= 8; 
     aux |= *p++; 
    } /* for */ 

    /* now we have in aux the NTP seconds offset */ 
    aux -= OFFSET; 
    tv->tv_sec = aux; 

    /* let's go with the fraction of second */ 
    aux = 0; 
    for (; i < 8; i++) { 
     aux <<= 8; 
     aux |= *p++; 
    } /* for */ 

    /* now we have in aux the NTP fraction (0..2^32-1) */ 
    aux *= 1000000; /* multiply by 1e6 */ 
    aux >>= 32;  /* and divide by 2^32 */ 
    tv->tv_usec = aux; 
} /* ntp2tv */ 

void tv2ntp(struct timeval *tv, uint8_t ntp[8]) { 
    uint64_t aux = 0; 
    uint8_t *p = ntp + 8; 
    int i; 

    aux = tv->tv_usec; 
    aux <<= 32; 
    aux /= 1000000; 

    /* we set the ntp in network byte order */ 
    for (i = 0; i < 4; i++) { 
     *--p = aux & 0xff; 
     aux >>= 8; 
    } /* for */ 

    aux = tv->tv_sec; 
    aux += OFFSET; 

    /* let's go with the fraction of second */ 
    for (; i < 8; i++) { 
     *--p = aux & 0xff; 
     aux >>= 8; 
    } /* for */ 
} /* ntp2tv */ 

void TimevalByNtp(struct timeval *tv) { //Преобразование timeval в NTP
    uint64_t aux = 0; 

    aux = tv->tv_usec; 
    aux <<= 32; 
    aux /= 1000000; 
    tv->tv_usec = PP_HTONL(aux);

    uint32_t cVal = (uint32_t)tv->tv_sec;
    aux = cVal; 
    aux += OFFSET; 
    tv->tv_sec = PP_HTONL(aux);
}

void NtpByTimeval(struct timeval *iTv, struct timeval *oTv) { //Преобразование NTP в timeval
    uint64_t aux = 0; 

    aux = PP_HTONL(iTv->tv_usec); 
    aux *= 1000000; 
    aux >>= 32; 
    oTv->tv_usec = aux;

    aux = PP_HTONL(iTv->tv_sec); 
    aux -= OFFSET; 
    oTv->tv_sec = aux;
}

uint64_t NtpToUnix64(struct timeval *iTv) { //Преобразование NTP в timeval
    uint64_t aux = 0; 
    uint64_t cRes;

    aux = PP_HTONL(iTv->tv_usec); 
    cRes = aux;

    aux = PP_HTONL(iTv->tv_sec); 
    aux -= OFFSET; 
    cRes |= (aux << 32);
    return cRes;
}

void Unix64ToTimeval(uint64_t iVal, struct timeval *oTv) { //Преобразование NTP в timeval
    uint64_t aux = 0; 

    aux = iVal & 0xffffffff; 
    aux *= 1000000; 
    aux >>= 32; 
    oTv->tv_usec = aux;

    aux = (iVal >> 32) & 0xffffffff; 
//    aux -= OFFSET; 
    oTv->tv_sec = aux;
}


#include "settings.h"

//Преобразование милисекунд текущего времени
void U64ToTimeval(uint64_t iMsecs, struct timeval *tv) { 
  tv->tv_sec = iMsecs / 1000;
  tv->tv_sec -= (rsettings->NtpSettings.TimeZone * 3600);
  tv->tv_usec = (iMsecs % 1000) * 1000;
}

uint64_t TimevalToU64(struct timeval *tv) {
  uint64_t cMsecs;
  cMsecs = tv->tv_sec * 1000;
  cMsecs += tv->tv_usec / 1000;
  return cMsecs;
}

uint64_t NtpToMsec(uint8_t ntp[8]) { //NTP -> милисекунды
  struct timeval cTimeval;
  ntp2tv(ntp, &cTimeval);
  return TimevalToU64(&cTimeval);
}

void MsecsToNtp(uint64_t iMsecs, uint8_t ntp[8]) { //Милисекунды -> NTP
  struct timeval cTimeval;
  U64ToTimeval(iMsecs, &cTimeval);
  tv2ntp(&cTimeval, ntp);
}

void MsecsToNtpTimeval(uint64_t iMsecs, struct timeval * ipTimeval) {
  U64ToTimeval(iMsecs, ipTimeval); //Милисекунды -> NTP в структуре timeval
  TimevalByNtp(ipTimeval);
}

extern uint64_t GetCurrMsecsTime();
extern uint64_t GetRefMsecsTime();
void GetMsecsByNtpTimeval(struct timeval * ipTimeval, enum eTimestampKind iTimestampKind) {
  uint64_t cTimeInMsecs = 0;
  switch (iTimestampKind) {
  case tkReference:
    cTimeInMsecs = GetRefMsecsTime();
    break;
  case tkTransmit:
    cTimeInMsecs = GetCurrMsecsTime();
    break;
  default:
    break;
  }
  if (cTimeInMsecs == 0) {
    ipTimeval->tv_sec = 0;
    ipTimeval->tv_usec = 0;
  } else {
    MsecsToNtpTimeval(cTimeInMsecs, ipTimeval);
  }
}

uint64_t tvToU64(struct timeval * iTv) {
  uint64_t cTv;
  cTv = iTv->tv_sec;
  cTv <<= 32;
  cTv |= iTv->tv_usec;
  return cTv;
}

#include "log.h"
extern uint64_t CurrTimeInMsecs();
extern void SetStartTime(time_t iNewStartTime, enum eEventCode iEventCode);

void CalcCurrTime(struct timeval * iaTimeval, uint64_t iLastMseconds) {
  //Вычисление текущего времени по ответу сервера
  //iaTimeval - ссылка на блок [Origrn, Receive, Transmit] ответа
  struct timeval cClientRxTv, cNewTime;
  uint64_t cMsecsRx, cClientRx, cClientTx, cServRx, cServTx, cServD;
  cClientRx = CurrTimeInMsecs();
  cClientTx = iLastMseconds;
  MsecsToNtpTimeval(cClientRx, &cClientRxTv);
  MsecsToNtpTimeval(cClientTx, &cNewTime);
  cClientRx = NtpToUnix64(&cClientRxTv);
  cClientTx = NtpToUnix64(&cNewTime);
  cServRx   = NtpToUnix64(&iaTimeval[1]);
  cServTx   = NtpToUnix64(&iaTimeval[2]);
  cMsecsRx = cClientRx;
  cMsecsRx -= cClientTx;
  cServD = cServTx;
  cServD -= cServRx;
  if (cMsecsRx > cServD) {
    cMsecsRx -= cServD;
  }
  cMsecsRx >>= 1;
  cServTx += cMsecsRx; //
  Unix64ToTimeval(cServTx, &cNewTime);
  cClientRx = NtpToUnix64(&iaTimeval[0]);
  Unix64ToTimeval(cClientRx, &cClientRxTv);
  time_t cNewSec = cNewTime.tv_sec;
  time_t cClientRxSec = cClientRxTv.tv_sec;
  int32_t cDiff = cNewSec - cClientRxSec;
  if ((cDiff > MAX_TIME_DIFF) || (cDiff < -MAX_TIME_DIFF)) {
    cNewSec += (rsettings->NtpSettings.TimeZone * 3600);
    SetStartTime(cNewSec, ecSetSysTimeNtp);
  } else {
    cNewSec += (rsettings->NtpSettings.TimeZone * 3600);
    SetStartTime(cNewSec, ecTestSysTime);
  }
}


