#ifndef __OTP_H__
#define __OTP_H__


#include "stdint.h"

/**
 * @brief  OTP memory start address
 */
#define OTP_START_ADDR		(0x1FFF7800)

/**
 * @brief  OTP memory lock address
 */
#define OTP_LOCK_ADDR		(0x1FFF7A00)
	
/**
 * @brief  Number of OTP blocks 
 */
#define OTP_BLOCKS              16

/**
 * @brief  Number of bytes in one block
 */
#define OTP_BYTES_IN_BLOCK	32

/**
 * @brief  Number of all OTP bytes
 */
#define OTP_SIZE			(OTP_BLOCKS * OTP_BYTES_IN_BLOCK)

/**
 * @}
 */
 
/**
 * @defgroup TM_OTP_Typedefs
 * @brief    Library Typedefs
 * @{
 */

/**
 * @brief  Result enumeration
 */
typedef enum {
    otp_result_ok = 0,  /*!< Everything OK */
    otp_result_error    /*!< An error occurred.
	                            This is returned when you try to write data to location which does not exists,
                                or if you try to write data to locked block */
} otp_result_t;

/**
 * @}
 */

/**
 * @defgroup OTP_Functions
 * @brief    Library Functions
 * @{
 */

/**
 * @brief  Writes one-time data to specific block and specific byte in this block
 * @note   You can only ONCE write data at desired byte in specific block, if you will try to do it more times, you can have broken data at this location.
 * @param  block: OTP block number, 0 to 15 is allowed
 * @param  byte: OTP byte inside one block, 0 to 31 is allowed
 * @param  data: Data to be written to OTP memory
 * @retval Member of @ref OTP_Result_t enumeration 
 */
otp_result_t otp_write(uint8_t block, uint8_t byte, uint8_t data);

/**
 * @brief  Reads data from specific block and specific byte in this block
 * @note   You can read data unlimited times from locations
 * @param  block: OTP block number, 0 to 15 is allowed
 * @param  byte: OTP byte inside one block, 0 to 31 is allowed
 * @retval Value at specific block and byte location, or 0 if location is invalid
 */
uint8_t otp_read(uint8_t block, uint8_t byte);

/**
 * @brief  Locks entire block to prevent future programming inside
 * @note   When you lock your block, then you are not able to program it anymore.
 * 	       Even, if it is totally empty. You can't unlock it back!
 * @param  block: OTP block number, 0 to 15 is allowed
 * @retval Member of @ref OTP_Result_t enumeration
 */
otp_result_t otp_blocklock(uint8_t block);

/**
 * @brief  Checks if block is locked or not
 * @param  block: OTP block number, 0 to 15 is allowed
 * @retval Block lock status
 *            - 0: Block is not locked
 *            - > 0: Block locked
 */
otp_result_t otp_blocklocked(uint8_t block);



#endif//__OTP_H__