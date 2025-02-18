//******************************************************************************
// Имя файла    :       service.h
// Заголовок    :       
// Автор        :       Вахрушев Д.В.
// Дата         :       08.02.2016
//
//------------------------------------------------------------------------------
//      
//      
//******************************************************************************
#ifndef __SERVICE_H_
#define __SERVICE_H_

#define SERVICE_PORT_TX    5522
#define SERVICE_PORT_RX    5522

#include "stdint.h"

struct service_msg_s {
  uint8_t type;
  uint8_t msg_id;
  uint8_t msg[];
};

struct service_msg_txt_s {
  uint8_t pkt_cnt;
  uint8_t pkt_size;
  uint8_t text;
};

enum service_msg_status_e {
  SERVICE_MSG_STATUS_LOADER  = 0x00,
  SERVICE_MSG_STATUS_UPDATER = 0x01,
  SERVICE_MSG_STATUS_FW      = 0x02,
  SERVICE_MSG_STATUS_FW2     = 0x03,
};

enum service_msg_type_e {
  SERVICE_MSG_RESPONSE              = 0x80,
  SERVICE_MSG_RESPONSE_ERROR        = 0x40,
  // 0x00 (msg type); 0x00 (MSB PktID); 0x0000 (STATE); 0x00000000(TIME sec); 0x0000 (VER BL); 0x0000 (VER FW); 0x00:00:00:00:00:00 (MAC); 0x00:00:00:00 (IP); 
  // STATUS 0x0000 - Loader
  //        0x0001 - Updater
  //        0x0002 - FW
  //        0x0003 - FW2
  SERVICE_MSG_GET_INFO              = 0x00,
  
  SERVICE_MSG_GET_PARAM             = 0x01,
  SERVICE_MSG_SET_PARAM             = 0x02,
  
  SERVICE_MSG_GET_INFO_BC           = 0x10,
  SERVICE_MSG_GET_PARAM_BC          = 0x11,
  SERVICE_MSG_SET_PARAM_BC          = 0x12,
  
  SERVICE_MSG_GET_TEXT_PARAM        = 0x21,
  SERVICE_MSG_SET_TEXT_PARAM        = 0x22,
  SERVICE_MSG_GET_TEXT_PARAM_BY_MAC = 0x23,
  SERVICE_MSG_SET_TEXT_PARAM_BY_MAC = 0x24,
  
  // 0x00 (msg type); 0x00 (MSB PktID); 0x00 (MSB Pkt Cnt); 0x00 (Pkt Size); TEXT REQUEST by Pkt Size
  SERVICE_MSG_TXT_REQUEST           = 0x23, 
  // 0x00 (msg type); 0x00 (MSB PktID); 0x00 (MSB Pkt Cnt); 0x00 (Pkt Size); TEXT RESPONSE by Pkt Size
  SERVICE_MSG_TXT_RESPONSE          = 0x24,
  // 0x00 (msg type); 0x00 (MSB PktID); 0x00 (MSB Pkt Cnt); 0x00 (Pkt Size); TEXT INDICATION by Pkt Size
  SERVICE_MSG_TXT_INDICATION        = 0x25,
  
  // 0x00 (msg type);
  SERVICE_MSG_REBOOT                = 0x30,
  
  SERVICE_MSG_GET_TEXT_PARAM_BC     = 0x31,
  SERVICE_MSG_SET_TEXT_PARAM_BC     = 0x32,
  SERVICE_MSG_GET_TEXT_PARAM_BY_MAC_BC = 0x33,
  SERVICE_MSG_SET_TEXT_PARAM_BY_MAC_BC = 0x34,
};

void service_init(void);

extern void __reboot__();

#endif //__SERVICE_H_


