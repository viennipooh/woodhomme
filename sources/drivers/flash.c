/**
  ******************************************************************************
  * @file    flash.c
  * @author  Dmitry Vakhrushev ( vdv.18@mail.ru )
  * @version V1.0
  * @date    09.05.2016
  * @brief   --
  ******************************************************************************
  * @attention
  * 
  ******************************************************************************
  */ 

#include "flash.h"
#include "stdarg.h"

//#define FLASH_KEY1      (0x45670123)
//#define FLASH_KEY2      (0xCDEF89AB)

#define FLASH_COMPLETE          1
#define FLASH_OK                0
#define FLASH_BUSY              -1
#define FLASH_ERROR_WRP         -2
#define FLASH_ERROR_PROGRAM     -3
#define FLASH_ERROR_OPERATION   -4

int flash_get_status(void)
{
  int flashstatus = FLASH_OK;
  
  if((FLASH->SR & FLASH_SR_BSY) == FLASH_SR_BSY) 
  {
    flashstatus = FLASH_BUSY;
  }
  else 
  {  
    if((FLASH->SR & FLASH_SR_WRPERR) != (uint32_t)0x00)
    { 
      flashstatus = FLASH_ERROR_WRP;
    }
    else 
    {
      if((FLASH->SR & (uint32_t)0xEF) != (uint32_t)0x00)
      {
        flashstatus = FLASH_ERROR_PROGRAM; 
      }
      else
      {
        if((FLASH->SR & FLASH_SR_SOP) != (uint32_t)0x00)
        {
          flashstatus = FLASH_ERROR_OPERATION;
        }
        else
        {
          flashstatus = FLASH_COMPLETE;
        }
      }
    }
  }
  /* Return the FLASH Status */
  return flashstatus;
}

int flash_wait_operation(void)
{ 
  volatile uint32_t result = 0;
  while(1)
  {
    result = FLASH->SR;
    result = result & FLASH_SR_BSY;
    if(result == 0)
    {
      break;
    }
  };
  return flash_get_status();
}



int flash_unlock()
{
  if((FLASH->CR & FLASH_CR_LOCK) != RESET)
  {
    /* Authorize the FLASH Registers access */
    FLASH->KEYR = FLASH_KEY1;
    FLASH->KEYR = FLASH_KEY2;
    return FLASH_OK;
  } 
  return -1;
}

int flash_lock()
{
  /* Set the LOCK Bit to lock the FLASH Registers access */
  FLASH->CR |= FLASH_CR_LOCK;
  return 0;
}

int flash_program_buffer(uint32_t volatile *addr, uint32_t *buffer, uint32_t len)
{
  for(;*addr<*addr+len;*addr+=4)
  {
    flash_program_word(*addr,*buffer++);
  }
  return 0;
}

int flash_program_halfword(uint32_t adrr, uint16_t data)
{
  int status = FLASH_COMPLETE;

  /* Wait for last operation to be completed */
  status = flash_wait_operation();
  
  if(status == FLASH_COMPLETE)
  {
    /* if the previous operation is completed, proceed to program the new data */
    FLASH->CR &= FLASH_CR_PSIZE;
    FLASH->CR |= FLASH_CR_PSIZE_0;// 16x
    FLASH->CR |= FLASH_CR_PG;
  
    *(__IO uint16_t*)adrr = data;
        
    /* Wait for last operation to be completed */
    status = flash_wait_operation();

    /* if the program operation is completed, disable the PG Bit */
    FLASH->CR &= (~FLASH_CR_PG);
  } 

  /* Return the Program Status */
  return status;
}

int flash_program_word(uint32_t adrr, uint32_t data)
{
  int status = FLASH_COMPLETE;

  /* Wait for last operation to be completed */
  status = flash_wait_operation();
  
  if(status == FLASH_COMPLETE)
  {
    /* if the previous operation is completed, proceed to program the new data */
    FLASH->CR &= FLASH_CR_PSIZE;
    FLASH->CR |= FLASH_CR_PSIZE_1;// 32x
    FLASH->CR |= FLASH_CR_PG;
  
    *(__IO uint32_t*)adrr = data;
        
    /* Wait for last operation to be completed */
    status = flash_wait_operation();

    /* if the program operation is completed, disable the PG Bit */
    FLASH->CR &= (~FLASH_CR_PG);
  } 

  /* Return the Program Status */
  return status;
}

int flash_program_byte(uint32_t adrr, uint8_t data)
{
  int status = FLASH_COMPLETE;

  /* Wait for last operation to be completed */
  status = flash_wait_operation();
  
  if(status == FLASH_COMPLETE)
  {
    /* if the previous operation is completed, proceed to program the new data */
    FLASH->CR &= FLASH_CR_PSIZE;
    FLASH->CR |= 0;// 8x
    FLASH->CR |= FLASH_CR_PG;
  
    *(__IO uint8_t*)adrr = data;
        
    /* Wait for last operation to be completed */
    status = flash_wait_operation();

    /* if the program operation is completed, disable the PG Bit */
    FLASH->CR &= (~FLASH_CR_PG);
  } 

  /* Return the Program Status */
  return status;
}

int flash_sector_erase( uint8_t sector )
{
  int status = FLASH_COMPLETE;
  
  if(sector>11)
  {
    sector -= 0x0C;
    sector |= 0x10;
  }
  
  if(sector >= 0x0C && sector <= 0x0F)
  {
    return FLASH_ERROR_OPERATION;
  }
  
  if(sector >= 0x1C)
  {
    return FLASH_ERROR_OPERATION;
  }
  
  /* Wait for last operation to be completed */
  status = flash_wait_operation();
  
  if(status == FLASH_COMPLETE)
  { 
    /* if the previous operation is completed, proceed to erase the sector */
    FLASH->CR &=~FLASH_CR_PSIZE;
    FLASH->CR |= 0;// 8x
    FLASH->CR &=~FLASH_CR_SNB;
    FLASH->CR |= FLASH_CR_SER | ((uint32_t)sector<<3);
    FLASH->CR |= FLASH_CR_STRT;
    
    /* Wait for last operation to be completed */
    status = flash_wait_operation();
    
    /* if the erase operation is completed, disable the SER Bit */
    FLASH->CR &= ~FLASH_CR_SER;
    FLASH->CR &= FLASH_CR_SNB;
  }
  /* Return the Erase Status */
  return status;
}

int flash_init( void )
{
  return flash_unlock();
}

int flash_deinit( void )
{
  return flash_lock();
}