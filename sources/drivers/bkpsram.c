/**
  ******************************************************************************
  * @file    bkpsram.c
  * @author  Dmitry Vakhrushev ( vdv.18@mail.ru )
  * @version V1.0
  * @date    02.05.2016
  * @brief   --
  ******************************************************************************
  * @attention
  * 
  ******************************************************************************
  */ 

#include "bkpsram.h"
#include "stdarg.h"

#include "stm32f4xx.h"

#include "crc.h"

const uint16_t BACKUP_SRAM_SIZE = 0x1000;
const uint16_t BACKUP_RTC_SIZE = 80;

void delay(uint32_t iter)
{
  volatile uint32_t temp = iter;
  while(temp) temp--;
}


void bkpsram_lock()
{
  uint32_t crcres = 0x00;
  uint8_t* base_addr = (uint8_t *) BKPSRAM_BASE;
  delay(10);
  crcres = hw_crc32(base_addr, BACKUP_SRAM_SIZE);
  bkprtc_write(&crcres,4,0);
  PWR->CR &=~PWR_CR_DBP;
  delay(10);
}
void bkpsram_unlock()
{
  delay(10);
  RCC->AHB1ENR |= RCC_AHB1ENR_BKPSRAMEN;
  RCC->APB1ENR |= RCC_APB1ENR_PWREN;
  PWR->CR |= PWR_CR_DBP;
  PWR->CSR |= PWR_CSR_BRE;
  delay(10);
}
int8_t bkpsram_write( uint8_t *data, uint16_t bytes, uint16_t offset ) {
  uint32_t crcres = 0x00;
  uint8_t* base_addr = (uint8_t *) BKPSRAM_BASE;
  uint16_t i;
  if( bytes + offset >= BACKUP_SRAM_SIZE ) {
    /* ERROR : the last byte is outside the backup SRAM region */
    return -1;
  }
  RCC->AHB1ENR |= RCC_AHB1ENR_BKPSRAMEN;
  /* disable backup domain write protection */
  RCC->APB1ENR |= RCC_APB1ENR_PWREN;
  PWR->CR |= PWR_CR_DBP;
  /** enable the backup regulator (used to maintain the backup SRAM content in
    * standby and Vbat modes).  NOTE : this bit is not reset when the device
    * wakes up from standby, system reset or power reset. You can check that
    * the backup regulator is ready on PWR->CSR.brr, see rm p144 */
  PWR->CSR |= PWR_CSR_BRE;
  for( i = 0; i < bytes; i++ ) {
    *(base_addr + offset + i) = *(data + i);
  }
  crcres = hw_crc32(base_addr, BACKUP_SRAM_SIZE);
  bkprtc_write(&crcres,4,0);
  PWR->CR &=~PWR_CR_DBP;
  
  return 0;
}

int8_t bkpsram_read( uint8_t *data, uint16_t bytes, uint16_t offset ) {
  const uint16_t BACKUP_SRAM_SIZE = 0x1000;
  uint8_t* base_addr = (uint8_t *) BKPSRAM_BASE;
  uint16_t i;
  if( bytes + offset >= BACKUP_SRAM_SIZE ) {
    /* ERROR : the last byte is outside the backup SRAM region */
    return -1;
  }
  RCC->AHB1ENR |= RCC_AHB1ENR_BKPSRAMEN;
  for( i = 0; i < bytes; i++ ) {
    *(data + i) = *(base_addr + offset + i);
  }
  return 0;
}

int8_t bkprtc_write( uint32_t *data, uint16_t bytes, uint16_t offset ) {
  volatile uint32_t* base_addr = &(RTC->BKP0R);
  uint16_t i;
  if( bytes + offset >= BACKUP_RTC_SIZE ) {
    /* ERROR : the last byte is outside the backup SRAM region */
    return -1;
  } else if( offset % 4 || bytes % 4 ) {
    /* ERROR: data start or num bytes are not word aligned */
    return -2;
  } else {
    bytes >>= 2;      /* divide by 4 because writing words */
  }
  /* disable backup domain write protection */
  RCC->APB1ENR |= RCC_APB1ENR_PWREN;
  PWR->CSR |= PWR_CSR_BRE;
  for( i = 0; i < bytes; i++ ) {
    *(base_addr + offset + i) = *(data + i);
  }
  PWR->CR &=~PWR_CR_DBP;
  // consider also disabling the power peripherial?
  return 0;
}

int8_t bkprtc_read( uint32_t *data, uint16_t bytes, uint16_t offset ) {
  volatile uint32_t* base_addr = &(RTC->BKP0R);
  uint16_t i;
  if( bytes + offset >= BACKUP_RTC_SIZE ) {
    /* ERROR : the last byte is outside the backup SRAM region */
    return -1;
  } else if( offset % 4 || bytes % 4 ) {
    /* ERROR: data start or num bytes are not word aligned */
    return -2;
  } else {
    bytes >>= 2;      /* divide by 4 because writing words */
  }
  /* read should be 32 bit aligned */
  for( i = 0; i < bytes; i++ ) {
    *(data + i) = *(base_addr + offset + i);
  }
  return 0;
}

void bkpsram_sram_reset( void )
{
  uint8_t* base_addr = (uint8_t *) BKPSRAM_BASE;
  uint16_t i;
  RCC->AHB1ENR |= RCC_AHB1ENR_BKPSRAMEN;
  RCC->APB1ENR |= RCC_APB1ENR_PWREN;
  PWR->CR |= PWR_CR_DBP;
  PWR->CSR |= PWR_CSR_BRE;
  for( i = 0; i < BACKUP_SRAM_SIZE; i++ ) {
    *(base_addr + i) = 0x00;
  }
  PWR->CR &=~PWR_CR_DBP;
}

void bkpsram_rtc_reset( void )
{
  // Reset clock and backup domain
  RCC->BDCR = RCC_BDCR_BDRST | RCC_BDCR_RTCEN;
  delay(100);
  RCC->BDCR &=~RCC_BDCR_BDRST;
  delay(100);
  RCC->BDCR |= RCC_BDCR_RTCSEL_1; // LSI clock
}

int bkpsram_crc_correct( void )
{
  uint32_t crcres = 0x00;
  uint32_t crccheck = 0x00;
  uint8_t* base_addr = (uint8_t *) BKPSRAM_BASE;
  bkprtc_read(&crccheck,4,0);
  crcres = hw_crc32(base_addr, BACKUP_SRAM_SIZE);
  if(crcres == crccheck)
  {
    return 1;
  }
  return 0;
}

int bkpsram_rtc_correct( void )
{
  int retval = 0;
  
  if(
      (RCC->BDCR & RCC_BDCR_RTCSEL) 
               == 
      ( RCC_BDCR_RTCSEL_1 )
    )
  {
    retval = 1;
  }
  return retval;
}

void bkpsram_reset( void )
{
  uint8_t* base_addr = (uint8_t *) BKPSRAM_BASE;
  uint16_t i;
  RCC->AHB1ENR |= RCC_AHB1ENR_BKPSRAMEN;
  RCC->APB1ENR |= RCC_APB1ENR_PWREN;
  PWR->CR |= PWR_CR_DBP;
  PWR->CSR |= PWR_CSR_BRE;
  for( i = 0; i < BACKUP_SRAM_SIZE; i++ ) {
    *(base_addr + i) = 0x00;
  }
  PWR->CR &=~PWR_CR_DBP;
}

void bkpsram_init( void )
{
  /**
   * Включаем доступ к RTC и BKPSRAM
   * 1. RCC_APB1ENR bit PWREN = 1
   * 2. PWR_CR bit DBP = 1
   * 3. RTC clk src; 
   * 4. RCC_BDCR bit RTCEN = 1
   * 5. RCC_AHB1ENR bit BKPSRAMEN = 1
   */
  delay(100);
  
  /* 1 */
  RCC->APB1ENR |= RCC_APB1ENR_PWREN;
  delay(100);
  /* 2 */
  PWR->CR |= PWR_CR_DBP;
  delay(100);
  /* 3 */
  RCC->CSR |= RCC_CSR_LSION;
  delay(100);
  /* 4 */
  RCC->BDCR|= RCC_BDCR_RTCEN;
  delay(100);
  /* 5 */
  delay(100);
  RCC->AHB1ENR |= RCC_AHB1ENR_BKPSRAMEN;
  if( (! bkpsram_rtc_correct())  || 
      (! bkpsram_crc_correct()) )
  {
    bkpsram_rtc_reset(); // reset clock
    bkpsram_sram_reset();
  }
  delay(100);
  PWR->CR &=~PWR_CR_DBP;
  delay(100);
}

