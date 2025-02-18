/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2014        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

#include "diskio.h"		/* FatFs lower layer API */

#include "extio.h"
#include "at45db_disk.h"

/* Definitions of physical drive number for each drive */
#define AT45DB             1
#define RAM                0

#define SDRAM_BLOCK_SIZE        0x200
#define SDRAM_SECTOR_SIZE       0x200
#define SDRAM_SECTOR_COUNT      0x1000

#define AT45DB_BLOCK_SIZE       0x200
#define AT45DB_SECTOR_SIZE      0x200
#define AT45DB_SECTOR_COUNT     0x1000


/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
	BYTE pdrv		/* Physical drive nmuber to identify the drive */
)
{
  DSTATUS stat;
//  int result;

  switch (pdrv) {
    case AT45DB :
      {
        stat = at45db_disk_status();
      }
      return stat;
    case RAM :
      {
        stat = 0;
      }
      return stat;
  }
  return STA_NOINIT;
}



/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
	BYTE pdrv				/* Physical drive nmuber to identify the drive */
)
{
//  DSTATUS stat;
  int result;

  switch (pdrv) {
    case AT45DB :
      {
        result = at45db_disk_initialize();
      }
      return result;
    case RAM :
      {
        result = 0;
      }
      return result;
  }
  return STA_NOINIT;
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/
#include "string.h"
DRESULT disk_read (
	BYTE pdrv,		/* Physical drive nmuber to identify the drive */
	BYTE *buff,		/* Data buffer to store read data */
	DWORD sector,	/* Sector address in LBA */
	UINT count		/* Number of sectors to read */
)
{
	DRESULT res;
//	int result;

	switch (pdrv) {
	case AT45DB :
		res = at45db_disk_read(buff, sector, count);
		return res;
	case RAM :
          {
                memcpy(buff,(void *)(SDRAM_BASE+sector*SDRAM_SECTOR_SIZE),count*SDRAM_SECTOR_SIZE);
		return RES_OK;
          }
	}

	return RES_PARERR;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if _USE_WRITE
DRESULT disk_write (
	BYTE pdrv,			/* Physical drive nmuber to identify the drive */
	const BYTE *buff,	/* Data to be written */
	DWORD sector,		/* Sector address in LBA */
	UINT count			/* Number of sectors to write */
)
{
	DRESULT res;
//	int result;

	switch (pdrv) {
	case AT45DB :
		res = at45db_disk_write(buff, sector, count);
		return res;
	case RAM :
          {
                memcpy((void *)(SDRAM_BASE+sector*SDRAM_SECTOR_SIZE),buff,count*SDRAM_SECTOR_SIZE);
		return RES_OK;
          }
	}

	return RES_PARERR;
}
#endif


/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

#if _USE_IOCTL
extern DRESULT at45db_disk_sync(/*const BYTE *buff, DWORD sector, UINT count*/);
DRESULT disk_ioctl (
	BYTE pdrv,		/* Physical drive nmuber (0..) */
	BYTE cmd,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)
{
  DRESULT res;
//  int result;

  switch (pdrv) {
    case AT45DB :
      {
        switch(cmd)
        {
          case CTRL_SYNC:
            {
              res = at45db_disk_sync();
            }
            break;
          case GET_SECTOR_COUNT:
            {
              DWORD *n_vol = (DWORD*)buff;
              *n_vol = AT45DB_SECTOR_COUNT;
              res = RES_OK;
            }
            break;
          case GET_SECTOR_SIZE:
            {
              DWORD *ss = (DWORD*)buff;
              *ss = AT45DB_SECTOR_SIZE;
              res = RES_OK;
            }
            break;
          case GET_BLOCK_SIZE:
            {
              DWORD *blk = (DWORD*)buff;
              *blk = AT45DB_BLOCK_SIZE;
              res = RES_OK;
            }
            break;
          case CTRL_TRIM:
            {
              res = RES_OK;
            }
            break;
        }
      }
      return res;
    case RAM :
      {
        switch(cmd)
        {
          case CTRL_SYNC:
            {
              res = RES_OK;
            }
            break;
          case GET_SECTOR_COUNT:
            {
              DWORD *n_vol = (DWORD*)buff;
              *n_vol = SDRAM_SECTOR_COUNT;
              res = RES_OK;
            }
            break;
          case GET_SECTOR_SIZE:
            {
              DWORD *ss = (DWORD*)buff;
              *ss = SDRAM_SECTOR_SIZE;
              res = RES_OK;
            }
            break;
          case GET_BLOCK_SIZE:
            {
              DWORD *blk = (DWORD*)buff;
              *blk = SDRAM_BLOCK_SIZE;
              res = RES_OK;
            }
            break;
          case CTRL_TRIM:
            {
              res = RES_OK;
            }
            break;
        }
      }
      return res;
  }
  return RES_PARERR;
}
#endif
