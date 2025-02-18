//******************************************************************************
// Имя файла    :       stp.cpp
// Заголовок    :       
// Автор        :       Сосновских А.А.
// Дата         :       07.08.2018
//
//------------------------------------------------------------------------------
/**
Средства поддержки протокола STP по IEEE 802.1d

**/

#include "settings.h"
#include "stp.h"
#include "switch.h"

const struct ethrstp_hdr gRstpHdr;

#if (!NO_SYS)
xQueueHandle gQueueStp;

//Инициализация блока RSTP/STP
#if (MKPSH10 != 0)
  extern bool IsHwVersion2();
#endif
void CreateTaskRstp() {
  //Отдельная задача, для RSTP
#if (MKPSH10 != 0)
  if (!IsHwVersion2())
#endif
  {
    xTaskCreate(rstp_task, (int8_t *) "Rstp", configMINIMAL_STACK_SIZE * 2, NULL, RSTP_TASK_PRIO, NULL);
    gQueueStp = xQueueCreate(4, sizeof(gRstpHdr) + 1);
  }
}
#else
struct sQueue gQueueStp;

//Инициализация блока RSTP/STP
void xQueueInit(struct sQueue * ipQueue);
bool xQueueSend(struct sQueue * ipQueue, uint8_t * ipItem);
bool xQueueReceive(struct sQueue * ipQueue, uint8_t * ipItem);

void CreateTaskRstp() {
  xQueueInit(&gQueueStp);
}
#endif

extern ksz8895fmq_t *SW1;
#ifdef TWO_SWITCH_ITEMS
extern ksz8895fmq_t *SW2;
#endif


extern struct netif gnetif;

#include "ksz8895fmq.h"
#ifdef TWO_SWITCH_ITEMS
extern ksz8895fmq_t sw[2];
#else
extern ksz8895fmq_t sw[1];
#endif

//Объекты Дескрипторы порта, Порты, Мост
struct sRstpBridgeDesc gaRstpBridgeDesc[BRIDGE_NUMBER] @ ".sram" = { //Блок уставок и управления для мостов
  {
    .Idx = 0,
    .SW = &sw[0],
    .aBPortDesc = { //
      { //1
        .PortIdx = 0,
        .pRstpBridgeDesc = &gaRstpBridgeDesc[0],
      },
      { //2
        .PortIdx = 1,
        .pRstpBridgeDesc = &gaRstpBridgeDesc[0],
      },
      { //3
        .PortIdx = 2,
        .pRstpBridgeDesc = &gaRstpBridgeDesc[0],
      },
      { //4
        .PortIdx = 3,
        .pRstpBridgeDesc = &gaRstpBridgeDesc[0],
      }
    }
  }
};

struct sRstpBridge gaRstpBridge[BRIDGE_NUMBER] @ ".sram" = { //Объект моста
  {
    .RootPortIdx = -1, //Нет Root порта
    .TailTagOn = false,
  }
};

//Загрузка параметров RSTP из Flash
void LoadRstpCfg(struct sRstpBridgeDesc * ipRstpBridgeDescSrc) {
  for (int i = 0; i < BRIDGE_NUMBER; ++i) {
    gaRstpBridgeDesc[i].RstpOn        = ipRstpBridgeDescSrc[i].RstpOn;
    gaRstpBridgeDesc[i].FirmId        = ipRstpBridgeDescSrc[i].FirmId;
    gaRstpBridgeDesc[i].forceVersion  = ipRstpBridgeDescSrc[i].forceVersion;
    gaRstpBridgeDesc[i].ForwardDelay  = ipRstpBridgeDescSrc[i].ForwardDelay;
    memcpy(&gaRstpBridgeDesc[i].MAC, ipRstpBridgeDescSrc[i].MAC, ETH_HWADDR_LEN);
    gaRstpBridgeDesc[i].MaxAge        = ipRstpBridgeDescSrc[i].MaxAge;
    gaRstpBridgeDesc[i].HelloTime     = ipRstpBridgeDescSrc[i].HelloTime;
    gaRstpBridgeDesc[i].Priority      = ipRstpBridgeDescSrc[i].Priority;
    gaRstpBridgeDesc[i].TxHoldCount   = ipRstpBridgeDescSrc[i].TxHoldCount;
    for (int j = 0; j < PORT_COUNT; ++j) {
      gaRstpBridgeDesc[i].aBPortDesc[j].adminEdge     = ipRstpBridgeDescSrc[i].aBPortDesc[j].adminEdge;
      gaRstpBridgeDesc[i].aBPortDesc[j].autoEdge      = ipRstpBridgeDescSrc[i].aBPortDesc[j].autoEdge;
      gaRstpBridgeDesc[i].aBPortDesc[j].ForcePriority = ipRstpBridgeDescSrc[i].aBPortDesc[j].ForcePriority;
      gaRstpBridgeDesc[i].aBPortDesc[j].MigrateTime   = ipRstpBridgeDescSrc[i].aBPortDesc[j].MigrateTime;
      gaRstpBridgeDesc[i].aBPortDesc[j].rstpVersion   = ipRstpBridgeDescSrc[i].aBPortDesc[j].rstpVersion;
      gaRstpBridgeDesc[i].aBPortDesc[j].stpVersion    = ipRstpBridgeDescSrc[i].aBPortDesc[j].stpVersion;
      gaRstpBridgeDesc[i].aBPortDesc[j].portEnabled   = ipRstpBridgeDescSrc[i].aBPortDesc[j].portEnabled;
      gaRstpBridgeDesc[i].aBPortDesc[j].VlanId        = ipRstpBridgeDescSrc[i].aBPortDesc[j].VlanId;
    }
  }
}

//Загрузка параметров RSTP из Flash
void CopyRstpCfg(struct sRstpBridgeDesc * ipRstpBridgeDescSrc, struct sRstpBridgeDesc * ipRstpBridgeDescDst) {
  for (int i = 0; i < BRIDGE_NUMBER; ++i) {
    ipRstpBridgeDescDst[i].RstpOn        = ipRstpBridgeDescSrc[i].RstpOn;
    ipRstpBridgeDescDst[i].FirmId        = ipRstpBridgeDescSrc[i].FirmId;
    ipRstpBridgeDescDst[i].forceVersion  = ipRstpBridgeDescSrc[i].forceVersion;
    ipRstpBridgeDescDst[i].ForwardDelay  = ipRstpBridgeDescSrc[i].ForwardDelay;
    memcpy(&ipRstpBridgeDescDst[i].MAC, ipRstpBridgeDescSrc[i].MAC, ETH_HWADDR_LEN);
    ipRstpBridgeDescDst[i].MaxAge        = ipRstpBridgeDescSrc[i].MaxAge;
    ipRstpBridgeDescDst[i].HelloTime     = ipRstpBridgeDescSrc[i].HelloTime;
    ipRstpBridgeDescDst[i].Priority      = ipRstpBridgeDescSrc[i].Priority;
    ipRstpBridgeDescDst[i].TxHoldCount   = ipRstpBridgeDescSrc[i].TxHoldCount;
    for (int j = 0; j < PORT_COUNT; ++j) {
      ipRstpBridgeDescDst[i].aBPortDesc[j].adminEdge     = ipRstpBridgeDescSrc[i].aBPortDesc[j].adminEdge;
      ipRstpBridgeDescDst[i].aBPortDesc[j].autoEdge      = ipRstpBridgeDescSrc[i].aBPortDesc[j].autoEdge;
      ipRstpBridgeDescDst[i].aBPortDesc[j].ForcePriority = ipRstpBridgeDescSrc[i].aBPortDesc[j].ForcePriority;
      ipRstpBridgeDescDst[i].aBPortDesc[j].MigrateTime   = ipRstpBridgeDescSrc[i].aBPortDesc[j].MigrateTime;
      ipRstpBridgeDescDst[i].aBPortDesc[j].rstpVersion   = ipRstpBridgeDescSrc[i].aBPortDesc[j].rstpVersion;
      ipRstpBridgeDescDst[i].aBPortDesc[j].stpVersion    = ipRstpBridgeDescSrc[i].aBPortDesc[j].stpVersion;
      ipRstpBridgeDescDst[i].aBPortDesc[j].portEnabled   = ipRstpBridgeDescSrc[i].aBPortDesc[j].portEnabled;
      ipRstpBridgeDescDst[i].aBPortDesc[j].VlanId        = ipRstpBridgeDescSrc[i].aBPortDesc[j].VlanId;
    }
  }
}

struct sRstpBridge * pMainRstpBridge;

void ProcessTimers(struct sRstpBridge * ipRstpBridge);

void ProcessPortSM(struct sRstpBridge * ipRstpBridge);
void InitBridgeByDesc(struct sRstpBridge * ipBridge, struct sRstpBridgeDesc * ipBridgeDesc);
void SetBridgeByDesc(struct sRstpBridge * ipBridge, struct sRstpBridgeDesc * ipBridgeDesc);

void SetRstpBlock() {
  SetBridgeByDesc(pMainRstpBridge, &gaRstpBridgeDesc[0]);
}

extern bool gSwitchStarted;
extern void LoadRstpCfgFlash();
bool StpInit() {
  //Инициализация блока RSTP
  if ((gnetif.flags & NETIF_FLAG_LINK_UP)) {
    if (!gSwitchStarted) {
      return false;
    }
#if (NO_SYS)
    CreateTaskRstp();
#endif
    pMainRstpBridge = &gaRstpBridge[0];
    SetBsp(true); //Broadcast Storm Protection

#ifdef RSTP_SET_OFF  //Отключение RSTP через Web-интерфейс
    LoadRstpCfgFlash(); //= Обновление конфигурации RSTP
#endif

    InitBridgeByDesc(pMainRstpBridge, &gaRstpBridgeDesc[0]);
#ifdef RSTP_SET_OFF  //Отключение RSTP через Web-интерфейс
    if (gaRstpBridgeDesc[0].RstpOn) {
#endif
      SetStpOn(pMainRstpBridge);
#if (ENABLE_RSTP)
      ProcessPortSM(pMainRstpBridge);
#endif
#ifdef RSTP_SET_OFF  //Отключение RSTP через Web-интерфейс
    } else {
      SetStpOff(pMainRstpBridge);
    }
#endif
    return true;
  }
  return false;
}

void ProcessPortTimers(struct sBPort * ipPort);
void ProcessSinglePortSM(struct sRstpBridge * ipRstpBridge, int8_t iPortIdx);

#include "vcc_ctrl.h"

#if (NO_SYS)
  #include "timer.h"
  //Подбором интервала подстраиваем цикл под значение Hello Time = 2 сек
  #define CYCLE_SIZE  (50)
#endif
void StpCycle() {
#if (ENABLE_RSTP)
  if (!IsVccNormal()) {
    //Питание не в норме - игнорируем входные пакеты
    return;
  }
 #if (!NO_SYS)
    ProcessTimers(pMainRstpBridge);
    ProcessPortSM(pMainRstpBridge);
 #else
  static timer_time_t cGlTime = 0;
  if (cGlTime == 0) {
    cGlTime = timer_timestamp();
    return;
  }
  timer_time_t cCurrTime = timer_timestamp();
  static int cPortIdx = 0;
  if ((cCurrTime - cGlTime) >= CYCLE_SIZE) {
    cGlTime = cCurrTime;
      ProcessPortTimers(&pMainRstpBridge->aPort[cPortIdx]);
      ProcessSinglePortSM(pMainRstpBridge, cPortIdx);
      ++cPortIdx;
      if (cPortIdx >= PORT_COUNT)
        cPortIdx = 0;
  }
 #endif
#endif
}

void SetTailTag(bool iOn, struct sRstpBridge * ipRstpBridge, bool iForce) {
  bool cIsTailTag = ReadSwRegBit(TAIL_TAG_BIT, TAIL_TAG_REG, SW1);
  if ((cIsTailTag != iOn) || (iForce)) {
    SetSwRegBit(iOn, TAIL_TAG_BIT, TAIL_TAG_REG, SW1, true);
  }
  ipRstpBridge->TailTagOn = iOn;
}

#if (!NO_SYS)
#include "fw.h"
extern void RestartSystem(uint32_t iStartAddr);
void rstp_task(void * pvParameters) {
  vTaskDelay(2000);
  while (!StpInit()) {
    vTaskDelay(10);
  }
#ifdef RSTP_SET_OFF  //Отключение RSTP через Web-интерфейс
  bool cIsRstpOn = gaRstpBridgeDesc[0].RstpOn;
  if (!cIsRstpOn) { //Разрешить работу всех портов
    enableRstpPorts();
    SetStpOff(pMainRstpBridge);
  }
#endif
  while (1) {
    vTaskDelay(RSTP_TASK_DELAY - 49);
#ifdef RSTP_SET_OFF  //Отключение RSTP через Web-интерфейс
    cIsRstpOn = gaRstpBridgeDesc[0].RstpOn;
    if (cIsRstpOn != (bool)gaRstpBridgeDesc[0].RstpOn) {
      vTaskDelay(500);
      RestartSystem(FW_START);
    }
    if (cIsRstpOn)
#endif
    StpCycle();
  }
}
#endif

//Прочитать собственный MAC-адрес свитча
void ReadSelfMac(uint8_t * iMac, ksz8895fmq_t * pSW) {
  uint8_t cMac[6];
  ksz8895fmq_read_registers(pSW, REG_SELF_MAC_ADDR, cMac, 6);
  memcpy(iMac, cMac, ETH_HWADDR_LEN);
}

//Записать собственный MAC-адрес свитча
void WriteSelfMac(uint8_t * iMac, ksz8895fmq_t * pSW) {
  ksz8895fmq_write_registers(pSW, REG_SELF_MAC_ADDR, iMac, 6);
}


//Сформировать Port Identifier по ipBPortDesc и номеру порта
uint16_t GetPortId(struct sBPortDesc * ipBPortDesc) {
  //Номер порта (1 - 4)
  //Проверки вх. параметров
  uint16_t cPortId = ipBPortDesc->PortIdx + 1;
  uint16_t cPriority = ipBPortDesc->ForcePriority;
  cPortId |= ((cPriority & 0x0f) << 12);
  cPortId = PP_HTONS(cPortId);
  return cPortId;
}


//==== Обработка входного кадра Ethernet STP

/**
 * Responds to STP.
 *
 * Should be called for incoming STP/RSTP packets. The pbuf in the argument
 * is freed by this function.
 *
 * @param p The STP/RSTP packet that arrived on netif. Is freed by this function.
 * @param netif The lwIP network interface on which the STP/RSTP packet pbuf arrived.
 *
 * @see pbuf_free()
 */

//uint32_t gTime = 0;

void ethstp_input(struct pbuf *p, struct netif *netif) {
  struct ethrstp_hdr * hdr;
  struct sRstpBlock * cpRstpBlock;

  LWIP_ERROR("netif != NULL", (netif != NULL), return;);

  if (!IsVccNormal()) {
    //Питание не в норме - игнорируем входные пакеты
    pbuf_free(p); //free STP packet
    return;
  }
  hdr = (struct ethrstp_hdr *)p->payload;
  cpRstpBlock = (struct sRstpBlock *)p->payload;
  
  if ((hdr->ProtId != 0x0000) || (hdr->DSAD != 0x42) || (hdr->SSAD != 0x42)) {
    //Обработка ошибок
    pbuf_free(p); //free STP packet
    return;
  }
  //Пока отключим отказ от STP
  if ((hdr->ProtVerId != 0x02)) {
    //Обработка STP кадров
    pbuf_free(p); //free packet
    return;
  }
  if (IsTailTag(0)) {
    if (cpRstpBlock->hdr.ForwardPorts & 0xfc) {
      //Неверный TailTag
      pbuf_free(p); //free STP packet
      return;
    }
    // !+ Для отладки
    if (cpRstpBlock->hdr.ForwardPorts == 0) {
      cpRstpBlock = (struct sRstpBlock *)p->payload;
    }
    if (cpRstpBlock->hdr.ForwardPorts == 1) {
      cpRstpBlock = (struct sRstpBlock *)p->payload;
    }
    if (cpRstpBlock->hdr.ForwardPorts == 2) {
      cpRstpBlock = (struct sRstpBlock *)p->payload;
    }
    // !- Для отладки
    if (p->len < 82) {
      cpRstpBlock->Port2 = 0xff;
    } else {
      cpRstpBlock = (struct sRstpBlock *)p->payload;
    }
#if (!NO_SYS)
    if( xQueueSend( gQueueStp, hdr, 50 ) != pdPASS ) {
#else
    if (xQueueSend(&gQueueStp, (uint8_t *)hdr)) {
#endif
      // Failed to post the message, even after 10 ticks.
      pbuf_free(p); //free STP packet
      return;
		}
  } else {
    //Ошибка: принят BPDU без TailTag - неизвестно, с какого порта принят
  }
  pbuf_free(p); //free STP packet
}

bool IsTailTag(int8_t iBridgeIdx) {
  if ((iBridgeIdx >= 0) && (iBridgeIdx <= 1)) {
    return gaRstpBridge[iBridgeIdx].TailTagOn;
  }
  return gaRstpBridge[0].TailTagOn;
}

//Включить режим поддержки STP/RSTP
extern int dmac_table_size[];
extern ksz8895fmq_read_dmac_t dmac_table[][1000];
void disableLearning(struct sBPort * ipPort, struct sBPortDesc * ipBPortDesc);

void SetStpOn(struct sRstpBridge * ipRstpBridge) {
  //Записать в SMAC STP Entry
#if (ENABLE_TAIL_TAG)
  struct s_SMACItem cSMACItem;

  memcpy(&cSMACItem.MAC[0], &ethrstp, ETH_HWADDR_LEN);
  cSMACItem.FwdPorts  = 0x10;
  cSMACItem.Valid     = 1;
  cSMACItem.Override  = 1;
  cSMACItem.UseFID    = 0;
  cSMACItem.FID       = 0;
  WriteSMacEntry(&cSMACItem, 0, SW1);
  //Включить режим TailTag
  SetTailTag(true, ipRstpBridge, true);
#endif
}

void SetStpOff(struct sRstpBridge * ipRstpBridge) {
  //Записать в SMAC STP Entry
#if (ENABLE_TAIL_TAG)
  struct s_SMACItem cSMACItem;
  memcpy(&cSMACItem.MAC[0], &ethrstp, ETH_HWADDR_LEN);
  cSMACItem.FwdPorts  = 0x10;
  cSMACItem.Valid     = 0;
  cSMACItem.Override  = 1;
  cSMACItem.UseFID    = 0;
  cSMACItem.FID       = 0;
  WriteSMacEntry(&cSMACItem, 0, SW1);
  //Включить режим TailTag
  SetTailTag(false, ipRstpBridge, true);
#endif
}

//==== Алгоритм RSTP ====

//Инициализация объектов моста и портов

void InitTimes(struct sTimes * ipTimes) {
  ipTimes->ForwardDelay = PP_HTONS(RSTP_FWD_DELAY_S << 8);
  ipTimes->HelloTime = PP_HTONS(RSTP_HELLO_TIME_S << 8);
  ipTimes->MaxAge = PP_HTONS(RSTP_MAX_AGE_S << 8);
  ipTimes->MessageAge = 0;
}

//Установка временных параметров (входные значения - в секундах)
void SetTimes(struct sTimes * ipTimes, int16_t iForwardDelay, int16_t iHelloTime, int16_t iMaxAge) {
  ipTimes->ForwardDelay = PP_HTONS(iForwardDelay << 8);
  ipTimes->HelloTime    = PP_HTONS(iHelloTime << 8);
  ipTimes->MaxAge       = PP_HTONS(iMaxAge << 8);
  ipTimes->MessageAge   = 0;
}

uint16_t InitPortId(uint8_t iPriority, uint16_t iPortIdx) {
  return PP_HTONS(((iPriority & 0x0f) << 12) | ((iPortIdx + 1) & 0x0fff));
}

void InitPortDesc(struct sBPortDesc * ipPortDesc, uint16_t iPortIdx) {
  
}


void InitPriVector(struct sPortPriVector * ipPriVector, struct sBridgeId * ipBridgeId) {
  memset((void *)ipPriVector, 0, sizeof(struct sPortPriVector));
  memcpy((void *)&ipPriVector->RootBridgeID, (void *)ipBridgeId, sizeof(*ipBridgeId));
  memcpy((void *)&ipPriVector->DesignatedBridgeID, (void *)ipBridgeId, sizeof(*ipBridgeId));
}

void SetPort(struct sBPort * ipPort, uint16_t iPortIdx, struct sRstpBridge * ipRstpBridge) {
  
  ipPort->portId = InitPortId(ipRstpBridge->pRstpBridgeDesc->aBPortDesc[iPortIdx].ForcePriority, iPortIdx);
  
  ipRstpBridge->pRstpBridgeDesc->aBPortDesc[iPortIdx].PortPathCost = DEF_PATH_COST_100;
  if (((ipRstpBridge->pRstpBridgeDesc->Idx == 0) && (iPortIdx == 0)) ||
      ((ipRstpBridge->pRstpBridgeDesc->Idx == 1) && (iPortIdx == 4))) { //Для этого порта - фактическая скорость берется из свитча
 #ifdef TWO_SWITCH_ITEMS
    // !!! Здесь нужны параметры порта 5 2-го свитча
    uint8_t cReg6 = 0;
    ksz8895fmq_read_register(SW2, 6, &cReg6);
    uint8_t cSpeed = (cReg6 & ETH_SPEED_10_MBPS)?(SWITCH_SPEED_10):(SWITCH_SPEED_100);
    if (cSpeed == SWITCH_SPEED_10)
      ipRstpBridge->pRstpBridgeDesc->aBPortDesc[iPortIdx].PortPathCost = DEF_PATH_COST_10;
 #endif
  }
  //designated
  InitPriVector(&ipPort->designatedPriority, &ipRstpBridge->BridgeId);
  ipPort->designatedPriority.DesignatedPortID = ipPort->portId;
  ipPort->designatedPriority.BridgePortID = ipPort->portId;
  SetTimes(&ipPort->designatedTimes, ipRstpBridge->pRstpBridgeDesc->ForwardDelay,
           ipRstpBridge->pRstpBridgeDesc->HelloTime, ipRstpBridge->pRstpBridgeDesc->MaxAge);
  
  ipPort->EdgeDelay = ipRstpBridge->pRstpBridgeDesc->aBPortDesc[iPortIdx].MigrateTime;
  ipPort->pRstpBridge = ipRstpBridge;
}

void InitPort(struct sBPort * ipPort, uint16_t iPortIdx, struct sRstpBridge * ipRstpBridge) {
  //Таймеры
  ipPort->edgeDelayWhile = 0;
  ipPort->fdWhile = 0;
  ipPort->helloWhen = 0;
  ipPort->mdelayWhile = 0;
  ipPort->rbWhile = 0;
  ipPort->rcvdInfoWhile = 0;
  ipPort->rrWhile = 0;
  ipPort->tcWhile = 0;
  //Счетчик передачи
  ipPort->txCount = 0;
  ipPort->DecTxCount = RSTP_TIME_COEF;
  
  ipPort->prState = prNONE;
  ipPort->ppmState = ppmNONE;
  ipPort->bdState = bdNONE;
  ipPort->ptState = ptNONE;
  ipPort->piState = piNONE;
  ipPort->prtState = prtNONE;
  ipPort->pstState = pstNONE;
  ipPort->tcState = tcNONE;
  
  ipPort->portId = InitPortId(ipRstpBridge->pRstpBridgeDesc->aBPortDesc[iPortIdx].ForcePriority, iPortIdx);

  ipRstpBridge->pRstpBridgeDesc->aBPortDesc[iPortIdx].PortPathCost = DEF_PATH_COST_100;
//  ipPort->PortPathCost = DEF_PATH_COST_100;
  if (((ipRstpBridge->pRstpBridgeDesc->Idx == 0) && (iPortIdx == 0)) ||
      ((ipRstpBridge->pRstpBridgeDesc->Idx == 1) && (iPortIdx == 4))) { //Для этого порта - фактическая скорость берется из свитча
    // !!! Здесь нужны параметры порта 5 2-го свитча
 #ifdef TWO_SWITCH_ITEMS
    uint8_t cReg6 = 0;
    ksz8895fmq_read_register(SW2, 6, &cReg6);
    uint8_t cSpeed = (cReg6 & ETH_SPEED_10_MBPS)?(SWITCH_SPEED_10):(SWITCH_SPEED_100);
    if (cSpeed == SWITCH_SPEED_10)
      ipRstpBridge->pRstpBridgeDesc->aBPortDesc[iPortIdx].PortPathCost = DEF_PATH_COST_10;
//      ipPort->PortPathCost = DEF_PATH_COST_10;
 #endif
  }
  //designated
  InitPriVector(&ipPort->designatedPriority, &ipRstpBridge->BridgeId);
  ipPort->designatedPriority.DesignatedPortID = ipPort->portId;
  ipPort->designatedPriority.BridgePortID = ipPort->portId;
  InitTimes(&ipPort->designatedTimes);
  
  ipPort->VlanId = 0; //Пока 0; (Нет поддержки VLAN)
  
  //operPointToPointMAC - считается = true;
  ipPort->operPointToPointMAC = true;
  
  ipPort->EdgeDelay = ipRstpBridge->pRstpBridgeDesc->aBPortDesc[iPortIdx].MigrateTime;
  ipPort->pRstpBridge = ipRstpBridge;
}



void InitBridgeId(struct sBridgeId * ipBridgeId, struct sRstpBridgeDesc * ipBridgeDesc, uint8_t * ipHwAddr) {
  memcpy(&ipBridgeId->MAC, (void *)ipHwAddr, ETH_HWADDR_LEN);
  ipBridgeId->Priority = PP_HTONS(((ipBridgeDesc->Priority & 0x0f) << 12) | ipBridgeDesc->FirmId);
  
}

void SetBridgeByDesc(struct sRstpBridge * ipBridge, struct sRstpBridgeDesc * ipBridgeDesc) {
  uint8_t cMAC[6];
  ReadSelfMac(&cMAC[0], SW1); //Работаем с обоими мостами
  if (memcmp(&cMAC[0], &ipBridgeDesc->MAC[0], ETH_HWADDR_LEN) != 0) {
    memcpy(&cMAC[0], &ipBridgeDesc->MAC[0], ETH_HWADDR_LEN);
    //Включить после перестройки на 2 моста
    // + запись в settings
    WriteSelfMac(&cMAC[0], ipBridgeDesc->SW);
#if (!NO_SYS)
    vTaskDelay(10);
#endif
  }
  
  InitBridgeId(&ipBridge->BridgeId, ipBridgeDesc, &cMAC[0]);
  InitPriVector(&ipBridge->BridgePriority, &ipBridge->BridgeId);

  SetTimes(&ipBridge->BridgeTimes, ipBridgeDesc->ForwardDelay, ipBridgeDesc->HelloTime, ipBridgeDesc->MaxAge);
  if (ipBridge->BridgeRole == brRoot) {
    InitPriVector(&ipBridge->RootPriority, &ipBridge->BridgeId);
    SetTimes(&ipBridge->RootTimes, ipBridgeDesc->ForwardDelay, ipBridgeDesc->HelloTime, ipBridgeDesc->MaxAge);
  }
  ipBridge->TxHoldCount = ipBridgeDesc->TxHoldCount;
  
  for (int i = 0; i < PORT_COUNT; ++i) {
    SetPort(&ipBridge->aPort[i], i, ipBridge);
  }
  
}

void InitBridgeByDesc(struct sRstpBridge * ipBridge, struct sRstpBridgeDesc * ipBridgeDesc) {
  ipBridge->pRstpBridgeDesc = ipBridgeDesc;
  
  ipBridge->BEGIN = true;
  uint8_t cMAC[6];
  ReadSelfMac(&cMAC[0], SW1); //Работаем с обоими мостами
#ifdef TWO_SWITCH_ITEMS
  uint8_t cMAC2[6];
  ReadSelfMac(&cMAC2[0], SW2); //Работаем с обоими мостами
#endif
  if (memcmp(&cMAC[0], &ipBridgeDesc->MAC[0], ETH_HWADDR_LEN) != 0) {
    memcpy(&cMAC[0], &ipBridgeDesc->MAC[0], ETH_HWADDR_LEN);
    //Включить после перестройки на 2 моста
    // + запись в settings
    WriteSelfMac(&cMAC[0], ipBridgeDesc->SW);
#ifdef TWO_SWITCH_ITEMS
    WriteSelfMac(&cMAC[0], SW2);
    vTaskDelay(100);
#endif
  }
  
  InitBridgeId(&ipBridge->BridgeId, ipBridgeDesc, &cMAC[0]);
  InitPriVector(&ipBridge->BridgePriority, &ipBridge->BridgeId);

  InitPriVector(&ipBridge->RootPriority, &ipBridge->BridgeId);

  ipBridge->prsState = prsNONE;
  
  SetTimes(&ipBridge->BridgeTimes, ipBridgeDesc->ForwardDelay, ipBridgeDesc->HelloTime, ipBridgeDesc->MaxAge);
  SetTimes(&ipBridge->RootTimes, ipBridgeDesc->ForwardDelay, ipBridgeDesc->HelloTime, ipBridgeDesc->MaxAge);
  ipBridge->TxHoldCount = ipBridgeDesc->TxHoldCount;
  
  for (int i = 0; i < PORT_COUNT; ++i) {
    InitPort(&ipBridge->aPort[i], i, ipBridge);
  }
}

//=== Таймеры

// 17.22 Port Timers state machine
void DecTimer(int32_t * iTimer) {
  if (*iTimer > 0)
    --*iTimer;
}

//void SetTimerValue(int32_t * iTimer, int32_t iValue) {
//  *iTimer = iValue * RSTP_TIME_COEF;
//}

void ProcessPortTimers(struct sBPort * ipPort) {
  DecTimer(&ipPort->edgeDelayWhile);
  DecTimer(&ipPort->fdWhile);
  DecTimer(&ipPort->helloWhen);
  DecTimer(&ipPort->mdelayWhile);
  DecTimer(&ipPort->rbWhile);
  DecTimer(&ipPort->rcvdInfoWhile);
  DecTimer(&ipPort->rrWhile);
  DecTimer(&ipPort->tcWhile);
  if (--ipPort->DecTxCount <= 0) {
    DecTimer(&ipPort->txCount);
    ipPort->DecTxCount = RSTP_TIME_COEF;
  }
}

//Обработчики КА

 //17.22 Port Timers state machine
void ProcessTimers(struct sRstpBridge * ipRstpBridge) {
  if (ipRstpBridge->BEGIN)
    return;
  for (int i = 0; i < PORT_COUNT; ++i) {
    ProcessPortTimers(&ipRstpBridge->aPort[i]);
  }
}

void SetConstTimerVar(int32_t * ipVar, uint16_t iValue) {
  *ipVar = iValue * RSTP_TIME_COEF; //iValue в 1 сек
}

//Для распределения по времени
void SetIdxConstTimerVar(int32_t * ipVar, uint16_t iValue, int8_t iOffset) {
  *ipVar = iValue * RSTP_TIME_COEF + iOffset; //iValue в 1 сек
}

void SetTimerVar(int32_t * ipVar, uint16_t iValue) {
  *ipVar = PP_HTONS(iValue) * RSTP_TIME_COEF >> 8; //iValue в 1/256 сек Rev
}

int CompareConstTimerVar(int32_t * ipVar, uint16_t iValue) {
  uint16_t cVar = *ipVar / RSTP_TIME_COEF; //iValue в 1 сек
  if (*ipVar % RSTP_TIME_COEF)
    ++cVar;
  if (cVar < iValue) {
    return -1;
  } else if (cVar == iValue) {
    return 0;
  } else {
    return 1;
  }
}

int CompareTimerVar(int32_t * ipVar, uint16_t iValue) {
  uint16_t cVar = *ipVar / RSTP_TIME_COEF; //iValue в 1/256 сек Rev
  if (*ipVar % RSTP_TIME_COEF)
    ++cVar;
  uint16_t cValue = PP_HTONS(iValue) >> 8;
  if (cVar < cValue) {
    return -1;
  } else if (cVar == cValue) {
    return 0;
  } else {
    return 1;
  }
}

 //17.23 Port Receive state machine
#define RX_TIMEOUT  (40)
void updtBPDUVersion(struct sBPort * ipPort);

void ProcessPortReceive(struct sBPort * ipPort, struct sBPortDesc * ipBPortDesc) {
//  static int cRxCount = RX_TIMEOUT;
  if (ipPort->pRstpBridge->BEGIN ||
      ((ipPort->rcvdBPDU ||
        (CompareConstTimerVar(&ipPort->edgeDelayWhile, MIGRATE_TIME) != 0)) && !ipBPortDesc->portEnabled)) {
    ipPort->RxCount = RX_TIMEOUT;
    ipPort->rcvdBPDU = false;
    ipPort->rcvdRSTP = false;
    ipPort->rcvdSTP  = false;
    ipPort->rcvdMsg  = false;
    SetConstTimerVar(&ipPort->edgeDelayWhile, MIGRATE_TIME);
    ipPort->prState  = prDISCARD;
  } else if (ipBPortDesc->portEnabled) {
    if (ipPort->prState == prDISCARD) {
      if (ipPort->rcvdBPDU) {
        ipPort->RxCount = RX_TIMEOUT;
        updtBPDUVersion(ipPort);
        ipPort->operEdge = false;  ipPort->rcvdBPDU = false;  ipPort->rcvdMsg = true;
        SetConstTimerVar(&ipPort->edgeDelayWhile, MIGRATE_TIME);
        ipPort->prState  = prRECEIVE;
      }
    } else if (ipPort->prState == prRECEIVE) {
      if (ipPort->rcvdBPDU && !ipPort->rcvdMsg) {
        ipPort->RxCount = RX_TIMEOUT;
        updtBPDUVersion(ipPort);
        ipPort->operEdge = false;  ipPort->rcvdBPDU = false;  ipPort->rcvdMsg = true;
        SetConstTimerVar(&ipPort->edgeDelayWhile, MIGRATE_TIME);
      } else {
        if (ipPort->RxCount > 0) {
          --ipPort->RxCount;
          if (ipPort->RxCount <= 0) {
            ipPort->prState = prNO_RECEIVE;
            ipPort->RxCount = RX_TIMEOUT;
          }
        }    
      }
    } else if (ipPort->prState == prNO_RECEIVE) {
      if (ipPort->rcvdBPDU && !ipPort->rcvdMsg) {
        ipPort->RxCount = RX_TIMEOUT;
        updtBPDUVersion(ipPort);
        ipPort->operEdge = false;  ipPort->rcvdBPDU = false;  ipPort->rcvdMsg = true;
        SetConstTimerVar(&ipPort->edgeDelayWhile, MIGRATE_TIME);
        ipPort->prState  = prRECEIVE;
      }
    }
  }
}

//17.24 Port Protocol Migration state machine
void ProcessPortProtocolMigration(struct sBPort * ipPort, struct sBPortDesc * ipBPortDesc) {
  if (ipPort->pRstpBridge->BEGIN) {
    ipPort->mcheck = false;
    ipPort->sendRSTP = ipBPortDesc->rstpVersion;
    SetConstTimerVar(&ipPort->mdelayWhile, MIGRATE_TIME);
    ipPort->ppmState = ppmCHECKING_RSTP;
    return;
  }
  switch (ipPort->ppmState) {
  case ppmNONE:
    ipPort->mcheck = false;
    ipPort->sendRSTP = ipBPortDesc->rstpVersion;
    SetConstTimerVar(&ipPort->mdelayWhile, MIGRATE_TIME);
    ipPort->ppmState = ppmCHECKING_RSTP;
    break;
  case ppmCHECKING_RSTP:
    if (ipPort->mdelayWhile == 0) {
      ipPort->rcvdRSTP = ipPort->rcvdSTP = false;
      ipPort->ppmState = ppmSENSING;
    }
    break;
  case ppmSELECTING_STP:
    if ((ipPort->mdelayWhile == 0) || !ipBPortDesc->portEnabled || ipPort->mcheck) {
      ipPort->rcvdRSTP = ipPort->rcvdSTP = false;
      ipPort->ppmState = ppmSENSING;
    }
    break;
  case ppmSENSING:
    if (ipPort->sendRSTP && ipPort->rcvdSTP) {
      ipPort->mcheck = false;
      ipPort->sendRSTP = ipBPortDesc->rstpVersion;
      SetConstTimerVar(&ipPort->mdelayWhile, MIGRATE_TIME);
      ipPort->ppmState = ppmCHECKING_RSTP;
      break;
    }
    if (!ipBPortDesc->portEnabled || ipPort->mcheck ||
        ((ipBPortDesc->rstpVersion) && !ipPort->sendRSTP && ipPort->rcvdRSTP)) {
      ipPort->mcheck = false;
      ipPort->sendRSTP = ipBPortDesc->rstpVersion;
      SetConstTimerVar(&ipPort->mdelayWhile, MIGRATE_TIME);
      ipPort->ppmState = ppmCHECKING_RSTP;
    }
    break;
  default:
    break;
  }
}

//17.25 Bridge Detection state machine
void ProcessBridgeDetection(struct sBPort * ipPort, struct sBPortDesc * ipBPortDesc) {
  if (ipPort->pRstpBridge->BEGIN) {
    if (ipBPortDesc->adminEdge) {
      ipPort->operEdge = true;
      ipPort->bdState = bdEDGE;
    } else {
      ipPort->operEdge = false;
      ipPort->bdState = bdNOT_EDGE;
    }
  } else {
    switch (ipPort->bdState) {
    case bdEDGE:
      if ((!ipBPortDesc->portEnabled && !ipBPortDesc->adminEdge) || !ipPort->operEdge) {
        ipPort->operEdge = false;
        ipPort->bdState = bdNOT_EDGE;
      }
      break;
    case bdNOT_EDGE:
      if ((!ipBPortDesc->portEnabled && ipBPortDesc->adminEdge) ||
          ((ipPort->edgeDelayWhile == 0) && ipBPortDesc->autoEdge
           && ipPort->sendRSTP && ipPort->proposing)) {
        ipPort->operEdge = true;
        ipPort->bdState = bdEDGE;
      }
      break;
    default:
      break;
    }
  }
}

//17.26 Port Transmit state machine
void txRstp(struct sBPort * ipPort, struct sBPortDesc * ipBPortDesc);
void txTcn(struct sBPort * ipPort, struct sBPortDesc * ipBPortDesc);
void txConfig(struct sBPort * ipPort, struct sBPortDesc * ipBPortDesc);

void ProcessPortTransmit(struct sBPort * ipPort, struct sBPortDesc * ipBPortDesc) {
  if (ipPort->pRstpBridge->BEGIN) {
    ipPort->newInfo = true;
    ipPort->txCount = 0;
    ipPort->ptState = ptTRANSMIT_INIT;
  } else {
    switch (ipPort->ptState) {
    case ptTRANSMIT_INIT:
      SetIdxConstTimerVar(&ipPort->helloWhen, ipBPortDesc->pRstpBridgeDesc->HelloTime, ipBPortDesc->PortIdx * 5);
      ipPort->ptState = ptIDLE;
      break;
    case ptIDLE:
      if (ipPort->helloWhen == 0) {
        ipPort->newInfo = ipPort->newInfo || (ipPort->selectedRole == prrDesignated ||
                          (ipPort->selectedRole == prrRoot && (ipPort->tcWhile != 0)));
        ipPort->ptState = ptTRANSMIT_PERIODIC;
        break;
      }
      if (ipPort->sendRSTP && ipPort->newInfo &&
          (ipPort->txCount < ipPort->pRstpBridge->TxHoldCount) &&
            (ipPort->helloWhen != 0)) {
        ipPort->newInfo = false;
        txRstp(ipPort, ipBPortDesc);
        ipPort->txCount += 1;
        ipPort->tcAck = false;
        ipPort->ptState = ptTRANSMIT_RSTP;
        break;
      }
      if (!ipPort->sendRSTP && ipPort->newInfo &&
          ipPort->selectedRole == prrRoot &&
            (ipPort->txCount < ipPort->pRstpBridge->TxHoldCount)
              && (ipPort->helloWhen != 0)) {
        ipPort->newInfo = false;
        txTcn(ipPort, ipBPortDesc);
        ipPort->txCount += 1;
        ipPort->ptState = ptTRANSMIT_TCN;
        break;
      }
      if (!ipPort->sendRSTP && ipPort->newInfo &&
          ipPort->selectedRole == prrDesignated &&
          (ipPort->txCount < ipPort->pRstpBridge->TxHoldCount) &&
          (ipPort->helloWhen != 0)) {
        ipPort->newInfo = false;
        txConfig(ipPort, ipBPortDesc);
        ipPort->txCount += 1;
        ipPort->tcAck = false;
        ipPort->ptState = ptTRANSMIT_CONFIG;
      }
      break;
    case ptTRANSMIT_CONFIG:
    case ptTRANSMIT_PERIODIC:
    case ptTRANSMIT_TCN:
    case ptTRANSMIT_RSTP:
      SetConstTimerVar(&ipPort->helloWhen, ipBPortDesc->pRstpBridgeDesc->HelloTime);
      ipPort->ptState = ptIDLE;
      break;
    default:
      break;
    }
  }
}

//17.27 Port Information state machine
bool betterorsameinfo(enum eInfoIs iNewInfoIs, struct sBPort * ipPort);
enum eInfoType rcvInfo(struct sBPort * ipPort);
void updtRcvdInfoWhile(struct sBPort * ipPort);
void recordPriority(struct sBPort * ipPort);
void recordTimes(struct sBPort * ipPort);
void recordProposal(struct sBPort * ipPort);
void setTcFlags(struct sBPort * ipPort);
void recordDispute(struct sBPort * ipPort);
void recordAgreement(struct sBPort * ipPort, struct sBPortDesc * ipBPortDesc);

void ProcessPortInformation(struct sBPort * ipPort, struct sBPortDesc * ipBPortDesc) {
  if ((!ipBPortDesc->portEnabled && (ipPort->infoIs != iiDisabled)) || ipPort->pRstpBridge->BEGIN) {
    ipPort->rcvdMsg = false;
    ipPort->proposing = ipPort->proposed = ipPort->agree = ipPort->agreed = false;
    ipPort->rcvdInfoWhile = 0;
    ipPort->infoIs = iiDisabled; ipPort->reselect = true; ipPort->selected = false;
    ipPort->piState = piDISABLED;
  } else {
    switch (ipPort->piState) {
    case piDISABLED:
      if (ipBPortDesc->portEnabled) {
        ipPort->infoIs = iiAged;
        ipPort->reselect = true;
        ipPort->selected = false;
        ipPort->piState = piAGED;
      }
      break;
    case piAGED:
      if (ipPort->selected && ipPort->updtInfo) {
        ipPort->proposing = ipPort->proposed = false;
        ipPort->agreed = ipPort->agreed && betterorsameinfo(iiMine, ipPort);
        ipPort->synced = ipPort->synced && ipPort->agreed;
        memcpy((void *)&ipPort->portPriority.RootBridgeID,
               (void *)&ipPort->designatedPriority.RootBridgeID,
               sizeof(ipPort->designatedPriority) - 2);
        memcpy((void *)&ipPort->portTimes, (void *)&ipPort->designatedTimes,
               sizeof(ipPort->designatedTimes));
        ipPort->updtInfo = false; ipPort->infoIs = iiMine; ipPort->newInfo = true;
        ipPort->piState = piUPDATE;
      }
      break;
    case piUPDATE:
    case piSUPERIOR_DESIGNATED:
    case piREPEATED_DESIGNATED:
    case piINFERIOR_DESIGNATED:
    case piNOT_DESIGNATED:
    case piOTHER:
      ipPort->piState = piCURRENT;
      break;
    case piCURRENT:
      if (ipPort->selected && ipPort->updtInfo) {
        ipPort->proposing = ipPort->proposed = false;
        ipPort->agreed = ipPort->agreed && betterorsameinfo(iiMine, ipPort);
        ipPort->synced = ipPort->synced && ipPort->agreed;
        memcpy((void *)&ipPort->portPriority.RootBridgeID,
               (void *)&ipPort->designatedPriority.RootBridgeID,
               sizeof(ipPort->designatedPriority) - 2);
        memcpy((void *)&ipPort->portTimes, (void *)&ipPort->designatedTimes,
               sizeof(ipPort->designatedTimes));
        ipPort->updtInfo = false; ipPort->infoIs = iiMine; ipPort->newInfo = true;
        ipPort->piState = piUPDATE;
        break;
      }
      if ((ipPort->infoIs == iiReceived) && (ipPort->rcvdInfoWhile == 0) &&
          !ipPort->updtInfo && !ipPort->rcvdMsg) {
//        // !!!
//        if (ipPort == &ipPort->pRstpBridge->aPort[2]) {
//          ipPort->selectedRole = prrDesignated;
//        }
//        // !!!
        ipPort->infoIs = iiAged;
        ipPort->reselect = true;
        ipPort->selected = false;
        ipPort->piState = piAGED;
        break;
      }
      if (ipPort->rcvdMsg && !ipPort->updtInfo) {
        ipPort->rcvdInfo = rcvInfo(ipPort);
        ipPort->piState = piRECEIVE;
      }
      break;
    case piRECEIVE:
      if (ipPort->rcvdInfo == itSuperiorDesignatedInfo) {
        ipPort->agreed = ipPort->proposing = false;
        recordProposal(ipPort);
        setTcFlags(ipPort);
        ipPort->agree = ipPort->agree && betterorsameinfo(iiReceived, ipPort);
        recordPriority(ipPort);
        recordTimes(ipPort);
        updtRcvdInfoWhile(ipPort);
        ipPort->infoIs = iiReceived; ipPort->reselect = true; ipPort->selected = false;
        ipPort->rcvdMsg = false;
        ipPort->piState = piSUPERIOR_DESIGNATED;
        break;
      }
      if (ipPort->rcvdInfo == itRepeatedDesignatedInfo) {
//        // !!! Не по стандарту, проба
//         if (ipPort->learning || ipPort->forwarding)
//          ipPort->disputed = true;
//        // !!! 
        recordProposal(ipPort);
        setTcFlags(ipPort);
        updtRcvdInfoWhile(ipPort);
        ipPort->rcvdMsg = false;
        ipPort->piState = piREPEATED_DESIGNATED;
        break;
      }
      if (ipPort->rcvdInfo == itInferiorDesignatedInfo) {
        recordDispute(ipPort);
        ipPort->rcvdMsg = false;
        ipPort->piState = piINFERIOR_DESIGNATED;
        break;
      }
      if (ipPort->rcvdInfo == itInferiorRootAlternateInfo) {
        recordAgreement(ipPort, ipBPortDesc);
        setTcFlags(ipPort);
        ipPort->rcvdMsg = false;
        ipPort->piState = piNOT_DESIGNATED;
        break;
      }
      if (ipPort->rcvdInfo == itOtherInfo) {
        ipPort->rcvdMsg = false;
        ipPort->piState = piOTHER;
      }
      break;
    default:
      break;
    }
  }
}

//17.28 Port Role Selection state machine
void clearReselectTree(struct sRstpBridge * ipRstpBridge);
void setSelectedTree(struct sRstpBridge * ipRstpBridge);
void updtRoleDisabledTree(struct sRstpBridge * ipRstpBridge);
void updtRolesTree(struct sRstpBridge * ipRstpBridge);

void ProcessPortRoleSelection(struct sRstpBridge * ipRstpBridge) {
  if (ipRstpBridge->BEGIN) {
    updtRoleDisabledTree(ipRstpBridge);
    ipRstpBridge->prsState = prsINIT_BRIDGE;
  } else {
    int8_t cReselect = 0;
    switch (ipRstpBridge->prsState) {
    case prsINIT_BRIDGE:
      clearReselectTree(ipRstpBridge);
      updtRolesTree(ipRstpBridge);
      setSelectedTree(ipRstpBridge);
      ipRstpBridge->prsState = prsROLE_SELECTION;
      break;
    case prsROLE_SELECTION:
      for (int i = 0; i < PORT_COUNT; ++i) {
        if (ipRstpBridge->aPort[i].reselect) {
          cReselect = i + 1;
          break;
        }
      }
      if (cReselect) {
        clearReselectTree(ipRstpBridge);
        updtRolesTree(ipRstpBridge);
        setSelectedTree(ipRstpBridge);
//        ipRstpBridge->prsState = prsROLE_SELECTION;
      }
      break;
    default:
      break;
    }
  }
}

void setSyncTree(struct sRstpBridge * ipRstpBridge);

void CheckAllSynced(struct sBPort * ipPort) {
  //Вычисление allSynced для порта
  for (int i = 0; i < PORT_COUNT; ++i) {
    if (&ipPort->pRstpBridge->aPort[i] != ipPort) {
      if ((!ipPort->pRstpBridge->aPort[i]./*synced*/selected)/* || (ipPort->pRstpBridge->aPort[i].role != ipPort->pRstpBridge->aPort[i].selectedRole)*/) {
        ipPort->allSynced = false;
        return;
      }
    }
  }
  if (ipPort->synced || (ipPort->role == prrRoot))
    ipPort->allSynced = true;
}

//17.29 Port Role Transitions state machine
void setReRootTree(struct sRstpBridge * ipRstpBridge);

void CalcForwardDelay(struct sBPort * ipPort, struct sBPortDesc * ipBPortDesc) {
  if (ipPort->sendRSTP) {
    SetConstTimerVar(&ipPort->forwardDelay, ipBPortDesc->pRstpBridgeDesc->HelloTime);
  } else {
    SetTimerVar(&ipPort->forwardDelay, ipPort->designatedTimes.ForwardDelay);
  }
}

uint32_t gfdWhile;
bool gSync, gReRoot, gSynced;

void ProcessPortRoleTransitions(struct sBPort * ipPort, struct sBPortDesc * ipBPortDesc) {
  if (ipPort->pRstpBridge->BEGIN) {
    ipPort->role = prrDisabled;
    ipPort->selectedRole = prrDisabled; //Не по диаграмме, но по логике должно быть так
    ipPort->learn = ipPort->forward = false;
    ipPort->synced = false;
    ipPort->sync = ipPort->reRoot = true;
    SetTimerVar(&ipPort->rrWhile, ipPort->pRstpBridge->BridgeTimes.ForwardDelay);
    SetTimerVar(&ipPort->fdWhile, ipPort->pRstpBridge->BridgeTimes.MaxAge);
    ipPort->rbWhile = 0;    
    ipPort->prtState = prtINIT_PORT;
    return;
  }
  if (ipPort->selected && !ipPort->updtInfo) {
    if (ipPort->role != ipPort->selectedRole) {
      switch (ipPort->selectedRole) {
      case prrDisabled:
        ipPort->role = ipPort->selectedRole;
        ipPort->learn = ipPort->forward = false;
        ipPort->prtState = prtDISABLE_PORT;
        return;
      case prrRoot:
        ipPort->role = prrRoot;
        SetTimerVar(&ipPort->rrWhile, ipPort->pRstpBridge->BridgeTimes.ForwardDelay);
        ipPort->prtState = prtROOT_PORT;
        return;
      case prrDesignated:
        ipPort->role = prrDesignated;
        ipPort->prtState = prtDESIGNATED_PORT;
        return;
      case prrAlternate:
      case prrBackup:
        ipPort->role = ipPort->selectedRole;
        ipPort->learn = ipPort->forward = false;
        ipPort->prtState = prtBLOCK_PORT;
        return;
      default:
        break;
      }
    }
  }
  // UCT - безусловный переход
  switch (ipPort->prtState) {
  //17.29.1 Disabled Port states
  case prtINIT_PORT:
    ipPort->role = ipPort->selectedRole;
    ipPort->learn = ipPort->forward = false;
    ipPort->prtState = prtDISABLE_PORT;
    return;
  //17.29.2 Root Port states
  case prtROOT_PROPOSED:
  case prtROOT_AGREED:
  case prtREROOT:
  case prtROOT_FORWARD:
  case prtROOT_LEARN:
  case prtREROOTED:
    ipPort->role = prrRoot;
    SetTimerVar(&ipPort->rrWhile, ipPort->pRstpBridge->BridgeTimes.ForwardDelay);
    ipPort->prtState = prtROOT_PORT;
    return;
  //17.29.3 Designated Port states
  case prtDESIGNATED_PROPOSE:
  case prtDESIGNATED_SYNCED:
  case prtDESIGNATED_RETIRED:
  case prtDESIGNATED_FORWARD:
  case prtDESIGNATED_LEARN:
  case prtDESIGNATED_DISCARD:
    ipPort->role = prrDesignated;
    ipPort->prtState = prtDESIGNATED_PORT;
    return;
  //17.29.4 Alternate and Backup Port states
  case prtALTERNATE_PROPOSED:
  case prtALTERNATE_AGREED:
  case prtBACKUP_PORT:
    ipPort->role = prrAlternate;
    SetTimerVar(&ipPort->fdWhile, ipPort->pRstpBridge->BridgeTimes.ForwardDelay);
    ipPort->synced = true;
    ipPort->rrWhile = 0;
    ipPort->sync = ipPort->reRoot = false;
    ipPort->prtState = prtALTERNATE_PORT;
    return;
  default:
    break;
  }
  
  if (ipPort->selected && !ipPort->updtInfo) {
    switch (ipPort->prtState) {
    //17.29.1 Disabled Port states
    case prtDISABLE_PORT:
      if (!ipPort->learning && !ipPort->forwarding) {
        SetTimerVar(&ipPort->fdWhile, ipPort->pRstpBridge->BridgeTimes.MaxAge);
        ipPort->synced = true;
        ipPort->rrWhile = 0;
        ipPort->sync = ipPort->reRoot = false;
        ipPort->prtState = prtDISABLED_PORT;
        return;
      }
      break;
    case prtDISABLED_PORT:
      if (ipPort == &ipPort->pRstpBridge->aPort[2]) {
        gfdWhile = ipPort->fdWhile;
        gSync = ipPort->sync; gReRoot = ipPort->reRoot; gSynced = ipPort->synced;
      }
      if ((CompareTimerVar(&ipPort->fdWhile, ipPort->pRstpBridge->BridgeTimes.MaxAge) != 0) || ipPort->sync ||
          ipPort->reRoot || !ipPort->synced) {
        SetTimerVar(&ipPort->fdWhile, ipPort->pRstpBridge->BridgeTimes.MaxAge);
        ipPort->synced = true;
        ipPort->rrWhile = 0;
        ipPort->sync = ipPort->reRoot = false;
        //ipPort->prtState = prtDISABLED_PORT;
        return;
      }
      break;
    //17.29.2 Root Port states
    case prtROOT_PORT:
      if (ipPort->proposed && !ipPort->agree) {
        setSyncTree(ipPort->pRstpBridge);
        ipPort->proposed = false;
        ipPort->prtState = prtROOT_PROPOSED;
        return;
      }
      CheckAllSynced(ipPort);
      if ((ipPort->allSynced && !ipPort->agree) || (ipPort->proposed && ipPort->agree)) {
        ipPort->proposed = ipPort->sync = false;
        ipPort->agree = true;
        ipPort->newInfo = true;
        ipPort->prtState = prtROOT_AGREED;
        return;
      }
      if (!ipPort->forward && !ipPort->reRoot) {
        setReRootTree(ipPort->pRstpBridge);
        ipPort->prtState = prtREROOT;
        return;
      }
      if (((ipPort->fdWhile == 0) ||
           (ipPort->reRooted && (ipPort->rbWhile == 0)) && (ipBPortDesc->rstpVersion)) &&
          ipPort->learn && !ipPort->forward) {
        ipPort->fdWhile = 0;
        ipPort->forward = true;
        ipPort->prtState = prtROOT_FORWARD;
        return;
      }
      if (((ipPort->fdWhile == 0) ||
           (ipPort->reRooted && (ipPort->rbWhile == 0)) && (ipBPortDesc->rstpVersion)) &&
          !ipPort->learn) {
        CalcForwardDelay(ipPort, ipBPortDesc); // ! вычисление ipPort->forwardDelay
        ipPort->fdWhile = ipPort->forwardDelay;
        ipPort->learn = true;
        ipPort->prtState = prtROOT_LEARN;
        return;
      }
      if (ipPort->reRoot && ipPort->forward) {
        ipPort->reRoot = false;
        ipPort->prtState = prtREROOTED;
        return;
      }
      // !!! Уточнить источник ForwardDelay
      if (CompareTimerVar(&ipPort->rrWhile, ipPort->pRstpBridge->RootTimes.ForwardDelay) != 0) {
        ipPort->role = prrRoot;
        SetTimerVar(&ipPort->rrWhile, ipPort->pRstpBridge->RootTimes.ForwardDelay);
        return;
      }
      break;
    //17.29.3 Designated Port states
    case prtDESIGNATED_PORT:
      if (!ipPort->forward && !ipPort->agreed && !ipPort->proposing && !ipPort->operEdge) {
        ipPort->proposing = true;
        // ! вычисление ipPort->EdgeDelay
        SetConstTimerVar(&ipPort->edgeDelayWhile, ipPort->EdgeDelay);
        ipPort->newInfo = true;
        ipPort->prtState = prtDESIGNATED_PROPOSE;
        return;
      }
      if ((!ipPort->learning && !ipPort->forwarding && !ipPort->synced) ||
          (ipPort->agreed && !ipPort->synced) ||
          (ipPort->operEdge && !ipPort->synced) ||
          (ipPort->sync && ipPort->synced)) {
        ipPort->rrWhile = 0; ipPort->synced = true;
        ipPort->sync = false;
        ipPort->prtState = prtDESIGNATED_SYNCED;
        return;
      }
      if ((ipPort->rrWhile == 0) && ipPort->reRoot) {
        ipPort->reRoot = false;
        ipPort->prtState = prtDESIGNATED_RETIRED;
        return;
      }
      if (((ipPort->sync && !ipPort->synced) ||
           (ipPort->reRoot && (ipPort->rrWhile != 0)) || ipPort->disputed) &&
          !ipPort->operEdge && (ipPort->learn || ipPort->forward)) {
        ipPort->learn = ipPort->forward = ipPort->disputed = false;
        CalcForwardDelay(ipPort, ipBPortDesc); // ! вычисление ipPort->forwardDelay
        ipPort->fdWhile = ipPort->forwardDelay;
        ipPort->prtState = prtDESIGNATED_DISCARD;
        return;
      }
      if (((ipPort->fdWhile == 0) || ipPort->agreed || ipPort->operEdge) &&
          ((ipPort->rrWhile == 0) || !ipPort->reRoot) && !ipPort->sync && !ipPort->learn) {
        ipPort->learn = true;
        CalcForwardDelay(ipPort, ipBPortDesc); // ! вычисление ipPort->forwardDelay
        ipPort->fdWhile = ipPort->forwardDelay;
        ipPort->prtState = prtDESIGNATED_LEARN;
        return;
      }
      if (((ipPort->fdWhile == 0) || ipPort->agreed || ipPort->operEdge) &&
          ((ipPort->rrWhile == 0) || !ipPort->reRoot) && !ipPort->sync &&
          (ipPort->learn && !ipPort->forward)) {
        ipPort->forward = true;
        ipPort->fdWhile = 0;
        ipPort->agreed = ipPort->sendRSTP;
        ipPort->prtState = prtDESIGNATED_FORWARD;
        return;
      }
      break;
    //17.29.4 Alternate and Backup Port states
    case   prtBLOCK_PORT:
      if (!ipPort->learning && !ipPort->forwarding) {
        ipPort->role = prrAlternate;
        SetTimerVar(&ipPort->fdWhile, ipPort->pRstpBridge->BridgeTimes.ForwardDelay);
        ipPort->synced = true;
        ipPort->rrWhile = 0;
        ipPort->sync = ipPort->reRoot = false;
        ipPort->prtState = prtALTERNATE_PORT;
        return;
      }
      break;
    case   prtALTERNATE_PORT:
      if (ipPort->proposed && !ipPort->agree) {
        setSyncTree(ipPort->pRstpBridge);
        ipPort->proposed = false;
        ipPort->prtState = prtALTERNATE_PROPOSED;
        return;
      }
      CheckAllSynced(ipPort);
      if ((ipPort->allSynced && !ipPort->agree) ||
          (ipPort->proposed && ipPort->agree)) {
        ipPort->proposed = false;
        ipPort->agree = true;
        ipPort->newInfo = true;
        ipPort->prtState = prtALTERNATE_AGREED;
        return;
      }
      if ((CompareTimerVar(&ipPort->rbWhile, 2 * ipPort->pRstpBridge->BridgeTimes.HelloTime) != 0) &&
          (ipPort->role == prrBackup)) {
        SetTimerVar(&ipPort->rbWhile, ipPort->pRstpBridge->BridgeTimes.HelloTime);
        ipPort->rbWhile *= 2;
        ipPort->prtState = prtBACKUP_PORT;
        return;
      }
      CalcForwardDelay(ipPort, ipBPortDesc); // ! вычисление ipPort->forwardDelay
      if ((CompareTimerVar(&ipPort->fdWhile, ipPort->forwardDelay) != 0) ||
          ipPort->sync || ipPort->reRoot || !ipPort->synced) {
        ipPort->role = prrAlternate;
        SetTimerVar(&ipPort->fdWhile, ipPort->pRstpBridge->BridgeTimes.ForwardDelay);
        ipPort->synced = true;
        ipPort->rrWhile = 0;
        ipPort->sync = ipPort->reRoot = false;
        //ipPort->prtState = prtALTERNATE_PORT;
        return;
      }
      break;
    default:
      break;
    }
  }
}

//17.30 Port State Transition state machine
void disableForwarding(struct sBPortDesc * ipBPortDesc);
void enableLearning(struct sBPortDesc * ipBPortDesc);
void enableForwarding(struct sBPortDesc * ipBPortDesc);

void ProcessPortStateTransitions(struct sBPort * ipPort, struct sBPortDesc * ipBPortDesc) {
  if (ipPort->pRstpBridge->BEGIN) {
    disableLearning(ipPort, ipBPortDesc);
    ipPort->learning = false;
    disableForwarding(ipBPortDesc);
    ipPort->forwarding = false;
    ipPort->pstState = pstDISCARDING;
    return;
  }
  switch (ipPort->pstState) {
  case pstDISCARDING:
    if (ipPort->learn) {
      enableLearning(ipBPortDesc);
      ipPort->learning = true;
      ipPort->pstState = pstLEARNING;
    }
    break;
  case pstLEARNING:
    if (ipPort->forward) {
      enableForwarding(ipBPortDesc);
      ipPort->forwarding = true;
      ipPort->pstState = pstFORWARDING;
      break;
    }
    if (!ipPort->learn) {
      disableLearning(ipPort, ipBPortDesc);
      ipPort->learning = false;
      disableForwarding(ipBPortDesc);
      ipPort->forwarding = false;
      ipPort->pstState = pstDISCARDING;
    }
    break;
  case pstFORWARDING:
    if (!ipPort->forward) {
      disableLearning(ipPort, ipBPortDesc);
      ipPort->learning = false;
      disableForwarding(ipBPortDesc);
      ipPort->forwarding = false;
      ipPort->pstState = pstDISCARDING;
    }
    break;
  default:
    break;
  }
}

//17.31 Topology Change state machine
void newTcWhile(struct sBPort * ipPort, struct sBPortDesc * ipBPortDesc);
void setTcPropTree(struct sBPort * ipPort);

void ProcessTopologyChange(struct sBPort * ipPort, struct sBPortDesc * ipBPortDesc) {
  if (ipPort->pRstpBridge->BEGIN) {
    ipPort->fdbFlush = true;
    ipPort->tcWhile = 0;
    ipPort->tcAck = false;
    ipPort->tcState = tcINACTIVE;
    return;
  }
  switch (ipPort->tcState) {
  case tcINACTIVE:
    if ((ipPort->learn) && (!ipPort->fdbFlush)) {
      ipPort->rcvdTc = ipPort->rcvdTcn = ipPort->rcvdTcAck = false;
//      ipPort->tcAck = false;
      ipPort->tcProp = false;
      ipPort->tcState = tcLEARNING;
    }
    break;
  case tcLEARNING:
    if (((ipPort->role == prrRoot) || (ipPort->role == prrDesignated)) &&
        ipPort->forward && !ipPort->operEdge) {
      newTcWhile(ipPort, ipBPortDesc);
      setTcPropTree(ipPort);
      ipPort->newInfo = true;
      ipPort->tcState = tcDETECTED;
      break;
    }
    if ((ipPort->role != prrRoot) && (ipPort->role != prrDesignated) &&
        !(ipPort->learn || ipPort->learning) &&
        !(ipPort->rcvdTc || ipPort->rcvdTcn || ipPort->rcvdTcAck || ipPort->tcProp)) {
      ipPort->fdbFlush = true;
      ipPort->tcWhile = 0;
      ipPort->tcAck = false;
      ipPort->tcState = tcINACTIVE;
      break;
    }
    if (ipPort->rcvdTc || ipPort->rcvdTcn || ipPort->rcvdTcAck || ipPort->tcProp) {
      ipPort->rcvdTc = ipPort->rcvdTcn = ipPort->rcvdTcAck = false;
      ipPort->rcvdTc = ipPort->tcProp = false;
      //ipPort->tcState = tcLEARNING;
    }
    break;
  case tcNOTIFIED_TCN:
    newTcWhile(ipPort, ipBPortDesc);
    ipPort->tcState = tcNOTIFIED_TC;
    break;
  case tcDETECTED:
  case tcNOTIFIED_TC:
  case tcPROPAGATING:
  case tcACKNOWLEDGED:
    ipPort->tcState = tcACTIVE;
    break;
  case tcACTIVE:
    if (((ipPort->role != prrRoot) && (ipPort->role != prrDesignated)) ||
        ipPort->operEdge) {
      ipPort->rcvdTc = ipPort->rcvdTcn = ipPort->rcvdTcAck = false;
      ipPort->rcvdTc = ipPort->tcProp = false;
      ipPort->tcState = tcLEARNING;
      break;
    }
    if (ipPort->rcvdTcn) {
      newTcWhile(ipPort, ipBPortDesc);
      ipPort->tcState = tcNOTIFIED_TCN;
      break;
    }
    if (ipPort->rcvdTc) {
      ipPort->rcvdTcn = ipPort->rcvdTc = false;
      if (ipPort->role == prrDesignated)
        ipPort->tcAck = true;
      setTcPropTree(ipPort); //В 17.31 - setTcPropBridge() (?)
      ipPort->tcState = tcNOTIFIED_TC;
      break;
    }
    if (ipPort->tcProp && !ipPort->operEdge) {
      newTcWhile(ipPort, ipBPortDesc);
      ipPort->fdbFlush = true;
      ipPort->tcProp = false;
      ipPort->tcState = tcPROPAGATING;
      break;
    }
    if (ipPort->rcvdTcAck) {
      ipPort->tcWhile = 0;
      ipPort->rcvdTcAck = false;
      ipPort->tcState = tcACKNOWLEDGED;
    }
    break;
  default:
    break;
  }
}

int gaPortRx[4] = { 0, 0, 0, 0 }; //Счетчики входных RSTP кадров

int8_t FrameReceived(struct sRstpBridge * ipRstpBridge) {
  int8_t cPortIdx = -1;
  if (!ipRstpBridge->BEGIN) {
    struct ethrstp_hdr cRstpFrame;
    struct ethrstp_hdr * cpRstpFrame = &cRstpFrame;
#if (!NO_SYS)
    if (pdTRUE == xQueueReceive(gQueueStp, cpRstpFrame, 2)) {
#else
    if (xQueueReceive(&gQueueStp, (uint8_t *)cpRstpFrame)) {
#endif
      cPortIdx = (cRstpFrame.ForwardPorts & 0x03);
      ++gaPortRx[cPortIdx];
      struct sBPort * cpBPort = &ipRstpBridge->aPort[cPortIdx]; //Объект порта
      memcpy((void *)&cpBPort->LastMsg, (void *)cpRstpFrame, sizeof(struct ethrstp_hdr));
      cpBPort->rcvdBPDU = true;
    }
  }
  return cPortIdx;
}

void ProcessSinglePortSM(struct sRstpBridge * ipRstpBridge, int8_t iPortIdx) { //Обработка всех КА одного порта
  int8_t cPortIdx = -1;
  if (!ipRstpBridge->BEGIN)
    cPortIdx = FrameReceived(ipRstpBridge);
  
  struct sBPort * cpPort = &ipRstpBridge->aPort[iPortIdx];
  struct sBPortDesc * cpBPortDesc = &ipRstpBridge->pRstpBridgeDesc->aBPortDesc[iPortIdx];
  
  if ((cPortIdx == iPortIdx) || (cpPort->prState == prRECEIVE))
    ProcessPortReceive(cpPort, cpBPortDesc);
  ProcessPortProtocolMigration(cpPort, cpBPortDesc);
  ProcessPortTransmit(cpPort, cpBPortDesc);
  ProcessPortInformation(cpPort, cpBPortDesc);
  ProcessPortInformation(cpPort, cpBPortDesc);
  ProcessPortRoleTransitions(cpPort, cpBPortDesc);
  ProcessPortStateTransitions(cpPort, cpBPortDesc);
  ProcessTopologyChange(cpPort, cpBPortDesc);
  if (iPortIdx == (PORT_COUNT - 1)) { //Обработан последний порт
    ProcessPortRoleSelection(ipRstpBridge);
    if (ipRstpBridge->BEGIN)
      ipRstpBridge->BEGIN = false;
  }
}

void ProcessPortSM(struct sRstpBridge * ipRstpBridge) { //Обработка всех КА
  int8_t cPortIdx = FrameReceived(ipRstpBridge);

  for (int i = 0; i < PORT_COUNT; ++i)
    ProcessPortReceive(&ipRstpBridge->aPort[i], &ipRstpBridge->pRstpBridgeDesc->aBPortDesc[i]);
  for (int i = 0; i < PORT_COUNT; ++i)
    ProcessPortProtocolMigration(&ipRstpBridge->aPort[i], &ipRstpBridge->pRstpBridgeDesc->aBPortDesc[i]);
  for (int i = 0; i < PORT_COUNT; ++i)
    ProcessBridgeDetection(&ipRstpBridge->aPort[i], &ipRstpBridge->pRstpBridgeDesc->aBPortDesc[i]);
  for (int i = 0; i < PORT_COUNT; ++i)
    ProcessPortTransmit(&ipRstpBridge->aPort[i], &ipRstpBridge->pRstpBridgeDesc->aBPortDesc[i]);
  for (int i = 0; i < PORT_COUNT; ++i)
    ProcessPortInformation(&ipRstpBridge->aPort[i], &ipRstpBridge->pRstpBridgeDesc->aBPortDesc[i]);
  for (int i = 0; i < PORT_COUNT; ++i)
    ProcessPortInformation(&ipRstpBridge->aPort[i], &ipRstpBridge->pRstpBridgeDesc->aBPortDesc[i]);
  for (int i = 0; i < PORT_COUNT; ++i)
    ProcessPortRoleTransitions(&ipRstpBridge->aPort[i], &ipRstpBridge->pRstpBridgeDesc->aBPortDesc[i]);
  for (int i = 0; i < PORT_COUNT; ++i)
    ProcessPortStateTransitions(&ipRstpBridge->aPort[i], &ipRstpBridge->pRstpBridgeDesc->aBPortDesc[i]);
  for (int i = 0; i < PORT_COUNT; ++i)
    ProcessTopologyChange(&ipRstpBridge->aPort[i], &ipRstpBridge->pRstpBridgeDesc->aBPortDesc[i]);
  
  ProcessPortRoleSelection(ipRstpBridge);
  
  if (ipRstpBridge->BEGIN)
    ipRstpBridge->BEGIN = false;
}

//Сравнение беззнаковых
int memucmp(uint8_t * iSrc, uint8_t * iDst, int iSize) {
  for (int i = 0; i < iSize; ++i) {
    if (iSrc[i] < iDst[i]) {
      return -1;
    } else if (iSrc[i] > iDst[i]) {
      return 1;
    }
  }
  return 0;
}

//=== Процедуры по 802.1D

int8_t ComparePriVectors(struct sPortPriVector * iSrcVector,
                         struct sPortPriVector * iDstVector) {
  int8_t cCmp;
  cCmp = memucmp((uint8_t *)&iSrcVector->RootBridgeID, (uint8_t *)&iDstVector->RootBridgeID, 8);
  if (cCmp != 0)
    return cCmp;
  cCmp = memucmp((uint8_t *)&iSrcVector->RootPathCost, (uint8_t *)&iDstVector->RootPathCost, 4);
  if (cCmp != 0)
    return cCmp;
  cCmp = memucmp((uint8_t *)&iSrcVector->DesignatedBridgeID, (uint8_t *)&iDstVector->DesignatedBridgeID, 8);
  if (cCmp != 0)
    return cCmp;
  cCmp = memucmp((uint8_t *)&iSrcVector->DesignatedPortID, (uint8_t *)&iDstVector->DesignatedPortID, 2);
  if (cCmp != 0)
    return cCmp;
  cCmp = memucmp((uint8_t *)&iSrcVector->DesignatedBridgeID.MAC, (uint8_t *)&iDstVector->DesignatedBridgeID.MAC, ETH_HWADDR_LEN);
  uint16_t cPortNum, cPortNum1;
  cPortNum = PP_HTONS(iSrcVector->DesignatedPortID & 0x0fff);
  cPortNum1 = PP_HTONS(iDstVector->DesignatedPortID & 0x0fff);
  if ((cPortNum == cPortNum1) && (cCmp == 0))
    return 0;
  return cCmp;
}

bool betterorsameinfo(enum eInfoIs iNewInfoIs, struct sBPort * ipPort) {
  if ((iNewInfoIs == iiReceived) && (ipPort->infoIs == iiReceived)) {
    if (ComparePriVectors(&ipPort->msgPriority, &ipPort->portPriority) <= 0)
      return true;
  }
  if ((iNewInfoIs == iiMine) && (ipPort->infoIs == iiMine)) {
    if (ComparePriVectors(&ipPort->designatedPriority, &ipPort->portPriority) <= 0)
      return true;
  }
  return false;
}

void clearReselectTree(struct sRstpBridge * ipRstpBridge) { //Очистить reselect всех портов
  for (int i = 0; i < PORT_COUNT; ++i)
    ipRstpBridge->aPort[i].reselect = false;
}

//Broadcast Storm Protection
void SetBsp(bool iOn) {
  for (int i = 0; i < PORT_COUNT; ++i) {
    SetPortKey(SW1, iOn, i, 0, 0x80); // Port BSP Enable = true
#ifdef TWO_SWITCH_ITEMS
    SetPortKey(SW2, iOn, i, 0, 0x80); // Port BSP Enable = true
#endif
  }
  SetSwRegBit(!iOn, 6, 4, SW1, false); // BSP Disable = false
#ifdef TWO_SWITCH_ITEMS
  SetSwRegBit(!iOn, 6, 4, SW2, false); // BSP Disable = false
#endif
}

//Включение и выключение порта с питанием и светодиодом
//Порт указывается для свитча
#include "extio.h"
#include "snmp_trap.h"
void SetPortEnabled(ksz8895fmq_t * pSW, bool iEnable, int8_t iPortIdx) {
  bool cChanged = SetPortKey(pSW, iEnable, iPortIdx, 2, 0x06);
  if (cChanged) {
    enum eEventCode cEventCode = ecRstpOff;
    if (iEnable)
      cEventCode = ecRstpOn;
    SendSnmpTrapPort(iPortIdx + 1, cEventCode);
  }
}

extern ksz8895fmq_t * SelectSwitch_Port(uint8_t _port, ksz8895fmq_port_t * oPort);

bool IsPortEnabled(uint8_t iPortIdx) {
  if (iPortIdx >= PORT_NUMBER)
    return false;
  ksz8895fmq_t *psw;
  ksz8895fmq_port_t port;
  psw = SelectSwitch_Port(iPortIdx, &port);
  bool cEnabled = fsettings->sw.ports[iPortIdx].port_enabled;
  if (psw == SW1)
    cEnabled = cEnabled & gaRstpBridgeDesc[0].aBPortDesc[port].portEnabled;
  return cEnabled;
}

//Эти ф-и устанавливают биты свитча
//По дескриптору порта
void disableForwarding(struct sBPortDesc * ipBPortDesc) { //
  SetPortEnabled(SW1, false, ipBPortDesc->PortIdx);
}

void enableForwarding(struct sBPortDesc * ipBPortDesc) { //
  if (ipBPortDesc->portEnabled)
    SetPortEnabled(SW1, true, ipBPortDesc->PortIdx);
}

//По свитчу и индексу порта
void disableSwPortForwarding(ksz8895fmq_t * pSW, int iPortIdx) { //
  SetPortEnabled(pSW, false, iPortIdx);
}

void enableSwPortForwarding(ksz8895fmq_t * pSW, int iPortIdx) { //
  SetPortEnabled(pSW, true, iPortIdx);
}

void disablePorts() {
  for (int i = 0; i < PORT_COUNT; ++i) {
    if (gaRstpBridge[0].aPort[i].forward) {
      disableForwarding(&gaRstpBridgeDesc[0].aBPortDesc[i]);
    }
#ifdef TWO_SWITCH_ITEMS
    disableSwPortForwarding(SW2, i);
#endif
  }
}

void enablePorts() {
  ksz8895fmq_t *psw;
  ksz8895fmq_port_t port;
  for (int i = 0; i < PORT_NUMBER; ++i) {
    if (IsPortEnabled(i)) {
      psw = SelectSwitch_Port(i, &port);
      enableSwPortForwarding(psw, port);
    }
  }
}

void disableSwPorts(uint8_t iSwIdx) {
  if (iSwIdx >= 2)
    return;
#ifndef TWO_SWITCH_ITEMS
  if (iSwIdx != 0)
    return;
#endif
  ksz8895fmq_t * pSW = &sw[iSwIdx];
  for (int i = 0; i < PORT_COUNT; ++i) {
    disableSwPortForwarding(pSW, i);
  }
}

void setPortsEnable(bool iEnable) {
  if (iEnable) {
    enablePorts();
  } else {
    disablePorts();
  }
}

void disableLearning(struct sBPort * ipPort, struct sBPortDesc * ipBPortDesc) { //
  SetPortKey(SW1, true, ipBPortDesc->PortIdx, 2, 0x01);
  SetSwRegBit(true, 5, 2, SW1, false); // + очистка таблицы DMAC этого порта
  SetSwRegBit(true, 4, 2, SW1, false); // + очистка таблицы SMAC этого порта
  ipPort->pRstpBridge->aPort[ipBPortDesc->PortIdx].fdbFlush = false;
}

void enableLearning(struct sBPortDesc * ipBPortDesc) { //
  SetPortKey(SW1, false, ipBPortDesc->PortIdx, 2, 0x01);
}

void enableRstpPorts() {
  for (int i = 0; i < PORT_COUNT; ++i) {
    enableForwarding(&gaRstpBridgeDesc[0].aBPortDesc[i]);
    enableLearning(&gaRstpBridgeDesc[0].aBPortDesc[i]);
  }
}

void newTcWhile(struct sBPort * ipPort, struct sBPortDesc * ipBPortDesc) {
  if (ipPort->tcWhile == 0) {
    if (ipPort->sendRSTP) {
      SetConstTimerVar(&ipPort->tcWhile, ipBPortDesc->pRstpBridgeDesc->HelloTime + 1);
      ipPort->newInfo = true;
    } else {
      int32_t cWhile;
      SetTimerVar(&ipPort->tcWhile, ipPort->pRstpBridge->RootTimes.MaxAge);
      SetTimerVar(&cWhile, ipPort->pRstpBridge->RootTimes.ForwardDelay);
      ipPort->tcWhile += cWhile;
    }
  }  
}

bool TimesAreDiffer(struct sTimes * iSrcTimes, struct sTimes * iDstTimes) {
  //Различие блоков времени
  if (iSrcTimes->MessageAge != iDstTimes->MessageAge)
    return true;
  if (iSrcTimes->MaxAge != iDstTimes->MaxAge)
    return true;
  if (iSrcTimes->ForwardDelay != iDstTimes->ForwardDelay)
    return true;
  if (iSrcTimes->HelloTime != iDstTimes->HelloTime)
    return true;
  return false;
}

void RefreshRootAddr(struct sBPort * ipPort) {
  // !!! Дополнение: Если ipPort->prtState == prtROOT_PORT || prtALTERNATE_PORT
  // и новый MAC адрес
  if ((ipPort->prtState == prtROOT_PORT) || (ipPort->prtState == prtALTERNATE_PORT)) {
    int8_t cDiff;
    cDiff = memucmp((uint8_t *)&ipPort->msgPriority.RootBridgeID,
                    (uint8_t *)&ipPort->designatedPriority.RootBridgeID,
                    ETH_HWADDR_LEN + 2);
    if (cDiff) {
      //Векторы порта
      struct sRstpBridge * cpBridge = ipPort->pRstpBridge;
      for (int i = 0; i < PORT_COUNT; ++i) {
        memcpy((uint8_t *)&cpBridge->aPort[i].designatedPriority.RootBridgeID,
               (uint8_t *)&ipPort->msgPriority.RootBridgeID, ETH_HWADDR_LEN + 2);
        memcpy((uint8_t *)&cpBridge->aPort[i].designatedPriority.RootBridgeID,
               (uint8_t *)&ipPort->portPriority.RootBridgeID, ETH_HWADDR_LEN + 2);
      }
      //Векторы моста
      memcpy((uint8_t *)&ipPort->pRstpBridge->RootPriority.RootBridgeID,
             (uint8_t *)&ipPort->msgPriority.RootBridgeID, ETH_HWADDR_LEN + 2);
      memcpy((uint8_t *)&ipPort->pRstpBridge->RootPriority.DesignatedBridgeID,
             (uint8_t *)&ipPort->portPriority.RootBridgeID, ETH_HWADDR_LEN + 2);
    }
  }
}

enum eInfoType rcvInfo(struct sBPort * ipPort) {
  memcpy((void *)&ipPort->msgPriority, (void *)&ipPort->LastMsg.RootId,
         sizeof(ipPort->msgPriority) - 2);
  RefreshRootAddr(ipPort);
  ipPort->msgTimes.MessageAge   = ipPort->LastMsg.MessageAge;
  ipPort->msgTimes.MaxAge       = ipPort->LastMsg.MaxAge;
  ipPort->msgTimes.ForwardDelay = ipPort->LastMsg.ForwardDelay;
  ipPort->msgTimes.HelloTime    = ipPort->LastMsg.HelloTime;
  uint8_t cRole = (ipPort->LastMsg.Flags >> 2) & 0x03;
  int8_t cDiff = ComparePriVectors(&ipPort->msgPriority, &ipPort->portPriority);
  //The received message conveys a Designated Port Role
  if (cRole == prfDesignated) {
    //The message priority is superior (17.6) to the Port’s port priority vector
    if (cDiff < 0) {
      return itSuperiorDesignatedInfo;
    }
    if (cDiff > 0) {
      return itInferiorDesignatedInfo;
    }
    if ((cDiff == 0) && (!TimesAreDiffer(&ipPort->msgTimes, &ipPort->portTimes))) {
      return itRepeatedDesignatedInfo;
    } else {
      return itSuperiorDesignatedInfo;
    }
  }
  if ((cRole == prfRoot) || (cRole == prfAltBack)) {
    if (cDiff >= 0) {
      return itInferiorRootAlternateInfo;
    }
  }
  return itOtherInfo;
}

void recordAgreement(struct sBPort * ipPort, struct sBPortDesc * ipBPortDesc) { //17.21.9 recordAgreement()
  if (ipBPortDesc->rstpVersion && ipPort->operPointToPointMAC &
      (ipPort->LastMsg.Flags & RSTP_AGREEMENT)) {
    ipPort->agreed = true;
    ipPort->proposing = false;
  } else {
    ipPort->agreed = false;
  }
}

void recordDispute(struct sBPort * ipPort) {
  if (ipPort->learning || ipPort->forwarding)
    ipPort->disputed = true;
  if (ipPort->LastMsg.Flags & RSTP_PSTATE_LEARN) {
    ipPort->agreed = true;
    ipPort->proposing = false;
  }
}

void recordProposal(struct sBPort * ipPort) {
  enum ePortRoleFlag cRole = (enum ePortRoleFlag)((ipPort->LastMsg.Flags & RSTP_PROLE_MASK) >> 2);
  if ((cRole == prfDesignated) && (ipPort->LastMsg.Flags & RSTP_PROPOSAL)) {
    ipPort->proposed = true;
  }
}

void recordPriority(struct sBPort * ipPort) {
  memcpy((void *)&ipPort->portPriority, (void *)&ipPort->msgPriority,
         sizeof(ipPort->msgPriority) - 2);
}

void recordTimes(struct sBPort * ipPort) {
  ipPort->portTimes.MessageAge   = ipPort->msgTimes.MessageAge;
  ipPort->portTimes.MaxAge       = ipPort->msgTimes.MaxAge;
  ipPort->portTimes.ForwardDelay = ipPort->msgTimes.ForwardDelay;
  uint16_t cHTime = PP_HTONS(ipPort->msgTimes.HelloTime);
  if (cHTime < MIN_HELLO_TIME)
    cHTime = MIN_HELLO_TIME;
  ipPort->portTimes.HelloTime = PP_HTONS(cHTime);
}

void setSyncTree(struct sRstpBridge * ipRstpBridge) {
  for (int i = 0; i < PORT_COUNT; ++i)
    ipRstpBridge->aPort[i].sync = true;
}

void setReRootTree(struct sRstpBridge * ipRstpBridge) {
  for (int i = 0; i < PORT_COUNT; ++i)
    ipRstpBridge->aPort[i].reRoot = true;
}

void setSelectedTree(struct sRstpBridge * ipRstpBridge) {
  for (int i = 0; i < PORT_COUNT; ++i) {
    if (ipRstpBridge->aPort[i].reselect)
      return;
  }
  for (int i = 0; i < PORT_COUNT; ++i)
    ipRstpBridge->aPort[i].selected = true;
}

void setTcFlags(struct sBPort * ipPort) { //17.21.17 setTcFlags()
  if (ipPort->LastMsg.Flags & RSTP_TC) {
    ipPort->rcvdTc = true;
    ipPort->rcvdTcn = true;
  }
  if (ipPort->LastMsg.Flags & RSTP_TC_ACK)
    ipPort->rcvdTcAck = true;
}

void setTcPropTree(struct sBPort * ipPort) {
  for (int i = 0; i < PORT_COUNT; ++i) {
    if (&ipPort->pRstpBridge->aPort[i] != ipPort)
      ipPort->pRstpBridge->aPort[i].tcProp = true;
  }
}

uint8_t GetBpduRole(enum ePortRstpRole iRole) {
  uint8_t cRole = 0;
  switch (iRole) {
  case prrDisabled:                 break;
  case prrRoot:       cRole = 2;    break;
  case prrDesignated: cRole = 3;    break;
  case prrAlternate:
  case prrBackup:     cRole = 1;    break;
  default:                          break;
  }
  return cRole;
}

void FillRstpBpdu(struct sBPort * ipPort, struct sBPortDesc * ipBPortDesc, struct sBpdu * ipBpdu) {
  //Содержимое BPDU
  ipBpdu->Buf = 0;
  ipBpdu->Type = btRstp;
  uint16_t cPoptId = GetPortId(ipBPortDesc);
  memcpy((uint8_t *)&ipBpdu->PortId, (uint8_t *)&ipPort->designatedPriority.DesignatedPortID, 2);
  uint32_t cRootPathCost = PP_HTONL(ipPort->designatedPriority.RootPathCost);
  memcpy((uint8_t *)&ipBpdu->RootPathCost, (uint8_t *)&cRootPathCost, 4);
  
  memcpy((uint8_t *)&ipBpdu->BridgeId, (uint8_t *)&ipPort->designatedPriority.DesignatedBridgeID, 8);
  memcpy((uint8_t *)&ipBpdu->RootId, (uint8_t *)&ipPort->designatedPriority.RootBridgeID, 8);
  
  //Флаги надо заполнять по текущей ситуации: по порту и его параметрам
  ipBpdu->Flags = 0;
  //Параметры в ipSwitchStp - заносятся из кадра, поступающенго от корневого порта
  ipBpdu->MaxAge = ipPort->designatedTimes.MaxAge;
  ipBpdu->MessageAge = 0; //При ретрансляции - должно вычисляться из входящего по RootPort
  ipBpdu->HelloTime = ipPort->designatedTimes.HelloTime;
  ipBpdu->ForwardDelay = ipPort->designatedTimes.ForwardDelay;
  ipBpdu->Version1Size = 0;
  memset(&ipBpdu->PaddingBytes[0], 0, RSTP_PADDING_BYTES);
  ipBpdu->ForwardPorts = (1 << (ipBPortDesc->PortIdx))/* | 0x40*/;
}

extern err_t low_level_output(struct netif *netif, struct pbuf *p);

void TxRstpFrame(struct sBpdu * ipBpdu, struct sBPortDesc * ipBPortDesc) {
  //Передача кадра BPDU в свитч
  
  struct pbuf *p;
  uint16_t cSize = SIZEOF_ETH_HDR + SIZEOF_STP_HDR + RSTP_PADDING_BYTES + 1;
  p = pbuf_alloc(PBUF_LINK, cSize, PBUF_RAM); /* allocate a pbuf for the outgoing STP request packet */
  if (p == NULL) { /* could allocate a pbuf for an ARP request? */
    return;
  }
  uint8_t * cpHead = (uint8_t *)p->payload;
  // !!! Для отладки
  struct sRstpBlock * cpRstpBlock = (struct sRstpBlock *)p->payload;
  // !!!
  // Передача на MAC low level output
  // LLC - заголовок
  memcpy(&cpHead[0], &ethrstp, ETH_HWADDR_LEN);
  memcpy(&cpHead[6], &ipBpdu->BridgeId.MAC, ETH_HWADDR_LEN);
  uint16_t cLen = PP_HTONS(SIZEOF_STP_HDR);
  memcpy(&cpHead[12], (uint8_t *)&cLen, 2);
  cpHead[SIZEOF_ETH_HDR + 0] = STP_LLC_SAP;  cpHead[SIZEOF_ETH_HDR + 1] = STP_LLC_SAP;
  cpHead[SIZEOF_ETH_HDR + 2] = 3;
  // STP - заголовок
  cpHead[SIZEOF_ETH_HDR + 3] = 0;  cpHead[SIZEOF_ETH_HDR + 4] = 0; //Тип протокола = STP
  cpHead[SIZEOF_ETH_HDR + 5] = RSTP_PROT_VER;
  
  memcpy((uint8_t *)p->payload + SIZEOF_ETH_HDR + 6, (uint8_t *)&ipBpdu->Type, SIZEOF_STP_HDR - 6 + RSTP_PADDING_BYTES + 1);
  
  err_t cErr = low_level_output(&gnetif, p);
  
  /* free RSTP query packet */
  pbuf_free(p);
  p = NULL;
#if (!NO_SYS)
  vTaskDelay(2);
#endif
}

void txConfig(struct sBPort * ipPort, struct sBPortDesc * ipBPortDesc) {
  struct sBpdu cBpdu;
  FillRstpBpdu(ipPort, ipBPortDesc, &cBpdu);
  //Флаги надо заполнять по текущей ситуации: по порту и его параметрам
  cBpdu.Flags |= ((GetBpduRole(ipPort->role) & 0x03) << 2);
  if (ipPort->tcWhile != 0)
    cBpdu.Flags |= RSTP_TC;
  if (ipPort->tcAck)
    cBpdu.Flags |= RSTP_TC_ACK;
  TxRstpFrame(&cBpdu, ipBPortDesc);
}

void txRstp(struct sBPort * ipPort, struct sBPortDesc * ipBPortDesc) {
  struct sBpdu cBpdu;
  FillRstpBpdu(ipPort, ipBPortDesc, &cBpdu);
  //Заполнить флаги
  cBpdu.Flags |= ((GetBpduRole(ipPort->role) & 0x03) << 2);
  if (ipPort->agree)
    cBpdu.Flags |= RSTP_AGREEMENT;
  if (ipPort->proposing)
    cBpdu.Flags |= RSTP_PROPOSAL;
  if (ipPort->tcWhile != 0)
    cBpdu.Flags |= RSTP_TC;
  if (ipPort->learning)
    cBpdu.Flags |= RSTP_PSTATE_LEARN;
  if (ipPort->forwarding)
    cBpdu.Flags |= RSTP_PSTATE_FORWD;
//  if (ipPort->tcAck)
//    cBpdu.Flags |= RSTP_TC_ACK;
  TxRstpFrame(&cBpdu, ipBPortDesc);
}

void txTcn(struct sBPort * ipPort, struct sBPortDesc * ipBPortDesc) {
  struct sBpdu cBpdu;
  FillRstpBpdu(ipPort, ipBPortDesc, &cBpdu);
  //Заполнить флаги
  cBpdu.Flags |= RSTP_TC;
  TxRstpFrame(&cBpdu, ipBPortDesc);
}

void updtBPDUVersion(struct sBPort * ipPort) { //17.21.22
  if (ipPort->LastMsg.ProtVerId == RSTP_PROT_VER) {
    ipPort->rcvdRSTP = true;
    ipPort->rcvdSTP = false;
  } else {
    ipPort->rcvdRSTP = false;
    ipPort->rcvdSTP = true;
  }
}

void updtRcvdInfoWhile(struct sBPort * ipPort) { //17.21.23
  uint16_t cMessageAge = (PP_HTONS(ipPort->portTimes.MessageAge) + 256) & 0xff00;
  uint16_t cMaxAge = PP_HTONS(ipPort->portTimes.MaxAge);
  // !!!
  if (ipPort == &ipPort->pRstpBridge->aPort[1]) {
    cMaxAge = PP_HTONS(ipPort->portTimes.MaxAge);
  }
  // !!!
  if (cMessageAge <= cMaxAge) {
    SetTimerVar(&ipPort->rcvdInfoWhile, ipPort->portTimes.HelloTime);
    ipPort->rcvdInfoWhile *= 3;
  } else {
    ipPort->rcvdInfoWhile = 0;
  }
}

void updtRoleDisabledTree(struct sRstpBridge * ipRstpBridge) { //17.21.24
  for (int i = 0; i < PORT_COUNT; ++i)
    ipRstpBridge->aPort[i].selectedRole = prrDisabled;
}

void SetPortPriority(struct sPortPriVector * ipPortPriority, struct sBPort * ipPort, struct sBPortDesc * ipBPortDesc) {
  //17.21.25 a) Вычислить root path priority vector для порта ipPort
  if (ipPort->infoIs == iiReceived) {
    memcpy((void *)&ipPortPriority->RootBridgeID, (void *)&ipPort->msgPriority.RootBridgeID,
           sizeof(ipPort->portPriority) - 2);
    memcpy((void *)&ipPortPriority->BridgePortID, (void *)&ipPort->portId, 2);
    ipPortPriority->RootPathCost += ipBPortDesc->PortPathCost; //ipPort->PortPathCost;
  } else {
    memcpy((void *)ipPortPriority, (void *)&ipPort->pRstpBridge->BridgePriority,
           sizeof(ipPort->pRstpBridge->BridgePriority));
  }
}

int8_t gDiffCount = 0;
void updtPortRole(struct sBPort * ipPort, struct sBPortDesc * ipBPortDesc, int8_t iDerivedIdx, int8_t iRootIdx) {
  //17.21.25
//  if (iDerivedIdx != iRootIdx) {
//    ++gDiffCount;
//  }
  if (!ipBPortDesc->portEnabled) { // f)
    ipPort->selectedRole = prrDisabled;
  } else if (ipPort->infoIs == iiAged) { // g)
    ipPort->selectedRole = prrDesignated;
    ipPort->updtInfo = true;
  } else if (ipPort->infoIs == iiMine) { // h)
    ipPort->selectedRole = prrDesignated;
    bool cTimesAreDiffer = false;
    if (ipPort->pRstpBridge->RootPortIdx >= 0)
      cTimesAreDiffer =
        TimesAreDiffer(&ipPort->portTimes, &ipPort->pRstpBridge->aPort[ipPort->pRstpBridge->RootPortIdx].portTimes);
    if ((ComparePriVectors(&ipPort->portPriority, &ipPort->designatedPriority) != 0) ||
        ((ipPort->pRstpBridge->RootPortIdx >= 0) && cTimesAreDiffer)) {
      ipPort->updtInfo = true;
    }  
  } else if (ipPort->infoIs == iiReceived) { // i), j), k), l)
    //Вычислить 'root priority vector is now derived from port priority vector received in a Configuration Message'
    if (ipBPortDesc->PortIdx == 1) {
      ++gDiffCount;
    }
    if ((iRootIdx == ipBPortDesc->PortIdx) && (iDerivedIdx == ipBPortDesc->PortIdx)) { // i)
      ipPort->selectedRole = prrRoot;
      ipPort->updtInfo = false;
    } else { // j) k) l)
      //the designated priority vector is not higher than the port priority vector (= не лучше (?))
      if (ComparePriVectors(&ipPort->designatedPriority, &ipPort->portPriority) >= 0) {
        //and the designated bridge and designated port components of the port priority
        // vector do not reflect another port on this bridge
        int8_t cDiff = memucmp((void *)&ipPort->pRstpBridge->BridgeId,
                               (void *)&ipPort->portPriority.DesignatedBridgeID,
                               sizeof(ipPort->pRstpBridge->BridgeId));
        if (cDiff == 0) {
          //components of the port priority vector reflect another port on this bridge
          bool cIsPortReflect = false; //
          bool cIsRootPort = false; //
          for (int i = 0; i < PORT_COUNT; ++i) { //
            if (&ipPort->pRstpBridge->aPort[i] != ipPort) {
              if (ipPort->portId == ipPort->pRstpBridge->aPort[i].portPriority.DesignatedPortID) {
                cIsPortReflect = true;
//                break;
              }
              if ((ipPort->pRstpBridge->aPort[i].selectedRole == prrRoot) ||
                  (ipPort->pRstpBridge->aPort[i].role == prrRoot)) {
                cIsRootPort = true;
              }
            }
          }
          if (cIsPortReflect) { // = reflect
            ipPort->selectedRole = prrBackup;
          } else { // = not reflect
            if (cIsRootPort || ((iRootIdx >= 0) && (iRootIdx != ipBPortDesc->PortIdx))) {
              ipPort->selectedRole = prrAlternate;
            } else {
              ipPort->selectedRole = prrRoot;
            }
          }
          ipPort->updtInfo = false;
        } else { // = cDiff != 0
          if (cDiff < 0) { //Сосед - больший ID (= младше по приоритету)
            ipPort->selectedRole = prrDesignated;
          } else {
            ipPort->selectedRole = prrAlternate;
          }
          ipPort->updtInfo = false;
        }
      } else {
        ipPort->selectedRole = prrDesignated;
        ipPort->updtInfo = true;
      }
    }
  }
}

void CalcDestinatedPriority(struct sBPort * ipPort, struct sRstpBridge * ipRstpBridge) {
    memcpy((void *)&ipPort->designatedPriority.RootBridgeID, (void *)&ipRstpBridge->RootPriority.RootBridgeID,
           sizeof(ipRstpBridge->RootPriority.RootBridgeID));
    ipPort->designatedPriority.RootPathCost = ipRstpBridge->RootPriority.RootPathCost;
    //Вычислить designatedTimes портов
    ipPort->designatedTimes.MessageAge   = ipRstpBridge->RootTimes.MessageAge;
    ipPort->designatedTimes.MaxAge       = ipRstpBridge->RootTimes.MaxAge;
    ipPort->designatedTimes.ForwardDelay = ipRstpBridge->RootTimes.ForwardDelay;
    ipPort->designatedTimes.HelloTime    = ipRstpBridge->BridgeTimes.HelloTime;
}

void updtRolesTree(struct sRstpBridge * ipRstpBridge) {
  struct sPortPriVector caPortPriority[PORT_COUNT];
  struct sPortPriVector cBestPortPriority;
  memcpy((void *)&cBestPortPriority, (void *)&ipRstpBridge->BridgePriority, sizeof(ipRstpBridge->BridgePriority));
  //17.21.25 a) root path priority vector для всех портов, имеющих port priority vector
  int cPriorityIdx = -1;
  for (int i = 0; i < PORT_COUNT; ++i) //Вычислить векторы приоритета портов
    SetPortPriority(&caPortPriority[i], &ipRstpBridge->aPort[i], &ipRstpBridge->pRstpBridgeDesc->aBPortDesc[i]);
  // (?? м.б. надо взять для сравнения ipRstpBridge->RootPriority ?)
  //17.21.25 b) [1]
  for (int i = 0; i < PORT_COUNT; ++i) { //Найти наилучший вектор
    if (memcmp((void *)&ipRstpBridge->BridgeId.MAC, (void *)&caPortPriority[i].DesignatedBridgeID.MAC, ETH_HWADDR_LEN) != 0) {
      if (ComparePriVectors(&caPortPriority[i], &cBestPortPriority) < 0) {
        memcpy((void *)&cBestPortPriority, (void *)&caPortPriority[i], sizeof(ipRstpBridge->RootPriority));
        cPriorityIdx = i;
      }
    }
  }
  //17.21.25 b), 17.21.25 c)
  int cDerivedIdx = -1;
  if (cPriorityIdx >= 0) { //Занести в ipRstpBridge->RootPriority
    int8_t cCmp;
    cCmp = memucmp((uint8_t *)&cBestPortPriority.RootBridgeID, (uint8_t *)&ipRstpBridge->BridgePriority.RootBridgeID, 8);
    if (cCmp < 0) { //Если cBestPortPriority->RootBridgeID < BridgeID
      memcpy((void *)&ipRstpBridge->RootPriority, (void *)&cBestPortPriority, sizeof(ipRstpBridge->RootPriority));
      memcpy((void *)&ipRstpBridge->RootTimes, (void *)&ipRstpBridge->aPort[cPriorityIdx].portTimes, sizeof(ipRstpBridge->RootTimes));
      ipRstpBridge->RootPortIdx = cPriorityIdx;
      ipRstpBridge->BridgeRole = brNoRoot;
      cDerivedIdx = cPriorityIdx;
    } else {
      memcpy((void *)&ipRstpBridge->RootPriority, (void *)&ipRstpBridge->BridgePriority, sizeof(ipRstpBridge->RootPriority));
      memcpy((void *)&ipRstpBridge->RootTimes, (void *)&ipRstpBridge->BridgeTimes, sizeof(ipRstpBridge->RootTimes));
      ipRstpBridge->RootPortIdx = -1;
      ipRstpBridge->BridgeRole = brRoot;
    }
  } else {
    memcpy((void *)&ipRstpBridge->RootPriority, (void *)&ipRstpBridge->BridgePriority, sizeof(ipRstpBridge->RootPriority));
    memcpy((void *)&ipRstpBridge->RootTimes, (void *)&ipRstpBridge->BridgeTimes, sizeof(ipRstpBridge->RootTimes));
    ipRstpBridge->RootPortIdx = -1;
    ipRstpBridge->BridgeRole = brRoot;
  }
  if (ipRstpBridge->BridgeRole == brRoot) {
    // !!! Проверка, ipRstpBridge->RootPriority.MAC д.б. равен ipRstpBridge->BridgePriority.MAC
    //    Если нет, то: - заменить RootBridgeID собственным во всех векторах
    //                  - обнулить RootPathCost во всех векторах
  }
  //designated
  //17.21.25 d)
  for (int i = 0; i < PORT_COUNT; ++i) {
    //Вычислить designatedPriority портов
    CalcDestinatedPriority(&ipRstpBridge->aPort[i], ipRstpBridge);
  }
  //Назначение ролей портам
  //17.21.25 f) - l)
  for (int i = 0; i < PORT_COUNT; ++i) {
    updtPortRole(&ipRstpBridge->aPort[i], &ipRstpBridge->pRstpBridgeDesc->aBPortDesc[i], cDerivedIdx, cPriorityIdx);
  }
}

void ProcessSwitchMAC() {
#ifdef TWO_SWITCH_ITEMS
  //Собственный MAC-адрес свитча
  uint8_t caMac[2][6];
  ReadSelfMac(caMac[0], SW1);
  ReadSelfMac(caMac[1], SW2);
  
  int cDiff = memcmp(caMac[0], caMac[1], ETH_HWADDR_LEN);
  if (!cDiff) { //MAC-адреса одинаковы:
    //адрес второго увеличиваем на 1
    caMac[1][5] = caMac[1][5] + 1;
    WriteSelfMac(caMac[1], SW2);
  }
#endif
}
