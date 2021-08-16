/*! \file      sim_systick.c
    \version   0.0
    \date      23 june 2020 ã. 21:40:20
    \brief     
    \details   
    \bug
    \copyright 
    \author ruarka
*/

/* ------------------------------------------------------------------------------------------------
 *                                 Includes
 * ------------------------------------------------------------------------------------------------
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "stdint.h"

#include "sim_inc.h"

/* ------------------------------------------------------------------------------------------------
 *                                 Type definitions
 * ------------------------------------------------------------------------------------------------
 */

/* ------------------------------------------------------------------------------------------------
 *                                     Macros
 * ------------------------------------------------------------------------------------------------
 */
// #define SYSTICK_TEST

/* ------------------------------------------------------------------------------------------------
 *                                 Global Variables
 * ------------------------------------------------------------------------------------------------
 */

/* ------------------------------------------------------------------------------------------------
 *                         Local Variables & Functions
 * ------------------------------------------------------------------------------------------------
 */
/*
 * RCC Callback function
 */
void systick_rcc_onupdate_cb(struct mcu_t * mcu, uint32_t param, uint64_t ticks_per_nsec, uint64_t frequency)
{
	mcu->systick->nano_ticks_per_cycle = ticks_per_nsec;
	mcu->systick->frequency            = frequency;
}

/**
  \brief   System Ticks Timer init function
  \details ...
  \param [in]      mcu  Pointer to MCU representation object
  \return             0  Was successfully initialized
  \return             Other Error
  \note    IRQn must not be negative.
 */
/*
 *
 */
static uint32_t stm_systick_init( mcu_t * mcu )
{
	stm_systick_t* p = mcu->systick;
	if( p )
	{
		p->CTRL  = 0x00;
		p->LOAD  = 0x00;
		p->CALIB = 0x00;
		p->VAL 	 = 0x00;

		p->CTRL_a  = 0x00;
//		p->LOAD_a  = 0x00;
//		p->CALIB_a = 0x00;
//		p->VAL_a	 = 0x00;

		p->when  	 = 0x00;
		p->when_started = 0x00;

		p->frequency 						= 0x00;
		p->nano_ticks_per_cycle = 0xaaaa;
	}

	// bind core to rcc
	stm_rcc_updt_cb_reg( mcu, systick_rcc_onupdate_cb, NULL );

	return 0x00;
}

/*
 *
 */
void stm_systick_reset(mcu_t * mcu, uint32_t type )
{
	stm_systick_t* p = mcu->systick;

	if( p )
	{
		p->CTRL  = 0x00;
		p->LOAD  = 0x00;
		p->CALIB = 0x00;
		p->VAL 	 = 0x00;

		p->CTRL_a  = 0x00;
//		p->LOAD_a  = 0x00;
//		p->CALIB_a = 0x00;
//		p->VAL_a	 = 0x00;

		p->when  	 = 0x00;
		p->when_started = 0x00;

		p->frequency 						= 0x00;
		p->nano_ticks_per_cycle = 0xaaaa;
	}
}

/*
 *
 */
void stm_systick_release( mcu_t * mcu, void* obj )
{
	if( mcu )
	{
		if( mcu->systick->cir_cb_ifs )
			free( mcu->systick->cir_cb_ifs );
		mcu->systick->cir_cb_ifs = NULL;

		if( mcu->systick )
			free( mcu->systick );
		mcu->systick = NULL;
	}
}

/*
 *
 */
void stm_systick_wr_cb_reg( struct mcu_t * mcu, void* cb_wr, uint32_t addr, void* param )
{
}

/*
 *
 */
void stm_systick_rd_cb_reg( struct mcu_t * mcu, void* cb_rd, uint32_t addr, void* param )
{
}

stm_cycle_count_t stm_systick_cycle_timer( struct stm_t * stm, stm_cycle_count_t when,	void * param)
{
	stm_systick_t* p = stm->mcu->systick;
	stm_cycle_count_t new_when =
			stm->mcu->core->nano_ticks +( p->LOAD +1 )* p->nano_ticks_per_cycle;

	p->VAL = p->LOAD;

	// register interrupt in nvic
	nvic_regist_interrupt( stm->mcu, SysTick_IRQN );

#ifdef SYSTICK_TEST
	// for testing
	new_when = stm->mcu->core->nano_ticks +100*p->nano_ticks_per_cycle;
	p->VAL = 100;
#endif

	// %I64u
	SYSTICK_TRACE( stm, LOG_TRACE, "\nCYT(reinit) after[%I64u] start[%I64u] cycle[%I64u]\n", p->when, stm->nano_ticks, stm->cycle );

	return new_when;
}


static void stm_systick_ctrl_wr_update( struct mcu_t* mcu )
{
	stm_systick_t* p = mcu->systick;

	// enabled/disabled
	if( p->CTRL_a & 0x01 )
	{
		if(!( p->CTRL & 0x01 ))
		{
			// cancel cycle timer
			stm_cycle_timer_cancel( mcu->core, stm_systick_cycle_timer, 0x00 );

			// update CVR
			uint64_t delta = mcu->core->nano_ticks - mcu->systick->when_started;
			p->VAL = 0xffffff - ((delta/mcu->systick->nano_ticks_per_cycle )& 0xffffff);
		}
	}else{
		if(p->CTRL & 0x01 )
		{
			// calculate when
			p->when_started = mcu->core->nano_ticks;

			// TODO: External clock should be also supported
			if( p->VAL )
			  p->when = p->VAL * p->nano_ticks_per_cycle;
			else
				p->when =( p->LOAD +1 )* p->nano_ticks_per_cycle;

#ifdef SYSTICK_TEST
			p->when = 100*p->nano_ticks_per_cycle;
			p->LOAD = 100;
#endif

			// register cycle timer
			stm_cycle_timer_register( mcu->core, p->when, stm_systick_cycle_timer, 0x00 );

	  	SYSTICK_TRACE( mcu->core, LOG_TRACE, "\nCYT(init) after[%I64u] start[%I64u] cycle[%I64u]\n", p->when, mcu->core->nano_ticks, mcu->core->cycle );

//	  	// ART
//			uint64_t val =( mcu->systick->when +mcu->systick->when_started -mcu->core->nano_ticks );
//			uint32_t val32 = val/ mcu->systick->nano_ticks_per_cycle;
//	  	SYSTICK_TRACE( mcu->core, LOG_TRACE, "\nCVR[0x%08x]", val32 );
		}
	}

	p->CTRL_a = p->CTRL;
}

static void stm_systick_cvr_wr_update( struct mcu_t* mcu )
{
	stm_systick_t* p = mcu->systick;

	if( p->CTRL & 0x01 )
	{
		// systick enabled
		// cancel cycle timer
		stm_cycle_timer_cancel( mcu->core, stm_systick_cycle_timer, 0x00 );

		// calculate when
		p->when_started = mcu->core->nano_ticks;

		// TODO: External clock should be also supported
		if( p->VAL )
		  p->when = p->VAL * p->nano_ticks_per_cycle;
		else
			p->when =( p->LOAD +1 )* p->nano_ticks_per_cycle;

#ifdef SYSTICK_TEST
			p->when = 3*p->nano_ticks_per_cycle;
			p->LOAD = 2;
#endif

		// register cycle timer
		stm_cycle_timer_register( mcu->core, p->when, stm_systick_cycle_timer, 0x00 );

		// %I64u
//  	SYSTICK_TRACE( mcu->core, LOG_TRACE, "\nCYT(init) when[%lu] start[%lu] cycle[%lu]", p->when, mcu->core->nano_ticks, mcu->core->cycle );
  	SYSTICK_TRACE( mcu->core, LOG_TRACE, "\nCYT(init) after[%I64u] start[%I64u] cycle[%I64u]\n", p->when, mcu->core->nano_ticks, mcu->core->cycle );

//  	// ART
//		uint64_t val =( mcu->systick->when +mcu->systick->when_started -mcu->core->nano_ticks );
//		uint32_t val32 = val/ mcu->systick->nano_ticks_per_cycle;
//  	SYSTICK_TRACE( mcu->core, LOG_TRACE, "\nCVR[0x%08x]", val32 );
	}

//	p->VAL_a = p->VAL;
}

/*
 * uint32_t word writing
 */
void stm_systick_write( struct mcu_t* mcu, uint32_t addr, uint32_t val, void* param )
{
 	switch (addr) {
		case 0x00:
			// CSR
		{
			//
			if((( val & 0x01 )!=( mcu->systick->CTRL & 0x01 ))
				||(( val & 0x04 )!=( mcu->systick->CTRL & 0x04 )))
			{
				mcu->systick->CTRL = val;
				stm_systick_ctrl_wr_update( mcu );
			}
		}
 		break;

		case 0x04:
			// RVR
			if( mcu->systick->LOAD != val )
			{
			   mcu->systick->LOAD = val;
//				mcu->systick->LOAD = 0x830;
//				stm_systick_load_wr_update( mcu );
			}
			break;

		case 0x08:
			// CVR
			mcu->systick->VAL = val;
			stm_systick_cvr_wr_update( mcu );
			break;

		default:
			return;
	}
}

/*
 * uint32_t word reading
 */
void stm_systick_read( struct mcu_t* mcu, uint32_t addr, uint32_t* val, void* param )
{
	switch (addr) {
		case 0x00:
			// CSR
			*val = mcu->systick->CTRL;
			break;

		case 0x04:
			// RVR
			*val = mcu->systick->LOAD;
			break;

		case 0x08:
			// CVR
			if( mcu->systick->CTRL & 0x01 )
			{
				uint64_t delta = mcu->core->nano_ticks - mcu->systick->when_started;
				*val = 0xffffff - ((delta/mcu->systick->nano_ticks_per_cycle )& 0xffffff);
			}else
				*val = mcu->systick->VAL;
			break;

		case 0x0c:
			// CALIB
			*val = mcu->systick->CALIB;
			break;

		default:
			return;
	}
}

/*
 *
 */
void stm_systick_dump_state( void* pSystick )
{
	stm_systick_t * st =( stm_systick_t* ) pSystick;
	stm_t * stm = st->mcu->core ;
	if( stm )
	{
		SYSTICK_TRACE( stm, LOG_TRACE, "\n\nSYSTICK Circuit Dumping\n--------------" );

		// CSR
  	SYSTICK_TRACE( stm, LOG_TRACE, "\nCSR[%08x]   COUNTFLAG[%d] CLKSOURCE[%d]", st->CTRL, (st->CTRL>>16)&0x01, (st->CTRL>>2)&0x01 );
  	SYSTICK_TRACE( stm, LOG_TRACE, " TICKINT[%d] ENABLE[%d]", (st->CTRL>>1)&0x01, (st->CTRL)&0x01 );

  	// RVR
  	SYSTICK_TRACE( stm, LOG_TRACE, "\nRVR[%08x]", (st->LOAD)&0x0ffffff );

  	// CVR
  	uint64_t val;
		if( stm->mcu->systick->CTRL & 0x01 )
		{
//			uint64_t delta = stm->nano_ticks - stm->mcu->systick->when_started;
//			val = 0xffffff - ((delta/stm->mcu->systick->nano_ticks_per_cycle )& 0xffffff);
			val =( stm->mcu->systick->when +stm->mcu->systick->when_started -stm->nano_ticks );
			val = val/ stm->mcu->systick->nano_ticks_per_cycle;
		}else
			val = stm->mcu->systick->VAL;
  	SYSTICK_TRACE( stm, LOG_TRACE, "\nCVR[0x%08x]", val & 0x0ffffff );

  	// CALIB
  	SYSTICK_TRACE( stm, LOG_TRACE, "\nCALIB[%08x] NOREF[%d] SKEW[%d]", (st->CALIB)&0x0ffffff, (st->CALIB>>31)&0x01, (st->CALIB>>30)&0x01 );
	}
}

/*
 *
 */
stm_systick_t* sysctick_circuit_fabric( struct mcu_t * mcu )
{
	if( !mcu ) return 0x00;

	stm_systick_t* p = malloc( sizeof( stm_systick_t ));
	if( p )
	{
	  memset( p, 0x00, sizeof( stm_systick_t ) );

		p->cir_cb_ifs = malloc( sizeof( stm_circuit_ifs_t ));
		if( p->cir_cb_ifs )
		{
		  memset( p->cir_cb_ifs, 0x00, sizeof( stm_circuit_ifs_t ));
			p->cir_cb_ifs->init					= stm_systick_init;
			p->cir_cb_ifs->reset				= stm_systick_reset;
			p->cir_cb_ifs->release			= stm_systick_release;
			p->cir_cb_ifs->wr_cb_reg		= stm_systick_wr_cb_reg;
			p->cir_cb_ifs->rd_cb_reg		= stm_systick_rd_cb_reg;
			p->cir_cb_ifs->write				= stm_systick_write;
			p->cir_cb_ifs->read					= stm_systick_read;
			p->cir_cb_ifs->dump					= stm_systick_dump_state;
		}
	}

	p->mcu = mcu;
	return p;
}

