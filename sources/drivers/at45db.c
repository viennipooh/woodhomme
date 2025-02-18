/**
  ******************************************************************************
  * @file    at45db.c
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

#include "at45db.h"
#include "dma.h"
#include "main.h"
#include <stdarg.h>
#include <string.h>
#include "lwipopts.h"
#include "extio.h"

#if (MKPSH10 != 0)
static SPI_TypeDef *pspi = SPI2;

#define SELECT()        {GPIO_OUT(GPIOB,12,0);delay(10);}
#define DESELECT()      {delay(10);GPIO_OUT(GPIOB,12,1);}
#endif

#if (IMC_FTX_MC != 0)
static SPI_TypeDef *pspi = SPI1;

#define SELECT()        {GPIO_OUT(GPIOA,4,0);delay(10);}
#define DESELECT()      {delay(10);GPIO_OUT(GPIOA,4,1);}
#endif

static int dma_tx=-1,dma_rx=-1;
static int device_number = -1;

void delay(uint32_t iter);

#define BUFER_SIZE 0x200
static uint8_t *buffer= (uint8_t*)0x20000000;//[BUFER_SIZE];

/* Private typedef -----------------------------------------------------------*/
#define SPI_FLASH_PageSize    0x200

/* Private define ------------------------------------------------------------*/
#define WRITE      0x82  /* Write to Memory instruction */
#define READ       0x52  /* Read from Memory instruction */
#define RDSR       0xD7  /* Read Status Register instruction  */
#define RDID       0x9F  /* Read identification */
#define PE         0x81  /* Page Erase instruction */
#define BE1        0xC7  /* Bulk Erase instruction */
#define BE2        0x94  /* Bulk Erase instruction */
#define BE3        0x80  /* Bulk Erase instruction */
#define BE4        0x9A  /* Bulk Erase instruction */

#define BUSY_Flag  0x80 /* Ready/busy status flag */

#define Dummy_Byte 0xA5

int at45db_id_read(int dev, uint8_t *res)
{
  buffer[0] = 0x9F;
  //spi->Control(ARM_SPI_CONTROL_SS, ARM_SPI_SS_ACTIVE);
  //spi->Transfer(buffer,buffer,5);
  
  SELECT();
  dma_transfer(dma_rx, (void*)buffer, 5);
  dma_transfer(dma_tx, (void*)buffer, 5);
  dma_transfer_wait(dma_tx);
  spi_wait(pspi);
  //while(spi->GetStatus().busy);
  //spi->Control(ARM_SPI_CONTROL_SS, ARM_SPI_SS_INACTIVE);
  DESELECT();
  memcpy(res,&buffer[1],4);
  return 1;
}
int at45db_status_read(int dev, uint8_t *res)
{
  buffer[0] = 0xD7;
  
  SELECT();
  dma_transfer(dma_rx, (void*)buffer, 3);
  dma_transfer(dma_tx, (void*)buffer, 3);
  dma_transfer_wait(dma_tx);
  spi_wait(pspi);
  DESELECT();
  //spi->Control(ARM_SPI_CONTROL_SS, ARM_SPI_SS_ACTIVE);
  //spi->Transfer(buffer,buffer,3);
  //while(spi->GetStatus().busy);
  //spi->Control(ARM_SPI_CONTROL_SS, ARM_SPI_SS_INACTIVE);
  memcpy(res,&buffer[1],2);
  return 1; 
}
//static int at45db_buffer_read(int dev, uint32_t address, uint8_t *res)
//{
//  buffer[0] = 0xD1;
//  
//  SELECT();
//  dma_transfer(dma_rx, (void*)buffer, 8);
//  dma_transfer(dma_tx, (void*)buffer, 8);
//  dma_transfer_wait(dma_tx);
//  spi_wait(pspi);
//  DESELECT();
////  spi->Control(ARM_SPI_CONTROL_SS, ARM_SPI_SS_ACTIVE);
////  spi->Transfer(buffer,buffer,8);
////  while(spi->GetStatus().busy);
////  spi->Control(ARM_SPI_CONTROL_SS, ARM_SPI_SS_INACTIVE);
//  memcpy(res,&buffer[1],4);
//  return 1; 
//}
int at45db_page_erase(int dev, uint32_t address)
{
  buffer[0] = 0x81;
  buffer[1] = (address & 0xFF0000) >> 16;
  buffer[2] = (address & 0xFF00) >> 8;
  buffer[3] = (address & 0xFF);
  
  SELECT();
  dma_transfer(dma_rx, (void*)buffer, 4);
  dma_transfer(dma_tx, (void*)buffer, 4);
  dma_transfer_wait(dma_tx);
  spi_wait(pspi);
  DESELECT();
//  spi->Control(ARM_SPI_CONTROL_SS, ARM_SPI_SS_ACTIVE);
//  spi->Transfer(buffer,buffer,4);
//  while(spi->GetStatus().busy);
//  spi->Control(ARM_SPI_CONTROL_SS, ARM_SPI_SS_INACTIVE);
  return 1; 
}

int at45db_bulk_erase(int dev)
{
  buffer[0] = 0xC7;
  buffer[1] = 0x94;
  buffer[1] = 0x80;
  buffer[1] = 0x9A;
  
  SELECT();
  dma_transfer(dma_rx, (void*)buffer, 4);
  dma_transfer(dma_tx, (void*)buffer, 4);
  dma_transfer_wait(dma_tx);
  spi_wait(pspi);
  DESELECT();
//  spi->Control(ARM_SPI_CONTROL_SS, ARM_SPI_SS_ACTIVE);
//  spi->Transfer(buffer,buffer,4);
//  while(spi->GetStatus().busy);
//  spi->Control(ARM_SPI_CONTROL_SS, ARM_SPI_SS_INACTIVE);
  return 1; 
}
int at45db_write_wait(int dev)
{
  buffer[0] = 0xD7;
  
  SELECT();
  dma_transfer(dma_rx, (void*)buffer, 1);
  dma_transfer(dma_tx, (void*)buffer, 1);
  dma_transfer_wait(dma_tx);
  spi_wait(pspi);
  
  do
  {
    dma_transfer(dma_rx, (void*)buffer, 1);
    dma_transfer(dma_tx, (void*)buffer, 1);
    dma_transfer_wait(dma_tx);
    spi_wait(pspi);
  }
  while ((buffer[0] & 0x80) == 0x00); /* Write in progress */
  DESELECT();
//  spi->Control(ARM_SPI_CONTROL_SS, ARM_SPI_SS_ACTIVE);
//  spi->Transfer(buffer,buffer,1);
//  while(spi->GetStatus().busy);
//
//  do
//  {
//    spi->Transfer(buffer,buffer,1);
//    while(spi->GetStatus().busy);
//  }
//  while ((buffer[0] & 0x80) == 0x00); /* Write in progress */
//
//  spi->Control(ARM_SPI_CONTROL_SS, ARM_SPI_SS_INACTIVE);
  return 1;
}

void at45db_sector_write(int dev, uint8_t* pBuffer, uint32_t SectorAddr, uint16_t NumByteToWrite)
{
  uint16_t  uiPageAddress;
//  uint8_t bytes[2];
  uiPageAddress  = (uint16_t)SectorAddr & 0x0FFF;
  uiPageAddress *= 4;
  buffer[0] = WRITE;
  buffer[1] = (uint8_t)(uiPageAddress>>8);				// msb
  buffer[2] = (uint8_t)(uiPageAddress & 0xFF);				// lsb
  buffer[3] = 0x00;

  SELECT();
  dma_transfer(dma_rx, (void*)buffer, 4);
  dma_transfer(dma_tx, (void*)buffer, 4);
  dma_transfer_wait(dma_tx);
  spi_wait(pspi);
  dma_transfer(dma_rx, (void*)buffer,  NumByteToWrite);
  dma_transfer(dma_tx, (void*)pBuffer, NumByteToWrite);
  dma_transfer_wait(dma_tx);
  spi_wait(pspi);
  DESELECT();
//  spi->Control(ARM_SPI_CONTROL_SS, ARM_SPI_SS_ACTIVE);
//  spi->Transfer(buffer,buffer,4);
//  while(spi->GetStatus().busy);
//
//  spi->Transfer(pBuffer,buffer,NumByteToWrite);
//  while(spi->GetStatus().busy);
//
//  spi->Control(ARM_SPI_CONTROL_SS, ARM_SPI_SS_INACTIVE);

  at45db_write_wait(dev);
}

/*******************************************************************************
* Function Name  : SPI_FLASH_ReadSector
* Description    : Reads a block of data from the FLASH.
* Input          : - pBuffer : pointer to the buffer that receives the data read
*                    from the FLASH.
*                  - ReadAddr : FLASH's internal Pages address to read from.
*                  - NumByteToRead : number of bytes to read from the FLASH.
* Output         : None
* Return         : None
*******************************************************************************/
int at45db_sector_read(int dev, uint8_t* pBuffer, uint32_t SectorAddr, uint16_t NumByteToRead)
{
  uint16_t  uiPageAddress;
//  uint8_t bytes[2];

  uiPageAddress  = (uint16_t)SectorAddr & 0x0FFF;
  uiPageAddress *= 4;

  buffer[0] = READ;
  buffer[1] = (uint8_t)(uiPageAddress>>8);					// msb
  buffer[2] = (uint8_t)(uiPageAddress & 0xFF);          	// lsb
  buffer[3] = 0x00;

  SELECT();
  dma_transfer(dma_rx, (void*)buffer, 8);
  dma_transfer(dma_tx, (void*)buffer, 8);
  dma_transfer_wait(dma_tx);
  spi_wait(pspi);
  dma_transfer(dma_rx, (void*)pBuffer, NumByteToRead);
  dma_transfer(dma_tx, (void*)buffer,  NumByteToRead);
  dma_transfer_wait(dma_tx);
  spi_wait(pspi);
  DESELECT();
//  spi->Control(ARM_SPI_CONTROL_SS, ARM_SPI_SS_ACTIVE);
//
//  spi->Transfer(buffer,buffer,8);
//  while(spi->GetStatus().busy);
//  
//  spi->Transfer(buffer,pBuffer,NumByteToRead);
//  while(spi->GetStatus().busy);
//  
//  spi->Control(ARM_SPI_CONTROL_SS, ARM_SPI_SS_INACTIVE);
  return 0;
}

int at45db_init( SPI_TypeDef *spi )
{
  if(device_number >= 0)
  {
    return -1;
  }
  if (spi != NULL)
    pspi = spi;
  else
    spi = pspi;
  spi_set_rcc_spi_enable(spi, 0);
  spi_set_rcc_spi_enable(spi, 1);
  
  
  spi_set_enable(spi, 0);
  
  spi_set_lsb_first(spi, 0);
  spi_set_master(spi, 1);
  spi_set_baudrate_control(spi, 0);
  
  spi_set_cpol(spi, 1);
  spi_set_cpha(spi, 1);
  
  spi_set_ssm(spi, 1);
  spi_set_ssi(spi, 1);
  
  spi_set_data_format(spi, SPI_DATA_FORMAT_8);
  spi_set_frame_format(spi, SPI_FRAME_FORMAT_MOTOROLA);
  //Для SPI1:
#if (MKPSH10 != 0)
  spi_set_gpio(spi, GPIOB, 14);//miso
  spi_set_gpio(spi, GPIOB, 15);//mosi
  spi_set_gpio(spi, GPIOD,  3);//sck
  
  GPIO_INIT(GPIOB,12,MODE_OUT,OTYPE_PP,OSPEED_VHS,PUPD_N,0,0);//PG12 CS2
  GPIO_OUT(GPIOB,12,1);
#endif

#if (IMC_FTX_MC != 0)
  spi_set_gpio(spi, GPIOA,  6);//miso
  spi_set_gpio(spi, GPIOB,  5);//mosi
  spi_set_gpio(spi, GPIOA,  5);//sck
  
  GPIO_INIT(GPIOA, 4,MODE_OUT,OTYPE_PP,OSPEED_VHS,PUPD_N,0,0);//PG12 CS2
  GPIO_OUT(GPIOA, 4,1);
#endif
  
  spi_set_dma_rx_enable(spi, 1);
  spi_set_dma_tx_enable(spi, 1);
  
  spi_set_enable(spi, 1);
  
  dma_tx = dma_alloc(spi, DMA_DIR_TO_PERIPHERY); // Выделяем поток для периферии
  dma_rx = dma_alloc(spi, DMA_DIR_FROM_PERIPHERY); // Выделяем поток для периферии
  
  
  device_number = 1;
  return device_number;
}

int at45db_cmd( int dev, at45db_cmd_t cmd, void *v)
{
  return 0;
}

int at45db_deinit( int dev )
{
  if(device_number >= 0)
  {
    device_number = -1;
    
    spi_set_dma_rx_enable(pspi, 0);
    spi_set_dma_tx_enable(pspi, 0);
    return 0;
  }
  return -1;
}
