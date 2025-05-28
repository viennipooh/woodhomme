//******************************************************************************
// Имя файла    :       main.h
// Заголовок    :
// Автор        :       Вахрушев Д.В.
// Дата         :       03.02.2016
//
//------------------------------------------------------------------------------
// 2016.02.03 - Интерфейс настроек сети
//            - Интерфейс обзора
//            - Интерфейс настроек последовательных портов
//
//            - Интерфейс настройки портов TCP/UDP
//            - Интерфейс настройки портов ethernet
//******************************************************************************

#ifndef __MAIN_H_
#define __MAIN_H_

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

/* deafult stm32f417zgt6 header  */
#include "stm32f4xx.h"

/* Scheduler includes */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"


#define MAIN_TASK_PRIO   ( tskIDLE_PRIORITY + 1 )
#define DHCP_TASK_PRIO   ( tskIDLE_PRIORITY + 4 )
#define LED_TASK_PRIO    ( tskIDLE_PRIORITY + 1 )

#include "defconf.h" // Настройки по умолчанию


/* Сделано переназначение define-ов в первую очерель для
 * избавления от ошибок, установки MAC адреса (групповой бит сброшен и
 * бит локального адреса выставлен)
 */

#if defined IP_ADDR_GROUP && IP_ADDR_GROUP == IP_ADDR_GROUP_A
#define IP_ADDR0       ((IP_ADDR_0) & ~(1<<7))
#elif  defined IP_ADDR_GROUP && IP_ADDR_GROUP == IP_ADDR_GROUP_B
#define IP_ADDR0       ((IP_ADDR_0) & ~(1<<6) | (1<<7))
#else
#define IP_ADDR0       ((IP_ADDR_0) & ~(1<<5) | (3<<6))
#endif

#define IP_ADDR1                IP_ADDR_1
#define IP_ADDR2                IP_ADDR_2
#define IP_ADDR3                IP_ADDR_3

#define NETMASK_ADDR0           NETMASK_ADDR_0
#define NETMASK_ADDR1           NETMASK_ADDR_1
#define NETMASK_ADDR2           NETMASK_ADDR_2
#define NETMASK_ADDR3           NETMASK_ADDR_3

#define GW_ADDR0                GW_ADDR_0
#define GW_ADDR1                GW_ADDR_1
#define GW_ADDR2                GW_ADDR_2
#define GW_ADDR3                GW_ADDR_3

#define DEBUG_IP_ADDR0          255
#define DEBUG_IP_ADDR1          255
#define DEBUG_IP_ADDR2          255
#define DEBUG_IP_ADDR3          255

#define DEBUG_PORT              50000

#undef GPIO_OUT
#define GPIO_OUT(GPIO,PIN,SET) {if(SET){GPIO->BSRRL |= (1<<PIN);}else{GPIO->BSRRH |= (1<<PIN);}}

#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

#ifdef VER_MAJ
#define MAIN_VER_MAJ            VER_MAJ
#else
#error "VER_MAJ not found"
#define MAIN_VER_MAJ            2		// версия 2 пиксель с простой камерой и повороткой с подсветкой без омывайки,  1- с тепловизором
#endif

#ifdef VER_MIN
#define MAIN_VER_MIN            VER_MIN
#else
#error "VER_MIN not found"
#define MAIN_VER_MIN            2     //1- верся с управлениенп повроткой из web
#endif

#ifdef VER_DATE
#define MAIN_VER_DATE           STR(VER_DATE)
#else
#error "VER_DATE not found"
#define MAIN_VER_DATE           "0"
#endif

#ifdef VER_CONF
#define MAIN_VER_CONFIG         STR(VER_CONF)
#else
#error "VER_CONF not found"
#define MAIN_VER_CONFIG           "0"
#endif

#define FIRMWARE
extern const struct fw_version_s firmware_data;


enum drive_mode_e {
		UP,
		DOWN,
		LEFT,
		RIGHT,
		STOP,
		START,
		NONE,
} ;
typedef enum drive_mode_e drive_mode_t;
 struct SDrive_s{
  uint32_t time ;
  drive_mode_t drive_ev;
	bool autoMove;
};
extern struct SDrive_s gDriveParam;
#endif // __MAIN_H_