#include "main.h"
#include "eth.h"
#include "switch.h"
#include "settings.h"
#include "extio.h"
#include "httpd.h"
#include "service.h"
#include "statistics.h"
#include "crc.h"
#include "snmp_mib2.h"
#include "lwip\apps\snmp.h"
#include "my_mibs.h"
#include "info.h"

#include "db.h"

#include "iperf_server.h"
#include "netconf.h"
#include "fw.h"

#define SLOW_CRC32

#define CRC_INITIALVALUE 0xFFFFFFFF
extern unsigned long __checksum;
extern unsigned long __vector_table;
extern unsigned int __checksum_begin;
extern unsigned int __checksum_end;
unsigned int __checksum_zero = 0;

#ifdef USE_SWITCH //Использовать свитч kmz8895
void settings_load_switch(struct switch_settings_s*);
#endif
static void settings_load_complete();


#ifdef USE_SWITCH //Использовать свитч kmz8895
  #include "ksz8895fmq.h"
  extern ksz8895fmq_t * SW1;
  #ifdef TWO_SWITCH_ITEMS
    extern ksz8895fmq_t * SW2;
  #endif

static uint8_t buffer_active=0;
volatile uint8_t buffer_sw[2][2][256];

int read_switch_iconn_status()
{
  int retVal = 0;
  if((buffer_sw[0][0][0x1E] & (1<<5)) > 0)
  {
    retVal |= 1;
  }
 #ifdef TWO_SWITCH_ITEMS
  if((buffer_sw[1][1][0x5E] & (1<<5)) > 0)
  {
    retVal |= 2;
  }
 #endif
  return retVal;
}

uint8_t read_switch_status(int port, uint8_t reg)
{
  uint8_t regindex = 0;
 #ifdef TWO_SWITCH_ITEMS
  uint8_t sw = 0;
 #endif
  switch(reg)
  {
    case 0:
      {
        regindex = 0x09;
      }
      break;
    case 1:
      {
        regindex = 0x0E;
      }
      break;
    case 2:
      {
        regindex = 0x0F;
      }
      break;
  }
 #ifdef TWO_SWITCH_ITEMS
  switch(port)
  {
    case 6:
      {
        sw = 2;
        regindex |= 0x10;
      }
      break;
    case 5:
      {
        sw = 2;
        regindex |= 0x20;
      }
      break;
    case 4:
      {
        sw = 1;
        regindex |= 0x20;
      }
      break;
    case 3:
      {
        sw = 2;
        regindex |= 0x30;
      }
      break;
    case 2:
      {
        sw = 2;
        regindex |= 0x40;
      }
      break;
    case 1:
      {
        sw = 1;
        regindex |= 0x30;
      }
      break;
    case 0:
      {
        sw = 1;
        regindex |= 0x40;
      }
      break;
    default:
      {
        return -1;
      }
      break;
  };
  if(buffer_active)
  {
    return buffer_sw[sw-1][0][regindex];
  }
  else
  {
    return buffer_sw[sw-1][1][regindex];
  }
 #else
  switch(port)
  {
    case 3:
      {
        regindex |= 0x40;
      }
      break;
    case 2:
      {
        regindex |= 0x30;
      }
      break;
    case 1:
      {
        regindex |= 0x20;
      }
      break;
    case 0:
      {
        regindex |= 0x10;
      }
      break;

    default:
      {
        return -1;
      }
      break;
  };
  return buffer_sw[0][0][regindex];
 #endif
}
#endif

#ifndef NO_USE_VLANS
  #include "vlan.h"
#endif
#ifdef USE_STP
  #include "stp.h"
#endif
#include "ntp_settings.h"
#include "vcc_ctrl.h"
#include "temp_ctrl.h"

#ifdef USE_SDRAM
  extern void sdram_init( void );
  extern void sdram_clear( void );
#endif
#ifdef USE_SWITCH //Использовать свитч kmz8895
  extern int sw_restart( int iSwIdx );
#endif
extern void settings_dmac_table_update();

extern void ReadSettingsForVlan();
extern void ProcessLog();
extern void __reboot__();

#include "flash.h"
#include "log.h"
#include "log_flash.h"

unsigned long ticks = 0;
volatile bool gSwitchStarted = false;

extern void ping_thread(void *arg);
#include "sntp.h"
#include "extio.h"
#if (IMC_FTX_MC != 0)
  #include "can.h"
#endif
#include "default_reset.h"
#include <string.h>
#include "snmp_def.h"
#if ((UTD_M != 0) || (IIP != 0))
  #include "dev_param.h"
  #include "snmp_client.h"
  #include "rarp.h"
  extern void eth_mac_phy_read_regs();
#endif
#include "ping.h"

uint8_t port_link[7]; //Состояния связи портов свитча (фактические)

#ifdef USE_SWITCH //Использовать свитч kmz8895
#ifdef PORT_MIGRATE_TRAP //Ловушка для "миграции" по портам
  static dmac_ctrl_t dmac;
#include "eth.h"
#define PORT_MIGRATE_ARRAY_SIZE (10)
uint8_t gaPortMgrate[PORT_MIGRATE_ARRAY_SIZE], gPortMigrateIdx = 0;
eth_mac_addr gaMacAddr[PORT_MIGRATE_ARRAY_SIZE];

void checkPortMigration() {
  static portTickType sTick = 0;
  portTickType cTick = xTaskGetTickCount();
  if (sTick == 0) {
    cTick = sTick;
  } else {
    int index;
    if ((cTick - sTick) >= 1000) {
      for(index = 0; index < 1000; index++) {
        settings_read_dmac(0, index, &(dmac.entry));
        if(dmac.entry.empty == 1) break;
        if(index>dmac.entry.entries) break;
        //++ Ловушка для проверки "миграции" по портам
        switch(dmac.entry.port) {
//        case 0: //SW2
        case 2: //Единственный подключенный порт
          break;
        case 4: //ЦПУ
          if (memcmp(&dmac.entry.mac[0], &rsettings->mac[0], NETIF_MAX_HWADDR_LEN) == 0) {
            break; //Свой МАС - пропустим
          }
        default:
          if (gPortMigrateIdx < PORT_MIGRATE_ARRAY_SIZE) {
            gaPortMgrate[gPortMigrateIdx] = dmac.entry.port;
            memcpy(&gaMacAddr[gPortMigrateIdx].b[0], &dmac.entry.mac[0], NETIF_MAX_HWADDR_LEN);
            ++gPortMigrateIdx;
          }
          break;
        }
        //-- Ловушка
      }
      cTick = sTick;
    }
  }
}
#endif //PORT_MIGRATE_TRAP //Ловушка для "миграции" по портам
#endif

#if (PIXEL!=0)

  #define POWER_CAM       0    // num in mass Outs
  #define POWER_CAM_GPORT GPIOD
  #define POWER_CAM_PIN   1

  #define LED_STAT       1
  #define LED_STAT_GPORT GPIOC
  #define LED_STAT_PIN   11
  #define LIGHT_1   2
  #define LIGHT_2   3
  #define OFF (0)
  #define ON  (1)

//  {.gpio=GPIOC ,.pin= 11,.time=0,.timeout=0,.mode=EXTIO_LED_OFF}, //LED SVOBODA
//  {.gpio=GPIOD ,.pin= 0 ,.time=0,.timeout=0,.mode=EXTIO_LED_OFF}, //Rele 1 clean cam
//  {.gpio=GPIOD ,.pin= 1 ,.time=0,.timeout=0,.mode=EXTIO_LED_OFF}, // Power cam termo
//  {.gpio=GPIOE ,.pin= 13,.time=0,.timeout=0,.mode=EXTIO_LED_OFF}, // L1
//  {.gpio=GPIOE ,.pin= 14,.time=0,.timeout=0,.mode=EXTIO_LED_OFF}, // L2

  uint32_t gPowerTermStatus = 0 ;
  uint32_t gPowerDelayPixel = 3000 ;  // power delay On

#define CLN_DRV_SW_PIN       0    // num in mass Outs
#define DRIVE_PORT GPIOD
#define DRIVE   1
#define CLEAN   0

#define CLEAN_MODE  extio_gpio_out(DRIVE_PORT, CLN_DRV_SW_PIN, CLEAN)
#define DRIVE_MODE  extio_gpio_out(DRIVE_PORT, CLN_DRV_SW_PIN, DRIVE)

#define POWER_PIN       2    // Питание моторов
#define POWER_PORT GPIOD

#define POWER_ON  extio_gpio_out(POWER_PORT, POWER_PIN, ON)

#define XY_PIN       3    // реле 2 ось у/ось х
#define XY_PORT GPIOD

#define DRIVE_X   extio_gpio_out(XY_PORT, XY_PIN, ON)
#define DRIVE_Y  extio_gpio_out(XY_PORT, XY_PIN, OFF)

#define FORWARD  extio_gpio_out(GPIOE, 12, OFF)
#define BACK  extio_gpio_out(GPIOE, 12, ON)

#define PWM_OFF extio_gpio_out(GPIOE, 11, ON)
#define PWM_ON extio_gpio_out(GPIOE, 11, OFF)

#endif
//
// enum drive_mode_e {
//		UP,
//		DOWN,
//		LEFT,
//		RIGHT,
//		STOP,
//} ;
//typedef enum drive_mode_e drive_mode_t;
// struct SDrive_s{
//  uint32_t time ;
//  drive_mode_t drive_ev;
//};

struct SDrive_s gDriveParam = {
  .time   = 200,      //Канал RS485
  .drive_ev   = STOP,      //Адрес
};


void AutoMove(void ){

	//	TickType_t startTick = xTaskGetTickCount();
	//	TickType_t waitTicks = pdMS_TO_TICKS(20000);
	//	if ((xTaskGetTickCount() - startTick) >= waitTicks) {

	PWM_OFF;
	POWER_OFF;
	DRIVE_MODE;		                          //extio_gpio_out(GPIOD, 0, ON);                 // DRIVE_O
	FORWARD;
	DRIVE_X;
	POWER_ON;
	PWM_ON;
	vTaskDelay(2000);

	PWM_OFF;
	BACK;
	DRIVE_X;
	PWM_ON;
	vTaskDelay(2000);
	PWM_OFF;

	BACK;
	DRIVE_Y;

	PWM_ON;
	vTaskDelay(2000);
	PWM_OFF;

	FORWARD;
	DRIVE_Y;

	PWM_ON;
	vTaskDelay(2000);

	PWM_OFF;
	POWER_OFF;
}

void main_task(void * pvParameters)
{
  memset(port_link, 0, 7);	// сбрасываем состояние связи портов свитча

  /** Обслуживание выводов и светодиодов */
  //DefResetInit(); default settings
  extio_init();
// OUTS
  gpio_init_pp(LED_STAT_GPORT , LED_STAT_PIN);
  gpio_init_pp(POWER_CAM_GPORT, POWER_CAM_PIN);
  extio_gpio_out(LED_STAT_GPORT, LED_STAT_PIN, ON);       // PIxel
  extio_gpio_out(POWER_CAM_GPORT, POWER_CAM_PIN, ON);      // PIxel  teplo power cam

	gpio_init_pp(GPIOE , 13);
	gpio_init_pp(GPIOE , 14);
	extio_gpio_out(GPIOE,13,ON);   // Light 1
  extio_gpio_out(GPIOE,14,ON);   // Light 2



//	AutoMove();
#ifdef USE_SDRAM
  sdram_init();
  sdram_clear();
#endif
#ifdef USE_SWITCH //Использовать свитч kmz8895
  /** Инициализируем свитч */
  switch_init();          // spi интерфейс

#endif

#ifdef USE_STP
//  extern void disableSwPorts(uint8_t iSwIdx);
//  disableSwPorts(0);        // debug
#endif

  /**
   * Загружаем настройки
   */
  settings_load(settings_load_complete);

#ifdef USE_STP
  CreateTaskRstp(); //Задача RSTP/STP
#endif

  /**
   * Веб интерфейс пользователя
   */
  httpd_init();
#ifdef USE_SWITCH //Использовать свитч kmz8895
 #if (UTD_M == 0) //Для перестройки на UTD_M
  statistics_init();
 #endif
#endif

#if ((MKPSH10 != 0) || (IMC_FTX_MC != 0)) //Сервер iperf
  iperf_server_init();
#endif

#ifndef USE_SDRAM
  if (!IsSectorEmptyByIdx(CODE_START_SECTOR_REZ))
    EraseCodeArea(caReserv);
#endif
  extern struct netif gnetif;
  static const struct snmp_mib *my_snmp_mibs[] = { &mib2, &my_mib2 };

  snmp_set_auth_traps_enabled(SNMP_AUTH_TRAPS_ENABLED);
  snmp_trap_dst_ip_set(0, &gnetif.gw);
  snmp_trap_dst_enable(0, ENABLE);
  //Задать OID устройства (МКПШ = 1)
  // * device a > 1.3.6.1.4.1.XXX(ent-oid).1(devices).1(device a)
  static struct snmp_obj_id dev_snmp_obj_id = {
    .len = 8,
    .id = { 1, 3, 6, 1, 4, 1,
           SNMP_ENTERPRISE_TRANSMASH_ID, SNMP_ENTERPRISE_TRANSMASH_TRAP_ID }
  };
  snmp_set_device_enterprise_oid(&dev_snmp_obj_id);

  snmp_set_mibs(my_snmp_mibs, LWIP_ARRAYSIZE(my_snmp_mibs));

  snmp_init();


#ifdef USE_STP
  // !!!RSTP
  ProcessSwitchMAC();
#endif


  // !!! Пробы для лога
  extern void InitLog();
  InitLog();

  //Инициализация SNTP
  sntp_setoperatingmode(SNTP_OPMODE_POLL);
  sntp_init();

#if PING_USE_SOCKETS
  ping_setup(rsettings);
#endif


#ifdef USE_SUBNETS
  SetSubnets(&rsettings->VlanCfgDesc); //Установить подсети
#endif
  /**
   * Завершаем задачу инициализации
   */
  uint32_t cTickCount = xTaskGetTickCount();  // for delay power ON termocam
  extio_gpio_out(POWER_CAM_GPORT, POWER_CAM_PIN, ON);

  while(1)
  {
#if (PIXEL != 0)
	//		AutoMove();
#if (PIXEL_TEPLO != 0)
      uint32_t cTickCount1 = xTaskGetTickCount();
      if (((cTickCount1 - cTickCount) >= gPowerDelayPixel) ) {
          cTickCount = cTickCount1;
          if (!gPowerTermStatus){
              gPowerTermStatus = 0xFF;   // status ON
              extio_gpio_out(LED_STAT_GPORT, LED_STAT_PIN, ON);       // PIxel led
             // extio_gpio_out(POWER_CAM_GPORT, POWER_CAM_PIN, ON);      // PIxel  teplo power cam
          }
		  else if (gPowerTermStatus){
              gPowerTermStatus = 0;   // status ON
              extio_gpio_out(LED_STAT_GPORT, LED_STAT_PIN, OFF);       // PIxel led
             // extio_gpio_out(POWER_CAM_GPORT, POWER_CAM_PIN, ON);      // PIxel  teplo power cam
          }
      }
#endif
#endif

    SetWD(); //Watchdog внешний - перезапуск


#ifdef USE_SWITCH //Использовать свитч kmz8895
    if(buffer_active)
    {
      buffer_active=0;
      ksz8895fmq_read_registers(SW1, 0, (uint8_t *)buffer_sw[0][buffer_active], 0x60);
 #ifdef TWO_SWITCH_ITEMS
      ksz8895fmq_read_registers(SW2, 0, (uint8_t *)buffer_sw[1][buffer_active], 0x60);
 #endif
    }
    else
    {
      buffer_active=1;
      ksz8895fmq_read_registers(SW1, 0, (uint8_t *)buffer_sw[0][buffer_active], 0x60);
 #ifdef TWO_SWITCH_ITEMS
      ksz8895fmq_read_registers(SW2, 0, (uint8_t *)buffer_sw[1][buffer_active], 0x60);
 #endif
    }
    statistics_update();
#endif

    settings_dmac_table_update();
#ifdef PORT_MIGRATE_TRAP //Ловушка для "миграции" по портам
    checkPortMigration();
#endif

#ifdef USE_VLAN  //Поддержка VLAN
    //Обновление таблицы VLAN
    RefreshVlanTablePart();
    static bool sFilled = false;
    if (gIsVlanTabFull && !sFilled) {
      FillVlanDescFromSwitch(1);
      sFilled = true;
    }
    vTaskDelay(3);
#endif
#ifdef USE_DMAC  //Поддержка DMAC таблицы
    ReadSelfMac(caMac[0], SW1);
  #ifdef TWO_SWITCH_ITEMS
    ReadSelfMac(caMac[1], SW2);
  #endif
#endif
    //Мониторинг связи по портам

#ifdef USE_SWITCH //Использовать свитч kmz8895
    uint8_t cLink;
    enum eEventCode cEventCode;
    for(int i = 0; i < PORT_NUMBER; i++) {
      cLink = read_switch_status(i, 1);//0xnE
      cLink = ((cLink >> 5) & 1);
      if (cLink != port_link[i]) {
        port_link[i] = cLink;
        if (cLink) {
          cEventCode = ecLinkOn;
        } else {
          cEventCode = ecLinkOff;
        }
        SaveIntEvent(etDevice, cEventCode, i + 1, 0);
      }
    }
#endif

    ProcessLog();
  }
}

#ifndef USE_SDRAM //Использование SDRAM
 #ifdef USE_AT45DB     //Использовать Flash-память at45db
  #include "at45db.h"
 #endif
#endif
#if ((UTD_M != 0) || (IIP != 0))
  #include "eth_mac.h"
#endif
/**
  * @brief  Main program.
  * @param  None
  * @retval None
  */
int main(void)
{
  /**
   * Проверяем контрольную сумму прошивки, если что перезагружаем.
   */
  static unsigned long sum=0xFFFFFFFF;
  sum = hw_crc32( (uint8_t*)&__checksum_begin,
                  (unsigned int)&__checksum_end - (unsigned int)&__checksum_begin + 1 );
  if( sum != __checksum )
  {
    /**
     * Несовпадение CRC - перезагрузка
     */
    __reboot__();
  }
  /** Помещаем таблицу векторов в указатель таблицы.  */
  SCB->VTOR = (uint32_t)&__vector_table - 0x08000000;

  RCC_ClocksTypeDef RCC_Clocks;
  RCC_GetClocksFreq(&RCC_Clocks);     // SYSCLK(Hz)     | 168000000

#ifdef USE_AT45DB     //Использовать Flash-память at45db
  at45db_init(NULL);
#endif

#if ((UTD_M != 0) || (IIP != 0))
  extern eth_mac ETH_MAC_(0);
  ETH_MAC_(0).uninitialize();
#endif
  /**
   * Загружаем таскер и первую задачу,
   * которая производит необходимую инициализацию свитча
   */
  xTaskCreate(main_task, (int8_t *) "Main", configMINIMAL_STACK_SIZE * 3, NULL,MAIN_TASK_PRIO, NULL);

  CreateTaskLog();

  vTaskStartScheduler();
  for( ;; ); /* Тута будем при ошибке инициализации таскера. */
}
//
void settings_load_complete()
{
#ifdef USE_SWITCH //Использовать свитч kmz8895
  /**
   * Конфигурирование свитча
   */
  settings_load_switch(&(rsettings->sw));
#endif

  /**
   * Инициализация стека lwip
   */
  LwIP_Init(&(rsettings->ip));

//  /**
//   * Сервис удаленного управления
//   */
//  service_init();

  /**
   * Конфигурирование TCP/UDP/MODBUS - USART/UART/RS485
   */
  settings_load_serial_mode(0); //МКПШ = RS485-2; IMC-FTX/PIXEL  = RS485-1

}

