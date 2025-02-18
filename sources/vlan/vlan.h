//******************************************************************************
// ��� �����    :       vlan.h
// ���������    :       
// �����        :       ���������� �.�.
// ����         :       07.06.2018
//
//------------------------------------------------------------------------------
/**
�������� ����������� ����������� ����� �� IEEE 802.1q

**/

#ifndef __VLAN_H__
#define __VLAN_H__

#include "usart.h"
#include "dma.h"

#include "sw_reg.h"

//#define USE_VLAN  //��������� VLAN
//#define USE_DMAC  //��������� DMAC �������

#define VLAN_TABLE_READ  (0x14)  //������ VLAN-�������
#define VLAN_TABLE_WRITE (0x04)  //������ VLAN-�������

#define VLAN_TAG_INSERT  (0x04)  //�������� VLAN-��� �� ������
#define VLAN_TAG_REMOVE  (0x02)  //������� VLAN-��� �� ������

//������������ ������: ���� � ���������� �������
#define EXT_PORT_NUM  (7) //���������� ������� ������
#define SW_PORT_NUM   (5) //���������� ������ ������

#include "extio.h"
#if (MKPSH10 != 0)
enum eDevMode { //����� ������ ����������:
  dmSwitch,       //����������
  dmDoubleRoute,  //������� ����������
  dmCount
};
#endif

#pragma pack(1)

//��������� ��� ������ - ������ ����� �� 4 vlan-������������
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

//���������� VLAN �� ������ ������
struct sVlan {
  uint16_t  VlanID;               //ID
  uint16_t  Members;              //����� � ������
  struct sVlanEntry VlanEntry;  // sVlanEntry ������
};

//=== �� ������ ����
//���������� VLAN: VlanId, Members, VId
struct sVlanDesc {
  uint16_t  VlanId;
  uint16_t  Members;      //����� � ����
  struct sVlanEntry VlanEntry[4]; //��� ������� ������ - ���� sVlanEntry
};


//=== ��������� VLAN

//���������� ������������ ����� ��� VLAN
struct sPortVlanDesc {
  uint8_t   VlanTagIns      : 1;  //������� � ����� ���� VLAN � VID ����� �� �����
  uint8_t   VlanTagDel      : 1;  //�������� ���� VLAN �� ������ �� ������, ��������� � �����
  uint8_t   NoVlanMemberDel : 1;  //�������� �������, ���� ������� ���� �� ���� VLAN
  uint8_t   NoInVidDel      : 1;  //�������� ������� c VID �� ������ VID �������� �����
  uint8_t   Reserved        : 4;  //������
  uint8_t   VlanMembers     : 7;  //����� - ����� VLAN ��� ����� �����
  uint16_t  DefVlanTag         ;  //��� VLAN �� ��������� ��� �����
};

struct sVlanItemDesc {
  uint16_t  VlanId    ;     //������������� VLAN
  uint16_t  VID     :7;     //������ VLAN
  uint16_t  Members :7;     //����� - ����� VLAN
  uint16_t  Valid   :1;     //�������� VLAN
};

#pragma pack()

#define MAX_VLAN_NUM  (10)
//���������� ������������ VLAN (���� ��� ������ SW)
struct sVlanCfgDesc {
  uint8_t   VlanOn  : 1;  //���. ������ VLAN
  uint8_t   UniDis  : 1;  //Unicast-������ �� ����� �������� �� ������� VLAN
  uint8_t   NulToVid: 1;  //NULL-VID �������� �� VID �����
  struct sPortVlanDesc aPortDesc[7];  //����������� ������ 1 - 7
  uint8_t   VlanNum;      //���������� ��������� VLAN
  struct sVlanItemDesc aVlanItem[MAX_VLAN_NUM]; //����������� VLAN
};

extern struct sVlanCfgDesc  gaVlanCfgDesc[];

extern struct sVlanCfgDesc  gTmpVlanCfgDesc;

//���������� ������������ ���������� (���� ��� ����)

//��� ����������� �����:
enum EPortConnType {
  pctNone,  //�� ���������
  pctCpu,   //��������� � ���
  pctExch,  //��������� � ������� ������
  pctExt,   //��������� ��� �������� ����
  pctCount
};

//���������� ����� (������ ��� Ethernet ������)
struct sPortDesc {
  int8_t        SwNum;      //����� ����� � ������ (-1 = �� �������)
  int8_t        DevNum;     //����� ����� � ���������� (-1 = �� ������� ��� ������� ����)
  enum EPortConnType ConnType;   //��� �����������
  int8_t        ConnSwNum;  //����� ������, � �������� ��������� ����
  
};

//���������� ������
struct sSwitchDesc {
  int8_t  Num;                    //���������� ����� ������
  struct  sPortDesc PortDesc[8];  //���� ������������ �����
};

//���������� ���������� (�����)
struct sConnDevDesc {
  int8_t  Num;                        //���������� ����� ���������� (?)
  struct  sSwitchDesc SwitchDesc[4];  //���� ������������ ������
};


#define VLAN_TAB_SIZE (4096)
extern struct sVlanEntry gaVlanEntry[VLAN_TAB_SIZE];
extern void RefreshVlanTablePart();
extern void FillVlanDescFromSwitch(int iSwIdx);
extern bool gIsVlanTabFull;

//extern void SetVlan(bool iEnable); //��� / ���� VLAN

extern void SetSwVlan(bool iEnable, ksz8895fmq_t * pSW); //��� / ���� VLAN ������

//��������� sVlanEntry �� ������ pSW �� VlanId
extern struct sVlanEntry ReadVlanEntry(uint16_t iVlanId, ksz8895fmq_t * pSW);

//�������� sVlanEntry � ����� pSW �� VlanId
extern void WriteVlanEntry(uint16_t iVlanId, struct sVlanEntry iVlanEntry, ksz8895fmq_t * pSW);
//�������� VlanID � ���� iPort ������ pSW
extern void SetPortVlanID(ksz8895fmq_t * pSW, int8_t iPort, uint16_t iVlanID);
//���������� ����� - ����� VLAN ��� �����
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