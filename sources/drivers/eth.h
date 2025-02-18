#ifndef __ETH_H
#define __ETH_H

#include "common.h"

/**
\brief Ethernet Media Interface type
*/
#define ETH_INTERFACE_MII           0       ///< Media Independent Interface (MII)
#define ETH_INTERFACE_RMII          1       ///< Reduced Media Independent Interface (RMII)
#define ETH_INTERFACE_SMII          2       ///< Serial Media Independent Interface (SMII)

/**
\brief Ethernet link speed
*/
#define ETH_SPEED_10M               0       ///< 10 Mbps link speed
#define ETH_SPEED_100M              1       ///< 100 Mbps link speed
#define ETH_SPEED_1G                2       ///< 1 Gpbs link speed

/**
\brief Ethernet duplex mode
*/
#define ETH_DUPLEX_HALF             0       ///< Half duplex link
#define ETH_DUPLEX_FULL             1       ///< Full duplex link

/**
\brief Ethernet link state
*/
typedef enum _eth_link_state {
  ETH_LINK_DOWN,                    ///< Link is down
  ETH_LINK_UP                       ///< Link is up
} eth_link_state;

/**
\brief Ethernet link information
*/
typedef struct _eth_link_info {
  uint32_t speed  : 2;                  ///< Link speed: 0= 10 MBit, 1= 100 MBit, 2= 1 GBit
  uint32_t duplex : 1;                  ///< Duplex mode: 0= Half, 1= Full
} eth_link_info;

/**
\brief Ethernet MAC Address
*/
typedef struct _eth_mac_addr {
  uint8_t b[6];                         ///< MAC Address (6 bytes), MSB first
} eth_mac_addr;


int eth_init();
int eth_deinit();


int eth_send(void *buff, int len);
int eth_recv(void *buff, int *len);

#endif /* __ETH_H */
