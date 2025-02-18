
#ifndef _STM32ETH_REGS_H__
#define _STM32ETH_REGS_H__

#include <stdint.h>
#include "stm32f4xx.h"

/**
 * Extendend descriptor definition
 */
#define ETHERNET_DESCRIPTOR_EXTENDED    0

/**
 * Descriptor control & status bits defenition
 */
#define DESCRIPTOR_TRANSMIT_STATUS_DB           0x00000001      /* Deferred bit */
#define DESCRIPTOR_TRANSMIT_STATUS_UF           0x00000002      /* Underflow error */
#define DESCRIPTOR_TRANSMIT_STATUS_ED           0x00000004      /* Excessive deferral */
#define DESCRIPTOR_TRANSMIT_STATUS_CC           0x00000078      /* Collision count */
#define DESCRIPTOR_TRANSMIT_STATUS_VF           0x00000080      /* VLAN frame */
#define DESCRIPTOR_TRANSMIT_STATUS_EC           0x00000100      /* Excessive collision */
#define DESCRIPTOR_TRANSMIT_STATUS_LCO          0x00000200      /* Late collision */
#define DESCRIPTOR_TRANSMIT_STATUS_NC           0x00000400      /* No carrier */
#define DESCRIPTOR_TRANSMIT_STATUS_LCA          0x00000800      /* Loss of carrier */
#define DESCRIPTOR_TRANSMIT_STATUS_IPE          0x00001000      /* IP payload error */
#define DESCRIPTOR_TRANSMIT_STATUS_FF           0x00002000      /* Frame flushed */
#define DESCRIPTOR_TRANSMIT_STATUS_JT           0x00004000      /* Jabber timeout */
#define DESCRIPTOR_TRANSMIT_STATUS_ES           0x00008000      /* Error summary */
#define DESCRIPTOR_TRANSMIT_STATUS_IHE          0x00010000

#define DESCRIPTOR_TRANSMIT_STATUS_TTSS         0x00020000      /* Transmit time stamp status (dependes on TDES0[29]) */
#define DESCRIPTOR_TRANSMIT_STATUS_TTSE         0x02000000      /* Transmit time stamp enable (dependes on TDES0[28],PTPTSCR[0]=0) */

#define DESCRIPTOR_TRANSMIT_STATUS_CTRL_TCH     0x00100000      /* Second address chained ((dependes on TDES0[20,21], TDES1[28:16]))*/
#define DESCRIPTOR_TRANSMIT_STATUS_CTRL_TER     0x00200000      /* Transmit end of ring */
#define DESCRIPTOR_TRANSMIT_STATUS_CTRL_CIC     0x00C00000      /* Checksum insertion control */
#define DESCRIPTOR_TRANSMIT_STATUS_CTRL_CIC_DISABLED            0x00000000      /* Checksum Insertion disabled */
#define DESCRIPTOR_TRANSMIT_STATUS_CTRL_CIC_ONLY_IP             0x00400000      /* Only IP header checksum calculation and insertion are enabled */
#define DESCRIPTOR_TRANSMIT_STATUS_CTRL_CIC_ALL_NOT_HARDWARE    0x00800000      /* IP header checksum and payload checksum calculation and insertion are enabled, but 
                                                                                   pseudo-header checksum is not calculated in hardware*/
#define DESCRIPTOR_TRANSMIT_STATUS_CTRL_CIC_ALL_HARDWARE        0x00C00000      /* IP Header checksum and payload checksum calculation and insertion are enabled, and 
                                                                                   pseudo-header checksum is calculated in hardware */
#define DESCRIPTOR_TRANSMIT_STATUS_CTRL_DP      0x04000000      /* Disable pad (dependes on TDES0[27,28]) */
#define DESCRIPTOR_TRANSMIT_STATUS_CTRL_DC      0x08000000      /* Disable CRC (dependes on TDES0[28]) */
#define DESCRIPTOR_TRANSMIT_STATUS_CTRL_FS      0x10000000      /* First segment - bufer 1 enable data */
#define DESCRIPTOR_TRANSMIT_STATUS_CTRL_LS      0x20000000      /* Last segment  - bufer 2 enable data */
#define DESCRIPTOR_TRANSMIT_STATUS_CTRL_IC      0x40000000      /* Interrupt on completion */

#define DESCRIPTOR_TRANSMIT_STATUS_OWN          0x80000000      /* OWN bit - owned by DMA=1 or by CPU=0*/


/**
 * Descriptor control & status bits defenition
 */
#define DESCRIPTOR_RECEIVE_STATUS_PCE_ESA      0x00000001      /* Payload checksum error / extended status available */
#define DESCRIPTOR_RECEIVE_STATUS_CE           0x00000002      /* CRC error */
#define DESCRIPTOR_RECEIVE_STATUS_DBE          0x00000004      /* Dribble bit error */
#define DESCRIPTOR_RECEIVE_STATUS_RE           0x00000008      /* Receive error */
#define DESCRIPTOR_RECEIVE_STATUS_RWT          0x00000010      /* Receive watchdog timeout */
#define DESCRIPTOR_RECEIVE_STATUS_FT           0x00000020      /* Frame type */
#define DESCRIPTOR_RECEIVE_STATUS_LCO          0x00000040      /* Late collision */
#define DESCRIPTOR_RECEIVE_STATUS_IPHCE_TSV    0x00000080      /* IPv header checksum error / time stamp valid */
#define DESCRIPTOR_RECEIVE_STATUS_LS           0x00000100      /* Last descriptor */
#define DESCRIPTOR_RECEIVE_STATUS_FS           0x00000200      /* First descriptor */
#define DESCRIPTOR_RECEIVE_STATUS_VLAN         0x00000400      /* VLAN tag */
#define DESCRIPTOR_RECEIVE_STATUS_OE           0x00000800      /* Overflow error */
#define DESCRIPTOR_RECEIVE_STATUS_LE           0x00001000      /* Length error */
#define DESCRIPTOR_RECEIVE_STATUS_SAF          0x00002000      /* Source address filter fail */
#define DESCRIPTOR_RECEIVE_STATUS_DE           0x00004000      /* Descriptor error */
#define DESCRIPTOR_RECEIVE_STATUS_ES           0x00008000      /* Error summary */
#define DESCRIPTOR_RECEIVE_STATUS_FL           0x3FFF0000      /* Frame length */
#define DESCRIPTOR_RECEIVE_STATUS_AFM          0x40000000      /* Destination address filter fail */
#define DESCRIPTOR_RECEIVE_STATUS_OWN          0x80000000      /* OWN bit - owned by DMA=1 or by CPU=0*/


/**
 * Descriptor buffer count defenition
 */
#define DESCRIPTOR_BUFFER_COUNT_DIC             0x80000000      /* Disable interrupt on completion */
#define DESCRIPTOR_BUFFER_COUNT_RER             0x00008000      /* Receive end of ring */
#define DESCRIPTOR_BUFFER_COUNT_RCH             0x00004000      /* Second address chained */
#define DESCRIPTOR_BUFFER_COUNT_TBS1            0x00001FFF      /* Transmit buffer 1 size */
#define DESCRIPTOR_BUFFER_COUNT_TBS2            0x1FFF0000      /* Transmit buffer 2 size (dependes on TDES0[20])*/

/**
 * Descriptor defenition (rx & tx)
 */
typedef struct ethernet_transmit_descriptor_s {
  uint32_t status;
  uint32_t buffer_count;
  uint32_t buffer_address;
  uint32_t buffer_next_address;
#ifdef defined(ETHERNET_DESCRIPTOR_EXTENDED) == 1
  uint32_t reserved0;
  uint32_t reserved1;
  uint32_t time_stamp_low;
  uint32_t time_stamp_high;
#endif
} ethernet_transmit_descriptor_t;


typedef struct ethernet_receive_descriptor_s {
  uint32_t status;
  uint32_t buffer_count;
  uint32_t buffer_address;
  uint32_t buffer_next_address;
#ifdef defined(ETHERNET_DESCRIPTOR_EXTENDED) == 1
  uint32_t status_extended;
  uint32_t reserved1;
  uint32_t time_stamp_low;
  uint32_t time_stamp_high;
#endif
} ethernet_receive_descriptor_t;


typedef struct ethernet_descriptor_s {
  uint32_t status;
  uint32_t buffer_count;
  uint32_t buffer_address;
  uint32_t buffer_next_address;
#ifdef defined(ETHERNET_DESCRIPTOR_EXTENDED) == 1
  uint32_t status_extended;
  uint32_t reserved1;
  uint32_t time_stamp_low;
  uint32_t time_stamp_high;
#endif
} ethernet_descriptor_t;

#endif /* _STM32ETH_REGS_H__ */