/**
  ******************************************************************************
  * @file    shdsl.h
  * @author  Dmitry Vakhrushev ( vdv.18@mail.ru )
  * @version V1.0
  * @date    20.05.2015
  * @brief   --
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; Copyright (C) 2015 by vdv.18@mail.ru ( V1.0 ) </center></h2>
  ******************************************************************************
  */ 
  
#ifndef _SHDSL_H__
#define _SHDSL_H__

#include "err_hw.h"

typedef enum shdsl_cmd_e {
  SHDSL_CMD_UPDATE,
  
} shdsl_cmd_t;


typedef struct shdsl_cfg_s {
  uint8_t master;
  uint8_t mode;
  uint8_t annex;
  uint8_t modulation;
  uint8_t tcpam;
  uint8_t sync;
  uint8_t pbo;
  uint32_t rate;
} shdsl_cfg_t;

typedef struct shdsl_stat_s {
  uint8_t dsl_link;
} shdsl_stat_t;

/**
 * @brief shdsl initialization
 *
 * @retval none.
 *
 */
err_hw_t shdsl_init( void );

/**
 * @brief shdsl deinitialization
 *
 * @retval none.
 *
 */
err_hw_t shdsl_deinit( void );

/**
 * @brief shdsl deinitialization
 *
 * @retval none.
 *
 */
err_hw_t shdsl_update( void );

err_hw_t shdsl_annex( uint32_t value );
err_hw_t shdsl_pbo( uint32_t value );
err_hw_t shdsl_sync( uint32_t value );
err_hw_t shdsl_rate( uint32_t rate );
err_hw_t shdsl_tcpam( uint32_t tcpam );
err_hw_t shdsl_mode( uint32_t mode );
err_hw_t shdsl_set_cfg( shdsl_cfg_t *cfg );
err_hw_t shdsl_get_cfg( shdsl_cfg_t *cfg );
err_hw_t shdsl_get_stat( shdsl_stat_t *stat );



/**
 * @brief shdsl command
 *
 * @param[in]    shdsl_cmd_t 		-	shdsl command.
 * @param[inout] ...  		                -	shdsl command parameters.
 *
 * @retval none.
 *
 */
err_hw_t shdsl_cmd( shdsl_cmd_t cmd, ... );


#endif /* _SHDSL_H__ */


