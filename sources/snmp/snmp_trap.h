//******************************************************************************
// Имя файла    :       snmp_trap.h
// Заголовок    :       
// Автор        :       Сосновских А.А.
// Дата         :       12.09.2019
//
//------------------------------------------------------------------------------
/**
Передача событий посредством SNMP-Trap
**/

#ifndef __SNMP_TRAP_H
#define __SNMP_TRAP_H

#include <stdint.h>

#include "log.h"
extern void SendSnmpTrap();
extern void SendSnmpTrapPort(uint8_t iPort, enum eEventCode iEventCode);


#endif //__SNMP_TRAP_H

