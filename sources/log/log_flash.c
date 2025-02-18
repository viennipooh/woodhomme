//******************************************************************************
// Имя файла    :       log_flash.c
// Заголовок    :       
// Автор        :       Сосновских А.А.
// Дата         :       15.01.2019
//
//------------------------------------------------------------------------------
/**
Логирование событий МКПШ. Операции с Flash-памятью

**/

/* Scheduler includes */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "stdbool.h"

#include "log.h"
#include "log_flash.h"

xSemaphoreHandle Flash_mutex;

void Flash_Program_Init() {
  Flash_mutex = xSemaphoreCreateMutex();
}

void Flash_Program_Word(uint32_t iAddr, uint32_t iWord) {
  /* Unlock the Flash */
  /* Enable the flash control register access */
  FLASH_Unlock();
    
  /* Clear pending flags (if any) */  
  FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | 
                  FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR|FLASH_FLAG_PGSERR); 

  if (FLASH_ProgramWord(iAddr, iWord) == FLASH_COMPLETE) {
    
  }
  
  FLASH_Lock(); 
}

void Flash_Program_Event(uint32_t iAddr, struct sEvent * ipEvent) {
  uint32_t uwAddress = 0;
  /* Unlock the Flash **/
  /* Enable the flash control register access */
  FLASH_Unlock();
    
  /* Program the user Flash area word by word */
  /* area defined by FLASH_USER_START_ADDR and FLASH_USER_END_ADDR */
  FLASH_Status status = FLASH_WaitForLastOperation();

  uwAddress = iAddr;
  uint8_t cTry = 0;
  for (int i = 0; i < WORDS_IN_EVENT; ++i) {
    /* Clear pending flags (if any) */  
    FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | 
                    FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR); 

    status = FLASH_ProgramWord(uwAddress, ipEvent->aEvent[i]);
    if (status == FLASH_COMPLETE) {
      uwAddress = uwAddress + 4;
    } else { 
      /* Error occurred while writing data in Flash memory. 
         User can add here some code to deal with this error */
      vTaskDelay(10);
      if (cTry < 3) {
        --i;
        ++cTry;
      } else {
        break;
      }
    }
  }
  /* Lock the Flash to disable the flash control register access (recommended
     to protect the FLASH memory against possible unwanted operation) */
  FLASH_Lock(); 
}

void Flash_Erase_Sector(uint32_t iSector) {
  /* Unlock the Flash *********************************************************/
  /* Enable the flash control register access */
  FLASH_Unlock();
    
  /* Clear pending flags (if any) */  
  FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | 
                  FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR|FLASH_FLAG_PGSERR); 

  if (FLASH_EraseSector(iSector, VoltageRange_3) != FLASH_COMPLETE)
  { 
    /* Error occurred while sector erase. 
       User can add here some code to deal with this error  */
  }
  /* Lock the Flash to disable the flash control register access (recommended
     to protect the FLASH memory against possible unwanted operation) */
  FLASH_Lock(); 
}

uint32_t SectorByIdx(uint8_t iSectorIdx) {
  switch(iSectorIdx) {
    case  0:  return FLASH_Sector_0;
    case  1:  return FLASH_Sector_1;
    case  2:  return FLASH_Sector_2;
    case  3:  return FLASH_Sector_3;
    case  4:  return FLASH_Sector_4;
    case  5:  return FLASH_Sector_5;
    case  6:  return FLASH_Sector_6;
    case  7:  return FLASH_Sector_7;
    case  8:  return FLASH_Sector_8;
    case  9:  return FLASH_Sector_9;
    case 10:  return FLASH_Sector_10;
    case 11:  return FLASH_Sector_11;
    case 12:  return FLASH_Sector_12;
    case 13:  return FLASH_Sector_13;
    case 14:  return FLASH_Sector_14;
    case 15:  return FLASH_Sector_15;
    case 16:  return FLASH_Sector_16;
    case 17:  return FLASH_Sector_17;
    case 18:  return FLASH_Sector_18;
    case 19:  return FLASH_Sector_19;
    case 20:  return FLASH_Sector_20;
    case 21:  return FLASH_Sector_21;
    case 22:  return FLASH_Sector_22;
    case 23:  return FLASH_Sector_23;
  };
  return 0;
}

void EraseSectorArea(uint8_t iStartSectorIdx, uint8_t iSectorNum) {
  FLASH_Unlock();
  for (int i = 0; i < iSectorNum; ++i) {
    Flash_Erase_Sector(SectorByIdx(iStartSectorIdx + i));
  }
  FLASH_Lock();
}

uint32_t Sector_Address(uint32_t iSectorIdx) {
  switch(iSectorIdx) {
    case 0:  return 0x08000000;
    case 1:  return 0x08004000;
    case 2:  return 0x08008000;
    case 3:  return 0x0800C000;
    case 4:  return 0x08010000;
    case 5:  return 0x08020000;
    case 6:  return 0x08040000;
    case 7:  return 0x08060000;
    case 8:  return 0x08080000;
    case 9:  return 0x080A0000;
    case 10: return 0x080C0000;
    case 11: return 0x080E0000;
    case 12: return 0x08100000;
    case 13: return 0x08104000;
    case 14: return 0x08108000;
    case 15: return 0x0810C000;
    case 16: return 0x08110000;
    case 17: return 0x08120000;
    case 18: return 0x08140000;
    case 19: return 0x08160000;
    case 20: return 0x08180000;
    case 21: return 0x081A0000;
    case 22: return 0x081C0000;
    case 23: return 0x081E0000;
  };
  return 0;
}

bool IsSectorEmptyByIdx(uint16_t iSectIdx) {
  if (iSectIdx <= 23) {
    uint32_t cStart = Sector_Address(iSectIdx);
    uint32_t * cpStart = (uint32_t *)cStart;
    bool cIsEmpty = (*cpStart == CLEAR_WORD);
    return cIsEmpty;
  }
  return false;
}

//Стереть во Flash область кода (рабочую / резервную)
void EraseCodeArea(enum eCodeArea iCodeArea) {
  uint8_t cStartSectorIdx;
  switch (iCodeArea) {
  case caWork:    cStartSectorIdx = CODE_START_SECTOR_WRK;   break;
  case caReserv:  cStartSectorIdx = CODE_START_SECTOR_REZ;   break;
  default: return;
  }
  EraseSectorArea(cStartSectorIdx, CODE_SECTOR_NUMBER);
}

