/**
  ******************************************************************************
  * @file    flash.h
  * @author  Dmitry Vakhrushev ( vdv.18@mail.ru )
  * @version V1.0
  * @date    09.05.2016
  * @brief   --
  ******************************************************************************
  * @attention
  *
  ******************************************************************************
  */ 
  
#ifndef _FLASH_H__
#define _FLASH_H__

#include "stm32f4xx.h"

#define FLASH_Sector0

int flash_unlock();
int flash_lock();

int flash_init( void );
int flash_program_buffer(uint32_t volatile*addr, uint32_t *buffer, uint32_t len);
int flash_program_word(uint32_t adrr, uint32_t data);
int flash_program_halfword(uint32_t adrr, uint16_t data);
int flash_program_byte(uint32_t adrr, uint8_t data);
int flash_sector_erase( uint8_t sector );
int flash_deinit( void );

#endif /* _FLASH_H__ */


