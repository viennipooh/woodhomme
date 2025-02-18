
/**
  ******************************************************************************
  * @file    at45db_disk.c
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
  * <h2><center>&copy; Copyright (C) 2014 by  Dmitry Vakhrushev (vdv.18@mail.ru) </center></h2>
  ******************************************************************************
  */ 

#include "at45db_disk.h"
#include "stdarg.h"
#include "at45db.h"

static int dev = -1;

DSTATUS at45db_disk_initialize()
{
  dev = at45db_init(/*SPI2*/_NULL);
  return 0;
}

extern int at45db_status_read(int dev, uint8_t *res);
DRESULT at45db_disk_status()
{
  uint8_t result[5];
  at45db_status_read(dev,result);
  
//	RES_OK = 0,		/* 0: Successful */
//	RES_ERROR,		/* 1: R/W Error */
//	RES_WRPRT,		/* 2: Write Protected */
//	RES_NOTRDY,		/* 3: Not Ready */
//	RES_PARERR		/* 4: Invalid Parameter */
  if(result[0]&0x80 == 0)
  {
    return RES_NOTRDY;
  }
  else
  {
    return RES_OK;
  }
}


//      case 1:
//        {
//          at45db_page_erase(dev,0x0);
//          at45db_write_wait(dev);
//          state = 0;
//        }
//        break;
extern int at45db_sector_read(int dev, uint8_t* pBuffer, uint32_t SectorAddr, uint16_t NumByteToRead);
DRESULT at45db_disk_read(BYTE *buff, DWORD sector, UINT count)
{
  at45db_sector_read(dev, buff, sector, count*512);
  return RES_OK;
}

extern void at45db_sector_write(int dev, uint8_t* pBuffer, uint32_t SectorAddr, uint16_t NumByteToWrite);
DRESULT at45db_disk_write(const BYTE *buff, DWORD sector, UINT count)
{
 // at45db_write_wait(dev);
  at45db_sector_write(dev, (uint8_t*)buff, sector, count*512);
  return RES_OK;
}

extern int at45db_write_wait(int dev);
DRESULT at45db_disk_sync(/*const BYTE *buff, DWORD sector, UINT count*/)
{
  at45db_write_wait(dev);
  return RES_OK;
}
