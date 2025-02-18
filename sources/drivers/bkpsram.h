/**
  ******************************************************************************
  * @file    bkpsram.h
  * @author  Dmitry Vakhrushev ( vdv.18@mail.ru )
  * @version V1.0
  * @date    02.05.2016
  * @brief   --
  ******************************************************************************
  * @attention
  *
  ******************************************************************************
  */ 
  
#ifndef _BKPSRAM_H__
#define _BKPSRAM_H__

#include "stdint.h"

void bkpsram_init( void );
void bkpsram_reset( void );

void bkpsram_lock();
void bkpsram_unlock();

int8_t bkprtc_write( uint32_t *data, uint16_t bytes, uint16_t offset );
int8_t bkprtc_read( uint32_t *data, uint16_t bytes, uint16_t offset );

int8_t bkpsram_write( uint8_t *data, uint16_t bytes, uint16_t offset );
int8_t bkpsram_read( uint8_t *data, uint16_t bytes, uint16_t offset );

#endif /* _BKPSRAM_H__ */


