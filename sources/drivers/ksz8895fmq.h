/**
  ******************************************************************************
  * @file    ksz8895fmq.h
  * @author  Dmitry Vakhrushev ( vdv.18@mail.ru )
  * @version V1.0
  * @date    14.05.2016
  * @brief   --
  ******************************************************************************
  * @attention
  *
  ******************************************************************************
  */ 
  
#ifndef _KSZ8895FMQ_H__
#define _KSZ8895FMQ_H__

#include "stm32f4xx.h"

enum ksz8895fmq_port_e {
  PORT_1,
  PORT_2,
  PORT_3,
  PORT_4,
  PORT_5,
  P1 = PORT_1,
  P2 = PORT_2,
  P3 = PORT_3,
  P4 = PORT_4,
  P5 = PORT_5,
};
typedef enum ksz8895fmq_port_e ksz8895fmq_port_t;

enum ksz8895fmq_flow_control_e {
  DISABLE_FLOW_CONTROL,
  ENABLE_FLOW_CONTROL,
};
typedef enum ksz8895fmq_flow_control_e ksz8895fmq_flow_control_t;
  
enum ksz8895fmq_speed_e {
  SPEED_10,
  SPEED_100,
};
typedef enum ksz8895fmq_speed_e ksz8895fmq_speed_t;

enum ksz8895fmq_duplex_e {
  HALF_DUPLEX,
  FULL_DUPLEX,
  HD = HALF_DUPLEX,
  FD = FULL_DUPLEX,
};
typedef enum ksz8895fmq_duplex_e ksz8895fmq_duplex_t;

enum ksz8895fmq_autonegotiation_e {
  AUTO_DISABLE,
  AUTO_ENABLE,
};
typedef enum ksz8895fmq_autonegotiation_e ksz8895fmq_autonegotiation_t;

enum ksz8895fmq_mdi_e {
  AUTO,
  MDI,
  MDIX,
};
typedef enum ksz8895fmq_mdi_e ksz8895fmq_mdi_t;

struct ksz8895fmq_pin_s {
  GPIO_TypeDef *gpio;
  int           pin;
};
typedef struct ksz8895fmq_pin_s ksz8895fmq_pin_t;

struct ksz8895fmq_s {
  /* SPI для управления */
  SPI_TypeDef *pspi;
  /* Пины управления сбросом, питанием и выбором */
  /* Пины управления оптикой */
  ksz8895fmq_pin_t rst, pwr, cs, fb[2];
  int dma_tx,dma_rx;
};

typedef struct ksz8895fmq_s ksz8895fmq_t;

/* Инициализация свитча */
void ksz8895fmq_init_pwr( ksz8895fmq_t *psw ); //Инициализация только pwr и rst
int ksz8895fmq_init( ksz8895fmq_t *psw ); // Достаточно просто инициализировать

/* Сброс свитча */
int ksz8895fmq_reset_on( ksz8895fmq_t *psw );
int ksz8895fmq_reset_off( ksz8895fmq_t *psw );

/* Питание свитча */
int ksz8895fmq_pwr_on( ksz8895fmq_t *psw );
int ksz8895fmq_pwr_off( ksz8895fmq_t *psw );

/* Питание портов */
void ksz8895fmq_port_power_set( ksz8895fmq_t *psw, ksz8895fmq_port_t port, uint8_t state );
int ksz8895fmq_port_state_set( ksz8895fmq_t *psw, ksz8895fmq_port_t port, uint8_t state );
int ksz8895fmq_port_on( ksz8895fmq_t *psw, ksz8895fmq_port_t );
int ksz8895fmq_port_off( ksz8895fmq_t *psw, ksz8895fmq_port_t );


int ksz8895fmq_read_registers( ksz8895fmq_t *psw, uint8_t reg, uint8_t *value, uint8_t len);
int ksz8895fmq_write_registers( ksz8895fmq_t *psw, uint8_t reg, uint8_t *value, uint8_t len);
int ksz8895fmq_read_register( ksz8895fmq_t *psw, uint8_t reg, uint8_t *value);
int ksz8895fmq_write_register( ksz8895fmq_t *psw, uint8_t reg, uint8_t value);
/* Выбор режима работы LED свтеодиодов, по умолчанию вызывается в ksz8895fmq_init*/

int ksz8895fmq_select_flow_control( ksz8895fmq_t *psw, ksz8895fmq_port_t,ksz8895fmq_flow_control_t);/* требуется reset для 3-его порта*/
int ksz8895fmq_select_duplex( ksz8895fmq_t *psw, ksz8895fmq_port_t,ksz8895fmq_duplex_t);/* требуется reset*/
int ksz8895fmq_select_speed( ksz8895fmq_t *psw, ksz8895fmq_port_t,ksz8895fmq_speed_t);/* требуется reset*/
int ksz8895fmq_select_autonegotiation( ksz8895fmq_t *psw, ksz8895fmq_port_t,ksz8895fmq_autonegotiation_t);/* требуется reset */


int ksz8895fmq_deinit( ksz8895fmq_t *psw ); /* на текущий момент не реализованно */

#endif /* _KSZ8895FMQ_H__ */
