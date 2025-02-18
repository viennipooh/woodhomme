/**
  ******************************************************************************
  * @file    defconf.h
  * @author  Dmitry Vakhrushev ( vdv.18@mail.ru )
  * @version V1.0
  * @date    20.11.2015
  * @brief   --
  ******************************************************************************
  * @attention
  ******************************************************************************
  */

#ifndef _DEFCONF_H__
#define _DEFCONF_H__

#include "def_gateway.h"

// Определить MAC адрес по ID устройства
#define DEVID_MAC_ADDRESS

#define   UID_ADDRESS      0x1FFF7A11
#define   DEV_ID_B         (((uint8_t *)UID_ADDRESS))
#define   DevID_SNo0       (*((uint32_t *)UID_ADDRESS))
#define   DevID_SNo1       (*((uint32_t *)UID_ADDRESS+32))
#define   DevID_SNo2       (*((uint32_t *)UID_ADDRESS+64))

#define MODBUS_TCP_PORT     (502)
#define MODBUS_RTU_SPEED    (38400)
/**
 * Настройка MAC адреса по умолчанию
 */
#ifdef DEVID_MAC_ADDRESS


#define MAC_ADDR_0       (DEV_ID_B[0])
#define MAC_ADDR_1       (DEV_ID_B[1])
#define MAC_ADDR_2       (DEV_ID_B[2])
#define MAC_ADDR_3       (DEV_ID_B[3])
#define MAC_ADDR_4       (DEV_ID_B[4])
#define MAC_ADDR_5       (DEV_ID_B[5])

#else

#define MAC_ADDR_0      (0xFF)
#define MAC_ADDR_1      (0x01)
#define MAC_ADDR_2      (0x02)
#define MAC_ADDR_3      (0x03)
#define MAC_ADDR_4      (0x04)
#define MAC_ADDR_5      (0x06)

#endif

#define IP_ADDR_GROUP_A 1
#define IP_ADDR_GROUP_B 2
#define IP_ADDR_GROUP_C 3

/**
 * Настройка IP группы адресов
 */
#define IP_ADDR_GROUP   IP_ADDR_GROUP_C

#include "extio.h"

#if (PIXEL != 0)
/**
 * Настройка IP адреса по умолчанию
 */
#define IP_ADDR_0       (192)
#define IP_ADDR_1       (168)
#define IP_ADDR_2       (20)
#define IP_ADDR_3       (69)
/**
 * Настройка адрес шлюза по умолчанию
 */
 #ifdef SET_GATEWAY_ADDRESS_0
  #define GW_ADDR_0       (0)
  #define GW_ADDR_1       (0)
  #define GW_ADDR_2       (0)
  #define GW_ADDR_3       (0)
 #else
  #define GW_ADDR_0       (192)
  #define GW_ADDR_1       (168)
  #define GW_ADDR_2       (1)
  #define GW_ADDR_3       (1)
 #endif
#endif

/**
 * Настройка маски подсети по умолчанию
 */
#define NETMASK_ADDR_0  (255)
#define NETMASK_ADDR_1  (255)
#define NETMASK_ADDR_2  (255)
#define NETMASK_ADDR_3  (0)



/**
 * Настройки доступа к веб серверу по имени и паролю
 */
#define WEBSERVER_INVITE
#define WEBSERVER_USER          "admin"
#define WEBSERVER_PASS          "pass"



#endif /* _DEFCONF_H__ */


