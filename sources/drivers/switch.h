/**
  ******************************************************************************
  * @file    switch.h
  * @author  Dmitry Vakhrushev ( vdv.18@mail.ru )
  * @version V1.0
  * @date    13.05.2016
  * @brief   --
  ******************************************************************************
  * @attention
  *
  ******************************************************************************
  */ 
  
#ifndef _SWITCH_H__
#define _SWITCH_H__

#include <stdint.h>
#include <stdbool.h>
#include "extio.h"

#include "FreeRTOS.h"
#include "task.h"
#include "lwip/sockets.h"

//Исполнение свитча:
//#define USE_OLD_FULFILLMENT
#ifdef USE_OLD_FULFILLMENT
 #define KMZ8895_FULFILLMENT CONFIG_REG_KMZ8895FMQ // Старое
#else
 #define KMZ8895_FULFILLMENT CONFIG_REG_KMZ8895FQX // Новое
#endif

#if (MKPSH10 != 0)
 #define TWO_SWITCH_ITEMS
#endif

#define ETH_SPEED_10_MBPS   (1 << 4)
#define ETH_HALF_DUPLEX     (1 << 6)
#define ETH_HALF_DUPLEX_BP  (1 << 7)

int switch_init( void );

int switch_port( uint8_t port, uint8_t state );

enum switch_port_e {
#ifdef TWO_SWITCH_ITEMS
  PORT1 = 0,
  PORT2 = 1,
  PORT3 = 2,
  PORT4 = 3,
  PORT5 = 4,
  PORT6 = 5,
  PORT7 = 6,
  OT1  = PORT1,
  OT2  = PORT2,
  OT3  = PORT3,
  OT4   = PORT4,
  ETH5   = PORT5,
  ETH6   = PORT6,
  ETH7   = PORT7,
#else
  PORT1 = 0,
  PORT2 = 1,
  PORT3 = 2,
  PORT4 = 3,  
  OT1   = PORT3,
  OT2   = PORT4,  
  ETH1  = PORT1,
  ETH2  = PORT2,  
#endif
};

enum switch_flow_control_e {
  SWITCH_DISABLE_FLOW_CONTROL,
  SWITCH_ENABLE_FLOW_CONTROL,
};
typedef enum switch_flow_control_e switch_flow_control_t;
  
enum switch_speed_e {
  SWITCH_SPEED_10,
  SWITCH_SPEED_100,
};
typedef enum switch_speed_e switch_speed_t;

enum switch_duplex_e {
  SWITCH_HALF_DUPLEX,
  SWITCH_FULL_DUPLEX,
  SWITCH_HD = SWITCH_HALF_DUPLEX,
  SWITCH_FD = SWITCH_FULL_DUPLEX,
};
typedef enum switch_duplex_e switch_duplex_t;

enum switch_autonegotiation_e {
  SWITCH_AUTO_DISABLE,
  SWITCH_AUTO_ENABLE,
};
typedef enum switch_autonegotiation_e switch_autonegotiation_t;

enum switch_mdi_e {
  SWITCH_AUTO,
  SWITCH_MDI,
  SWITCH_MDIX,
};
typedef enum switch_mdi_e switch_mdi_t;


typedef struct ksz8895fmq_read_dmac_s{
  uint16_t empty:1;
  uint16_t entries:10;
  uint16_t timestamp:2;
  uint16_t port:3;
  uint8_t data_ready:1;
  uint8_t fid:7;
  uint8_t mac[6];
}ksz8895fmq_read_dmac_t;


typedef struct ksz8895fmq_read_smac_s{
  uint8_t fid:7;
  uint8_t use_fid:1;
  
  uint8_t reserved:1;
  uint8_t override:1;
  uint8_t valid:1;
  uint8_t ports:5;
  
  uint8_t mac[6];
}ksz8895fmq_read_smac_t;

typedef struct ksz8895fmq_read_vlan_s{
  uint16_t valid:1;
  uint16_t ports:5;
  uint16_t fid:7;
}ksz8895fmq_read_vlan_t;

//Структура для сопровождения включения питания устройств (switch, Port Fiber)
enum e_PowerOnState {
  posNone,    //Нет состояния
  posOff,     //Выключен
  posPulse,   //Включение, импульс
  posPause,   //Включение, пауза
  posPWm_End, //Конец ШИМ
  posOn,      //Включен
  posCount
};

//Блок ШИМ управления питанием объекта, включаемого программно
#define POWER_ON_TIMER  TIM4
#define POWER_ON_TIMER_IRQ  TIM4_IRQn
#define POWER_ON_TICK (21)     //Тик ШИМ включения питания
#define POWER_ON_TIM_CLK (84) //Clock ШИМ включения питания (в МГц)

#define POWER_ON_PERIOD (50)  //Период ШИМ включения питания

//#define POWER_ON_TEST //Проверка ШИМ вкл. питания на выводе PD7 (82 на 144Pin)

#ifdef POWER_ON_TEST
  #define POWER_ON_PORT GPIOD   //Порт ШИМ включения питания
  #define POWER_ON_PIN  (12)    //Пин ШИМ включения питания
#else
  #define POWER_ON_PORT GPIOG   //Порт ШИМ включения питания
  #define POWER_ON_PIN  ( 7)    //Пин ШИМ включения питания
#endif

#define POWER_ON_STEP (1)     //Шаг ШИМ включения питания
#define POWER_ON_RANGE (50)   //Интервал ШИМ включения питания
typedef struct s_PowerOnItem {
  bool    On;     //Объект включен
  enum e_PowerOnState PowerOnState; //Состояние включения
  uint8_t Stage;  //Стадия ШИМ-пуска (от 0 до 50)
  
} t_PowerOnItem;

int switch_select_mdi( uint8_t port,switch_mdi_t);
int switch_select_flow_control( uint8_t port,switch_flow_control_t);
int switch_select_duplex( uint8_t port,switch_duplex_t);
int switch_select_speed( uint8_t port,switch_speed_t);
int switch_select_autonegotiation( uint8_t port,switch_autonegotiation_t);
int switch_read_dmac( uint8_t sw, uint16_t index, ksz8895fmq_read_dmac_t * read);

int switch_deinit( void );
void switch_complete( void );

extern int switch_port_power( uint8_t _port, uint8_t state );
extern int switch_port_state( uint8_t _port, uint8_t state );
extern int switch_on(int n);
extern int switch_off(int n);
extern int8_t switch_by_port(int8_t iPort);
extern bool is_port_fiber( uint8_t _port );
extern void InitPowerOn(int n);
extern t_PowerOnItem switch_PowerOnItem;
extern void SetEthOnPins();
#if (MKPSH10 != 0)
  extern void SetPWM_Pulse(int n);
#endif


#endif /* _SWITCH_H__ */


