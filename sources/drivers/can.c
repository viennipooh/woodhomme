//******************************************************************************
// Имя файла    :       can.c
// Заголовок    :       
// Автор        :       Сосновских А.А.
// Дата         :       13.06.2019
//
//------------------------------------------------------------------------------
/**
Драйвер CAN

**/

#include "can.h"

void NVIC_Config(void) {
  NVIC_InitTypeDef NVIC_InitStructure;
  NVIC_InitStructure.NVIC_IRQChannel = CAN1_RX0_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x1;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
  NVIC_InitStructure.NVIC_IRQChannel = CAN1_RX1_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x1;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
  NVIC_InitStructure.NVIC_IRQChannel = CAN1_TX_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x1;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
}

uint8_t InitCAN(CAN_TypeDef * ipCAN, uint16_t iBaudrateKbod) {
  
  //GPIO Configuration
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
  //Rx = D0
  //Tx = D1
  GPIO_PinAFConfig(GPIOD, GPIO_PinSource0, GPIO_AF_CAN1);
  GPIO_PinAFConfig(GPIOD, GPIO_PinSource1, GPIO_AF_CAN1);
  
  GPIO_InitTypeDef GPIO_InitStruct;
  GPIO_StructInit(&GPIO_InitStruct);
  GPIO_InitStruct.GPIO_Pin    = GPIO_Pin_0 | GPIO_Pin_1;
  GPIO_InitStruct.GPIO_Mode   = GPIO_Mode_AF;
  GPIO_InitStruct.GPIO_Speed  = GPIO_Speed_50MHz;
  GPIO_Init(GPIOD, &GPIO_InitStruct);
  //EN = C12 x
  //OS = C11 - Наличие питания на разъеме CAN
  GPIO_InitStruct.GPIO_Pin    = GPIO_Pin_11;
  GPIO_InitStruct.GPIO_Mode   = GPIO_Mode_IN;
  GPIO_InitStruct.GPIO_Speed  = GPIO_Speed_2MHz;
  GPIO_Init(GPIOC, &GPIO_InitStruct);
  
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN1, ENABLE); //for CAN1
  
  CAN_DeInit(ipCAN);
  
  CAN_InitTypeDef        CAN_InitStructure;
  CAN_StructInit(&CAN_InitStructure);
  CAN_InitStructure.CAN_NART = ENABLE;
  CAN_InitStructure.CAN_TXFP = ENABLE;
  //Для отладки - замыкание выхода на вход
  //При нормальной работе - CAN_Mode_Normal (по умолчанию в CAN_StructInit)
//  CAN_InitStructure.CAN_Mode = CAN_Mode_Silent_LoopBack;
//  CAN_InitStructure.CAN_Mode = CAN_Mode_LoopBack;
  CAN_InitStructure.CAN_Mode = CAN_Mode_Normal;
//  CAN_InitStructure.CAN_Mode = CAN_Mode_Silent;

  CAN_InitStructure.CAN_SJW = CAN_SJW_1tq;  
  CAN_InitStructure.CAN_BS1 = CAN_BS1_7tq;
  CAN_InitStructure.CAN_BS2 = CAN_BS2_6tq;
  uint16_t cPrescaler; 
  switch (iBaudrateKbod)  {
    case 1000: cPrescaler =   3; break; //1 Mb
    case 500:  cPrescaler =   6; break; //500 kB
    case 250:  cPrescaler =  12; break; //250 kB
    case 125:  cPrescaler =  24; break; //125 kB
    case 100:  cPrescaler =  30; break; //100 kB
    case 50:   cPrescaler =  60; break; // 50 kB
    case 20:   cPrescaler = 150; break; // 20 kB
    case 10:   cPrescaler = 300; break; // 10 kB
    default:   cPrescaler = 144; break;
  }
  if (cPrescaler == 144) {
    return CAN_InitStatus_Failed;
  } else {
    CAN_InitStructure.CAN_Prescaler = cPrescaler; 
    CAN_Init(ipCAN, &CAN_InitStructure);
  }
  
//Filter Configuration
  CAN_FilterInitTypeDef  CAN_FilterInitStructure;
  CAN_FilterInitStructure.CAN_FilterNumber = 1;
  CAN_FilterInitStructure.CAN_FilterMode = CAN_FilterMode_IdMask;
  CAN_FilterInitStructure.CAN_FilterScale = CAN_FilterScale_32bit;
  CAN_FilterInitStructure.CAN_FilterIdHigh = 0x0000;
  CAN_FilterInitStructure.CAN_FilterIdLow = 0x0000;
  CAN_FilterInitStructure.CAN_FilterMaskIdHigh = 0x0000;
  CAN_FilterInitStructure.CAN_FilterMaskIdLow = 0x0000;
  CAN_FilterInitStructure.CAN_FilterFIFOAssignment = CAN_Filter_FIFO0;
  CAN_FilterInitStructure.CAN_FilterActivation = ENABLE;
  CAN_FilterInit(&CAN_FilterInitStructure);

  NVIC_Config();
  //Разрешить прерывание приема
  CAN_ITConfig(ipCAN, CAN_IT_FMP0, ENABLE);
  return CAN_InitStatus_Success;
}

//Функция отправки:
void CAN_Send() {
  CanTxMsg TxMessage;
//  if (CAN_GetITStatus (CAN1, CAN_IT_TME))
//  {
//    CAN_ClearITPendingBit(CAN1,CAN_IT_TME);
    char ASK1_DATA0 = '1';
    TxMessage.StdId = CAN_CMD_Test_StdId;
    TxMessage.ExtId = 0x00;        // Расширенную команду указывать нет смысла
    TxMessage.RTR = CAN_RTR_DATA;
    TxMessage.IDE = CAN_ID_STD;
    TxMessage.DLC = 1;
    TxMessage.Data[0] = ASK1_DATA0;
    CAN_Transmit(CAN1,&TxMessage);
//  }
}

//Функция приёма:
uint32_t gCANRxCount = 0;
void CAN1_RX0_IRQHandler(void)  {
  CanRxMsg RxMessage;
  if (CAN_GetITStatus (CAN1, CAN_IT_FMP0))  {
    CAN_ClearITPendingBit(CAN1, CAN_IT_FMP0);
    CAN_Receive(CAN1, CAN_FIFO0, &RxMessage);
    CAN_FIFORelease(CAN1, CAN_FIFO0);
    ++gCANRxCount;
//    READ_DATA0 = RxMessage.Data[0];
//    READ_DATA1 = RxMessage.Data[1];
//    READ_DATA2 = RxMessage.Data[2];
//    READ_DATA3 = RxMessage.Data[3];
//    Can1Flag = ENABLE;
  }
}

//Отправка данных в CAN
void CAN_Send_Test(void) {
	CanTxMsg TxMessage;
	TxMessage.StdId = CAN_CMD_Test_StdId; // Команда шины

	TxMessage.ExtId = 0x00;       // Расширенную команду указывать нет смысла

	TxMessage.IDE = CAN_ID_STD;   // Формат кадра
	TxMessage.RTR = CAN_RTR_DATA; // Тип сообщения
	TxMessage.DLC = 3;            // Длина блока данных 3 - передадим три байта

	TxMessage.Data[0] = 0x00;     // Байт данных №1
	TxMessage.Data[1] = 0x01;     // Байт данных №2
	TxMessage.Data[2] = 0x02;     // Байт данных №3
	CAN_Transmit(CAN1, &TxMessage);
}

#include "stdio.h"
#include "FreeRTOS.h"
#include "task.h"
#include "lwipopts.h"
uint32_t gCANTick = 0;
void ProcessCAN() {
  portTickType cCANTick = xTaskGetTickCount();
  if (gCANTick != 0) {
    uint32_t cTime = cCANTick - gCANTick;
    if (cTime > 150) {
      //Проверить, есть ли ошибка.
      //  Если есть, то: сброс, в нормальное состояние
      if ((CAN1->TSR&CAN_TSR_TERR0) == CAN_TSR_TERR0) {
        CAN1->TSR &= ~(CAN_TSR_RQCP0);
      }
      CAN_Send();
      gCANTick = cCANTick;
    } else {
      cCANTick = 0;
    }
  } else {
    gCANTick = cCANTick;
  }
}

//Заготовки потоков для CAN
static void CAN_socket_recv_thread( void * arg ) {
  
  while (1) {
    
  }
}

static void CAN_socket_send_thread( void * arg ) {
  
  while (1) {
    
  }
}

int CAN_mode_init() {
  char buffer[60];
  int result;
  xTaskHandle CreatedTask;

  sprintf(buffer,"CANgwTx-%d", 0);
  result = xTaskCreate( CAN_socket_recv_thread, ( signed portCHAR * ) buffer,
                       DEFAULT_THREAD_STACKSIZE, NULL, CAN_THREAD_PRIO, &CreatedTask );
  sprintf(buffer,"CANgwRx-%d", 1);
  result = xTaskCreate( CAN_socket_send_thread, ( signed portCHAR * ) buffer,
                       DEFAULT_THREAD_STACKSIZE, NULL, CAN_THREAD_PRIO, &CreatedTask );
  return result;
}
