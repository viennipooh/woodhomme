//******************************************************************************
// ��� �����    :       log_flash.h
// ���������    :       
// �����        :       ���������� �.�.
// ����         :       15.01.2019
//
//------------------------------------------------------------------------------
/**
����������� ������� ����. �������� � Flash-�������

**/
#ifndef __LOG_FLASH_H
#define __LOG_FLASH_H

#define CLEAR_WORD  (0xffffffff)

#include <semphr.h>
#include "extio.h"

//������� Flash-������
#define CODE_START_SECTOR_WRK   ( 6)  //code start sector for work area
#define CODE_START_SECTOR_REZ   (18)  //code start sector for reserved area

//��� ������� ���������� ������ ����, ������������� ��� �������� ����� Web
//����� ��������� ����� ��������� ������������ ���� � Linker
#if (UTD_M != 0)
  #define CODE_SECTOR_NUMBER      ( 6)  //����� ��������, ���������� ����� (�� 128��)
#else
  #define CODE_SECTOR_NUMBER      ( 6)  //����� ��������, ���������� ����� (�� 128��)
#endif
#define CODE_SECTOR_SIZE        (0x20000)  //������ �������, ����������� ����� (128��)
#define CODE_AREA_SIZE     (CODE_SECTOR_SIZE * CODE_SECTOR_NUMBER)  //������ ������� ����

enum eBuffWriteResult { //��������� ������ ����� ������ � ����� ����
  bwrOk = 0,  //�����
  bwrMem,     //������ ������
  bwrFin,     //����� ������
  bwrCount
};

enum eCodeArea { //������� Flash-������:
  caWork,   //�������
  caReserv, //���������
  caUpload, //����������� (��� �������� �� ����� Web)
  caCount
};

extern xSemaphoreHandle Flash_mutex;

extern void Flash_Program_Init();
extern void Flash_Program_Word(uint32_t iAddr, uint32_t iWord);
extern void Flash_Program_Event(uint32_t iAddr, struct sEvent * ipEvent);
extern void Flash_Erase_Sector(uint32_t iSector);

extern void EraseSectorArea(uint8_t iStartSectorIdx, uint8_t iSectorNum);
extern uint32_t Sector_Address(uint32_t iSectorIdx);
extern bool IsSectorEmptyByIdx(uint16_t iSectIdx);
extern void EraseCodeArea(enum eCodeArea iCodeArea);

#endif //__LOG_FLASH_H
