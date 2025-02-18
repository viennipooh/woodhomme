//******************************************************************************
// Имя файла    :       cable.h
// Заголовок    :       
// Автор        :       Сосновских А.А.
// Дата         :       20.03.2019
//
//------------------------------------------------------------------------------
/**
Контроль кабелей портов Ethernet
**/

#ifndef __CABLE_H
#define __CABLE_H

#include <stdint.h>

//Состояние кабеля
enum eCableState {
  csNormal,   //Нормальное
  csOpen,     //Обрыв
  csShort,    //КЗ
  csError,    //Ошибка диагностики
  csNoTested, //Не проверен
  csCount
};

struct sCableTestResult {
  enum eCableState  CableState;
  int16_t           Distance;
  int16_t           SrcValue; //Исходное значение
};

extern struct sCableTestResult gCableTestResult;
extern void CableTest(uint8_t iPort);

#endif //__CABLE_H
