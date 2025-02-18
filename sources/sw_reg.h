//******************************************************************************
// Имя файла    :       sw_reg.h
// Заголовок    :       
// Автор        :       Сосновских А.А.
// Дата         :       07.06.2018
//
//------------------------------------------------------------------------------
/**
Работа с регистрами свитча

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

#define INDIRECT_OPERATION_KIND  (0x6e) //Регистр типа непрямого доступа
#define INDIRECT_OPERATION_ADDR  (0x6f) //Регистр адреса непрямого доступа

#define INDIRECT_OPERATION_REGS_DMAC  (0x70) //Блок регистров непрямого доступа DMAC
#define INDIRECT_OPERATION_REGS_SMAC  (0x71) //Блок регистров непрямого доступа SMAC
#define INDIRECT_OPERATION_REGS_VLAN  (0x72) //Блок регистров непрямого доступа VLAN

#define DMAC_TABLE_READ  (0x18)  //Чтение DMAC-таблицы

#define SMAC_TABLE_READ  (0x10)  //Чтение SMAC-таблицы
#define SMAC_TABLE_WRITE (0x00)  //Запись SMAC-таблицы

//==== MAC-таблицы

//=== Динамическая MAC-таблица
//Дескриптор элемента Динамической MAC-таблицы
struct s_DMACItem {
  uint8_t   MAC[6];         //MAC-адрес
  uint8_t   FID;            //FID
  uint8_t   SRCPort  :3;    //Входной порт (3 бита, начиная с 0 (0 = порт 1))
  uint8_t   TimeStamp:2;    //Внутреннее время жизни (2 бита)
  uint16_t  ValidNumber;    //Количество валидных элементов в таблице
  uint8_t   MACEmpty;       //1 = нет валидных элементов в таблице
};

//Дескриптор элемента Статической MAC-таблицы
struct s_SMACItem {
  uint8_t   MAC[6];         //MAC-адрес
  uint8_t   FwdPorts:5;     //Порты, куда передаются кадры
  uint8_t   Valid   :1;     //Элемент валидный
  uint8_t   Override:1;     //Перекрытие (переназначение) уставок spanning tree
  uint8_t   UseFID  :1;     //Учитывается FID при поиске в таблице
  uint8_t   FID;            //FID
};


//Блок внутреннего порта
struct sIntPort {
  ksz8895fmq_t *sw;
  int8_t port;
};

extern struct sIntPort * InternalPort(int iExtPort);
extern int8_t ExternalPort(ksz8895fmq_t * iSw, int8_t iIntPort);

extern bool ReadSwRegBit(int8_t iBit, int8_t iReg, ksz8895fmq_t * pSW); //Чтение бита в регистре свитча
extern void SetSwRegBit(bool iEnable, int8_t iBit, int8_t iReg, ksz8895fmq_t * pSW, bool iForced); //Вкл / Выкл бит в регистре свитча
//Чтение поля в регистре свитча
extern uint8_t ReadSwRegField(int8_t iStartBit, int8_t iBitSize, int8_t iReg, ksz8895fmq_t * pSW);
//Записать значение поля в регистр свитча
extern void SetSwRegField(uint8_t iValue, int8_t iStartBit, int8_t iBitSize, int8_t iReg, ksz8895fmq_t * pSW, bool iForced);
 //Вкл / Выкл бит (биты) в регистре порта
//            SetPortKey(ksz8895fmq_t * pSW, bool iEnable, int8_t iPort, int8_t iReg, uint8_t iKey)
extern bool SetPortKey(ksz8895fmq_t * pSW, bool iEnable, int8_t iPort, int8_t iReg, uint8_t iKey);

extern uint8_t ReadPortField(int8_t iStartBit, int8_t iBitSize,
                             int8_t iPort, int8_t iReg, ksz8895fmq_t * pSW);
extern void SetPortField(uint8_t iValue, int8_t iStartBit, int16_t iBitSize,
                         int8_t iPort, int8_t iReg, ksz8895fmq_t * pSW, bool iForced);

//MAC-таблицы
extern struct s_DMACItem ReadDMacEntry(uint16_t iDMacIdx, ksz8895fmq_t * pSW);

extern struct s_SMACItem ReadSMacEntry(uint16_t iSMacIdx, ksz8895fmq_t * pSW);
extern void WriteSMacEntry(struct s_SMACItem * ipSMACItem, uint16_t iSMacIdx, ksz8895fmq_t * pSW);


#endif /* __SW_REG_H__ */
