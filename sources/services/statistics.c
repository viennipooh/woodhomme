/**
  ******************************************************************************
  * @file    statistics.c
  * @author  Dmitry Vakhrushev ( vdv.18@mail.ru )
  * @version V1.0
  * @date    14.05.2016
  * @brief   --
  ******************************************************************************
  * @attention
  * 
  ******************************************************************************
  */ 

#include "statistics.h"
#include "stdarg.h"
#include "log.h"
#include "string.h"

#if ((MKPSH10 != 0) || (IMC_FTX_MC != 0))
  #include "ksz8895fmq.h"
  extern ksz8895fmq_t *SW1;
  #ifdef TWO_SWITCH_ITEMS
  extern ksz8895fmq_t *SW2;
  #endif
#endif
struct statistics_s stat[10] @ ".sram";
struct statistics_s *statistics;
time_t gSysTick = 0;
#define FULL_TRAFFIC (12500000) //Предельный трафик при 100 Мбит/с


void statistics_init( void )
{
  statistics = stat;
  memset((void *)stat, 0, sizeof(stat));
  gSysTick = GetCurrTime();
}

#if ((MKPSH10 != 0) || (IMC_FTX_MC != 0))
static uint32_t statistics_read_mib(ksz8895fmq_t *psw, int port, int counter)
{
  uint32_t temp = 0;
  static volatile uint8_t stat_read_buffer[4];
  
  if(port<1 || port>5)
  {
    return temp;
  }
  if(counter >= 0x100)
  {
    port=1;
  }
  
  stat_read_buffer[0] = 0x1c | ((counter>>8)&0x03);
  stat_read_buffer[1] = (counter&0x1F) | (0x20*(port-1));
  ksz8895fmq_write_registers( psw, 110, (uint8_t*)stat_read_buffer, 2);
  if(counter >= 0x100)
  {
    stat_read_buffer[0] = 0;
    stat_read_buffer[1] = 0;
    stat_read_buffer[2] = 0;
    stat_read_buffer[3] = 0;
    ksz8895fmq_read_registers ( psw, 119, (uint8_t*)stat_read_buffer, 2);
    temp =  ((uint32_t)stat_read_buffer[0]<<8);
    temp |= ((uint32_t)stat_read_buffer[1]);
    return temp;
  }
  else
  {
    ksz8895fmq_read_registers ( psw, 117, (uint8_t*)stat_read_buffer, 4);
    temp  = ((uint32_t)stat_read_buffer[0]<<24);
    temp |= ((uint32_t)stat_read_buffer[1]<<16); 
    temp |= ((uint32_t)stat_read_buffer[2]<<8); 
    temp |= ((uint32_t)stat_read_buffer[3]);
  }
  if(temp&(3<<30) != 1<<30)
    temp = 0;
  temp &= ~(3<<30);
  return temp;
}

#include "FreeRTOS.h"
#include "task.h"
void statistics_update( void )
{
  static int index = 0;
  const struct statistics_update_s {
    ksz8895fmq_t *sw;
    int port;
  } ports[STATISTIC_PORTS] = {
#ifdef TWO_SWITCH_ITEMS
    {SW1,4},
    {SW1,3},
    {SW2,4},
    {SW2,3},
    {SW1,2},
    {SW2,2},
    {SW2,1},
    {SW1,1}, //между свитчами
    {SW2,5}, //между свитчами
    {SW1,5}, //CPU
#else
    {SW1,1},
    {SW1,2},
    {SW1,3},
    {SW1,4},
    {SW1,5},    
#endif
  };
  int cSize = sizeof(ports)/sizeof(struct statistics_update_s);
  if(index>=cSize)
    index=0;
  if(1){
    stat[index].RxLoPriorityByte += statistics_read_mib(ports[index].sw,ports[index].port,RxLoPriorityByte);vTaskDelay(1);
    stat[index].RxHiPriorityByte += statistics_read_mib(ports[index].sw,ports[index].port,RxHiPriorityByte);vTaskDelay(1);
    stat[index].RxUndersizePkt += statistics_read_mib(ports[index].sw,ports[index].port,RxUndersizePkt);vTaskDelay(1);
    stat[index].RxFragments += statistics_read_mib(ports[index].sw,ports[index].port,RxFragments);vTaskDelay(1);
    stat[index].RxOversize += statistics_read_mib(ports[index].sw,ports[index].port,RxOversize);vTaskDelay(1);
    stat[index].RxJabbers += statistics_read_mib(ports[index].sw,ports[index].port,RxJabbers);vTaskDelay(1);
    stat[index].RxSymbolError += statistics_read_mib(ports[index].sw,ports[index].port,RxSymbolError);vTaskDelay(1);
    stat[index].RxCRCerror += statistics_read_mib(ports[index].sw,ports[index].port,RxCRCerror);vTaskDelay(1);
    stat[index].RxAlignmentError += statistics_read_mib(ports[index].sw,ports[index].port,RxAlignmentError);vTaskDelay(1);
    stat[index].RxControl8808Pkts += statistics_read_mib(ports[index].sw,ports[index].port,RxControl8808Pkts);vTaskDelay(1);
    stat[index].RxPausePkts += statistics_read_mib(ports[index].sw,ports[index].port,RxPausePkts);vTaskDelay(1);
    stat[index].RxBroadcast += statistics_read_mib(ports[index].sw,ports[index].port,RxBroadcast);vTaskDelay(1);
    stat[index].RxMulticast += statistics_read_mib(ports[index].sw,ports[index].port,RxMulticast);vTaskDelay(1);
    stat[index].RxUnicast += statistics_read_mib(ports[index].sw,ports[index].port,RxUnicast);vTaskDelay(1);
    stat[index].Rx64Octets += statistics_read_mib(ports[index].sw,ports[index].port,Rx64Octets);vTaskDelay(1);
    stat[index].Rx65to127Octets += statistics_read_mib(ports[index].sw,ports[index].port,Rx65to127Octets);vTaskDelay(1);
    stat[index].Rx128to255Octets += statistics_read_mib(ports[index].sw,ports[index].port,Rx128to255Octets);vTaskDelay(1);
    stat[index].Rx256to511Octets += statistics_read_mib(ports[index].sw,ports[index].port,Rx256to511Octets);vTaskDelay(1);
    stat[index].Rx512to1023Octets += statistics_read_mib(ports[index].sw,ports[index].port,Rx512to1023Octets);vTaskDelay(1);
    stat[index].Rx1024to1522Octets += statistics_read_mib(ports[index].sw,ports[index].port,Rx1024to1522Octets);vTaskDelay(1);
    stat[index].TxLoPriorityByte += statistics_read_mib(ports[index].sw,ports[index].port,TxLoPriorityByte);vTaskDelay(1);
    stat[index].TxHiPriorityByte += statistics_read_mib(ports[index].sw,ports[index].port,TxHiPriorityByte);vTaskDelay(1);
    stat[index].TxLateCollision += statistics_read_mib(ports[index].sw,ports[index].port,TxLateCollision);vTaskDelay(1);
    stat[index].TxPausePkts += statistics_read_mib(ports[index].sw,ports[index].port,TxPausePkts);vTaskDelay(1);
    stat[index].TxBroadcastPkts += statistics_read_mib(ports[index].sw,ports[index].port,TxBroadcastPkts);vTaskDelay(1);
    stat[index].TxMulticastPkts += statistics_read_mib(ports[index].sw,ports[index].port,TxMulticastPkts); vTaskDelay(1);
    stat[index].TxUnicastPkts += statistics_read_mib(ports[index].sw,ports[index].port,TxUnicastPkts);vTaskDelay(1);
    stat[index].TxDeferred += statistics_read_mib(ports[index].sw,ports[index].port,TxDeferred);vTaskDelay(1);
    stat[index].TxTotalCollision += statistics_read_mib(ports[index].sw,ports[index].port,TxTotalCollision);vTaskDelay(1); 
    stat[index].TxExcessiveCollision += statistics_read_mib(ports[index].sw,ports[index].port,TxExcessiveCollision);vTaskDelay(1);
    stat[index].TxSingleCollision += statistics_read_mib(ports[index].sw,ports[index].port,TxSingleCollision);vTaskDelay(1);
    stat[index].TxMultipleCollision += statistics_read_mib(ports[index].sw,ports[index].port,TxMultipleCollision);vTaskDelay(1);
    
    stat[index].TxDropPackets += statistics_read_mib(ports[index].sw,ports[index].port,P3TxDropPackets);vTaskDelay(1);
    stat[index].RxDropPackets += statistics_read_mib(ports[index].sw,ports[index].port,P3RxDropPackets);vTaskDelay(1);
  }
  index++;
  
  time_t cSysTick = GetCurrTime();
  if ((cSysTick - gSysTick) >= STATISTIC_INTERVAL) {
    for (int i = 0; i < cSize; ++i) {
      uint64_t cRx = stat[i].RxLoPriorityByte + stat[i].RxHiPriorityByte;
      uint64_t cTx = stat[i].TxLoPriorityByte + stat[i].TxHiPriorityByte;
      stat[i].RxBytesPerInterval = cRx - stat[i].RxBytesLastInterval;
      stat[i].TxBytesPerInterval = cTx - stat[i].TxBytesLastInterval;
      stat[i].RxBytesLastInterval = cRx;
      stat[i].TxBytesLastInterval = cTx;
    }
    gSysTick = cSysTick;
  }
}
#endif

#ifdef USE_ETH_PORT_LOAD //Вычисление нагрузки порта в %
float GetPortLoad(uint8_t iPortIdx) {
  uint64_t cRx = stat[iPortIdx].RxBytesPerInterval;
  uint64_t cTx = stat[iPortIdx].TxBytesPerInterval;
  if (cTx > cRx)
    cRx = cTx;
  cRx = cRx * 100000000 / (FULL_TRAFFIC * STATISTIC_INTERVAL);
  double cdLoad = cRx;
  cdLoad  /= 1000000;
  float cfLoad = cdLoad;
  return (float)cfLoad;
}
#endif

uint32_t GetPortTraffic(bool iRx, uint8_t iPortIdx) {
  uint64_t cTraffic;
  if (iRx)
    cTraffic = stat[iPortIdx].RxBytesPerInterval;
  else
    cTraffic = stat[iPortIdx].TxBytesPerInterval;
  cTraffic = cTraffic * 8 / (1024 * STATISTIC_INTERVAL);
  return cTraffic;
}

void statistics_deinit( void )
{
}
