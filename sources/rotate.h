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


////��������� ��� ������������� ��������� ������� ��������� (switch, Port Fiber)
//enum e_PowerOnState {
//  posNone,    //��� ���������
//  posOff,     //��������
//  posPulse,   //���������, �������
//  posPause,   //���������, �����
//  posPWm_End, //����� ���
//  posOn,      //�������
//  posCount
//};

//���� ��� ���������� �������� �������, ����������� ����������
#define DRIVE_ON_TIMER  TIM1
#define DRIVE_ON_TIMER_IRQ  TIM1_IRQn
#define DRIVE_ON_TICK (21)     //��� ��� ��������� �������
#define DRIVE_ON_TIM_CLK (84) //Clock ��� ��������� ������� (� ���)

#define DRIVE_ON_PERIOD (50)  //������ ��� ��������� �������



//  #define DRIVE_ON_PORT GPIOD   //���� �����������
//  #define DRIVE_ON_PIN  (11)    //��� �����������

#define DRIVE_ON_STEP (1)     //��� ��� ��������� �������
#define DRIVE_ON_RANGE (50)   //�������� ��� ��������� �������



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
//  bool    On;     //������ �������
//  enum e_PowerOnState PowerOnState; //��������� ���������
//  uint8_t Stage;  //������ ���-����� (�� 0 �� 50)
//
//} t_PowerOnItem;

extern t_PowerOnItem drive_PowerOnItem;
extern void InitDriveOn(int n);
extern void SetPWM_Drive(int n);



#endif /* _ROTATE_H__ */


