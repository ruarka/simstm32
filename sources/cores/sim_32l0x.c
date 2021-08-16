/*
 * sim_32l031.c
 *
 *  Created on: 31 oct 2019 ã.
 *      Author: dell
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "sim_inc.h"
#include "sim_32l0x.h"
#include "sim_m0pins.h"
#include "sim_m0p_r0p1.h"

/*
 *
 */
void stm32l0x_init(struct stm_t * stm)
{
	// init devices pool used to define address region
	// in read write operations
	stm->devs_pool = NULL;

  stm->flash_start_adr =(uint32_t )0x08000000;
  stm->flash_end_adr =(uint32_t )0x08000000 +0x7fff;

  stm->flash = malloc( 0x7fff +16 ); // One extra byte just for insurance

  void * p = stm->flash;
  memset( p, 0xa5, 0x7fff+16 );

  // internal register initialization
  for( uint32_t i=0; i<15; i++ )
  	stm->R[ i ] = 0xffffffff;

  // sram - data initializaton
  stm->data = malloc( 8192 +1 ); //

  p 							= stm->data;
  stm->ramstart 	= 0x20000000;
  stm->ramend 		= 0x20000000 +8192;
  memset( p, 0xff, 8192+1 );

  stm->mcu->sram 				= stm->data;
  stm->mcu->sram_start 	= stm->ramstart;
  stm->mcu->sram_end 		= stm->ramend;

  // initialize various cycles counters
  stm->cycle 						= 0;
  stm->run_cycle_count	= 0;
	stm->run_cycle_limit  = 0;

	// nano ticks instead of cycle mechanics
	stm->nano_ticks 						= 0;
	stm->nano_ticks_per_cycle 	= 0;
	stm->nano_ticks_limit				= 0;

	// bind core to rcc
	stm_rcc_updt_cb_reg( stm->mcu, core_rcc_onupdate_cb, NULL );

#ifdef CONFIG_SIMSTM_TRACE
  // trace off
  stm->trace = 1;
#else
  stm->trace = 0;
#endif
}
/*
 *
 */
void stm32l0x_reset(struct stm_t * stm)
{
	// core reset
  stm->reset_pc = stm->flash[ 7 ]<<24 | stm->flash[ 6 ]<<16 | stm->flash[ 5 ]<<8 | stm->flash[ 4 ];
  stm->reset_sp = stm->flash[ 3 ]<<24 | stm->flash[ 2 ]<<16 | stm->flash[ 1 ]<<8 | stm->flash[ 0 ];

  stm->pc 		= stm->reset_pc;
  stm->msp 		= stm->reset_sp;
  stm->R[13] 	= stm->reset_sp;
  stm->R[14]  = 0x00;

  // xPSR
  stm->APSR.val = 0;
  stm->IPSR.val = 0;
  stm->EPSR.val = 0;
  stm->EPSR.bits.T = 1;

  stm->state = cpu_Running;
}
/*
 *
 */
void stm32l0x_release(struct stm_t * stm)
{
	if( stm->flash )
	{
		free(stm->flash);
		stm->flash = NULL;
	}

	if( stm->data )
	{
		free( stm->data );
		stm->data = NULL;
	}

	// free device pool list
	hw_device_address_cell_t* dp = stm->devs_pool;
	while( dp )
	{
		hw_device_address_cell_t* ptmp = dp;
		dp = dp->next;
		free( ptmp );
	}

	if( stm->circuit_ifs )
	{
	  free( stm->circuit_ifs );
	  stm->circuit_ifs = NULL;
	}

  stm->mcu->core = NULL;
	free( stm );
}
/*
 *
 */
void stm32l0x_dump( struct stm_t * stm )
{
	STM_LOG( stm, LOG_TRACE, "\n\nCORE Dumping" );
	STM_LOG( stm, LOG_TRACE, "\ncycle[%lu] run_cycle_count[%lu]", stm->cycle, stm->run_cycle_count );
	STM_LOG( stm, LOG_TRACE, "\nASPR[N.Z.C.V]=[%01d.%01d.%01d.%01d]", stm->APSR.bits.N, stm->APSR.bits.Z, stm->APSR.bits.C, stm->APSR.bits.V );

	for (uint32_t var = 0; var < 13; ++var)
	{
		STM_LOG( stm, LOG_TRACE, "\nR[%d]=%08x", var, stm->R[var] );
	}
	STM_LOG( stm, LOG_TRACE, "\nSP,MSP,PSP=[%08x,%08x,%08x]", stm->R[ 13 ], stm->msp, stm->psp );
	STM_LOG( stm, LOG_TRACE, "\nR[14](LR)=%08x", stm->R[ 14 ]);
	STM_LOG( stm, LOG_TRACE, "\nR[15](PC)=[%08x]", stm->pc );

	STM_LOG( stm, LOG_TRACE, "\nNanoTicks[%I64u]", stm->nano_ticks );
}
/*
 * RCC Callback function
 */
void core_rcc_onupdate_cb(struct mcu_t * mcu, uint32_t param, uint64_t ticks_per_nsec, uint64_t frequency)
{
	mcu->core->nano_ticks_per_cycle = ticks_per_nsec;
	mcu->core->frequency 					 = frequency;
}


const struct mcu_t SIM_CORENAME = {
};
