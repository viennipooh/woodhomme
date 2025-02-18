//******************************************************************************
// Имя файла    :       sw_reg.c
// Заголовок    :
// Автор        :       /Сосновских А.А.
// Дата         :       07.06.2018
//
//------------------------------------------------------------------------------
/**
Работа с регистрами свитча

**/
#include "sw_reg.h"
#include "switch.h"

extern ksz8895fmq_t *SW1;
#ifdef TWO_SWITCH_ITEMS
  extern ksz8895fmq_t sw[2];
  extern ksz8895fmq_t *SW2;
#else
  extern ksz8895fmq_t sw[1];
#endif

struct sIntPort gIntPort[PORT_NUMBER] = {
#ifdef TWO_SWITCH_ITEMS
  {&sw[0], 3},
  {&sw[0], 4},
  {&sw[1], 3},
  {&sw[1], 4},
  {&sw[0], 2},
  {&sw[1], 2},
  {&sw[1], 1},
#else
  {&sw[0], 1},
 #if ((UTD_M == 0) && (IIP == 0)) //Для перестройки на UTD_M
  {&sw[0], 2},
  {&sw[0], 3},
  #ifndef PIXEL
    {&sw[0], 4},    // потомучто у пикселя  только 3 порта
  #endif
 #endif
#endif
};

struct sIntPort * InternalPort(int iExtPort) { //1 .. 7
  if ((iExtPort <= 0) || (iExtPort > PORT_NUMBER))
    return NULL;
  return &gIntPort[iExtPort - 1];
}

int8_t ExternalPort(ksz8895fmq_t * iSw, int8_t iIntPort) {
  if ((iIntPort <= 0) || (iIntPort > 5))
    return 0;
  if (iSw == NULL)
    return 0;
  for (int i = 0; i < PORT_NUMBER; ++i) {
    if ((gIntPort[i].sw == iSw) && (gIntPort[i].port == iIntPort))
      return i + 1; //1 .. 7
  }
  return 0;
}

bool ReadSwRegBit(int8_t iBit, int8_t iReg, ksz8895fmq_t * pSW) {
  uint8_t cWRegs; //Вкл / Выкл бит в регистре свитча
  ksz8895fmq_read_registers(pSW, iReg, &cWRegs, 1);
  bool cValue = ((cWRegs >> iBit) & 0x01);
  return cValue;
}

void SetSwRegBit(bool iEnable, int8_t iBit, int8_t iReg, ksz8895fmq_t * pSW, bool iForced) {
  uint8_t cWRegs; //Вкл / Выкл бит в регистре свитча
  ksz8895fmq_read_registers(pSW, iReg, &cWRegs, 1);
  bool cIsDiff = ((cWRegs & (0x01 << iBit)) != ((iEnable & 0x01) << iBit));
  if (!cIsDiff && iForced) { //Записать сначала противоположное значение
    if (!iEnable) cWRegs |= (0x01 << iBit);
    else         cWRegs &= ~(0x01 << iBit);
    ksz8895fmq_write_registers(pSW, iReg, &cWRegs, 1);
  }
  if (cIsDiff || iForced) { //Записать требуемое значение
    if (iEnable) cWRegs |= (0x01 << iBit);
    else         cWRegs &= ~(0x01 << iBit);
    ksz8895fmq_write_registers(pSW, iReg, &cWRegs, 1);
  }
}

uint8_t ReadSwRegField(int8_t iStartBit, int8_t iBitSize, int8_t iReg, ksz8895fmq_t * pSW) {
  uint8_t cWRegs; //Записать значение поля в регистр свитча
  ksz8895fmq_read_registers(pSW, iReg, &cWRegs, 1);
  uint8_t cMask = ((1 << iBitSize) - 1) << iStartBit;
  uint8_t cRegValue = (cWRegs & cMask) >> iStartBit;
  return cRegValue;
}

void SetSwRegField(uint8_t iValue, int8_t iStartBit, int8_t iBitSize, int8_t iReg, ksz8895fmq_t * pSW, bool iForced) {
  uint8_t cWRegs; //Записать значение поля в регистр свитча
  ksz8895fmq_read_registers(pSW, iReg, &cWRegs, 1);
  uint8_t cMask = ((1 << iBitSize) - 1) << iStartBit;
  uint8_t cRegValue = (cWRegs & cMask);
  uint8_t cValue = ((iValue << iStartBit) & cMask);
  bool cIsDiff = (cRegValue != cValue);
  if (!cIsDiff && iForced) { //Записать сначала противоположное значение
    cRegValue = (~cValue & cMask);
    ksz8895fmq_write_registers(pSW, iReg, &cRegValue, 1);
  }
  if (cIsDiff || iForced) { //Записать требуемое значение
    cWRegs &= !cMask;
    cWRegs |= cValue;
    ksz8895fmq_write_registers(pSW, iReg, &cWRegs, 1);
  }
}

uint8_t gaPortReg2[4];

 //Вкл / Выкл бит (биты) в регистре порта (iKey = биты регистра, которые Вкл/Выкл одновременно)
bool SetPortKey(ksz8895fmq_t * pSW, bool iEnable, int8_t iPort, int8_t iReg, uint8_t iKey) {
  if (iKey == 0)  return false;
  if ((iReg < 0) || (iReg > 15)) return false;
  if ((iPort < 0) || (iPort > 4)) return false;

  int8_t cReg = 0x10 + (iPort * 0x10) + iReg;
  uint8_t cWRegs = 0;
  ksz8895fmq_read_registers(pSW, cReg, &cWRegs, 1);
  if (iReg == 2) {
    gaPortReg2[iPort] = cWRegs;
  }
  bool Rez = false;
  if (iEnable && (cWRegs & iKey != iKey))
    Rez = true;
  if (!iEnable && (cWRegs & iKey != 0))
    Rez = true;
  if (iEnable) cWRegs |= iKey;
  else         cWRegs &= ~iKey;
  ksz8895fmq_write_registers(pSW, cReg, &cWRegs, 1);
  if (iReg == 2) {
    cWRegs = ReadPortField(0, 8, iPort, 2, pSW);
    gaPortReg2[iPort] = cWRegs;
  }
  return Rez;
}

uint8_t ReadPortField(int8_t iStartBit, int8_t iBitSize, int8_t iPort, int8_t iReg, ksz8895fmq_t * pSW) {
  uint8_t cWRegs; //Записать значение поля в регистр свитча
  if ((iReg < 0) || (iReg > 15)) return 0;
  if ((iPort < 0) || (iPort > 4)) return 0;

  int8_t cReg = 0x10 + (iPort * 0x10) + iReg;
  ksz8895fmq_read_registers(pSW, cReg, &cWRegs, 1);
  uint8_t cMask = ((1 << iBitSize) - 1) << iStartBit;
  uint8_t cRegValue = (cWRegs & cMask) >> iStartBit;
  return cRegValue;
}

void SetPortField(uint8_t iValue, int8_t iStartBit, int16_t iBitSize, int8_t iPort, int8_t iReg, ksz8895fmq_t * pSW, bool iForced) {
  uint8_t cWRegs; //Записать значение поля в регистр свитча
  if ((iReg < 0) || (iReg > 15)) return;
  if ((iPort < 0) || (iPort > 4)) return;

  int8_t cReg = 0x10 + (iPort * 0x10) + iReg;
  ksz8895fmq_read_registers(pSW, cReg, &cWRegs, 1);
  uint16_t cMask = ((1 << iBitSize) - 1) << iStartBit;
  uint8_t cRegValue = (cWRegs & cMask);
  uint8_t cValue = ((iValue << iStartBit) & cMask);
  bool cIsDiff = (cRegValue != cValue);
  if (!cIsDiff && iForced) { //Записать сначала противоположное значение
    cRegValue = (~cValue & cMask);
    ksz8895fmq_write_registers(pSW, cReg, &cRegValue, 1);
  }
  if (cIsDiff || iForced) { //Записать требуемое значение
    cWRegs &= !cMask;
    cWRegs |= cValue;
    ksz8895fmq_write_registers(pSW, cReg, &cWRegs, 1);
  }
}


//======= DMAC =========

//Прочитать s_DMACItem из свитча pSW по DMacIdx
// !!! Уже есть чтение DMAC, и обновление в dmac_table, dmac_table_size
struct s_DMACItem ReadDMacEntry(uint16_t iDMacIdx, ksz8895fmq_t * pSW) {
  struct s_DMACItem cDMACItem;
  uint8_t cCmd[2], cBuff[9];

  memset((uint8_t *)&cDMACItem, 0, sizeof(cDMACItem));
  cCmd[0] = ((iDMacIdx >> 8) & 0x3f) | DMAC_TABLE_READ;
  cCmd[1] = iDMacIdx & 0xff;
  ksz8895fmq_write_registers(pSW, INDIRECT_OPERATION_KIND, &cCmd[0], 2);
  uint8_t cReady = 1;
  int16_t cCount = 20;
  while (cReady) {
    ksz8895fmq_read_registers( pSW, INDIRECT_OPERATION_REGS_DMAC, &cBuff[0], 9);
    cReady = cBuff[2] & 0x80;
    vTaskDelay(5);
    if (--cCount <= 0)
      break;
  }
  if (!cReady) { //Есть чтение - заполнить cDMACItem
    for (int i = 0; i < 6; ++i) {
      cDMACItem.MAC[i] = cBuff[3 + i];
    }
    cDMACItem.FID = cBuff[2] & 0x7f;
    cDMACItem.SRCPort = cBuff[1] & 0x07;
    cDMACItem.TimeStamp = (cBuff[1] >> 3) & 0x03;
    cDMACItem.ValidNumber = (cBuff[1] >> 5) & 0x07;
    cDMACItem.ValidNumber |= (cBuff[0] & 0x7f) << 3;
    cDMACItem.MACEmpty = (cBuff[0] & 0x80) >> 7;
  }
  return cDMACItem;
}

//======= SMAC =========

//Прочитать s_SMACItem из свитча pSW по DMacIdx
struct s_SMACItem ReadSMacEntry(uint16_t iSMacIdx, ksz8895fmq_t * pSW) {
  struct s_SMACItem cSMACItem;
  uint8_t cCmd[2], cBuff[8];

  memset((uint8_t *)&cSMACItem, 0, sizeof(cSMACItem));
  cCmd[0] = SMAC_TABLE_READ;
  cCmd[1] = iSMacIdx & 0x1f;
  ksz8895fmq_write_registers(pSW, INDIRECT_OPERATION_KIND, &cCmd[0], 2);
  ksz8895fmq_read_registers( pSW, INDIRECT_OPERATION_REGS_SMAC, &cBuff[0], 8);
//  if (!cReady) { //Есть чтение - заполнить cDMACItem
    for (int i = 0; i < 6; ++i) {
      cSMACItem.MAC[i] = cBuff[2 + i];
    }
    cSMACItem.FwdPorts  =  cBuff[1] & 0x1f;
    cSMACItem.Valid     = (cBuff[1] & 0x20) >> 5;
    cSMACItem.Override  = (cBuff[1] & 0x40) >> 6;
    cSMACItem.UseFID    = (cBuff[0] & 0x01);
    cSMACItem.FID       = (cBuff[0] & 0xfe) >> 1;
//  }
  return cSMACItem;
}

//Записать s_SMACItem в свитч pSW по iSMacIdx
void WriteSMacEntry(struct s_SMACItem * ipSMACItem, uint16_t iSMacIdx, ksz8895fmq_t * pSW) {
  uint8_t cCmd[2], cBuff[8];

  cCmd[0] = SMAC_TABLE_WRITE;
  cCmd[1] = iSMacIdx & 0x1f;

  for (int i = 0; i < 6; ++i) {
    cBuff[2 + i] = ipSMACItem->MAC[i];
  }
  cBuff[1] = (ipSMACItem->FwdPorts & 0x1f) | ((ipSMACItem->Valid & 0x01) << 5) |
             ((ipSMACItem->Override & 0x01) << 6) |
             ((ipSMACItem->UseFID & 0x01) << 7);
  cBuff[0] = (ipSMACItem->FID & 0x7f);
  ksz8895fmq_write_registers(pSW, INDIRECT_OPERATION_REGS_SMAC, &cBuff[0], 8);
  ksz8895fmq_write_registers(pSW, INDIRECT_OPERATION_KIND, &cCmd[0], 2);

}

void clearDMAC() { //Очистить таблицы DMAC в свитчах
  for (int i = 0; i < 4; ++i)
    SetPortKey(SW1, true, i, 2, 0x01);
  SetSwRegBit(true, 5, 2, SW1, true);
  vTaskDelay(100);
  for (int i = 0; i < 4; ++i)
    SetPortKey(SW1, false, i, 2, 0x01);
#ifdef TWO_SWITCH_ITEMS
  for (int i = 0; i < 4; ++i)
    SetPortKey(SW2, true, i, 2, 0x01);
  SetSwRegBit(true, 5, 2, SW2, true);
  vTaskDelay(100);
  for (int i = 0; i < 4; ++i)
    SetPortKey(SW2, false, i, 2, 0x01);
#endif
}


