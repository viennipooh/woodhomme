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
 #define PING_START_INTERVAL //��������� �������� - ������, � � �������
 #define PING_NO_SETOFF //������������� PING

 #define IPADDR_CAM  LWIP_MAKEU32(192, 168, 20, 138)
 #define IPADDR_TV   LWIP_MAKEU32(192, 168, 20, 137)
 #define IPADDR_PKP  LWIP_MAKEU32(192, 168, 20, 1  )
#else
 #if (IIP == 0)
//  #define PING_TEST_INTERVAL  //�������� ����� �������� ������� = 5 ��� (����� = 60 ���)
  #define PING_START_INTERVAL //��������� �������� - ������, � � �������
  #define USE_PING_IWDG //������������ IWDG-������
 #endif
#endif

/**
 * PING_USE_SOCKETS: Set to 1 to use sockets, otherwise the raw api is used
 */

#ifdef USE_PING_IWDG //������������ IWDG-������
 #ifndef PING_USE_SOCKETS
  #define PING_USE_SOCKETS    LWIP_SOCKET
 #endif
#endif

#ifdef PVS2
 #define PING_ITEM_NUM    (3)
#else
 #define PING_ITEM_NUM    (1)
#endif

//��������� ������ �����
enum ePingErrorState {
  pesNone,    //���
  pesPort,    //����
  pesSwitch,  //�����
  pesDevice,  //����������
  pesCount
};
   
#if PING_USE_SOCKETS
#include "settings.h"
extern void ping_setup(struct settings_s * ip_rsettings);
#else
void ping_send_now(void);
#endif /* !PING_USE_SOCKETS */


#endif /* LWIP_PING_H */
