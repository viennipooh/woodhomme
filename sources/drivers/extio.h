//******************************************************************************
// Имя файла    :       extio.h
// Заголовок    :
// Автор        :       Вахрушев Д.В.
// Дата         :       19.02.2016
//
//------------------------------------------------------------------------------
//
//
//******************************************************************************
#ifndef __EXTIO_H_
#define __EXTIO_H_

#include <stdint.h>
#include "stm32f4xx.h"

#define DEFAULT_LED_UPDATE              1
#define DEFAULT_LED_SUB                 DEFAULT_LED_UPDATE
#define DEFAULT_LED_BLINK_TIME          1000
#define DEFAULT_LED_BLINK_FAST_TIME     250
#define DEFAULT_LED_BLINK_ONCE_TIME     100
#define DEFAULT_LED_ON_TIMEOUT          2000 //Для отслеживания приема в Web

//Устройство: МКПШ10, IMC-FTX-MC, УТД-М, ИИП
#define MKPSH10       0
#define IMC_FTX_MC    1
#define UTD_M         0
#define IIP           0
#define PIXEL         1

#if (MKPSH10 != 0)
  #define USE_SDRAM
#endif
//Тип корпуса STM32Fx
#if (MKPSH10 != 0)
  #define LQFP_144
#endif
#if ((IMC_FTX_MC != 0) || (UTD_M != 0) || (IIP != 0) ||(PIXEL != 0))
  #define LQFP_100
#endif
// Количество портов Ethernet
#if (MKPSH10 != 0)
  #define PORT_NUMBER (7)
#endif
#if (IMC_FTX_MC != 0)
  #ifdef PIXEL
    #define PORT_NUMBER (3)     //  влияет на структуру static const struct settings_flash_s    dsettings = {
  #else
    #define PORT_NUMBER (4)
  #endif
#endif
#if ((UTD_M != 0) || (IIP != 0))
  #define PORT_NUMBER (1)
#endif

enum eDevCode { //Код устройства
  dcNoDev   = 0,
  dcMkpsh   = 1,
  dcImcFtx  = 2,
  dcCount,
  dcUtdM    = 51, //Начало области оконечных устройств
  dcIip     = 52, //ИИП
};

#if (MKPSH10 != 0)
  #define DEVICE_CODE (dcMkpsh)
#endif
#if (IMC_FTX_MC != 0)
  #define DEVICE_CODE (dcImcFtx)
#endif
#if (UTD_M != 0)
  #define DEVICE_CODE (dcUtdM)
#endif
#if (IIP != 0)
  #define DEVICE_CODE (dcIip)
#endif

enum extio_led_mode_e {
  EXTIO_LED_ON          = 1,
  EXTIO_LED_OFF         = 0,
  EXTIO_LED_BLINK       = 2,
  EXTIO_LED_BLINK_FAST  = 3,
  EXTIO_LED_BLINK_ONCE  = 4,
  EXTIO_LED_BLINK_ONCE_ON = 5,
};

enum extio_led_e {
  EXTIO_ETH1 = 0,
  EXTIO_ETH2 = 1,
  EXTIO_ETH3 = 2,
  EXTIO_OT1  = 3,
  EXTIO_OT2  = 4,
  EXTIO_OT3  = 5,
  EXTIO_OT4  = 6,
  EXTIO_RS232           = 9,
  EXTIO_RS485_1         = 8,
  EXTIO_RS485_2         = 7,
  EXTIO_MC   = 10,
};
enum extio_in_e { //Наличие питания
  EXTIO_CPU_33_V1,  //SWITCH_1
  EXTIO_CPU_33_V2,  //SWITCH_2
  EXTIO_CPU_33_V3,  //Модем
  EXTIO_CPU_33_V4,  //RS485_2
  EXTIO_CPU_33_V5,  //RS485_1
  EXTIO_Count
};
enum extio_pwr_e {
  EXTIO_MODEM,
  EXTIO_SWITCH,
};

#define MODE_IN  0
#define MODE_OUT 1
#define MODE_AF  2
#define MODE_AN  3

#define OTYPE_PP 0
#define OTYPE_OD 1

#define OSPEED_LS 0
#define OSPEED_MS 1
#define OSPEED_HS 2
#define OSPEED_VHS 3

#define PUPD_N   0
#define PUPD_PU  1
#define PUPD_PD  2
#define PUPD_RES 3

void extio_init(void);
int extio_read_idx(enum extio_in_e pin);
int extio_led(enum extio_led_e led, enum extio_led_mode_e mode);
int extio_led_blinked(enum extio_led_e led);
void extio_gpio_init(GPIO_TypeDef *gpio, uint8_t pin, uint8_t mode, uint8_t otype, uint8_t ospeed, uint8_t pupd, uint8_t af, uint8_t lock  );
void extio_gpio_out(GPIO_TypeDef *gpio, uint8_t pin, uint8_t set );

extern void gpio_init_pp(GPIO_TypeDef *gpio, int pin);
extern void gpio_set_pp(GPIO_TypeDef *gpio, int pin, int enabled);

#undef GPIO_OUT
#define GPIO_OUT extio_gpio_out
#undef GPIO_INIT
#define GPIO_INIT extio_gpio_init

#endif //__EXTIO_H_


