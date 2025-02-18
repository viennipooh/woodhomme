#ifndef __ETHERNETIF_H__
#define __ETHERNETIF_H__


#include "lwip/err.h"
#include "lwip/netif.h"

//���������� ������ Rx � Tx
#define LWIP_SEPARATE_RX_TX_BUFFERS

//������ �������� ��� ������������
#define REPEAT_TX_ON_TX_ERROR

//������ ����� vTaskDelay(1)
//#define REPEAT_TX_ON_TX_ERROR_BY_RTOS

err_t ethernetif_init(struct netif *netif);



#endif 
