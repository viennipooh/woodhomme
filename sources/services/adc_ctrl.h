//******************************************************************************
// ��� �����    :       adc_ctrl.h
// ���������    :       
// �����        :       ���������� �.�.
// ����         :       21.06.2019
//
//------------------------------------------------------------------------------
/**
�������� �������� ��� (ADC)

**/

#ifndef __ADC_CTRL_H
#define __ADC_CTRL_H

#include <stdint.h>
#include <stdbool.h>

struct sValLevelDesc;
typedef void (*adc_event_fn)(const struct sValLevelDesc * ipTempLevelDesc, int iCode);

enum eValState {
  vsNone,         //0
  vsFaultLow,     //1
  vsWarningLow,   //2
  vs09V,
  vs10V,
  vs11V,
  vs12V,
  vs13V,
  vs14V,
  vs15V,
  vsNormal,       //10
  vsWarningHigh,  //11
  vsFaultHigh,    //12
  vsCount
};

struct sAdcBuff;
typedef float (*CalcValue_t)(uint16_t iSrcValue);

#define BUFF_SIZE (10)
struct sAdcBuff {
  uint16_t aAdcValue[BUFF_SIZE];
  float    afAdcValue[BUFF_SIZE];
  uint8_t CurrIdx;
  enum eValState State;
  bool Full;
  bool Disabled;
  CalcValue_t CalcValueCB;
};

struct sValLevelDesc {
  enum eValState  ValState;     //���������
  float           InLevelDown;  //���� (������)
  float           InLevelUp;    //���� (�����)
  float           LevelHyst;    //���������� (����� ������ ������ � �����)
};

struct sAdcBlock {
  struct sValLevelDesc  * aValLevelDesc;
  int8_t        LevelDescNumber;
  int8_t        CurrLevelDescIdx;
  adc_event_fn  EventFunc;
};

extern void BuffInit(struct sAdcBuff * ipAdcBuff);
extern void BuffNext(struct sAdcBuff * ipAdcBuff, uint16_t iValue);
extern float fBuffAverage(struct sAdcBuff * ipAdcBuff);
extern uint16_t uBuffAverage(struct sAdcBuff * ipAdcBuff);

#endif //__ADC_CTRL_H
