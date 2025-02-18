/**
  ******************************************************************************
  * @file    httpd_cg_ssi.c
  * @author  MCD Application Team
  * @version V1.1.0
  * @date    31-July-2013
  * @brief   Webserver SSI and CGI handlers
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2013 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/

#include "stdint.h"
#include "stdio.h"
#include "string.h"
#include "lwip/debug.h"
#include "httpd.h"
#include "lwip/tcp.h"
#include "fs.h"
#include "main.h"
    
#include <string.h>
#include <stdlib.h>


/* These functions must be implemented by the application */

/** Called when a POST request has been received. The application can decide
 * whether to accept it or not.
 *
 * @param connection Unique connection identifier, valid until httpd_post_end
 *        is called.
 * @param uri The HTTP header URI receiving the POST request.
 * @param http_request The raw HTTP request (the first packet, normally).
 * @param http_request_len Size of 'http_request'.
 * @param content_len Content-Length from HTTP header.
 * @param response_uri Filename of response file, to be filled when denying the
 *        request
 * @param response_uri_len Size of the 'response_uri' buffer.
 * @param post_auto_wnd Set this to 0 to let the callback code handle window
 *        updates by calling 'httpd_post_data_recved' (to throttle rx speed)
 *        default is 1 (httpd handles window updates automatically)
 * @return ERR_OK: Accept the POST request, data may be passed in
 *         another err_t: Deny the POST request, send back 'bad request'.
 */
//Буфер, заполняющийся оперативно, в процессе приема прошивки
#if (MKPSH10 != 0)
 #include "sdram.h"
 static uint8_t *buffer_bin  = (uint8_t*)CODE_ADDRESS_UPL;
#else
 #include "extio.h"
#endif
volatile uint32_t buffer_bin_this = 0xffffffff; //Указатель в buffer_bin
volatile uint32_t buffer_bin_len  = 0; //Размер кода прошивки

#define BOUNDARY_MAX_SIZE 70
#define FILENAME_MAX_SIZE 255
static uint8_t boundary[BOUNDARY_MAX_SIZE+3] = {0,};
static uint8_t file_name[FILENAME_MAX_SIZE+3] = {0,};
//Параметры прошивки: Версия, Подверсия, Дата
static uint8_t gFwVersion, gFwSubversion;
static int8_t gFwYear, gFwMonth, gFwDay;

static volatile uint32_t buffer_content_len = 0;
static volatile uint32_t buffer_http_request_len = 0;
static volatile uint32_t buffer_data_len = 0;
#include "ff.h"
uint32_t gPrgTime;

//Отдельные функции для инициализации и записи буфера прошивки
void InitBinBuf() {
  buffer_bin_this = 0xffffffff;
  buffer_bin_len = 0;
#if (MKPSH10 != 0)
  //Очистить конец блока данных
  uint8_t * cpBuf = (uint8_t *)CODE_ADDRESS_UPL + buffer_data_len - 512;
  memset(cpBuf, 0, 512);
#endif
}

bool IsFirstFrame() {
  if (buffer_bin_this == 0xffffffff) { //Найти начало кода, размер кода
    buffer_bin_this = 0;
    buffer_bin_len = buffer_bin_this;
    return true;
  }
  return false;
}

enum STATE_RCV_FILE {
  STATE_INIT,
  STATE_FILENAME,
  STATE_BEGIN,
  STATE_RCV,
  STATE_END
};
err_t httpd_post_begin(void *connection, const char *uri, const char *http_request,
                       u16_t http_request_len, int content_len, char *response_uri,
                       u16_t response_uri_len, u8_t *post_auto_wnd)
{
  gPrgTime = xTaskGetTickCount();
  char *p=0;
  char *cpContent = 0, *cpData = 0;
  int len =0;
  p=strstr(http_request,"Content-Length:");
  if(p)
  {
    len = strlen(p);
    p[len] = '\r';
  }
  if( (p!=0) && 
      ((p-http_request) <= http_request_len) && 
      (content_len <= 2*1024*1024) && 
      (len) )
  {
    memset(boundary,0,BOUNDARY_MAX_SIZE);
    memset(boundary,'-',2);
    p = strstr(http_request,"boundary=");
    if(p)
    {
      p+=strlen("boundary=");
      cpContent = p;
      len=0;
      while((p[len]!='\r' && p[len]!=0x00) && (len < BOUNDARY_MAX_SIZE)) { len++; };
      memcpy(&boundary[2],p,len); // Скопировали опцию метки границы
      cpData = p + len + 2;
      cpData = strstr(cpData,"\r");
      cpData += 2;
    }
    else
    {
      *post_auto_wnd=1;
      memcpy(response_uri,"/upload.html",sizeof("/upload.html"));
      return ERR_MEM;
    }
  }
  else
  {
    *post_auto_wnd=1;
    memcpy(response_uri,"/upload.html",sizeof("/upload.html"));
    return ERR_MEM;
  }
  
  *post_auto_wnd=0;
  //memcpy(&buffer_post[buffer_post_this],http_request,http_request_len);
  //buffer_post_this+=http_request_len;
  buffer_content_len = content_len;
  buffer_http_request_len = http_request_len;
  buffer_data_len = content_len - (uint32_t)cpData + (uint32_t)cpContent;
  // !!!
  InitBinBuf();
  // !!!
  return ERR_OK;
}

#include <time.h>
bool IsDigit(char iByte) {
  return ((iByte >= '0') && (iByte <= '9'));
}

int8_t DigitFromChar(char iByte) {
  if (IsDigit(iByte)) {
    return (iByte - '0');
  } else {
    return - 1;
  }
}

void ParseFw(char * ipBuf) { //Поиск и разбор имени файла
  char * p = 0;
  int cVer = 0, cSubVer = 0;
  int8_t cFwMonth, cFwDay;
  gFwYear = 0; gFwMonth = 0; cFwDay = 0;
  gFwVersion = 0;
  gFwSubversion = 0;
  if(p = strstr((char *)ipBuf,"filename=\""))
  {
    char *ptr=0;
    p=strstr(p,"\""); // находим название файла
    p++;
    memset(file_name,0,FILENAME_MAX_SIZE+3);
    ptr = (char *)&file_name[strlen((char *)file_name)];
    while(*p!='\"'){*ptr++=*p++;}; // копируем имя файла
  } else {
    //Не найдено имя файла
    return;
  }
  p = (char *)file_name;
  while (p = strstr(p,"-")) {
    ++p;
    if (IsDigit(*p)) {
      cVer = DigitFromChar(*p);
      ++p;
      if (IsDigit(*p)) {
        cVer = cVer * 10 + DigitFromChar(*p);
        ++p;
        if (IsDigit(*p)) {
          cVer = cVer * 10 + DigitFromChar(*p);
          ++p;
        }
      } //Разделительная точка
      if (*p == '.') { //Подверсия
        ++p;
        if (IsDigit(*p)) {
          cSubVer = DigitFromChar(*p);
          ++p;
          if (IsDigit(*p)) {
            cSubVer = cSubVer * 10 + DigitFromChar(*p);
            ++p;
            if (IsDigit(*p)) {
              cSubVer = cSubVer * 10 + DigitFromChar(*p);
              ++p;
            }
          }
          ++p; //День
          if (IsDigit(*p) && IsDigit(p[1])) {
            cFwDay = DigitFromChar(*p) * 10 + DigitFromChar(p[1]);
            p += 3;
            if (IsDigit(*p) && IsDigit(p[1])) {
              cFwMonth = DigitFromChar(*p) * 10 + DigitFromChar(p[1]);
              p += 3;
              if (IsDigit(*p) && IsDigit(p[1]) && IsDigit(p[2]) && IsDigit(p[3])) {
                gFwYear = DigitFromChar(*p) * 1000
                  + DigitFromChar(p[1]) * 100
                  + DigitFromChar(p[2]) * 10
                  + DigitFromChar(p[3]) - 1900;
                gFwMonth      = cFwMonth;
                gFwDay        = cFwDay;
                gFwVersion    = cVer;
                gFwSubversion = cSubVer;
              }
            }
          }
        }
      }
      break;
    }
  }
}

#include "log.h"

void Flash_Program_Word_Buff(uint32_t iAddr, uint32_t * ipBuff, uint16_t iSize) {
  uint32_t uwAddress = 0;
  /* Unlock the Flash **/
  /* Enable the flash control register access */
  FLASH_Unlock();
    
  /* Program the user Flash area word by word */
  /* area defined by FLASH_USER_START_ADDR and FLASH_USER_END_ADDR */
  FLASH_Status status = FLASH_WaitForLastOperation();

  uwAddress = iAddr;
  for (int i = 0; i < iSize; ++i) {
    /* Clear pending flags (if any) */  
    FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | 
                    FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR); 

    if (FLASH_ProgramWord(uwAddress, ipBuff[i]) == FLASH_COMPLETE) {
      uwAddress = uwAddress + 4;
    } else { 
      /* Error occurred while writing data in Flash memory. 
         User can add here some code to deal with this error */
      vTaskDelay(10);
      break;
    }
  }
  /* Lock the Flash to disable the flash control register access (recommended
     to protect the FLASH memory against possible unwanted operation) */
  FLASH_Lock();
  vTaskDelay(5);
}

extern int check_application(uint32_t addr, uint32_t addr_size);

struct sWriteTemp {
  uint16_t Size;
  uint16_t Time;
};

#define WRITE_TEMP_NUM  (1000)
//struct sWriteTemp aWriteTemp[WRITE_TEMP_NUM] @ ".sram";
//uint16_t gWriteTempIdx = 0;
float gfTime;
int gIsRight;

void FixBinBuf() { //Завершение загрузки
  buffer_bin_len = buffer_bin_this;
#if (MKPSH10 != 0)
  gIsRight = check_application((uint32_t)buffer_bin, buffer_bin_len);
  if (gIsRight) {
    uint32_t * cAddr = (uint32_t *)(buffer_bin + buffer_bin_len);
    *cAddr = CODE_RAM_CORRECT;
  }
#endif
#if ((IMC_FTX_MC != 0) || (UTD_M != 0) || (IIP != 0))
  //Проверить CRC
  gIsRight = check_application(CODE_ADDRESS_UPL_FLASH, buffer_bin_len);
//  uint32_t cTime = 0, cSize = 0;
//  for (int i = 0; i < gWriteTempIdx; ++i) {
//    cTime += aWriteTemp[i].Time;
//    cSize += aWriteTemp[i].Size;
//  }
//  gfTime = cTime;
//  gfTime /= cSize;
  gPrgTime = xTaskGetTickCount() - gPrgTime;
#endif
  if (!gIsRight) {
    return;
  }
  // + событие загрузки прошивки: время загрузки, версия ПО
  if ((gFwVersion >= 0) && (gFwSubversion > 0) && (gFwYear > 0)) {
    SaveDateEvent(etSys, ecUploadFwCode, gFwVersion, gFwSubversion,
                  gFwYear, gFwMonth, gFwDay);
    SaveEvent(0, etCount, ecCount); //Спец. событие: Переход на загрузчик
  }
}

/** Called for each pbuf of data that has been received for a POST.
 * ATTENTION: The application is responsible for freeing the pbufs passed in!
 *
 * @param connection Unique connection identifier.
 * @param p Received data.
 * @return ERR_OK: Data accepted.
 *         another err_t: Data denied, http_post_get_response_uri will be called.
 */

#if (MKPSH10 != 0)
err_enum_t UpdateBinBuf(uint8_t * ipBuf, int iLen) {
  if((buffer_bin_this + iLen) >= 2*1024*1024) {
    return ERR_MEM;
  }
  memcpy(&buffer_bin[buffer_bin_this], ipBuf, iLen);
  buffer_bin_this += iLen;
  buffer_bin_len = buffer_bin_this;
  return ERR_OK;
}
#endif

#include "log_flash.h"
#if ((IMC_FTX_MC != 0) || (UTD_M != 0) || (IIP != 0))
enum eBuffWriteResult UpdateFlashBuf(uint8_t * ipBuf, int iLen) {
  //iLen - в байтах, но кратно целым словам
//  uint32_t cTick;
  enum eBuffWriteResult cRez = bwrOk;
//  // +++ Для проверок при записи последнего блока
//  if((CODE_ADDRESS_UPL_FLASH - FLASH_BASE + buffer_bin_this + iLen) >= 2*1024*1024) {
//    cTick = 0;
//  }
//  // +++
  if ((buffer_bin_this + iLen) > CODE_AREA_SIZE) {
    iLen = CODE_AREA_SIZE - buffer_bin_this;
    cRez = bwrFin;
  }
  if((CODE_ADDRESS_UPL_FLASH - FLASH_BASE + buffer_bin_this + iLen) > 2*1024*1024) {
    return bwrMem;
  }
//  if (gWriteTempIdx < WRITE_TEMP_NUM) {
//    cTick = xTaskGetTickCount();
//  }
  uint32_t cAddr = CODE_ADDRESS_UPL_FLASH + buffer_bin_this;
  Flash_Program_Word_Buff(cAddr, (uint32_t *)ipBuf, iLen >> 2);
  buffer_bin_this += iLen;
  buffer_bin_len = buffer_bin_this;
//  if (gWriteTempIdx < WRITE_TEMP_NUM) {
//    cTick = xTaskGetTickCount() - cTick;
//    aWriteTemp[gWriteTempIdx].Time = cTick;
//    aWriteTemp[gWriteTempIdx].Size = iLen;
//    ++gWriteTempIdx;
//  }
  return cRez;
}
#endif

char cPostBuf[1800]/* @ ".sram"*/;
uint16_t cLen = 0;
err_t httpd_post_receive_data(void *connection, struct pbuf *pb)
{
  char * p = 0;
  static int start = 0;
  struct pbuf *buf = pb;
  // !!!
  cLen = start;
  while(buf)
  {
    // !!!
    memcpy(&cPostBuf[cLen], buf->payload, buf->len);
    cLen += buf->len;
    // !!!
    buf = buf->next;
  }
  // !!!
  if (IsFirstFrame()) { //Найти начало кода, размер кода
    p = strstr(cPostBuf,"\r\n\r\n");
    if (p) {
      p += 4;
      start = (uint32_t)p - (uint32_t)cPostBuf;
      cLen = cLen - start;
#if (MKPSH10 != 0)
      start = 0;
      if (UpdateBinBuf((uint8_t *)p, cLen) != ERR_OK) {
        pbuf_free(pb);
        return ERR_MEM;
      }
#endif
#if ((IMC_FTX_MC != 0) || (UTD_M != 0) || (IIP != 0))
      start = cLen & 0x03;
      if (UpdateFlashBuf((uint8_t *)p, cLen & 0xfffc) != bwrOk) {
        pbuf_free(pb);
        return ERR_MEM;
      }
      p += cLen & 0xfffc;
      memcpy(cPostBuf, (uint8_t *)p, start);
#endif
    }
    ParseFw(cPostBuf); //Вычисление даты и номера версии
  } else {
    p = strstr(cPostBuf, (char *)boundary);
    if (p) {
      cLen = (uint32_t)p - (uint32_t)cPostBuf - 2;
    }
#if (MKPSH10 != 0)
    start = 0;
    if (UpdateBinBuf((uint8_t *)cPostBuf, cLen) != ERR_OK) {
#endif
#if ((IMC_FTX_MC != 0) || (UTD_M != 0) || (IIP != 0))
    start = cLen & 0x03;
    enum eBuffWriteResult cRez = UpdateFlashBuf((uint8_t *)cPostBuf, cLen & 0xfffc);
    if (cRez != bwrOk) {
      if (cRez == bwrFin) {
        FixBinBuf(); //Завершение загрузки
        pbuf_free(pb);
        return ERR_OK;
      }
#endif
      pbuf_free(pb);
      return ERR_MEM;
    }
#if ((IMC_FTX_MC != 0) || (UTD_M != 0) || (IIP != 0))
      memcpy(cPostBuf, (uint8_t *)&(cPostBuf[cLen & 0xfffc]), start);
#endif
    if (p) { //Размер и признак правильной загрузки
      FixBinBuf(); //Завершение загрузки
    }
  }
  // !!!
  pbuf_free(pb);
  return ERR_OK;
}

/** Called when all data is received or when the connection is closed.
 * The application must return the filename/URI of a file to send in response
 * to this POST request. If the response_uri buffer is untouched, a 404
 * response is returned.
 *
 * @param connection Unique connection identifier.
 * @param response_uri Filename of response file, to be filled when denying the request
 * @param response_uri_len Size of the 'response_uri' buffer.
 */
void httpd_post_finished(void *connection, char *response_uri, u16_t response_uri_len)
{
  memcpy(response_uri,"/upload.html",sizeof("/upload.html"));
  return;
}

void httpd_post_data_recved(void *connection, u16_t recved_len)
{
//  buffer_post_rcv_len = recved_len;
  return;
}