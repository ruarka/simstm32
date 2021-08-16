/*! \file      sim_systick.h
    \version   0.0
    \date      23 june 2020 ã. 21:40:37
    \brief     
    \details   
    \bug
    \copyright 
    \author ruarka
*/

#ifndef SIM_SIM_SYSTICK_H_
#define SIM_SIM_SYSTICK_H_

#ifdef __cplusplus
extern "C"
{
#endif


/* ------------------------------------------------------------------------------------------------
 *                                     Macros
 * ------------------------------------------------------------------------------------------------
 */
/* ------------------------------------------------------------------------------------------------
 *                                 Type definitions
 * ------------------------------------------------------------------------------------------------
 */
/**
  \brief  Structure type to access the System Timer (SysTick).
 */
//typedef struct
//{
//  uint32_t CTRL;                   /*!< Offset: 0x000 (R/W)  SysTick Control and Status Register */
//  uint32_t LOAD;                   /*!< Offset: 0x004 (R/W)  SysTick Reload Value Register */
//  uint32_t VAL;                    /*!< Offset: 0x008 (R/W)  SysTick Current Value Register */
//  uint32_t CALIB;                  /*!< Offset: 0x00C (R/ )  SysTick Calibration Register */
//} systick_regs_t;

/*
 * System ticks counter.
 */
typedef struct stm_systick_t
{
	struct stm_circuit_ifs_t* cir_cb_ifs;

	struct mcu_t* mcu;

//	systick_regs_t* regs;

	uint64_t frequency;							// frequency rcc feed with
	uint64_t nano_ticks_per_cycle;	// to calculate cycletimer gap

	uint64_t when_started; 	/*!< The time in nano when cycle timer was started */
	uint64_t when;					/*!< The time in nano when cycle timer should be fired */

	uint64_t	CTRL_a; 	/*!< CTRL value cycle timer was initiated */
//	uint64_t	LOAD_a;		/*!< LOAD value cycle timer was initiated */
// 	uint64_t  VAL_a;    /*!< CVR value cycle timer was initiated */
// 	uint64_t  CALIB_a;  /*!< CALIB value cycle timer was initiated */

	uint64_t	CTRL; 		/*!< Offset: 0x000 (R/W)  SysTick Control and Status Register */
	uint64_t	LOAD;			/*!< Offset: 0x004 (R/W)  SysTick Reload Value Register */
	uint64_t  VAL; 			/*!< Offset: 0x008 (R/W)  SysTick Current Value Register */
	uint64_t  CALIB;		/*!< Offset: 0x00C (R/ )  SysTick Calibration Register */


} stm_systick_t;

/* ------------------------------------------------------------------------------------------------
 *                                    Functions declarations
 * ------------------------------------------------------------------------------------------------
 */

/* On RCC update callback function  */
extern void systick_rcc_onupdate_cb(struct mcu_t * mcu, uint32_t param, uint64_t ticks_per_nsec, uint64_t frequency);

/* Systick Timer fabric function */
stm_systick_t* sysctick_circuit_fabric( struct mcu_t * mcu );

#ifdef __cplusplus
}
#endif


#endif /* SIM_SIM_SYSTICK_H_ */
