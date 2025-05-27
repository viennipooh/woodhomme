//******************************************************************************
// Имя файла    :       settings.c
// Заголовок    :
// Автор        :       Вахрушев Д.В.
// Дата         :       26.01.2016
//
//------------------------------------------------------------------------------
//
//
//******************************************************************************

#include "settings.h"
#include <string.h>

#include "extio.h"
#include "info.h"
#include "main.h"
#include "log.h"
#if (IIP != 0)
  #include "esp32.h"
#endif

// 'settings_version_s' - не изменять!!!
struct settings_version_s {
  uint8_t major;
  uint8_t minor;
};
struct settings_compile_s {
#define SETTINGS_VERSION_MAX_STR 16
  uint8_t date[SETTINGS_VERSION_MAX_STR];
  uint8_t configuration[SETTINGS_VERSION_MAX_STR];
};

// 15.01.21: Убираем скорости >= 1 000 000 Бод
#define RS_BAUDRATE_NUM (15) //(20)

const uint32_t rs485_Baudrate[RS_BAUDRATE_NUM] = {
	1200,
	1800,
	2400,
	4800,
	9600,
	14400,
	19200,
	28800,
	38400,
	56000,
	57600,
	115200,
	230400,
	460800,
	921600,
};

bool IsBaudrateRight(uint32_t iBaudrate) {
  for (int i = 0; i < RS_BAUDRATE_NUM; ++i) {
    if (iBaudrate == rs485_Baudrate[i])
      return true;
  }
  return false;
}

/**
 * Локальные настройки устрйоства
 */
//Позицию 'settings' в settings_flash_s не изменять!!!
// = 'settings_version_s' - не изменять
struct settings_flash_s {
  struct settings_version_s version;
  struct settings_s settings;
  struct settings_compile_s compiler;
  uint16_t crc; /* контрольная сумма настроек */
};

/*
 * Dynamic MAC table
 */
#if (MKPSH10 != 0)
  #define DMAC_TAB_NUMBER (2)
#endif
#if (IMC_FTX_MC != 0)
  #define DMAC_TAB_NUMBER (1)
#endif
#if ((UTD_M != 0) || (IIP != 0))
  #define DMAC_TAB_NUMBER (0)
#else
  int dmac_table_size[DMAC_TAB_NUMBER];
  ksz8895fmq_read_dmac_t dmac_table[DMAC_TAB_NUMBER][1000] @ ".sram";
#endif

int get_port_by_mac(char *mac)
{
#if ((UTD_M == 0) && (IIP == 0))
  for(int i=0;i<dmac_table_size[0];i++){
    if(0==memcmp(mac,dmac_table[0][i].mac,6))
    {
      switch(dmac_table[0][i].port)
      {
 #if (MKPSH10 != 0)
        case 4/*PORT5*/:return 0;break;
        case 3/*PORT4*/:return 1;break;
        case 2/*PORT3*/:return 2;break;
        case 1/*PORT2*/:return 5;break;
        case 0/*PORT1*/:break;
 #endif
 #if (IMC_FTX_MC != 0)
        case 4/*PORT5*/:return 0;break;
        case 3/*PORT4*/:return 4;break;
        case 2/*PORT3*/:return 3;break;
        case 1/*PORT2*/:return 2;break;
        case 0/*PORT1*/:return 1;break;
 #endif
      }
    }
  }
 #if (MKPSH10 != 0)
  for(int i=0;i<dmac_table_size[1];i++){
    if(0==memcmp(mac,dmac_table[1][i].mac,6))
    {
      switch(dmac_table[1][i].port)
      {
        case 4/*PORT5*/:return 0;break;
        case 3/*PORT4*/:return 3;break;
        case 2/*PORT3*/:return 4;break;
        case 1/*PORT2*/:return 6;break;
        case 0/*PORT1*/:return 7;break;
      }
    }
  }
 #endif
#endif
  return -1;
}

/***
 * Настройки по умолчанию
 */
static const struct settings_flash_s    dsettings = {
  .version = {.major = MAIN_VER_MAJ,.minor = MAIN_VER_MIN},
  .settings = {
    .ip_debug = {
      .ip       = {.addr = SETTINGS_IP_ADDR(DEBUG_IP_ADDR0,DEBUG_IP_ADDR1,DEBUG_IP_ADDR2,DEBUG_IP_ADDR3)},
      .port     = DEBUG_PORT,
    },
    .ip = {
      .addr     = {.addr = SETTINGS_IP_ADDR(IP_ADDR0,IP_ADDR1,IP_ADDR2,IP_ADDR3)},
      .mask     = {.addr = SETTINGS_IP_ADDR(NETMASK_ADDR0,NETMASK_ADDR1,NETMASK_ADDR2,NETMASK_ADDR3)},
      .gw       = {.addr = SETTINGS_IP_ADDR(GW_ADDR0,GW_ADDR1,GW_ADDR2,GW_ADDR3)},
      .type     = SETTINGS_IP_TYPE_DEFAULT
    },
    .sw = {
      .ports = {
        {
          .port_enabled         = 1,

          .autonegotiation      = SWITCH_AUTO_ENABLE,

          .speed                = SWITCH_SPEED_100,
          .full_duplex          = SWITCH_FULL_DUPLEX,
          .flow_control         = SWITCH_ENABLE_FLOW_CONTROL,
          .mdi                  = SWITCH_AUTO,
          .alias                = "Eth.1"
        },

        {
          .port_enabled         = 1,

          .autonegotiation      = SWITCH_AUTO_ENABLE,

          .speed                = SWITCH_SPEED_100,
          .full_duplex          = SWITCH_FULL_DUPLEX,
          .flow_control         = SWITCH_ENABLE_FLOW_CONTROL,
          .mdi                  = SWITCH_AUTO,
          .alias                = "Eth.2"
        },

         {
          .port_enabled         = 1,
          .autonegotiation      = SWITCH_AUTO_ENABLE,
          .speed                = SWITCH_SPEED_100,
          .full_duplex          = SWITCH_FULL_DUPLEX,
          .flow_control         = SWITCH_ENABLE_FLOW_CONTROL,
          .mdi                  = SWITCH_AUTO,
          .alias                = "Eth.3"
        },
      },
    },
#if (MKPSH10 != 0)
    .alias        = "MKPSH-10",
#endif
#if (IMC_FTX_MC != 0)
    #if (PIXEL !=0)
      .alias        = "PIXEL",
    #else
      .alias        = "IMC-FTX-MC",
    #endif
#endif
#if (UTD_M != 0)
    .alias        = "UTD-M",
#endif
#if (IIP != 0)
    .alias        = "IIP",
#endif
    .serial       = "1",
    .password     = "pass",
    .user         = "admin",
    .modempwr     = 1,
    .swpwr        = 0,
    .ip_port = {
      {
        .type = SETTINGS_IP_PORT_MODBUS_GW,
        .ip = {.addr = SETTINGS_IP_ADDR(255,255,255,255)},
        .port_local = 502,
        .port_conn = 502,
        .timeout = 0,
      },
#if (RS_PORTS_NUMBER > 1)
      {
        .type = SETTINGS_IP_PORT_MODBUS_GW,
        .ip = {.addr = SETTINGS_IP_ADDR(255,255,255,255)},
        .port_local = 501,
        .port_conn = 501,
        .timeout = 0,
      },
#endif
    },
    .uart = {
      {
        .baudrate = 9600,
        .databits = 8,
        .stopbits = SETINGS_UART_STOPBITS_1,
        .parity   = SETTINGS_UART_PARITY_NOT,
        .idle     = 1,
        .timeout  = 0,
        .alias    = "1_RS485",
      },
#if (RS_PORTS_NUMBER > 1)
      {
        .baudrate = 9600,
        .databits = 8,
        .stopbits = SETINGS_UART_STOPBITS_1,
        .parity   = SETTINGS_UART_PARITY_NOT,
        .idle     = 1,
        .timeout  = 0,
        .alias    = "2_RS485",
      },
#endif
    },
#ifdef USE_STP
    .aRstpBridgeDesc = {
      {
        .MAC = {0x00, 0x10, 0xa1, 0xff, 0xff, MAC5_PRIORITY_CURR},
        .RstpOn = 0,
        .Idx = 0,
        .Priority = DEF_BRIDGE_PRIORITY,
        .FirmId = DEF_BRIDGE_SYS_ID,
        .MaxAge = RSTP_MAX_AGE_S,
        .HelloTime = RSTP_HELLO_TIME_S,
        .ForwardDelay = RSTP_FWD_DELAY_S,
        .forceVersion = RSTP_PROT_VER,
        .TxHoldCount = DEF_TX_HOLD_COUNT,
        .aBPortDesc = { //aPortDesc
          { //1
            .PortIdx = 0,
            .adminEdge = false, //Упр
            .autoEdge  = false, //Упр
            .ForcePriority = DEF_PORT_PRIORITY,
            .MigrateTime = MIGRATE_TIME,
            .PortPathCost = DEF_PATH_COST_100,
            .rstpVersion = true,
            .stpVersion = false,
            .portEnabled = true,
            .VlanId = 0,
            .pRstpBridgeDesc = NULL,
          },
          { //2
            .PortIdx = 1,
            .adminEdge = false,
            .autoEdge  = false,
            .ForcePriority = DEF_PORT_PRIORITY,
            .MigrateTime = MIGRATE_TIME,
            .PortPathCost = DEF_PATH_COST_100,
            .rstpVersion = true,
            .stpVersion = false,
            .portEnabled = true,
            .VlanId = 0,
            .pRstpBridgeDesc = NULL,
          },
          { //3
            .PortIdx = 2,
            .adminEdge = false,
            .autoEdge  = false,
            .ForcePriority = DEF_PORT_PRIORITY,
            .MigrateTime = MIGRATE_TIME,
            .PortPathCost = DEF_PATH_COST_100,
            .rstpVersion = true,
            .stpVersion = false,
            .portEnabled = true,
            .VlanId = 0,
            .pRstpBridgeDesc = NULL,
          },
          { //4
            .PortIdx = 3,
            .adminEdge = false, //Упр
            .autoEdge  = false, //Упр
            .ForcePriority = DEF_PORT_PRIORITY, //Упр
            .MigrateTime = MIGRATE_TIME, //Упр
            .PortPathCost = DEF_PATH_COST_100,
            .rstpVersion = true,
            .stpVersion = false,
            .portEnabled = true, //Упр
            .VlanId = 0,
            .pRstpBridgeDesc = NULL,
          }
        }
      }
    },
#endif
    .NtpSettings = {
      .IpAddr        = {.addr = SETTINGS_IP_ADDR(GW_ADDR0,GW_ADDR1,GW_ADDR2,GW_ADDR3)},
      .TimeCorrMode  = tcmManual,
      .TimeZone      = TIME_ZONE,
      .PeriodNum     = 5,
      .PeriodKind    = pkHour,
    },
    .TimeCorr = 0,
    .IntervalCorr = 0,
#if (MKPSH10 != 0)
    .DevMode = dmSwitch,
#endif
    .TempWarningLevel = 75.0,
    .TempFaultLevel = 85.0,
    .PingSettings = {
#if (UTD_M != 0)
      .CtrlIsOn = true,        //Режим контроля включен
      .IpAddr   = SETTINGS_IP_ADDR(GW_ADDR0,GW_ADDR1,GW_ADDR2,GW_ADDR3), //IP-адрес для пингования
//      .IpAddr = PP_HTONL(IPADDR_LOOPBACK), //IP-адрес для пингования (default = 127.0.0.1)
#else
      .CtrlIsOn = false,        //Режим контроля включен
      .IpAddr = PP_HTONL(IPADDR_LOOPBACK), //IP-адрес для пингования (default = 127.0.0.1)
#endif
      .StartAfterBoot = 1,      //Интервал запуска контроля после загрузки
      .CtrlInterval = 1 * 60,   //Интервал между проверками (1 минута)
      .PingNum = 4,             //Количество пингов в пакете проверки
      .PingErrNum = 3,          //Количество ошибочных проверок для перезагрузки
#if (UTD_M != 0)
    },
    .UtdParam = {
      .AntType = uaDirectional,
#endif
#if (IIP != 0)
    },
    .IipParam = {
      .AP_Name = "TRANSMASH-TOMSK",
      .AP_Pass = "#TMT_2019#",
      .ip = {
        .addr     = {.addr = SETTINGS_IP_ADDR(ESP_ADDR0, ESP_ADDR1, ESP_ADDR2, ESP_ADDR3)},
        .mask     = {.addr = SETTINGS_IP_ADDR(NETMASK_ADDR0, NETMASK_ADDR1, NETMASK_ADDR2, NETMASK_ADDR3)},
        .gw       = {.addr = SETTINGS_IP_ADDR(GW_ADDR0, GW_ADDR1, GW_ADDR2, GW_ADDR3)},
        .type     = SETTINGS_IP_TYPE_STATIC
      },
      .port = 502,
      .ConnOnStart = false,
#endif
    },
#if (MKPSH10 != 0)
    .HwVersion2 = false,
#endif
  },
  .compiler = {.date = MAIN_VER_DATE, .configuration = MAIN_VER_CONFIG},
  .crc = 0
};
/**
 * Указатели на настройки с CRC, для хранения
 */
static struct settings_flash_s          rfsettings;
static struct settings_flash_s         *ffsettings = &rfsettings; // указвыет на скетор на флэше с настройками

bool gSettingsRight = false;
#if (MKPSH10 != 0)
  bool gHwVersion2 = true;
#endif

/**
 * Глобальные указательи на настройки устрйоства без CRC
 * rsettings - хранимые в оперативной памяти
 * fsettings - хранимые во флеш памяти контроллера
 */
struct settings_s              *rsettings = &(rfsettings.settings);
struct settings_s              *fsettings = 0;

static settings_load_complete_t callback = 0;

static int settings_is_load = 0;

static int check_flash_crc();
uint16_t ram_crc_p(struct settings_flash_s * ip_settings_flash);
static void settings_flash();

bool SettingsLoaded() {
  return (settings_is_load > 0);
}

uint32_t settings_get_sector_address(uint32_t sector);
void settings_ll_save(uint32_t sector,void *buffer, uint32_t len);
void settings_ll_load(uint32_t sector,void *buffer, uint32_t len);

bool ChecPokVersion() {
  if ((ffsettings->version.major != dsettings.version.major) ||
      (ffsettings->version.minor != dsettings.version.minor))
    return false;
  return true;
}

#if (MKPSH10 != 0)
bool IsSettingsEmpty() {
  uint32_t * cpSettings = (uint32_t *)settings_get_sector_address(SECTOR);
  if (*cpSettings == 0xffffffff) { //Пустой settings
    return true;
  }
  return false;
}

extern ksz8895fmq_t * SW2;
extern void ksz8895fmq_init_pwr( ksz8895fmq_t *psw );
extern void delay(uint32_t iter);
void CheckFlashSettings() { //Проверка верности settings во Flash памяти
  //Проверить фактическое наличие питания SW2
  ffsettings= (struct settings_flash_s*)settings_get_sector_address(SECTOR);
  fsettings = (struct settings_s*)&(ffsettings->settings);
  if (IsSettingsEmpty()) { //Пустой settings
    bool c3_3v_Right, c0v_Right;
    gSettingsRight = false;
    GPIO_INIT(GPIOG,11,MODE_IN,OTYPE_PP,OSPEED_VHS,PUPD_PU,0,0); // CPU_3.3V2
    delay(100000);
    c3_3v_Right = (extio_read_idx(EXTIO_CPU_33_V2) == 1) ? true : false;
    GPIO_INIT(GPIOG,11,MODE_IN,OTYPE_PP,OSPEED_VHS,PUPD_PD,0,0); // CPU_3.3V2
    delay(100000);
    c0v_Right = (extio_read_idx(EXTIO_CPU_33_V2) == 0) ? true : false;
    bool cHwVersion2 = !(c3_3v_Right && c0v_Right);
    delay(100000);
    gHwVersion2 = cHwVersion2;
    settings_default(true);
  } else {
    gHwVersion2 = fsettings->HwVersion2;
  }
}

bool IsHwVersion2() {
  return gHwVersion2;
}
#endif

extern bool gSwitchStarted;
void settings_load(settings_load_complete_t cb)
{
  if(cb)
  {
    callback = cb;
  }
  if(!settings_is_load)
  {
    rsettings = &(rfsettings.settings);

    ffsettings= (struct settings_flash_s*)settings_get_sector_address(SECTOR);
    fsettings = (struct settings_s*)&(ffsettings->settings);

    /**
     * Загружаем настройки либо с флешки, либо по умолчанию
     */
		settings_default(false);       // загружаем настройки по умолчанию
    settings_is_load = 2;
//    if(check_flash_crc() && ChecPokVersion()) // проверяем CRC и если верссии ПО не обновилась
//    {
//      settings_flash();            // загружаем настройки сохраненые во флэше
//      settings_is_load = 1;
//      gSwitchStarted = true;
//    }
//    else
//    {
//      settings_default(false);       // загружаем настройки по умолчанию
//      settings_is_load = 2;
//    }
  }
  settings_apply();
}

void settings_save_to_flash() {
  if(fsettings)
  {
    struct info_s info;
    if(info_current(&info) == 0) {
      memcpy(rfsettings.settings.aRstpBridgeDesc[0].MAC, info.mac, ETH_HWADDR_LEN);
    } else {
      memcpy(rfsettings.settings.aRstpBridgeDesc[0].MAC, rfsettings.settings.mac, ETH_HWADDR_LEN);
    }
    rfsettings.crc = ram_crc_p(&rfsettings);
    settings_ll_save(SECTOR, &rfsettings,sizeof(struct settings_flash_s));
  }
}

extern void SaveSettingsToFlash();
void settings_save()
{
#if (IIP != 0)
  memcpy(&rfsettings.settings.IipParam.ip, &rfsettings.settings.ip,
         sizeof(struct ip_settings_s));
#endif
  SaveSettingsToFlash();
}

void settings_apply()
{
  if(settings_is_load)
  {
    if(callback)
    {
      callback();
    }
  }
}

const uint8_t mac1[6] = {0x00,0x00,0x00,0x00,0x00,0x00};
const uint8_t mac2[6] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};

extern struct info_s * p_default_info;
bool IsDefaultMac(char * iMac) {
  if ((memcmp(mac1, iMac, ETH_HWADDR_LEN) == 0) ||
      (memcmp(mac2, iMac, ETH_HWADDR_LEN) == 0) ||
      (memcmp(p_default_info->mac, iMac, ETH_HWADDR_LEN) == 0))
    return true;
  return false;
}

const uint8_t mac_bridge[ETH_HWADDR_LEN] = {0x00,0x10,0xa1,0xff,0xff,0xff};

void SetMacFromUID() {
  rfsettings.settings.mac[0] = MAC_ADDR0;
  rfsettings.settings.mac[1] = MAC_ADDR1;
  rfsettings.settings.mac[2] = MAC_ADDR2;
  rfsettings.settings.mac[3] = MAC_ADDR3;
  rfsettings.settings.mac[4] = MAC_ADDR4;
  rfsettings.settings.mac[5] = MAC_ADDR5;
}

#ifdef USE_STP
void CopyRstpCfg(struct sRstpBridgeDesc * ipRstpBridgeDescSrc, struct sRstpBridgeDesc * ipRstpBridgeDescDst);
#endif
void settings_flash()
{
  struct info_s info;
  settings_ll_load(SECTOR,&rfsettings,sizeof(struct settings_flash_s));


  if(0 == info_current(&info))
  {
    if(memcmp(p_default_info->mac, info.mac,6) != 0) //Если не равно Default, то: записан в OTP
    {
      if((memcmp(mac1,rfsettings.settings.mac,6) == 0) || (memcmp(mac2,rfsettings.settings.mac,6) == 0))
      {
        memcpy(rfsettings.settings.mac, info.mac, 6);
      }
      if(memcmp(info.mac,rfsettings.settings.mac,6) != 0)
      {
        memcpy(rfsettings.settings.mac, info.mac,6);
      }
    }
  }
  if(IsDefaultMac(rfsettings.settings.mac))
  {
    SetMacFromUID();
  }
#ifdef USE_STP
  CopyRstpCfg(rfsettings.settings.aRstpBridgeDesc, gaRstpBridgeDesc);
#endif
}

struct settings_s_bak csettings_bak;

bool IsTrueAddr(uint32_t i_ipaddr) {
  if ((i_ipaddr == IPADDR_NONE) || (i_ipaddr == IPADDR_LOOPBACK) ||
      (i_ipaddr == IPADDR_ANY))
    return false;
  uint8_t cAddr0 = (i_ipaddr >>  0) & 0xff;
  uint8_t cAddr1 = (i_ipaddr >>  8) & 0xff;
  uint8_t cAddr2 = (i_ipaddr >> 16) & 0xff;
  uint8_t cAddr3 = (i_ipaddr >> 24) & 0xff;
  if ((cAddr2 == 0xff) || (cAddr3 == 0xff) ||
      (cAddr2 == 0x00) || (cAddr3 == 0x00) ||
      ((cAddr0 == 0x00) && (cAddr1 == 0x00)))
    return false;
  return true;
}

void settings_default(bool iFullReset)
{
  memcpy(&rfsettings, &dsettings, sizeof(struct settings_flash_s));
  struct info_s info;

  if(0 == info_current(&info)){
    if (IsDefaultMac((char *)info.mac)) {
      SetMacFromUID();
    } else {
      memcpy(rfsettings.settings.mac, info.mac, 6);
    }
  } else {
    SetMacFromUID();
  }

  rfsettings.settings.ip.addr.addr      = ( (uint32_t)(IP_ADDR0) |
                                            (uint32_t)(IP_ADDR1)<<8 |
                                            (uint32_t)(IP_ADDR2)<<16 |
                                            (uint32_t)(IP_ADDR3)<<24 );

  rfsettings.settings.ip.mask.addr      = ( (uint32_t)(NETMASK_ADDR0) |
                                            (uint32_t)(NETMASK_ADDR1)<<8 |
                                            (uint32_t)(NETMASK_ADDR2)<<16 |
                                            (uint32_t)(NETMASK_ADDR3)<<24 );

  rfsettings.settings.ip.gw.addr        = ( (uint32_t)(GW_ADDR0) |
                                            (uint32_t)(GW_ADDR1)<<8 |
                                            (uint32_t)(GW_ADDR2)<<16 |
                                            (uint32_t)(GW_ADDR3)<<24 );

  rfsettings.settings.ip.type           = SETTINGS_IP_TYPE_DEFAULT;
  if (!iFullReset) {
    //Скопировать settings из flash в старой структуре
    struct settings_flash_s * cffsettings= (struct settings_flash_s*)settings_get_sector_address(SECTOR);
    //Считаем, что позиция 'settings' в settings_flash_s не изменяется никогда
    struct settings_s_bak * cfsettings = (struct settings_s_bak*)&(ffsettings->settings);
    memcpy((void *)&csettings_bak, (void *)cfsettings, sizeof(struct settings_s_bak));

    if (!IsTrueAddr(csettings_bak.ip.addr.addr)) { //Ещё нет уставок
      //Пишем заводские настройки
    } else {
      memcpy((void *)&rfsettings.settings.ip,
             (void *)&csettings_bak.ip, sizeof(struct ip_settings_s));
#ifdef USE_STP
      if (!IsDefaultMac((char *)csettings_bak.aRstpBridgeDesc[0].MAC)) {
        memcpy((void *)rfsettings.settings.aRstpBridgeDesc[0].MAC,
               (void *)csettings_bak.aRstpBridgeDesc[0].MAC, ETH_HWADDR_LEN);
      } else {
        memcpy((void *)rfsettings.settings.aRstpBridgeDesc[0].MAC,
               (void *)mac_bridge, ETH_HWADDR_LEN);

      }
#endif
    }
  }
#if (MKPSH10 != 0)
  rfsettings.settings.HwVersion2 = IsHwVersion2();
#endif
  settings_save_to_flash();
  gSwitchStarted = true;  //  settings_save();
}

static int settings_check(void *f, void *s)
{
  int settings_check_flash = 0;
  char *first=(char*)f,*second=(char*)s;
  for(int i=0;i<sizeof(struct settings_flash_s);i++)
  {
    if(first[i] != second[i])
    {
      settings_check_flash = i;
      return settings_check_flash;
    }
  }
  settings_check_flash = memcmp(&rfsettings,ffsettings,sizeof(struct settings_flash_s));
  return settings_check_flash;
}
int settings_is_changed()
{
  int check = 0;
  switch(settings_is_load)
  {
    case 1:
      {
        check = settings_check(&rfsettings,ffsettings);
        if(0 == check)
        {
          return 0;
        }
      }
      break;
    case 2:
    default:
      {
        check = settings_check(&rfsettings,ffsettings);
        if(0 == check)
        {
          return 0;
        }
      }
      break;
  }
  return 1;
}


/******************************************************************************/
static const uint16_t crc16_table[] = {
    0x0000, 0xC0C1, 0xC181, 0x0140, 0xC301, 0x03C0, 0x0280, 0xC241,
    0xC601, 0x06C0, 0x0780, 0xC741, 0x0500, 0xC5C1, 0xC481, 0x0440,
    0xCC01, 0x0CC0, 0x0D80, 0xCD41, 0x0F00, 0xCFC1, 0xCE81, 0x0E40,
    0x0A00, 0xCAC1, 0xCB81, 0x0B40, 0xC901, 0x09C0, 0x0880, 0xC841,
    0xD801, 0x18C0, 0x1980, 0xD941, 0x1B00, 0xDBC1, 0xDA81, 0x1A40,
    0x1E00, 0xDEC1, 0xDF81, 0x1F40, 0xDD01, 0x1DC0, 0x1C80, 0xDC41,
    0x1400, 0xD4C1, 0xD581, 0x1540, 0xD701, 0x17C0, 0x1680, 0xD641,
    0xD201, 0x12C0, 0x1380, 0xD341, 0x1100, 0xD1C1, 0xD081, 0x1040,
    0xF001, 0x30C0, 0x3180, 0xF141, 0x3300, 0xF3C1, 0xF281, 0x3240,
    0x3600, 0xF6C1, 0xF781, 0x3740, 0xF501, 0x35C0, 0x3480, 0xF441,
    0x3C00, 0xFCC1, 0xFD81, 0x3D40, 0xFF01, 0x3FC0, 0x3E80, 0xFE41,
    0xFA01, 0x3AC0, 0x3B80, 0xFB41, 0x3900, 0xF9C1, 0xF881, 0x3840,
    0x2800, 0xE8C1, 0xE981, 0x2940, 0xEB01, 0x2BC0, 0x2A80, 0xEA41,
    0xEE01, 0x2EC0, 0x2F80, 0xEF41, 0x2D00, 0xEDC1, 0xEC81, 0x2C40,
    0xE401, 0x24C0, 0x2580, 0xE541, 0x2700, 0xE7C1, 0xE681, 0x2640,
    0x2200, 0xE2C1, 0xE381, 0x2340, 0xE101, 0x21C0, 0x2080, 0xE041,
    0xA001, 0x60C0, 0x6180, 0xA141, 0x6300, 0xA3C1, 0xA281, 0x6240,
    0x6600, 0xA6C1, 0xA781, 0x6740, 0xA501, 0x65C0, 0x6480, 0xA441,
    0x6C00, 0xACC1, 0xAD81, 0x6D40, 0xAF01, 0x6FC0, 0x6E80, 0xAE41,
    0xAA01, 0x6AC0, 0x6B80, 0xAB41, 0x6900, 0xA9C1, 0xA881, 0x6840,
    0x7800, 0xB8C1, 0xB981, 0x7940, 0xBB01, 0x7BC0, 0x7A80, 0xBA41,
    0xBE01, 0x7EC0, 0x7F80, 0xBF41, 0x7D00, 0xBDC1, 0xBC81, 0x7C40,
    0xB401, 0x74C0, 0x7580, 0xB541, 0x7700, 0xB7C1, 0xB681, 0x7640,
    0x7200, 0xB2C1, 0xB381, 0x7340, 0xB101, 0x71C0, 0x7080, 0xB041,
    0x5000, 0x90C1, 0x9181, 0x5140, 0x9301, 0x53C0, 0x5280, 0x9241,
    0x9601, 0x56C0, 0x5780, 0x9741, 0x5500, 0x95C1, 0x9481, 0x5440,
    0x9C01, 0x5CC0, 0x5D80, 0x9D41, 0x5F00, 0x9FC1, 0x9E81, 0x5E40,
    0x5A00, 0x9AC1, 0x9B81, 0x5B40, 0x9901, 0x59C0, 0x5880, 0x9841,
    0x8801, 0x48C0, 0x4980, 0x8941, 0x4B00, 0x8BC1, 0x8A81, 0x4A40,
    0x4E00, 0x8EC1, 0x8F81, 0x4F40, 0x8D01, 0x4DC0, 0x4C80, 0x8C41,
    0x4400, 0x84C1, 0x8581, 0x4540, 0x8701, 0x47C0, 0x4680, 0x8641,
    0x8201, 0x42C0, 0x4380, 0x8341, 0x4100, 0x81C1, 0x8081, 0x4040
};

uint16_t crc(char *data,uint16_t len)
{
    uint16_t crc = 0xFFFF;

    for (uint16_t i = 0; i < len; i++)
        crc = (crc >> 8) ^ crc16_table[(crc & 0xFF) ^ data[i]];

    uint8_t temp = crc >> 8;
    crc = (crc << 8) | temp;
    return crc;
}


int check_flash_crc()
{
  if ((fsettings) && (ram_crc_p(ffsettings) == ffsettings->crc))
    return 1;
  return 0;
}

uint16_t ram_crc_p(struct settings_flash_s * ip_settings_flash)
{
  return crc((char*)&(ip_settings_flash->settings),sizeof(struct settings_s));
}

void settings_dmac_table_update()
{
#ifdef USE_SWITCH //Использовать свитч kmz8895
    if(1)
    {
      int index=-1,i=0;

      dmac_table_size[0] = 0;
      for(i=0;i<1000;i++)
      {
        switch_read_dmac(0, i,&(dmac_table[0][++index]));
        if(dmac_table[0][index].empty == 1) break;
        if(dmac_table[0][index].data_ready != 0) continue;
        if(i>dmac_table[0][index].entries) break;
        if(dmac_table[0][index].port == 0 /*PORT1*/) continue;
      }
      dmac_table_size[0] = (index>=0)?index:0;
  #if (MKPSH10 != 0)
      index=-1;
      dmac_table_size[1] = 0;
      for(i=0;i<1000;i++)
      {
        switch_read_dmac(1, i,&(dmac_table[1][++index]));
        if(dmac_table[1][index].empty == 1) break;
        if(dmac_table[1][index].data_ready != 0) continue;
        if(i>dmac_table[1][index].entries) break;
        if(dmac_table[1][index].port == 4 /*PORT5*/) continue;
      }
      dmac_table_size[1] = (index>=0)?index:0;
  #endif
    }
#endif //USE_SWITCH //Использовать свитч kmz8895
}

#ifdef USE_SWITCH //Использовать свитч kmz8895
void settings_read_dmac( uint8_t sw, uint16_t index, ksz8895fmq_read_dmac_t * read ){
  memcpy(read,&dmac_table[sw][index],sizeof(ksz8895fmq_read_dmac_t));
}
#endif //USE_SWITCH

bool IsHttpPortRight(uint8_t iPort) {
  if ((iPort > 0) && (iPort <= PORT_NUMBER))
    return true;
  return false;
}

bool IsPort_MyModbus(uint16_t iPort) {
  for (int i = 0; i < RS_PORTS_NUMBER; ++i) {
    if (fsettings->ip_port[i].port_local == iPort)
      return true;
  }
  return false;
}

//Восстановить служебные символы в строке (преобразованные в '%XX')
int HexToInt(char iHex) {
  int cChar = 0;
  if ((iHex >= '0') && (iHex <= '9')) {
    cChar = (iHex - '0');
  } else if ((iHex >= 'A') && (iHex <= 'F')) {
    cChar = (iHex - 'A' + 10);
  } else if ((iHex >= 'a') && (iHex <= 'a')) {
    cChar = (iHex - 'a' + 10);
  }
  return cChar;
}

char HexToChar(char * ipHex) {
  char cChar = 0;
  if (ipHex[0] == '%') {
    cChar = '%';
  } else {
    cChar = (HexToInt(ipHex[0]) << 4) + HexToInt(ipHex[1]);
  }
  return cChar;
}

//Преобразовать строку с заменами служебных знаков в строку без замен
void RefreshHttpString(char * ipSrc, char * ipDst, int iMaxLength) {
  int cLen = strlen(ipSrc);
  int cDstIdx = 0;
  char cChr;
  if (cLen >= iMaxLength)
    cLen = iMaxLength - 1;
  for (int i = 0; i < cLen; ++i) {
    if (ipSrc[i] == '%') {
      cChr = HexToChar(&ipSrc[i + 1]);
      ipDst[cDstIdx++] = cChr;
      if (cChr == '%') {
        i += 1;
      } else {
        i += 2;
      }
    } else {
      ipDst[cDstIdx++] = ipSrc[i];
    }
  }
  ipDst[cDstIdx] = 0;
}

//Заменить все подстроки *ipOrig на *ipRepl в строке *ipStr. результат - в ipDst
void ReplaceSubstr(char * ipStr, char * ipOrig, char * ipRepl, char * ipDst) {
  char * cpFound = ipStr, * cpFoundLast = ipStr, * cpDst = ipDst;
  int cOrigLen = strlen(ipOrig), cReplLen = strlen(ipRepl);
  bool cEnd = false;
  if (strlen(ipStr) == 0) {
    ipDst[0] = 0;
    return;
  }
  while (!cEnd) {
    cpFound = strstr(cpFound, ipOrig);
    if (cpFound == NULL) {
      cEnd = true;
      if (*cpFoundLast != 0) {
        memcpy(cpDst, cpFoundLast, strlen(cpFoundLast));
        cpDst += strlen(cpFoundLast);
      }
    } else {
      int cSize = cpFound - cpFoundLast;
      if (cSize > 0) {
        memcpy(cpDst, cpFoundLast, cSize);
        cpDst += cSize;
      }
      if (cReplLen > 0) {
        memcpy(cpDst, ipRepl, cReplLen);
        cpDst += cReplLen;
      }
      cpFound += cOrigLen;
      cpFoundLast = cpFound;
    }
  }
  *cpDst = 0;
}

char gIpAddr[16];
char * IpToStr(uint32_t iIpAddr) {
  sprintf ( gIpAddr, "%d.%d.%d.%d",
            (uint8_t)((iIpAddr >>  0) & 0xff),
            (uint8_t)((iIpAddr >>  8) & 0xff),
            (uint8_t)((iIpAddr >> 16) & 0xff),
            (uint8_t)((iIpAddr >> 24) & 0xff) );
  return gIpAddr;
}
