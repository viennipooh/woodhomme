//******************************************************************************
// Имя файла    :       settings.h
// Заголовок    :       
// Автор        :       Вахрушев Д.В.
// Дата         :       26.01.2016
//
//------------------------------------------------------------------------------
//      
//      
//******************************************************************************
#ifndef __SETTINGS_H_
#define __SETTINGS_H_

#include <stdint.h>
#include "ip.h"
#include "main.h"
#include "switch.h"


#define SECTOR                          FLASH_Sector_4

#define SETTINGS_MAX_SERIAL_NAME        32
#define SETTINGS_MAX_PORT_NAME          16
#define SETTINGS_MAX_DEVICE_LEN         16
#define SETTINGS_MAX_PASS_LEN           16
#define SETTINGS_MAX_USER_LEN           16

#if BYTE_ORDER == BIG_ENDIAN
/** Set an IP address given by the four byte-parts */
#define SETTINGS_IP_ADDR(a,b,c,d) \
                        (((u32_t)((a) & 0xff) << 24) | \
                         ((u32_t)((b) & 0xff) << 16) | \
                         ((u32_t)((c) & 0xff) << 8)  | \
                          (u32_t)((d) & 0xff))
#else
/** Set an IP address given by the four byte-parts.
    Little-endian version that prevents the use of htonl. */
#define SETTINGS_IP_ADDR(a,b,c,d) \
                        (((uint32_t)((d) & 0xff) << 24) | \
                         ((uint32_t)((c) & 0xff) << 16) | \
                         ((uint32_t)((b) & 0xff) << 8)  | \
                          (uint32_t)((a) & 0xff))
#endif
                          
#if (MKPSH10 != 0)
  #define RS_PORTS_NUMBER (2)
#endif
#if (IMC_FTX_MC != 0)
  #define RS_PORTS_NUMBER (1)
#endif
#if ((UTD_M != 0) || (IIP != 0))
  #define RS_PORTS_NUMBER (2)
#endif

struct ip_client_settings_s {
  ip_addr_t addr;
  uint16_t port;
};
struct ip_server_settings_s {
  uint16_t port;
};

enum settings_ip_type_s {
  SETTINGS_IP_TYPE_DEFAULT,
  SETTINGS_IP_TYPE_STATIC,
  SETTINGS_IP_TYPE_DHCP,
};

struct ip_settings_s {
  ip_addr_t                     addr;
  ip_addr_t                     mask;
  ip_addr_t                     gw;
  enum settings_ip_type_s       type;
};

struct port_setings_s {
  int                           port_enabled;
  int                           fef;
  int                           fdd;
  switch_mdi_t                  mdi;
  switch_autonegotiation_t      autonegotiation;
  switch_speed_t                speed;
  switch_flow_control_t         flow_control;
  switch_duplex_t               full_duplex;
  char                          alias[SETTINGS_MAX_PORT_NAME];
};

struct switch_settings_s {
  struct port_setings_s ports[PORT_NUMBER];
};

enum settings_uart_parity {
  SETTINGS_UART_PARITY_NOT,
  SETTINGS_UART_PARITY_ODD,
  SETTINGS_UART_PARITY_EVEN,
  SETTINGS_UART_PARITY_MARK,
  SETTINGS_UART_PARITY_SPACE,
};

enum settings_uart_stopbits {
  SETINGS_UART_STOPBITS_1 = 1,
  SETINGS_UART_STOPBITS_1_5 = 15,
  SETINGS_UART_STOPBITS_2 = 2,
};

struct uart_port_s {
  uint32_t baudrate;
  uint8_t  databits;
  enum settings_uart_stopbits   stopbits;
  enum settings_uart_parity     parity;
  uint8_t  idle;
  uint32_t timeout;
  char alias[SETTINGS_MAX_PORT_NAME];
};

enum ip_port_mode_e {
  //Шлюз MODBUS TCP RTU
  SETTINGS_IP_PORT_MODBUS_GW,
  SETTINGS_IP_PORT_TCP_SERVER,
  SETTINGS_IP_PORT_TCP_CLIENT,
  SETTINGS_IP_PORT_UDP,
  SETTINGS_IP_PORT_SIGRAND,
  //Шлюз MODBUS RTU поверх TCP
  SETTINGS_IP_PORT_MODBUS_RTU_OVER_TCP,
  //UDP шлюзы
  SETTINGS_IP_PORT_MODBUS_GW_UDP,
  SETTINGS_IP_PORT_MODBUS_RTU_OVER_UDP,
  SETTINGS_IP_PORT_MODBUS_SERVER_SLAVE, //ModBus сервер в режиме Slave
};

struct ip_port_s {
  enum ip_port_mode_e   type;
  ip_addr_t             ip;
  uint16_t              port_conn;
  uint16_t              port_local;
  uint32_t              timeout;
  uint32_t              keepalive;
  uint32_t              inactivetime;
  char                  delimiter[2];
  char                  delimiteren[2];
};

struct ip_debug_s {
  ip_addr_t ip;
  uint16_t  port;
};

#include "ntp_settings.h"
#include "ping_settings.h"
#include "info.h"
#if (UTD_M != 0)
  // Параметры УТД-М
  //Тип антенны: Направленная, Круговая
  enum eUtdAnt { uaDirectional, uaCircular, uaCount };
  struct sUtdParam {
    enum eUtdAnt AntType;
    //
  };

#endif
#if (IIP != 0)
  struct sIipParam {
    char      AP_Name[30];    //Имя точки доступа
    char      AP_Pass[20];    //Пароль точки доступа
    struct ip_settings_s ip;  //IP адрес ESP32
//    ip_addr_t ip;             //IP адрес ESP32
    uint16_t  port;           //Порт для Modbus-сервера источника
    uint8_t   ConnOnStart;    //Подключение к ТД при вкл.питания
  };
#endif
#ifndef NO_USE_VLANS
  #include "vlan.h"
#endif
#ifdef USE_STP
  #include "stp.h"
#endif
struct settings_s {
  struct ip_settings_s          ip;
  char                          mac[6];
  char                          modempwr;
  char                          swpwr;
  struct switch_settings_s      sw;
  struct uart_port_s            uart[RS_PORTS_NUMBER];
  struct ip_port_s              ip_port[RS_PORTS_NUMBER];
  struct ip_debug_s             ip_debug;
  char alias[SETTINGS_MAX_DEVICE_LEN];
  char serial[SETTINGS_MAX_SERIAL_NAME];
  char password[SETTINGS_MAX_PASS_LEN];
  char user[SETTINGS_MAX_USER_LEN];
#ifndef NO_USE_VLANS
  struct sVlanCfgDesc           VlanCfgDesc;
#endif
#ifdef USE_STP
  struct sRstpBridgeDesc        aRstpBridgeDesc[BRIDGE_NUMBER];
#endif
  struct sNtpSettings           NtpSettings;
  uint8_t                       HttpPort;
   //Коэффициент коррекции времени. участвует в вычислении времени события
  int32_t                       TimeCorr;
  int32_t                       IntervalCorr; //Интервал коррекции
#if (MKPSH10 != 0)
  //Режим работы устройства: 
  enum eDevMode                 DevMode;
#endif
  //Температура кристалла:
  float                         TempWarningLevel; //Уровень предупреждения
  float                         TempFaultLevel;   //Уровень аварии
  struct sPingSettings          PingSettings;     //Уставки контроля связи
#if (UTD_M != 0)
  struct sUtdParam              UtdParam;
#endif
#if (IIP != 0)
  struct sIipParam              IipParam;
#endif
#if (MKPSH10 != 0)
  bool                          HwVersion2;       //Версия 2.0 платы
#endif
//  uint32_t                      TryValue; //Для принудительного обновления
};

//Структура для поддержки предыдущих версий settings_s
// Не изменять !!!
struct settings_s_bak {
  struct ip_settings_s          ip;
  char                          mac[6];
  char                          modempwr;
  char                          swpwr;
  struct switch_settings_s      sw;
  struct uart_port_s            uart[RS_PORTS_NUMBER];
  struct ip_port_s              ip_port[RS_PORTS_NUMBER];
  struct ip_debug_s             ip_debug;
  char alias[SETTINGS_MAX_DEVICE_LEN];
  char serial[SETTINGS_MAX_SERIAL_NAME];
  char password[SETTINGS_MAX_PASS_LEN];
  char user[SETTINGS_MAX_USER_LEN];
#ifdef USE_SUBNETS
  struct sVlanCfgDesc           VlanCfgDesc;
#endif
#ifdef USE_STP
  struct sRstpBridgeDesc        aRstpBridgeDesc[BRIDGE_NUMBER];
#endif
  struct sNtpSettings           NtpSettings;
  uint8_t                       HttpPort;
   //Коэффициент коррекции времени. участвует в вычислении времени события
  int32_t                       TimeCorr;
  int32_t                       IntervalCorr; //Интервал коррекции
#if (MKPSH10 != 0)
  //Режим работы устройства: 
  enum eDevMode                 DevMode;
#endif
  //Температура кристалла:
  float                         TempWarningLevel; //Уровень предупреждения
  float                         TempFaultLevel;   //Уровень аварии
//  struct sPingSettings          PingSettings;     //Уставки контроля связи
};

/**
 * Вызов функции после завершения чтения настроек
 */
typedef void (*settings_load_complete_t)(void);

/**
 * Глобально доступные указатели на настройки в оперативной памяти и во флеш
 */
extern struct settings_s        *rsettings; /* в оперативной памяти */
extern struct settings_s        *fsettings; /* во флеш */

typedef struct {
  uint16_t index;
  ksz8895fmq_read_dmac_t entry;
} dmac_ctrl_t;

typedef struct {
  uint16_t index;
  uint16_t read;
  uint16_t write;
  uint16_t del;
  uint16_t error;
  ksz8895fmq_read_smac_t entry;
} smac_ctrl_t;

typedef struct {
  uint16_t index;
  uint16_t read;
  uint16_t write;
  uint16_t del;
  uint16_t error;
  ksz8895fmq_read_vlan_t entry;
} vlan_ctrl_t;

/**
 * Загружаем настройки из флешки в оперативную память
 */
void settings_load(settings_load_complete_t);

/**
 * Сохраняем настройки из оперативной памяти во флеш
 */
void settings_save();
void settings_is_not_save();

/**
 * Применяем настройки из оперативной памяти
 */
void settings_apply();

/**
 * Загружаем в оперативную память настройки по умолчанию
 */
void settings_default(bool iFullReset);

/**
 * Проверяем изменения в настройках
 */
int settings_is_changed();

/**
 * Инициализация настроек
 */

void settings_load_serial_uart(USART_TypeDef *pusart);
void settings_load_serial_rs485(USART_TypeDef *pusart);
void settings_load_serial_mode(int iSettingIdx);

/**
* Статистика 
*/

void settings_init_smac( void );
void settings_init_vlan( void );

void settings_read_dmac( uint8_t sw, uint16_t index, ksz8895fmq_read_dmac_t * read );
void settings_write_smac( uint16_t index, ksz8895fmq_read_smac_t * write );
void settings_read_smac( uint16_t index, ksz8895fmq_read_smac_t * read );
void settings_write_vlan( uint16_t index, ksz8895fmq_read_vlan_t * write );
void settings_read_vlan( uint16_t index, ksz8895fmq_read_vlan_t * read );

int get_port_by_mac(char *mac);

#if (MKPSH10 != 0)
extern bool IsSettingsEmpty();
extern void CheckFlashSettings();
extern bool IsHwVersion2();
#endif

extern bool SettingsLoaded();
extern bool IsTrueAddr(uint32_t i_ipaddr);
extern void RefreshHttpString(char * ipSrc, char * ipDst, int iMaxLength);
extern void ReplaceSubstr(char * ipStr, char * ipOrig, char * ipRepl, char * ipDst);
extern char * IpToStr(uint32_t iIpAddr);


#endif //__SETTINGS_H_


