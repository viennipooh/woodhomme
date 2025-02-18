/**
  ******************************************************************************
  * @file    rotate.h
  * @author  Dmitry Vakhrushev ( vdv.18@mail.ru )
  * @version V1.0
  * @date    13.05.2016
  * @brief   --
  ******************************************************************************
  * @attention
  *
  ******************************************************************************
  */

#ifndef _ROTATE_H__
#define _ROTATE_H__

#include "switch.h"

#include "switch.h"
#include <stdint.h>
#include <stdbool.h>
#include "extio.h"

#include "FreeRTOS.h"
#include "task.h"
#include "stm32f4xx_tim.h"


////Структура для сопровождения включения питания устройств (switch, Port Fiber)
//enum e_PowerOnState {
//  posNone,    //Нет состояния
//  posOff,     //Выключен
//  posPulse,   //Включение, импульс
//  posPause,   //Включение, пауза
//  posPWm_End, //Конец ШИМ
//  posOn,      //Включен
//  posCount
//};

//Блок ШИМ управления питанием объекта, включаемого программно
#define DRIVE_ON_TIMER  TIM1
#define DRIVE_ON_TIMER_IRQ  TIM1_IRQn
#define DRIVE_ON_TICK (21)     //Тик ШИМ включения питания
#define DRIVE_ON_TIM_CLK (84) //Clock ШИМ включения питания (в МГц)

#define DRIVE_ON_PERIOD (50)  //Период ШИМ включения питания



//  #define DRIVE_ON_PORT GPIOD   //Порт направления
//  #define DRIVE_ON_PIN  (11)    //Пин направления

#define DRIVE_ON_STEP (1)     //Шаг ШИМ включения питания
#define DRIVE_ON_RANGE (50)   //Интервал ШИМ включения питания



#ifdef PIXEL
  #define CLN_DRV_SW_PIN       0    // num in mass Outs
  #define DRIVE_PORT GPIOD
  #define DRIVE   1
  #define CLEAN   0

#define CLEAN_MODE  extio_gpio_out(DRIVE_PORT, CLN_DRV_SW_PIN, CLEAN)
#define DRIVE_MODE  extio_gpio_out(DRIVE_PORT, CLN_DRV_SW_PIN, DRIVE)

  #define POWER_PIN       2    // num in mass Outs
  #define POWER_PORT GPIOD

#define POWER_ON  extio_gpio_out(POWER_PORT, POWER_PIN, ON)

  #define XY_PIN       3    // num in mass Outs
  #define XY_PORT GPIOD

#define DRIVE_X   extio_gpio_out(XY_PORT, XY_PIN, ON)
#define DRIVE_Y  extio_gpio_out(XY_PORT, XY_PIN, OFF)

#define FORWARD  extio_gpio_out(GPIOE, 12, OFF)
#define BACK  extio_gpio_out(GPIOE, 12, ON)

#endif
//
//typedef struct s_PowerOnItem {
//  bool    On;     //Объект включен
//  enum e_PowerOnState PowerOnState; //Состояние включения
//  uint8_t Stage;  //Стадия ШИМ-пуска (от 0 до 50)
//
//} t_PowerOnItem;

extern t_PowerOnItem drive_PowerOnItem;
extern void InitDriveOn(int n);
extern void SetPWM_Drive(int n);



#endif /* _ROTATE_H__ */


