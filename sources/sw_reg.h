//******************************************************************************
// ��� �����    :       sw_reg.h
// ���������    :       
// �����        :       ���������� �.�.
// ����         :       07.06.2018
//
//------------------------------------------------------------------------------
/**
������ � ���������� ������

**/

#ifndef __SW_REG_H__
#define __SW_REG_H__

#include <stdbool.h>
#include <stdint.h>

#include "ksz8895fmq.h"

#include <stdint.h>
#include <string.h>
#include "ip.h"

#if (!NO_SYS)
  #include "lwip/sockets.h"
  #include "FreeRTOS.h"
  #include "task.h"
#else
  #include "lwipopts.h"
#endif

#define INDIRECT_OPERATION_KIND  (0x6e) //������� ���� ��������� �������
#define INDIRECT_OPERATION_ADDR  (0x6f) //������� ������ ��������� �������

#define INDIRECT_OPERATION_REGS_DMAC  (0x70) //���� ��������� ��������� ������� DMAC
#define INDIRECT_OPERATION_REGS_SMAC  (0x71) //���� ��������� ��������� ������� SMAC
#define INDIRECT_OPERATION_REGS_VLAN  (0x72) //���� ��������� ��������� ������� VLAN

#define DMAC_TABLE_READ  (0x18)  //������ DMAC-�������

#define SMAC_TABLE_READ  (0x10)  //������ SMAC-�������
#define SMAC_TABLE_WRITE (0x00)  //������ SMAC-�������

//==== MAC-�������

//=== ������������ MAC-�������
//���������� �������� ������������ MAC-�������
struct s_DMACItem {
  uint8_t   MAC[6];         //MAC-�����
  uint8_t   FID;            //FID
  uint8_t   SRCPort  :3;    //������� ���� (3 ����, ������� � 0 (0 = ���� 1))
  uint8_t   TimeStamp:2;    //���������� ����� ����� (2 ����)
  uint16_t  ValidNumber;    //���������� �������� ��������� � �������
  uint8_t   MACEmpty;       //1 = ��� �������� ��������� � �������
};

//���������� �������� ����������� MAC-�������
struct s_SMACItem {
  uint8_t   MAC[6];         //MAC-�����
  uint8_t   FwdPorts:5;     //�����, ���� ���������� �����
  uint8_t   Valid   :1;     //������� ��������
  uint8_t   Override:1;     //���������� (��������������) ������� spanning tree
  uint8_t   UseFID  :1;     //����������� FID ��� ������ � �������
  uint8_t   FID;            //FID
};


//���� ����������� �����
struct sIntPort {
  ksz8895fmq_t *sw;
  int8_t port;
};

extern struct sIntPort * InternalPort(int iExtPort);
extern int8_t ExternalPort(ksz8895fmq_t * iSw, int8_t iIntPort);

extern bool ReadSwRegBit(int8_t iBit, int8_t iReg, ksz8895fmq_t * pSW); //������ ���� � �������� ������
extern void SetSwRegBit(bool iEnable, int8_t iBit, int8_t iReg, ksz8895fmq_t * pSW, bool iForced); //��� / ���� ��� � �������� ������
//������ ���� � �������� ������
extern uint8_t ReadSwRegField(int8_t iStartBit, int8_t iBitSize, int8_t iReg, ksz8895fmq_t * pSW);
//�������� �������� ���� � ������� ������
extern void SetSwRegField(uint8_t iValue, int8_t iStartBit, int8_t iBitSize, int8_t iReg, ksz8895fmq_t * pSW, bool iForced);
 //��� / ���� ��� (����) � �������� �����
//            SetPortKey(ksz8895fmq_t * pSW, bool iEnable, int8_t iPort, int8_t iReg, uint8_t iKey)
extern bool SetPortKey(ksz8895fmq_t * pSW, bool iEnable, int8_t iPort, int8_t iReg, uint8_t iKey);

extern uint8_t ReadPortField(int8_t iStartBit, int8_t iBitSize,
                             int8_t iPort, int8_t iReg, ksz8895fmq_t * pSW);
extern void SetPortField(uint8_t iValue, int8_t iStartBit, int16_t iBitSize,
                         int8_t iPort, int8_t iReg, ksz8895fmq_t * pSW, bool iForced);

//MAC-�������
extern struct s_DMACItem ReadDMacEntry(uint16_t iDMacIdx, ksz8895fmq_t * pSW);

extern struct s_SMACItem ReadSMacEntry(uint16_t iSMacIdx, ksz8895fmq_t * pSW);
extern void WriteSMacEntry(struct s_SMACItem * ipSMACItem, uint16_t iSMacIdx, ksz8895fmq_t * pSW);


#endif /* __SW_REG_H__ */
