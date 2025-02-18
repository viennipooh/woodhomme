/**
  ******************************************************************************
  * @file    crc.h
  * @author  Dmitry Vakhrushev ( vdv.18@mail.ru )
  * @version V1.0
  * @date    31.01.2017
  * @brief   --
  ******************************************************************************
  * @attention
  *
  ******************************************************************************
  */ 
  
#ifndef _CRC_H__
#define _CRC_H__

unsigned long fast_crc32(unsigned long sum, unsigned char *p, unsigned long len);
unsigned long slow_crc32(unsigned long sum, unsigned char *p, unsigned long len);
unsigned long set_crc32_init_value(unsigned long start);
unsigned short slow_crc16(unsigned short sum, unsigned char *p, unsigned int len);
unsigned short fast_crc16(unsigned short sum, unsigned char *p, unsigned int len);
unsigned long hw_crc32(unsigned char* data, unsigned long len);

#endif /* _CRC_H__ */


