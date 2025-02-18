#ifndef __MY_MIBS_H__
#define __MY_MIBS_H__

//#include "main.h"
#include "statistics.h"
#include "cc.h"

/*Версия аппаратной части*/
#define HW_VER_MAJ 0
#define HW_VER_MIN 0

extern const struct snmp_mib my_mib2;
extern struct statistics_s stat[10];

#endif
