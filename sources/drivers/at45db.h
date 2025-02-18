/**
  ******************************************************************************
  * @file    at45db.h
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
  
#ifndef _AT45DB_H__
#define _AT45DB_H__

#include <stdint.h>
#include "spi.h"

#define AT45_ATMEL_ADESTRO      0x1F           

#define AT45_FAMILY(n)          ((n&0xE0)>>5)
#define AT45_DENSITY(n)         (n&0x1F)

#define AT45DB161D              0x26
#define AT45DB161D_FAMILY       0x01
#define AT45DB161D_DENSITY      0x06


typedef enum at45db_devices_e {
  AT45DB011 = 0x22,
  AT45DB021 = 0x23,
  AT45DB041 = 0x24,
  AT45DB081 = 0x25,
  AT45DB161 = 0x26,
  AT45DB321 = 0x27,
  AT45DB641 = 0x28,
}at45db_device_t;

typedef enum at45db_cmd_e {
  AT45DB_CMD_RESET,
  AT45DB_CMD_STATUS_READ,
  AT45DB_CMD_MDID_READ,
  AT45DB_CMD_PAGE_ERASE,
  AT45DB_CMD_BLOCK_ERASE,
  AT45DB_CMD_SECTOR_ERASE,
  AT45DB_CMD_CHIP_ERASE,
}at45db_cmd_t;

typedef enum at45db_command_e {
  AT45DB_COMMAND_RESET,
  AT45DB_COMMAND_STATUS_READ,
  AT45DB_COMMAND_MDID_READ,
  AT45DB_COMMAND_PAGE_ERASE,
  AT45DB_COMMAND_BLOCK_ERASE,
  AT45DB_COMMAND_SECTOR_ERASE,
  AT45DB_COMMAND_CHIP_ERASE,
}at45db_command_t;


typedef struct at45db_driver_s {
  uint8_t               mid;
  at45db_device_t       id;
  SPI_TypeDef           *pspi;
  uint32_t              size;  /* общий размер в байтах */
  uint32_t              psize; /* размер страницы */
  uint32_t              pages; /* количество страниц */
}at45db_driver_t;


int at45db_init( SPI_TypeDef *pspi ); // return device number

int at45db_cmd( int dev, at45db_cmd_t, void * );

int at45db_deinit( int dev );

int at45db_sector_read(int dev, uint8_t* pBuffer, uint32_t SectorAddr, uint16_t NumByteToRead);
void at45db_sector_write(int dev, uint8_t* pBuffer, uint32_t SectorAddr, uint16_t NumByteToWrite);

#endif /* _AT45DB_H__ */


