/**
 * @file
 * Ping sender module
 *
 */

/*
 * Redistribution and use in source and binary forms, with or without modification, 
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission. 
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED 
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT 
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, 
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT 
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING 
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY 
 * OF SUCH DAMAGE.
 *
 * This file is part of the lwIP TCP/IP stack.
 * 
 */

/** 
 * This is an example of a "ping" sender (with raw API and socket API).
 * It can be used as a start point to maintain opened a network connection, or
 * like a network "watchdog" for your device.
 *
 */

#include "ping.h"

#if LWIP_RAW /* don't build if not configured for use in lwipopts.h */

#include "lwip/mem.h"
#include "lwip/raw.h"
#include "lwip/icmp.h"
#include "lwip/sys.h"
#include "lwip/inet_chksum.h"

#if PING_USE_SOCKETS
#endif /* PING_USE_SOCKETS */

#ifdef USE_PING_IWDG //Использовать IWDG-защиту
//Уставки для пингования IWDG
struct sPingSettings gPingSettingsIWDG = {
  .CtrlIsOn = true,         //Режим контроля включен
  .IpAddr = PP_HTONL(IPADDR_ANY), //IP-адрес для пингования (default = 0.0.0.0)
  .StartAfterBoot = 1,      //Интервал запуска контроля после загрузки
#ifdef PING_TEST_INTERVAL
  .CtrlInterval =  5,       //Интервал между проверками (5 сек)
#else
  .CtrlInterval = 60,       //Интервал между проверками (1 минута)
#endif
  .PingNum = 3,             //Количество пингов в пакете проверки
  .PingErrNum = 3,          //Количество ошибочных проверок для перезагрузки
};

//Управление IWDG связи
struct sPingCtrl gPingCtrlIWDG = {
  .StartTimeout = 0,
  .StepTimeout = 0,
  .PingCount = 0,
  .PingResultCount = 0,
  .PingErrorCount = 0,
  .PingOn = false,  //Контроль связи включен
  .MAC = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
  .ping_seq_num = 0x8000,
  .pPingSettings = &gPingSettingsIWDG
};
#endif

#ifdef PVS2
#include "timer.h"
//Уставки WD связи
struct sPingSettings gaPingSettings[] = {
  {
    .CtrlIsOn = true,         //Режим контроля включен
    .IpAddr = PP_HTONL(IPADDR_CAM), //IP-адрес для пингования
    .StartAfterBoot = 1,      //Интервал запуска контроля после загрузки
    .CtrlInterval = 30,       //Интервал между проверками (1 минута)
    .PingNum = 2,             //Количество пингов в пакете проверки
    .PingErrNum = 1,          //Количество ошибочных проверок для перезагрузки
  },
  {
    .CtrlIsOn = true,         //Режим контроля включен
    .IpAddr = PP_HTONL(IPADDR_TV ), //IP-адрес для пингования
    .StartAfterBoot = 1,      //Интервал запуска контроля после загрузки
    .CtrlInterval = 30,       //Интервал между проверками (1 минута)
    .PingNum = 2,             //Количество пингов в пакете проверки
    .PingErrNum = 1,          //Количество ошибочных проверок для перезагрузки
  },
  {
    .CtrlIsOn = true,         //Режим контроля включен
    .IpAddr = PP_HTONL(IPADDR_PKP), //IP-адрес для пингования
    .StartAfterBoot = 1,      //Интервал запуска контроля после загрузки
    .CtrlInterval = 30,       //Интервал между проверками (1 минута)
    .PingNum = 2,             //Количество пингов в пакете проверки
    .PingErrNum = 1,          //Количество ошибочных проверок для перезагрузки
  }
};

//Управление WD связи
struct sPingCtrl gaPingCtrl[] = {
  {
    .StartTimeout = 0,
    .StepTimeout = 0,
    .PingCount = 0,
    .PingResultCount = 0,
    .PingErrorCount = 0,
    .PingOn = false,  //Контроль связи включен
    .MAC = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    .ping_seq_num = 0x0000,
    .pPingSettings = &gaPingSettings[0],
    .PortEventCount = 0,
    .SwitchEventCount = 0
  },
  {
    .StartTimeout = 0,
    .StepTimeout = 0,
    .PingCount = 0,
    .PingResultCount = 0,
    .PingErrorCount = 0,
    .PingOn = false,  //Контроль связи включен
    .MAC = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    .ping_seq_num = 0x5000,
    .pPingSettings = &gaPingSettings[1],
    .PortEventCount = 0,
    .SwitchEventCount = 0
  },
  {
    .StartTimeout = 0,
    .StepTimeout = 0,
    .PingCount = 0,
    .PingResultCount = 0,
    .PingErrorCount = 0,
    .PingOn = false,  //Контроль связи включен
    .MAC = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    .ping_seq_num = 0xa000,
    .pPingSettings = &gaPingSettings[2],
    .PortEventCount = 0,
    .SwitchEventCount = 0
  }
};
#else
struct sPingCtrl gaPingCtrl[] = {
  {
    .StartTimeout = 0,
    .StepTimeout = 0,
    .PingCount = 0,
    .PingResultCount = 0,
    .PingErrorCount = 0,
    .PingOn = false,  //Контроль связи включен
    .MAC = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    .ping_seq_num = 0x0000,
    .pPingSettings = NULL //Подключается из settings
  }
};
#endif


int8_t gResultCount = 0;

/**
 * PING_DEBUG: Enable debugging for PING.
 */
#ifndef PING_DEBUG
#define PING_DEBUG     LWIP_DBG_ON
#endif

/** ping receive timeout - in milliseconds */
#ifndef PING_RCV_TIMEO
#define PING_RCV_TIMEO 600 //1000
#endif

/** ping delay - in milliseconds */
#ifndef PING_DELAY
#define PING_DELAY     1000
#endif

/** ping identifier - must fit on a u16_t */
#ifndef PING_ID
#define PING_ID        0xAFAF
#endif

/** ping additional data size to include in the packet */
#ifndef PING_DATA_SIZE
#define PING_DATA_SIZE 32
#endif

///** ping result action - no default action */
#ifndef PING_RESULT
#define PING_RESULT(ping_ok)
#endif

bool PingResult(struct icmp_echo_hdr * ipecho) {
  if ((ipecho->id == PING_ID) && (ICMPH_TYPE(ipecho) == ICMP_ER)) {
    for (int i = 0; i < PING_ITEM_NUM; ++i) {
      if (ipecho->seqno == lwip_htons(gaPingCtrl[i].ping_seq_num)) {
        --gaPingCtrl[i].PingResultCount;
        return true;
      }
    }
#ifndef PVS2
 #ifdef USE_PING_IWDG //Использовать IWDG-защиту
    if (ipecho->seqno == lwip_htons(gPingCtrlIWDG.ping_seq_num)) {
      if (gPingCtrlIWDG.PingResultCount > 0)
        --gPingCtrlIWDG.PingResultCount;
      return true;
    }
 #else
    return true;
 #endif
#endif
  } else {
    //LWIP_DEBUGF( PING_DEBUG, ("ping: drop\n"));
  }
  return false;
}


/** Prepare a echo ICMP request */
static void
ping_prepare_echo( struct icmp_echo_hdr *iecho, u16_t len, struct sPingCtrl * ipPingCtrl)
{
  size_t i;
  size_t data_len = len - sizeof(struct icmp_echo_hdr);

  ICMPH_TYPE_SET(iecho, ICMP_ECHO);
  ICMPH_CODE_SET(iecho, 0);
  iecho->chksum = 0;
  iecho->id     = PING_ID;
  iecho->seqno  = lwip_htons(++ipPingCtrl->ping_seq_num);

  /* fill the additional data buffer with some data */
  for(i = 0; i < data_len; i++) {
    ((char*)iecho)[sizeof(struct icmp_echo_hdr) + i] = (char)i;
  }
#if CHECKSUM_GEN_ICMP
  iecho->chksum = inet_chksum(iecho, len);
#else
  iecho->chksum = 0;
#endif
}

#include "settings.h"

/* Ping using the socket ip */
static err_t
ping_send(int s, struct sPingCtrl * ipPingCtrl)
{
#if !NO_SYS
  int err;
#endif
  struct icmp_echo_hdr *iecho;
  size_t ping_size = sizeof(struct icmp_echo_hdr) + PING_DATA_SIZE;
  LWIP_ASSERT("ping_size is too big", ping_size <= 0xffff);

  iecho = (struct icmp_echo_hdr *)mem_malloc((mem_size_t)ping_size);
  if (!iecho) {
    return ERR_MEM;
  }

  ping_prepare_echo(iecho, (u16_t)ping_size, ipPingCtrl);
  
#if !NO_SYS
  struct sockaddr_storage to;
  if(IP_IS_V4(&ipPingCtrl->pPingSettings->IpAddr)) {
    struct sockaddr_in *to4 = (struct sockaddr_in*)&to;
    to4->sin_len    = sizeof(to4);
    to4->sin_family = AF_INET;
    inet_addr_from_ip4addr(&to4->sin_addr, ip_2_ip4(&ipPingCtrl->pPingSettings->IpAddr));
  }
#else
  struct pbuf *p;
  err_t ret = ERR_VAL;
  p = pbuf_alloc(PBUF_TRANSPORT, ping_size, PBUF_POOL);
  if (p != NULL) {
    pbuf_take(p, (void *) iecho, ping_size);
    ret = ip4_output(p, &fsettings->ip.addr,
                     ip_2_ip4(&ipPingCtrl->pPingSettings->IpAddr),
                     ICMP_TTL, 0, IP_PROTO_ICMP);
    pbuf_free(p);
  }
  mem_free(iecho);
  return ret;
#endif
#if !NO_SYS
  err = lwip_sendto(s, iecho, ping_size, 0, (struct sockaddr*)&to, sizeof(to));
  mem_free(iecho);
  return (err ? ERR_OK : ERR_VAL);
#endif
}


#if !NO_SYS
#include "string.h"
int32_t gWhileCount = -1;
static void
ping_recv(int s)
{
  gWhileCount = 0;
  int cPingCount = gPingCtrlIWDG.PingResultCount;
  uint32_t cTick = xTaskGetTickCount() + 800;
  while (cPingCount <= gPingCtrlIWDG.PingResultCount) {
    vTaskDelay(10);
    ++gWhileCount;
    if (xTaskGetTickCount() >= cTick) {
      break;
    }
  }
}
#endif //NO_SYS

#include <string.h>
extern u8_t * etharp_find_entry_by_ip(const ip4_addr_t * ipaddr);
//Ответ на эхо (echo reply)
void ping_lopback_er(struct pbuf * ip, struct ip_hdr * ip_iphdr, struct eth_hdr * i_ethhdr) {
  struct icmp_echo_hdr * iecho = (struct icmp_echo_hdr *)ip->payload;
  for (int i = 0; i < PING_ITEM_NUM; ++i) {
    if (gaPingCtrl[i].pPingSettings->IpAddr.addr == ip_iphdr->src.addr) {
      u8_t * cMAC = etharp_find_entry_by_ip(&gaPingCtrl[i].pPingSettings->IpAddr);
      if (cMAC != NULL)
        memcpy(&gaPingCtrl[i].MAC[0], cMAC, 6);
    }
  }
  PingResult(iecho);
}

//
void InitTimeout(struct sPingCtrl * ipPingCtrl, bool iStart) {
  if (iStart) {
#ifdef PING_START_INTERVAL //Стартовый интервал - другой, и в минутах
    ipPingCtrl->StartTimeout = ipPingCtrl->pPingSettings->StartAfterBoot * 60;
#else
    ipPingCtrl->StartTimeout = ipPingCtrl->pPingSettings->CtrlInterval;
#endif
    ipPingCtrl->PingErrorCount = 0;
    ipPingCtrl->Checked = false;
  } else {
    ipPingCtrl->StepTimeout = ipPingCtrl->pPingSettings->CtrlInterval;
 }
}

void PingCtrlInit(struct sPingCtrl * ipPingCtrl, struct sPingSettings * ipPingSettings) {
  if (ipPingCtrl->pPingSettings == NULL)
    ipPingCtrl->pPingSettings = ipPingSettings;
#if !NO_SYS
  ipPingCtrl->Mutex = xSemaphoreCreateMutex();
#endif
  ipPingCtrl->StepTimeout = 0;
  ipPingCtrl->PingOn = true;
  ipPingCtrl->PingCount = 0;
  ipPingCtrl->PingResultCount = 0;
  
//  uint8_t   MAC[6];         //MAC-адрес устройства пингования
}

#ifndef PVS2
  #include "stm32f4xx_iwdg.h"
  #include "log.h"
#else
  #include "extio.h"
  #include "switch.h"
  extern int get_port_by_mac(char *mac);
#endif
extern void __reboot__();
uint8_t cEmptyMAC[ETH_HWADDR_LEN] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

bool OtherPingCtrlTest(struct sPingCtrl * ipPingCtrl) {
  for (int i = 0; i < PING_ITEM_NUM; ++i) {
    if (&gaPingCtrl[i] != ipPingCtrl) {
      if ((gaPingCtrl[i].Checked) &&
          (memcmp(gaPingCtrl[i].MAC, cEmptyMAC, 6) == 0))
        return false;
      if (gaPingCtrl[i].PingErrorCount > 0)
        return false;
    }
  }
  return true;
}

int8_t FindPingCtrlErrorItem() {
  for (int i = 0; i < PING_ITEM_NUM; ++i) {
    if ((memcmp(gaPingCtrl[i].MAC, cEmptyMAC, 6) == 0) ||
          (gaPingCtrl[i].PingErrorCount > 0))
      return i;
  }
  return -1;
}

#ifdef PVS2
#define PORT_NUMBER (3)
#endif
#include "info.h"
void ProcessPingError(struct sPingCtrl * ipPingCtrl, enum ePingErrorState iPingErrorState) { //Обработка потери связи
  int8_t cNum = 0;
  //cNum = Найти порт по IP-адресу
  if (memcmp(ipPingCtrl->MAC, cEmptyMAC, 6) != 0) {
    cNum = get_port_by_mac((char *)ipPingCtrl->MAC);
  }
  //Если локальный, то: возврат
  if ((cNum > 0) && (cNum <= PORT_NUMBER)) {
  } else { //Если не найден порт:
    if (iPingErrorState == pesPort) {
      //Проверить остальные PingCtrl
      bool cOtherAreRight = OtherPingCtrlTest(ipPingCtrl);
      if (!cOtherAreRight) {
        iPingErrorState = pesSwitch; //Пропуск пересбросов порта
        ipPingCtrl->PingErrorCount =
          ipPingCtrl->pPingSettings->PingErrNum + pesSwitch - pesPort;
      }
    }
  }
  bool cIsFiber;
  switch (iPingErrorState) {
  case pesPort:
    //Найти PingCtrl без MAC и с ошибкой
    cNum = FindPingCtrlErrorItem();
    //Иниц-я PingCtrl
    PingCtrlInit(ipPingCtrl, NULL);
    InitTimeout(ipPingCtrl, false);
#ifndef PVS2
    //Событие порт: потеря связи
    SaveIntEvent(etDevice, ecLinkErrPort, cNum + 1,
                 (int32_t)ipPingCtrl->pPingSettings->IpAddr.addr);
 #ifdef USE_SWITCH //Использовать свитч kmz8895
    //Пересброс порта, если это оптика
    cIsFiber = is_port_fiber( cNum );
    switch_port_state( cNum, 0 );
    if (cIsFiber)
      switch_port_power( cNum, 0 );
    vTaskDelay(300);
    if (cIsFiber) {
      switch_port_power( cNum, 1 );
      vTaskDelay(1000);
    }
    switch_port_state( cNum, 1 );
 #endif
#else
 #ifdef USE_SWITCH //Использовать свитч kmz8895
    //Иниц-я PingCtrl
    switch_port_restart( cNum );
 #endif
#endif
    ++ipPingCtrl->PortEventCount;
    break;
  case pesSwitch:
    //Иниц-я PingCtrl
    for (int i = 0; i < PING_ITEM_NUM; ++i) {
      PingCtrlInit(&gaPingCtrl[i], NULL);
      InitTimeout(ipPingCtrl, false);
    }
#ifndef PVS2
 #ifdef USE_SWITCH //Использовать свитч kmz8895
    //Событие свитч: потеря связи
    //cNum = Найти свитч по порту
    cNum = switch_by_port(cNum);
    SaveIntEvent(etDevice, ecLinkErrSwitch, cNum,
                 (int32_t)ipPingCtrl->pPingSettings->IpAddr.addr);
    //Пересброс свитча
    switch_off(cNum - 1);
    vTaskDelay(300);
    switch_on(cNum - 1);
    vTaskDelay(1000);
 #endif
#else
 #ifdef USE_SWITCH //Использовать свитч kmz8895
    //Пересброс свитча
    switch_on_start(0);
 #endif
#endif
    ++ipPingCtrl->SwitchEventCount;
    break;
  case pesDevice:
#ifndef PVS2
    //Событие перезагрузка по потере связи
    SaveIntEvent(etDevice, ecLinkErrReboot, 0,
                 (int32_t)ipPingCtrl->pPingSettings->IpAddr.addr);
    vTaskDelay(1000);
#endif
    __reboot__(); //Перезагрузка
    break;
  default:
    break;
  }
}

#ifdef PVS2
void ProcessPingItem(struct sPingCtrl * ipPingCtrl) {
  if ((ipPingCtrl->pPingSettings != NULL) && (ipPingCtrl->pPingSettings->CtrlIsOn)) {
    if ((ipPingCtrl->StartTimeout == 0) && (ipPingCtrl->StepTimeout == 0)) {
      if (ping_send(0, ipPingCtrl) == ERR_OK) {
        ++ipPingCtrl->PingResultCount; //Пинг передан
        //ping_recv(s);
      } else {       //Ошибка передачи ping
      }
      if (--ipPingCtrl->PingCount == 0) {
        ipPingCtrl->Checked = true;
        ipPingCtrl->StepTimeout = ipPingCtrl->pPingSettings->CtrlInterval;
        //Проверка качества связи:
        if (ipPingCtrl->PingResultCount < ipPingCtrl->pPingSettings->PingNum) { //- Если gPingResultCount == 0, то:
          //  - Есть связь
          ipPingCtrl->PingErrorCount = 0; //  - Обнулить счетчик ошибок
        } else { //- Иначе:
          ++ipPingCtrl->PingErrorCount; //
          if (ipPingCtrl->PingErrorCount >= ipPingCtrl->pPingSettings->PingErrNum) { //  - Если gPingErrorCount > PingErrNum
            //Событие потеря связи
            enum ePingErrorState cPingErrorState = pesPort + ipPingCtrl->PingErrorCount - ipPingCtrl->pPingSettings->PingErrNum;
            ProcessPingError(ipPingCtrl, cPingErrorState);
            ipPingCtrl->StepTimeout = 2; //2 секунды
          }
        }
        ipPingCtrl->PingResultCount = 0;
      }
    } else { 
      if (ipPingCtrl->StartTimeout > 0) {
        --ipPingCtrl->StartTimeout;
        if (ipPingCtrl->StartTimeout == 0) {
          ipPingCtrl->PingCount = ipPingCtrl->pPingSettings->PingNum;
          ipPingCtrl->PingResultCount = 0;
        }
      } else {
        if (ipPingCtrl->StepTimeout > 0) {
          --ipPingCtrl->StepTimeout;
          if (ipPingCtrl->StepTimeout == 0) {
            ipPingCtrl->PingCount = ipPingCtrl->pPingSettings->PingNum;
            ipPingCtrl->PingResultCount = 0;
          }
        }
      }
    }
  }
}
#else //PVS2
int s;

void ProcessPingItem(struct sPingCtrl * ipPingCtrl) {
  if ((ipPingCtrl->pPingSettings != NULL) && (ipPingCtrl->pPingSettings->CtrlIsOn)) {
    if ((ipPingCtrl->StartTimeout == 0) && (ipPingCtrl->StepTimeout == 0)) {
      if (ping_send(s, ipPingCtrl) == ERR_OK) {
        ++ipPingCtrl->PingResultCount; //Пинг передан
        ping_recv(s);
      } else {       //Ошибка передачи ping
      }
      if (--ipPingCtrl->PingCount == 0) {
        ipPingCtrl->StepTimeout = ipPingCtrl->pPingSettings->CtrlInterval;
        //Проверка качества связи:
        if (ipPingCtrl->PingResultCount < ipPingCtrl->pPingSettings->PingNum) { //- Если gPingResultCount == 0, то:
          //  - Есть связь
          ipPingCtrl->PingErrorCount = 0; //  - Обнулить счетчик ошибок
        } else { //- Иначе:
          ++ipPingCtrl->PingErrorCount; //
          if (ipPingCtrl->PingErrorCount >= ipPingCtrl->pPingSettings->PingErrNum) { //  - Если gPingErrorCount > PingErrNum
            //Событие потеря связи
            enum ePingErrorState cPingErrorState =
              (enum ePingErrorState)(pesPort + ipPingCtrl->PingErrorCount - ipPingCtrl->pPingSettings->PingErrNum);
            ProcessPingError(ipPingCtrl, cPingErrorState);
            ipPingCtrl->StepTimeout = 2; //2 секунды
          }
        }
        ipPingCtrl->PingResultCount = 0;
      }
    } else { 
      if (ipPingCtrl->StartTimeout > 0) {
        --ipPingCtrl->StartTimeout;
        if (ipPingCtrl->StartTimeout == 0)
          ipPingCtrl->PingCount = ipPingCtrl->pPingSettings->PingNum;
      } else {
        if (ipPingCtrl->StepTimeout > 0) {
          --ipPingCtrl->StepTimeout;
          if (ipPingCtrl->StepTimeout == 0)
            ipPingCtrl->PingCount = ipPingCtrl->pPingSettings->PingNum;
        }
      }
    }
  }
}

void ping_thread(void *arg)
{
  int ret; 
  
  struct timeval timeout;
  timeout.tv_sec = PING_RCV_TIMEO/1000;
  timeout.tv_usec = (PING_RCV_TIMEO%1000)*1000;
  LWIP_UNUSED_ARG(arg);

  s = lwip_socket(AF_INET,  SOCK_RAW, IP_PROTO_ICMP);
  if (s < 0) {
    return;
  }
  ret = lwip_setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
  LWIP_ASSERT("setting receive timeout failed", ret == 0);
  LWIP_UNUSED_ARG(ret);

  while (1) {
    for (int i = 0; i < PING_ITEM_NUM; ++i) {
      if (xSemaphoreTake(gaPingCtrl[0].Mutex, 200)) {
        ProcessPingItem(&gaPingCtrl[0]);
        xSemaphoreGive(gaPingCtrl[0].Mutex);
      }
    }
    sys_msleep(PING_DELAY);
  }
}
#endif

#ifdef USE_PING_IWDG //Использовать IWDG-защиту
uint32_t gIWDG_RestartCount = 0, gRecvTick = 0;
uint32_t gTick, gPingSendCount = 0;
void ping_thread_IWDG(void *arg)
{
  int s;
  int ret;

  struct timeval timeout;
  timeout.tv_sec = PING_RCV_TIMEO/1000;
  timeout.tv_usec = (PING_RCV_TIMEO%1000)*1000;
  LWIP_UNUSED_ARG(arg);

  s = lwip_socket(AF_INET,  SOCK_RAW, IP_PROTO_ICMP);
  if (s < 0) {
    return;
  }

  ret = lwip_setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
  LWIP_ASSERT("setting receive timeout failed", ret == 0);
  LWIP_UNUSED_ARG(ret);

  while (1) {
    if (xSemaphoreTake(gPingCtrlIWDG.Mutex, 200)) {
      if (gPingSettingsIWDG.CtrlIsOn) {
        if ((gPingCtrlIWDG.StartTimeout == 0) && (gPingCtrlIWDG.StepTimeout == 0)) {
          if (ping_send(s, &gPingCtrlIWDG) == ERR_OK) {
            ++gPingSendCount;
            gTick = xTaskGetTickCount();
            ++gPingCtrlIWDG.PingResultCount;
            ping_recv(s);
            gRecvTick = xTaskGetTickCount() - gTick;
          } else {  //Ошибка передачи ping
            ++gIWDG_RestartCount;
            IWDG_ReloadCounter();
            sys_msleep(PING_DELAY / 10);
            continue;
          }
          if (--gPingCtrlIWDG.PingCount == 0) {
            gPingCtrlIWDG.StepTimeout = gPingSettingsIWDG.CtrlInterval;
            //Проверка качества связи:
            if (gPingCtrlIWDG.PingResultCount < gPingSettingsIWDG.PingNum) { //- Если gPingResultCount == 0, то:
              //  - Есть связь
              gPingCtrlIWDG.PingErrorCount = 0; //  - Обнулить счетчик ошибок
            } else { //- Иначе:
              ++gPingCtrlIWDG.PingErrorCount; //
              if (gPingCtrlIWDG.PingErrorCount >= gPingSettingsIWDG.PingErrNum) { //  - Если gPingErrorCount > PingErrNum
                //Событие зависание стека LwIP
                SaveIntEvent(etDevice, ecLinkErrReboot, 0,
                             (int32_t)gPingSettingsIWDG.IpAddr.addr);
                vTaskDelay(1000);
                __reboot__(); //Перезагрузка
              }
            }
            gPingCtrlIWDG.PingResultCount = 0;
          }
        } else { 
          if (gPingCtrlIWDG.StartTimeout > 0) {
            --gPingCtrlIWDG.StartTimeout;
            if (gPingCtrlIWDG.StartTimeout == 0)
              gPingCtrlIWDG.PingCount = gPingSettingsIWDG.PingNum;
          } else {
            if (gPingCtrlIWDG.StepTimeout > 0) {
              --gPingCtrlIWDG.StepTimeout;
              if (gPingCtrlIWDG.StepTimeout == 0)
                gPingCtrlIWDG.PingCount = gPingSettingsIWDG.PingNum;
            }
          }
        }
        ++gIWDG_RestartCount;
        IWDG_ReloadCounter();
      } else {
        ++gIWDG_RestartCount;
        IWDG_ReloadCounter();
      }
      xSemaphoreGive(gPingCtrlIWDG.Mutex);
    }
    sys_msleep(PING_DELAY);
  }
}

void SetIWDG_IpAddr(struct settings_s * ip_rsettings) {
  gPingSettingsIWDG.IpAddr.addr = ip_rsettings->ip.addr.addr;
//  gPingSettingsIWDG.IpAddr.addr += 1;
}

void IWDG_Init();
#endif

#ifdef PVS2
static void timer_ping_update(timer_t _timer)
{
  static int cCtrlIdx = 0;
  ProcessPingItem(&gaPingCtrl[cCtrlIdx++]);
  if (cCtrlIdx >= PING_ITEM_NUM)
    cCtrlIdx = 0;
}
#endif

void ping_setup(struct settings_s * ip_rsettings) {
#ifndef PVS2
  PingCtrlInit(&gaPingCtrl[0], &ip_rsettings->PingSettings);
  InitTimeout(&gaPingCtrl[0], true);
  sys_thread_new("ping_thread", ping_thread, NULL, DEFAULT_THREAD_STACKSIZE, DEFAULT_THREAD_PRIO);
#else
  for (int i = 0; i < PING_ITEM_NUM; ++i) {
    PingCtrlInit(&gaPingCtrl[i], NULL);
    InitTimeout(&gaPingCtrl[i], true);
    gaPingCtrl[i].StartTimeout += (gaPingCtrl[0].pPingSettings->CtrlInterval / 2) * i;
  }
  timer_create(TIMER_REPEAT_START, TIMER_MILLISECOND(1000 / PING_ITEM_NUM), timer_ping_update); 
#endif
#ifdef USE_PING_IWDG //Использовать IWDG-защиту
  //IWDG
  PingCtrlInit(&gPingCtrlIWDG, &gPingSettingsIWDG);
  InitTimeout(&gPingCtrlIWDG, true);
  SetIWDG_IpAddr(ip_rsettings);
  IWDG_Init();
  gPingCtrlIWDG.PingOn = true;
  sys_thread_new("ping_threadIWDG", ping_thread_IWDG, NULL, DEFAULT_THREAD_STACKSIZE, DEFAULT_THREAD_PRIO);
#endif
}

#ifndef PING_NO_SETOFF //Неотключаемый PING
void ResetPingItem(struct sPingCtrl * ipPingCtrl) {
  ipPingCtrl->PingOn = false;
  ipPingCtrl->PingCount = ipPingCtrl->pPingSettings->PingNum;
  ipPingCtrl->StartTimeout = 0;
  ipPingCtrl->StepTimeout = 0;
  ipPingCtrl->PingOn = true;
}

void ping_reset() {
  for (int i = 0; i < PING_ITEM_NUM; ++i)
    ResetPingItem(&gaPingCtrl[i]);
}
#endif //PING_NO_SETOFF //Неотключаемый PING


#ifdef USE_PING_IWDG //Использовать IWDG-защиту
//Блок IWDG
void IWDG_Init() {
  uint16_t cCounter = gPingCtrlIWDG.pPingSettings->CtrlInterval * 1000 / 8;
  RCC_LSICmd(ENABLE);
  while (RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET);
  IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);
  IWDG_SetPrescaler(IWDG_Prescaler_256);
  IWDG_SetReload(cCounter);
  IWDG_ReloadCounter();
  IWDG_Enable();
}
#endif //USE_PING_IWDG //Использовать IWDG-защиту

#endif /* LWIP_RAW */
