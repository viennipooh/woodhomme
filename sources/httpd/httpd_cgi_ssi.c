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

#include "settings.h"
#include "info.h"
#ifdef USE_SWITCH
  #include "statistics.h"
#endif
#include "extio.h"

#include <string.h>
#include <stdlib.h>

#ifdef USE_STP
  #include "stp.h"
#endif

tSSIHandler ADC_Page_SSI_Handler;
uint32_t ADC_not_configured=1;

#ifdef USE_SWITCH
  static dmac_ctrl_t dmac;
#endif

/* we will use character "t" as tag for CGI */
char const* TAG_SYSTEM="system";
char const* TAGS[]={
  "work_time",
  "state",
  "system",
  "system_stat",
  "system_ports",
  "system_shdsl",
  "system_dmac",
  "system_smac",
  "system_network",
  "system_device_name",
  "system_serial_num",
  "system_hw_ver",
  "system_fw_ver",
  "system_up_time",
  "system_ip_addr",
  "system_mac_addr",
  "system_table_dmac",
  "system_table_smac",
  "system_table_vlan",
  "statistics",
  "file_list",  //Не используется
  "system_ports_alias",
  "system_ports_conn",
  "system_ports_en",
  "system_ports_auto",
  "system_ports_speed",
  "system_ports_duplex",
  "system_ports_cflw",
  "system_ports_mdi",
  "system_ports_fef",
  "system_ports_cflw",
  "system_shdsl_ms",
  "system_shdsl_speed",
  "system_shdsl_modul",
  "system_shdsl_sync",
  "system_shdsl_amp",
  "system_shdsl_annex",
  "system_power_modem",
  "system_power_rs485_1",
  "system_power_rs485_2",
  "system_power_sw1", //Отключен
  "system_power_sw2",
  "system_iconn_sw",
  "system_dmac_csv",

  "usart_regs", //Поллинг RS485: Регистры usart
  "rs_devaddr",
  "rs_modbusfunc",
  "rs_startreg",
  "rs_regnumber",
  "rs_pollperiod",
  "rs_pollchannel",
  "rs_rxerror",
  "rs_startpoll",
  "rs_stoppoll",
  "rs_txcount",
  "rs_errcount",
  //=== vlan
  //Общие для vlan
  "vl_vlan_on",
  "vl_uni_off",
  "vl_chgvid_on",
  //Порты
  "vl_port_ins",
  "vl_port_del",
  "vl_system_ports",
  //VLANs
  "vl_system_vlans",
  //RSTP
  "stp_system_rstp",
  "stp_ports",
  "system_vcc",
  //Температура кристалла
  "system_temp",
  "system_warn_temp",
  "system_fault_temp",
  "system_state_temp",

  "system_time",
  "tm_is_assigned",
  //Таблица событий
  "events_tab",
  "time_ev_count",
  "time_ev_pos",
  "time_corr_value",
  "time_corr_int",
  //Контроль кабеля
  "cable_link_md",
  "system_ping",
  //УТД-М
  "system_ant",
  "system_power_WiFi_1",
  "system_power_WiFi_2",
  //ИИП
  "sys_ap_name",
  "sys_ap_pass",
  "sys_esp_info",
  "sys_esp_ip_info",
  "sys_esp_ip_addr",
  "sys_esp_ip_port",
  "sys_esp_ap_state",
  "sys_conn_on_start",
};

enum {
  WORK_TIME = 0,
  STATE = 1,
  SYSTEM = 2,
  SYSTEM_STAT = 3,
  SYSTEM_PORTS = 4,
  SYSTEM_SHDSL = 5,
  SYSTEM_DMAC = 6,
  SYSTEM_SMAC = 7,
  SYSTEM_NETWORK = 8,
  SYSTEM_DEVICE_NAME = 9,
  SYSTEM_SERIAL_NUM = 10,
  SYSTEM_HW_VER = 11,
  SYSTEM_FW_VER = 12,
  SYSTEM_UP_TIME = 13,
  SYSTEM_IP_ADDR = 14,
  SYSTEM_MAC_ADDR = 15,
  SYSTEM_TABLE_DMAC = 16,
  SYSTEM_TABLE_SMAC = 17,
  SYSTEM_TABLE_VLAN = 18,
  STATISTICS = 19,
  TAG_FILE_LIST = 20,  //Не используется
  SYSTEM_POWER_MODEM   = 37,
  SYSTEM_POWER_RS485_1 = 38,
  SYSTEM_POWER_RS485_2 = 39,
  SYSTEM_POWER_SWITCH_1  = 40, //Отключен
  SYSTEM_POWER_SWITCH_2  = 41,
  SYSTEM_ICONN_SW  = 42,
  SYSTEM_DMAC_CSV  = 43,

  USART_REGS      = 44, //Поллинг RS485: Регистры usart
  //Уставки
  RS_DEVADDR      = 45,
  RS_MODBUSFUNC   = 46,
  RS_STARTREG     = 47,
  RS_REGNUMBER    = 48,
  RS_POLLPERIOD   = 49,
  RS_POLLCHANNEL  = 50,
  //Оперативные
  RS_RXERROR      = 51,
  RS_STARTPOLL    = 52,
  RS_STOPPOLL     = 53,
  RS_TXCOUNT      = 54,
  RS_ERRCOUNT     = 55,
  //=== vlan
  //Общие для vlan
  VL_VLAN_ON      = 56,
  VL_UNI_OFF      = 57,
  VL_CHGVID_ON    = 58,
  //Порты
  VL_PORT_INS     = 59,
  VL_PORT_DEL     = 60,
  VL_SYSTEM_PORTS = 61,
  //VLANs
  VL_SYSTEM_VLANS = 62,
  //=== RSTP
  STP_SYSTEM_RSTP = 63,
  STP_PORTS       = 64,
  SYSTEM_VCC      = 65,
  //Температура кристалла
  SYSTEM_TEMP     = 66,
  SYSTEM_WARN_TEMP  = 67,
  SYSTEM_FAULT_TEMP = 68,
  SYSTEM_STATE_TEMP = 69,

  SYSTEM_TIME     = 70,
  TIME_ASSIGNED   = 71,
  //Таблица событий
  EVENTS_TAB      = 72,
  TIME_EV_COUNT   = 73,
  TIME_EV_POS     = 74,
  TIME_CORR_VALUE = 75,
  TIME_CORR_INT   = 76,
  //Контроль кабеля
  CABLE_LINK_MD   = 77,
  SYSTEM_PING     = 78,
  //УТД-М
  SYSTEM_ANT      = 79,
  SYSTEM_POWER_WIFI_1 = 80,
  SYSTEM_POWER_WIFI_2 = 81,
  //ИИП
  SYS_AP_NAME = 82,
  SYS_AP_PASS = 83,
  SYS_ESP_INFO = 84,
  SYS_ESP_IP_INFO = 85,
  SYS_ESP_IP_ADDR = 86,
  SYS_ESP_IP_PORT = 87,
  SYS_ESP_AP_STATE = 88,
  SYS_CONN_ON_START = 89,
};


enum {
  CGI_SYSTEM = 0,
  CGI_SYSTEM_TABLE_DMAC = 1,
  CGI_SYSTEM_TABLE_SMAC = 2,
  CGI_SYSTEM_TABLE_VLAN = 3,
  CGI_SETTINGS = 4,
  CGI_STATISTICS = 5,
  CGI_SYSTEM_STAT = 6,
  CGI_SYSTEM_PORT = 7,
  CGI_SYSTEM_IP = 8,
  CGI_SYSTEM_RESET_SAVE = 9,
  CGI_SYSTEM_DMAC = 10,
  CGI_SYSTEM_BOOT = 11,
  CGI_SYSTEM_PORTRS = 12,		// найтсрйоки портов
  CGI_SYSTEM_PORTIP = 13,
  CGI_SYSTEM_PORTPOLL = 14,
  CGI_SYSTEM_FRAMEPOLL = 15,
  CGI_SYSTEM_PWR  = 16,
  CGI_SYSTEM_PASS = 17,
  CGI_SYSTEM_VLAN = 18,
  CGI_SYSTEM_RSTP = 19,
  CGI_SYSTEM_TIME = 20,
  CGI_SYSTEM_PING = 21,
  CGI_SYSTEM_EVENTS = 22,
  CGI_SYSTEM_EVENT_TAB = 23,
  CGI_SYSTEM_TEMP = 24,
  CGI_SYSTEM_CONV_WIFI = 25,
};


/* CGI handler for LED control */
const char * SYSTEM_CGI_Handler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]);
const char * SYSTEM_TABLE_CGI_Handler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]);
const char * SYSTEM_SETTINGS_CGI_Handler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]);
const char * SYSTEM_STAT_CGI_Handler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]);
const char * SYSTEM_PORT_CGI_Handler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]);
const char * SYSTEM_IP_CGI_Handler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]);
const char * SYSTEM_SAVE_RESTART_CGI_Handler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]);
const char * SYSTEM_DMAC_CGI_Handler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]);
const char * SYSTEM_BOOT_CGI_Handler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]);
const char * SYSTEM_PORTRS_CGI_Handler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]);
const char * SYSTEM_PORTIP_CGI_Handler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]);
const char * SYSTEM_PORTPOLL_CGI_Handler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]);
const char * SYSTEM_FRAMEPOLL_CGI_Handler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]);
const char * SYSTEM_PWR_CGI_Handler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]);
const char * SYSTEM_PASS_CGI_Handler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]);
const char * SYSTEM_VLAN_CGI_Handler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]);
const char * SYSTEM_RSTP_CGI_Handler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]);
const char * SYSTEM_TIME_CGI_Handler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]);
const char * SYSTEM_PING_CGI_Handler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]);
const char * SYSTEM_EVENTS_CGI_Handler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]);
const char * SYSTEM_EVENT_TAB_CGI_Handler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]);
const char * SYSTEM_TEMP_CGI_Handler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]);
#if ((UTD_M != 0) || (IIP != 0))
const char * SYSTEM_CONV_WIFI_CGI_Handler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]);
#endif

/* Html request for "/leds.cgi" will start LEDS_CGI_Handler */

/* Cgi call table, only one CGI used */
tCGI CGI_TAB[] = {
  {"/cgi/system.cgi", SYSTEM_CGI_Handler},
  {"/cgi/table_dmac.cgi", SYSTEM_TABLE_CGI_Handler},
  {"/cgi/table_smac.cgi", SYSTEM_TABLE_CGI_Handler},
  {"/cgi/table_vlan.cgi", SYSTEM_TABLE_CGI_Handler},
  {"/cgi/settings.cgi", SYSTEM_SETTINGS_CGI_Handler},
  {"/cgi/statistics.cgi", SYSTEM_CGI_Handler},
  {"/stat.htm", SYSTEM_STAT_CGI_Handler},
  {"/port.htm", SYSTEM_PORT_CGI_Handler},
  {"/ip.htm", SYSTEM_IP_CGI_Handler},
  {"/save_restart.htm",SYSTEM_SAVE_RESTART_CGI_Handler},
  {"/dmac.htm",SYSTEM_DMAC_CGI_Handler},
  {"/boot.cgi",SYSTEM_BOOT_CGI_Handler},
  {"/portrs.htm",SYSTEM_PORTRS_CGI_Handler},
  {"/portip.htm",SYSTEM_PORTIP_CGI_Handler},
  {"/portpoll.htm",SYSTEM_PORTPOLL_CGI_Handler},
  {"/framepoll.htm",SYSTEM_FRAMEPOLL_CGI_Handler},
  {"/portpwr.htm",SYSTEM_PWR_CGI_Handler},
  {"/pass.htm",SYSTEM_PASS_CGI_Handler},
  {"/vlan.htm",SYSTEM_VLAN_CGI_Handler},
  {"/stp.htm",SYSTEM_RSTP_CGI_Handler},
  {"/time.htm",SYSTEM_TIME_CGI_Handler},
  {"/ping.htm",SYSTEM_PING_CGI_Handler},
  {"/events.htm",SYSTEM_EVENTS_CGI_Handler},
  {"/event_tab.htm",SYSTEM_EVENT_TAB_CGI_Handler},
  {"/temp.htm",SYSTEM_TEMP_CGI_Handler},
#if ((UTD_M != 0) || (IIP != 0))
  {"/conv_wifi.htm",SYSTEM_CONV_WIFI_CGI_Handler},
#endif
};

const char * SYSTEM_PWR_CGI_Handler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[])
{
  uint8_t i;
  int value;
  switch(iIndex){
    case CGI_SYSTEM_PWR:
      {
        if(iNumParams)
        {
          rsettings->modempwr = 0;
        }
        for (i=0; i<iNumParams; i++)
        {
          if (strcmp(pcParam[i] , "modempwr") == 0)
          {
            sscanf(pcValue[i],"%d",&value);
            rsettings->modempwr = value;
          } else if (strcmp(pcParam[i] , "swpwr") == 0)
          {
            sscanf(pcValue[i],"%d",&value);
            rsettings->swpwr = value;
          }
        }
      }
      break;
  }
  return "/portpwr.htm";
}

extern void __reboot__();
const char * SYSTEM_BOOT_CGI_Handler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[])
{
//  dbbkpsram.set("BOOTLAODER","1");
  __reboot__();
  return "/index.htm";
}

const char * SYSTEM_DMAC_CGI_Handler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[])
{
  return "/dmac.htm";
}

#include "log.h"
extern void LoadSettingsSerial_ByIdx(int iSettingIdx);
void LoadSettingsForSerials() {
  LoadSettingsSerial_ByIdx(0);
  LoadSettingsSerial_ByIdx(1);
}

const char * SYSTEM_SAVE_RESTART_CGI_Handler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[])
{
  uint8_t i;
  switch(iIndex){
    case CGI_SYSTEM_RESET_SAVE:
      {
        for (i=0; i<iNumParams; i++)
        {
          if (strcmp(pcParam[i] , "save_refresh") == 0) {
            settings_save();
            LoadSettingsForSerials(); //= Обновление конфигурации RS485
          }
          if (strcmp(pcParam[i] , "save_restart") == 0) {
            settings_save();
            SaveEvent(0, etCount, ecCount); //Спец. событие: Переход на загрузчик
          }
          if (strcmp(pcParam[i] , "restart") == 0) {
            SaveEvent(0, etCount, ecCount); //Спец. событие: Переход на загрузчик
          }
          if (strcmp(pcParam[i] , "reset_settings") == 0) {
            settings_default(false);
            SaveEvent(0, etCount, ecCount); //Спец. событие: Переход на загрузчик
          }
        }
        return "/save_restart.htm";
      }
      break;
  }
  return "/save_restart.htm";
}

/****     СИСТЕМНЫЕ НАСТРОЙКИ ДОСТУПА    ****/
const char * SYSTEM_PASS_CGI_Handler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[])
{
  const char *login    = "administrator";
  const char *password = "4ZCGG6Nu";
  char new_pass[SETTINGS_MAX_PASS_LEN],new_login[SETTINGS_MAX_USER_LEN];
  uint8_t i, check = 0x00;
  memset(new_pass,0,SETTINGS_MAX_PASS_LEN);
  memset(new_login,0,SETTINGS_MAX_USER_LEN);
  switch(iIndex){
    case CGI_SYSTEM_PASS:
      {
        if(iNumParams>0)
        {
          for (i=0; i<iNumParams; i++)
          {
            if (strcmp(pcParam[i] , "login") == 0)
            {
              check |= 0x01;
              if ( strcmp(pcValue[i],login) == 0 )
                  check |= 0x10;
              if ( strcmp(pcValue[i],rsettings->user) == 0 )
                  check |= 0x10;
            } else if (strcmp(pcParam[i] , "pass") == 0) {
              check |= 0x02;
              if ( strcmp(pcValue[i],password) == 0 )
                  check |= 0x20;
              if ( strcmp(pcValue[i],rsettings->password) == 0 )
                  check |= 0x20;
            }
          }
          for (i=0; i<iNumParams; i++)
          {
            if ( strcmp(pcParam[i] , "newlogin") == 0) {
              if(strlen(pcValue[i])>=4 && strlen(pcValue[i])<=SETTINGS_MAX_USER_LEN)
                strcpy(new_login, pcValue[i]);
              else
                check &= ~0x30;
            } else if ( (strcmp(pcParam[i] , "newpass2") == 0) ||
                        (strcmp(pcParam[i] , "newpass")  == 0) ) {

              if(new_pass[0] == 0)
              {
                if( strlen(pcValue[i])>=4 && strlen(pcValue[i])<=SETTINGS_MAX_PASS_LEN )
                  strcpy(new_pass, pcValue[i]);
                else
                  check &= ~0x30;
              }
              else
              {
                if( strcmp(new_pass, pcValue[i]) == 0 )
                  check |= 0x80;
                else
                  check &= ~0x30;
              }
            }
          }
        }
      }
      break;
  }
  if( (check & 0x33) == 0x33)
  {
    if(check & 0x80)
    {
      strcpy(rsettings->password, new_pass);
      strcpy(rsettings->user, new_login);
      return "/pass_ok.htm";
    }
    else
    {
      return "/pass_err.htm";
    }
  }
  else if( (check & 0x33) > 0)
  {
    return "/pass_err.htm";
  }
  else
  {
    return "/pass.htm";
  }
}

#ifndef NO_USE_VLANS
  #include "vlan.h"
#endif
const char * SYSTEM_IP_CGI_Handler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[])
{
  uint8_t i;
  switch(iIndex){
    case CGI_SYSTEM_IP:
      {
        if(iNumParams>0)
        {
          for (i=0; i<iNumParams; i++)
          {
            if (strcmp(pcParam[i] , "ip") == 0)
            {
              int ip[4];
              sscanf(pcValue[i],"%d.%d.%d.%d",&ip[0],&ip[1],&ip[2],&ip[3]);
              rsettings->ip.addr.addr = (ip[0] | ip[1]<<8 | ip[2]<<16 | ip[3]<<24);
            } else if (strcmp(pcParam[i] , "netmask") == 0) {
              int ip[4];
              sscanf(pcValue[i],"%d.%d.%d.%d",&ip[0],&ip[1],&ip[2],&ip[3]);
              rsettings->ip.mask.addr = (ip[0] | ip[1]<<8 | ip[2]<<16 | ip[3]<<24);
            } else if (strcmp(pcParam[i] , "gateway") == 0) {
              int ip[4];
              sscanf(pcValue[i],"%d.%d.%d.%d",&ip[0],&ip[1],&ip[2],&ip[3]);
              rsettings->ip.gw.addr = (ip[0] | ip[1]<<8 | ip[2]<<16 | ip[3]<<24);
            } else if (strcmp(pcParam[i] , "mac") == 0) {
              uint32_t mac[6];
              if(pcValue[i][2]=='%')
              {
                sscanf(pcValue[i],"%x%%3A%x%%3A%x%%3A%x%%3A%x%%3A%x",
                       &mac[0],&mac[1],&mac[2],&mac[3],&mac[4],&mac[5]);
              }
              else if(pcValue[i][2]==':')
              {
                sscanf(pcValue[i],"%x:%x:%x:%x:%x:%x",
                       &mac[0],&mac[1],&mac[2],&mac[3],&mac[4],&mac[5]);
              }
              else if(pcValue[i][2]=='-')
              {
                sscanf(pcValue[i],"%x-%x-%x-%x-%x-%x",
                       &mac[0],&mac[1],&mac[2],&mac[3],&mac[4],&mac[5]);
              }
              for(int j=0;j<6;j++)
              {
                rsettings->mac[j] = (char)mac[j];
              }
            } else if (strcmp(pcParam[i] , "ipconfig") == 0) {
#if (MKPSH10 != 0)
            } else if (strcmp(pcParam[i] , "ipmode") == 0) {
              int ipmode;
              sscanf(pcValue[i], "%d", &ipmode);
              //Если не равно тому что в settings, то:
              if (rsettings->DevMode != (enum eDevMode)ipmode) {
                //- Записать в settings
                rsettings->DevMode = (enum eDevMode)ipmode;
                settings_save();
                //Установить соответствующий режим
                SetDevMode(rsettings->DevMode);
              }
#endif
            } else {
              //
            }
          }
        }
      }
      break;
  }
  return "/ip.htm";
}

static int get_portnum_from_char(char ch)
{
  char temp = '1';
  if(ch >= '1' && ch <= '7')
  {
    return (int)(ch - temp);
  }
  return -1;
}

int ItemNum(char ch) {
  return get_portnum_from_char(ch);
}

const char * SYSTEM_PORTRS_CGI_Handler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[])
{
  uint8_t i;
  int value;
  bool cApplying = false;
  switch(iIndex){
    case CGI_SYSTEM_PORTRS:
      {
        if(iNumParams>0)
        {
          for (i=0; i<iNumParams; i++)
          {
            if (strcmp(pcParam[i] , "applying") == 0) {
              sscanf(pcValue[i],"%d",&value);
              if (value)
                cApplying = true;
            }
          }
          if (!cApplying)
            break;
          for (i=0; i<iNumParams; i++)
          {
            if (strncmp(pcParam[i] , "rs_baudrate_", 12) == 0)
            {
              sscanf(pcValue[i],"%d",&value);

              int cIdx = get_portnum_from_char(pcParam[i][12]);
              int cValue = value;
              if (rsettings->uart[cIdx].baudrate != cValue) {
                SaveSettingEvent(etSetting, ecParamChangePort, (enum ePortName)(cIdx + 10), snSpeed, cValue);
                rsettings->uart[cIdx].baudrate = cValue;
              }
            } else if (strncmp(pcParam[i] , "rs_databits_", 12) == 0) {
              sscanf(pcValue[i],"%d",&value);
              rsettings->uart[get_portnum_from_char(pcParam[i][12])].
                databits = value;
            } else if (strncmp(pcParam[i] , "rs_stopbits_", 12) == 0) {
              sscanf(pcValue[i],"%d",&value);
              rsettings->uart[get_portnum_from_char(pcParam[i][12])].
                stopbits = (enum settings_uart_stopbits)value;
            } else if (strncmp(pcParam[i] , "rs_parity_", 10) == 0) {
              if(strcmp(pcValue[i],"none") == 0)
              {
                rsettings->uart[get_portnum_from_char(pcParam[i][10])].
                  parity = SETTINGS_UART_PARITY_NOT;
              }
              else if(strcmp(pcValue[i],"odd") == 0)
              {
                rsettings->uart[get_portnum_from_char(pcParam[i][10])].
                  parity = SETTINGS_UART_PARITY_ODD;
              }
              else if(strcmp(pcValue[i],"even") == 0)
              {
                rsettings->uart[get_portnum_from_char(pcParam[i][10])].
                  parity = SETTINGS_UART_PARITY_EVEN;
              }
            } else if (strncmp(pcParam[i] , "rs_fctrl_", 4) == 0) {
            } else {
            }
          }
          settings_save();
          LoadSettingsForSerials(); //= Обновление конфигурации RS485
        }
      }
      break;
  }
  return "/portrs.htm";
}

#include "sockets.h"
#include "mode.h"

extern struct mode_struct_s * GetMode(int iModeIdx);

void SocketRestart(struct mode_struct_s *ist) {
  ist->state = MODE_STATE_RESTART;
}

extern void LoadSettingsSerialIP_ByIdx(int iSettingIdx);
void LoadSettingsForSerialsIP() {
  for (int i = 0; i < RS_PORTS_NUMBER; ++i)
    LoadSettingsSerialIP_ByIdx(i);
  for (int i = 0; i < RS_PORTS_NUMBER; ++i)
    SocketRestart(GetMode(1));
}

const char * SYSTEM_PORTIP_CGI_Handler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[])
{
  uint8_t i;
  int value;
  bool cApplying = false;
  bool cChanged = false;
  bool cModeChanged = false;
  switch(iIndex){
    case CGI_SYSTEM_PORTIP:
      {
        if(iNumParams>0)
        {
          for (i=0; i<iNumParams; i++)
          {
            if (strcmp(pcParam[i] , "applying") == 0) {
              sscanf(pcValue[i],"%d",&value);
              if (value)
                cApplying = true;
            }
          }
          if (!cApplying)
            break;
          for (i=0; i<iNumParams; i++)
          {
            if (strncmp(pcParam[i] , "ip_mode_", 8) == 0)
            {
              int8_t cPortNum = get_portnum_from_char(pcParam[i][8]);
              if(strcmp(pcValue[i],"modbusgw") == 0)
              {
                if (rsettings->ip_port[cPortNum].type != SETTINGS_IP_PORT_MODBUS_GW) {
                  cChanged = true;
                  cModeChanged = true;
                }
                rsettings->ip_port[cPortNum].type = SETTINGS_IP_PORT_MODBUS_GW;
              }
              else if(strcmp(pcValue[i],"modbus_rtu_over_tcp") == 0)
              {
                if (rsettings->ip_port[cPortNum].type != SETTINGS_IP_PORT_MODBUS_RTU_OVER_TCP) {
                  cChanged = true;
                  cModeChanged = true;
                }
                rsettings->ip_port[cPortNum].type = SETTINGS_IP_PORT_MODBUS_RTU_OVER_TCP;
              }
              else if(strcmp(pcValue[i],"modbus_gw_udp") == 0)
              {
                if (rsettings->ip_port[cPortNum].type != SETTINGS_IP_PORT_MODBUS_GW_UDP) {
                  cChanged = true;
                  cModeChanged = true;
                }
                rsettings->ip_port[cPortNum].type = SETTINGS_IP_PORT_MODBUS_GW_UDP;
              }
              else if(strcmp(pcValue[i],"modbus_rtu_over_udp") == 0)
              {
                if (rsettings->ip_port[cPortNum].type != SETTINGS_IP_PORT_MODBUS_RTU_OVER_UDP) {
                  cChanged = true;
                  cModeChanged = true;
                }
                rsettings->ip_port[cPortNum].type = SETTINGS_IP_PORT_MODBUS_RTU_OVER_UDP;
              }
              else if(strcmp(pcValue[i],"tcpclient") == 0)
              {
                if (rsettings->ip_port[cPortNum].type != SETTINGS_IP_PORT_TCP_CLIENT) {
                  cChanged = true;
                  cModeChanged = true;
                }
                rsettings->ip_port[cPortNum].type = SETTINGS_IP_PORT_TCP_CLIENT;
              }
            } else if (strncmp(pcParam[i] , "ip_addr_", 8) == 0) {
              int ip[4];
              sscanf(pcValue[i],"%d.%d.%d.%d",&ip[0],&ip[1],&ip[2],&ip[3]);
              uint32_t cAddr = (ip[0] | ip[1]<<8 | ip[2]<<16 | ip[3]<<24);
              if (rsettings->ip_port[get_portnum_from_char(pcParam[i][8])].ip.addr != cAddr) {
                cChanged = true;
              }
              rsettings->ip_port[get_portnum_from_char(pcParam[i][8])].ip.addr = cAddr;
            } else if (strncmp(pcParam[i] , "ip_port_local_", 14) == 0) {
              sscanf(pcValue[i],"%d",&value);
              if (rsettings->ip_port[get_portnum_from_char(pcParam[i][14])].
                  port_local != value) {
                cChanged = true;
              }
              rsettings->ip_port[get_portnum_from_char(pcParam[i][14])].
                port_local = value;
            } else if (strncmp(pcParam[i] , "ip_port_conn_", 13) == 0) {
              sscanf(pcValue[i],"%d",&value);
              if (rsettings->ip_port[get_portnum_from_char(pcParam[i][13])].
                  port_conn != value) {
                cChanged = true;
              }
              rsettings->ip_port[get_portnum_from_char(pcParam[i][13])].
                port_conn = value;
            } else if (strncmp(pcParam[i] , "rs_fctrl_", 4) == 0) {
            } else {
              //
            }
          }
          if (cChanged) {
            settings_save();
            LoadSettingsForSerialsIP();
            if (cModeChanged) {
              vTaskDelay(500);
              SaveEvent(0, etCount, ecCount); //Спец. событие: Переход на загрузчик
            }
          }
        }
      }
      break;
  }
  return "/portip.htm";
}

bool gApplying = false, gStoping = false;
int gTxCount = 0, gErrCount = 0;
extern int pack_regs_rtu(struct sMBParam * ipMBParam);

//Параметры поллинга по умолчанию:
struct sMBParam * GetPollMBParam();
extern void PackAndSendRtuFrame(struct mode_struct_s *st);

const char * SYSTEM_PORTPOLL_CGI_Handler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[])
{ //
  uint8_t i;
  int value;
  bool cApplying = false, cStoping = false;
  struct sMBParam * cpMBParam = GetPollMBParam();
  struct mode_struct_s *st = GetMode(cpMBParam->Chan - 1);
  switch(iIndex){
    case CGI_SYSTEM_PORTPOLL:
      {
        if(iNumParams>0)
        {
          for (i=0; i<iNumParams; i++)
          {
            if (strcmp(pcParam[i] , "startpoll") == 0) {
              sscanf(pcValue[i],"%d",&value);
              if (value)
                cApplying = true;
            }
            if (strcmp(pcParam[i] , "for_change") == 0) {
              sscanf(pcValue[i],"%d",&value);
              if (value)
                cStoping = true;
            }
            if (strncmp(pcParam[i] , "rs_tx_count", 11) == 0) {
              sscanf(pcValue[i],"%d",&value);
              gTxCount = value;
            }
            if (strncmp(pcParam[i] , "rs_err_count", 12) == 0) {
              sscanf(pcValue[i],"%d",&value);
              gErrCount = value;
            }
          }
          gApplying = cApplying;
          gStoping = cStoping;
          if (gApplying) {
            bool cChanged = false;
            for (i=0; i<iNumParams; i++)
            {
              if (strncmp(pcParam[i] , "rs_dev_addr_1", 13) == 0)
              {
                sscanf(pcValue[i],"%d",&value);
                if (value < 0)
                  value = 0;
                if (value > 254)
                  value = 254;
                if (cpMBParam->Addr != value) {
                  cpMBParam->Addr = value;
                  cChanged = true;
                }
              } else if (strncmp(pcParam[i] , "rs_modbus_func_1", 15) == 0) {
                sscanf(pcValue[i],"%d",&value);
                if (cpMBParam->Func != value) {
                  cpMBParam->Func = value;
                  cChanged = true;
                }
              } else if (strncmp(pcParam[i] , "rs_start_reg_1", 12) == 0) {
                sscanf(pcValue[i],"%d",&value);
                if (value < 0)
                  value = 0;
                if (value > 65635)
                  value = 65635;
                if (cpMBParam->Start != value) {
                  cpMBParam->Start = value;
                  cChanged = true;
                }
              } else if (strncmp(pcParam[i] , "rs_reg_number_1", 14) == 0) {
                sscanf(pcValue[i],"%d",&value);
                if (value < 1)
                  value = 1;
                if (value > 65635)
                  value = 65635;
                if (cpMBParam->Number != value) {
                  cpMBParam->Number = value;
                  cChanged = true;
                }
              } else if (strncmp(pcParam[i] , "rs_poll_period_1", 15) == 0) {
                sscanf(pcValue[i],"%d",&value);
                if (value < 500)
                  value = 500;
                if (value > 10000)
                  value = 10000;
                if (cpMBParam->Period != value) {
                  cpMBParam->Period = value;
                  cChanged = true;
                }
              } else if (strncmp(pcParam[i] , "rs_poll_channel_1", 16) == 0) {
                sscanf(pcValue[i],"%d",&value);
                if (cpMBParam->Chan != value) {
                  cpMBParam->Chan = value;
                  cChanged = true;
                }
              }
            }
            if (cChanged) {
            }
            cpMBParam->Error = mbrNone;
          } else { //Останов
          }
        }
      }
      break;
  }
  return "/portpoll.htm";
}

//#include "main.h"
//const char * SYSTEM_MOVE_CGI_Handler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[])
//{ //
//  uint8_t i;
//  int value;
//  bool cApplying = false, cStoping = false;
//
////	struct SDrive_s gDriveParam = {
////  .time   = 200,      //Канал RS485
////  .drive_ev   = STOP,      //Адрес
////};
//
//  switch(iIndex){
//    case CGI_SYSTEM_PORTPOLL:
//      {
//        if(iNumParams>0)
//        {
//          for (i=0; i<iNumParams; i++)
//          {
//            if (strcmp(pcParam[i] , "startpoll") == 0) {
//              sscanf(pcValue[i],"%d",&value);
//              if (value)
//                cApplying = true;
//            }
//            if (strcmp(pcParam[i] , "for_change") == 0) {
//              sscanf(pcValue[i],"%d",&value);
//              if (value)
//                cStoping = true;
//            }
//            if (strncmp(pcParam[i] , "rs_tx_count", 11) == 0) {
//              sscanf(pcValue[i],"%d",&value);
//              gTxCount = value;
//            }
//            if (strncmp(pcParam[i] , "rs_err_count", 12) == 0) {
//              sscanf(pcValue[i],"%d",&value);
//              gErrCount = value;
//            }
//          }
//          gApplying = cApplying;
//          gStoping = cStoping;
//          if (gApplying) {
//            bool cChanged = false;
//            for (i=0; i<iNumParams; i++)
//            {
//              if (strncmp(pcParam[i] , "rs_dev_addr_1", 13) == 0)
//              {
//                sscanf(pcValue[i],"%d",&value);
//                if (value < 0)
//                  value = 0;
//                if (value > 254)
//                  value = 254;
//                if (cpMBParam->Addr != value) {
//                  cpMBParam->Addr = value;
//                  cChanged = true;
//                }
//              } else if (strncmp(pcParam[i] , "rs_modbus_func_1", 15) == 0) {
//                sscanf(pcValue[i],"%d",&value);
//                if (cpMBParam->Func != value) {
//                  cpMBParam->Func = value;
//                  cChanged = true;
//                }
//              } else if (strncmp(pcParam[i] , "rs_start_reg_1", 12) == 0) {
//                sscanf(pcValue[i],"%d",&value);
//                if (value < 0)
//                  value = 0;
//                if (value > 65635)
//                  value = 65635;
//                if (cpMBParam->Start != value) {
//                  cpMBParam->Start = value;
//                  cChanged = true;
//                }
//              } else if (strncmp(pcParam[i] , "rs_reg_number_1", 14) == 0) {
//                sscanf(pcValue[i],"%d",&value);
//                if (value < 1)
//                  value = 1;
//                if (value > 65635)
//                  value = 65635;
//                if (cpMBParam->Number != value) {
//                  cpMBParam->Number = value;
//                  cChanged = true;
//                }
//              } else if (strncmp(pcParam[i] , "rs_poll_period_1", 15) == 0) {
//                sscanf(pcValue[i],"%d",&value);
//                if (value < 500)
//                  value = 500;
//                if (value > 10000)
//                  value = 10000;
//                if (cpMBParam->Period != value) {
//                  cpMBParam->Period = value;
//                  cChanged = true;
//                }
//              } else if (strncmp(pcParam[i] , "rs_poll_channel_1", 16) == 0) {
//                sscanf(pcValue[i],"%d",&value);
//                if (cpMBParam->Chan != value) {
//                  cpMBParam->Chan = value;
//                  cChanged = true;
//                }
//              }
//            }
//            if (cChanged) {
//            }
//            cpMBParam->Error = mbrNone;
//          } else { //Останов
//          }
//        }
//      }
//      break;
//  }
//  return "/portpoll.htm";
//}


const char * SYSTEM_FRAMEPOLL_CGI_Handler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[])
{ //
  struct sMBParam * cpMBParam = GetPollMBParam();
  struct mode_struct_s *st = GetMode(cpMBParam->Chan - 1);
  switch(iIndex){
    case CGI_SYSTEM_FRAMEPOLL:
      {
        if (gApplying) {
          PackAndSendRtuFrame(st);
        }
      }
      break;
  }
  return "/framepoll.htm";
}

#define VLAN_USED_TAB_SIZE (3)

uint8_t status_sw[7][3]; //Состояния портов свитча (фактические)

const char * SYSTEM_PORT_CGI_Handler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[])
{
  uint8_t i;
	bool cApplying ;
  switch(iIndex){
    case CGI_SYSTEM_PORT:
      {
        if(iNumParams>0)
        {
          struct switch_settings_s sw;
          memcpy((void *)&sw, (void *)&rsettings->sw, sizeof(struct switch_settings_s));
          bool cPortChanged = false;
          bool cPortEnableChanged = false;
#ifdef USE_SUBNETS
          bool cChanged = false;
          struct sVlanCfgDesc * cpVlanCfgDesc = &rsettings->VlanCfgDesc;
#endif
          for (i=0; i<iNumParams; i++)
          {
						int value;

							if (strcmp(pcParam[i], "move_up") == 0)
								{
										gDriveParam.drive_ev = UP;
										sscanf(pcValue[i], "%d", &value);
										if (value)
										{
												// Действие для "вверх"
												cApplying = true;
										}
								}
							else if (strcmp(pcParam[i], "move_down") == 0)
							{
									gDriveParam.drive_ev = DOWN;
									sscanf(pcValue[i], "%d", &value);
									if (value)
									{
											// Действие для "вниз"
											cApplying = true;
									}
							}
							else if (strcmp(pcParam[i], "move_left") == 0)
							{
									gDriveParam.drive_ev = LEFT;
									sscanf(pcValue[i], "%d", &value);
									if (value)
									{
											// Действие для "влево"
											cApplying = true;
									}
							}
							else if (strcmp(pcParam[i], "move_righ") == 0)
							{
									gDriveParam.drive_ev = RIGHT;
									sscanf(pcValue[i], "%d", &value);
									if (value)
									{
											// Действие для "вправо"
											cApplying = true;
									}
							}
							else if (strcmp(pcParam[i], "stop") == 0)
							{
									gDriveParam.autoMove = false;
									sscanf(pcValue[i], "%d", &value);
									if (value)
									{
											// Действие для "стоп"
											cApplying = true;
									}
							}
							else if (strcmp(pcParam[i], "start") == 0)
							{
									gDriveParam.autoMove = true;
									sscanf(pcValue[i], "%d", &value);
									if (value)
									{
											// Действие для "стоп"
											cApplying = true;
									}
							}
            if (strncmp(pcParam[i] , "palias_", 7) == 0)
            {
              if(strlen(pcValue[i]) < SETTINGS_MAX_PORT_NAME)
              {
                strcpy(sw.ports[get_portnum_from_char(pcParam[i][7])].alias,pcValue[i]);
              }
            } else if (strncmp(pcParam[i] , "pen_", 4) == 0) {
              sscanf(pcValue[i],"%d",&value);
              int cIdx = get_portnum_from_char(pcParam[i][4]);
              int cValue = (value>0)?1:0;
              if (sw.ports[cIdx].port_enabled != cValue) {
                SaveSettingEvent(etSetting, ecParamChangePort, (enum ePortName)cIdx, snIsOn, cValue);
                sw.ports[cIdx].port_enabled = cValue;
                bool cChecked = (cValue > 0);
                ksz8895fmq_t *psw;
                ksz8895fmq_port_t port;
                extern ksz8895fmq_t * SelectSwitch_Port(uint8_t _port, ksz8895fmq_port_t * oPort);
                extern ksz8895fmq_t sw[];
                psw = SelectSwitch_Port(cIdx, &port);
                if ((psw == &sw[0]) && (rsettings->aRstpBridgeDesc[0].aBPortDesc[port].portEnabled != cChecked)) {
                  rsettings->aRstpBridgeDesc[0].aBPortDesc[port].portEnabled = cChecked;
                  cPortEnableChanged = true;
                }
                cPortChanged = true;
              }
            } else if (strncmp(pcParam[i] , "pauto_", 6) == 0) {
              sscanf(pcValue[i],"%d",&value);
              int cIdx = get_portnum_from_char(pcParam[i][6]);
              switch_autonegotiation_t cValue = (value>0)?SWITCH_AUTO_ENABLE:SWITCH_AUTO_DISABLE;
              if (sw.ports[cIdx].autonegotiation != cValue) {
                SaveSettingEvent(etSetting, ecParamChangePort, (enum ePortName)cIdx, snSpeedAuto, cValue);
                sw.ports[cIdx].autonegotiation = cValue;
                cPortChanged = true;
              }
            } else if (strncmp(pcParam[i] , "pspeed_", 7) == 0) {
              sscanf(pcValue[i],"%d",&value);
              int cIdx = get_portnum_from_char(pcParam[i][7]);
              switch_speed_t cValue = (value>0)?SWITCH_SPEED_100:SWITCH_SPEED_10;
              if (sw.ports[cIdx].speed != cValue) {
                if (!sw.ports[cIdx].autonegotiation) {
                  SaveSettingEvent(etSetting, ecParamChangePort, (enum ePortName)cIdx, snSpeed, cValue);
                  sw.ports[cIdx].speed = cValue;
                  cPortChanged = true;
                }
              }
            } else if (strncmp(pcParam[i] , "pduplex_", 8) == 0) {
              sscanf(pcValue[i],"%d",&value);
              int cIdx = get_portnum_from_char(pcParam[i][8]);
              switch_duplex_t cValue = (value>0)?SWITCH_FD:SWITCH_HD;
              if (sw.ports[cIdx].full_duplex != cValue) {
                if (!sw.ports[cIdx].autonegotiation) {
                  SaveSettingEvent(etSetting, ecParamChangePort, (enum ePortName)cIdx, snDuplex, cValue);
                  sw.ports[cIdx].full_duplex = cValue;
                  cPortChanged = true;
                }
              }
            } else if (strncmp(pcParam[i] , "pcflw_", 6) == 0) {
              sscanf(pcValue[i],"%d",&value);
              int cIdx = get_portnum_from_char(pcParam[i][6]);
              switch_flow_control_t cValue = (value>0)?SWITCH_ENABLE_FLOW_CONTROL:SWITCH_DISABLE_FLOW_CONTROL;
              if (sw.ports[cIdx].flow_control != cValue) {
                if (!sw.ports[cIdx].autonegotiation) {
                  SaveSettingEvent(etSetting, ecParamChangePort, (enum ePortName)cIdx, snFlowCtrl, cValue);
                  sw.ports[cIdx].flow_control = cValue;
                  cPortChanged = true;
                }
              }
            } else if (strncmp(pcParam[i] , "pamdi_", 6) == 0) {
              sscanf(pcValue[i],"%d",&value);
              int cIdx = get_portnum_from_char(pcParam[i][6]);
              switch_mdi_t cValue = (value>0)?SWITCH_MDIX:SWITCH_AUTO;
              if (sw.ports[cIdx].mdi != cValue) {
                SaveSettingEvent(etSetting, ecParamChangePort, (enum ePortName)cIdx, snMdiMdx, cValue);
                sw.ports[cIdx].mdi =  cValue;
                cPortChanged = true;
              }
            } else if (strncmp(pcParam[i] , "pmdi_", 5) == 0) {
              sscanf(pcValue[i],"%d",&value);
              int cIdx = get_portnum_from_char(pcParam[i][5]);
              switch_mdi_t cValue = (value>0)?SWITCH_MDI:SWITCH_AUTO;
              if (sw.ports[cIdx].mdi != cValue) {
                SaveSettingEvent(etSetting, ecParamChangePort, (enum ePortName)cIdx, snMdiMdx, cValue);
                sw.ports[cIdx].mdi =  cValue;
                cPortChanged = true;
              }
#ifdef USE_SUBNETS
            } else if (strncmp(pcParam[i] , "subnet_member_", 14) == 0) {
              sscanf(pcValue[i],"%d",&value);
              int cItemNum = ItemNum(pcParam[i][14]);
              if ((cItemNum >= 0) && (cItemNum < VLAN_USED_TAB_SIZE)) {
                if (cpVlanCfgDesc->aVlanItem[cItemNum].Members != value) {
                  cpVlanCfgDesc->aVlanItem[cItemNum].Members = value;
                  cChanged = true;
                }
              }
#endif
            } else {
              //
            }
          }
#ifdef USE_SUBNETS
          if (cChanged) { //Изменилось состояние подсетей
            settings_save();
            SetSubnets(cpVlanCfgDesc); //Установить подсети
          }
#endif
          if (cPortChanged) { //Изменились настройки портов
            memcpy((void *)&rsettings->sw, (void *)&sw, sizeof(struct switch_settings_s));
#ifdef USE_SUBNETS
            if (!cChanged)
#endif
              settings_save();
            if (cPortEnableChanged) {
              LoadRstpCfg(&rsettings->aRstpBridgeDesc[0]); //= Обновление конфигурации RSTP
              SetRstpBlock(); //+ Установка параметров RSTP
            }
            //Задать настройки портов
            extern void settings_load_switch(struct switch_settings_s* sw);
            settings_load_switch(&(rsettings->sw));
          }
        }
        return "/port.htm";
      }
      break;
  }
  return "/port.htm";
}

#ifdef USE_SWITCH //Использовать свитч kmz8895
  #include "ksz8895fmq.h"
  #include "switch.h"
  extern ksz8895fmq_t * SW1;
  #ifdef TWO_SWITCH_ITEMS
    extern ksz8895fmq_t * SW2;
  #endif
#endif

#ifndef NO_USE_VLANS
void ReadSettingsForVlan() {
  //Чтение уставок из регистров свитчей
  struct sVlanCfgDesc * cpVlanCfgDesc = &rsettings->VlanCfgDesc;
  //== Общие уставки
  //Вкл VLAN
  cpVlanCfgDesc->VlanOn = ReadSwRegBit(7, 5, SW1);
  //Unicast-пакеты могут выходить за пределы VLAN
  cpVlanCfgDesc->UniDis = ~ReadSwRegBit(7, 4, SW1);
  //NULL-VID заменять на VID порта
  cpVlanCfgDesc->NulToVid = ReadSwRegBit(3, 6, SW1);
  //Уставки портов
  struct sIntPort * cpIntPort = NULL;
  for (int8_t i = 0; i < 7; ++i) {
    cpIntPort = InternalPort(i + 1);
    if (cpIntPort == NULL)
      return;
    int16_t cStartReg = 0x10 * (cpIntPort->port);
    //Вставка в пакет тега VLAN с VID порта на входе
    cpVlanCfgDesc->aPortDesc[i].VlanTagIns = ReadSwRegBit(2, cStartReg + 0, cpIntPort->sw);
    //Удаление на выходе тега VLAN из пакета, принятого с тегом
    cpVlanCfgDesc->aPortDesc[i].VlanTagDel = ReadSwRegBit(1, cStartReg + 0, cpIntPort->sw);
    //Удаление пакетов, если входной порт не член VLAN
    cpVlanCfgDesc->aPortDesc[i].VlanTagDel = ReadSwRegBit(6, cStartReg + 2, cpIntPort->sw);
    //Удаление пакетов c VID не равным VID входного порта
    cpVlanCfgDesc->aPortDesc[i].VlanTagDel = ReadSwRegBit(5, cStartReg + 2, cpIntPort->sw);
    //Порты - члены VLAN для этого порта
    cpVlanCfgDesc->aPortDesc[i].VlanMembers = ReadSwRegField(0, 5, cStartReg + 1, cpIntPort->sw);
    //VID порта по умолчанию 0
    uint16_t cVid = 0;
    uint8_t cByte;
    cByte = ReadSwRegField(0, 7, cStartReg + 4, cpIntPort->sw);
    cVid = cByte;
    //VID порта по умолчанию 1
    cByte = ReadSwRegField(0, 7, cStartReg + 3, cpIntPort->sw);
    cVid |= (cByte << 8);
    cpVlanCfgDesc->aPortDesc[i].DefVlanTag = cVid;
  }
  //Уставки VLAN
  for (int8_t i = 0; i < VLAN_USED_TAB_SIZE; ++i) {

  }
  //cpVlanCfgDesc->VlanNum
}


void SetSettingsForVlan() {
  //Запись уставок в регистры свитчей
  struct sVlanCfgDesc * cpVlanCfgDesc = &rsettings->VlanCfgDesc;
  //== Общие уставки
  //Выкл VLAN для записи уставок
  SetSwVlan(0, SW1);
#ifdef TWO_SWITCH_ITEMS
  SetSwVlan(0, SW2);
#endif
  //Unicast-пакеты могут выходить за пределы VLAN
  SetSwRegBit(~cpVlanCfgDesc->UniDis , 7, 4, SW1, false);
#ifdef TWO_SWITCH_ITEMS
  SetSwRegBit(~cpVlanCfgDesc->UniDis , 7, 4, SW2, false);
#endif
  //NULL-VID заменять на VID порта
  SetSwRegBit(cpVlanCfgDesc->NulToVid, 3, 6, SW1, false);
#ifdef TWO_SWITCH_ITEMS
  SetSwRegBit(cpVlanCfgDesc->NulToVid, 3, 6, SW2, false);
#endif
  //Уставки портов
  struct sIntPort * cpIntPort = NULL;
  for (int8_t i = 0; i < PORT_NUMBER; ++i) {
    cpIntPort = InternalPort(i);
    int16_t cStartReg = 0x10 * (cpIntPort->port);
    //Вставка в пакет тега VLAN с VID порта на входе
    SetSwRegBit(cpVlanCfgDesc->aPortDesc[i].VlanTagIns, 2, cStartReg + 0, cpIntPort->sw, false);
    //Удаление на выходе тега VLAN из пакета, принятого с тегом
    SetSwRegBit(cpVlanCfgDesc->aPortDesc[i].VlanTagDel, 1, cStartReg + 0, cpIntPort->sw, false);
    //Удаление пакетов, если входной порт не член VLAN
    SetSwRegBit(cpVlanCfgDesc->aPortDesc[i].VlanTagDel, 6, cStartReg + 2, cpIntPort->sw, false);
    //Удаление пакетов c VID не равным VID входного порта
    SetSwRegBit(cpVlanCfgDesc->aPortDesc[i].VlanTagDel, 5, cStartReg + 2, cpIntPort->sw, false);
    //Порты - члены VLAN для этого порта
    SetSwRegField(cpVlanCfgDesc->aPortDesc[i].VlanMembers, 0, 5, cStartReg + 1, cpIntPort->sw, false);
    //VID порта по умолчанию 0
    uint8_t cByte = (cpVlanCfgDesc->aPortDesc[i].DefVlanTag & 0xff);
    SetSwRegField(cByte, 0, 7, cStartReg + 4, cpIntPort->sw, false);
    //VID порта по умолчанию 1
    cByte = ((cpVlanCfgDesc->aPortDesc[i].DefVlanTag >> 8) & 0xff);
    SetSwRegField(cByte, 0, 7, cStartReg + 3, cpIntPort->sw, false);
  }
  //Уставки VLAN
  for (int8_t i = 0; i < cpVlanCfgDesc->VlanNum; ++i) {

  }
  if (cpVlanCfgDesc->VlanOn) {
    SetSwVlan(cpVlanCfgDesc->VlanOn, SW1);
#ifdef TWO_SWITCH_ITEMS
    SetSwVlan(cpVlanCfgDesc->VlanOn, SW2);
#endif
  }
}
#endif

const char * SYSTEM_VLAN_CGI_Handler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[])
{ //
#ifndef NO_USE_VLANS
  uint8_t i;
  int value;
  bool cApplying = false;
  struct sMBParam * cpMBParam = GetPollMBParam();
  struct mode_struct_s *st = GetMode(cpMBParam->Chan - 1);
  switch(iIndex){
    case CGI_SYSTEM_VLAN:
      {
        if(iNumParams > 0)
        {
          for (i = 0; i < iNumParams; i++)
          {
            if (strcmp(pcParam[i], "applying") == 0) {
              sscanf(pcValue[i], "%d", &value);
              if (value)
                cApplying = true;
            }
          }
          if (!cApplying)
            break;
          bool cChanged = false;
          for (i = 0; i < iNumParams; i++)
          {
            value = 0;
            int cItemNum = 0;
            struct sVlanCfgDesc * cpVlanCfgDesc = &rsettings->VlanCfgDesc;
            struct sPortVlanDesc * caPortDesc = cpVlanCfgDesc->aPortDesc;
            //Общие параметры VLAN
            if (strncmp(pcParam[i] , "vlan_on", 7) == 0)
            {
              sscanf(pcValue[i],"%d",&value);
              if (cpVlanCfgDesc->VlanOn != value) {
                cpVlanCfgDesc->VlanOn = value;
                cChanged = true;
              }
            } else if (strncmp(pcParam[i] , "uni_off", 7) == 0) {
              sscanf(pcValue[i],"%d",&value);
              if (cpVlanCfgDesc->UniDis != value) {
                cpVlanCfgDesc->UniDis = value;
                cChanged = true;
              }
            } else if (strncmp(pcParam[i] , "chg_vid_on", 10) == 0) {
              sscanf(pcValue[i],"%d",&value);
              if (cpVlanCfgDesc->NulToVid != value) {
                cpVlanCfgDesc->NulToVid = value;
                cChanged = true;
              }
            //Параметры портов по VLAN
            } else if (strncmp(pcParam[i] , "otag_ins_m", 10) == 0) {
              sscanf(pcValue[i],"%d",&value);
              for (int i = 0; i < EXT_PORT_NUM; ++i) {
                bool cChecked = (value & (1 << i));
                if ((bool)caPortDesc[i].VlanTagIns != cChecked) {
                  caPortDesc[i].VlanTagIns = cChecked;
                  cChanged = true;
                }
              }
            } else if (strncmp(pcParam[i] , "otag_del_m", 10) == 0) {
              sscanf(pcValue[i],"%d",&value);
              for (int i = 0; i < EXT_PORT_NUM; ++i) {
                bool cChecked = (value & (1 << i));
                if ((bool)caPortDesc[i].VlanTagDel != cChecked) {
                  caPortDesc[i].VlanTagDel = cChecked;
                  cChanged = true;
                }
              }
            } else if (strncmp(pcParam[i] , "no_v_pack_del_", 14) == 0) {
              sscanf(pcValue[i],"%d",&value);
              for (int i = 0; i < EXT_PORT_NUM; ++i) {
                bool cChecked = (value & (1 << i));
                if ((bool)caPortDesc[i].NoVlanMemberDel != cChecked) {
                  caPortDesc[i].NoVlanMemberDel = cChecked;
                  cChanged = true;
                }
              }
            } else if (strncmp(pcParam[i] , "no_p_vid_pack_del_", 18) == 0) {
              sscanf(pcValue[i],"%d",&value);
              for (int i = 0; i < EXT_PORT_NUM; ++i) {
                bool cChecked = (value & (1 << i));
                if ((bool)caPortDesc[i].NoInVidDel != cChecked) {
                  caPortDesc[i].NoInVidDel = cChecked;
                  cChanged = true;
                }
              }
            } else if (strncmp(pcParam[i] , "def_port_vid_", 13) == 0) {
              sscanf(pcValue[i],"%d",&value);
              cItemNum = ItemNum(pcParam[i][13]);
              if (caPortDesc[cItemNum].DefVlanTag != value) {
                caPortDesc[cItemNum].DefVlanTag = value;
                cChanged = true;
              }
            } else if (strncmp(pcParam[i] , "pv_member_", 10) == 0) {
              sscanf(pcValue[i],"%d",&value);
              cItemNum = ItemNum(pcParam[i][10]);
              if (caPortDesc[cItemNum].VlanMembers != value) {
                caPortDesc[cItemNum].VlanMembers = value;
                cChanged = true;
              }
            //Параметры VLAN
            } else if (strncmp(pcParam[i] , "vlan_id_", 8) == 0) {
              sscanf(pcValue[i],"%d",&value);
              cItemNum = ItemNum(pcParam[i][8]);
              if (cpVlanCfgDesc->aVlanItem[cItemNum].VlanId != value) {
                cpVlanCfgDesc->aVlanItem[cItemNum].VlanId = value;
                cChanged = true;
              }
            } else if (strncmp(pcParam[i] , "vlan_vid_", 9) == 0) {
              sscanf(pcValue[i],"%d",&value);
              cItemNum = ItemNum(pcParam[i][9]);
              if (cpVlanCfgDesc->aVlanItem[cItemNum].VID != value) {
                cpVlanCfgDesc->aVlanItem[cItemNum].VID = value;
                cChanged = true;
              }
            } else if (strncmp(pcParam[i] , "vlan_member_", 12) == 0) {
              sscanf(pcValue[i],"%d",&value);
              cItemNum = ItemNum(pcParam[i][12]);
              if (cpVlanCfgDesc->aVlanItem[cItemNum].Members != value) {
                cpVlanCfgDesc->aVlanItem[cItemNum].Members = value;
                cChanged = true;
              }
            } else if (strncmp(pcParam[i] , "vlan_valid_", 11) == 0) {
              sscanf(pcValue[i],"%d",&value);
              cItemNum = ItemNum(pcParam[i][11]);
              if (cpVlanCfgDesc->aVlanItem[cItemNum].Valid != value) {
                cpVlanCfgDesc->aVlanItem[cItemNum].Valid = value;
                cChanged = true;
              }
            } else {
            }
          }
          if (cChanged) {
            settings_save();
//            SetSettingsForVlan(); //= Обновление конфигурации VLAN
          }
        }
      }
      break;
  }
#endif
  return "/vlan.htm";
}

#ifdef USE_SWITCH //Использовать свитч kmz8895
  #include "stp.h"
#endif

uint8_t Sw1PortFromMkpshPort(uint8_t iPort) { //Порт 1-го свитча из порта МКПШ
  uint8_t cPort;
  switch (iPort) {
  case 1:
    cPort = 4;
    break;
  case 2:
    cPort = 3;
    break;
  case 5:
    cPort = 2;
    break;
  case 3:
  case 4:
  case 6:
  case 7:
    cPort = 1;
    break;
  default:
    cPort = 0;
    break;
  }
  return cPort;
}

void ConvertAmpStr(char * iStr, int iSize, char * oStr) {
  int cOutIdx = 0;
  char aPatt[] = "0123456789ABCDEFabcdef";
  char aStr[3], cVal;
  aStr[2] = 0;
  for (int i = 0; i < iSize; ++i) {
    if (iStr[i] == 0x25) {
      aStr[0] = strchr(aPatt, iStr[i + 1]) - aPatt;
      if (aStr[0] >= 16)
        aStr[0] -= 6;
      aStr[1] = strchr(aPatt, iStr[i + 2]) - aPatt;
      if (aStr[1] >= 16)
        aStr[1] -= 6;
      cVal = aStr[0] * 16 + aStr[1];
      oStr[cOutIdx++] = cVal;
      i += 2;
    } else {
      oStr[cOutIdx++] = iStr[i];
    }
  }
}

#ifdef USE_STP
void LoadRstpCfgFlash() {
  LoadRstpCfg(&fsettings->aRstpBridgeDesc[0]); //= Обновление конфигурации RSTP
}
#endif

const char * SYSTEM_RSTP_CGI_Handler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[])
{ //
#ifdef USE_STP
  uint8_t i;
  int value;
  bool cApplying = false;
  switch(iIndex){
    case CGI_SYSTEM_RSTP:
      {
        if(iNumParams > 0)
        {
          for (i = 0; i < iNumParams; i++)
          {
            if (strcmp(pcParam[i], "applying") == 0) {
              sscanf(pcValue[i], "%d", &value);
              if (value)
                cApplying = true;
            }
          }
          if (!cApplying)
            break;
          bool cChanged = false;
          bool cChecked;
          for (i = 0; i < iNumParams; i++)
          {
            value = 0;
            int cItemNum = 0;
            //Общие параметры RSTP
            if (strncmp(pcParam[i] , "rstp_on",  7) == 0) {
              sscanf(pcValue[i],"%d",&value);
              if (rsettings->aRstpBridgeDesc[0].RstpOn != value) {
                rsettings->aRstpBridgeDesc[0].RstpOn = value;
                cChanged = true;
              }
            } else if (strncmp(pcParam[i] , "mac_addr", 8) == 0)
            {
              //Защита от кодировки (русской)
              bool cNoDigit = false;
              char cStr[20];
              ConvertAmpStr(pcValue[i], strlen(pcValue[i]), cStr);
              char aPatt[] = "0123456789ABCDEFabcdef%:-";
              char * cpPatt = NULL;
              for (int j = 0; j < 17; ++j) {
                cpPatt = strchr(aPatt, cStr[j]);
                if (cpPatt == NULL) {
                  cNoDigit = true;
                  break;
                }
              }
              if (cNoDigit)
                continue;
              //
              uint8_t mac[6];
              if(pcValue[i][2]=='%')
              {
                sscanf(pcValue[i],"%x%%3A%x%%3A%x%%3A%x%%3A%x%%3A%x",
                       &mac[0],&mac[1],&mac[2],&mac[3],&mac[4],&mac[5]);
              }
              else if(pcValue[i][2]==':')
              {
                sscanf(pcValue[i],"%x:%x:%x:%x:%x:%x",
                       &mac[0],&mac[1],&mac[2],&mac[3],&mac[4],&mac[5]);
              }
              else if(pcValue[i][2]=='-')
              {
                sscanf(pcValue[i],"%x-%x-%x-%x-%x-%x",
                       &mac[0],&mac[1],&mac[2],&mac[3],&mac[4],&mac[5]);
              }
              if (memcmp(&mac[0], &rsettings->aRstpBridgeDesc[0].MAC[0], 6) != 0) {
                for(int j=0;j<6;j++) {
                  rsettings->aRstpBridgeDesc[0].MAC[j] = (char)mac[j];
                }
                cChanged = true;
              }
            } else if (strncmp(pcParam[i] , "sw_priority", 11) == 0) {
              sscanf(pcValue[i],"%d",&value);
              if (rsettings->aRstpBridgeDesc[0].Priority != value) {
                rsettings->aRstpBridgeDesc[0].Priority = value;
                cChanged = true;
              }
            } else if (strncmp(pcParam[i] , "owner_id",  8) == 0) {
              sscanf(pcValue[i],"%d",&value);
              if (rsettings->aRstpBridgeDesc[0].FirmId != value) {
                rsettings->aRstpBridgeDesc[0].FirmId = value;
                cChanged = true;
              }
            } else if (strncmp(pcParam[i] , "max_age",  7) == 0) {
              sscanf(pcValue[i],"%d",&value);
              if (rsettings->aRstpBridgeDesc[0].MaxAge != value) {
                rsettings->aRstpBridgeDesc[0].MaxAge = value;
                cChanged = true;
              }
            } else if (strncmp(pcParam[i] , "hello_time", 10) == 0) {
              sscanf(pcValue[i],"%d",&value);
              if (rsettings->aRstpBridgeDesc[0].HelloTime != value) {
                rsettings->aRstpBridgeDesc[0].HelloTime = value;
                cChanged = true;
              }
            } else if (strncmp(pcParam[i] , "tx_hold_count", 13) == 0) {
              sscanf(pcValue[i],"%d",&value);
              if (rsettings->aRstpBridgeDesc[0].TxHoldCount != value) {
                rsettings->aRstpBridgeDesc[0].TxHoldCount = value;
                cChanged = true;
              }
            //Параметры портов
            } else if (strncmp(pcParam[i] , "enabled_m",  9) == 0) {
              sscanf(pcValue[i],"%d",&value);
              for (int i = 0; i < PORT_COUNT_HTTP; ++i) {
                cChecked = ((value & (1 << i)) != 0);
                if (Sw1PortFromMkpshPort(rsettings->HttpPort) == (i + 1)) {
                  cChecked = true; //Для порта управления - должно быть всегда Enabled
                }
                if (rsettings->aRstpBridgeDesc[0].aBPortDesc[i].portEnabled != cChecked) {
                  rsettings->aRstpBridgeDesc[0].aBPortDesc[i].portEnabled = cChecked;
                  cChanged = true;
                }
              }
            } else if (strncmp(pcParam[i] , "admin_edge_m", 12) == 0) {
              sscanf(pcValue[i],"%d",&value);
              for (int i = 0; i < PORT_COUNT_HTTP; ++i) {
                cChecked = ((value & (1 << i)) != 0);
                if (rsettings->aRstpBridgeDesc[0].aBPortDesc[i].adminEdge != cChecked) {
                  rsettings->aRstpBridgeDesc[0].aBPortDesc[i].adminEdge = cChecked;
                  cChanged = true;
                }
              }
            } else if (strncmp(pcParam[i] , "auto_edge_m", 11) == 0) {
              sscanf(pcValue[i],"%d",&value);
              for (int i = 0; i < PORT_COUNT_HTTP; ++i) {
                cChecked = ((value & (1 << i)) != 0);
                if (rsettings->aRstpBridgeDesc[0].aBPortDesc[i].autoEdge != cChecked) {
                  rsettings->aRstpBridgeDesc[0].aBPortDesc[i].autoEdge = cChecked;
                  cChanged = true;
                }
              }
            } else if (strncmp(pcParam[i] , "port_priority_", 14) == 0) {
              sscanf(pcValue[i],"%d",&value);
              cItemNum = ItemNum(pcParam[i][14]);
              if (rsettings->aRstpBridgeDesc[0].aBPortDesc[cItemNum].ForcePriority != value) {
                rsettings->aRstpBridgeDesc[0].aBPortDesc[cItemNum].ForcePriority = value;
                cChanged = true;
              }
            } else if (strncmp(pcParam[i] , "migrate_time_", 13) == 0) {
              sscanf(pcValue[i],"%d",&value);
              cItemNum = ItemNum(pcParam[i][13]);
              if (rsettings->aRstpBridgeDesc[0].aBPortDesc[cItemNum].MigrateTime != value) {
                rsettings->aRstpBridgeDesc[0].aBPortDesc[cItemNum].MigrateTime = value;
                cChanged = true;
              }
            //Для опроса состояния
//            } else if (strncmp(pcParam[i] , "startpoll",  9) == 0) {
//            } else if (strncmp(pcParam[i] , "for_change", 10) == 0) {
            } else {
            }
          }
          if (cChanged) {
            settings_save();
            LoadRstpCfg(&rsettings->aRstpBridgeDesc[0]); //= Обновление конфигурации RSTP
            SetRstpBlock(); //+ Установка параметров RSTP
          }
        }
      }
      break;
  }
#endif
  return "/stp.htm";
}

#include <time.h>
extern time_t GetCurrTime();
extern bool gbTimeIsAssigned;
extern void SetStartTime(time_t iNewStartTime, enum eEventCode iEventCode);
extern void InitNtpServer();
extern void SntpRestart();
const char * SYSTEM_TIME_CGI_Handler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[])
{ //
  uint8_t i;
  int value;
  switch(iIndex){
    case CGI_SYSTEM_TIME:
      {
        time_t cStartTime;
        if(iNumParams > 0)
        {
          cStartTime = GetCurrTime();
          struct tm * cpTime = localtime(&cStartTime);
          bool cTimeChanged = false, cParamChanged = false;
          bool cPeriodChanged = false;
          int8_t cParamChange = 0;
          for (i = 0; i < iNumParams; i++) {
            if (strncmp(pcParam[i] , "setparam", 9) == 0)
            { //Признак: изменить параметры = 1/время = 0
              sscanf(pcValue[i],"%d",&value);
              cParamChange = value;
            }
          }
          for (i = 0; i < iNumParams; i++)
          {
            value = 0;

            //Время
            if (!cParamChange) {
              if (strncmp(pcParam[i] , "day", 3) == 0) {
                sscanf(pcValue[i],"%d",&value);
                if (value != cpTime->tm_mday) {
                  cpTime->tm_mday = value;
                  cTimeChanged = true;
                }
              } else if (strncmp(pcParam[i] , "month", 5) == 0) {
                sscanf(pcValue[i],"%d",&value);
                --value;
                if (value != cpTime->tm_mon) {
                  cpTime->tm_mon = value;
                  cTimeChanged = true;
                }
              } else if (strncmp(pcParam[i] , "year", 4) == 0) {
                sscanf(pcValue[i],"%d",&value);
                value -= 1900;
                if (value != cpTime->tm_year) {
                  cpTime->tm_year = value;
                  cTimeChanged = true;
                }
              } else if (strncmp(pcParam[i] , "hour", 4) == 0) {
                sscanf(pcValue[i],"%d",&value);
                if (value != cpTime->tm_hour) {
                  cpTime->tm_hour = value;
                  cTimeChanged = true;
                }
              } else if (strncmp(pcParam[i] , "minute", 6) == 0) {
                sscanf(pcValue[i],"%d",&value);
                if (value != cpTime->tm_min) {
                  cpTime->tm_min = value;
                  cTimeChanged = true;
                }
              } else {
              }
            } else {
              //Параметры NTP
              if (strncmp(pcParam[i] , "ntp_ip", 6) == 0) {
                int ip[4];
                sscanf(pcValue[i],"%d.%d.%d.%d",&ip[0],&ip[1],&ip[2],&ip[3]);
                ip_addr_t cIpAddr;
                cIpAddr.addr = (ip[0] | ip[1]<<8 | ip[2]<<16 | ip[3]<<24);
                if (cIpAddr.addr != rsettings->NtpSettings.IpAddr.addr) {
                  rsettings->NtpSettings.IpAddr.addr = cIpAddr.addr;
                  cParamChanged = true;
                }
              } else if (strncmp(pcParam[i] , "ntp_mode", 8) == 0) {
                sscanf(pcValue[i],"%d",&value);
                if (value != rsettings->NtpSettings.TimeCorrMode) {
                  rsettings->NtpSettings.TimeCorrMode = (enum eTimeCorrMode)value;
                  cParamChanged = true;
                }
              } else if (strncmp(pcParam[i] , "period_num", 10) == 0) {
                sscanf(pcValue[i],"%d",&value);
                if (value != rsettings->NtpSettings.PeriodNum) {
                  rsettings->NtpSettings.PeriodNum = value;
                  cParamChanged = true;
                  cPeriodChanged = true;
                }
              } else if (strncmp(pcParam[i] , "period_kind", 11) == 0) {
                sscanf(pcValue[i],"%d",&value);
                if (value != rsettings->NtpSettings.PeriodKind) {
                  rsettings->NtpSettings.PeriodKind = (enum ePeriodKind)value;
                  cParamChanged = true;
                  cPeriodChanged = true;
                }
              } else if (strncmp(pcParam[i] , "time_zone", 9) == 0) {
                sscanf(pcValue[i],"%d",&value);
                if (value != rsettings->NtpSettings.TimeZone) {
                  rsettings->NtpSettings.TimeZone = value;
                  cParamChanged = true;
                }
              } else {
              }
            }
          }
          if (cParamChange == 0) {
            if (cTimeChanged) {
              cpTime->tm_sec = 0;
              cStartTime = mktime(cpTime);
              SetStartTime(cStartTime, ecSetSysTime);
            }
          } else {
            if (cParamChanged) {
              settings_save();
              InitNtpServer();
              if (cPeriodChanged) {
                SntpRestart();
              }
            }
          }
        }
      }
      break;
  }
  return "/time.htm";
}

#include "temp_ctrl.h"
const char * SYSTEM_TEMP_CGI_Handler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[])
{ //
  uint8_t i;
  int value;
  switch(iIndex){
    case CGI_SYSTEM_TEMP:
      {
        if(iNumParams > 0)
        {
          bool cParamChanged = false;
          for (i = 0; i < iNumParams; i++) {
            value = 0;
            //Параметры контроля температуры
            if (strncmp(pcParam[i] , "dev_warn_temp", 13) == 0) {
              sscanf(pcValue[i],"%d",&value);
              if (value != rsettings->TempWarningLevel) {
                rsettings->TempWarningLevel = value;
                cParamChanged = true;
              }
            } else if (strncmp(pcParam[i] , "dev_fault_temp", 8) == 0) {
              sscanf(pcValue[i],"%d",&value);
              if (value != rsettings->TempFaultLevel) {
                rsettings->TempFaultLevel = value;
                cParamChanged = true;
              }
            } else {
            }
          }
            if (cParamChanged) {
              settings_save();
#ifdef USE_TEMP_CTRL
              SetupZoneCtrl();
#endif
            }
        }
      }
      break;
  }
  return "/temp.htm";
}

#if ((UTD_M != 0) || (IIP != 0))
 #include "dev_param.h"
#if (IIP != 0)
 #include "esp32.h"
  uint32_t gConvWifiCount = 0;
#endif
const char * SYSTEM_CONV_WIFI_CGI_Handler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[])
{ //
  uint8_t i;
  int value;
  switch(iIndex){
    case CGI_SYSTEM_CONV_WIFI:
      {
        if(iNumParams > 0)
        {
          bool cParamChanged = false;
#if (UTD_M != 0)
          for (i = 0; i < iNumParams; i++) {
            value = 0;
            //Параметры WiFi
            if (strncmp(pcParam[i] , "dev_ant", 7) == 0) {
              sscanf(pcValue[i],"%d",&value);
              if (value != rsettings->UtdParam.AntType) {
                rsettings->UtdParam.AntType = (enum eUtdAnt)value;
                cParamChanged = true;
              }
            } else {
            }
          }
          if (cParamChanged) {
            settings_save();
            SetWiFiByParam();
          }
#endif
#if (IIP != 0)
          ++gConvWifiCount;
          int8_t cSetAP = -1;
          bool cPass = false;
          struct sIipParam cIipParam;
          bool cCheckIs = false;
          char cStr[100];
          memcpy((void *)&cIipParam, (void *)&rsettings->IipParam, sizeof(struct sIipParam));
          for (i = 0; i < iNumParams; i++) {
            value = 0;
            cStr[0] = 0;
            //Параметры WiFi
            if (strncmp(pcParam[i] , "ap_name", 5) == 0) {
              RefreshHttpString(pcValue[i], cStr, 100);
              if(strcmp(cStr, cIipParam.AP_Name) != 0) {
                strcpy(cIipParam.AP_Name, cStr);
                cParamChanged = true;
              }
            } else if (strncmp(pcParam[i] , "ap_pass", 4) == 0) {
              RefreshHttpString(pcValue[i], cStr, 100);
              if (strlen(cStr) > 0) {
                cPass = true;
                if(strcmp(cStr, cIipParam.AP_Pass) != 0) {
                  if (strlen(cStr) > 0) {
                    strcpy(cIipParam.AP_Pass, cStr);
                    cParamChanged = true;
                  }
                }
              }
            } else if (strncmp(pcParam[i] , "setparam", 8) == 0) {
              sscanf(pcValue[i],"%d",&value);
              cSetAP = value;
            } else if (strncmp(pcParam[i] , "conn_on_start", 13) == 0) {
              value = (strncmp(pcValue[i], "on", 2) == 0) ? 1:0;
              if (cIipParam.ConnOnStart != value) {
                cCheckIs = true;
                cIipParam.ConnOnStart = value;
                cParamChanged = true;
              }
            } else {
            }
          }
          if (!cCheckIs) {
//            if (cIipParam.ConnOnStart != 0) {
//              cIipParam.ConnOnStart = 0;
//              cParamChanged = true;
//              cCheckIs = true;
//            }
          }
          if (cParamChanged && (cSetAP <= 1)) {
            memcpy((void *)&rsettings->IipParam, (void *)&cIipParam, sizeof(struct sIipParam));
          }
          extern void SetAutoconnect();
          extern void ConnectToAP(char * iAPName, char * iPass);
          extern void DisconnectFromAP();
          extern void ServerOn();
          extern void ServerOff();
          switch (cSetAP) {
          case 0: //Сохранить параметры доступа
            if (cParamChanged) {
              settings_save();
              if (cCheckIs) {
                SetAutoconnect();
              }
            }
            break;
          case 1: //Подключиться к точке доступа
            if (cParamChanged)
              settings_save();
            if (cPass) { //Подключение к ТД
              ConnectToAP(rsettings->IipParam.AP_Name, rsettings->IipParam.AP_Pass);
            }
            break;
          case 2: //Отключиться от точки доступа
            DisconnectFromAP();
            break;
          case 3: //Пуск MODBUS сервера
            ServerOn();
            break;
          case 4: //Останов MODBUS сервера
            ServerOff();
            break;
          default: //
            break;
          }
#endif
        }
      }
      break;
  }
  return "/conv_wifi.htm";
}
#endif

#include "ping_settings.h"
#include "semphr.h"
#define MAX_INTERVAL_SIZE (99 * 60)

#ifdef PING_USE_SOCKETS
  extern struct sPingCtrl gaPingCtrl[];
#endif
extern void ping_reset();
#include "snmp_def.h"
#if (_USE_SNMP_CLIENT) //Подключение клиента SNMP
  extern void snmp_bulk_dst_ip_set(u8_t dst_idx, const ip_addr_t *dst);
#endif

const char * SYSTEM_PING_CGI_Handler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[])
{ //
  uint8_t i;
  int value;
  switch(iIndex){
    case CGI_SYSTEM_PING:
      {
        if(iNumParams > 0)
        {
          bool cParamChanged = false, cStartChanged = false, cIntChanged = false;
          struct sPingSettings cPingSettings;
          int cStart = -1, cInt = -1;
          memcpy((void *)&cPingSettings, (void *)&rsettings->PingSettings,
                 sizeof(struct sPingSettings));
          if (cPingSettings.CtrlInterval < 0) {
            cPingSettings.CtrlInterval = 0;
            cInt = 0;
            cParamChanged = true;
          }
          if (cPingSettings.CtrlInterval > MAX_INTERVAL_SIZE) {
            cPingSettings.CtrlInterval = MAX_INTERVAL_SIZE;
            cInt = MAX_INTERVAL_SIZE;
            cParamChanged = true;
          }
          if (cPingSettings.StartAfterBoot < 0) {
            cPingSettings.StartAfterBoot = 0;
            cStart = 0;
            cParamChanged = true;
          }
          if (cPingSettings.StartAfterBoot > MAX_INTERVAL_SIZE) {
            cPingSettings.StartAfterBoot = MAX_INTERVAL_SIZE;
            cStart = MAX_INTERVAL_SIZE;
            cParamChanged = true;
          }
          for (i = 0; i < iNumParams; i++) {
            value = 0;

            //Параметры ping
            if (strncmp(pcParam[i] , "ping_ip", 7) == 0) {
              int ip[4];
              sscanf(pcValue[i],"%d.%d.%d.%d",&ip[0],&ip[1],&ip[2],&ip[3]);
              ip_addr_t cIpAddr;
              cIpAddr.addr = (ip[0] | ip[1]<<8 | ip[2]<<16 | ip[3]<<24);
              if (cIpAddr.addr != cPingSettings.IpAddr.addr) {
                cPingSettings.IpAddr.addr = cIpAddr.addr;
                cParamChanged = true;
              }
            } else if (strncmp(pcParam[i] , "ping_mode", 9) == 0) {
              sscanf(pcValue[i],"%d",&value);
              if ((bool)value != cPingSettings.CtrlIsOn) {
                cPingSettings.CtrlIsOn = (bool)value;
                cParamChanged = true;
              }
            } else if (strncmp(pcParam[i] , "start_hour", 10) == 0) {
              sscanf(pcValue[i],"%d",&value);
              if (value != (cPingSettings.StartAfterBoot / 60)) {
                cStart = value * 60 + (cPingSettings.StartAfterBoot % 60);
                cParamChanged = true;
                cStartChanged = true;
              }
            } else if (strncmp(pcParam[i] , "start_min", 9) == 0) {
              sscanf(pcValue[i],"%d",&value);
              if ((value != (cPingSettings.StartAfterBoot % 60)) ||
                   (cStartChanged)) {
                if (cStart == -1)
                  cStart = cPingSettings.StartAfterBoot;
                cStart = (cStart / 60) * 60;
                cStart += value;
                cParamChanged = true;
                cStartChanged = true;
              }

            } else if (strncmp(pcParam[i] , "int_min", 7) == 0) {
              sscanf(pcValue[i],"%d",&value);
              if (value != (cPingSettings.CtrlInterval / 60)) {
                cInt = value * 60 + (cPingSettings.CtrlInterval % 60);
                if (cInt < 0)
                  cInt = 0;
                cIntChanged = true;
                cParamChanged = true;
              }
            } else if (strncmp(pcParam[i] , "int_sec", 7) == 0) {
              sscanf(pcValue[i],"%d",&value);
              if (value != (cPingSettings.CtrlInterval % 60)) {
                if (cInt == -1)
                  cInt = cPingSettings.CtrlInterval;
                cInt = (cInt / 60) * 60;
                cInt += value;
                cIntChanged = true;
                cParamChanged = true;
              }

            } else if (strncmp(pcParam[i] , "ping_num", 8) == 0) {
              sscanf(pcValue[i],"%d",&value);
              if (value != cPingSettings.PingNum) {
                cPingSettings.PingNum = value;
                cParamChanged = true;
              }
            } else if (strncmp(pcParam[i] , "ping_err_num", 12) == 0) {
              sscanf(pcValue[i],"%d",&value);
              if (value != cPingSettings.PingErrNum) {
                cPingSettings.PingErrNum = value;
                cParamChanged = true;
              }
            } else {
            }
          }
          if (cParamChanged) {
            if (cStartChanged)
              cPingSettings.StartAfterBoot = cStart;
            if (cIntChanged)
              cPingSettings.CtrlInterval = cInt;
#ifdef PING_USE_SOCKETS
            if (xSemaphoreTake(gaPingCtrl[0].Mutex, 200)) {
              memcpy((void *)&rsettings->PingSettings, &cPingSettings,
                     sizeof(struct sPingSettings));
 #if (_USE_SNMP_CLIENT) //Подключение клиента SNMP
              //Инициализация SNMP-клиента
              snmp_bulk_dst_ip_set(0, &cPingSettings.IpAddr);
 #endif
              ping_reset();
              xSemaphoreGive(gaPingCtrl[0].Mutex);
            }
#endif
            settings_save();
          }
        }
      }
      break;
  }
  return "/ping.htm";
}

int32_t gTableFirstEvent = -1; //Позиция первого события в таблице событий

const char * SYSTEM_EVENTS_CGI_Handler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[])
{ //
  switch(iIndex){
    case CGI_SYSTEM_EVENTS:
      {
        if(iNumParams > 0)
        {
        }
      }
      break;
  }
  return "/events.htm";
}

const char * SYSTEM_EVENT_TAB_CGI_Handler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[])
{ //
  uint8_t i;
  int value;
  switch(iIndex){
    case CGI_SYSTEM_EVENT_TAB:
      {
        if(iNumParams > 0)
        {
          for (i = 0; i < iNumParams; i++)
          {
            value = 0;
            if (strncmp(pcParam[i] , "tab_pos", 7) == 0)
            {
              sscanf(pcValue[i], "%d", &value);
              if (value != gTableFirstEvent) {
                gTableFirstEvent = value;
              }
//            } else {
            }
          }
        }
      }
      break;
  }
  return "/event_tab.htm";
}

/**
  * @brief  CGI handler for System control
  */
#ifdef USE_SWITCH //Использовать свитч kmz8895
static int stat_sel_[5]={0,0,0,0,0};
#endif
const char * SYSTEM_STAT_CGI_Handler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[])
{
#ifdef USE_SWITCH //Использовать свитч kmz8895
  uint8_t i;
  switch(iIndex){
    case CGI_SYSTEM_STAT:
      {
        for (i=0; i<iNumParams; i++)
        {
          if (strcmp(pcParam[i] , "stat_sel_1") == 0) {
            sscanf(pcValue[i],"%d",&stat_sel_[0]);
          } else if (strcmp(pcParam[i] , "stat_sel_2") == 0) {
            sscanf(pcValue[i],"%d",&stat_sel_[1]);
          } else if (strcmp(pcParam[i] , "stat_sel_3") == 0) {
            sscanf(pcValue[i],"%d",&stat_sel_[2]);
          } else if (strcmp(pcParam[i] , "stat_sel_4") == 0) {
            sscanf(pcValue[i],"%d",&stat_sel_[3]);
          } else if (strcmp(pcParam[i] , "stat_sel_5") == 0) {
            sscanf(pcValue[i],"%d",&stat_sel_[4]);
          } else {
          }
        }
      }
      break;
  }
#endif
  return "/stat.htm";
}

/**
  * @brief  ADC_Handler : SSI handler for ADC page
  */
#include "ff.h"
const char * get_ip_mode_str(enum ip_port_mode_e mode)
{
  switch(mode)
  {
    case SETTINGS_IP_PORT_MODBUS_GW:
      {
        return "modbusgw";
      }
      break;
    case SETTINGS_IP_PORT_TCP_SERVER:
      {
        return "tcpserver";
      }
      break;
    case SETTINGS_IP_PORT_TCP_CLIENT:
      {
        return "tcpclient";
      }
      break;
    case SETTINGS_IP_PORT_UDP:
      {
        return "udp";
      }
      break;
    case SETTINGS_IP_PORT_SIGRAND:
      {
        return "sigrand";
      }
      break;
    case SETTINGS_IP_PORT_MODBUS_RTU_OVER_TCP:
      {
        return "modbus_rtu_over_tcp";
      }
      break;
    case SETTINGS_IP_PORT_MODBUS_GW_UDP:
      {
        return "modbus_gw_udp";
      }
      break;
    case SETTINGS_IP_PORT_MODBUS_RTU_OVER_UDP:
      {
        return "modbus_rtu_over_udp";
      }
      break;
    case SETTINGS_IP_PORT_MODBUS_SERVER_SLAVE:
      {
        return "modbus_server_slave";
      }
      break;
  };
  return "error";
}
const char * get_uart_parity_str(enum settings_uart_parity parity)
{
  switch(parity)
  {
    case SETTINGS_UART_PARITY_NOT:
      {
        return "none";
      }
      break;
    case SETTINGS_UART_PARITY_ODD:
      {
        return "odd";
      }
      break;
    case SETTINGS_UART_PARITY_EVEN:
      {
        return "even";
      }
      break;
    case SETTINGS_UART_PARITY_MARK:
      {
        return "mark";
      }
      break;
    case SETTINGS_UART_PARITY_SPACE:
      {
        return "parity";
      }
      break;
  };
  return "none";
}

extern uint64_t xTaskGetTickCountL( void );
extern uint32_t CurrTimeInSecs();

extern int32_t GetEventCount();
extern struct sEvent * GetEventByIdx(int32_t iEventIdx);

extern void GetDateString(char * oStr, uint8_t iYear, uint8_t iMonth, uint8_t iDay);
extern void GetDateTimeString(char * oStr, time_t iTime);

extern bool IsHttpPortRight(uint8_t iPort);
#include "cable.h"
#include "vcc_ctrl.h"

u16_t httpd_handler(int iIndex, char *pcInsert, int iInsertLen)
{
  u16_t res_len=0;
  switch(iIndex)
  {
    case SYSTEM_STAT:
      {
        uint32_t length = 0;
#ifdef USE_SWITCH //Использовать свитч kmz8895
        uint64_t *temp;
        //char device[]=DEFAULT_SYSTEM_NAME;
        if(stat_sel_[0] > (0x1F+2))stat_sel_[0]=0;
        if(stat_sel_[1] > (0x1F+2))stat_sel_[0]=1;
        if(stat_sel_[2] > (0x1F+2))stat_sel_[0]=2;
        if(stat_sel_[3] > (0x1F+2))stat_sel_[0]=3;
        if(stat_sel_[4] > (0x1F+2))stat_sel_[0]=4;
        length += sprintf ( pcInsert+length, "\"stat_sel_1\":\"%d\",", stat_sel_[0] );
        length += sprintf ( pcInsert+length, "\"stat_sel_2\":\"%d\",", stat_sel_[1] );
        length += sprintf ( pcInsert+length, "\"stat_sel_3\":\"%d\",", stat_sel_[2] );
        length += sprintf ( pcInsert+length, "\"stat_sel_4\":\"%d\",", stat_sel_[3] );
        length += sprintf ( pcInsert+length, "\"stat_sel_5\":\"%d\",", stat_sel_[4] );
        temp=(uint64_t*)(&statistics[0]);
        length += sprintf ( pcInsert+length, "\"stat_1_\":[\"%llu\",\"%llu\",\"%llu\",\"%llu\",\"%llu\"],", temp[stat_sel_[0]],
                                                                                                            temp[stat_sel_[1]],
                                                                                                            temp[stat_sel_[2]],
                                                                                                            temp[stat_sel_[3]],
                                                                                                            temp[stat_sel_[4]]);
        temp=(uint64_t*)(&statistics[1]);
        length += sprintf ( pcInsert+length, "\"stat_2_\":[\"%llu\",\"%llu\",\"%llu\",\"%llu\",\"%llu\"],", temp[stat_sel_[0]],
                                                                                                            temp[stat_sel_[1]],
                                                                                                            temp[stat_sel_[2]],
                                                                                                            temp[stat_sel_[3]],
                                                                                                            temp[stat_sel_[4]]);
        temp=(uint64_t*)(&statistics[2]);
        length += sprintf ( pcInsert+length, "\"stat_3_\":[\"%llu\",\"%llu\",\"%llu\",\"%llu\",\"%llu\"],", temp[stat_sel_[0]],
                                                                                                            temp[stat_sel_[1]],
                                                                                                            temp[stat_sel_[2]],
                                                                                                            temp[stat_sel_[3]],
                                                                                                            temp[stat_sel_[4]]);
        temp=(uint64_t*)(&statistics[3]);
        length += sprintf ( pcInsert+length, "\"stat_4_\":[\"%llu\",\"%llu\",\"%llu\",\"%llu\",\"%llu\"],", temp[stat_sel_[0]],
                                                                                                            temp[stat_sel_[1]],
                                                                                                            temp[stat_sel_[2]],
                                                                                                            temp[stat_sel_[3]],
                                                                                                            temp[stat_sel_[4]]);
        temp=(uint64_t*)(&statistics[4]);
        length += sprintf ( pcInsert+length, "\"stat_5_\":[\"%llu\",\"%llu\",\"%llu\",\"%llu\",\"%llu\"],", temp[stat_sel_[0]],
                                                                                                            temp[stat_sel_[1]],
                                                                                                            temp[stat_sel_[2]],
                                                                                                            temp[stat_sel_[3]],
                                                                                                            temp[stat_sel_[4]]);
        temp=(uint64_t*)(&statistics[5]);
        length += sprintf ( pcInsert+length, "\"stat_6_\":[\"%llu\",\"%llu\",\"%llu\",\"%llu\",\"%llu\"],", temp[stat_sel_[0]],
                                                                                                            temp[stat_sel_[1]],
                                                                                                            temp[stat_sel_[2]],
                                                                                                            temp[stat_sel_[3]],
                                                                                                            temp[stat_sel_[4]]);
        temp=(uint64_t*)(&statistics[6]);
        length += sprintf ( pcInsert+length, "\"stat_7_\":[\"%llu\",\"%llu\",\"%llu\",\"%llu\",\"%llu\"],", temp[stat_sel_[0]],
                                                                                                            temp[stat_sel_[1]],
                                                                                                            temp[stat_sel_[2]],
                                                                                                            temp[stat_sel_[3]],
                                                                                                            temp[stat_sel_[4]]);
#endif
        return length;
      }
      break;
    case SYSTEM_PORTS:
      {
        uint32_t length = 0;
#ifdef USE_SWITCH //Использовать свитч kmz8895
        extern uint8_t read_switch_status(uint8_t port, uint8_t reg);
        for(int i=0;i<PORT_NUMBER;i++)
        {
          status_sw[i][0] = read_switch_status(i,0);//0xnA
          status_sw[i][1] = read_switch_status(i,1);//0xnE
          status_sw[i][2] = read_switch_status(i,2);//0xnF

            if (rsettings->sw.ports[i].autonegotiation) {
              //При autonegotiation эти параметры назначаяются свитчом автоматически
              rsettings->sw.ports[i].speed        = (switch_speed_t)(((status_sw[i][0]>>2)&1)?1:0);
              rsettings->sw.ports[i].full_duplex  = (switch_duplex_t)(((status_sw[i][0]>>1)&1)?1:0);
              rsettings->sw.ports[i].flow_control = (switch_flow_control_t)(((status_sw[i][0]>>3)&3)?1:0);
            }

        }
        //--------------------------------
        length += sprintf ( pcInsert+length, "\"pconn_\":[");
        for(int i = 0; i < PORT_NUMBER ; i++)
        {
          length += sprintf ( pcInsert+length, "\"%d\",", ((status_sw[i][1]>>5)&1));
        }
        length += sprintf ( pcInsert+length, "\"%d\"],", ((status_sw[PORT_NUMBER ][1]>>5)&1));

        //--------------------------------
        length += sprintf ( pcInsert+length, "\"pen_\":[");
        for(int i = 0; i < PORT_NUMBER ; i++)
        {
          length += sprintf ( pcInsert+length, "\"%d\",", rsettings->sw.ports[i].port_enabled);
        }
        length += sprintf ( pcInsert+length, "\"%d\"],", rsettings->sw.ports[PORT_NUMBER].port_enabled);

        //--------------------------------
 #if (MKPSH10 != 0)
        length += sprintf ( pcInsert+length, "\"pauto_\":[\"-\",\"-\",\"-\",\"-\",\"%d\",\"%d\",\"%d\"],",
                           rsettings->sw.ports[4].autonegotiation,
                           rsettings->sw.ports[5].autonegotiation,
                           rsettings->sw.ports[6].autonegotiation);
 #endif
 #if (IMC_FTX_MC != 0)
        length += sprintf ( pcInsert+length, "\"pauto_\":[\"%d\",\"%d\",\"%d\"],",
                           rsettings->sw.ports[0].autonegotiation,
                           rsettings->sw.ports[1].autonegotiation,
													 rsettings->sw.ports[2].autonegotiation);
 #endif
        //--------------------------------
 #if (MKPSH10 != 0)
        length += sprintf ( pcInsert+length, "\"pspeed_\":[\"-\",\"-\",\"-\",\"-\",\"%d\",\"%d\",\"%d\"],",
                           (rsettings->sw.ports[4].autonegotiation)?(((status_sw[4][0]>>2)&1)):(rsettings->sw.ports[4].speed),
                           (rsettings->sw.ports[5].autonegotiation)?(((status_sw[5][0]>>2)&1)):(rsettings->sw.ports[5].speed),
                           (rsettings->sw.ports[6].autonegotiation)?(((status_sw[6][0]>>2)&1)):(rsettings->sw.ports[6].speed));
 #endif
 #if (IMC_FTX_MC != 0)
        length += sprintf ( pcInsert+length, "\"pspeed_\":[\"%d\",\"%d\",\"%d\"],",
                           (rsettings->sw.ports[0].autonegotiation)?(((status_sw[0][0]>>2)&1)):(rsettings->sw.ports[0].speed),
                           (rsettings->sw.ports[1].autonegotiation)?(((status_sw[1][0]>>2)&1)):(rsettings->sw.ports[1].speed),
													 (rsettings->sw.ports[2].autonegotiation)?(((status_sw[2][0]>>2)&1)):(rsettings->sw.ports[2].speed));
 #endif

        //--------------------------------
        length += sprintf ( pcInsert+length, "\"pduplex_\":[");
        for(int i = 0; i < PORT_NUMBER; i++)
        {
          length += sprintf ( pcInsert+length, "\"%d\",", (rsettings->sw.ports[i].autonegotiation)?((((status_sw[i][0])>>1)&1 > 0)?1:0):(rsettings->sw.ports[i].full_duplex) );
        }
        length += sprintf ( pcInsert+length, "\"%d\"],", (rsettings->sw.ports[PORT_NUMBER ].autonegotiation)?((((status_sw[PORT_NUMBER ][0]>>1)&1)<<1 > 0)?1:0):(rsettings->sw.ports[PORT_NUMBER - 1].full_duplex));

        //--------------------------------
        length += sprintf ( pcInsert+length, "\"pcflw_\":[");
        for(int i = 0; i < PORT_NUMBER ; i++)
        {
          length += sprintf ( pcInsert+length, "\"%d\",", (rsettings->sw.ports[i].autonegotiation)?(((status_sw[i][0]>>3)&3)?1:0):(rsettings->sw.ports[i].flow_control) );
        }
        length += sprintf ( pcInsert+length, "\"%d\"],", (rsettings->sw.ports[PORT_NUMBER - 1].autonegotiation)?(((status_sw[PORT_NUMBER ][0]>>3)&3)?1:0):(rsettings->sw.ports[PORT_NUMBER - 1].flow_control));

        //--------------------------------
 #if (MKPSH10 != 0)
        length += sprintf ( pcInsert+length, "\"pamdi_\":[\"-\",\"-\",\"-\",\"-\",\"%d\",\"%d\",\"%d\"],",
                           (rsettings->sw.ports[4].mdi == 2)?1:0,
                           (rsettings->sw.ports[5].mdi == 2)?1:0,
                           (rsettings->sw.ports[6].mdi == 2)?1:0);
        //--------------------------------
        length += sprintf ( pcInsert+length, "\"pmdi_\":[\"-\",\"-\",\"-\",\"-\",\"%d\",\"%d\",\"%d\"],",
                           (rsettings->sw.ports[4].mdi == 2)?((((status_sw[4][1]>>7)&1)>0)?1:0):((rsettings->sw.ports[4].mdi == 1)?1:0),
                           (rsettings->sw.ports[5].mdi == 2)?((((status_sw[5][1]>>7)&1)>0)?1:0):((rsettings->sw.ports[5].mdi == 1)?1:0),
                           (rsettings->sw.ports[6].mdi == 2)?((((status_sw[6][1]>>7)&1)>0)?1:0):((rsettings->sw.ports[6].mdi == 1)?1:0));
 #endif

        length += sprintf ( pcInsert+length, "\"pamdi_\":[\"%d\",\"%d\",\"%d\"],",
                           (rsettings->sw.ports[0].mdi == 2)?1:0,
                           (rsettings->sw.ports[1].mdi == 2)?1:0,
														(rsettings->sw.ports[2].mdi == 2)?1:0 );
        //--------------------------------
        length += sprintf ( pcInsert+length, "\"pmdi_\":[\"%d\",\"%d\",\"%d\"],",
                           (rsettings->sw.ports[0].mdi == 2)?((((status_sw[0][1]>>7)&1)>0)?1:0):((rsettings->sw.ports[0].mdi == 1)?1:0),
                           (rsettings->sw.ports[1].mdi == 2)?((((status_sw[1][1]>>7)&1)>0)?1:0):((rsettings->sw.ports[1].mdi == 1)?1:0),
													 (rsettings->sw.ports[2].mdi == 2)?((((status_sw[2][1]>>7)&1)>0)?1:0):((rsettings->sw.ports[2].mdi == 1)?1:0)	 );

        //--------------------------------
        length += sprintf ( pcInsert+length, "\"palias_\":[");
        for(int i = 0; i < PORT_NUMBER ; i++)
        {
          length += sprintf ( pcInsert+length, "\"%s\",", rsettings->sw.ports[i].alias );
        }
        length += sprintf ( pcInsert+length, "\"%s\"],", rsettings->sw.ports[PORT_NUMBER ].alias );
        //--------------------------------
#endif //USE_SWITCH

#if ((MKPSH10 != 0) || (UTD_M != 0) || (IIP != 0))
        length += sprintf ( pcInsert+length, "\"ip_mode_\":[\"%s\",\"%s\",\"-\",\"-\",\"-\",\"-\",\"-\"],",
                           get_ip_mode_str(rsettings->ip_port[0].type),
                           get_ip_mode_str(rsettings->ip_port[1].type) );
#endif
#if (IMC_FTX_MC != 0)
        length += sprintf ( pcInsert+length, "\"ip_mode_\":[\"%s\"],",
                           get_ip_mode_str(rsettings->ip_port[0].type));
#endif
        //--------------------------------
#define UINT32_TO_4INT(N) (((N)>>0)&0xFF),(((N)>>8)&0xFF),(((N)>>16)&0xFF),(((N)>>24)&0xFF)
        extern bool GetTxState(int iModeIdx);
#if ((MKPSH10 != 0) || (UTD_M != 0) || (IIP != 0))
        length += sprintf ( pcInsert+length, "\"ip_addr_\":[\"%d.%d.%d.%d\",\"%d.%d.%d.%d\",\"-\",\"-\",\"-\",\"-\",\"-\"],",
                           UINT32_TO_4INT(rsettings->ip_port[0].ip.addr),
                           UINT32_TO_4INT(rsettings->ip_port[1].ip.addr));
        //--------------------------------
        length += sprintf ( pcInsert+length, "\"ip_port_local_\":[\"%d\",\"%d\",\"-\",\"-\",\"-\",\"-\",\"-\"],",
                           rsettings->ip_port[0].port_local,
                           rsettings->ip_port[1].port_local);
        //--------------------------------
        length += sprintf ( pcInsert+length, "\"ip_port_conn_\":[\"%d\",\"%d\",\"-\",\"-\",\"-\",\"-\",\"-\"],",
                           rsettings->ip_port[0].port_conn,
                           rsettings->ip_port[1].port_conn);

        //--------------------------------++++++++++++++++++++++++++
        length += sprintf ( pcInsert+length, "\"rs_baudrate_\":[\"%d\",\"%d\",\"-\",\"-\",\"-\",\"-\",\"-\"],",
                           rsettings->uart[0].baudrate,
                           rsettings->uart[1].baudrate);
        //--------------------------------++++++++++++++++++++++++++
        length += sprintf ( pcInsert+length, "\"rs_databits_\":[\"%d\",\"%d\",\"-\",\"-\",\"-\",\"-\",\"-\"],",
                           rsettings->uart[0].databits,
                           rsettings->uart[1].databits);
        //--------------------------------++++++++++++++++++++++++++
        length += sprintf ( pcInsert+length, "\"rs_stopbits_\":[\"%d\",\"%d\",\"-\",\"-\",\"-\",\"-\",\"-\"],",
                           rsettings->uart[0].stopbits,
                           rsettings->uart[1].stopbits);
        //--------------------------------++++++++++++++++++++++++++
        length += sprintf ( pcInsert+length, "\"rs_parity_\":[\"%s\",\"%s\",\"-\",\"-\",\"-\",\"-\",\"-\"],",
                           get_uart_parity_str(rsettings->uart[0].parity),
                           get_uart_parity_str(rsettings->uart[1].parity));
        //--------------------------------++++++++++++++++++++++++++
        length += sprintf ( pcInsert+length, "\"rs_fctrl_\":[\"%s\",\"%s\",\"-\",\"-\",\"-\",\"-\",\"-\"],",
                           "none",
                           "none");
        //--------------------------------++++++++++++++++++++++++++
        struct mode_struct_s *st1 = GetMode(0);
        struct mode_struct_s *st2 = GetMode(1);
        length += sprintf ( pcInsert+length, "\"rs_conn_\":[\"%d\",\"%d\",\"-\",\"-\",\"-\",\"-\",\"-\"],",
                           (extio_read_idx(EXTIO_CPU_33_V5)>0) ? 0:1,
                           (extio_read_idx(EXTIO_CPU_33_V4)>0) ? 0:1);
        length += sprintf ( pcInsert+length, "\"rs_tx_\":[\"%d\",\"%d\",\"-\",\"-\",\"-\",\"-\",\"-\"],",
                           (GetTxState(0)) ? 1:0,
                           (GetTxState(1)) ? 1:0 );
//                           (st1->state == MODE_STATE_CONNECTED) ? 1:0,
//                           (st2->state == MODE_STATE_CONNECTED) ? 1:0);
        length += sprintf ( pcInsert+length, "\"rs_rx_\":[\"%d\",\"%d\",\"-\",\"-\",\"-\",\"-\",\"-\"],",
                           (extio_led_blinked(EXTIO_RS485_1)>0) ? 1:0,
                           (extio_led_blinked(EXTIO_RS485_2)>0) ? 1:0);
 #if (MKPSH10 != 0)
  #ifdef USE_ETH_PORT_LOAD //Вычисление нагрузки порта в %
        length += sprintf ( pcInsert+length, "\"load_\":[\"%#.2f%%\",\"%#.2f%%\",\"%#.2f%%\",\"%#.2f%%\",\"%#.2f%%\",\"%#.2f%%\",\"%#.2f%%\"],",
                            GetPortLoad(0), GetPortLoad(1), GetPortLoad(2), GetPortLoad(3),
                            GetPortLoad(4), GetPortLoad(5), GetPortLoad(6));
  #endif
        length += sprintf ( pcInsert+length, "\"load_tx_\":[\"%d\",\"%d\",\"%d\",\"%d\",\"%d\",\"%d\",\"%d\"],",
                            GetPortTraffic(0, 0), GetPortTraffic(0, 1), GetPortTraffic(0, 2), GetPortTraffic(0, 3),
                            GetPortTraffic(0, 4), GetPortTraffic(0, 5), GetPortTraffic(0, 6));
        length += sprintf ( pcInsert+length, "\"load_rx_\":[\"%d\",\"%d\",\"%d\",\"%d\",\"%d\",\"%d\",\"%d\"],",
                            GetPortTraffic(1, 0), GetPortTraffic(1, 1), GetPortTraffic(1, 2), GetPortTraffic(1, 3),
                            GetPortTraffic(1, 4), GetPortTraffic(1, 5), GetPortTraffic(1, 6));

 #endif
#endif
#if (IMC_FTX_MC != 0)
        length += sprintf ( pcInsert+length, "\"ip_addr_\":[\"%d.%d.%d.%d\"],",
                           UINT32_TO_4INT(rsettings->ip_port[0].ip.addr));
        //--------------------------------
        length += sprintf ( pcInsert+length, "\"ip_port_local_\":[\"%d\"],",
                           rsettings->ip_port[0].port_local);
        //--------------------------------
        length += sprintf ( pcInsert+length, "\"ip_port_conn_\":[\"%d\"],",
                           rsettings->ip_port[0].port_conn);
        //--------------------------------++++++++++++++++++++++++++
        length += sprintf ( pcInsert+length, "\"rs_baudrate_\":[\"%d\"],",
                           rsettings->uart[0].baudrate);
        //--------------------------------++++++++++++++++++++++++++
        length += sprintf ( pcInsert+length, "\"rs_databits_\":[\"%d\"],",
                           rsettings->uart[0].databits);
        //--------------------------------++++++++++++++++++++++++++
        length += sprintf ( pcInsert+length, "\"rs_stopbits_\":[\"%d\"],",
                           rsettings->uart[0].stopbits);
        //--------------------------------++++++++++++++++++++++++++
        length += sprintf ( pcInsert+length, "\"rs_parity_\":[\"%s\"],",
                           get_uart_parity_str(rsettings->uart[0].parity));
        //--------------------------------++++++++++++++++++++++++++
        length += sprintf ( pcInsert+length, "\"rs_fctrl_\":[\"%s\"],",
                           "none");

        struct mode_struct_s *st1 = GetMode(0);
        length += sprintf ( pcInsert+length, "\"rs_conn_\":[\"%d\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\"],",
                           (extio_read_idx(EXTIO_CPU_33_V5)==0) ? 1:0);  //RS485_1
        length += sprintf ( pcInsert+length, "\"rs_tx_\":[\"%d\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\"],",
                           (GetTxState(0)) ? 1:0);
//                           (st1->state == MODE_STATE_CONNECTED) ? 1:0);
        length += sprintf ( pcInsert+length, "\"rs_rx_\":[\"%d\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\"],",
                           (extio_led_blinked(EXTIO_RS485_1)>0) ? 1:0);
  #ifdef USE_ETH_PORT_LOAD //Вычисление нагрузки порта в %
        length += sprintf ( pcInsert+length, "\"load_\":[\"%#.2f%%\",\"%#.2f%%\",\"%#.2f%%\",\"%#.2f%%\",\"-\",\"-\",\"-\"],",
                            GetPortLoad(0), GetPortLoad(1), GetPortLoad(2), GetPortLoad(3));
  #endif
        length += sprintf ( pcInsert+length, "\"load_tx_\":[\"%d\",\"%d\",\"%d\",\"%d\",\"-\",\"-\",\"-\"],",
                            GetPortTraffic(0, 0), GetPortTraffic(0, 1), GetPortTraffic(0, 2), GetPortTraffic(0, 3));
        length += sprintf ( pcInsert+length, "\"load_rx_\":[\"%d\",\"%d\",\"%d\",\"%d\",\"-\",\"-\",\"-\"],",
                            GetPortTraffic(1, 0), GetPortTraffic(1, 1), GetPortTraffic(1, 2), GetPortTraffic(1, 3));
 #ifdef USE_SUBNETS
        //Подсети на базе портов
        length += sprintf ( pcInsert+length, "\"subnet_member_\":[\"%d\",\"%d\",\"%d\"],",
                           rsettings->VlanCfgDesc.aVlanItem[0].Members,
                           rsettings->VlanCfgDesc.aVlanItem[1].Members,
                           rsettings->VlanCfgDesc.aVlanItem[2].Members
                           );
 #endif //USE_SUBNETS
#endif
        return length;
      }
      break;
    case SYSTEM_ICONN_SW:
      {
        uint32_t length = 0;
#ifdef USE_SWITCH //Использовать свитч kmz8895
        int sw_iconn_state = 0;
        extern int read_switch_iconn_status();
        sw_iconn_state = read_switch_iconn_status();
        length += sprintf ( pcInsert+length, "%d", sw_iconn_state );
#endif
        return length;
      }
      break;
    case SYSTEM_VCC:
        {
          uint32_t length = 0;
          float cVcc = 0;
          extern float GetAverageVcc();
          cVcc = GetAverageVcc();
          length += sprintf ( pcInsert+length, "%#.1f", cVcc );
          return length;
        }
        break;
      case SYSTEM_TEMP:
        {
          uint32_t length = 0;
          float cTemp = 0;
#ifdef USE_TEMP_CTRL
          extern float GetAverageTemp();
          cTemp = GetAverageTemp();
#endif
          length += sprintf ( pcInsert+length, "%#.1f", cTemp );
          return length;
        }
        break;
      case SYSTEM_WARN_TEMP:
        {
          uint32_t length = 0;
          float cTemp = rsettings->TempWarningLevel;
          length += sprintf ( pcInsert+length, "%#.1f", cTemp );
          return length;
        }
        break;
      case SYSTEM_FAULT_TEMP:
        {
          uint32_t length = 0;
          float cTemp = rsettings->TempFaultLevel;
          length += sprintf ( pcInsert+length, "%#.1f", cTemp );
          return length;
        }
        break;
      case SYSTEM_STATE_TEMP:
        {
          uint32_t length = 0;
          uint16_t cTemp = 0;
#ifdef USE_TEMP_CTRL
          cTemp = (uint16_t)GetTempZone();
#endif
          length += sprintf ( pcInsert+length, "%d", cTemp );
          return length;
        }
        break;
#if (UTD_M != 0)
      case SYSTEM_ANT:
        {
          uint32_t length = 0;
          int cAnt = rsettings->UtdParam.AntType;
          length += sprintf ( pcInsert+length, "%d", cAnt );
          return length;
        }
        break;
      case SYSTEM_POWER_WIFI_1:
        {
          uint32_t length = 0;
          uint8_t cPwr = GetWiFiPwr(0);
          length += sprintf ( pcInsert+length, "%d", cPwr );
          return length;
        }
        break;
      case SYSTEM_POWER_WIFI_2:
        {
          uint32_t length = 0;
          uint8_t cPwr = GetWiFiPwr(1);
          length += sprintf ( pcInsert+length, "%d", cPwr );
          return length;
        }
        break;
#endif
      case SYSTEM_TIME:
        {
          uint32_t length = 0;
          time_t cStartTime = GetCurrTime();
          struct tm * cpTime = localtime(&cStartTime);
          length += sprintf ( pcInsert+length, "\"day\":\"%d\",", cpTime->tm_mday );
          length += sprintf ( pcInsert+length, "\"month\":\"%d\",", cpTime->tm_mon );
          length += sprintf ( pcInsert+length, "\"year\":\"%d\",", cpTime->tm_year );
          length += sprintf ( pcInsert+length, "\"hour\":\"%d\",", cpTime->tm_hour );
          length += sprintf ( pcInsert+length, "\"minute\":\"%d\",", cpTime->tm_min );

          uint8_t ip[4]={192,168,2,100};
          memcpy(ip,&(rsettings->NtpSettings.IpAddr),4);
          length += sprintf ( pcInsert+length, "\"ntp_ip\":\"%d.%d.%d.%d\",", ip[0],ip[1],ip[2],ip[3] );
          length += sprintf ( pcInsert+length, "\"ntp_mode\":\"%d\",", rsettings->NtpSettings.TimeCorrMode );
          length += sprintf ( pcInsert+length, "\"period_num\":\"%d\",", rsettings->NtpSettings.PeriodNum );
          length += sprintf ( pcInsert+length, "\"period_kind\":\"%d\",", rsettings->NtpSettings.PeriodKind );
          length += sprintf ( pcInsert+length, "\"time_zone\":\"%d\",", rsettings->NtpSettings.TimeZone );

          return length;
        }
        break;
      case SYSTEM_PING:
        {
          uint32_t length = 0;
          uint8_t ip[4];
          memcpy(ip,&(rsettings->PingSettings.IpAddr),4);
          length += sprintf ( pcInsert+length, "\"ping_ip\":\"%d.%d.%d.%d\",", ip[0],ip[1],ip[2],ip[3] );
          length += sprintf ( pcInsert+length, "\"ping_mode\":\"%d\",", rsettings->PingSettings.CtrlIsOn );

          length += sprintf ( pcInsert+length, "\"start_hour\":\"%d\",", rsettings->PingSettings.StartAfterBoot / 60 );
          length += sprintf ( pcInsert+length, "\"start_min\":\"%d\",", rsettings->PingSettings.StartAfterBoot % 60 );

          length += sprintf ( pcInsert+length, "\"int_min\":\"%d\",", rsettings->PingSettings.CtrlInterval / 60 );
          length += sprintf ( pcInsert+length, "\"int_sec\":\"%d\",", rsettings->PingSettings.CtrlInterval % 60 );

          length += sprintf ( pcInsert+length, "\"ping_num\":\"%d\",", rsettings->PingSettings.PingNum );
          length += sprintf ( pcInsert+length, "\"ping_err_num\":\"%d\",", rsettings->PingSettings.PingErrNum );

          return length;
        }
        break;
      case TIME_ASSIGNED:
        {
          uint32_t length = 0;
          uint8_t cAssigned = (uint8_t)gbTimeIsAssigned;
          length += sprintf ( pcInsert+length, "%d", cAssigned );
          return length;
        }
        break;
      case SYSTEM_NETWORK:
        {
          uint32_t length = 0;
          uint8_t ip[4]={192,168,2,100};
          uint8_t mac[6]={0x02,0x01,0x02,0x03,0x04,0x05};
          memcpy(ip,&(rsettings->ip.addr.addr),4);
          length += sprintf ( pcInsert+length, "\"ip\":\"%d.%d.%d.%d\",", ip[0],ip[1],ip[2],ip[3] );
          memcpy(ip,&(rsettings->ip.mask),4);
          length += sprintf ( pcInsert+length, "\"netmask\":\"%d.%d.%d.%d\",", ip[0],ip[1],ip[2],ip[3] );
          memcpy(ip,&(rsettings->ip.gw),4);
          length += sprintf ( pcInsert+length, "\"gateway\":\"%d.%d.%d.%d\",", ip[0],ip[1],ip[2],ip[3] );
          memcpy(mac,rsettings->mac,6);
          length += sprintf ( pcInsert+length, "\"mac\":\"%02X:%02X:%02X:%02X:%02X:%02X\"", mac[0],mac[1],mac[2],mac[3],mac[4],mac[5] );
#if (MKPSH10 != 0)
          length += sprintf ( pcInsert+length, ",\"ipmode\":\"%d\"", (int)rsettings->DevMode );
#endif
          return length;
        }
        break;
  case USART_REGS: //Поллинг RS485: Регистры usart
      {
#define REG_NUM (40) //== количеству регистров, выводимых в portpoll.htm
        uint32_t length = 0;
        struct sMBParam * cpMBParam = GetPollMBParam();
        struct mode_struct_s *st = GetMode(cpMBParam->Chan - 1);
        int cTimeout;
        if (gApplying) {
          cTimeout = cpMBParam->Period * 7 / 10;
          while (cTimeout > 0) { //Ожидаем ответ от канала RS485
            if ((cpMBParam->rxCount > 0) || (cpMBParam->Error > 0)) {
              break;
            }
            vTaskDelay(20);
            cTimeout -= 20;
          }
          if (cTimeout > 0) {
          } else {
            cpMBParam->Error = mbrTimeout;
            cpMBParam->rxCount = 0;
          }
        }

        uint16_t * cpBuf = (uint16_t *)&cpMBParam->rBuffer[0];

        length += sprintf ( pcInsert + length, "\"reg_\":[");
        for(int i = 0; i < REG_NUM; i++)
        {
          if (i == REG_NUM - 1)
            length += sprintf ( pcInsert + length, "\"%d\"", cpBuf[i]);
          else
            length += sprintf ( pcInsert  +length, "\"%d\",", cpBuf[i]);
        }
        length += sprintf ( pcInsert + length, "],");

        return length;
      }
      break;
    case RS_DEVADDR:
      {
        uint32_t length = 0;
        struct sMBParam * cpMBParam = GetPollMBParam();
        length += sprintf ( pcInsert+length, "%d", cpMBParam->Addr );
        return length;
      }
      break;
    case RS_MODBUSFUNC:
      {
        uint32_t length = 0;
        struct sMBParam * cpMBParam = GetPollMBParam();
        length += sprintf ( pcInsert+length, "%d", cpMBParam->Func );
        return length;
      }
      break;
    case RS_STARTREG:
      {
        uint32_t length = 0;
        struct sMBParam * cpMBParam = GetPollMBParam();
        length += sprintf ( pcInsert+length, "%d", cpMBParam->Start );
        return length;
      }
      break;
    case RS_REGNUMBER:
      {
        uint32_t length = 0;
        struct sMBParam * cpMBParam = GetPollMBParam();
        length += sprintf ( pcInsert+length, "%d", cpMBParam->Number );
        return length;
      }
      break;
    case RS_POLLPERIOD:
      {
        uint32_t length = 0;
        struct sMBParam * cpMBParam = GetPollMBParam();
        length += sprintf ( pcInsert+length, "%d", cpMBParam->Period );
        return length;
      }
      break;
    case RS_POLLCHANNEL:
      {
        uint32_t length = 0;
        struct sMBParam * cpMBParam = GetPollMBParam();
        length += sprintf ( pcInsert+length, "%d", cpMBParam->Chan );
        return length;
      }
      break;
    case RS_RXERROR:
      {
        uint32_t length = 0;
        int cMsgIdx = 0;
        struct sMBParam * cpMBParam = GetPollMBParam();
        if (cpMBParam->Error & 0x8000) {
          cMsgIdx = mbrModbus;
        } else {
          if ((cpMBParam->Error >= 0) && (cpMBParam->Error < mbrCount))
            cMsgIdx = cpMBParam->Error;
        }
        length += sprintf ( pcInsert+length, "%d", cMsgIdx );
        return length;
      }
      break;
    case RS_STARTPOLL:
      {
        uint32_t length = 0;
        length += sprintf ( pcInsert+length, "%d", (uint8_t)gApplying );
        return length;
      }
      break;
//////////////////////////////////////////////////////////////////////
//		case D_UP:
//      {
//        uint32_t length = 0;
//        length += sprintf ( pcInsert+length, "%d", (uint8_t)gApplying );
//        return length;
//      }
//      break;
/////////////////////////////////////////////////////////////////////
    case RS_STOPPOLL:
      {
        uint32_t length = 0;
        length += sprintf ( pcInsert+length, "%d", (uint8_t)gStoping );
        return length;
      }
      break;
    case RS_TXCOUNT:
      {
        uint32_t length = 0;
        length += sprintf ( pcInsert+length, "%d", (uint8_t)gTxCount );
        return length;
      }
      break;
    case RS_ERRCOUNT:
      {
        uint32_t length = 0;
        length += sprintf ( pcInsert+length, "%d", (uint8_t)gErrCount );
        return length;
      }
      break;
#ifdef USE_VLAN  //Поддержка VLAN
    //Общие для vlan
    case VL_VLAN_ON:
      {
        uint32_t length = 0;
        length += sprintf ( pcInsert+length, "%d", (uint8_t)rsettings->VlanCfgDesc.VlanOn );
        return length;
      }
      break;
    case VL_UNI_OFF:
      {
        uint32_t length = 0;
        length += sprintf ( pcInsert+length, "%d", (uint8_t)rsettings->VlanCfgDesc.UniDis );
        return length;
      }
      break;
    case VL_CHGVID_ON:
      {
        uint32_t length = 0;
        length += sprintf ( pcInsert+length, "%d", (uint8_t)rsettings->VlanCfgDesc.NulToVid );
        return length;
      }
      break;
    //Порты
    case VL_SYSTEM_PORTS:
      {
        uint32_t length = 0;
        uint8_t cValue;
        cValue = 0;
        for (int i = 0; i < EXT_PORT_NUM; ++i) {
          if (rsettings->VlanCfgDesc.aPortDesc[i].VlanTagIns)
            cValue |= (1 << i);
        }
        length += sprintf ( pcInsert+length, "\"otag_ins_\":[\"%d\",\"0\",\"0\",\"0\",\"0\",\"0\",\"0\"],", cValue);
        cValue = 0;
        for (int i = 0; i < EXT_PORT_NUM; ++i) {
          if (rsettings->VlanCfgDesc.aPortDesc[i].VlanTagDel)
            cValue |= (1 << i);
        }
        length += sprintf ( pcInsert+length, "\"otag_del_\":[\"%d\",\"0\",\"0\",\"0\",\"0\",\"0\",\"0\"],", cValue);
        cValue = 0;
        for (int i = 0; i < EXT_PORT_NUM; ++i) {
          if (rsettings->VlanCfgDesc.aPortDesc[i].NoVlanMemberDel)
            cValue |= (1 << i);
        }
        length += sprintf ( pcInsert+length, "\"no_v_pack_del_\":[\"%d\",\"0\",\"0\",\"0\",\"0\",\"0\",\"0\"],", cValue);
        cValue = 0;
        for (int i = 0; i < EXT_PORT_NUM; ++i) {
          if (rsettings->VlanCfgDesc.aPortDesc[i].NoInVidDel)
            cValue |= (1 << i);
        }
        length += sprintf ( pcInsert+length, "\"no_p_vid_pack_del_\":[\"%d\",\"0\",\"0\",\"0\",\"0\",\"0\",\"0\"],", cValue);
        length += sprintf ( pcInsert+length, "\"def_port_vid_\":[\"%d\",\"%d\",\"%d\",\"%d\",\"%d\",\"%d\",\"%d\"],",
                           rsettings->VlanCfgDesc.aPortDesc[0].DefVlanTag,
                           rsettings->VlanCfgDesc.aPortDesc[1].DefVlanTag,
                           rsettings->VlanCfgDesc.aPortDesc[2].DefVlanTag,
                           rsettings->VlanCfgDesc.aPortDesc[3].DefVlanTag,
                           rsettings->VlanCfgDesc.aPortDesc[4].DefVlanTag,
                           rsettings->VlanCfgDesc.aPortDesc[5].DefVlanTag,
                           rsettings->VlanCfgDesc.aPortDesc[6].DefVlanTag);
        length += sprintf ( pcInsert+length, "\"pv_member_\":[\"%d\",\"%d\",\"%d\",\"%d\",\"%d\",\"%d\",\"%d\"],",
                           rsettings->VlanCfgDesc.aPortDesc[0].VlanMembers,
                           rsettings->VlanCfgDesc.aPortDesc[1].VlanMembers,
                           rsettings->VlanCfgDesc.aPortDesc[2].VlanMembers,
                           rsettings->VlanCfgDesc.aPortDesc[3].VlanMembers,
                           rsettings->VlanCfgDesc.aPortDesc[4].VlanMembers,
                           rsettings->VlanCfgDesc.aPortDesc[5].VlanMembers,
                           rsettings->VlanCfgDesc.aPortDesc[6].VlanMembers);
        return length;
      }
      break;
    //VLANs
    case VL_SYSTEM_VLANS:
      {
        uint32_t length = 0;
        length += sprintf ( pcInsert+length, "\"vlan_id_\":[\"%d\",\"%d\",\"%d\"],",
                           rsettings->VlanCfgDesc.aVlanItem[0].VlanId,
                           rsettings->VlanCfgDesc.aVlanItem[1].VlanId,
                           rsettings->VlanCfgDesc.aVlanItem[2].VlanId
                           );
        length += sprintf ( pcInsert+length, "\"vlan_vid_\":[\"%d\",\"%d\",\"%d\"],",
                           rsettings->VlanCfgDesc.aVlanItem[0].VID,
                           rsettings->VlanCfgDesc.aVlanItem[1].VID,
                           rsettings->VlanCfgDesc.aVlanItem[2].VID
                           );
        length += sprintf ( pcInsert+length, "\"vlan_member_\":[\"%d\",\"%d\",\"%d\"],",
                           rsettings->VlanCfgDesc.aVlanItem[0].Members,
                           rsettings->VlanCfgDesc.aVlanItem[1].Members,
                           rsettings->VlanCfgDesc.aVlanItem[2].Members
                           );
        length += sprintf ( pcInsert+length, "\"vlan_valid_\":[\"%d\",\"%d\",\"%d\"],",
                           rsettings->VlanCfgDesc.aVlanItem[0].Valid,
                           rsettings->VlanCfgDesc.aVlanItem[1].Valid,
                           rsettings->VlanCfgDesc.aVlanItem[2].Valid
                           );
        return length;
      }
      break;
    // -
#endif
#ifdef USE_STP
    case STP_SYSTEM_RSTP:
      {
        uint32_t length = 0;
        uint8_t mac[6]={0x02,0x01,0x02,0x03,0x04,0x05};
        uint16_t cVar = 0;
        cVar = gaRstpBridgeDesc[0].RstpOn;
        length += sprintf ( pcInsert+length, "\"rstp_on\":\"%d\",", cVar );
        memcpy(mac, gaRstpBridgeDesc[0].MAC, ETH_HWADDR_LEN);
        length += sprintf ( pcInsert+length, "\"mac_addr\":\"%02X:%02X:%02X:%02X:%02X:%02X\",",
                           mac[0],mac[1],mac[2],mac[3],mac[4],mac[5] );
        cVar = gaRstpBridgeDesc[0].Priority;
        length += sprintf ( pcInsert+length, "\"sw_priority\":\"%d\",", cVar );
        cVar = gaRstpBridgeDesc[0].FirmId;
        length += sprintf ( pcInsert+length, "\"owner_id\":\"%d\",", cVar );
        cVar = gaRstpBridgeDesc[0].MaxAge;
        length += sprintf ( pcInsert+length, "\"max_age\":\"%d\",", cVar );
        cVar = gaRstpBridgeDesc[0].HelloTime;
        length += sprintf ( pcInsert+length, "\"hello_time\":\"%d\",", cVar );
        cVar = gaRstpBridgeDesc[0].TxHoldCount;
        length += sprintf ( pcInsert+length, "\"tx_hold_count\":\"%d\",", cVar );

        cVar = gaRstpBridge[0].BridgeRole;
        length += sprintf ( pcInsert+length, "\"stp_sw_role\":\"%d\",", cVar );
        cVar = gaRstpBridge[0].RootPortIdx + 1;
        length += sprintf ( pcInsert+length, "\"stp_root_port_num\":\"%d\",", cVar );

#if (MKPSH10 != 0)
        if (IsHwVersion2()) {
          memcpy(mac, gaRstpBridgeDesc[0].MAC, ETH_HWADDR_LEN);
        } else {
          memcpy(mac, gaRstpBridge[0].RootPriority.RootBridgeID.MAC.addr, ETH_HWADDR_LEN);
        }
#else
        memcpy(mac, gaRstpBridge[0].RootPriority.RootBridgeID.MAC.addr, ETH_HWADDR_LEN);
#endif
        length += sprintf ( pcInsert+length, "\"stp_root_bridge\":\"%02X:%02X:%02X:%02X:%02X:%02X\",",
                           mac[0],mac[1],mac[2],mac[3],mac[4],mac[5] );
#if (MKPSH10 != 0)
        cVar = (IsHwVersion2()) ? 1 : 0;
        length += sprintf ( pcInsert+length, "\"hw_version2\":\"%d\"", cVar );
#endif

        return length;
      }
      break;
    case STP_PORTS:
      {
        uint32_t length = 0;
        length += sprintf ( pcInsert+length, "\"enabled\":[\"%d\",\"%d\",\"%d\",\"%d\"],",
                           gaRstpBridgeDesc[0].aBPortDesc[0].portEnabled,
                           gaRstpBridgeDesc[0].aBPortDesc[1].portEnabled,
                           gaRstpBridgeDesc[0].aBPortDesc[2].portEnabled,
                           gaRstpBridgeDesc[0].aBPortDesc[3].portEnabled
                             );
        length += sprintf ( pcInsert+length, "\"admin_edge\":[\"%d\",\"%d\",\"%d\",\"%d\"],",
                           gaRstpBridgeDesc[0].aBPortDesc[0].adminEdge,
                           gaRstpBridgeDesc[0].aBPortDesc[1].adminEdge,
                           gaRstpBridgeDesc[0].aBPortDesc[2].adminEdge,
                           gaRstpBridgeDesc[0].aBPortDesc[3].adminEdge
                             );
        length += sprintf ( pcInsert+length, "\"auto_edge\":[\"%d\",\"%d\",\"%d\",\"%d\"],",
                           gaRstpBridgeDesc[0].aBPortDesc[0].autoEdge,
                           gaRstpBridgeDesc[0].aBPortDesc[1].autoEdge,
                           gaRstpBridgeDesc[0].aBPortDesc[2].autoEdge,
                           gaRstpBridgeDesc[0].aBPortDesc[3].autoEdge
                             );
        length += sprintf ( pcInsert+length, "\"port_priority\":[\"%d\",\"%d\",\"%d\",\"%d\"],",
                           gaRstpBridgeDesc[0].aBPortDesc[0].ForcePriority,
                           gaRstpBridgeDesc[0].aBPortDesc[1].ForcePriority,
                           gaRstpBridgeDesc[0].aBPortDesc[2].ForcePriority,
                           gaRstpBridgeDesc[0].aBPortDesc[3].ForcePriority
                             );
        length += sprintf ( pcInsert+length, "\"migrate_time\":[\"%d\",\"%d\",\"%d\",\"%d\"],",
                           gaRstpBridgeDesc[0].aBPortDesc[0].MigrateTime,
                           gaRstpBridgeDesc[0].aBPortDesc[1].MigrateTime,
                           gaRstpBridgeDesc[0].aBPortDesc[2].MigrateTime,
                           gaRstpBridgeDesc[0].aBPortDesc[3].MigrateTime
                            );
        length += sprintf ( pcInsert+length, "\"vlan_id\":[\"%d\",\"%d\",\"%d\",\"%d\"],",
                           gaRstpBridgeDesc[0].aBPortDesc[0].VlanId,
                           gaRstpBridgeDesc[0].aBPortDesc[1].VlanId,
                           gaRstpBridgeDesc[0].aBPortDesc[2].VlanId,
                           gaRstpBridgeDesc[0].aBPortDesc[3].VlanId
                             );

        length += sprintf ( pcInsert+length, "\"port_role\":[\"%d\",\"%d\",\"%d\",\"%d\"],",
                           gaRstpBridge[0].aPort[0].prtState,
                           gaRstpBridge[0].aPort[1].prtState,
                           gaRstpBridge[0].aPort[2].prtState,
                           gaRstpBridge[0].aPort[3].prtState
                             );
        length += sprintf ( pcInsert+length, "\"port_state\":[\"%d\",\"%d\",\"%d\",\"%d\"],",
                           gaRstpBridge[0].aPort[0].pstState,
                           gaRstpBridge[0].aPort[1].pstState,
                           gaRstpBridge[0].aPort[2].pstState,
                           gaRstpBridge[0].aPort[3].pstState
                             );
        length += sprintf ( pcInsert+length, "\"port_rx\":[\"%d\",\"%d\",\"%d\",\"%d\"],",
                           gaRstpBridge[0].aPort[0].prState,
                           gaRstpBridge[0].aPort[1].prState,
                           gaRstpBridge[0].aPort[2].prState,
                           gaRstpBridge[0].aPort[3].prState
                             );
        return length;
      }
      break;
#endif
    case SYSTEM_DEVICE_NAME:
      {
        uint32_t length = 0;
        length += sprintf ( pcInsert+length, "%s", rsettings->alias );
        return length;
      }
      break;
    case SYSTEM_SERIAL_NUM:
      {
        uint32_t length = 0;
        uint32_t sID = GetInfo_sID();
        if (sID == 0) {
          length += sprintf ( pcInsert+length, "NONE");
        } else {
          length += sprintf ( pcInsert+length, "%d", sID);
        }
        return length;
      }
      break;
    case SYSTEM_HW_VER:
      {
        uint32_t length = 0;
        length += sprintf ( pcInsert+length, "%s", GetInfo_fw() );
        return length;
      }
      break;
    case SYSTEM_FW_VER:
      {
        uint32_t length = 0;
        length += sprintf ( pcInsert+length, "%s", GetInfo_sw() );
        return length;
      }
      break;
    case SYSTEM_UP_TIME:
      {
        uint32_t length = 0;
        uint32_t time = CurrTimeInSecs();
        length = sprintf ( pcInsert, "%d", time );
        return length;
      }
      break;

#define EVENT_TAB_SIZE  (30)
  case EVENTS_TAB: //Таблица событий
      {
        uint32_t length = 0;
        //Получить количество строк, номер первого события
        int cFirstRowIdx = 1;
        int32_t cEventCount = GetEventCount();
        int32_t cFirstEvent = cEventCount - EVENT_TAB_SIZE;
        if (gTableFirstEvent < 0) {
          gTableFirstEvent = cEventCount - EVENT_TAB_SIZE;
          if (gTableFirstEvent < 0)
              gTableFirstEvent = 0;
        } else {

        }
        if (gTableFirstEvent < cFirstEvent) {
          cFirstEvent = gTableFirstEvent;
        } else {
          if (cFirstEvent < 0)
            cFirstEvent = 0;
          gTableFirstEvent = cFirstEvent;
        }
        int32_t cLastEvent = cFirstEvent + EVENT_TAB_SIZE;
        if (cLastEvent > cEventCount)
          cLastEvent = cEventCount;
        //Сформировать массивы: Номер, Время, Тип, Событие, Значение, [Предыдущее]
        struct sEvent * cpCurrEvent;
        char cTimeStr[20], cNewTimeStr[20];
        int32_t cValue;
        float * cfValue = (float *)&cValue;
        for (int i = cFirstEvent; i < cLastEvent; ++i) {
          cpCurrEvent = GetEventByIdx(i);
          GetDateTimeString(cTimeStr, cpCurrEvent->Time);
          GetDateTimeString(cNewTimeStr, cpCurrEvent->Value);
          cValue = cpCurrEvent->Value;
#define MAX_VALUE (6000000) //= 100 000 минут
          int cResValue;
          if ((cpCurrEvent->Code == ecSetSysTime) ||
              (cpCurrEvent->Code == ecSetSysTimeNtp)) {
            cResValue = cpCurrEvent->Value - cpCurrEvent->Time;
            if (cResValue >= MAX_VALUE)
              cResValue = MAX_VALUE;
            if (cResValue <= -MAX_VALUE)
              cResValue = -MAX_VALUE;
            length += sprintf ( pcInsert + length,
                               "\"event_%d\":[\"%d\",\"%s\",\"%d\",\"%d\",\"%s\",\"%d\",\"%d\"]",
                                i - cFirstEvent, cFirstRowIdx + i, cTimeStr,
                                cpCurrEvent->Type, cpCurrEvent->Code,
                                cNewTimeStr, cResValue, cpCurrEvent->Value8
                               );
          } else if ((cpCurrEvent->Code == ecTestSysTime)) {
            cResValue = cpCurrEvent->Value - cpCurrEvent->Time;
            if (cResValue >= MAX_VALUE)
              cResValue = MAX_VALUE;
            if (cResValue <= -MAX_VALUE)
              cResValue = -MAX_VALUE;
            length += sprintf ( pcInsert + length,
                               "\"event_%d\":[\"%d\",\"%s\",\"%d\",\"%d\",\"%s\",\"%d\",\"%d\"]",
                                i - cFirstEvent, cFirstRowIdx + i, cTimeStr,
                                cpCurrEvent->Type, cpCurrEvent->Code,
                                cNewTimeStr, cResValue, cpCurrEvent->Value8
                               );
          } else if (cpCurrEvent->Code == ecParamChangePort) {
            enum ePortName cPortName = (enum ePortName)cpCurrEvent->Value8;
            if ((cPortName >= pnRS485_1) && (cPortName <= pnRS485_2)) {
              cResValue = (cValue >> 8) & 0xffffff;
              if (cResValue &0x800000)
                cResValue |= 0xff000000;
              length += sprintf ( pcInsert + length,
                                 "\"event_%d\":[\"%d\",\"%s\",\"%d\",\"%d\",\"%d\",\"%d\",\"%d\"]", //,\"%d\"
                                  i - cFirstEvent, cFirstRowIdx + i, cTimeStr,
                                  cpCurrEvent->Type, cpCurrEvent->Code,
                                  cResValue, cpCurrEvent->Value8, cValue & 0xff
                                 );
            } else {
              cResValue = (cValue >> 16) & 0xffff;
              if (cResValue &0x8000)
                cResValue |= 0xffff0000;
              length += sprintf ( pcInsert + length,
                                 "\"event_%d\":[\"%d\",\"%s\",\"%d\",\"%d\",\"%d\",\"%d\",\"%d\"]",
                                  i - cFirstEvent, cFirstRowIdx + i, cTimeStr,
                                  cpCurrEvent->Type, cpCurrEvent->Code,
                                  cResValue, cpCurrEvent->Value8, cValue & 0xffff
                                 );
            }
          } else if (cpCurrEvent->Code == ecDeviceStart) {
            if (cpCurrEvent->Value != 0) {
              uint8_t ip[4]; //Если есть IP адрес - вывести в параметре [4]
              memcpy(ip, &cpCurrEvent->Value, 4);
              length += sprintf ( pcInsert + length,
                                 "\"event_%d\":[\"%d\",\"%s\",\"%d\",\"%d\",\"%d.%d.%d.%d\"]",
                                  i - cFirstEvent, cFirstRowIdx + i, cTimeStr,
                                  cpCurrEvent->Type, cpCurrEvent->Code, ip[0], ip[1], ip[2], ip[3]
                                 );
            } else {
              length += sprintf ( pcInsert + length,
                                 "\"event_%d\":[\"%d\",\"%s\",\"%d\",\"%d\",\" \"]",
                                  i - cFirstEvent, cFirstRowIdx + i, cTimeStr,
                                  cpCurrEvent->Type, cpCurrEvent->Code
                                 );
            }
          } else if (cpCurrEvent->Code == ecDeviceStop) {
            length += sprintf ( pcInsert + length,
                               "\"event_%d\":[\"%d\",\"%s\",\"%d\",\"%d\",\" \"]",
                                i - cFirstEvent, cFirstRowIdx + i, cTimeStr,
                                cpCurrEvent->Type, cpCurrEvent->Code
                               );
          } else if ((cpCurrEvent->Code == ecUploadFwCode)) { //Загрузка ПО:
            //cEvent.Value = gFwYear, gFwMonth, gFwDay, gFwSubversion
            //cEvent.Value8 = gFwVersion;
            char cDateStr[12];
            char cVersStr[8];
            GetDateString(cDateStr,
                           ((cpCurrEvent->Value >> 24) & 0xff),
                           ((cpCurrEvent->Value >> 16) & 0xff),
                           ((cpCurrEvent->Value >>  8) & 0xff));
            sprintf(cVersStr, "%d.%d", cpCurrEvent->Value8, cpCurrEvent->Value & 0xff);
            length += sprintf ( pcInsert + length,
                               "\"event_%d\":[\"%d\",\"%s\",\"%d\",\"%d\",\"%s\",\"%s\"]",
                                i - cFirstEvent, cFirstRowIdx + i, cTimeStr,
                                cpCurrEvent->Type, cpCurrEvent->Code,
                                cVersStr, cDateStr //4 = Версия; 5 = Дата
                               );
          } else if ((cpCurrEvent->Code == ecLinkErrPort) ||   //Пересброс порта по потере связи
                     (cpCurrEvent->Code == ecLinkErrSwitch) || //Пересброс свитча по потере связи
                     (cpCurrEvent->Code == ecLinkErrReboot)) { //Перезагрузка по ошибке связи
            uint8_t ip[4]; //Если есть IP адрес - вывести в параметре [4]
            memcpy(ip, &cpCurrEvent->Value, 4);
            length += sprintf ( pcInsert + length,
                               "\"event_%d\":[\"%d\",\"%s\",\"%d\",\"%d\",\"%d.%d.%d.%d\",\"%d\"]",
                                i - cFirstEvent, cFirstRowIdx + i, cTimeStr,
                                cpCurrEvent->Type, cpCurrEvent->Code, ip[0], ip[1], ip[2], ip[3],
                                cpCurrEvent->Value8
                               );
          } else if ((cpCurrEvent->Code == ecLinkOn) ||  //Порт х: Есть связь
                     (cpCurrEvent->Code == ecLinkOff)) { //Порт х: Нет связи
            length += sprintf ( pcInsert + length,
                               "\"event_%d\":[\"%d\",\"%s\",\"%d\",\"%d\",\"%d\"]",
                                i - cFirstEvent, cFirstRowIdx + i, cTimeStr,
                                cpCurrEvent->Type, cpCurrEvent->Code, cpCurrEvent->Value8
                               );
          } else {
            length += sprintf ( pcInsert + length,
                               "\"event_%d\":[\"%d\",\"%s\",\"%d\",\"%d\",\"%.1f\",\"%d\"]",
                                i - cFirstEvent, cFirstRowIdx + i, cTimeStr,
                                cpCurrEvent->Type, cpCurrEvent->Code,
                                *cfValue, cpCurrEvent->Value8
                               );
          }
          if (i != (cEventCount - 1)) {
            length += sprintf ( pcInsert + length, "," );
          }
        }
        return length;
      }
      break;
    case TIME_EV_COUNT: //Количество событий
      {
        uint32_t length = 0;
        int32_t cCount = GetEventCount();
        length += sprintf ( pcInsert+length, "%d", cCount );
        return length;
      }
      break;
    case TIME_EV_POS: //Позиция первого события в таблице
      {
        uint32_t length = 0;
        int32_t cFirstEvent;
        int32_t cEventCount;
        if (gTableFirstEvent < 0) { //Вычислить стартовую позицию
          cEventCount = GetEventCount();
          cFirstEvent = cEventCount - EVENT_TAB_SIZE;
          if (cFirstEvent < 0)
              cFirstEvent = 0;
          gTableFirstEvent = cFirstEvent;
        } else {
          cFirstEvent = cEventCount - EVENT_TAB_SIZE;
        }
        length += sprintf ( pcInsert+length, "%d", gTableFirstEvent );
        return length;
      }
      break;
    case TIME_CORR_VALUE: //Число коррекции текущее
      {
        uint32_t length = 0;
        length += sprintf ( pcInsert+length, "%d", rsettings->TimeCorr );
        return length;
      }
      break;
    case TIME_CORR_INT: //Интервал коррекции текущий
      {
        uint32_t length = 0;
        length += sprintf ( pcInsert+length, "%d", rsettings->IntervalCorr );
        return length;
      }
      break;
    case CABLE_LINK_MD: //Тест кабеля Ethernet
      {
        uint32_t length = 0;
#ifndef NO_USE_CABLE_TEST //Не использовать модуль тест кабеля
 #if (MKPSH10 != 0)
  #define ETH_CABLE_NUM (3)
  #define ETH_CABLE_START (5)
        char aParam[] = ":[\"%d\",\"%d\",\"%d\"],";
 #endif
 #if (IMC_FTX_MC != 0)
  #define ETH_CABLE_NUM (2)
  #define ETH_CABLE_START (1)
        char aParam[] = ":[\"%d\",\"%d\"],";
 #endif
        struct sCableTestResult aCableTestResult[ETH_CABLE_NUM];
        bool cIsPortRight = IsHttpPortRight(rsettings->HttpPort);
        enum eCableState cState;
        if (cIsPortRight) cState = csNormal;
        else              cState = csNoTested;
        for (int i = 0; i < ETH_CABLE_NUM; ++i) {
          //Порт связи - не проверяем
          if (cIsPortRight && (i != (rsettings->HttpPort - ETH_CABLE_START)))
            CableTest(i + ETH_CABLE_START);
        }
        for (int i = 0; i < ETH_CABLE_NUM; ++i) {
          //Порт связи - не проверяем
          if (cIsPortRight && (i != (rsettings->HttpPort - ETH_CABLE_START))) {
            CableTest(i + ETH_CABLE_START);
            aCableTestResult[i].CableState = gCableTestResult.CableState;
            aCableTestResult[i].Distance = gCableTestResult.Distance;
          } else {
            aCableTestResult[i].CableState = cState;
            aCableTestResult[i].Distance = 0;
          }
        }
        char cPattern[30];
        sprintf(cPattern, "\"cable_state\"");
        strcat(cPattern, aParam);
        length += sprintf ( pcInsert+length, cPattern,
                           aCableTestResult[0].CableState,
                           aCableTestResult[1].CableState
 #if (MKPSH10 != 0)
                           ,
                           aCableTestResult[2].CableState
 #endif
                             );
        sprintf(cPattern, "\"cable_length\"");
        strcat(cPattern, aParam);
        length += sprintf ( pcInsert+length, cPattern,
                           aCableTestResult[0].Distance,
                           aCableTestResult[1].Distance
 #if (MKPSH10 != 0)
                           ,
                           aCableTestResult[2].Distance
 #endif
                             );
#endif //NO_USE_CABLE_TEST //Не использовать модуль тест кабеля
        return length;
      }
      break;

    case SYSTEM_IP_ADDR:
      {
        uint32_t length = 0;
        uint8_t ip[4]={192,168,2,100};
        memcpy(ip,&(rsettings->ip.addr.addr),4);
        length += sprintf ( pcInsert+length, "\"ip\":\"%d.%d.%d.%d\",", ip[0],ip[1],ip[2],ip[3] );
        return length;
      }
      break;
    case SYSTEM_MAC_ADDR:
      {
        uint32_t length = 0;
        uint8_t mac[6]={0x02,0x01,0x02,0x03,0x04,0x05};
        memcpy(mac,rsettings->mac,6);
        length += sprintf ( pcInsert+length, "%02X", mac[0] );
        length += sprintf ( pcInsert+length, ":" );
        length += sprintf ( pcInsert+length, "%02X", mac[1] );
        length += sprintf ( pcInsert+length, ":" );
        length += sprintf ( pcInsert+length, "%02X", mac[2] );
        length += sprintf ( pcInsert+length, ":" );
        length += sprintf ( pcInsert+length, "%02X", mac[3] );
        length += sprintf ( pcInsert+length, ":" );
        length += sprintf ( pcInsert+length, "%02X", mac[4] );
        length += sprintf ( pcInsert+length, ":" );
        length += sprintf ( pcInsert+length, "%02X", mac[5] );
        return length;
      }
      break;
    case SYSTEM_POWER_MODEM:
      {
        uint32_t length = 0;
        length += sprintf ( pcInsert+length, "%d", (extio_read_idx(EXTIO_CPU_33_V3)==0)?1:0 );
        return length;
      }
      break;
    case SYSTEM_POWER_RS485_1:
      {
        uint32_t length = 0;
        length += sprintf ( pcInsert+length, "%d", (extio_read_idx(EXTIO_CPU_33_V5)==0)?1:0 );
        return length;
      }
      break;
    case SYSTEM_POWER_RS485_2:
      {
        uint32_t length = 0;
        length += sprintf ( pcInsert+length, "%d", (extio_read_idx(EXTIO_CPU_33_V4)==0)?1:0 );
        return length;
      }
      break;
    case SYSTEM_POWER_SWITCH_2:
      {
        uint32_t length = 0;
        int sw_iconn_state = 0;
        extern int read_switch_iconn_status();
        sw_iconn_state = read_switch_iconn_status();
        length += sprintf ( pcInsert+length, "%d", sw_iconn_state >> 1 );
        return length;
      }
      break;

///////////////////////////////////////////////////
  case SYSTEM_DMAC_CSV:
      {
        uint32_t length = 0;
#ifdef USE_SWITCH //Использовать свитч kmz8895
        uint32_t index=0;

        for(index=0;index<1000;index++)
        {
          settings_read_dmac(0, index, &(dmac.entry));
          if(dmac.entry.empty == 1) break;
          if(dmac.entry.data_ready != 0) continue;
          if(index>dmac.entry.entries) break;
  #ifdef TWO_SWITCH_ITEMS
          if(dmac.entry.port == 0 /*PORT1*/) continue;
          // Доработать, чтобы выводить все записи
          //length += sprintf ( pcInsert+length, "[");
          // mac
          length += sprintf ( pcInsert+length, "\"%02X:%02X:%02X:%02X:%02X:%02X\",",
                             dmac.entry.mac[0], dmac.entry.mac[1], dmac.entry.mac[2], dmac.entry.mac[3], dmac.entry.mac[4], dmac.entry.mac[5]);
          // port
//        //XP5//        psw=SW1;//        port=PORT_2;
//        //XP2//        psw=SW1;//        port=PORT_3;
//        //XP1//        psw=SW1;//        port=PORT_4;
          switch(dmac.entry.port)
          {
            case 4/*PORT5*/:length += sprintf ( pcInsert+length, "\"CPU\",");break;
            case 3/*PORT4*/:length += sprintf ( pcInsert+length, "\"1\",");break;
            case 2/*PORT3*/:length += sprintf ( pcInsert+length, "\"2\",");break;
            case 1/*PORT2*/:length += sprintf ( pcInsert+length, "\"5\",");break;
            case 0/*PORT1*/:length += sprintf ( pcInsert+length, "\"SW1\",");break;
            default:length += sprintf ( pcInsert+length, "\"-\",");break;
          }
          // vlan
          length += sprintf ( pcInsert+length, "\"%d\",", dmac.entry.fid );
          // vlan alias
          length += sprintf ( pcInsert+length, "\"-\""/*, dmac.entry.fid*/ );

          length += sprintf ( pcInsert+length, "\r\n");
  #else
          //if(dmac.entry.port == 0 /*PORT1*/) continue;
          // Доработать, чтобы выводить все записи
          //length += sprintf ( pcInsert+length, "[");
          // mac
          length += sprintf ( pcInsert+length, "\"%02X:%02X:%02X:%02X:%02X:%02X\",",
                             dmac.entry.mac[0], dmac.entry.mac[1], dmac.entry.mac[2], dmac.entry.mac[3], dmac.entry.mac[4], dmac.entry.mac[5]);

          switch(dmac.entry.port)
          {
            case 4/*PORT5*/:length += sprintf ( pcInsert+length, "\"CPU\",");break;
            case 3/*PORT4*/:length += sprintf ( pcInsert+length, "\"4\",");break;
            case 2/*PORT3*/:length += sprintf ( pcInsert+length, "\"3\",");break;
            case 1/*PORT2*/:length += sprintf ( pcInsert+length, "\"2\",");break;
            case 0/*PORT1*/:length += sprintf ( pcInsert+length, "\"1\",");break;
            default:length += sprintf ( pcInsert+length, "\"-\",");break;
          }
          // vlan
          length += sprintf ( pcInsert+length, "\"%d\",", dmac.entry.fid );
          // vlan alias
          length += sprintf ( pcInsert+length, "\"-\","/*, dmac.entry.fid*/ );

          length += sprintf ( pcInsert+length, "\r\n");
  #endif
        }
#endif //USE_SWITCH     //Использовать свитч kmz8895
#ifdef TWO_SWITCH_ITEMS
        for(index=0;index<1000;index++)
        {
          settings_read_dmac(1, index, &(dmac.entry));
          if(dmac.entry.empty == 1) break;
          if(dmac.entry.data_ready != 0) continue;
          if(index>dmac.entry.entries) break;
          if(dmac.entry.port == 4 /*PORT5*/) continue;
          // Доработать, чтобы выводить все записи
          // length += sprintf ( pcInsert+length, "[");
          // mac
          length += sprintf ( pcInsert+length, "\"%02X:%02X:%02X:%02X:%02X:%02X\",",
                             dmac.entry.mac[0], dmac.entry.mac[1], dmac.entry.mac[2], dmac.entry.mac[3], dmac.entry.mac[4], dmac.entry.mac[5]);
          // port

//        //XP7//        psw=SW2;//        port=PORT_1;
//        //XP6//        psw=SW2;//        port=PORT_2;
//        //XP4//        psw=SW2;//        port=PORT_3;
//        //XP3//        psw=SW2;//        port=PORT_4;
          switch(dmac.entry.port)
          {
            case 4/*PORT5*/:length += sprintf ( pcInsert+length, "\"SW2\",");break;
            case 3/*PORT4*/:length += sprintf ( pcInsert+length, "\"3\",");break;
            case 2/*PORT3*/:length += sprintf ( pcInsert+length, "\"4\",");break;
            case 1/*PORT2*/:length += sprintf ( pcInsert+length, "\"6\",");break;
            case 0/*PORT1*/:length += sprintf ( pcInsert+length, "\"7\",");break;
            default:length += sprintf ( pcInsert+length, "\"-\",");break;
          }
          // vlan
          length += sprintf ( pcInsert+length, "\"%d\",", dmac.entry.fid );
          // vlan alias
          length += sprintf ( pcInsert+length, "\"-\""/*, dmac.entry.fid*/ );

          length += sprintf ( pcInsert+length, "\r\n");
        }
#endif
        return length;
      }
      break;
  case SYSTEM_DMAC:
      {
        uint32_t length = 0;
#ifdef USE_SWITCH //Использовать свитч kmz8895
        uint32_t index=0;

        for(index=0;index<1000;index++)
        {
          settings_read_dmac(0, index, &(dmac.entry));
          if(dmac.entry.empty == 1) break;
          if(dmac.entry.data_ready != 0) continue;
          if(index>dmac.entry.entries) break;
  #if (MKPSH10 != 0)
          if(dmac.entry.port == 0 /*PORT1*/) continue;
  #endif
          // Доработать, чтобы выводить все записи
          length += sprintf ( pcInsert+length, "[");
          // mac
          length += sprintf ( pcInsert+length, "\"%02X:%02X:%02X:%02X:%02X:%02X\",",
                             dmac.entry.mac[0], dmac.entry.mac[1], dmac.entry.mac[2], dmac.entry.mac[3], dmac.entry.mac[4], dmac.entry.mac[5]);
          // port
          switch(dmac.entry.port) {
  #if (MKPSH10 != 0)
            case 4/*PORT5*/:length += sprintf ( pcInsert+length, "\"CPU\",");break;
            case 3/*PORT4*/:length += sprintf ( pcInsert+length, "\"1\",");break;
            case 2/*PORT3*/:length += sprintf ( pcInsert+length, "\"2\",");break;
            case 1/*PORT2*/:length += sprintf ( pcInsert+length, "\"5\",");break;
            case 0/*PORT1*/:length += sprintf ( pcInsert+length, "\"SW1\",");break;
  #endif
  #if (IMC_FTX_MC != 0)
            case 4/*PORT5*/:length += sprintf ( pcInsert+length, "\"CPU\",");break;
            case 3/*PORT4*/:length += sprintf ( pcInsert+length, "\"4\",");break;
            case 2/*PORT3*/:length += sprintf ( pcInsert+length, "\"3\",");break;
            case 1/*PORT2*/:length += sprintf ( pcInsert+length, "\"2\",");break;
            case 0/*PORT1*/:length += sprintf ( pcInsert+length, "\"1\",");break;
  #endif
            default:length += sprintf ( pcInsert+length, "\"-\",");break;
          }
          // vlan
          length += sprintf ( pcInsert+length, "\"%d\",", dmac.entry.fid );
          // vlan alias
          length += sprintf ( pcInsert+length, "\"-\""/*, dmac.entry.fid*/ );

          length += sprintf ( pcInsert+length, "],");
        }
#endif //USE_SWITCH //Использовать свитч kmz8895
#if (MKPSH10 != 0)
        for(index=0;index<1000;index++)
        {
          settings_read_dmac(1, index,&(dmac.entry));
          if(dmac.entry.empty == 1) break;
          if(dmac.entry.data_ready != 0) continue;
          if(index>dmac.entry.entries) break;
          if(dmac.entry.port == 4 /*PORT5*/) continue;
          // Доработать, чтобы выводить все записи
          length += sprintf ( pcInsert+length, "[");
          // mac
          length += sprintf ( pcInsert+length, "\"%02X:%02X:%02X:%02X:%02X:%02X\",",
                             dmac.entry.mac[0], dmac.entry.mac[1], dmac.entry.mac[2], dmac.entry.mac[3], dmac.entry.mac[4], dmac.entry.mac[5]);
          // port
          switch(dmac.entry.port)
          {
            case 4/*PORT5*/:length += sprintf ( pcInsert+length, "\"SW2\",");break;
            case 3/*PORT4*/:length += sprintf ( pcInsert+length, "\"3\",");break;
            case 2/*PORT3*/:length += sprintf ( pcInsert+length, "\"4\",");break;
            case 1/*PORT2*/:length += sprintf ( pcInsert+length, "\"6\",");break;
            case 0/*PORT1*/:length += sprintf ( pcInsert+length, "\"7\",");break;
            default:length += sprintf ( pcInsert+length, "\"-\",");break;
          }
          // vlan
          length += sprintf ( pcInsert+length, "\"%d\",", dmac.entry.fid );
          // vlan alias
          length += sprintf ( pcInsert+length, "\"-\""/*, dmac.entry.fid*/ );

          length += sprintf ( pcInsert+length, "],");
        }
#endif
        return length;
      }
      break;
#if (IIP != 0)
  case SYS_AP_NAME:
      {
        uint32_t length = 0;
        length += sprintf ( pcInsert+length, "%s", rsettings->IipParam.AP_Name );
        return length;
      }
      break;
  case SYS_AP_PASS:
      {
        uint32_t length = 0;
//        length += sprintf ( pcInsert+length, "%s", rsettings->IipParam.AP_Pass );
        return length;
      }
      break;
  case SYS_ESP_INFO:
      {
        uint32_t length = 0;
        extern char * GetEspInfo();
        length += sprintf ( pcInsert+length, "%s", GetEspInfo() );
        return length;
      }
      break;
  case SYS_ESP_IP_INFO:
      {
        uint32_t length = 0;
        extern char * GetEspIpInfo();
        char * cpIpInfo = GetEspIpInfo();
        length += sprintf ( pcInsert+length, "%s", cpIpInfo );
        return length;
      }
      break;
  case SYS_ESP_IP_ADDR:
      {
        uint32_t length = 0;
        extern char * GetEspIpAddr();
        length += sprintf ( pcInsert+length, "%s", GetEspIpAddr() );
        return length;
      }
      break;
  case SYS_ESP_IP_PORT:
      {
        uint32_t length = 0;
        extern uint16_t GetEspPort();
        length += sprintf ( pcInsert+length, "%d", GetEspPort() );
        return length;
      }
      break;
  case SYS_ESP_AP_STATE:
      {
        uint32_t length = 0;
        extern int8_t GetEspApConnected();
        length += sprintf ( pcInsert+length, "%d", GetEspApConnected() );
        return length;
      }
      break;
  case SYS_CONN_ON_START:
      {
        uint32_t length = 0;
        length += sprintf ( pcInsert+length, "%d", fsettings->IipParam.ConnOnStart );
        return length;
      }
      break;
#endif
 /////////////////////////////////////////////
  }
  return res_len;
}

/**
  * @brief  CGI handler for System control
  */
const char * SYSTEM_SETTINGS_CGI_Handler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[])
{
  return "/cgi/system.ssi";
}

/**
  * @brief  CGI handler for System control
  */
const char * SYSTEM_TABLE_CGI_Handler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[])
{
  return "/cgi/system.ssi";
}
/**
  * @brief  CGI handler for System control
  */
const char * SYSTEM_CGI_Handler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[])
{
//  FRESULT res;
////  FATFS fs_ram,fs_flash;
//  FIL fil;
//  UINT testBytes;
////  UINT bw;       /* Bytes written */
  switch(iIndex){
    case CGI_SYSTEM:
      {
        if(strcmp(pcParam[0],"file") == 0)
        {
//          char file_str[20];
//          memset(file_str,0,20);
//          strcpy(file_str,"RAM:/");
//          strcat(file_str,pcValue[0]);
//          res = f_open(&fil, file_str, FA_READ);
//          if(res == FR_OK)
//          {
//            memset(buffer_cgi,0,0x200000);
//            res = f_read(&fil, buffer_cgi, 0x200000, &testBytes);
//          }
//          f_close(&fil);
//        }
//        /* Check cgi parameter : example GET /leds.cgi?led=2&led=4 */
////        for (i=0; i<iNumParams; i++)
////        {
////        }
        }
      }
      break;
  }
  return "/cgi/system.ssi";
}

/**
 * Initialize SSI handlers
 */
void httpd_ssi_init(void)
{
  /* configure SSI handlers (ADC page SSI) */
  http_set_ssi_handler(httpd_handler, (char const **)TAGS, sizeof(TAGS)/sizeof(char const*)/*TAGS_COUNT*/);
}

/**
 * Initialize CGI handlers
 */
void httpd_cgi_init(void)
{

  /* configure CGI handlers (LEDs control CGI) */
  http_set_cgi_handlers(CGI_TAB, sizeof(CGI_TAB)/sizeof(tCGI));
}


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
