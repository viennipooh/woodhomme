//******************************************************************************
// ��� �����    :       snmp_def.h
// ���������    :       
// �����        :       ���������� �.�.
// ����         :       18.03.2020
//
//------------------------------------------------------------------------------
/**
  ����������� ��� SNMP
**/

#ifndef __SNMP_DEF_H
#define __SNMP_DEF_H

#define SNMP_ENTERPRISE_TRANSMASH_ID 50675
#define SNMP_ENTERPRISE_TRANSMASH_OID { 1, 3, 6, 1, 4, 1, SNMP_ENTERPRISE_TRANSMASH_ID }
#define SNMP_ENTERPRISE_OID_LEN 7
//ID ���������� ��� �������� Trap-������
#define SNMP_ENTERPRISE_TRANSMASH_TRAP_ID 20

#define _USE_SNMP_CLIENT  0 //����������� ������� SNMP


#endif //__SNMP_DEF_H

