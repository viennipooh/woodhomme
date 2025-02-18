//******************************************************************************
// Имя файла    :       snmp_trap.c
// Заголовок    :       
// Автор        :       Сосновских А.А.
// Дата         :       12.09.2019
//
//------------------------------------------------------------------------------
/**
Передача событий посредством SNMP-Trap
**/

#include "snmp_trap.h"
#include "snmp_msg.h"
#include "snmp_def.h"

struct snmp_obj_id snmp_obj_id_PortConnect = {
  .len = 7,
  .id = { 1, 3, 6, 1, 4, 1, SNMP_ENTERPRISE_TRANSMASH_ID, SNMP_ENTERPRISE_TRANSMASH_TRAP_ID }
};

#define VAR_COUNT (2)
uint32_t cValue[VAR_COUNT] = { 2, 1 };
struct snmp_varbind varbind_PortConnect[VAR_COUNT] = {
  {
    .next = &varbind_PortConnect[1],
    .prev = NULL,
    .oid = {
      .len = 9,
      .id = { 1, 3, 6, 1, 4, 1, SNMP_ENTERPRISE_TRANSMASH_ID, SNMP_ENTERPRISE_TRANSMASH_TRAP_ID, 1 }
    },
    .type = SNMP_ASN1_TYPE_INTEGER,
    .value_len = 4,
    .value = &cValue[0]
  },
  {
    .next = NULL,
    .prev = &varbind_PortConnect[0],
    .oid = {
      .len = 9,
      .id = { 1, 3, 6, 1, 4, 1, SNMP_ENTERPRISE_TRANSMASH_ID, SNMP_ENTERPRISE_TRANSMASH_TRAP_ID, 2 }
    },
    .type = SNMP_ASN1_TYPE_INTEGER,
    .value_len = 4,
    .value = &cValue[1]
  }
};

void SendSnmpTrapPort(uint8_t iPort, enum eEventCode iEventCode) {
  uint32_t cPort, cEventCode;
  cPort = iPort;
  cEventCode = iEventCode;
  varbind_PortConnect[0].value = &cPort;
  varbind_PortConnect[1].value = &cEventCode;
//  snmp_send_trap_specific(1, varbind_PortConnect);  
  snmp_send_trap(&snmp_obj_id_PortConnect, SNMP_GENTRAP_ENTERPRISE_SPECIFIC, 1,
                 varbind_PortConnect);
}

