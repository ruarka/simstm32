#ifndef SIM_SIM_INC_H_
#define SIM_SIM_INC_H_

/*! \file      sim_inc.h
    \version   0.0
    \date      23 june 2020 ã. 22:45:05
    \brief     
    \details   
    \bug
    \copyright 
    \author ruarka
*/

// #include "stm_globals.h"

/**
  * @brief Configuration of the Cortex-M0+ Processor and Core Peripherals
  */
#define __CM0PLUS_REV             0U /*!< Core Revision r0p0                            */
#define __MPU_PRESENT             0U /*!< STM32L0xx  provides no MPU                    */
#define __VTOR_PRESENT            1U /*!< Vector  Table  Register supported             */
// #define __NVIC_PRIO_BITS          2U /*!< STM32L0xx uses 2 Bits for the Priority Levels */
// #define __Vendor_SysTickConfig    0U /*!< Set to 1 if different SysTick Config is used  */


#include "core_cm0plus.h"
#include "sim_irq.h"
#include "cir_ifs.h"
#include "stm_rcc.h"
#include "stm_flash.h"
#include "sim_cycle_timers.h"
#include "sim_systick.h"
#include "sim_scb.h"
#include "sim_nvic.h"
#include "sim_gpio.h"
#include "sim_stm.h"
#include "sim_32l0x.h"
#include "sim_core.h"
#include "sim_m0pins.h"
#include "sim_io.h"
#include "sim_m0pins.h"

/* ------------------------------------------------------------------------------------------------
 *                                     Macros
 * ------------------------------------------------------------------------------------------------
 */
/* ------------------------------------------------------------------------------------------------
 *                                 Type definitions
 * ------------------------------------------------------------------------------------------------
 */

/* ------------------------------------------------------------------------------------------------
 *                                    Functions declarations
 * ------------------------------------------------------------------------------------------------
 */
#ifdef __cplusplus
extern "C"
{
#endif


#ifdef __cplusplus
}
#endif


#endif /* SIM_SIM_INC_H_ */
