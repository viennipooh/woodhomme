#include "settings.h"
#include <string.h>

#include "main.h"

uint32_t settings_get_sector_address(uint32_t sector)
{
  switch(sector)
  {
    case FLASH_Sector_0:return 0x08000000;
    case FLASH_Sector_1:return 0x08004000;
    case FLASH_Sector_2:return 0x08008000;
    case FLASH_Sector_3:return 0x0800C000;
    case FLASH_Sector_4:return 0x08010000;
    case FLASH_Sector_5:return 0x08020000;
    case FLASH_Sector_6:return 0x08040000;
    case FLASH_Sector_7:return 0x08060000;
    case FLASH_Sector_8:return 0x08080000;
    case FLASH_Sector_9:return 0x080A0000;
    case FLASH_Sector_10:return 0x080C0000;
    case FLASH_Sector_11:return 0x080E0000;
    case FLASH_Sector_12:return 0x08100000;
    case FLASH_Sector_13:return 0x08104000;
    case FLASH_Sector_14:return 0x08108000;
    case FLASH_Sector_15:return 0x0810C000;
    case FLASH_Sector_16:return 0x08110000;
    case FLASH_Sector_17:return 0x08120000;
    case FLASH_Sector_18:return 0x08140000;
    case FLASH_Sector_19:return 0x08160000;
    case FLASH_Sector_20:return 0x08180000;
    case FLASH_Sector_21:return 0x081A0000;
    case FLASH_Sector_22:return 0x081C0000;
    case FLASH_Sector_23:return 0x081E0000;
  };
  return 0;
}

void settings_ll_load(uint32_t sector,void *buffer, uint32_t len)
{
  void *addr = 0;
  addr = (void*)(settings_get_sector_address(sector));
  memcpy(buffer,addr,len);
}

void settings_ll_save(uint32_t sector,void *buffer, uint32_t len)
{
  uint32_t index = 0;
  uint8_t *addr = 0;
  uint32_t uwAddress = 0;
  /* Unlock the Flash *********************************************************/
  /* Enable the flash control register access */
  FLASH_Unlock();
    
  /* Erase the user Flash area ************************************************/
  /* area defined by FLASH_USER_START_ADDR and FLASH_USER_END_ADDR */

  /* Clear pending flags (if any) */  
  FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | 
                  FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR|FLASH_FLAG_PGSERR); 

  if (FLASH_EraseSector(sector, VoltageRange_3) != FLASH_COMPLETE)
  { 
    /* Error occurred while sector erase. 
       User can add here some code to deal with this error  */
    while (1)
    {
    }
  }
  /* Program the user Flash area word by word ********************************/
  /* area defined by FLASH_USER_START_ADDR and FLASH_USER_END_ADDR */
  addr = (uint8_t*)buffer;
  uwAddress = settings_get_sector_address(sector);
  for(index=0; index<len+4; index+=1){
    if (FLASH_ProgramByte(uwAddress, addr[index]) == FLASH_COMPLETE)
    {
      uwAddress = uwAddress + 1;
    }
    else
    { 
      /* Error occurred while writing data in Flash memory. 
         User can add here some code to deal with this error */
      while (1)
      {
      }
    }
  }
  /* Lock the Flash to disable the flash control register access (recommended
     to protect the FLASH memory against possible unwanted operation) */
  FLASH_Lock(); 
}

