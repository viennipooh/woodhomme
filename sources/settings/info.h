#ifndef __INFO_H__
#define __INFO_H__

#include <stdbool.h>
#include "stdint.h"
#include "otp.h"
#include "extio.h"

#define USE_PROTOTYPE_MKPSH //Учет опытного образца МКПШ

#define INFO_MAX_SERIAL_STRING  14
#define INFO_MAX_SERIAL_STRING_OLD  16

#if ((MKPSH10 != 0) || (IMC_FTX_MC != 0))
  #define USE_STP        //Использовать STP
  #define USE_SWITCH     //Использовать свитч kmz8895
#endif

#if ((UTD_M != 0) || (IIP != 0))
  #define NO_USE_VLANS      //Не использовать Vlan (виртуальные сети)
  #define NO_USE_CABLE_TEST //Не использовать модуль тест кабеля
#endif

#if (IMC_FTX_MC != 0)
  #define USE_SUBNETS    //Использовать подсети (подсети только в IMC-FTX-MC)
  #define USE_AT45DB     //Использовать Flash-память at45db
#endif

//#define PORT_MIGRATE_TRAP //Ловушка для "миграции" по портам

/* Сделано переназначение define-ов в первую очерель для 
 * избавления от ошибок, установки MAC адреса (групповой бит сброшен и 
 * бит локального адреса выставлен)
 */

// Мульткастовый/Уникастовый бит
#define MAC_ADDR_MULTICAST_Bit          (1<<0) 
// Локальная/Глобальная индивидуальность MAC адресов
#define MAC_ADDR_LOCALLY_Bit            (1<<1) 
// MAC адрес удовлетворяет параметрам:
// 1) Локальный адрес
// 2) Уникастовый адрес
#define MAC_ADDR_IG(A) ((( A )\
                        &( ~MAC_ADDR_MULTICAST_Bit ) \
                        |( MAC_ADDR_LOCALLY_Bit ) ))

#define MAC_ADDR0       MAC_ADDR_IG(MAC_ADDR_0)
#define MAC_ADDR1       MAC_ADDR_1
#define MAC_ADDR2       MAC_ADDR_2
#define MAC_ADDR3       MAC_ADDR_3
#define MAC_ADDR4       MAC_ADDR_4
#define MAC_ADDR5       MAC_ADDR_5


#define MAC_ADDR_MICR0  0x00
#define MAC_ADDR_MICR1  0x01
#define MAC_ADDR_MICR2  0xa1
#include "extio.h"
#define MAC_ADDR_MICR3  DEVICE_CODE

enum eFulfil {
  fNone,    //Нет исполнения
  fOld,     //Старое исполнение
  fCurrent, //Новое исполнение
  fError,   //Ошибка исполнения
  fCount
};

#pragma pack(1)
struct version_s {
  uint8_t major;
  uint8_t minor;
};
struct info_new_s {
  uint32_t timestamp;                       //4
  struct version_s hw_version;              //2
  uint16_t serial_number;                   //2
  char serial_str[INFO_MAX_SERIAL_STRING_OLD];  //16
  uint8_t mac[6];                           //6
  uint16_t crc;                             //2
};
struct info_s {
  uint32_t timestamp;                       //4
  uint32_t serial_number;                   //2 -> 4
  struct version_s hw_version;              //2
  char serial_str[INFO_MAX_SERIAL_STRING];  //14
  uint8_t mac[6];                           //6
  uint16_t crc;                             //2
};
#pragma pack()

/* index == -1; return current */
//int info_read(int _index, struct info_s *_info);
int current_pos();
int info_current(struct info_s * _info);
int info_count();

otp_result_t WriteHWSetting(uint32_t iHWSetting, int iHWIndex);

const char * GetInfo_fw();
const char * GetInfo_sw();
uint32_t GetInfo_sID();
extern bool IsFirmMAC(uint8_t * ipMAC);

#endif//__INFO_H__