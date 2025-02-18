//******************************************************************************
// Имя файла    :       log_flash.h
// Заголовок    :       
// Автор        :       Сосновских А.А.
// Дата         :       15.01.2019
//
//------------------------------------------------------------------------------
/**
Логирование событий МКПШ. Операции с Flash-памятью

**/
#ifndef __LOG_FLASH_H
#define __LOG_FLASH_H

#define CLEAR_WORD  (0xffffffff)

#include <semphr.h>
#include "extio.h"

//Секторы Flash-памяти
#define CODE_START_SECTOR_WRK   ( 6)  //code start sector for work area
#define CODE_START_SECTOR_REZ   (18)  //code start sector for reserved area

//Как вариант сокращения объёма кода, передаваемого при загрузке через Web
//Нужно настроить также параметры формирования кода в Linker
#if (UTD_M != 0)
  #define CODE_SECTOR_NUMBER      ( 6)  //Число секторов, занимаемых кодом (по 128кБ)
#else
  #define CODE_SECTOR_NUMBER      ( 6)  //Число секторов, занимаемых кодом (по 128кБ)
#endif
#define CODE_SECTOR_SIZE        (0x20000)  //Размер сектора, занимаемого кодом (128кБ)
#define CODE_AREA_SIZE     (CODE_SECTOR_SIZE * CODE_SECTOR_NUMBER)  //Размер области кода

enum eBuffWriteResult { //Результат записи блока данных в буфер кода
  bwrOk = 0,  //Норма
  bwrMem,     //Ошибка памяти
  bwrFin,     //Конец памяти
  bwrCount
};

enum eCodeArea { //Область Flash-памяти:
  caWork,   //Рабочая
  caReserv, //Резервная
  caUpload, //Загрузочная (для загрузки ПО через Web)
  caCount
};

extern xSemaphoreHandle Flash_mutex;

extern void Flash_Program_Init();
extern void Flash_Program_Word(uint32_t iAddr, uint32_t iWord);
extern void Flash_Program_Event(uint32_t iAddr, struct sEvent * ipEvent);
extern void Flash_Erase_Sector(uint32_t iSector);

extern void EraseSectorArea(uint8_t iStartSectorIdx, uint8_t iSectorNum);
extern uint32_t Sector_Address(uint32_t iSectorIdx);
extern bool IsSectorEmptyByIdx(uint16_t iSectIdx);
extern void EraseCodeArea(enum eCodeArea iCodeArea);

#endif //__LOG_FLASH_H
