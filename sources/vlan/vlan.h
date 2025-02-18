//******************************************************************************
// Имя файла    :       vlan.h
// Заголовок    :       
// Автор        :       Сосновских А.А.
// Дата         :       07.06.2018
//
//------------------------------------------------------------------------------
/**
Средства организации виртуальных сетей по IEEE 802.1q

**/

#ifndef __VLAN_H__
#define __VLAN_H__

#include "usart.h"
#include "dma.h"

#include "sw_reg.h"

//#define USE_VLAN  //Поддержка VLAN
//#define USE_DMAC  //Поддержка DMAC таблицы

#define VLAN_TABLE_READ  (0x14)  //Чтение VLAN-таблицы
#define VLAN_TABLE_WRITE (0x04)  //Запись VLAN-таблицы

#define VLAN_TAG_INSERT  (0x04)  //Вставить VLAN-тег на выходе
#define VLAN_TAG_REMOVE  (0x02)  //Удалить VLAN-тег на выходе

//Соответствие портов: МКПШ и внутренних свитчей
#define EXT_PORT_NUM  (7) //Количество внешних портов
#define SW_PORT_NUM   (5) //Количество портов свитча

#include "extio.h"
#if (MKPSH10 != 0)
enum eDevMode { //Режим работы устройства:
  dmSwitch,       //Коммутатор
  dmDoubleRoute,  //Двойная магистраль
  dmCount
};
#endif

#pragma pack(1)

//Структура для чтения - записи блока из 4 vlan-дескрипторов
struct sVlanEntry {
  union {
    uint16_t  u16;
    struct {
      uint16_t  FID     : 7;
      uint16_t  Members : 5;
      uint16_t  Valid   : 1;
    };
  };
};

typedef struct sVlanSet {
  union {
    uint64_t  u64;
    uint8_t   u8[8];
  };

} SVlanSet; //vlan_set_t;

//Дескриптор VLAN на уровне свитча
struct sVlan {
  uint16_t  VlanID;               //ID
  uint16_t  Members;              //Порты в свитче
  struct sVlanEntry VlanEntry;  // sVlanEntry свитча
};

//=== На уровне МКПШ
//Дескриптор VLAN: VlanId, Members, VId
struct sVlanDesc {
  uint16_t  VlanId;
  uint16_t  Members;      //Порты в МКПШ
  struct sVlanEntry VlanEntry[4]; //Для каждого свитча - своя sVlanEntry
};


//=== Настройки VLAN

//Дескриптор конфигурации порта для VLAN
struct sPortVlanDesc {
  uint8_t   VlanTagIns      : 1;  //Вставка в пакет тега VLAN с VID порта на входе
  uint8_t   VlanTagDel      : 1;  //Удаление тега VLAN из пакета на выходе, принятого с тегом
  uint8_t   NoVlanMemberDel : 1;  //Удаление пакетов, если входной порт не член VLAN
  uint8_t   NoInVidDel      : 1;  //Удаление пакетов c VID не равным VID входного порта
  uint8_t   Reserved        : 4;  //Резерв
  uint8_t   VlanMembers     : 7;  //Порты - члены VLAN для этого порта
  uint16_t  DefVlanTag         ;  //Тег VLAN по умолчанию для порта
};

struct sVlanItemDesc {
  uint16_t  VlanId    ;     //Идентификатор VLAN
  uint16_t  VID     :7;     //Индекс VLAN
  uint16_t  Members :7;     //Порты - члены VLAN
  uint16_t  Valid   :1;     //Валидный VLAN
};

#pragma pack()

#define MAX_VLAN_NUM  (10)
//Дескриптор конфигурации VLAN (пока для одного SW)
struct sVlanCfgDesc {
  uint8_t   VlanOn  : 1;  //Вкл. режима VLAN
  uint8_t   UniDis  : 1;  //Unicast-пакеты не могут выходить за пределы VLAN
  uint8_t   NulToVid: 1;  //NULL-VID заменять на VID порта
  struct sPortVlanDesc aPortDesc[7];  //Дескрипторы портов 1 - 7
  uint8_t   VlanNum;      //Количество ненулевых VLAN
  struct sVlanItemDesc aVlanItem[MAX_VLAN_NUM]; //Дескрипторы VLAN
};

extern struct sVlanCfgDesc  gaVlanCfgDesc[];

extern struct sVlanCfgDesc  gTmpVlanCfgDesc;

//Дескриптор конфигурации устройства (пока для МКПШ)

//Тип подключения порта:
enum EPortConnType {
  pctNone,  //Не подключен
  pctCpu,   //Подключен к ЦПУ
  pctExch,  //Подключен к другому свитчу
  pctExt,   //Подключен как выходной порт
  pctCount
};

//Дескриптор порта (только для Ethernet портов)
struct sPortDesc {
  int8_t        SwNum;      //Номер порта в свитче (-1 = не активен)
  int8_t        DevNum;     //Номер порта в устройстве (-1 = не выведен как внешний порт)
  enum EPortConnType ConnType;   //Тип подключения
  int8_t        ConnSwNum;  //Номер свитча, к которому подключен порт
  
};

//Дескриптор свитча
struct sSwitchDesc {
  int8_t  Num;                    //Порядковый номер свитча
  struct  sPortDesc PortDesc[8];  //Блок дескрипторов порта
};

//Дескриптор устройства (связи)
struct sConnDevDesc {
  int8_t  Num;                        //Порядковый номер устройства (?)
  struct  sSwitchDesc SwitchDesc[4];  //Блок дескрипторов свитча
};


#define VLAN_TAB_SIZE (4096)
extern struct sVlanEntry gaVlanEntry[VLAN_TAB_SIZE];
extern void RefreshVlanTablePart();
extern void FillVlanDescFromSwitch(int iSwIdx);
extern bool gIsVlanTabFull;

//extern void SetVlan(bool iEnable); //Вкл / Выкл VLAN

extern void SetSwVlan(bool iEnable, ksz8895fmq_t * pSW); //Вкл / Выкл VLAN свитча

//Прочитать sVlanEntry из свитча pSW по VlanId
extern struct sVlanEntry ReadVlanEntry(uint16_t iVlanId, ksz8895fmq_t * pSW);

//Записать sVlanEntry в свитч pSW по VlanId
extern void WriteVlanEntry(uint16_t iVlanId, struct sVlanEntry iVlanEntry, ksz8895fmq_t * pSW);
//Записать VlanID в порт iPort свитча pSW
extern void SetPortVlanID(ksz8895fmq_t * pSW, int8_t iPort, uint16_t iVlanID);
//Установить порты - члены VLAN для порта
extern void SetPortMembers(ksz8895fmq_t * pSW, int8_t iPortIdx, uint8_t iMembers);

extern struct sVlanDesc aVlanDesc[];
extern struct sConnDevDesc DevDesc;

#if (MKPSH10 != 0)
extern void SetDevMode(enum eDevMode iDevMode);
#endif

#if (IMC_FTX_MC != 0)
extern void SetSubnets(struct sVlanCfgDesc * ipVlanCfgDesc);
#endif

extern void InitVLAN();

#endif