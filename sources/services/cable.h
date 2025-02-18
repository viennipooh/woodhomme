//******************************************************************************
// ��� �����    :       cable.h
// ���������    :       
// �����        :       ���������� �.�.
// ����         :       20.03.2019
//
//------------------------------------------------------------------------------
/**
�������� ������� ������ Ethernet
**/

#ifndef __CABLE_H
#define __CABLE_H

#include <stdint.h>

//��������� ������
enum eCableState {
  csNormal,   //����������
  csOpen,     //�����
  csShort,    //��
  csError,    //������ �����������
  csNoTested, //�� ��������
  csCount
};

struct sCableTestResult {
  enum eCableState  CableState;
  int16_t           Distance;
  int16_t           SrcValue; //�������� ��������
};

extern struct sCableTestResult gCableTestResult;
extern void CableTest(uint8_t iPort);

#endif //__CABLE_H
