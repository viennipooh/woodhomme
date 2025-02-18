/**
  ******************************************************************************
  * @file    db.c
  * @author  Dmitry Vakhrushev ( vdv.18@mail.ru )
  * @version V1.0
  * @date    03.05.2016
  * @brief   --
  ******************************************************************************
  * @attention
  * 
  ******************************************************************************
  */ 

#include "db.h"
#include "stdarg.h"
#include "bkpsram.h"
#include "stm32f4xx.h"
#include "string.h"

void db_init( void )
{
  bkpsram_init();
}

void db_deinit( void )
{
  
}