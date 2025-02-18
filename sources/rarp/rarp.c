//******************************************************************************
// Имя файла    :       rarp.c
// Заголовок    :       
// Автор        :       Сосновских А.А.
// Дата         :       03.10.2019
//
//------------------------------------------------------------------------------
/**
Блок работы с запросами RARP
= ответы

**/

#include "lwip/opt.h"

#if LWIP_ARP || LWIP_ETHERNET

#include "lwip/etharp.h"
#include "lwip/stats.h"
#include "lwip/snmp.h"
#include "lwip/dhcp.h"
#include "lwip/autoip.h"
#include "netif/ethernet.h"

#include <string.h>

#include "rarp.h"

uint8_t aRarpData[1000];
struct sPortEnv gaPortEnv[ARP_TABLE_SIZE];
int8_t gPortEnvCount = 0;

ip_addr_t getIpFromArpTable(uint8_t * ipMAC) {
  ip4_addr_t cAddr;
  cAddr.addr = 0;
  for (int i = 0; i < gPortEnvCount; ++i) {
    if (memcmp(ipMAC, gaPortEnv[i].MAC, ETH_HWADDR_LEN) == 0)
      cAddr.addr = gaPortEnv[i].IP;
  }
  return cAddr;
}

#include "info.h"
#ifdef USE_SWITCH //Использовать свитч kmz8895
  extern uint8_t read_switch_status(int port, uint8_t reg);
#endif
//Подключения портов
uint16_t GetPortsConnection() {
  uint16_t cPortConnection = 0;
#ifdef USE_SWITCH //Использовать свитч kmz8895
  for (int i = 0; i < PORT_NUMBER; ++i) {
    uint8_t cConn = read_switch_status(i, 1);
    if (cConn & (1 << 5)) {
      cPortConnection |= 1 << i;
    }
  }
#endif
  return cPortConnection;
}

//Количество подключенных портов
uint8_t PortConnectionCount(uint16_t iPortConnection) {
  uint8_t cCount = 0;
  for (int i = 0; i < 16; ++i) {
    if (iPortConnection & (1 << i))
    ++cCount;
  }
  return cCount;
}

//Полнота подключенных портов в DMAC-таблице
bool IsDmacTableFull(SDmacItem * ipDmacItem, int iDmacTableSize, uint16_t iPortsConnection) {
  uint16_t cPortsConnection = 0;
  SDmacItem * cpDmacItem;
  for (int i = 0; i < iDmacTableSize; ++i) {
    cpDmacItem = &ipDmacItem[i];
    if (cpDmacItem->Port > 0)
      cPortsConnection |= (1 << (cpDmacItem->Port - 1));
  }
  //Избыточные подключения не учитываются
  return ((cPortsConnection & iPortsConnection) == iPortsConnection);
}

//Удаление избыточных подключений. Возврат - итоговый размер DMAC-таблицы
int8_t RemoveExtraConnections(SDmacItem * ipDmacItem, int iDmacTableSize, uint16_t iPortsConnection) {
  int cSize = iDmacTableSize;
  SDmacItem * cpDmacItem;
  int cPos = 0;
  int8_t cIsExtra = 0;
  // !!! Другой алгоритм!!!
  for (int i = 0; i < cSize; ++i) {
    cpDmacItem = &ipDmacItem[i];
    if (cpDmacItem->Port > 0) {
      if ((iPortsConnection & (1 << (cpDmacItem->Port - 1))) == 0) {
        ++cIsExtra;
        if (i < (cSize - 1)) { // !!! Проверить краевые эффекты
          memcpy((uint8_t *)&ipDmacItem[cPos], (uint8_t *)&ipDmacItem[i + 1],
                 sizeof(SDmacItem));
        }
      } else {
        ++cPos;
        if (i >= cPos) {
          if (i < (cSize - 1)) { // !!! Проверить краевые эффекты
            memcpy((uint8_t *)&ipDmacItem[cPos], (uint8_t *)&ipDmacItem[i + 1],
                   sizeof(SDmacItem));
          }
        }
      }
    }
  }
  return iDmacTableSize - cIsExtra;
}

#ifdef USE_SWITCH //Использовать свитч kmz8895
 #if (UTD_M == 0) //Для перестройки на UTD_M
extern u32_t sys_now(void);
void Refresh_gDmacTable() {
  //Обновить gpDmacTable, gDmacRefreshTime
  memcpy((uint8_t *)gpDmacTable, (uint8_t *)lpDmacTable,
         sizeof(SDmacItem) * lDmacTableSize);
  gDmacTableSize = lDmacTableSize;
  gDmacRefreshTime = sys_now();
}

extern void settings_dmac_table_update();
void UpdateDmacTable() {
  uint16_t cPortConnection = GetPortsConnection();
  settings_dmac_table_update();
  ReadDmacTableTo(lpDmacTable, &lDmacTableSize); //Сформировать блок таблицы DMAC
  bool cIsDmacFull = IsDmacTableFull(lpDmacTable, lDmacTableSize, cPortConnection);
  int cSize = RemoveExtraConnections(lpDmacTable, lDmacTableSize, cPortConnection);
  if (cIsDmacFull) {
    Refresh_gDmacTable(); //Обновить gpDmacTable, gDmacRefreshTime
  } else {
    u32 cSysNow = sys_now();
    if ((cSysNow - gDmacRefreshTime) > DMAC_REFRESH_INTERVAL) {
      Refresh_gDmacTable(); //Обновить gpDmacTable, gDmacRefreshTime
    } else {
      if (cSize < lDmacTableSize) { // !!! Есть записи без подключения
        //
      }
    }
  }
}
 #endif
#endif

bool IsSnmpClientFound(uint32_t iIP) {
  gPortEnvCount = getArpTable(gaPortEnv);
  if (gPortEnvCount > 0) {
    for (int i = 0; i < gPortEnvCount; ++i) {
      if (gaPortEnv[i].IP == iIP) {
        return true;
      }
    }
  }
  return false;
}

extern void clearDMAC();
extern uint8_t cEmptyMAC[ETH_HWADDR_LEN];
void rarp_process(uint8_t iOper, uint8_t ** ippData, uint16_t * ipSize) {
  *ippData = NULL;
  *ipSize = 0;
  struct sPortEnv caPortEnv[ARP_TABLE_SIZE];
  int8_t cPortEnvCount;
  gPortEnvCount = getArpTable(gaPortEnv);
  if (iOper == RARP_REQ_FIND) {
//    clearDMAC(); //Очистить DMAC-таблицы в свитчах
    uint16_t cPortConnection = GetPortsConnection(), * cpPortConn;
#ifdef USE_SWITCH //Использовать свитч kmz8895
 #if (UTD_M == 0) //Для перестройки на UTD_M
    UpdateDmacTable(); //Обновить блок таблицы DMAC
 #endif
#endif
    aRarpData[0] = DEVICE_CODE;
    cpPortConn = (uint16_t *)&aRarpData[1];
    *cpPortConn = (cPortConnection/* | 0x8000*/);
    *ippData = aRarpData;
    *ipSize = 3;
  } else {
#ifdef USE_SWITCH //Использовать свитч kmz8895
 #if (UTD_M == 0) //Для перестройки на UTD_M
    UpdateDmacTable(); //Обновить блок таблицы DMAC
 #endif
#endif
    cPortEnvCount = 0;
    uint16_t cDmacSize = 0;
    uint8_t * cpData = aRarpData + 2;
#ifdef USE_SWITCH //Использовать свитч kmz8895
 #if (UTD_M == 0) //Для перестройки на UTD_M
    SDmacItem * cpDmacItem;
    for (int i = 0; i < gDmacTableSize; ++i) {
      cpDmacItem = &gpDmacTable[i];
      if (IsFirmMAC(&cpDmacItem->MAC[0])) {
  #ifdef LARGE_DMAC_ITEM //Размер элемента передаваемой таблицы DMAC = 7байт
        memcpy(cpData, &cpDmacItem->MAC[0], ETH_HWADDR_LEN);
        cpData += ETH_HWADDR_LEN;
  #else //Размер элемента передаваемой таблицы DMAC = 4байта
        memcpy(cpData, &cpDmacItem->MAC[3], 3);
        cpData += 3;
  #endif
        *cpData++ = cpDmacItem->Port;
        ++cDmacSize;
      } else {  //Записать во временную таблицу Edge
        if (cPortEnvCount < ARP_TABLE_SIZE) {
          if (memcmp(cpDmacItem->MAC, cEmptyMAC, ETH_HWADDR_LEN) != 0) {
            memcpy(caPortEnv[cPortEnvCount].MAC, cpDmacItem->MAC, ETH_HWADDR_LEN);
            caPortEnv[cPortEnvCount].IP = getIpFromArpTable(cpDmacItem->MAC).addr;
            caPortEnv[cPortEnvCount].Port = cpDmacItem->Port;
            caPortEnv[cPortEnvCount].IsEdge = 1;
            caPortEnv[cPortEnvCount].IsSingle = 0;
            caPortEnv[cPortEnvCount].Reserved = 0;
            ++cPortEnvCount;
          }
        }
      }
    }
 #endif
#endif
    uint16_t * cpSize = (uint16_t *)aRarpData;
    *cpSize = cDmacSize;
#ifdef LARGE_DMAC_ITEM //Размер элемента передаваемой таблицы DMAC = 7байт
    *cpSize |= 0x8000;
#endif
    *cpData++ = cPortEnvCount;
    if (cPortEnvCount > 0) { //Добавить таблицу Edge
      memcpy(cpData, (uint8_t *)caPortEnv, cPortEnvCount * sizeof(struct sPortEnv));
      cpData += cPortEnvCount * sizeof(struct sPortEnv);
    }
   
    *ippData = aRarpData;
    *ipSize = cpData - aRarpData;
  }
}

#endif /* LWIP_ARP || LWIP_ETHERNET */

