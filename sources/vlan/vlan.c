//******************************************************************************
// Имя файла    :       vlan.c
// Заголовок    :       
// Автор        :       /Сосновских А.А.
// Дата         :       07.06.2018
//
//------------------------------------------------------------------------------
/**
Средства организации виртуальных сетей по IEEE 802.1q

**/
#include "vlan.h"
#include "switch.h"

extern ksz8895fmq_t *SW1;
#ifdef TWO_SWITCH_ITEMS
extern ksz8895fmq_t *SW2;
#endif

void ReadVlanSet(struct sVlanSet * iVlanSet, uint8_t iVlanSetIdx, ksz8895fmq_t * pSW) {
  uint8_t cWRegs[2];
  cWRegs[0] = VLAN_TABLE_READ;
  cWRegs[1] = iVlanSetIdx;
  ksz8895fmq_write_registers(pSW, INDIRECT_OPERATION_KIND, &cWRegs[0], 2);
  ksz8895fmq_read_registers(pSW, INDIRECT_OPERATION_REGS_VLAN, &iVlanSet->u8[0], 7);
}

void WriteVlanSet(struct sVlanSet * iVlanSet, uint8_t iVlanSetIdx, ksz8895fmq_t * pSW) {
  uint8_t cWRegs[2];
  cWRegs[0] = VLAN_TABLE_WRITE;
  cWRegs[1] = iVlanSetIdx;
  ksz8895fmq_write_registers(pSW, INDIRECT_OPERATION_REGS_VLAN, &iVlanSet->u8[0], 7);
  ksz8895fmq_write_registers(pSW, INDIRECT_OPERATION_KIND, &cWRegs[0], 2);
}

bool GetVlanEntry(struct sVlanEntry * iVlanEntry, struct sVlanSet * iVlanSet, uint8_t iVlanIdx) {
  //Получить VlanEntry из считанного блока
  struct sVlanEntry aVlanEntry[4];
  aVlanEntry[0].u16 = (iVlanSet->u8[5] & 0x1f) | ((iVlanSet->u8[6] & 0xff) << 5);
  aVlanEntry[1].u16 = (iVlanSet->u8[3] & 0x03) | ((iVlanSet->u8[4] & 0xff) << 2) | ((iVlanSet->u8[5] & 0xe0) << 5);
  aVlanEntry[2].u16 = (iVlanSet->u8[2] & 0x7f) | ((iVlanSet->u8[3] & 0xfc) << 5);
  aVlanEntry[3].u16 = (iVlanSet->u8[0] & 0x0f) | ((iVlanSet->u8[1] & 0xff) << 4) | ((iVlanSet->u8[2] & 0x80) << 5);
  
  iVlanEntry->u16 = 0;
  switch (iVlanIdx) {
  case 0:
    iVlanEntry->u16 = aVlanEntry[0].u16;
    break;
  case 1:
    iVlanEntry->u16 = aVlanEntry[1].u16;
    break;
  case 2:
    iVlanEntry->u16 = aVlanEntry[2].u16;
    break;
  case 3:
    iVlanEntry->u16 = aVlanEntry[3].u16;
    break;
  default:
    return false;
  }
  return true;
}

bool SetVlanEntry(struct sVlanEntry * iVlanEntry, struct sVlanSet * iVlanSet, uint8_t iVlanIdx) {
  //Запись VlanEntry в считанный VlanSet (4 шт. VlanEntry)
  uint8_t cByte;
  switch (iVlanIdx) {
  case 0:
    cByte = iVlanEntry->u16 & 0x1f;
    iVlanSet->u8[5] &= ~0x1f;
    iVlanSet->u8[5] |= cByte;
    cByte = (iVlanEntry->u16 >> 5) & 0xff;
    iVlanSet->u8[5] &= ~0xff;
    iVlanSet->u8[5] |= cByte;
    break;
  case 1:
    cByte = iVlanEntry->u16 & 0x03;
    iVlanSet->u8[3] &= ~0x03;
    iVlanSet->u8[3] |= cByte;
    cByte = (iVlanEntry->u16 >> 2) & 0xff;
    iVlanSet->u8[4] &= ~0xff;
    iVlanSet->u8[4] |= cByte;
    cByte = (iVlanEntry->u16 >> 5) & 0xe0;
    iVlanSet->u8[5] &= ~0xe0;
    iVlanSet->u8[5] |= cByte;
    break;
  case 2:
    cByte = iVlanEntry->u16 & 0x7f;
    iVlanSet->u8[2] &= ~0x7f;
    iVlanSet->u8[2] |= cByte;
    cByte = (iVlanEntry->u16 >> 5) & 0xfc;
    iVlanSet->u8[3] &= ~0xfc;
    iVlanSet->u8[3] |= cByte;
    break;
  case 3:
    cByte = iVlanEntry->u16 & 0x0f;
    iVlanSet->u8[0] &= ~0x0f;
    iVlanSet->u8[0] |= cByte;
    cByte = (iVlanEntry->u16 >> 4) & 0xff;
    iVlanSet->u8[1] &= ~0xff;
    iVlanSet->u8[1] |= cByte;
    cByte = (iVlanEntry->u16 >> 5) & 0x80;
    iVlanSet->u8[2] &= ~0x80;
    iVlanSet->u8[2] |= cByte;
    break;
  default:
    return false;
  }
  return true;
}

void SetSwVlan(bool iEnable, ksz8895fmq_t * pSW) { //Вкл / Выкл VLAN свитча
  uint8_t cWRegs;
  ksz8895fmq_read_registers(pSW, 5, &cWRegs, 1);
  if (iEnable) cWRegs |= 0x80;
  else         cWRegs &= ~0x80;
  ksz8895fmq_write_registers(pSW, 5, &cWRegs, 1);
}

//void SetVlan(bool iEnable) { //Вкл / Выкл VLAN
//  SetSwVlan(iEnable, SW1);
//  SetSwVlan(iEnable, SW2);
//}


//Прочитать sVlanEntry из свитча pSW по VlanId
struct sVlanEntry ReadVlanEntry(uint16_t iVlanId, ksz8895fmq_t * pSW) {
  struct sVlanEntry cVlanEntry;
  struct sVlanSet cVlanSet;
  cVlanSet.u64 = 0;
  int16_t cVlanSetIdx = iVlanId / 4;
  uint8_t cVlanIdx = iVlanId % 4;
  ReadVlanSet(&cVlanSet, cVlanSetIdx, pSW);
  GetVlanEntry(&cVlanEntry, &cVlanSet, cVlanIdx);
  
  return cVlanEntry;
}

//Записать sVlanEntry в свитч pSW по VlanId
void WriteVlanEntry(uint16_t iVlanId, struct sVlanEntry iVlanEntry, ksz8895fmq_t * pSW) {
  struct sVlanSet cVlanSet;
  int16_t cVlanSetIdx = iVlanId / 4;
  uint8_t cVlanIdx = iVlanId % 4;
  ReadVlanSet(&cVlanSet, cVlanSetIdx, pSW);
  SetVlanEntry(&iVlanEntry, &cVlanSet, cVlanIdx);
  WriteVlanSet(&cVlanSet, cVlanSetIdx, pSW);
}

//Записать VlanID в порт iPort свитча pSW
void SetPortVlanID(ksz8895fmq_t * pSW, int8_t iPort, uint16_t iVlanID) {
  if ((iPort < 0) || (iPort > 4)) return;
  
  int8_t cReg = 0x10 + (iPort * 0x10) + 3;
  uint8_t cWRegs[2] = {0, 0};
  ksz8895fmq_read_registers(pSW, cReg, &cWRegs[0], 2);
  cWRegs[0] &= 0xf0;
  cWRegs[0] |= (iVlanID >> 8) & 0x0f;
  cWRegs[1] = iVlanID & 0x0f;
  ksz8895fmq_write_registers(pSW, cReg, &cWRegs[0], 2);
}

//Записать Members в порт iPortIdx свитча pSW
#define MEMBERS_MASK (0x1f)
void SetPortMembers(ksz8895fmq_t * pSW, int8_t iPortIdx, uint8_t iMembers) {
  if ((iPortIdx < 0) || (iPortIdx > 4)) return;
  uint8_t cMembers = iMembers & MEMBERS_MASK;
  
  int8_t cReg = 0x10 + (iPortIdx * 0x10) + 1;
  uint8_t cWRegs[2] = {0, 0};
  ksz8895fmq_read_registers(pSW, cReg, &cWRegs[0], 1);
  cWRegs[0] &= ~MEMBERS_MASK;
  cWRegs[0] |= cMembers;
//  cWRegs[0] &= ~(1 << iPortIdx); //Убрать собственный порт (?)
  ksz8895fmq_write_registers(pSW, cReg, &cWRegs[0], 1);
}

void SetPortToVlan(ksz8895fmq_t * pSW, int8_t iPort, struct sVlan iVlan) {
  SetPortVlanID(pSW, iPort, iVlan.VlanID);
  SetPortMembers(pSW, iPort, iVlan.Members);
  SetPortKey(pSW, true, iPort, 0, VLAN_TAG_REMOVE);
  
}

#if (MKPSH10 != 0)
void SetDevMode(enum eDevMode iDevMode) {
  switch (iDevMode) {
  case dmSwitch:
    SetPortMembers(SW1, 0, 0x1f); //Порты - члены 1 .. 5 для порта 1
    SetPortMembers(SW1, 1, 0x1f); //Порты - члены 1 .. 5 для порта 2
    SetPortMembers(SW1, 2, 0x1f); //Порты - члены 1 .. 5 для порта 3
    SetPortMembers(SW1, 3, 0x1f); //Порты - члены 1 .. 5 для порта 4

    SetPortMembers(SW2, 0, 0x1f); //Порты - члены 1 .. 5 для порта 1
    SetPortMembers(SW2, 1, 0x1f); //Порты - члены 1 .. 5 для порта 2
    SetPortMembers(SW2, 2, 0x1f); //Порты - члены 1 .. 5 для порта 3
    SetPortMembers(SW2, 3, 0x1f); //Порты - члены 1 .. 5 для порта 4
    break;
  case dmDoubleRoute:
    SetPortMembers(SW1, 0, 0x00); //Порты - члены 1 и 5 для порта 1
    SetPortMembers(SW1, 1, 0x1e); //Порты - члены 2 .. 5 для порта 2
    SetPortMembers(SW1, 2, 0x1e); //Порты - члены 2 .. 5 для порта 3
    SetPortMembers(SW1, 3, 0x1e); //Порты - члены 2 .. 5 для порта 4

    SetPortMembers(SW2, 0, 0x0f); //Порты - члены 1 .. 4 для порта 1
    SetPortMembers(SW2, 1, 0x0f); //Порты - члены 1 .. 4 для порта 2
    SetPortMembers(SW2, 2, 0x0f); //Порты - члены 1 .. 4 для порта 3
    SetPortMembers(SW2, 3, 0x0f); //Порты - члены 1 .. 4 для порта 4
    break;
  default:
    break;
  }
}
#endif
#if (IMC_FTX_MC != 0)
#include "settings.h"
int8_t gaPort[4];
void SetSubnets(struct sVlanCfgDesc * ipVlanCfgDesc) {
  //Установить подсети по ipVlanCfgDesc
  bool cEmptyVlans = true;
  for (int i = 0; i < MAX_VLAN_NUM; ++i) {
    if (ipVlanCfgDesc->aVlanItem[i].Members != 0) {
      cEmptyVlans = false;
      break;
    }
  }
    memset(gaPort, 0x1f, sizeof(gaPort)); //Заполнить все Members портов 0x1f
  if (!cEmptyVlans) {
    //Вычислить Subnet для каждого внешнего порта
    for (int i = 0; i < 4; ++i) {
      uint8_t cMembers = 0;
      for (int j = 0; j < MAX_VLAN_NUM; ++j) {
        if (ipVlanCfgDesc->aVlanItem[j].Members & (1 << i))
          cMembers |= ipVlanCfgDesc->aVlanItem[j].Members;
      }
      gaPort[i] = cMembers | 0x10;
    }
  }
  for (int i = 0; i < 4; ++i) {
    //Записать Members в порт
    SetPortMembers(SW1, i, gaPort[i]); //Порты - члены 1 .. 5 для порта 1
  }
}
#endif //IMC_FTX_MC

//====== Таблица VLAN
#define VLAN_ENTRY_NUM (VLAN_TAB_SIZE >> 2)
#define VLAN_PART_SIZE (128)
struct sVlanEntry gaVlanEntry[VLAN_TAB_SIZE];
uint16_t gVlanBlockIdx = 0;
bool gIsVlanTabFull = false;

void InitVLAN() {
  //Очистка aVLAN
  memset((uint8_t *)&gaVlanEntry[0], 0, sizeof(gaVlanEntry));
// !!!VLAN TRY+
  //Установка подсетей VLAN
  SetSwVlan(false, SW1);
#ifdef TWO_SWITCH_ITEMS
  SetSwVlan(false, SW2);

  struct sVlanEntry VlanEntry;
  VlanEntry = ReadVlanEntry(2, SW1);
  VlanEntry = ReadVlanEntry(4, SW1);
  VlanEntry = ReadVlanEntry(8, SW1);
  VlanEntry.FID     = 0;
  VlanEntry.Members = 0x0;
  VlanEntry.Valid   = 0;
//  WriteVlanEntry(2, VlanEntry, SW1);
  
  VlanEntry = ReadVlanEntry(0, SW2);
  VlanEntry = ReadVlanEntry(4, SW2);
  VlanEntry = ReadVlanEntry(8, SW2);

  VlanEntry.FID     = 2;
  VlanEntry.Members = 0x02;
  VlanEntry.Valid   = 1;
  VlanEntry.u16 = 0;
  WriteVlanEntry(2, VlanEntry, SW2);
  VlanEntry.u16 = 0;
  vTaskDelay(500);
//  VlanEntry = ReadVlanEntry(2, SW2);
//  SetSwVlan(true, SW1);
  SetPortKey(SW2, true, 1, 0, VLAN_TAG_REMOVE);
  SetPortKey(SW2, true, 2, 0, VLAN_TAG_REMOVE);
//  SetPortKey(SW2, true, 2, 0, VLAN_TAG_INSERT);
  SetPortKey(SW2, true, 5, 0, VLAN_TAG_REMOVE);
  vTaskDelay(500);
  SetSwVlan(false, SW2);
#endif
// !!!VLAN TRY
}

//Обновление (по частям) таблицы VLAN
void RefreshVlanTablePart() {
#ifdef TWO_SWITCH_ITEMS
  struct sVlanSet cVlanSet;
#endif
  if (gVlanBlockIdx >= VLAN_ENTRY_NUM) {
    gVlanBlockIdx = 0;
    gIsVlanTabFull = true;
  }
#ifdef TWO_SWITCH_ITEMS
  for (int i = 0; i < VLAN_PART_SIZE; ++i) {
    cVlanSet.u64 = 0;
    ReadVlanSet(&cVlanSet, gVlanBlockIdx + i, SW2);
    for (int j = 0; j < 4; ++j) { 
      GetVlanEntry(&gaVlanEntry[(gVlanBlockIdx + i) * 4 + j], &cVlanSet, j);
    }
  }
#endif
  gVlanBlockIdx += VLAN_PART_SIZE;
}

//Дескриптор настройки конфигурации VLAN
struct sVlanCfgDesc  gaVlanCfgDesc[2];

struct sVlanCfgDesc  gTmpVlanCfgDesc;

// Заполнение gVlanCfgDesc
void FillVlanDescFromSwitch(int iSwIdx) {
  if ((iSwIdx < 0) || (iSwIdx > 1))
    return;
  ksz8895fmq_t * pSW = &SW1[iSwIdx];
  uint8_t cBuff[8];
  //Прочитать общие флаги
  ksz8895fmq_read_registers( pSW, 4, &cBuff[0], 3);
  gaVlanCfgDesc[iSwIdx].VlanOn   = (cBuff[1] >> 7) & 0x01;  //5:7
  gaVlanCfgDesc[iSwIdx].UniDis   = (cBuff[0] >> 7) & 0x01;  //4:7
  gaVlanCfgDesc[iSwIdx].NulToVid = (cBuff[2] >> 3) & 0x01;  //6:3
  //Прочитать уставки портов [5]
  struct sPortVlanDesc * cpPortVlanDesc;
  for (int i = 0; i < 5; ++i) {
    cpPortVlanDesc = &gaVlanCfgDesc[iSwIdx].aPortDesc[i];
    ksz8895fmq_read_registers( pSW, 0x10 * (i + 1), &cBuff[0], 5);
    cpPortVlanDesc->VlanTagIns      = (cBuff[0] >> 2) & 0x01;  //Вставка в пакет тега VLAN с VID порта на входе
    cpPortVlanDesc->VlanTagDel      = (cBuff[0] >> 1) & 0x01;  //Удаление тега VLAN из пакета на выходе, принятого с тегом
    cpPortVlanDesc->NoVlanMemberDel = (cBuff[2] >> 6) & 0x01;  //Удаление пакетов, если входной порт не член VLAN
    cpPortVlanDesc->NoInVidDel      = (cBuff[2] >> 5) & 0x01;  //Удаление пакетов c VID не равным VID входного порта
    cpPortVlanDesc->Reserved        = 0;  //Резерв
    cpPortVlanDesc->VlanMembers     = (cBuff[1] >> 0) & 0x1f;  //Порты - члены VLAN для этого порта
    cpPortVlanDesc->DefVlanTag      = (cBuff[3] << 8) | cBuff[4];  //Тег VLAN по умолчанию для порта
  }
  //Прочитать ненулевые VLAN[10]
  struct sVlanItemDesc * cpVlanItemDesc;
  uint8_t cIdx = 0;
  for (int i = 0; i < VLAN_TAB_SIZE; ++i) {
    if (gaVlanEntry[i].u16 == 0)
      continue;
    cpVlanItemDesc = &gaVlanCfgDesc[iSwIdx].aVlanItem[cIdx];
    cpVlanItemDesc->VID     = i;
    cpVlanItemDesc->VID     = gaVlanEntry[i].FID;
    cpVlanItemDesc->Members = gaVlanEntry[i].Members;
    cpVlanItemDesc->Valid   = gaVlanEntry[i].Valid;
    ++cIdx;
    if (cIdx >= MAX_VLAN_NUM)
      break;
  }
  gaVlanCfgDesc[iSwIdx].VlanNum = cIdx;
}

#ifdef TWO_SWITCH_ITEMS
extern ksz8895fmq_t sw[2];
#else
extern ksz8895fmq_t sw[1];
#endif


//===== Служебные функции

//Обнулить (очистить) VlanEntry
void ClearVlanEntry(struct sVlanEntry * iVlanEntry) {
  iVlanEntry->FID = 0;
  iVlanEntry->Members = 0x1f;
  iVlanEntry->Valid = 0;
}

// ==== Дескрипторы и уставки


//Дескриптор устройства (МКПШ, и т.п.)
struct sConnDevDesc DevDesc = {
  .Num = 0,
  .SwitchDesc = { //2 свитча в устройстве
    { .Num = 0,
    .PortDesc = {
      { .SwNum =  1, .DevNum =  5, .ConnType = pctExt , .ConnSwNum = -1 },
      { .SwNum =  2, .DevNum = -1, .ConnType = pctExch, .ConnSwNum =  2 },
      { .SwNum =  3, .DevNum =  1, .ConnType = pctExt , .ConnSwNum = -1 },
      { .SwNum =  4, .DevNum =  2, .ConnType = pctExt , .ConnSwNum = -1 },
      { .SwNum =  5, .DevNum = -1, .ConnType = pctCpu , .ConnSwNum = -1 },
      { .SwNum = -1, .DevNum = -1, .ConnType = pctNone, .ConnSwNum = -1 },
      { .SwNum = -1, .DevNum = -1, .ConnType = pctNone, .ConnSwNum = -1 },
      { .SwNum = -1, .DevNum = -1, .ConnType = pctNone, .ConnSwNum = -1 },
    },
    },
    { .Num = 1,
    .PortDesc = {
      { .SwNum =  1, .DevNum =  6, .ConnType = pctExt , .ConnSwNum = -1 },
      { .SwNum =  2, .DevNum = -1, .ConnType = pctExch, .ConnSwNum =  1 },
      { .SwNum =  3, .DevNum =  3, .ConnType = pctExt , .ConnSwNum = -1 },
      { .SwNum =  4, .DevNum =  4, .ConnType = pctExt , .ConnSwNum = -1 },
      { .SwNum =  5, .DevNum =  7, .ConnType = pctCpu , .ConnSwNum = -1 },
      { .SwNum = -1, .DevNum = -1, .ConnType = pctNone, .ConnSwNum = -1 },
      { .SwNum = -1, .DevNum = -1, .ConnType = pctNone, .ConnSwNum = -1 },
      { .SwNum = -1, .DevNum = -1, .ConnType = pctNone, .ConnSwNum = -1 },
    }
    },
  }
};

//Дескрипторы VLAN
//default = выделение VLAN для связи портов МКПШ 5 и 7
//        вторая VLAN - все порты кроме 7. полнофункциональный свитч кроме порта 7
struct sVlanDesc aVlanDesc[2] = {
  { //VLAN 1
    .VlanId     = 2,
    .Members    = 0x42, // , , , ,5, ,7
    .VlanEntry = {
      { //Свитч 1
        .FID      = 2,
        .Members  = 0x0a, //1,2, , ,
        .Valid    = 1,
      },
      { //Свитч 2
        .FID      = 2,
        .Members  = 0x12, // ,2, , ,5
        .Valid    = 1,
      },
      //Резерв для других устройств
      {
        .FID      = 0,
        .Members  = 0x1f, //1,2,3,4,5
        .Valid    = 0,
      },
      {
        .FID      = 0,
        .Members  = 0x1f, //1,2,3,4,5
        .Valid    = 0,
      },
    },
  },
  
  { //VLAN 2
    .VlanId     = 3,
    .Members    = 0x3f, //1,2,3,4,5,6,
    .VlanEntry = {
      { //Свитч 1
        .FID      = 3,
        .Members  = 0x1f, //1,2,3,4,5
        .Valid    = 1,
      },
      { //Свитч 2
        .FID      = 3,
        .Members  = 0x0f, //1,2,3,4,
        .Valid    = 1,
      },
      //Резерв для других устройств
      {
        .FID      = 0,
        .Members  = 0x1f, //1,2,3,4,5
        .Valid    = 0,
      },
      {
        .FID      = 0,
        .Members  = 0x1f, //1,2,3,4,5
        .Valid    = 0,
      },
    },
  },
  
};

