/**
  ******************************************************************************
  * @file    statistics.h
  * @author  Dmitry Vakhrushev ( vdv.18@mail.ru )
  * @version V1.0
  * @date    14.05.2016
  * @brief   --
  ******************************************************************************
  * @attention
  *
  ******************************************************************************
  */ 
  
#ifndef _STATISTICS_H__
#define _STATISTICS_H__

#include <stdint.h>
#include "switch.h"

#ifdef TWO_SWITCH_ITEMS
  #define STATISTIC_PORTS (10)
#else
  #define STATISTIC_PORTS (5)
#endif

#define STATISTIC_INTERVAL (10) //В секундах. До 30, 60

//#define USE_ETH_PORT_LOAD //Вычисление нагрузки порта в %

enum STATISTICS_COMPONENT{
  RxLoPriorityByte = 0x00,
  RxHiPriorityByte,
  RxUndersizePkt,
  RxFragments,
  RxOversize,
  RxJabbers,
  RxSymbolError,
  RxCRCerror,
  RxAlignmentError,
  RxControl8808Pkts,
  RxPausePkts,
  RxBroadcast,
  RxMulticast,
  RxUnicast,
  Rx64Octets,
  Rx65to127Octets,
  Rx128to255Octets,
  Rx256to511Octets,
  Rx512to1023Octets,
  Rx1024to1522Octets,
  TxLoPriorityByte,
  TxHiPriorityByte,
  TxLateCollision,
  TxPausePkts,
  TxBroadcastPkts,
  TxMulticastPkts, 
  TxUnicastPkts,
  TxDeferred,
  TxTotalCollision,   
  TxExcessiveCollision,   
  TxSingleCollision,
  TxMultipleCollision,
  P1TxDropPackets = 0x100,
  P2TxDropPackets = 0x101,
  P3TxDropPackets = 0x102,
  P4TxDropPackets = 0x103,
  P5TxDropPackets = 0x104,
  P1RxDropPackets = 0x105,
  P2RxDropPackets = 0x106,
  P3RxDropPackets = 0x107,
  P4RxDropPackets = 0x108,
  P5RxDropPackets = 0x109,
};


struct statistics_s {
  uint64_t RxLoPriorityByte;
  uint64_t RxHiPriorityByte;
  uint64_t RxUndersizePkt;
  uint64_t RxFragments;
  uint64_t RxOversize;
  uint64_t RxJabbers;
  uint64_t RxSymbolError;
  uint64_t RxCRCerror;
  uint64_t RxAlignmentError;
  uint64_t RxControl8808Pkts;
  uint64_t RxPausePkts;
  uint64_t RxBroadcast;
  uint64_t RxMulticast;
  uint64_t RxUnicast;
  uint64_t Rx64Octets;
  uint64_t Rx65to127Octets;
  uint64_t Rx128to255Octets;
  uint64_t Rx256to511Octets;
  uint64_t Rx512to1023Octets;
  uint64_t Rx1024to1522Octets;
  uint64_t TxLoPriorityByte;
  uint64_t TxHiPriorityByte;
  uint64_t TxLateCollision;
  uint64_t TxPausePkts;
  uint64_t TxBroadcastPkts;
  uint64_t TxMulticastPkts; 
  uint64_t TxUnicastPkts;
  uint64_t TxDeferred;
  uint64_t TxTotalCollision;   
  uint64_t TxExcessiveCollision;   
  uint64_t TxSingleCollision;
  uint64_t TxMultipleCollision;
  uint64_t TxDropPackets;
  uint64_t RxDropPackets;
  
  uint64_t RxBytesLastInterval;
  uint64_t TxBytesLastInterval;
  uint64_t RxBytesPerInterval;
  uint64_t TxBytesPerInterval;

};


extern struct statistics_s *statistics;
void statistics_init( void );
void statistics_update( void );
void statistics_deinit( void );

#ifdef USE_ETH_PORT_LOAD //Вычисление нагрузки порта в %
extern float GetPortLoad(uint8_t iPortIdx);
#endif
extern uint32_t GetPortTraffic(bool iRx, uint8_t iPortIdx);

#endif /* _STATISTICS_H__ */


