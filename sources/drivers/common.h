/* -----------------------------------------------------------------------------
 * Copyright (c) 2013-2014 xxx Ltd.
 *
 * $Date:        2. Jan 2014
 * $Revision:    V2.00
 *
 * Project:      Common Driver definitions
 * -------------------------------------------------------------------------- */

/* History:
 *  Version 2.00
 *    Changed prefix ARM_DRV -> ARM_DRIVER
 *    Added General return codes definitions
 *  Version 1.10
 *    Namespace prefix ARM_ added
 *  Version 1.00
 *    Initial release
 */

#ifndef __COMMON_H
#define __COMMON_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#define DRIVER_VERSION_MAJOR_MINOR(major,minor) (((major) << 8) | (minor))

/**
\brief Driver Version
*/
typedef struct _driver_version {
  uint16_t api;                         ///< API version
  uint16_t drv;                         ///< Driver version
} driver_version;

/* General return codes */
#define DRIVER_OK                 0 ///< Operation succeeded 
#define DRIVER_ERROR             -1 ///< Unspecified error
#define DRIVER_ERROR_BUSY        -2 ///< Driver is busy
#define DRIVER_ERROR_TIMEOUT     -3 ///< Timeout occurred
#define DRIVER_ERROR_UNSUPPORTED -4 ///< Operation not supported
#define DRIVER_ERROR_PARAMETER   -5 ///< Parameter error
#define DRIVER_ERROR_SPECIFIC    -6 ///< Start of driver specific errors 

/**
\brief General power states
*/ 
typedef enum _power_state {
  POWER_OFF,                        ///< Power off: no operation possible
  POWER_LOW,                        ///< Low Power mode: retain state, detect and signal wake-up events
  POWER_FULL                        ///< Power on: full operation at maximum performance
} power_state;

#endif /* __COMMON_H */
