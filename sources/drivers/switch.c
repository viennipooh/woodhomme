/**
  ******************************************************************************
  * @file    switch.c
  * @author  Dmitry Vakhrushev ( vdv.18@mail.ru )
  * @version V1.0
  * @date    14.05.2016
  * @brief   --
  ******************************************************************************
  * @attention
  *
  ******************************************************************************
  */

#include <string.h>
#include "switch.h"
#include "stdarg.h"
#include "main.h"

#include "ksz8895fmq.h"
#include "spi.h"
#include "dma.h"

#include "info.h"

static const unsigned char CONFIG_REG_KMZ8895FMQ[] = {
/*0*/	0x95, 0x00, 0x04, 0x04, 0xE0, 0x00, 0x00, 0x4A,
/*8*/	0x00, 0x4C, 0x00, 0x00, 0x54, 0x00, 0x00, 0x50,
/*16*/	0x00, 0x1F, 0x06, 0x00, 0x01, 0x00, 0x00, 0x00,
/*24*/	0x30, 0x80, 0x00, 0x00, 0x5F, 0x00, 0x80, 0x01,
/*32*/	0x00, 0x1F, 0x06, 0x00, 0x01, 0x00, 0x00, 0x00,
/*40*/	0x30, 0x9E, 0x00, 0x00, 0x5F, 0x00, 0x80, 0x01,
/*48*/	0x00, 0x1F, 0x06, 0x00, 0x01, 0x00, 0x00, 0x00,
/*56*/	0x30, 0x84, 0x00, 0x00, 0x7F, 0x00, 0x80, 0x01,
/*64*/	0x00, 0x1F, 0x06, 0x00, 0x01, 0x00, 0x00, 0x00,
/*72*/	0x30, 0x84, 0x00, 0x00, 0x7F, 0x00, 0x80, 0x01,
/*80*/	0x00, 0x1F, 0x06, 0x00, 0x01, 0x00, 0x00, 0x00,
/*88*/	0x30, 0x9E, 0x00, 0x00, 0x5F, 0x00, 0x80, 0x01,
/*96*/	0x30, 0x33, 0x04, 0x04, 0x05, 0x08, 0x98, 0x27,
/*104*/	0x00, 0x10, 0xA1, 0xFF, 0xFF, 0xFF, 0x00, 0x00,
/*112*/	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/*120*/	0x00, 0x0C, 0x00, 0x2F, 0x0E, 0x00, 0x00, 0x00,
/*128*/	0x50, 0xFA, 0x84, 0x80, 0x40, 0x00, 0x00, 0x10,
/*136*/	0x00, 0x30, 0x00, 0x0C, 0x60, 0x30, 0x20, 0x20,
/*144*/	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/*152*/	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/*160*/	0x00, 0x80, 0x07, 0x00, 0x42, 0x10, 0xC0, 0x80,
/*168*/	0x80, 0x0F, 0x00, 0x20, 0x04, 0x03, 0x00, 0x20,
/*176*/	0x00, 0x00, 0x88, 0x84, 0x82, 0x81, 0x00, 0x00,
/*184*/	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80,
/*192*/	0x00, 0x00, 0x88, 0x84, 0x82, 0x81, 0x00, 0x00,
/*200*/	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x15,
/*208*/	0x00, 0x00, 0x88, 0x84, 0x82, 0x81, 0x00, 0x00,
/*216*/	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0C,
/*224*/	0x00, 0x00, 0x88, 0x84, 0x82, 0x81, 0x00, 0x00,
/*232*/	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80,
/*240*/	0x00, 0x00, 0x88, 0x84, 0x82, 0x81, 0x00, 0x00,
/*248*/	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

static const unsigned char CONFIG_REG_KMZ8895FQX[] = {
/*0*/	0x95, 0x00, 0x04, 0x04, 0xE0, 0x00, 0x00, 0x4A,
/*8*/	0x00, 0x4C, 0x00, 0x00, 0x54, 0x00, 0x00, 0x50,
/*16*/	0x00, 0x1F, 0x06, 0x00, 0x01, 0x00, 0x00, 0x00,
/*24*/	0x30, 0x80/*0x9E*/, 0x00, 0x00, 0x5F, 0x00, 0x80, 0x01,
/*32*/	0x00, 0x1F, 0x06, 0x00, 0x01, 0x00, 0x00, 0x00,
/*40*/	0x30, 0x9E, 0x00, 0x00, 0x5F, 0x00, 0x80, 0x01,
/*48*/	0x00, 0x1F, 0x06, 0x00, 0x01, 0x00, 0x00, 0x00,
/*56*/	0x30, 0x84, 0x00, 0x00, 0xFF, 0x00, 0x80, 0x01,
/*64*/	0x00, 0x1F, 0x06, 0x00, 0x01, 0x00, 0x00, 0x00,
/*72*/	0x30, 0x84, 0x00, 0x00, 0xFF, 0x00, 0x80, 0x01,
/*80*/	0x00, 0x1F, 0x06, 0x00, 0x01, 0x00, 0x00, 0x00,
/*88*/	0x30, 0x9E, 0x00, 0x00, 0x5F, 0x00, 0x80, 0x01,
/*96*/	0x30, 0x33, 0x04, 0x04, 0x05, 0x08, 0x98, 0x27,
/*104*/	0x00, 0x10, 0xA1, 0xFF, 0xFF, 0xFF, 0x00, 0x00,
/*112*/	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/*120*/	0x00, 0x0C, 0x00, 0x2F, 0x0E, 0x00, 0x00, 0x00,
/*128*/	0x50, 0xFA, 0x84, 0x80, 0x40, 0x00, 0x00, 0x10,
/*136*/	0x00, 0x30, 0x00, 0x0C, 0x60, 0x30, 0x20, 0x20,
/*144*/	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/*152*/	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/*160*/	0x00, 0x80, 0x07, 0x00, 0x42, 0x30/*0x20*/, 0xC0, 0x80,
/*168*/	0x80, 0x0F, 0x00, 0x20, 0x04, 0x03, 0x00, 0x20,
/*176*/	0x00, 0x00, 0x88, 0x84, 0x82, 0x81, 0x00, 0x00,
/*184*/	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80,
/*192*/	0x00, 0x00, 0x88, 0x84, 0x82, 0x81, 0x00, 0x00,
/*200*/	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x15,
/*208*/	0x00, 0x00, 0x88, 0x84, 0x82, 0x81, 0x00, 0x00,
/*216*/	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0C,
/*224*/	0x00, 0x00, 0x88, 0x84, 0x82, 0x81, 0x00, 0x00,
///*232*/	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xB2,       // for fiber optic type physic для оптиКИ
/*232*/	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x32,     // for couper type ля меди
/*240*/	0x00, 0x00, 0x88, 0x84, 0x82, 0x81, 0x00, 0x00,
/*248*/	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

#ifdef TWO_SWITCH_ITEMS
ksz8895fmq_t sw[2] = {
  {
    .pspi = SPI3,
    .cs = {GPIOG,14},
    .rst= {GPIOE, 4},
    .pwr= {GPIOG, 6},
    .fb = {{GPIOG,10},{GPIOG,9}},
  },
  {
    .pspi = SPI3,
    .cs = {GPIOG,12},
    .rst= {GPIOE, 3},
    .pwr= {GPIOG, 7},
    .fb = {{GPIOC,15},{GPIOC,14}},
  }
};
ksz8895fmq_t *SW1 = &sw[0];
ksz8895fmq_t *SW2 = &sw[1];
#else
ksz8895fmq_t sw[1] = {
  {
    .pspi = SPI2,
    .cs = {GPIOB, 9},
    .rst= {GPIOE, 4},               // pixel  через этот сбрасывается свитч

  #if (PIXEL!=0)
    .pwr= {NULL , NULL},               // no in Pixel debug
  #else
    .pwr= {GPIOG, 6},               //
  #endif

    .fb = {{GPIOD, 8},{GPIOD, 9}},    // in pixel not  VKL_op1/2
  }
};
ksz8895fmq_t *SW1 = &sw[0];
#endif

#include "extio.h"

void SetEthOnPins() {
  //Установить все пины, где надо обеспечить "1" при старте
#if (MKPSH10 != 0)
  ksz8895fmq_t *psw = &sw[0];
//  GPIO_SetBits(psw->pwr.gpio, (1 << psw->pwr.pin)); //SW1 не включается (в схеме)
  RCC_AHB1PeriphClockCmd(GPIO_PeriphClock(psw->fb[0].gpio) |
                         GPIO_PeriphClock(psw->fb[1].gpio), ENABLE);
  GPIO_SetBits(psw->fb[0].gpio, (1 << psw->fb[0].pin));
  GPIO_SetBits(psw->fb[1].gpio, (1 << psw->fb[1].pin));
  psw = &sw[1];
  RCC_AHB1PeriphClockCmd(GPIO_PeriphClock(psw->pwr.gpio) |
                         GPIO_PeriphClock(psw->fb[0].gpio) |
                         GPIO_PeriphClock(psw->fb[1].gpio), ENABLE);
  GPIO_SetBits(psw->pwr.gpio, (1 << psw->pwr.pin));
  GPIO_SetBits(psw->fb[0].gpio, (1 << psw->fb[0].pin));
  GPIO_SetBits(psw->fb[1].gpio, (1 << psw->fb[1].pin));
 #ifdef POWER_ON_TEST
  //Для пробы на PD12
  GPIO_SetBits(POWER_ON_PORT, (1 << POWER_ON_PIN));
 #endif
#endif
}

//Определить свитч и порт свитча по порту МКПШ
ksz8895fmq_t * SelectSwitch_Port(uint8_t _port, ksz8895fmq_port_t * oPort) {
  ksz8895fmq_t *psw;
  ksz8895fmq_port_t port;
  switch(_port)
  {
#ifdef TWO_SWITCH_ITEMS
    case 6:
      {
        // XP6
        psw=SW2;
        port=PORT_1;
      }
      break;
    case 5:
      {
        //XP5
        psw=SW2;
        port=PORT_2;
      }
      break;
    case 4:
      {
        //XP4
        psw=SW1;
        port=PORT_2;
      }
      break;
    case 3:
      {
        psw=SW2;
        port=PORT_3;
      }
      break;
    case 2:
      {
        psw=SW2;
        port=PORT_4;
      }
      break;
    case 1:
      {
        psw=SW1;
        port=PORT_3;
      }
      break;
    case 0:
      {
        psw=SW1;
        port=PORT_4;
      }
      break;
    default:
      {
        return NULL;
      }
      break;
#else
    case 3:
      {
        psw=SW1;
        port=PORT_4;
      }
      break;
    case 2:
      {
        psw=SW1;
        port=PORT_3;
      }
      break;
    case 1:
      {
        psw=SW1;
        port=PORT_2;
      }
      break;
    case 0:
      {
        psw=SW1;
        port=PORT_1;
      }
      break;
    default:
      {
        return NULL;
      }
      break;
#endif
  };
  *oPort = port;
  return psw;
}

//Определить порт МКПШ / IMC-FTX по свитчу и порту свитча
int SelectDevPort(uint8_t iSwitch, uint8_t iPort) {
  switch (iSwitch) {
  case 0:
    switch (iPort) {
#ifdef TWO_SWITCH_ITEMS
    case 1:
      return 4; //Порт 5
    case 2:
      return 1; //Порт 5
    case 3:
      return 0; //Порт 5
#else
    case 0:
      return 0; //Порт 1
    case 1:
      return 1; //Порт 2
    case 2:
      return 2; //Порт 3
    case 3:
      return 3; //Порт 4
#endif
    default:
      break;
    }
    break;
  case 1:
    switch (iPort) {
    case 0:
      return 6; //Порт 7
    case 1:
      return 5; //Порт 6
    case 2:
      return 3; //Порт 4
    case 3:
      return 2; //Порт 3
    default:
      break;
    }
    break;
  default:
    break;
  }
  return -1;
}

bool is_port_fiber( uint8_t _port ) { //Порт является оптическим
  ksz8895fmq_t *psw;
  ksz8895fmq_port_t port;
  psw = SelectSwitch_Port(_port, &port);

  if (psw == NULL) {
    return false;
  }
  return ((port == P3) || (port == P4));
}

//Только питание оптического порта
int switch_port_power( uint8_t _port, uint8_t state )
{
  ksz8895fmq_t *psw;
  ksz8895fmq_port_t port;
  psw = SelectSwitch_Port(_port, &port);

  if (psw == NULL) {
    return -1;
  }
  ksz8895fmq_port_power_set( psw, port, state );
  return 0;
}

int switch_port_state( uint8_t _port, uint8_t state )
{
  ksz8895fmq_t *psw;
  ksz8895fmq_port_t port;
  psw = SelectSwitch_Port(_port, &port);

  if (psw == NULL) {
    return -1;
  }
 #ifndef PIXEL
  if(state)
    extio_led((enum extio_led_e)_port,EXTIO_LED_ON);
  else
    extio_led((enum extio_led_e)_port,EXTIO_LED_OFF);
 #endif
  return ksz8895fmq_port_state_set(psw, port, state);
}

int switch_port( uint8_t _port, uint8_t state )
{
  ksz8895fmq_t *psw;
  ksz8895fmq_port_t port;
  psw = SelectSwitch_Port(_port, &port);

  if (psw == NULL) {
    return -1;
  }
  if(state)
  {
    #ifndef PIXEL
     extio_led((enum extio_led_e)_port,EXTIO_LED_ON);
    #endif
    return ksz8895fmq_port_on( psw, port );
  }
  #ifndef PIXEL
    extio_led((enum extio_led_e)_port,EXTIO_LED_OFF);
  #endif
  return ksz8895fmq_port_off( psw, port );
}

#include "semphr.h"
#include "stm32f4xx_tim.h"

#ifdef TWO_SWITCH_ITEMS
//Структуры для инициализации таймера
TIM_TimeBaseInitTypeDef gTIM_BaseInitStruct = {
  .TIM_Prescaler = POWER_ON_TIM_CLK / POWER_ON_TICK - 1,
  .TIM_CounterMode = TIM_CounterMode_Up,
  .TIM_Period = POWER_ON_PERIOD * POWER_ON_TICK,
  .TIM_ClockDivision = TIM_CKD_DIV1,
  .TIM_RepetitionCounter = 1
};
TIM_OCInitTypeDef gTIM_OCInitStruct = {
  .TIM_OCMode = TIM_OCMode_Timing,        /*!< Specifies the TIM mode.
                                   This parameter can be a value of @ref TIM_Output_Compare_and_PWM_modes */
  .TIM_OutputState = TIM_OutputState_Disable/*TIM_OutputState_Enable*/,   /*!< Specifies the TIM Output Compare state.
                                   This parameter can be a value of @ref TIM_Output_Compare_State */
  .TIM_OutputNState = TIM_OutputNState_Disable,  /*!< Specifies the TIM complementary Output Compare state.
                                   This parameter can be a value of @ref TIM_Output_Compare_N_State
                                   @note This parameter is valid only for TIM1 and TIM8. */
  .TIM_Pulse = POWER_ON_TICK,         /*!< Specifies the pulse value to be loaded into the Capture Compare Register.
                                   This parameter can be a number between 0x0000 and 0xFFFF */
  .TIM_OCPolarity = TIM_OCPolarity_Low,    /*!< Specifies the output polarity.
                                   This parameter can be a value of @ref TIM_Output_Compare_Polarity */
  .TIM_OCNPolarity = TIM_OCNPolarity_High,   /*!< Specifies the complementary output polarity.
                                   This parameter can be a value of @ref TIM_Output_Compare_N_Polarity
                                   @note This parameter is valid only for TIM1 and TIM8. */
  .TIM_OCIdleState = TIM_OCIdleState_Reset,  /*!< Specifies the TIM Output Compare pin state during Idle state.
                                   This parameter can be a value of @ref TIM_Output_Compare_Idle_State
                                   @note This parameter is valid only for TIM1 and TIM8. */
  .TIM_OCNIdleState = TIM_OCNIdleState_Reset,  /*!< Specifies the TIM Output Compare pin state during Idle state.
                                   This parameter can be a value of @ref TIM_Output_Compare_N_Idle_State
                                   @note This parameter is valid only for TIM1 and TIM8. */
};
t_PowerOnItem switch_PowerOnItem = {
  .On = false,
  .PowerOnState = posNone,
  .Stage = 0,
};

void SetTimer(bool iOn) {
  if (iOn) {
    /* Включаем прерывание переполнения счётчика */
    TIM_Cmd(POWER_ON_TIMER, ENABLE);
    TIM_ClearITPendingBit(POWER_ON_TIMER, TIM_IT_CC1);
    NVIC_EnableIRQ(POWER_ON_TIMER_IRQ);
  } else {
    TIM_Cmd(POWER_ON_TIMER, DISABLE);
    NVIC_DisableIRQ(POWER_ON_TIMER_IRQ);
  }
}
#endif

#ifdef TWO_SWITCH_ITEMS
  #define NO_LINEAR_PWM_PULSE
#endif
#ifdef NO_LINEAR_PWM_PULSE
  #define FIXED_PULSE_WIDTH (6)
  #define USE_DEGREE_4 //Взять степенной ряд ^4 (иначе: подобранный ряд)
#endif
void InitPowerOn(int n) {
  //Инициализация блока питания ШИМ (пока только свитча 2)
  // Инициализация выхода GPIOD 12 (81 в 144Pin)
#ifdef TWO_SWITCH_ITEMS
  if (n == 1) {
 #ifdef POWER_ON_TEST
    GPIO_SetBits(POWER_ON_PORT, (1 << POWER_ON_PIN));
    extio_gpio_init(POWER_ON_PORT, POWER_ON_PIN, GPIO_Mode_OUT, GPIO_OType_PP, GPIO_Speed_50MHz, GPIO_PuPd_NOPULL, 0, 0  );
 #endif
    // + Инициализация таймера TIM4
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
    TIM_TimeBaseInit(POWER_ON_TIMER, &gTIM_BaseInitStruct);
    TIM_SelectOnePulseMode(POWER_ON_TIMER, TIM_OPMode_Single);
    TIM_OC1Init(POWER_ON_TIMER, &gTIM_OCInitStruct);

    /* Включаем прерывание переполнения счётчика */
    TIM_ITConfig(POWER_ON_TIMER, TIM_IT_CC1, ENABLE);
    switch_PowerOnItem.PowerOnState = posOff;
  }
#endif

  ksz8895fmq_reset_on( &sw[n] );
  vTaskDelay(100);
 #ifndef PIXEL
  ksz8895fmq_pwr_off( &sw[n] );
 #endif
  vTaskDelay(100);
}

#if (MKPSH10 != 0)
uint16_t gPulse[POWER_ON_RANGE];
uint16_t gStage[POWER_ON_RANGE];
uint16_t gPulseIdx = 0;

////Степень 4 * 2
//uint8_t gStep[POWER_ON_RANGE] = {
//  1,  1,  1,  2,  2,  2,  2,  3,  3,  3,
//  4,  4,  4,  5,  5,  6,  6,  7,  7,  8,
//  8,  9,  9, 10, 10, 11, 12, 13, 13, 14,
// 15, 16, 17, 17, 18, 19, 20, 21, 22, 24,
// 25, 26, 27, 28, 30, 31, 32, 34, 35, 37
//};

////Степень 2 * 35
//uint8_t gStep[POWER_ON_RANGE] = {
//  1,  1,  1,  1,  1,  1,  1,  1,  1,  2,
//  2,  2,  3,  3,  3,  4,  4,  5,  5,  6,
//  6,  7,  7,  8,  9,  9, 10, 11, 11, 12,
// 13, 14, 15, 16, 17, 18, 19, 20, 21, 22,
// 23, 24, 25, 26, 28, 29, 30, 31, 33, 34
//};

//Подбор
uint8_t gStep[POWER_ON_RANGE] = {
  1,  2,  4,  5,  5,  6,  7,  7,  8,  8,
  8,  8,  8,  8,  9,  9,  9,  9,  9,  9,
  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,
  9,  9,  9,  9,  9, 10, 10, 10, 10, 11,
 11, 11, 12, 12, 15, 18, 23, 28, 34, 40
};

void SetPWM_Pulse(int n) {
  static uint8_t sPinValue = 0;
  if (switch_PowerOnItem.PowerOnState == posOff)
    sPinValue = 1;
  //Управление ШИМ включения
  if (sPinValue)
    POWER_ON_PORT->BSRRH = (1 << POWER_ON_PIN);
  else
    POWER_ON_PORT->BSRRL = (1 << POWER_ON_PIN);
  //
#ifdef NO_LINEAR_PWM_PULSE
  uint16_t cPulse;
 #ifdef USE_DEGREE_4 //Взять степенной ряд ^4
  cPulse = gStep[POWER_ON_RANGE - switch_PowerOnItem.Stage];
 #else
  cPulse = FIXED_PULSE_WIDTH;
  if (switch_PowerOnItem.Stage >= 35) {
    if (switch_PowerOnItem.Stage >= 45)
      cPulse -= 2;
    else
      cPulse += 1;
  } else {
    if (switch_PowerOnItem.Stage >= 20)
      cPulse += (35 - switch_PowerOnItem.Stage) >> 2;
    else {
      int16_t cDif = (20 - switch_PowerOnItem.Stage) >> 1;
//      cDif += cDif >> 1;
      cPulse += 2 + (cDif/* << 1*/);
    }
  }
 #endif
  if (cPulse >= POWER_ON_RANGE)
    cPulse = POWER_ON_RANGE - 1;
#endif
  if ((switch_PowerOnItem.PowerOnState == posOff) ||
      (switch_PowerOnItem.PowerOnState == posPause)) {
    if (--switch_PowerOnItem.Stage > 0) {
      if (gPulseIdx < POWER_ON_RANGE)
#ifdef NO_LINEAR_PWM_PULSE
        gPulse[gPulseIdx++] = cPulse;
#else
        gPulse[gPulseIdx++] = POWER_ON_RANGE - switch_PowerOnItem.Stage;
#endif
      TIM_SetCounter(POWER_ON_TIMER, 0);
#ifdef NO_LINEAR_PWM_PULSE
      TIM_SetCompare1(POWER_ON_TIMER, POWER_ON_TICK * cPulse - 5);
#else
      TIM_SetCompare1(POWER_ON_TIMER, POWER_ON_TICK * (POWER_ON_RANGE - switch_PowerOnItem.Stage) - 5);
#endif
      SetTimer(true);
      switch_PowerOnItem.PowerOnState = posPulse;
      sPinValue = 0;
//      GPIO_ResetBits(POWER_ON_PORT, (1 << POWER_ON_PIN));
    } else {
      switch_PowerOnItem.PowerOnState = posPWm_End;
      GPIO_ResetBits(POWER_ON_PORT, (1 << POWER_ON_PIN));
      SetTimer(false);
    }
  } else if (switch_PowerOnItem.PowerOnState == posPulse) {
    TIM_SetCounter(POWER_ON_TIMER, 0);
#ifdef NO_LINEAR_PWM_PULSE
    TIM_SetCompare1(POWER_ON_TIMER, POWER_ON_TICK * (POWER_ON_RANGE - cPulse));
#else
    TIM_SetCompare1(POWER_ON_TIMER, POWER_ON_TICK * switch_PowerOnItem.Stage);
#endif
//    GPIO_SetBits(POWER_ON_PORT, (1 << POWER_ON_PIN));
    SetTimer(true);
    switch_PowerOnItem.PowerOnState = posPause;
    sPinValue = 1;
  }
}

void SetPowerOn(int n) {
  //Вкл блок питания ШИМ (пока только свитча 2)
  switch_PowerOnItem.Stage = POWER_ON_RANGE;
  SetPWM_Pulse(n);
}
#endif

int switch_on(int n)
{
#if (MKPSH10 != 0)
  if (n > 0) {
 #ifdef POWER_ON_TEST
    ksz8895fmq_pwr_on( &sw[n] );
 #endif
    SetPowerOn(n);
    int8_t cTick = 5;
    while (cTick > 0) {
      vTaskDelay(3);
      if (switch_PowerOnItem.PowerOnState < posPWm_End) {
        --cTick;
      } else {
        break;
      }
    }
  } else {
    ksz8895fmq_pwr_on( &sw[n] );
  }
  vTaskDelay(100);
#endif
  ksz8895fmq_reset_off( &sw[n] );
  vTaskDelay(100);
#if (MKPSH10 != 0)
  if (n > 0) {
    switch_PowerOnItem.PowerOnState = posOn;
  }
#endif
  return 0;
}

int switch_off(int n)
{
  ksz8895fmq_reset_on( &sw[n] );
  vTaskDelay(100);
  ksz8895fmq_pwr_off( &sw[n] );
  vTaskDelay(100);
  return 0;
}

int switch_select_mdi( uint8_t port,switch_mdi_t mdi)
{
  return 0;
}

int switch_select_flow_control( uint8_t _port,switch_flow_control_t _flw)
{
  ksz8895fmq_t *psw;
  ksz8895fmq_port_t port;
  ksz8895fmq_flow_control_t flow = (ksz8895fmq_flow_control_t)_flw;
  psw = SelectSwitch_Port(_port, &port);

  return ksz8895fmq_select_flow_control( psw, port, flow );
}

int switch_select_duplex( uint8_t _port, switch_duplex_t _dp)
{
  ksz8895fmq_t *psw;
  ksz8895fmq_port_t port;
  ksz8895fmq_duplex_t duplex = (ksz8895fmq_duplex_t)_dp;
  psw = SelectSwitch_Port(_port, &port);

  return ksz8895fmq_select_duplex( psw, port, duplex );

}

int switch_select_speed( uint8_t _port,switch_speed_t _speed)
{
  ksz8895fmq_t *psw;
  ksz8895fmq_port_t port;
  ksz8895fmq_speed_t speed = (ksz8895fmq_speed_t)_speed;
  psw = SelectSwitch_Port(_port, &port);

  return ksz8895fmq_select_speed( psw, port, speed );
}

int switch_select_autonegotiation( uint8_t _port,switch_autonegotiation_t _auton)
{
  ksz8895fmq_t *psw;
  ksz8895fmq_port_t port;
  ksz8895fmq_autonegotiation_t auton = (ksz8895fmq_autonegotiation_t)_auton;
  psw = SelectSwitch_Port(_port, &port);

  return ksz8895fmq_select_autonegotiation( psw, port, auton );
}

static uint8_t *rwbuff = 0;
static uint8_t rwreg = 0;

int switch_read_dmac( uint8_t sw, uint16_t index, ksz8895fmq_read_dmac_t * read )
{
  ksz8895fmq_t *_sw;
  uint8_t write_read[9] = {0x18,0x0,};
#ifdef TWO_SWITCH_ITEMS
  if(sw)
    _sw = SW2;
  else
#endif
    _sw = SW1;
  rwbuff[0] = (1<<4) | (2<<2) | ((uint8_t)(index>>8))&0x03;
  rwbuff[1] = (uint8_t)(index & 0xFF);
  /*
  spi_select(SPI_SWITCH);
  spi_write(SPI_SWITCH, 0x02); // Write
  spi_write(SPI_SWITCH, 110);
  spi_write(SPI_SWITCH, write_read[0]);
  spi_write(SPI_SWITCH, write_read[1]);
  spi_deselect(SPI_SWITCH);
  */
  ksz8895fmq_write_registers(_sw, 110, rwbuff, 2);
  /*
  spi_select(SPI_SWITCH);
  spi_write(SPI_SWITCH, 0x03); // Read
  spi_write(SPI_SWITCH, 112);
  write_read[8] = spi_write(SPI_SWITCH, 0x00);
  write_read[7] = spi_write(SPI_SWITCH, 0x00);
  write_read[6] = spi_write(SPI_SWITCH, 0x00);
  write_read[5] = spi_write(SPI_SWITCH, 0x00);
  write_read[4] = spi_write(SPI_SWITCH, 0x00);
  write_read[3] = spi_write(SPI_SWITCH, 0x00);
  write_read[2] = spi_write(SPI_SWITCH, 0x00);
  write_read[1] = spi_write(SPI_SWITCH, 0x00);
  write_read[0] = spi_write(SPI_SWITCH, 0x00);
  spi_deselect(SPI_SWITCH);
  */
  rwreg = 112;
  ksz8895fmq_read_registers(_sw, rwreg, rwbuff, 9);
  write_read[8] = rwbuff[0];
  write_read[7] = rwbuff[1];
  write_read[6] = rwbuff[2];
  write_read[5] = rwbuff[3];
  write_read[4] = rwbuff[4];
  write_read[3] = rwbuff[5];
  write_read[2] = rwbuff[6];
  write_read[1] = rwbuff[7];
  write_read[0] = rwbuff[8];


  read->mac[0] = write_read[5];
  read->mac[1] = write_read[4];
  read->mac[2] = write_read[3];
  read->mac[3] = write_read[2];
  read->mac[4] = write_read[1];
  read->mac[5] = write_read[0];

  read->fid = write_read[6]&0x7F;
  read->data_ready = (write_read[6]&0x80) >> 7;
  read->port = write_read[7]&0x07;
  read->timestamp = (write_read[7]&0x18) >> 3;
  read->entries = (((uint16_t)write_read[7]&0xE0)>>5)|(((uint16_t)write_read[8]&0x7F)<<3);
  read->empty = ((uint16_t)write_read[8]&0x80)>>7;

  return 0;
}

#include "vcc_ctrl.h"
//int switch_restart( void )
//{
//  static uint8_t buff[256];
//  static uint8_t reg = 0x01;
//  rwbuff = buff;
//  /* Включение свитча */
//  vTaskDelay(50);
//  if (CheckVccRight(false)) {
//    switch_on(0); vTaskDelay(50);
//#ifdef TWO_SWITCH_ITEMS
//    if (CheckVccRight(false)) {
//      switch_on(1); vTaskDelay(50);
//    } else {
//      return -1;
//    }
//#endif
//  } else {
//    return -1;
//  }
//  /**
//   * Module 2 Errata Sheet
//   * http://ww1.microchip.com/downloads/en/DeviceDoc/80000697A.pdf
//   */
//  if(1)
//  {
//    const uint8_t err_module_2[7+5*2][2] = {
//      {0x47,0x01}, // data 1
//      {0x27,0x00}, // write
//      {0x37,0x00}, // indirect register address 0x00
//      {0x47,0x01}, // data 1
//      {0x27,0x00}, // write
//      {0x37,0x01}, // indirect register address 0x01
//      {0x79,0x00}, // write 0x00 to direct register 0x79
//                   // re-link each PHY port
//      /* Port PHY on/off*/
//      {0x1D,0x08}, // Port 1 PHY power down
//      {0x1D,0x00}, // Port 1 PHY power up
//      {0x2D,0x08}, //  ...
//      {0x2D,0x00},
//      {0x3D,0x08},
//      {0x3D,0x00},
//      {0x4D,0x08},
//      {0x4D,0x00},
//      {0x5D,0x08},
//      {0x5D,0x00}, // now configure the device as normal
//    };
//    reg=0x89;
//    ksz8895fmq_read_register(SW1, reg, buff);
//    buff[0]>>=4;
//    buff[0]&=0x0F;
//    if((buff[0] == 0x04) || (buff[0] == 0x05) /*A2/B2/A3/B3*/){
//      for(int i=0;i<sizeof(err_module_2)/2;i++)
//        ksz8895fmq_write_register(SW1, err_module_2[i][0], err_module_2[i][1]);
//    }
//#ifdef TWO_SWITCH_ITEMS
//    ksz8895fmq_read_register(SW2, reg, buff);
//    buff[0]>>=4;
//    buff[0]&=0x0F;
//    if((buff[0] == 0x04) || (buff[0] == 0x05) /*A2/B2/A3/B3*/){
//      for(int i=0;i<sizeof(err_module_2)/2;i++)
//        ksz8895fmq_write_register(SW2, err_module_2[i][0], err_module_2[i][1]);
//    }
//#endif
//  }
////  /**
////   * Module 3 Errata Sheet
////   * http://ww1.microchip.com/downloads/en/DeviceDoc/80000697A.pdf
////   */
////  if(0)
////  {
////    const uint8_t err_module_3[2][2] = {
////      {0xAC,0x05}, // write 0x05 to direct register 0xAC
////      {0xAD,0x01}, // write 0x01 to direct register 0xADC
////    };
////    reg=0x89;
////    ksz8895fmq_read_register(SW1, reg, buff);
////    buff[0]>>=4;
////    buff[0]&=0x0F;
////    if((buff[0] == 0x04) /*A2/B2*/){
////      for(int i=0;i<sizeof(err_module_3)/2;i++)
////        ksz8895fmq_write_register(SW1, err_module_3[i][0], err_module_3[i][1]);
////    }
////#ifdef TWO_SWITCH_ITEMS
////    ksz8895fmq_read_register(SW2, reg, buff);
////    buff[0]>>=4;
////    buff[0]&=0x0F;
////    if((buff[0] == 0x04) /*A2/B2*/){
////      for(int i=0;i<sizeof(err_module_3)/2;i++)
////        ksz8895fmq_write_register(SW2, err_module_3[i][0], err_module_3[i][1]);
////    }
////#endif
////  }
//
//  if(1)
//  {
//    struct info_s info;
//    const unsigned char *config = 0;
//    char temp = 'X';
//
//    if(info_current(&info) == 0)
//    {
//      info.serial_str[INFO_MAX_SERIAL_STRING - 1] = 0x00;
//      temp = info.serial_str[2];
//    }
//
//    switch(temp){
//      case 'x':
//      case 'X': //Новый свитч
//        config = CONFIG_REG_KMZ8895FQX;
//        break;
//      case 'q':
//      case 'Q': //Старый свитч
//        config = CONFIG_REG_KMZ8895FMQ;
//        break;
//    default:
//      config = KMZ8895_FULFILLMENT; //CONFIG_REG_KMZ8895FQX;
//      break;
//    }
//    for (int i = 0; i<=255; i++) //записываем последовательно весь массив конфигурационных регистров
//    {
//      ksz8895fmq_write_register(SW1, i, config[i]);
//#ifdef TWO_SWITCH_ITEMS
//      ksz8895fmq_write_register(SW2, i, config[i]);
//#endif
//    }
//
//  }
//
//  /* Работа портов */
//  reg = 0;
//  switch_port(0,1); vTaskDelay(50);
//  ksz8895fmq_read_registers(SW1, reg, buff, 128);
//#ifdef TWO_SWITCH_ITEMS
//  ksz8895fmq_read_registers(SW2, reg, buff, 128);
//#endif
//  memset(buff, 0, 128);
//  switch_port(1,1); vTaskDelay(50);
//  ksz8895fmq_read_registers(SW1, reg, buff, 128);
//#ifdef TWO_SWITCH_ITEMS
//  ksz8895fmq_read_registers(SW2, reg, buff, 128);
//#endif
//  memset(buff, 0, 128);
//  switch_port(2,1); vTaskDelay(50);
//  ksz8895fmq_read_registers(SW1, reg, buff, 128);
//#ifdef TWO_SWITCH_ITEMS
//  ksz8895fmq_read_registers(SW2, reg, buff, 128);
//#endif
//  memset(buff, 0, 128);
//  switch_port(3,1); vTaskDelay(50);
//  ksz8895fmq_read_registers(SW1, reg, buff, 128);
//#ifdef TWO_SWITCH_ITEMS
//  ksz8895fmq_read_registers(SW2, reg, buff, 128);
//#endif
//  memset(buff, 0, 128);
//#ifdef TWO_SWITCH_ITEMS
//  switch_port(4,1); vTaskDelay(50);
//  ksz8895fmq_read_registers(SW1, reg, buff, 128);
//  ksz8895fmq_read_registers(SW2, reg, buff, 128);
//  switch_port(5,1); vTaskDelay(50);
//  ksz8895fmq_read_registers(SW1, reg, buff, 128);
//  ksz8895fmq_read_registers(SW2, reg, buff, 128);
//  switch_port(6,1); vTaskDelay(50);
//  ksz8895fmq_read_registers(SW1, reg, buff, 128);
//  ksz8895fmq_read_registers(SW2, reg, buff, 128);
//#endif
//
//  ksz8895fmq_write_register(SW1, 0x01, 1); vTaskDelay(250);
//#ifdef TWO_SWITCH_ITEMS
//  ksz8895fmq_write_register(SW2, 0x01, 1); vTaskDelay(250);
//#endif
//
//
//
//  {
////    buff[0] |= (1<<7);
////    ksz8895fmq_write_register(SW1, 0xEF, *buff);
////    buff[0] |= (1<<7);
//#ifdef TWO_SWITCH_ITEMS
////    ksz8895fmq_write_register(SW2, 0xEF, *buff);
//#endif
////    buff[0] = 0;
////    ksz8895fmq_write_register(SW1, 0xEF, *buff);
////    buff[0] = 0;
//#ifdef TWO_SWITCH_ITEMS
////    ksz8895fmq_write_register(SW2, 0xEF, *buff);
//#endif
//    buff[0] = 0;
//
//    ksz8895fmq_read_registers(SW1, reg, buff, 128);
//#ifdef TWO_SWITCH_ITEMS
//    ksz8895fmq_read_registers(SW2, reg, buff, 128);
//#endif
//    //ksz8895fmq_read_register(SW1, 0x12, buff);
//    buff[0] = 0x1F;
//    //ksz8895fmq_write_register(SW1, 0x12, *buff);
//#ifdef TWO_SWITCH_ITEMS
//    //ksz8895fmq_write_register(SW2, 0x52, *buff);
//#endif
//
//    buff[0] = 0xFF;
//    ksz8895fmq_write_register(SW1, 0x1C, *buff);
//#ifdef TWO_SWITCH_ITEMS
//    ksz8895fmq_write_register(SW2, 0x5C, *buff);
//#endif
//
//
//    ksz8895fmq_read_registers(SW1, reg, buff, 128);
//#ifdef TWO_SWITCH_ITEMS
//    ksz8895fmq_read_registers(SW2, reg, buff, 128);
//#endif
//// !!! Порт 5: Включение 10 Mbps Speed и Half-Duplex Mode
//    ksz8895fmq_read_register(SW1, 0x06, buff);
//    uint8_t cReg = (ETH_SPEED_10_MBPS | ETH_HALF_DUPLEX);
//    buff[0] &= ~cReg;
//    buff[0] |= (0/* | ETH_SPEED_10_MBPS | ETH_HALF_DUPLEX*/);
//    ksz8895fmq_write_register(SW1, 0x06, *buff);
//// !!!
//    ksz8895fmq_read_register(SW1, 0x01, buff);
//#ifdef TWO_SWITCH_ITEMS
//    ksz8895fmq_read_register(SW2, 0x01, buff);
//#endif
//    ksz8895fmq_read_registers(SW1, reg, buff, 128);
//#ifdef TWO_SWITCH_ITEMS
//    ksz8895fmq_read_registers(SW2, reg, buff, 128);
//#endif
//    vTaskDelay(1000);
//  }
//  return 0;
//}

#if (MKPSH10 != 0)
  #include "settings.h"
#endif
int sw_restart( int iSwIdx )
{
  ksz8895fmq_t * cpSW;
  if (iSwIdx == 0)
    cpSW = SW1;
#ifdef TWO_SWITCH_ITEMS
  else
    cpSW = SW2;
#endif
  static uint8_t buff[256];
  static uint8_t reg = 0x01;
  rwbuff = buff;
  /* Включение свитча */
//  vTaskDelay(50);
//  if (CheckVccRight(false)) {
//    switch_on(iSwIdx); vTaskDelay(50);
//  } else {
//    return -1;
//  }
  /**
   * Module 2 Errata Sheet
   * http://ww1.microchip.com/downloads/en/DeviceDoc/80000697A.pdf
   */
  if(1)
  {
    const uint8_t err_module_2[7+5*2][2] = {
      {0x47,0x01}, // data 1
      {0x27,0x00}, // write
      {0x37,0x00}, // indirect register address 0x00
      {0x47,0x01}, // data 1
      {0x27,0x00}, // write
      {0x37,0x01}, // indirect register address 0x01
      {0x79,0x00}, // write 0x00 to direct register 0x79
                   // re-link each PHY port
      /* Port PHY on/off*/
      {0x1D,0x08}, // Port 1 PHY power down
      {0x1D,0x00}, // Port 1 PHY power up
      {0x2D,0x08}, //  ...
      {0x2D,0x00},
      {0x3D,0x08},
      {0x3D,0x00},
      {0x4D,0x08},
      {0x4D,0x00},
      {0x5D,0x08},
      {0x5D,0x00}, // now configure the device as normal
    };
    reg=0x89;
    ksz8895fmq_read_register(cpSW, reg, buff);
    buff[0]>>=4;
    buff[0]&=0x0F;
    if((buff[0] == 0x04) || (buff[0] == 0x05) /*A2/B2/A3/B3*/){
      for(int i=0;i<sizeof(err_module_2)/2;i++)
        ksz8895fmq_write_register(cpSW, err_module_2[i][0], err_module_2[i][1]);
    }
  }

  if(1)
  {
    struct info_s info;
    const unsigned char *config = 0;
    char temp = 'X';

    if(info_current(&info) == 0)
    {
      info.serial_str[INFO_MAX_SERIAL_STRING - 1] = 0x00;
      temp = info.serial_str[2];
    }
    switch(temp){
      case 'x':
      case 'X': //Новый свитч
        config = CONFIG_REG_KMZ8895FQX;
        break;
      case 'q':
      case 'Q': //Старый свитч
        config = CONFIG_REG_KMZ8895FMQ;
        break;
    default:
      config = KMZ8895_FULFILLMENT; //CONFIG_REG_KMZ8895FQX;
      break;
    }
    for (int i = 0; i<=255; i++) //записываем последовательно весь массив конфигурационных регистров
    {
      ksz8895fmq_write_register(cpSW, i, config[i]);
    }
  }

  /* Работа портов */
  reg = 0;
  if (cpSW == SW1) {
    #ifndef PIXEL
      switch_port(0,1);
    #endif
    vTaskDelay(50);
    ksz8895fmq_read_registers(cpSW, reg, buff, 128);
    memset(buff, 0, 128);
    #ifndef PIXEL
      switch_port(1,1);
    #endif
    vTaskDelay(50);
    ksz8895fmq_read_registers(cpSW, reg, buff, 128);
    memset(buff, 0, 128);
#if (MKPSH10 != 0)
    switch_port(4,1); vTaskDelay(50);
    ksz8895fmq_read_registers(cpSW, reg, buff, 128);
#else
    #ifndef PIXEL
      switch_port(2,1);
    #endif
    vTaskDelay(50);
    ksz8895fmq_read_registers(cpSW, reg, buff, 128);
    memset(buff, 0, 128);
    #ifndef PIXEL
      switch_port(3,1);
    #endif
    vTaskDelay(50);
    ksz8895fmq_read_registers(cpSW, reg, buff, 128);
    memset(buff, 0, 128);
#endif
  }
#ifdef TWO_SWITCH_ITEMS
  if (cpSW == SW2) {
    switch_port(2,1); vTaskDelay(50);
    ksz8895fmq_read_registers(cpSW, reg, buff, 128);
    memset(buff, 0, 128);
    switch_port(3,1); vTaskDelay(50);
    ksz8895fmq_read_registers(cpSW, reg, buff, 128);
    memset(buff, 0, 128);
    switch_port(5,1); vTaskDelay(50);
    ksz8895fmq_read_registers(cpSW, reg, buff, 128);
    switch_port(6,1); vTaskDelay(50);
    ksz8895fmq_read_registers(cpSW, reg, buff, 128);
  }
#endif

  ksz8895fmq_write_register(cpSW, 0x01, 1); vTaskDelay(250);
  {
#ifdef TWO_SWITCH_ITEMS
    //Между свитчами - полный дуплекс и 100 Мбит
    buff[0] = 0xFF;
    if (cpSW == SW1)
      ksz8895fmq_write_register(cpSW, 0x1C, *buff);
    if (cpSW == SW2)
      ksz8895fmq_write_register(cpSW, 0x5C, *buff);
#endif

    ksz8895fmq_read_registers(cpSW, reg, buff, 128);
    if (cpSW == SW1) {
      ksz8895fmq_read_register(cpSW, 0x06, buff);
      uint8_t cReg = (ETH_SPEED_10_MBPS | ETH_HALF_DUPLEX | ETH_HALF_DUPLEX_BP);
      buff[0] &= ~cReg;
#if (MKPSH10 != 0)
      // !!! Это для версии платы 2.0 (PG11 == 1)
      if (IsHwVersion2()) {
        // !!! Порт 5: Включение 10 Mbps Speed и Half-Duplex Mode
        buff[0] |= ( ETH_SPEED_10_MBPS | ETH_HALF_DUPLEX/* | ETH_HALF_DUPLEX_BP*/ );
      }
      // !!!
#endif
      ksz8895fmq_write_register(cpSW, 0x06, *buff);
    }
    ksz8895fmq_read_register(cpSW, 0x01, buff);
    ksz8895fmq_read_registers(cpSW, reg, buff, 128);
    vTaskDelay(1000);
  }
  return 0;
}

//int switch_interconnect_state( void )
//{
//  int retVal = 0;
//  static uint8_t buff;
//  ksz8895fmq_read_register(SW1, 0x1E, &buff);
//  if((buff & (1<<5)) > 0)
//  {
//    retVal |= 1;
//  }
//#ifdef TWO_SWITCH_ITEMS
//  ksz8895fmq_read_register(SW2, 0x5E, &buff);
//  if((buff & (1<<5)) > 0)
//  {
//    retVal |= 2;
//  }
//#endif
//  return retVal;
//}
int switch_init( void )
{
#ifdef LQFP_144
  spi_set_rcc_spi_enable(SPI3, 0);
  spi_set_rcc_spi_enable(SPI3, 1);


  spi_set_enable(SPI3, 0);

  spi_set_lsb_first(SPI3, 0);
  spi_set_master(SPI3, 1);
  spi_set_baudrate_control(SPI3, 7);

  spi_set_cpol(SPI3, 0);
  spi_set_cpha(SPI3, 0);

  spi_set_ssm(SPI3, 1);
  spi_set_ssi(SPI3, 1);

  spi_set_data_format(SPI3, SPI_DATA_FORMAT_8);
  spi_set_frame_format(SPI3, SPI_FRAME_FORMAT_MOTOROLA);


  spi_set_gpio(SPI3, GPIOC, 10);//sck
  spi_set_gpio(SPI3, GPIOC, 11);//miso
  spi_set_gpio(SPI3, GPIOD,  6);//mosi

  spi_set_dma_rx_enable(SPI3, 1);
  spi_set_dma_tx_enable(SPI3, 1);

  spi_set_enable(SPI3, 1);

  ksz8895fmq_init(&sw[0]);
 #ifdef TWO_SWITCH_ITEMS
  ksz8895fmq_init(&sw[1]);

  sw[0].dma_tx = sw[1].dma_tx = dma_alloc(sw[0].pspi, DMA_DIR_TO_PERIPHERY); // Выделяем поток для периферии
  sw[0].dma_rx = sw[1].dma_rx = dma_alloc(sw[0].pspi, DMA_DIR_FROM_PERIPHERY); // Выделяем поток для периферии
 #else
  sw[0].dma_tx = dma_alloc(sw[0].pspi, DMA_DIR_TO_PERIPHERY); // Выделяем поток для периферии
  sw[0].dma_rx = dma_alloc(sw[0].pspi, DMA_DIR_FROM_PERIPHERY); // Выделяем поток для периферии
 #endif
#endif //LQFP_144
#ifdef LQFP_100
  spi_set_rcc_spi_enable(SPI2, 0);
  spi_set_rcc_spi_enable(SPI2, 1);


  spi_set_enable(SPI2, 0);

  spi_set_lsb_first(SPI2, 0);
  spi_set_master(SPI2, 1);
  spi_set_baudrate_control(SPI2, 7);

  spi_set_cpol(SPI2, 0);
  spi_set_cpha(SPI2, 0);

  spi_set_ssm(SPI2, 1);
  spi_set_ssi(SPI2, 1);

  spi_set_data_format(SPI2, SPI_DATA_FORMAT_8);
  spi_set_frame_format(SPI2, SPI_FRAME_FORMAT_MOTOROLA);


  spi_set_gpio(SPI2, GPIOB, 10);//sck
  spi_set_gpio(SPI2, GPIOB, 14);//miso
  spi_set_gpio(SPI2, GPIOB, 15);//mosi

  spi_set_dma_rx_enable(SPI2, 1);
  spi_set_dma_tx_enable(SPI2, 1);

  spi_set_enable(SPI2, 1);

  ksz8895fmq_init(&sw[0]);

  sw[0].dma_tx = dma_alloc(sw[0].pspi, DMA_DIR_TO_PERIPHERY); // Выделяем поток для периферии
  sw[0].dma_rx = dma_alloc(sw[0].pspi, DMA_DIR_FROM_PERIPHERY); // Выделяем поток для периферии
#endif //LQFP_100
  InitPowerOn(0);
#ifdef TWO_SWITCH_ITEMS
  InitPowerOn(1);
#endif
  vTaskDelay(500);
  if (CheckVccRight(false)) {
    switch_on(0); vTaskDelay(50);
    sw_restart(0); vTaskDelay(50);
#ifdef TWO_SWITCH_ITEMS
    if (CheckVccRight(false)) {
      switch_on(1); vTaskDelay(200);
      sw_restart(1);
    }
#endif
  }
  return 0;
}

int switch_deinit( void )
{
#ifdef LQFP_144
  spi_set_rcc_spi_enable(SPI3, 0);
#endif
#ifdef LQFP_100
  spi_set_rcc_spi_enable(SPI2, 0);
#endif
  return 0;
}

void switch_complete( void )
{
}

int8_t switch_by_port(int8_t iPort) {
  switch (iPort) {
#ifdef TWO_SWITCH_ITEMS
  case 1:
  case 2:
  case 5:
    return 1;
  case 3:
  case 4:
  case 6:
  case 7:
    return 2;
#else
  case 1:
  case 2:
  case 3:
  case 4:
    return 1;
#endif
  default:
    return -1;
  }
}
