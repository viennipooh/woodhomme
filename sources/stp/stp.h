//******************************************************************************
// Имя файла    :       stp.h
// Заголовок    :       
// Автор        :       Сосновских А.А.
// Дата         :       07.08.2018
//
//------------------------------------------------------------------------------
/**
Средства поддержки протокола STP по IEEE 802.1d

**/

#ifndef __STP_H
#define __STP_H

#include "dma.h"

#include "ip.h"
#include "pbuf.h"

#include "sw_reg.h"
#include "ethernet.h"
#include "info.h"

#if (!NO_SYS)
  /* Scheduler includes */
  #include "FreeRTOS.h"
  #include "task.h"
  #include "queue.h"
#else

#endif

#include <stdint.h>
#include <string.h>
#include <stdbool.h>

#define RSTP_TASK_PRIO   ( tskIDLE_PRIORITY + 3 )

#define REG_SELF_MAC_ADDR  (0x68)  //Нач. регистр собственного MAC-адреса свитча

#define STP_LLC_SAP   (0x42)


#define SIZEOF_STP_HDR  (39)

//TailTag
#define TAIL_TAG_REG  (0x0c)
#define TAIL_TAG_BIT  (0x01)

#define RSTP_SET_OFF  //Отключение RSTP через Web-интерфейс

//========== Определения STP/RSTP
//Нужны:
//  Приоритет свитча
//  Таблица цен по скорости порта
//  Определения флагов RSTP
//  Цена от корневого свитча
//  Интервалы
//  port priority vector = {RootBridgeID : RootPathCost : DesignatedBridgeID : DesignatedPortID : BridgePortID}
//  message priority vector = {RD : RPCD : D : PD : PB}
//  Флаги
#define RSTP_TC           (0x01)  //Topology Change
#define RSTP_TC_ACK       (0x80)  //Topology Change ACK
////Роли порта
#define RSTP_PROLE_MASK   (0x0c)  //Port Role Mask
//#define RSTP_PROLE_UNKN   (0x00)  //Port Role Unknown
//#define RSTP_PROLE_ALT    (0x04)  //Port Role Alternate
//#define RSTP_PROLE_ROOT   (0x08)  //Port Role Root
//#define RSTP_PROLE_DESG   (0x0c)  //Port Role Designated
//Состояния порта
#define RSTP_PSTATE_LEARN (0x10)  //Port State Learning
#define RSTP_PSTATE_FORWD (0x20)  //Port State Forwarding

#define RSTP_PROPOSAL     (0x02)  //Proposal
#define RSTP_AGREEMENT    (0x40)  //Port Agreement



//====== Блок поддержки STP =======

#define STP_PROT_VER    (0) //Версия протокола STP
#define RSTP_PROT_VER   (2) //Версия протокола RSTP

#define RSTP_HELLO_TIME_S (2) //HelloTime, seconds
#define RSTP_MAX_AGE_S   (20) //(20) //MaxAge, seconds
#define RSTP_FWD_DELAY_S (15) //(15) //ForwardDelay, seconds

#define RSTP_TASK_DELAY (200) //Цикл работы RSTP задачи

#if (NO_SYS)
#define configTICK_RATE_HZ (1000)
#endif

#define RSTP_TIME_COEF (configTICK_RATE_HZ / RSTP_TASK_DELAY)
#define RSTP_TIME_VAR_COEF (1 / 256)

//==== Идентификаторы и приоритеты

#define DEF_BRIDGE_PRIORITY      (8) //4 старших бита BridgeId.Priority
#define DEF_BRIDGE_SYS_ID      (100) //12 младших бит BridgeId.Priority (Ид системы)

#define DEF_PORT_PRIORITY        (8) //4 старших бита Port.Priority
#define DEF_PATH_COST_1000   (20000)
#define DEF_PATH_COST_100   (200000)
#define DEF_PATH_COST_10   (2000000)


//

#define MIGRATE_TIME             (3)
#define DEF_TX_HOLD_COUNT        (6)


#ifdef USE_STP
  //Отключение TagTail и вкл RSTP
  #define ENABLE_TAIL_TAG  (1)
  #define ENABLE_RSTP  (1)
#else
  #define ENABLE_TAIL_TAG  (0)
  #define ENABLE_RSTP  (0)
#endif

//Определения MAC-адресов свитчей в МКПШ-10
#define MAC5_HIGHEST_PRIORITY   (0x00)
#define MAC5_LOWEST_PRIORITY    (0xfe)

#define MAC5_PRIORITY_0      MAC5_HIGHEST_PRIORITY
#define MAC5_PRIORITY_1      (MAC5_HIGHEST_PRIORITY + 0x8e)
#define MAC5_PRIORITY_2      MAC5_LOWEST_PRIORITY

//MAC[5] первого свитча в МКПШ-10
//#define MAC5_PRIORITY_CURR   MAC5_PRIORITY_0
//#define MAC5_PRIORITY_CURR   MAC5_PRIORITY_1
#define MAC5_PRIORITY_CURR   MAC5_PRIORITY_2

//Блок состояния свитча
#define BRIDGE_NUMBER (1)     //Количество мостов STP
#define PORT_COUNT  (4)       //Количество портов моста
#define PORT_COUNT_HTTP  (4)

#define MIN_FRAME_SIZE  (60)

#define RSTP_PADDING_BYTES  (MIN_FRAME_SIZE - SIZEOF_ETH_HDR - SIZEOF_STP_HDR)

#pragma pack(1)

//Идентификатор моста
struct sBridgeId {
  uint16_t        Priority;
  struct eth_addr MAC;
};

//==== Блок состояний =========
//Состояния свитча в STP
enum eSwitchStpState {
  sssNone,          //Пустое
//  sssNoDesignated,  //Не назначен
  sssDesignated,    //Назначен
  sssRoot,          //Корневой
  sssCount          //Количество состояний
};

//Состояния порта в STP
enum ePortStpState {
  pssDisable,   //Отключен
  pssBlocked,   //Блокирован
  pssListen,    //Прослушивание
  pssLearn,     //Обучение
  pssForward,   //Передача
  pssCount      //Количество состояний
};

//Роли порта в STP
enum ePortStpRole {
  psrNoDesignated,  //Не назначен
  psrDesignated,    //Назначен
  psrRoot,          //Корневой
  psrCount          //Количество ролей
};

//Тип порта в STP
enum ePortEndType {
  petNoEdge,  //Не конечный
  petEdge,    //Конечный
  petCount    //Количество типов
};


enum eBpduType {
  btStp   = 0,
  btRstp  = 2,
};

typedef struct sBpdu {
	uint16_t  Buf;              //2 - для выравнивания
	enum eBpduType Type;        //4
  uint8_t   Flags;            //5
	struct sBridgeId RootId;    //6
	uint32_t  RootPathCost;     //14
	struct sBridgeId BridgeId;  //18
	uint16_t  PortId;           //26
	uint16_t  MessageAge;       //28
	uint16_t  MaxAge;           //30
	uint16_t  HelloTime;        //32
	uint16_t  ForwardDelay;     //34
  uint8_t   Version1Size;     //35
  uint8_t   PaddingBytes[RSTP_PADDING_BYTES];  //36  Байты для заполнения до 46
  uint8_t   ForwardPorts;     //47  Порты, куда передаются кадры RSTP
} SBpdu;

#pragma pack()

#ifdef PACK_STRUCT_USE_INCLUDES
#  include "arch/bpstruct.h"
#endif
/** the STP message */
PACK_STRUCT_BEGIN
struct ethrstp_hdr {
  //EthHdr
  PACK_STRUCT_FLD_S(struct eth_addr dHwAddr);
  PACK_STRUCT_FLD_S(struct eth_addr sHwAddr);
  PACK_STRUCT_FIELD(u16_t Length);
  PACK_STRUCT_FLD_8(u8_t  SSAD);  //LLC
  PACK_STRUCT_FLD_8(u8_t  DSAD);
  PACK_STRUCT_FLD_8(u8_t  Ctrl);
  //BPDU
  PACK_STRUCT_FIELD(u16_t ProtId);
  PACK_STRUCT_FLD_8(u8_t  ProtVerId);
  PACK_STRUCT_FLD_8(u8_t  Type);
  PACK_STRUCT_FLD_8(u8_t  Flags);
	PACK_STRUCT_FIELD(struct sBridgeId RootId);    //6
	PACK_STRUCT_FIELD(uint32_t  RootPathCost);     //14
	PACK_STRUCT_FIELD(struct sBridgeId BridgeId);  //18
	PACK_STRUCT_FIELD(uint16_t  PortId);           //26
	PACK_STRUCT_FIELD(uint16_t  MessageAge);       //28
	PACK_STRUCT_FIELD(uint16_t  MaxAge);           //30
	PACK_STRUCT_FIELD(uint16_t  HelloTime);        //32
	PACK_STRUCT_FIELD(uint16_t  ForwardDelay);     //34
  PACK_STRUCT_FIELD(uint8_t   Version1Size);     //35
  PACK_STRUCT_FIELD(uint8_t   PaddingBytes[RSTP_PADDING_BYTES]); //36  Байты для заполнения до 46
  PACK_STRUCT_FIELD(uint8_t   ForwardPorts);     //47  Порты, куда передаются кадры RSTP
  
} PACK_STRUCT_STRUCT;
PACK_STRUCT_END

//Структура для передачи STP кадра (через очередь)
#if (!NO_SYS)
#else
//Очередь вместо xQueue
#define QUEUE_SIZE    (4)
struct sQueue {
  uint8_t aBuf[QUEUE_SIZE][sizeof(struct ethrstp_hdr) + 1];
  uint8_t Start, End;
  bool Full, Empty;
};
#endif

struct sRstpBlock {
  struct ethrstp_hdr  hdr;
  uint8_t             Port2;
  uint16_t            Size;
};
//Кодировка Роли Порта в BPDU, в поле Flags
enum ePortRoleFlag {
  prfUnknown,     //Неизвестный (не должен передаваться в RSTP)
  prfAltBack,     //Альтернативный или Резервный
  prfRoot,        //Корневой
  prfDesignated,  //Назначенный
  prfCount
};

//=== Блоки управления, автоматы RSTP

//Вектор приоритета порта
typedef struct sPortPriVector {
  struct sBridgeId  RootBridgeID;
  uint32_t          RootPathCost;
  struct sBridgeId  DesignatedBridgeID;
  uint16_t          DesignatedPortID;
  uint16_t          BridgePortID;
} SPortPriVector;

#define MIN_HELLO_TIME (1 << 8)

typedef struct sTimes { //Времена и задержки
	uint16_t  MessageAge;       //
	uint16_t  MaxAge;           //
	uint16_t  ForwardDelay;     //
	uint16_t  HelloTime;        //
} STimes;

//Статус информации:
enum eInfoIs {
  iiMine,     //Добыта
  iiAged,     //Устарела
  iiReceived, //Принята
  iiDisabled, //Отключена
  iiCount
};

//Тип принятой информации
enum eInfoType {
  itSuperiorDesignatedInfo,     //
  itRepeatedDesignatedInfo,     //
  itInferiorDesignatedInfo, //
  itInferiorRootAlternateInfo, //
  itOtherInfo, //
  itCount
};

//Роли порта в RSTP
enum ePortRstpRole {
  prrDisabled,      //Отключен
  prrRoot,          //Корневой
  prrDesignated,    //Назначен
  prrAlternate,     //Альтернативный
  prrBackup,        //Резервный
  prrCount          //Количество ролей
};


//Блок порта
struct sRstpBridge;
struct sRstpBridgeDesc;

//Параметры управления портом
/*
a) Port Number—the number of the Bridge Port.
b) Path Cost—the new value (17.13.11).
c) Port Priority—the new value of the priority field for the Port Identifier (17.19.21).
d) adminEdgePort—the new value of the adminEdgePort parameter (17.13.1). Present in
implementations that support the identification of edge ports.
e) autoEdgePort — the new value of the autoEdgePort parameter (17.13.3). Optional and provided
only by RSTP Bridges that support the automatic identification of edge ports.
f) MAC Enabled—the new value of the MAC Enabled parameter (6.4.2). May be present if the
implementation supports the MAC Enabled parameter.
g) adminPointToPointMAC—the new value of the adminPointToPointMAC parameter (6.4.3). May be
present if the implementation supports the adminPointToPointMAC parameter.*/

typedef struct sBPortDesc { //
  uint16_t    PortIdx;        //Индекс порта в массиве

  bool        adminEdge;      //Порт подключен к конечной станции
  bool        autoEdge;       //Автоматическое определение порта как Edge
  
	uint16_t    ForcePriority;  //Приоритет (1-15), задаваемый извне
	uint16_t    MigrateTime;    // 3 сек 
  uint32_t    PortPathCost;   //
  
  bool        rstpVersion;    //Версия RSTP
  bool        stpVersion;
  
  bool        portEnabled;    //Разрешение работы порта
  
  uint16_t    VlanId;         //Идентификатор VLAN (первые 12 бит)
  
  struct sRstpBridgeDesc * pRstpBridgeDesc;
  
} SBPortDesc;

//=== Состояния КА
//17.23 Port Receive state machine
enum ePortReceive {
  prNONE        = 0, //Нет состояния
  prDISCARD     = 1, //Нет приема кадров RSTP
  prRECEIVE     = 2, //Прием кадров RSTP
  prNO_RECEIVE  = 3, //Пауза приема кадров RSTP
  prCount
};

//17.24 Port Protocol Migration state machine
enum ePortProtocolMigration {
  ppmNONE,
  ppmCHECKING_RSTP,
  ppmSELECTING_STP,
  ppmSENSING,
  ppmCount
};

//17.25 Bridge Detection state machine
enum eBridgeDetection {
  bdNONE,
  bdEDGE,
  bdNOT_EDGE,
  bdCount
};

//17.26 Port Transmit state machine
enum ePortTransmit {
  ptNONE,
  ptTRANSMIT_INIT,
  ptIDLE,
  ptTRANSMIT_CONFIG,
  ptTRANSMIT_PERIODIC,
  ptTRANSMIT_TCN,
  ptTRANSMIT_RSTP,
  ptCount
};

//17.27 Port Information state machine
enum ePortInformation {
  piNONE,
  piDISABLED,
  piAGED,
  piUPDATE,
  piSUPERIOR_DESIGNATED,
  piREPEATED_DESIGNATED,
  piINFERIOR_DESIGNATED,
  piNOT_DESIGNATED,
  piOTHER,
  piCURRENT,
  piRECEIVE,
  piCount
};

//17.28 Port Role Selection state machine
enum ePortRoleSelection {
  prsNONE,
  prsINIT_BRIDGE,
  prsROLE_SELECTION,
  prsCount
};

//17.29 Port Role Transitions state machine
enum ePortRoleTransitions {
  prtNONE                 =  0, //Нет роли
  prtINIT_PORT            =  1, //Инициализация порта
  prtDISABLE_PORT         =  2, //Запрещение работы порта
  prtDISABLED_PORT        =  3, //Порт запрещен
  
  prtROOT_PORT            =  4, //Корневой
  prtROOT_PROPOSED        =  5,
  prtROOT_AGREED          =  6,
  prtREROOT               =  7,
  prtROOT_FORWARD         =  8,
  prtROOT_LEARN           =  9,
  prtREROOTED             = 10,
  
  prtDESIGNATED_PORT      = 11, //Назначен
  prtDESIGNATED_PROPOSE   = 12,
  prtDESIGNATED_SYNCED    = 13,
  prtDESIGNATED_RETIRED   = 14,
  prtDESIGNATED_FORWARD   = 15,
  prtDESIGNATED_LEARN     = 16,
  prtDESIGNATED_DISCARD   = 17,
  
  prtALTERNATE_PORT       = 18, //Альтернативный
  prtALTERNATE_PROPOSED   = 19,
  prtALTERNATE_AGREED     = 20, 
  prtBLOCK_PORT           = 21, //Блокирован
  prtBACKUP_PORT          = 22, //Запасной порт
  prtCount
};

//17.30 Port State Transition state machine
enum ePortStateTransitions {
  pstNONE	          = 0,  //Нет состояния
  pstDISCARDING     = 1,  //Порт закрыт
  pstLEARNING       = 2,  //Обучение
  pstFORWARDING     = 3,  //Порт открыт
  pstCount
};

//17.31 Topology Change state machine
enum eTopologyChange {
  tcNONE,
  tcINACTIVE,
  tcLEARNING,
  tcDETECTED,
  tcACTIVE,
  
  tcNOTIFIED_TCN,
  tcNOTIFIED_TC,
  tcPROPAGATING,
  tcACKNOWLEDGED,
  tcCount
};


//Переменные порта
typedef struct sBPort { //
  //Последнее принятое сообщение
  struct ethrstp_hdr  LastMsg;
  //Таймеры (управляются в потоке RSTP)
  int32_t   edgeDelayWhile;
  int32_t   fdWhile;
  int32_t   helloWhen;
  int32_t   mdelayWhile;
  int32_t   rbWhile;
  int32_t   rcvdInfoWhile;
  int32_t   rrWhile;
  int32_t   tcWhile;
  
  //Переменные состояний КА
  enum ePortReceive prState;
  enum ePortProtocolMigration ppmState;
  enum eBridgeDetection bdState;
  enum ePortTransmit ptState;
  enum ePortInformation piState;
  enum ePortRoleTransitions prtState;
  enum ePortStateTransitions pstState;
  enum eTopologyChange tcState;
  
  bool        allSynced;      //Завершение синхронизации (дерева(?))
  //Returns the value of MigrateTime if operPointToPointMAC is TRUE, and the value of MaxAge otherwise.
	uint16_t    EdgeDelay;      //(Вычисляется)
  //Returns the value of HelloTime if sendRSTP is TRUE, and the value of FwdDelay otherwise.
	int32_t     forwardDelay;   //(Вычисляется)
  bool        reRooted;
  
  
	uint16_t  ageingTime;     //7.9.2, Tab 7-5
  bool      agree;          //17.10
  bool      agreed;         //17.10
  struct  sPortPriVector  designatedPriority; //17.6 (первые 4 компонента)
  struct  sTimes          designatedTimes;
  bool      disputed;       //17.21.10
  bool      fdbFlush;       //КА изменения топологии (удалить все записи этого порта)
  
  bool      forward;        //17.30
  bool      forwarding;     //17.30
  enum eInfoIs    infoIs;   //Статус информации
  
  bool      learn;          //17.30
  bool      learning;       //17.30
  bool      mcheck;         //Принудительный запуск КА Протокол Миграции Порта
  
  struct  sPortPriVector  msgPriority; //17.6 из сообщения (первые 4 компонента)
  struct  sTimes          msgTimes;
  
  bool      newInfo;        //Должен ли передаваться BPDU
  bool      operEdge;       //Определяется в КА Обнаружение Моста (17.25)
  
	uint16_t  portId;         //
//  uint32_t  PortPathCost;   //
  struct  sPortPriVector  portPriority;
  struct  sTimes          portTimes; //Времена, передаваемые портом в BPDU
  
  bool      proposed;       //17.10
  bool      proposing;      //17.10
  
  bool      rcvdBPDU;       //Принят BPDU
  
  enum eInfoType          rcvdInfo; //Результат rcvInfo() (17.21.8)
  
  bool      rcvdMsg;        //17.23
  bool      rcvdRSTP;       //17.23
  bool      rcvdSTP;        //17.23
  bool      rcvdTc;         //17.21.17, 17.31
  bool      rcvdTcAck;      //17.21.17, 17.31
  bool      rcvdTcn;        //17.21.17, 17.31
  bool      reRoot;         //17.29.2
  bool      reselect;       //17.28
  
  enum ePortRstpRole      role;         //Назначенная роль (17.7)

  bool      selected;       //17.28, 17.21.16
  enum ePortRstpRole      selectedRole; //Вычисленная роль (17.7, 17.28, ...)
  bool      sendRSTP;       //17.24, 17.26
  bool      sync;           //17.10
  bool      synced;         //17.10

  bool      tcAck;          //
  bool      tcProp;         //
  bool      tick;           //17.22
  
	int32_t   txCount;        //+1 по 17.26 и -1 по 17.22
  int8_t    DecTxCount;     //Делитель на 10
  
	int32_t   RxCount;        //

  bool      updtInfo;       //КА Выбора Роли Порта, для КА Информация Порта
  
	uint16_t  VlanId;         //Идентификатор VLAN для порта

  bool      operPointToPointMAC;
  
  struct sRstpBridge * pRstpBridge;
} SBPort;

//Переключение на массив мостов
/*
  Bridge Max Age—the new value (17.18.4).
  Bridge Hello Time—the new value (17.18.4).
  Bridge Forward Delay—the new value (17.18.4).
  Bridge Priority—the new value of the priority part of the Bridge Identifier (17.18.3).
  forceVersion—the new value of the Force Protocol Version parameter (17.13.4).
  TxHoldCount— the new value of TxHoldCount (17.13.12).
*/
struct sRstpBridgeDesc { //Блок уставок и управления
  uint8_t   MAC[6];
  uint8_t   RstpOn;       //RSTP включен
  uint8_t   Idx;          //Индекс моста (в массиве мостов)
  uint16_t  Priority;
  uint16_t  FirmId;       //Идентификатор фирмы
	uint16_t  MaxAge;       //
	uint16_t  HelloTime;    //
	uint16_t  ForwardDelay; //
	uint16_t  forceVersion; //
	uint16_t  TxHoldCount;  //
  ksz8895fmq_t * SW;
  struct sBPortDesc aBPortDesc[PORT_COUNT];
};

enum eBridgeRole { //Роль моста: корневой / не корневой
  brRoot,
  brNoRoot,
  brCount
};

struct sRstpBridge {
  //Параметры управления (17.13 ) (? нет?)
  
  enum ePortRoleSelection prsState;
  //
  bool    BEGIN;
	struct  sBridgeId       BridgeId;         //Упр: Bridge.Addr.addr, Bridge.Priority
  struct  sPortPriVector  BridgePriority;
  struct  sTimes          BridgeTimes; //Упр

//  uint16_t                RootPortId;
  struct  sPortPriVector  RootPriority;
  struct  sTimes          RootTimes;
  int8_t                  RootPortIdx;
  enum eBridgeRole        BridgeRole;
  
  //If the Transmit Hold Count is modified the value of txCount (17.19.44) for all Ports shall be set to zero.
  uint16_t                TxHoldCount;    //
  //TailTag
  bool                    TailTagOn; //Упр
  
  struct sRstpBridgeDesc  * pRstpBridgeDesc;
  struct sBPort           aPort[PORT_COUNT];
};


extern void CreateTaskRstp();
extern void SetRstpBlock();
extern void rstp_task(void * pvParameters);


extern void ReadSelfMac(uint8_t * iMac, ksz8895fmq_t * pSW);
extern void WriteSelfMac(uint8_t * iMac, ksz8895fmq_t * pSW);

extern void ethstp_input(struct pbuf *p, struct netif *netif);

extern void SetStpOn(struct sRstpBridge * ipRstpBridge);
extern void SetStpOff(struct sRstpBridge * ipRstpBridge);

//
extern struct sRstpBridgeDesc gaRstpBridgeDesc[];
extern struct sRstpBridge gaRstpBridge[];

extern void SetBsp(bool iOn);
extern bool IsTailTag(int8_t iBridgeIdx);
extern void LoadRstpCfg(struct sRstpBridgeDesc * ipRstpBridgeDescSrc);

extern void disablePorts();
extern void enablePorts();

extern void setPortsEnable(bool iEnable);
extern void ProcessSwitchMAC();

extern void enableRstpPorts();

#endif
