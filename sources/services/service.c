//******************************************************************************
// Имя файла    :       service.c
// Заголовок    :       
// Автор        :       Вахрушев Д.В.
// Дата         :       
//
//------------------------------------------------------------------------------
//  Сервис для поиска устройства в сети широковещательными запросами и ответами.     
//  Так же сервис необходим для настройки устройства, 
//      когда адрес устройства неизвестен
//******************************************************************************
#include "service.h"

#include <stdint.h>
#include <stdio.h>
#include "lwip/udp.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "string.h"

#include "db.h"

#include "main.h"

/* Private typedef -----------------------------------------------------------*/
#define MAX_DHCP_TRIES        4

/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
extern struct netif gnetif;


//static struct udp_pcb *ptel_pcb;

void __reboot__() {
  NVIC_SystemReset();
}

#include "fw.h"
#ifdef BOOTLOADER
#include "timer.h"
#include "timer_hw_defined.h"
  #define REBOOT(time) timer_create(TIMER_ONE_SHOT_START,TIMER_MILLISECOND(time),service_reboot);
void service_reboot(timer_t timer)
{
  __reboot__();
}

#else
  #define REBOOT_NOW() { __reboot__(); }
  #define REBOOT(time) {vTaskDelay(time); __reboot__();}
#endif

#define BL_BASE         0x08000000
#define BL_OFFSET       0x00020000
#define BL_VER_OFFSET   0x00000400

#ifdef BOOTLOADER
uint8_t *fw_base = (uint8_t*)(FW_BASE + FW_OFFSET + FW_VER_OFFSET);
#else
uint8_t *bl_base = (uint8_t*)(BL_BASE + BL_OFFSET + BL_VER_OFFSET);
#endif


static char resp[128];
void service_udp_recv(void *arg, struct udp_pcb *pcb, struct pbuf *p, const ip_addr_t *addr, u16_t port)
{
  uint8_t *msg_in;
  uint8_t temp;
  struct pbuf *_p;
  if (p != NULL) 
  {
    msg_in = (uint8_t*)p->payload;
    switch(msg_in[0]){  
      case SERVICE_MSG_GET_INFO:
      case SERVICE_MSG_GET_INFO_BC:
        {
          /***
                Request:
           0x00 (msg type)
                
                Response:
           0x00 (msg type); 
           0x00 (MSB PktID); 
           0x0000 (MSB STATE); 
           0x00000000(TIME sec); 
           0x00/MAJ 0x00/MIN (VER BL);
           0x00/MAJ 0x00/MIN (VER FW); 
           0x00:00:00:00:00:00 (LSB MAC); 
           0x00:00:00:00 (LSB IP); 
          // STATUS 0x0000 - Loader
          //        0x0001 - Updater
          //        0x0002 - FW
          //        0x0003 - FW2
          */
          int size = 0;
          uint8_t msg[] = { SERVICE_MSG_RESPONSE | msg_in[0],  msg_in[1], 
#ifdef BOOTLOADER
                0x00, SERVICE_MSG_STATUS_UPDATER, // STATE
#else
                0x00, SERVICE_MSG_STATUS_FW, // STATE
#endif
                0, 0, 0, 0, // TIME
#ifdef BOOTLOADER
                firmware_data.fvh,firmware_data.fvl,
                fw_base[0], fw_base[1], // VER FW
#else
                bl_base[0], bl_base[1], // VER BL
                firmware_data.fvh,firmware_data.fvl,
#endif
                
                (uint8_t)(gnetif.hwaddr[0]),
                (uint8_t)(gnetif.hwaddr[1]),
                (uint8_t)(gnetif.hwaddr[2]),
                (uint8_t)(gnetif.hwaddr[3]),
                (uint8_t)(gnetif.hwaddr[4]),
                (uint8_t)(gnetif.hwaddr[5]), // MAC
                
                (uint8_t)(gnetif.ip_addr.addr), 
                (uint8_t)(gnetif.ip_addr.addr>>8), 
                (uint8_t)(gnetif.ip_addr.addr>>16), 
                (uint8_t)(gnetif.ip_addr.addr>>24), // IP
          };
          size = sizeof(msg);
          if(size < 128)
          {
            _p = pbuf_alloc(PBUF_TRANSPORT,size,PBUF_RAM);
            if(_p)
            {
              memcpy (_p->payload, msg, size);
              if(msg_in[0] & 0x10 > 0)
              {
                udp_sendto(pcb, _p, IP_ADDR_BROADCAST, SERVICE_PORT_TX); //dest port
              }
              else
              {
                udp_sendto(pcb, _p, addr, SERVICE_PORT_TX); //dest port
              }
              pbuf_free(_p);
            }
          }
        }
        break;
      case SERVICE_MSG_GET_PARAM:
        {
        }
        break;
      case SERVICE_MSG_SET_PARAM:
        {
        }
        break;
  
      case SERVICE_MSG_GET_TEXT_PARAM_BY_MAC_BC:
      case SERVICE_MSG_GET_TEXT_PARAM_BY_MAC:
        {
          /**
          Request:
          00 0xNN (SERVICE_MSG_GET_TEXT_PARAM)
          01 0xID (0-255)
          02 0x00 00 00 00 00 00 <- MAC
          08 0xSIZE PARAM (0-255) <- TEXT PARAMETER
          09 0x00 ... (SIZE PARAM)
          
          Response:
          0xNN | SERVICE_MSG_RESPONSE
          0xID SAME 
          0x00 00 00 00 00 00 <- MAC
          0xSIZE PARAMETER
          0x00 ... (SIZE PARAMETER)
          0xSIZE VALUE
          0x00 ... (SIZE VALUE)
          */
          
          resp[0] = SERVICE_MSG_RESPONSE | msg_in[0];
          resp[1] = msg_in[1];
          memcpy(resp,msg_in,p->len);
          if(memcmp(&msg_in[2],gnetif.hwaddr,6) != 0)
          {
            break;
          }
          temp = resp[8];
          resp[9+temp] = 0x00;
          if(dbbkpsram.get(&resp[9],&resp[9+temp+1]) != DB_OK)
          {
            break;
          }
          resp[9+temp] = strlen(&resp[9+temp+1]);
          temp = 9 + temp + resp[9+temp] + 2;
          if(temp < 128)
          {
            _p = pbuf_alloc(PBUF_TRANSPORT,temp,PBUF_RAM);
            if(_p)
            {
              memcpy (_p->payload, resp, temp);
              if(msg_in[0] & 0x10 > 0)
              {
                udp_sendto(pcb, _p, IP_ADDR_BROADCAST, SERVICE_PORT_TX); //dest port
              }
              else
              {
                udp_sendto(pcb, _p, addr, SERVICE_PORT_TX); //dest port
              }
              pbuf_free(_p);
            }
          }
        }
        break;
      case SERVICE_MSG_SET_TEXT_PARAM_BY_MAC_BC:
      case SERVICE_MSG_SET_TEXT_PARAM_BY_MAC:
        {
          /**
          Request:
          00 0xNN (SERVICE_MSG_GET_TEXT_PARAM)
          01 0xID (0-255)
          02 0x00 00 00 00 00 00 <- MAC
          08 0xSIZE PARAM (0-255) <- TEXT PARAMETER
          09 0x00 ... (SIZE PARAM)
          09+SIZE PARAM 0xSIZE VALUE
          10+SIZE PARAM 0x00 ... (SIZE VAL)
          
          Response:
          0xNN | SERVICE_MSG_RESPONSE
          0xID SAME 
          0x00 00 00 00 00 00 <- MAC
          0xSIZE PARAMETER
          0x00 ... (SIZE PARAMETER)
          0xSIZE VALUE
          0x00 ... (SIZE VALUE)
          */
          
          resp[0] = SERVICE_MSG_RESPONSE | msg_in[0];
          resp[1] = msg_in[1];
          memcpy(resp,msg_in,p->len);
          if(memcmp(&msg_in[2],gnetif.hwaddr,6) != 0)
          {
            break;
          }          
          temp = resp[8];
          temp = resp[9+temp];
          resp[9+resp[8]+temp+1] = 0x00;
          resp[9+resp[8]] = 0x00;
          if(dbbkpsram.set(&resp[9],&resp[9+resp[8]+1]) != DB_OK)
          {
            break;
          }
          if(temp < 128)
          {
            _p = pbuf_alloc(PBUF_TRANSPORT,p->len,PBUF_RAM);
            if(_p)
            {
              msg_in[0] |= SERVICE_MSG_RESPONSE;
              memcpy (_p->payload, msg_in, p->len);
              if(msg_in[0] & 0x10 > 0)
              {
                udp_sendto(pcb, _p, IP_ADDR_BROADCAST, SERVICE_PORT_TX); //dest port
              }
              else
              {
                udp_sendto(pcb, _p, addr, SERVICE_PORT_TX); //dest port
              }
              pbuf_free(_p);
            }
          }
        }
        break;
      case SERVICE_MSG_GET_TEXT_PARAM_BC:
      case SERVICE_MSG_GET_TEXT_PARAM:
        {
          /**
          Request:
          0xNN (SERVICE_MSG_GET_TEXT_PARAM)
          0xID (0-255)
          0x00 -- NUMBER PARAMETERS
          0xSIZE PARAM (0-255) <- TEXT PARAMETER
          0x00 ... (SIZE PARAM)
          0xSIZE PARAM (0-255) <- TEXT PARAMETER
          0x00 ... (SIZE PARAM)
          
          Response:
          0xNN | SERVICE_MSG_RESPONSE
          0xID SAME 
          0xCNT (0,1,2 ... 255 sequence of answere; 255 LAST, 0,1,255 or only 255)
          0xSIZE PARAMETER
          0x00 ... (SIZE PARAMETER)
          0xSIZE VALUE
          0x00 ... (SIZE VALUE)
          0xSIZE PARAMETER
          0x00 ... (SIZE PARAMETER)
          0xSIZE VALUE
          0x00 ... (SIZE VALUE)
          
          ...
          */
          
          resp[0] = SERVICE_MSG_RESPONSE | msg_in[0];
          resp[1] = msg_in[1];
          if(msg_in[2] != 0x00 && msg_in[2] != 0x01)
          {
            _p = pbuf_alloc(PBUF_TRANSPORT,2,PBUF_RAM);
            if(_p)
            {
              ((uint8_t*)_p->payload)[0] = SERVICE_MSG_RESPONSE_ERROR | msg_in[0];
              ((uint8_t*)_p->payload)[1] = msg_in[1];
              if(msg_in[0] & 0x10 > 0)
              {
                udp_sendto(pcb, _p, IP_ADDR_BROADCAST, SERVICE_PORT_TX); //dest port
              }
              else
              {
                udp_sendto(pcb, _p, addr, SERVICE_PORT_TX); //dest port
              }
              pbuf_free(_p);
            }
            break;
          }
          resp[2] = 0x00;
          resp[3] = msg_in[3];
          memcpy(&resp[4],&msg_in[4],msg_in[3]);
          temp = msg_in[4+msg_in[3]];
          resp[4+resp[3]] = 0x00;
          msg_in[4+msg_in[3]+temp] = 0x00;
          if(dbbkpsram.get(&resp[4],&resp[4+resp[3]+1]) != DB_OK)
          {
            _p = pbuf_alloc(PBUF_TRANSPORT,2,PBUF_RAM);
            if(_p)
            {
              ((uint8_t*)_p->payload)[0] = SERVICE_MSG_RESPONSE_ERROR | msg_in[0];
              ((uint8_t*)_p->payload)[1] = msg_in[1];
              if(msg_in[0] & 0x10 > 0)
              {
                udp_sendto(pcb, _p, IP_ADDR_BROADCAST, SERVICE_PORT_TX); //dest port
              }
              else
              {
                udp_sendto(pcb, _p, addr, SERVICE_PORT_TX); //dest port
              }
              pbuf_free(_p);
            }
            break;
          }
          resp[4+resp[3]] = strlen(&resp[4+resp[3]+1]);
          temp = resp[4+resp[3]] + resp[3] + 4+2;
          if(temp < 128)
          {
            _p = pbuf_alloc(PBUF_TRANSPORT,temp,PBUF_RAM);
            if(_p)
            {
              memcpy (_p->payload, resp, temp);
              udp_sendto(pcb, _p, IP_ADDR_BROADCAST, SERVICE_PORT_TX); //dest port
              pbuf_free(_p);
            }
          }
        }
        break;
      case SERVICE_MSG_SET_TEXT_PARAM_BC:
      case SERVICE_MSG_SET_TEXT_PARAM:
        {
          /**
          Request:
          0xNN (SERVICE_MSG_GET_TEXT_PARAM)
          0xID (0-255)
          0x00 PARAMETERS COUNT
          0xSIZE PARAMETER (0-255) <-TEXT PARAMETER
          0x00 ... (SIZE PARAM)
          0xSIZE VALUE
          0x00 ... (SIZE VAL)
          Response: Same Readed
          0xNN | SERVICE_MSG_RESPONSE
          0xID (0-255)
          0x00 PARAMETERS COUNT
          0xSIZE PARAMETER (0-255) <-TEXT PARAMETER
          0x00 ... (SIZE PARAM)
          0xSIZE VALUE
          0x00 ... (SIZE VAL)
          */
          memcpy(resp,msg_in,p->len);
          resp[0] |= SERVICE_MSG_RESPONSE;
          if(msg_in[2] != 0x00 && msg_in[2] != 0x01)
          {
            _p = pbuf_alloc(PBUF_TRANSPORT,2,PBUF_RAM);
            if(_p)
            {
              ((uint8_t*)_p->payload)[0] = SERVICE_MSG_RESPONSE_ERROR | msg_in[0];
              ((uint8_t*)_p->payload)[1] = msg_in[1];
              if(msg_in[0] & 0x10 > 0)
              {
                udp_sendto(pcb, _p, IP_ADDR_BROADCAST, SERVICE_PORT_TX); //dest port
              }
              else
              {
                udp_sendto(pcb, _p, addr, SERVICE_PORT_TX); //dest port
              }
              pbuf_free(_p);
            }
            break;
          }
          resp[2] = 0x00;
          resp[3] = msg_in[3];
          temp = resp[4+resp[3]];
          resp[4+resp[3]] = 0x00;
          resp[4+resp[3]+temp+1] = 0x00;
          if(dbbkpsram.set(&resp[4],&resp[4+resp[3]+1]) != DB_OK)
          {
            _p = pbuf_alloc(PBUF_TRANSPORT,2,PBUF_RAM);
            if(_p)
            {
              ((uint8_t*)_p->payload)[0] = SERVICE_MSG_RESPONSE_ERROR | msg_in[0];
              ((uint8_t*)_p->payload)[1] = msg_in[1];
              if(msg_in[0] & 0x10 > 0)
              {
                udp_sendto(pcb, _p, IP_ADDR_BROADCAST, SERVICE_PORT_TX); //dest port
              }
              else
              {
                udp_sendto(pcb, _p, addr, SERVICE_PORT_TX); //dest port
              }
              pbuf_free(_p);
            }
            break;
          }
          resp[4+resp[3]] = strlen(&resp[4+resp[3]+1]);
          temp = resp[4+resp[3]] + resp[3] + 4+2;
          
          if(temp < 128)
          {
            _p = pbuf_alloc(PBUF_TRANSPORT,temp,PBUF_RAM);
            if(_p)
            {
              msg_in[0] |= SERVICE_MSG_RESPONSE;
              memcpy (_p->payload, msg_in, p->len);
              if(msg_in[0] & 0x10 > 0)
              {
                udp_sendto(pcb, _p, IP_ADDR_BROADCAST, SERVICE_PORT_TX); //dest port
              }
              else
              {
                udp_sendto(pcb, _p, addr, SERVICE_PORT_TX); //dest port
              }
              pbuf_free(_p);
            }
          }
        }
        break;
      case SERVICE_MSG_REBOOT:
        {
          char value[16];
#ifndef BOOTLOADER
          int address[6];           
#endif  
          memset(value,0,sizeof(value));
          
          msg_in[0] |= SERVICE_MSG_RESPONSE_ERROR;
          msg_in[1]  = msg_in[1];
          
          if(p->len >= 6+4){
            if((msg_in[2] == 0) && (msg_in[3] == 0))
            {
              if(0 != memcmp(gnetif.hwaddr, &msg_in[4], 6))
              {
                break;
              }
            }
          }
          
#ifndef BOOTLOADER
          address[0] = gnetif.hwaddr[0];
          address[1] = gnetif.hwaddr[1];
          address[2] = gnetif.hwaddr[2];
          address[3] = gnetif.hwaddr[3];
          address[4] = gnetif.hwaddr[4];
          address[5] = gnetif.hwaddr[5];
          sprintf(value,"%02X_%02X_%02X_%02X_%02X_%02X",
                  address[0],address[1],address[2],
                  address[3],address[4],address[5]);
          dbbkpsram.set("HW_ADDR",value);
          
          address[3] = (gnetif.ip_addr.addr>>24)&0xFF;
          address[2] = (gnetif.ip_addr.addr>>16)&0xFF;
          address[1] = (gnetif.ip_addr.addr>>8)&0xFF;
          address[0] = (gnetif.ip_addr.addr)&0xFF;
          sprintf(value,"%d_%d_%d_%d",address[0], address[1], address[2], address[3]);
          dbbkpsram.set("IP_ADDR",value);
          
          address[3] = (gnetif.netmask.addr>>24)&0xFF;
          address[2] = (gnetif.netmask.addr>>16)&0xFF;
          address[1] = (gnetif.netmask.addr>>8)&0xFF;
          address[0] = (gnetif.netmask.addr)&0xFF;
          sprintf(value,"%d_%d_%d_%d",address[0], address[1], address[2], address[3]);
          dbbkpsram.set("IP_MASK",value);
          
          address[3] = (gnetif.gw.addr>>24)&0xFF;
          address[2] = (gnetif.gw.addr>>16)&0xFF;
          address[1] = (gnetif.gw.addr>>8)&0xFF;
          address[0] = (gnetif.gw.addr)&0xFF;
          sprintf(value,"%d_%d_%d_%d",address[0], address[1], address[2], address[3]);
          dbbkpsram.set("IP_GW",value);
#endif  
          udp_sendto(pcb, p, IP_ADDR_BROADCAST, SERVICE_PORT_TX); //dest port
          REBOOT(1000);
        }
        break;
      default:
        {
          //msg_in[0] |= SERVICE_MSG_RESPONSE_ERROR;
          //udp_sendto(pcb, p, IP_ADDR_BROADCAST, SERVICE_PORT_TX); //dest port
        }
        break;
    }
    pbuf_free(p);
  }
}


//#ifdef BOOTLOADER
//void service_start_timer(timer_t timer)
//{
//  static int service_cnt = 0;
//  uint8_t size = 0;
//  const char message[] = "BOOTLOADER=ON;STATUS=";
//  struct pbuf *p;
//  
//  char msg[128]={SERVICE_MSG_TXT_INDICATION|SERVICE_MSG_RESPONSE,(char)service_cnt, 0x00, 0x00,  };
//  memset(&msg[4],0,sizeof(msg)-4);
//  size = sizeof(message)+4 -1 ;
//  strcpy(&msg[4],message);
//  if(dbbkpsram.get("STATUS",&msg[size]) != DB_OK)
//  {
//    strcpy(&msg[size],"NONE");
//  }
//  size = strlen(&msg[4]) + 4;
//  msg[size++] = ';';
//  msg[size] = '\0';
//  
//  p = pbuf_alloc(PBUF_TRANSPORT,size,PBUF_RAM);
//  if(p)
//  {
//    memcpy (p->payload, msg, size);
//    udp_sendto(ptel_pcb, p, IP_ADDR_BROADCAST, 5522);
//    pbuf_free(p);
//  }
//  
//  if(++service_cnt>10)
//  {
//    service_cnt = 0;
//    timer_stop(timer);
//    timer_delete(&timer);
//  }
//}
//#else
//#endif


//void service_init(void)
//{
//
////  ptel_pcb = udp_new();
////
////  udp_bind(ptel_pcb, IP_ADDR_ANY, SERVICE_PORT_RX);
////  udp_recv(ptel_pcb, service_udp_recv, NULL);
//
////#ifdef BOOTLOADER
////  timer_create(TIMER_REPEAT_START,TIMER_MILLISECOND(100),service_start_timer);
////#else  
////#endif 
//}
