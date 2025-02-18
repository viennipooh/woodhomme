#ifndef __DRIVER_ETH_PHY_H
#define __DRIVER_ETH_PHY_H

#include "eth.h"

#define ETH_PHY_API_VERSION DRIVER_VERSION_MAJOR_MINOR(2,00)  /* API version */


#define _ETH_PHY_(n)  eth_phy##n
#define  ETH_PHY_(n) _ETH_PHY_(n)


/****** Ethernet PHY Mode *****/
#define ETH_PHY_SPEED_Pos            0
#define ETH_PHY_SPEED_Msk           (3UL             << ETH_PHY_SPEED_Pos)
#define ETH_PHY_SPEED_10M           (ETH_SPEED_10M   << ETH_PHY_SPEED_Pos)  ///< 10 Mbps link speed
#define ETH_PHY_SPEED_100M          (ETH_SPEED_100M  << ETH_PHY_SPEED_Pos)  ///< 100 Mbps link speed
#define ETH_PHY_SPEED_1G            (ETH_SPEED_1G    << ETH_PHY_SPEED_Pos)  ///< 1 Gpbs link speed
#define ETH_PHY_DUPLEX_Pos           2
#define ETH_PHY_DUPLEX_Msk          (1UL             << ETH_PHY_DUPLEX_Pos)
#define ETH_PHY_DUPLEX_HALF         (ETH_DUPLEX_HALF << ETH_PHY_DUPLEX_Pos) ///< Half duplex link
#define ETH_PHY_DUPLEX_FULL         (ETH_DUPLEX_FULL << ETH_PHY_DUPLEX_Pos) ///< Full duplex link
#define ETH_PHY_AUTO_NEGOTIATE      (1UL << 3)                                      ///< Auto Negotiation mode
#define ETH_PHY_LOOPBACK            (1UL << 4)                                      ///< Loop-back test mode
#define ETH_PHY_ISOLATE             (1UL << 5)                                      ///< Isolate PHY from MII/RMII interface


// Function documentation
/**
  \fn          VERSION ETH_PHY_GetVersion (void)
  \brief       Get driver version.
  \return      \ref VERSION
*/
/**
  \fn          int32_t ETH_PHY_Initialize (ETH_PHY_Read_t  fn_read,
                                               ETH_PHY_Write_t fn_write)
  \brief       Initialize Ethernet PHY Device.
  \param[in]   fn_read   Pointer to \ref ETH_MAC_PHY_Read
  \param[in]   fn_write  Pointer to \ref ETH_MAC_PHY_Write
  \return      \ref execution_status
*/
/**
  \fn          int32_t ETH_PHY_Uninitialize (void)
  \brief       De-initialize Ethernet PHY Device.
  \return      \ref execution_status
*/
/**
  \fn          int32_t ETH_PHY_PowerControl (POWER_STATE state)
  \brief       Control Ethernet PHY Device Power.
  \param[in]   state  Power state
  \return      \ref execution_status
*/
/**
  \fn          int32_t ETH_PHY_SetInterface (uint32_t interface)
  \brief       Set Ethernet Media Interface.
  \param[in]   interface  Media Interface type
  \return      \ref execution_status
*/
/**
  \fn          int32_t ETH_PHY_SetMode (uint32_t mode)
  \brief       Set Ethernet PHY Device Operation mode.
  \param[in]   mode  Operation Mode
  \return      \ref execution_status
*/
/**
  \fn          ETH_LINK_STATE ETH_PHY_GetLinkState (void)
  \brief       Get Ethernet PHY Device Link state.
  \return      current link status \ref ETH_LINK_STATE
*/
/**
  \fn          ETH_LINK_INFO ETH_PHY_GetLinkInfo (void)
  \brief       Get Ethernet PHY Device Link information.
  \return      current link parameters \ref ETH_LINK_INFO
*/


typedef int32_t (*eth_phy_read_t)  (uint8_t phy_addr, uint8_t reg_addr, uint16_t *data); ///< Pointer to \ref ETH_MAC_PHY_Read : Read Ethernet PHY Register.
typedef int32_t (*eth_phy_write_t) (uint8_t phy_addr, uint8_t reg_addr, uint16_t  data); ///< Pointer to \ref ETH_MAC_PHY_Write : Write Ethernet PHY Register.


/**
\brief Access structure of the Ethernet PHY Driver
*/
typedef struct _eth_phy {
  driver_version 	(*get_version)   	(void);                          ///< Pointer to \ref ETH_PHY_GetVersion : Get driver version.
  int32_t        	(*initialize)   	(eth_phy_read_t  fn_read,
						eth_phy_write_t fn_write);  ///< Pointer to \ref ETH_PHY_Initialize : Initialize PHY Device.
  int32_t          	(*uninitialize) 	(void);                          ///< Pointer to \ref ETH_PHY_Uninitialize : De-initialize PHY Device.
  int32_t          	(*power_control) 	(power_state state);         ///< Pointer to \ref ETH_PHY_PowerControl : Control PHY Device Power.
  int32_t         	(*set_interface) 	(uint32_t interface);            ///< Pointer to \ref ETH_PHY_SetInterface : Set Ethernet Media Interface.
  int32_t        	(*set_mode)      	(uint32_t mode);                 ///< Pointer to \ref ETH_PHY_SetMode : Set Ethernet PHY Device Operation mode.
  eth_link_state 	(*get_link_state) 	(void);                          ///< Pointer to \ref ETH_PHY_GetLinkState : Get Ethernet PHY Device Link state.
  eth_link_info  	(*get_link_info)  	(void);                          ///< Pointer to \ref ETH_PHY_GetLinkInfo : Get Ethernet PHY Device Link information.
} const eth_phy;

#endif /* __ETH_PHY_H */
