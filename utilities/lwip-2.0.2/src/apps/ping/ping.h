#ifndef LWIP_PING_H
#define LWIP_PING_H

//#define PVS2
#ifndef PVS2
#include "FreeRTOS.h"
#include "task.h"
#include "lwip/sockets.h"
#endif

#include "lwip/ip_addr.h"
#include "ping_settings.h"
#include "extio.h"

#ifdef PVS2
 #define PING_START_INTERVAL //Стартовый интервал - другой, и в минутах
 #define PING_NO_SETOFF //Неотключаемый PING

 #define IPADDR_CAM  LWIP_MAKEU32(192, 168, 20, 138)
 #define IPADDR_TV   LWIP_MAKEU32(192, 168, 20, 137)
 #define IPADDR_PKP  LWIP_MAKEU32(192, 168, 20, 1  )
#else
 #if (IIP == 0)
//  #define PING_TEST_INTERVAL  //Интервал между группами пакетов = 5 сек (норма = 60 сек)
  #define PING_START_INTERVAL //Стартовый интервал - другой, и в минутах
  #define USE_PING_IWDG //Использовать IWDG-защиту
 #endif
#endif

/**
 * PING_USE_SOCKETS: Set to 1 to use sockets, otherwise the raw api is used
 */

#ifdef USE_PING_IWDG //Использовать IWDG-защиту
 #ifndef PING_USE_SOCKETS
  #define PING_USE_SOCKETS    LWIP_SOCKET
 #endif
#endif

#ifdef PVS2
 #define PING_ITEM_NUM    (3)
#else
 #define PING_ITEM_NUM    (1)
#endif

//Состояния ошибки связи
enum ePingErrorState {
  pesNone,    //Нет
  pesPort,    //Порт
  pesSwitch,  //Свитч
  pesDevice,  //Устройство
  pesCount
};
   
#if PING_USE_SOCKETS
#include "settings.h"
extern void ping_setup(struct settings_s * ip_rsettings);
#else
void ping_send_now(void);
#endif /* !PING_USE_SOCKETS */


#endif /* LWIP_PING_H */
