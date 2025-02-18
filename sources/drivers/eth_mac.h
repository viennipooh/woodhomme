#ifndef __DRIVER_ETH_MAC_H
#define __DRIVER_ETH_MAC_H

#include "eth.h"

#define ETH_MAC_API_VERSION DRIVER_VERSION_MAJOR_MINOR(2,01)  /* API version */


#define _ETH_MAC_(n)  eth_mac##n
#define  ETH_MAC_(n) _ETH_MAC_(n)


/****** Ethernet MAC Control Codes *****/

#define ETH_MAC_CONFIGURE           (0x01)      ///< Configure MAC; arg = configuration
#define ETH_MAC_CONTROL_TX          (0x02)      ///< Transmitter; arg: 0=disabled (default), 1=enabled
#define ETH_MAC_CONTROL_RX          (0x03)      ///< Receiver; arg: 0=disabled (default), 1=enabled
#define ETH_MAC_FLUSH               (0x04)      ///< Flush buffer; arg = ETH_MAC_FLUSH_...
#define ETH_MAC_SLEEP               (0x05)      ///< Sleep mode; arg: 1=enter and wait for Magic packet, 0=exit
#define ETH_MAC_VLAN_FILTER         (0x06)      ///< VLAN Filter for received frames; arg15..0: VLAN Tag; arg16: optional ETH_MAC_VLAN_FILTER_ID_ONLY; 0=disabled (default)

/*----- Ethernet MAC Configuration -----*/
#define ETH_MAC_SPEED_Pos            0
#define ETH_MAC_SPEED_Msk           (3UL                 << ETH_MAC_SPEED_Pos)
#define ETH_MAC_SPEED_10M           (ETH_SPEED_10M   << ETH_MAC_SPEED_Pos)  ///< 10 Mbps link speed
#define ETH_MAC_SPEED_100M          (ETH_SPEED_100M  << ETH_MAC_SPEED_Pos)  ///< 100 Mbps link speed
#define ETH_MAC_SPEED_1G            (ETH_SPEED_1G    << ETH_MAC_SPEED_Pos)  ///< 1 Gpbs link speed
#define ETH_MAC_DUPLEX_Pos           2
#define ETH_MAC_DUPLEX_Msk          (1UL                 << ETH_MAC_DUPLEX_Pos)
#define ETH_MAC_DUPLEX_HALF         (ETH_DUPLEX_HALF << ETH_MAC_DUPLEX_Pos) ///< Half duplex link
#define ETH_MAC_DUPLEX_FULL         (ETH_DUPLEX_FULL << ETH_MAC_DUPLEX_Pos) ///< Full duplex link
#define ETH_MAC_LOOPBACK            (1UL << 4)  ///< Loop-back test mode
#define ETH_MAC_CHECKSUM_OFFLOAD_RX (1UL << 5)  ///< Receiver Checksum offload
#define ETH_MAC_CHECKSUM_OFFLOAD_TX (1UL << 6)  ///< Transmitter Checksum offload
#define ETH_MAC_ADDRESS_BROADCAST   (1UL << 7)  ///< Accept frames with Broadcast address
#define ETH_MAC_ADDRESS_MULTICAST   (1UL << 8)  ///< Accept frames with any Multicast address
#define ETH_MAC_ADDRESS_ALL         (1UL << 9)  ///< Accept frames with any address (Promiscuous Mode)

/*----- Ethernet MAC Flush Flags -----*/
#define ETH_MAC_FLUSH_RX            (1UL << 0)  ///< Flush Receive buffer
#define ETH_MAC_FLUSH_TX            (1UL << 1)  ///< Flush Transmit buffer

/*----- Ethernet MAC VLAN Filter Flag -----*/
#define ETH_MAC_VLAN_FILTER_ID_ONLY (1UL << 16) ///< Compare only the VLAN Identifier (12-bit)


/****** Ethernet MAC Frame Transmit Flags *****/
#define ETH_MAC_TX_FRAME_FRAGMENT   (1UL << 0)  ///< Indicate frame fragment
#define ETH_MAC_TX_FRAME_EVENT      (1UL << 1)  ///< Generate event when frame is transmitted
#define ETH_MAC_TX_FRAME_TIMESTAMP  (1UL << 2)  ///< Capture frame time stamp


/****** Ethernet MAC Timer Control Codes *****/
#define ETH_MAC_TIMER_GET_TIME      (0x01)      ///< Get current time
#define ETH_MAC_TIMER_SET_TIME      (0x02)      ///< Set new time
#define ETH_MAC_TIMER_INC_TIME      (0x03)      ///< Increment current time
#define ETH_MAC_TIMER_DEC_TIME      (0x04)      ///< Decrement current time
#define ETH_MAC_TIMER_SET_ALARM     (0x05)      ///< Set alarm time
#define ETH_MAC_TIMER_ADJUST_CLOCK  (0x06)      ///< Adjust clock frequency; time->ns: correction factor * 2^31


/**
\brief Ethernet MAC Time
*/
typedef struct _eth_mac_time {
  uint32_t ns;                          ///< Nano seconds
  uint32_t sec;                         ///< Seconds
} eth_mac_time;


/****** Ethernet MAC Event *****/
#define ETH_MAC_EVENT_RX_FRAME      (1UL << 0)  ///< Frame Received
#define ETH_MAC_EVENT_TX_FRAME      (1UL << 1)  ///< Frame Transmitted
#define ETH_MAC_EVENT_WAKEUP        (1UL << 2)  ///< Wake-up (on Magic Packet)
#define ETH_MAC_EVENT_TIMER_ALARM   (1UL << 3)  ///< Timer Alarm


// Function documentation
/**
  \fn          DRIVER_VERSION ETH_MAC_GetVersion (void)
  \brief       Get driver version.
  \return      \ref DRIVER_VERSION
*/
/**
  \fn          ETH_MAC_CAPABILITIES ETH_MAC_GetCapabilities (void)
  \brief       Get driver capabilities.
  \return      \ref ETH_MAC_CAPABILITIES
*/
/**
  \fn          int32_t ETH_MAC_Initialize (ETH_MAC_SignalEvent_t cb_event)
  \brief       Initialize Ethernet MAC Device.
  \param[in]   cb_event  Pointer to \ref ETH_MAC_SignalEvent
  \return      \ref execution_status
*/
/**
  \fn          int32_t ETH_MAC_Uninitialize (void)
  \brief       De-initialize Ethernet MAC Device.
  \return      \ref execution_status
*/
/**
  \fn          int32_t ETH_MAC_PowerControl (POWER_STATE state)
  \brief       Control Ethernet MAC Device Power.
  \param[in]   state  Power state
  \return      \ref execution_status
*/
/**
  \fn          int32_t ETH_MAC_GetMacAddress (ETH_MAC_ADDR *ptr_addr)
  \brief       Get Ethernet MAC Address.
  \param[in]   ptr_addr  Pointer to address
  \return      \ref execution_status
*/
/**
  \fn          int32_t ETH_MAC_SetMacAddress (const ETH_MAC_ADDR *ptr_addr)
  \brief       Set Ethernet MAC Address.
  \param[in]   ptr_addr  Pointer to address
  \return      \ref execution_status
*/
/**
  \fn          int32_t ETH_MAC_SetAddressFilter (const ETH_MAC_ADDR *ptr_addr,
                                                           uint32_t          num_addr)
  \brief       Configure Address Filter.
  \param[in]   ptr_addr  Pointer to addresses
  \param[in]   num_addr  Number of addresses to configure
  \return      \ref execution_status
*/
/**
  \fn          int32_t ETH_MAC_SendFrame (const uint8_t *frame, uint32_t len, uint32_t flags)
  \brief       Send Ethernet frame.
  \param[in]   frame  Pointer to frame buffer with data to send
  \param[in]   len    Frame buffer length in bytes
  \param[in]   flags  Frame transmit flags (see ETH_MAC_TX_FRAME_...)
  \return      \ref execution_status
*/
/**
  \fn          int32_t ETH_MAC_ReadFrame (uint8_t *frame, uint32_t len)
  \brief       Read data of received Ethernet frame.
  \param[in]   frame  Pointer to frame buffer for data to read into
  \param[in]   len    Frame buffer length in bytes
  \return      number of data bytes read or execution status
                 - value >= 0: number of data bytes read
                 - value < 0: error occurred, value is execution status as defined with \ref execution_status 
*/
/**
  \fn          uint32_t ETH_MAC_GetRxFrameSize (void)
  \brief       Get size of received Ethernet frame.
  \return      number of bytes in received frame
*/
/**
  \fn          int32_t ETH_MAC_GetRxFrameTime (ETH_MAC_TIME *time)
  \brief       Get time of received Ethernet frame.
  \param[in]   time  Pointer to time structure for data to read into
  \return      \ref execution_status
*/
/**
  \fn          int32_t ETH_MAC_GetTxFrameTime (ETH_MAC_TIME *time)
  \brief       Get time of transmitted Ethernet frame.
  \param[in]   time  Pointer to time structure for data to read into
  \return      \ref execution_status
*/
/**
  \fn          int32_t ETH_MAC_Control (uint32_t control, uint32_t arg)
  \brief       Control Ethernet Interface.
  \param[in]   control  Operation
  \param[in]   arg      Argument of operation (optional)
  \return      \ref execution_status
*/
/**
  \fn          int32_t ETH_MAC_ControlTimer (uint32_t control, ETH_MAC_TIME *time)
  \brief       Control Precision Timer.
  \param[in]   control  Operation
  \param[in]   time     Pointer to time structure
  \return      \ref execution_status
*/
/**
  \fn          int32_t ETH_MAC_PHY_Read (uint8_t phy_addr, uint8_t reg_addr, uint16_t *data)
  \brief       Read Ethernet PHY Register through Management Interface.
  \param[in]   phy_addr  5-bit device address
  \param[in]   reg_addr  5-bit register address
  \param[out]  data      Pointer where the result is written to
  \return      \ref execution_status
*/
/**
  \fn          int32_t ETH_MAC_PHY_Write (uint8_t phy_addr, uint8_t reg_addr, uint16_t data)
  \brief       Write Ethernet PHY Register through Management Interface.
  \param[in]   phy_addr  5-bit device address
  \param[in]   reg_addr  5-bit register address
  \param[in]   data      16-bit data to write
  \return      \ref execution_status
*/

/**
  \fn          void ETH_MAC_SignalEvent (uint32_t event)
  \brief       Callback function that signals a Ethernet Event.
  \param[in]   event  event notification mask
  \return      none
*/

typedef void (*eth_mac_signalevent_t) (uint32_t event);  ///< Pointer to \ref ETH_MAC_SignalEvent : Signal Ethernet Event.


/**
\brief Ethernet MAC Capabilities
*/
typedef struct _eth_mac_capabilities {
  uint32_t checksum_offload_rx_ip4  : 1;        ///< 1 = IPv4 header checksum verified on receive
  uint32_t checksum_offload_rx_ip6  : 1;        ///< 1 = IPv6 checksum verification supported on receive
  uint32_t checksum_offload_rx_udp  : 1;        ///< 1 = UDP payload checksum verified on receive
  uint32_t checksum_offload_rx_tcp  : 1;        ///< 1 = TCP payload checksum verified on receive
  uint32_t checksum_offload_rx_icmp : 1;        ///< 1 = ICMP payload checksum verified on receive
  uint32_t checksum_offload_tx_ip4  : 1;        ///< 1 = IPv4 header checksum generated on transmit
  uint32_t checksum_offload_tx_ip6  : 1;        ///< 1 = IPv6 checksum generation supported on transmit
  uint32_t checksum_offload_tx_udp  : 1;        ///< 1 = UDP payload checksum generated on transmit
  uint32_t checksum_offload_tx_tcp  : 1;        ///< 1 = TCP payload checksum generated on transmit
  uint32_t checksum_offload_tx_icmp : 1;        ///< 1 = ICMP payload checksum generated on transmit
  uint32_t media_interface          : 2;        ///< Ethernet Media Interface type
  uint32_t mac_address              : 1;        ///< 1 = driver provides initial valid MAC address
  uint32_t event_rx_frame           : 1;        ///< 1 = callback event \ref ETH_MAC_EVENT_RX_FRAME generated
  uint32_t event_tx_frame           : 1;        ///< 1 = callback event \ref ETH_MAC_EVENT_TX_FRAME generated
  uint32_t event_wakeup             : 1;        ///< 1 = wakeup event \ref ETH_MAC_EVENT_WAKEUP generated
  uint32_t precision_timer          : 1;        ///< 1 = Precision Timer supported
} eth_mac_capabilities;


/**
\brief Access structure of the Ethernet MAC Driver
*/
typedef struct _eth_mac {
  driver_version	(*get_version)      (void);                                                ///< Pointer to \ref ETH_MAC_GetVersion : Get driver version.
  eth_mac_capabilities  (*get_capabilities) (void);                                                ///< Pointer to \ref ETH_MAC_GetCapabilities : Get driver capabilities.
  int32_t       	(*initialize)       (eth_mac_signalevent_t cb_event);                  ///< Pointer to \ref ETH_MAC_Initialize : Initialize Ethernet MAC Device.
  int32_t	        (*uninitialize)     (void);                                                ///< Pointer to \ref ETH_MAC_Uninitialize : De-initialize Ethernet MAC Device.
  int32_t		(*power_control)    (power_state state);                               ///< Pointer to \ref ETH_MAC_PowerControl : Control Ethernet MAC Device Power.
  int32_t     		(*get_mac_address)  (      eth_mac_addr *ptr_addr);                    ///< Pointer to \ref ETH_MAC_GetMacAddress : Get Ethernet MAC Address.
  int32_t        	(*set_mac_address)  (const eth_mac_addr *ptr_addr);                    ///< Pointer to \ref ETH_MAC_SetMacAddress : Set Ethernet MAC Address.
  int32_t        	(*set_address_filter)(const eth_mac_addr *ptr_addr, uint32_t num_addr); ///< Pointer to \ref ETH_MAC_SetAddressFilter : Configure Address Filter.
  int32_t         	(*send_frame)       (const uint8_t *frame, uint32_t len, uint32_t flags);  ///< Pointer to \ref ETH_MAC_SendFrame : Send Ethernet frame.
  int32_t         	(*read_frame)       (      uint8_t *frame, uint32_t len);                  ///< Pointer to \ref ETH_MAC_ReadFrame : Read data of received Ethernet frame.
  uint32_t        	(*get_rx_frame_size)  (void);                                                ///< Pointer to \ref ETH_MAC_GetRxFrameSize : Get size of received Ethernet frame.
  int32_t         	(*get_rx_frame_time)  (eth_mac_time *time);                              ///< Pointer to \ref ETH_MAC_GetRxFrameTime : Get time of received Ethernet frame.
  int32_t         	(*get_tx_frame_time)  (eth_mac_time *time);                              ///< Pointer to \ref ETH_MAC_GetTxFrameTime : Get time of transmitted Ethernet frame.
  int32_t          	(*control_timer)    (uint32_t control, eth_mac_time *time);            ///< Pointer to \ref ETH_MAC_ControlTimer : Control Precision Timer.
  int32_t          	(*control)          (uint32_t control, uint32_t arg);                      ///< Pointer to \ref ETH_MAC_Control : Control Ethernet Interface.
  int32_t         	(*phy_read)         (uint8_t phy_addr, uint8_t reg_addr, uint16_t *data);  ///< Pointer to \ref ETH_MAC_PHY_Read : Read Ethernet PHY Register through Management Interface.
  int32_t          	(*phy_write)        (uint8_t phy_addr, uint8_t reg_addr, uint16_t  data);  ///< Pointer to \ref ETH_MAC_PHY_Write : Write Ethernet PHY Register through Management Interface.
} const eth_mac;

#endif /* __ETH_MAC_H */
