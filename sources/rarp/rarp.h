//******************************************************************************
// Имя файла    :       rarp.h
// Заголовок    :       
// Автор        :       Сосновских А.А.
// Дата         :       03.10.2019
//
//------------------------------------------------------------------------------
/**
Блок работы с запросами RARP
= ответы

**/

#ifndef __RARP_H
#define __RARP_H

#include <stdint.h>
#include <stdbool.h>
#include "dmac_mib2.h"

#define LAN_TOPOLOGY_FINDING  //Алгоритм построения топологии сети

#define LARGE_DMAC_ITEM //Размер элемента передаваемой таблицы DMAC = 7байт

enum rarp_req_code {
  RARP_REQ_DMAC = 0xfe,
  RARP_REQ_FIND = 0xff
};

#pragma pack(1)
//Описание окружения порта
typedef struct sPortEnv {
  uint8_t   MAC[6]; //MAC
  uint32_t  IP;     //IP
  uint8_t   Port  ; //Номер порта
  uint8_t   IsEdge  :1; //Оконечный порт
  uint8_t   IsSingle:1; //Единственный на этом порту (== подключён к нему)
  uint8_t   Reserved:6; //Резерв
} SPortEnv;


//Описание окружения устройства
typedef struct sDevEnv {
  uint16_t  Size;               //Размер структуры
  uint8_t   PortNum;            //Количество портов
  struct sPortEnv PortEnv[10];  //Окружения портов
} SDevEnv;


#pragma pack()

extern void rarp_process(uint8_t iOper, uint8_t ** ippData, uint16_t * ipSize);
extern bool IsSnmpClientFound(uint32_t iIP);

#endif //__RARP_H

