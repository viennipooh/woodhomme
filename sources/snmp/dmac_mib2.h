//******************************************************************************
// Имя файла    :       dmac_mib2.h
// Заголовок    :       
// Автор        :       Сосновских А.А.
// Дата         :       16.08.2019
//
//------------------------------------------------------------------------------
/**
База данных MIB2 для передачи таблицы DMAC МКПШ / IMC-FTX

**/

#ifndef __DMAC_MIB2_H
#define __DMAC_MIB2_H

#include "extio.h"

#if (MKPSH10 != 0)
//Место формирования таблицы DMAC Local
#define CODE_ADDRESS_DMAC (CODE_ADDRESS_UPL + 0x00002000)
#endif

#pragma pack(1)
typedef struct sDmacItem {
  uint8_t MAC[6];
  uint8_t Port;
  uint8_t VlanId;
} SDmacItem;
#pragma pack()

#define DMAC_REFRESH_INTERVAL (30000)

extern SDmacItem * gpDmacTable;
extern int gDmacTableSize;
extern SDmacItem * lpDmacTable;
extern int lDmacTableSize;
extern uint32_t gDmacRefreshTime;

extern void ReadDmacTableTo(SDmacItem * ipDmacItem, int * ipDmacTableSize);

#endif //__DMAC_MIB2_H

