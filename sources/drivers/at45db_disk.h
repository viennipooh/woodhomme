/**
  ******************************************************************************
  * @file    at45db_disk.h
  * @author  Dmitry Vakhrushev ( vdv.18@mail.ru )
  * @version V1.0
  * @date    07.01.2016
  * @brief   --
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; Copyright (C) 2015 by vdv.18@mail.ru ( V1.0 ) </center></h2>
  ******************************************************************************
  */ 
  
#ifndef _AT45DB_DISK_H__
#define _AT45DB_DISK_H__

#include "diskio.h"
#include "at45db.h"


DSTATUS at45db_disk_initialize();
DRESULT at45db_disk_status();

DRESULT at45db_disk_read(BYTE *buff, DWORD sector, UINT count);
DRESULT at45db_disk_write(const BYTE *buff, DWORD sector, UINT count);

#endif /* _AT45DB_DISK_H__ */


