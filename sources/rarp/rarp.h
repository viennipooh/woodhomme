//******************************************************************************
// ��� �����    :       rarp.h
// ���������    :       
// �����        :       ���������� �.�.
// ����         :       03.10.2019
//
//------------------------------------------------------------------------------
/**
���� ������ � ��������� RARP
= ������

**/

#ifndef __RARP_H
#define __RARP_H

#include <stdint.h>
#include <stdbool.h>
#include "dmac_mib2.h"

#define LAN_TOPOLOGY_FINDING  //�������� ���������� ��������� ����

#define LARGE_DMAC_ITEM //������ �������� ������������ ������� DMAC = 7����

enum rarp_req_code {
  RARP_REQ_DMAC = 0xfe,
  RARP_REQ_FIND = 0xff
};

#pragma pack(1)
//�������� ��������� �����
typedef struct sPortEnv {
  uint8_t   MAC[6]; //MAC
  uint32_t  IP;     //IP
  uint8_t   Port  ; //����� �����
  uint8_t   IsEdge  :1; //��������� ����
  uint8_t   IsSingle:1; //������������ �� ���� ����� (== ��������� � ����)
  uint8_t   Reserved:6; //������
} SPortEnv;


//�������� ��������� ����������
typedef struct sDevEnv {
  uint16_t  Size;               //������ ���������
  uint8_t   PortNum;            //���������� ������
  struct sPortEnv PortEnv[10];  //��������� ������
} SDevEnv;


#pragma pack()

extern void rarp_process(uint8_t iOper, uint8_t ** ippData, uint16_t * ipSize);
extern bool IsSnmpClientFound(uint32_t iIP);

#endif //__RARP_H

