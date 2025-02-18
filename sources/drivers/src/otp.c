#include "otp.h"
#include "stm32f4xx_flash.h"

//#define TEST
#ifdef TEST


static uint8_t otp_memory[512+16];

/**
 * @brief  OTP memory start address
 */
#undef  OTP_START_ADDR
#define OTP_START_ADDR		((uint32_t)&otp_memory[0])

/**
 * @brief  OTP memory lock address
 */
#undef OTP_LOCK_ADDR
#define OTP_LOCK_ADDR		((uint32_t)&otp_memory[512])
otp_result_t otp_write(uint8_t block, uint8_t byte, uint8_t data) {
	FLASH_Status status = FLASH_BUSY;
	
	/* Check input parameters */
	if (
		block >= OTP_BLOCKS ||
		byte >= OTP_BYTES_IN_BLOCK
	) {
		/* Invalid parameters */
		
		/* Return error */
		return otp_result_error;
	}
        
        *(uint8_t*)(OTP_START_ADDR+block*32+byte) = data;
        status == FLASH_COMPLETE;
	
	/* Check status */
	if (status == FLASH_COMPLETE) {
		/* Return OK */
		return otp_result_ok;
	} else {
		/* Return error */
		return otp_result_error;
	}
}

uint8_t otp_read(uint8_t block, uint8_t byte) {
	uint8_t data;
	
	/* Check input parameters */
	if (
		block >= OTP_BLOCKS ||
		byte >= OTP_BYTES_IN_BLOCK
	) {
		/* Invalid parameters */
		return 0;
	}
	
	/* Get value */
	data = *(uint8_t *)(OTP_START_ADDR + block * OTP_BYTES_IN_BLOCK + byte);
	
	/* Return data */
	return data;
}

otp_result_t otp_blocklock(uint8_t block) {
	FLASH_Status status = FLASH_BUSY;
	
	/* Check input parameters */
	if (block >= OTP_BLOCKS) {
		/* Invalid parameters */
		
		/* Return error */
		return otp_result_error;
	}
	
	/* Write byte */
        *(uint8_t *)(OTP_LOCK_ADDR + block) = 0x00;
        status = FLASH_COMPLETE;
	
	/* Check status */
	if (status == FLASH_COMPLETE) {
		/* Return OK */
		return otp_result_ok;
	}
	
	/* Return error */
	return otp_result_error;
}

#define OTP_BLOCK_LOCKED(block) ((*(uint8_t *) (OTP_LOCK_ADDR + block)) == 0x00 ? 1 : 0)
otp_result_t otp_blocklocked(uint8_t _block)
{
  if(_block >=16)
  {
    return otp_result_error;
  }
  if(OTP_BLOCK_LOCKED(_block))
  {
    return otp_result_ok;
  }
  return otp_result_error;
}

#else
otp_result_t otp_write(uint8_t block, uint8_t byte, uint8_t data) {
	FLASH_Status status;
	
	/* Check input parameters */
	if (
		block >= OTP_BLOCKS ||
		byte >= OTP_BYTES_IN_BLOCK
	) {
		/* Invalid parameters */
		
		/* Return error */
		return otp_result_error;
	}
	
	/* Unlock FLASH */
	FLASH_Unlock();

	/* Clear pending flags (if any) */  
	FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | 
					FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR|FLASH_FLAG_PGSERR); 
	
	/* Wait for last operation */
	status = FLASH_WaitForLastOperation();
	
	/* If it is not success, return error */
	if (status != FLASH_COMPLETE) {
		/* Lock FLASH */
		FLASH_Lock();
		
		/* Return error */
		return otp_result_error;
	}
	
	/* Write byte */
	status = FLASH_ProgramByte(OTP_START_ADDR + block * OTP_BYTES_IN_BLOCK + byte, data);

	/* Lock FLASH */
	FLASH_Lock();
	
	/* Check status */
	if (status == FLASH_COMPLETE) {
		/* Return OK */
		return otp_result_ok;
	} else {
		/* Return error */
		return otp_result_error;
	}
}

uint8_t otp_read(uint8_t block, uint8_t byte) {
	uint8_t data;
	
	/* Check input parameters */
	if (
		block >= OTP_BLOCKS ||
		byte >= OTP_BYTES_IN_BLOCK
	) {
		/* Invalid parameters */
		return 0;
	}
	
	/* Get value */
	data = *(__IO uint8_t *)(OTP_START_ADDR + block * OTP_BYTES_IN_BLOCK + byte);
	
	/* Return data */
	return data;
}

otp_result_t otp_blocklock(uint8_t block) {
	FLASH_Status status;
	
	/* Check input parameters */
	if (block >= OTP_BLOCKS) {
		/* Invalid parameters */
		
		/* Return error */
		return otp_result_error;
	}
	
	/* Unlock FLASH */
	FLASH_Unlock();

	/* Clear pending flags (if any) */  
	FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | 
					FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR|FLASH_FLAG_PGSERR); 
	
	/* Wait for last operation */
	status = FLASH_WaitForLastOperation();
	
	/* If it is not success, return error */
	if (status != FLASH_COMPLETE) {
		/* Lock FLASH */
		FLASH_Lock();
		
		/* Return error */
		return otp_result_error;
	}
	
	/* Write byte */
	status = FLASH_ProgramByte(OTP_LOCK_ADDR + block, 0x00);

	/* Lock FLASH */
	FLASH_Lock();
	
	/* Check status */
	if (status == FLASH_COMPLETE) {
		/* Return OK */
		return otp_result_ok;
	}
	
	/* Return error */
	return otp_result_error;
}

#define OTP_BLOCK_LOCKED(block) ((*(uint8_t *) (OTP_LOCK_ADDR + block)) == 0x00 ? 1 : 0)
otp_result_t otp_blocklocked(uint8_t _block)
{
  if(_block >=16)
  {
    return otp_result_error;
  }
  if(OTP_BLOCK_LOCKED(_block))
  {
    return otp_result_ok;
  }
  return otp_result_error;
}
                        
#endif
