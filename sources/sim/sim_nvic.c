/*! \file      sim_nvic.c
    \version   0.0
    \date      8 July 2020 22:39:16
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
void nvic_rcc_onupdate_cb(struct mcu_t * mcu, uint32_t param, uint64_t ticks_per_nsec, uint64_t frequency)
{
	mcu->nvic->nano_ticks_per_cycle = ticks_per_nsec;
	mcu->nvic->frequency 					 = frequency;
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
static uint32_t stm_nvic_init( mcu_t * mcu )
{
	stm_nvic_t* p = mcu->nvic;

	if( p )
	{
		p->regs.ISER[ 0 ]= 0x00;
		p->regs.ICER[ 0 ]= 0x00;
		p->regs.ISPR[ 0 ]= 0x00;
		p->regs.ICPR[ 0 ]= 0x00;

		for (int var = 0; var < 8; ++var)
			p->regs.IP[ var ]= 0x00;

		// clear pending register
		mcu->nvic->pending_interrupts = 0;
		mcu->nvic->pending_interrupts_ext = 0;
	}

	// bind core to rcc
	stm_rcc_updt_cb_reg( mcu, nvic_rcc_onupdate_cb, NULL );

	return 0x00;
}

/*
 *
 */
void stm_nvic_reset(mcu_t * mcu, uint32_t type )
{
	stm_nvic_t* p = mcu->nvic;

	if( p )
	{
		p->regs.ISER[ 0 ]= 0x00;
		p->regs.ICER[ 0 ]= 0x00;
		p->regs.ISPR[ 0 ]= 0x00;
		p->regs.ICPR[ 0 ]= 0x00;

		for (int var = 0; var < 8; ++var)
			p->regs.IP[ var ]= 0x00;

		// clear pending register
		mcu->nvic->pending_interrupts = 0;
		mcu->nvic->pending_interrupts_ext = 0;
	}
}

/*
 *
 */
void stm_nvic_release( mcu_t * mcu, void* obj )
{
	if( mcu )
	{
		if( mcu->nvic )
			free( mcu->nvic->cir_cb_ifs );
		mcu->nvic->cir_cb_ifs = NULL;

		if( mcu->nvic )
		  free( mcu->nvic );

		mcu->nvic = NULL;
	}
}

/*
 *
 */
void stm_nvic_wr_cb_reg( struct mcu_t * mcu, void* cb_wr, uint32_t addr, void* param )
{
}

/*
 *
 */
void stm_nvic_rd_cb_reg( struct mcu_t * mcu, void* cb_rd, uint32_t addr, void* param )
{
}

/*
 * uint32_t word writing
 */
void stm_nvic_write( struct mcu_t* mcu, uint32_t addr, uint32_t val, void* param )
{
	switch ( addr ) {
		case 0x00:
			mcu->nvic->regs.ISER[ 0 ] = val;
			break;

		case 0x80:
			mcu->nvic->regs.ICER[ 0 ]  = val;
			break;

		case 0x100:
			mcu->nvic->regs.ISPR[ 0 ] = val;
			break;

		case 0x180:
			mcu->nvic->regs.ICPR[ 0 ] = val;
			break;

		default:
			{
				if(( 0x300 <= addr )
						&&( addr <( 0x300+8 ) ))
				{
					mcu->nvic->regs.IP[ addr-0x300 ] = val;
				}
			}
			break;
	}
}

/*
 * uint32_t word reading
 */
void stm_nvic_read( struct mcu_t* mcu, uint32_t addr, uint32_t* val, void* param )
{
	switch ( addr ) {
		case 0x00:
			*val = mcu->nvic->regs.ISER[ 0 ];
			break;

		case 0x80:
			*val = mcu->nvic->regs.ICER[ 0 ];
			break;

		case 0x100:
			*val = mcu->nvic->regs.ISPR[ 0 ];
			break;

		case 0x180:
			*val = mcu->nvic->regs.ICPR[ 0 ];
			break;

		default:
			{
				if(( 0x300 <= addr )
						&&( addr <( 0x300+8 ) ))
				{
					*val = mcu->nvic->regs.IP[ addr-0x300 ];
				}
			}
			break;
	}
}

/*
 *
 */
void stm_nvic_dump_state( void* pnvic )
{
//	stm_nvic_t * nvic =( stm_nvic_t* ) pnvic;
//	stm_t * stm = nvic->mcu->core;
//	if( stm )
//	{
//		NVIC_TRACE( stm, LOG_TRACE, "\n\nFLASH Circuit Dumping\n ..." );
////	NVIC_TRACE( stm, LOG_TRACE, "\ncycle[%lu] run_cycle_count[%lu]", stm->cycle, stm->run_cycle_count );
////	NVIC_TRACE( stm, LOG_TRACE, "\nASPR[N.Z.C.V]=[%01d.%01d.%01d.%01d]", stm->APSR.bits.N, stm->APSR.bits.Z, stm->APSR.bits.C, stm->APSR.bits.V );
////
////	for (uint32_t var = 0; var < 13; ++var)
////	{
////		NVIC_TRACE( stm, LOG_TRACE, "\nR[%d]=%08x", var, stm->R[var] );
////	}
////	NVIC_TRACE( stm, LOG_TRACE, "\nSP,MSP,PSP=[%08x,%08x,%08x]", stm->R[ 13 ], stm->msp, stm->psp );
////	NVIC_TRACE( stm, LOG_TRACE, "\nR[14](LR)=%08x", stm->R[ 14 ]);
////	NVIC_TRACE( stm, LOG_TRACE, "\nR[15](PC)=[%08x]", stm->pc );
////
////	NVIC_TRACE( stm, LOG_TRACE, "\nTicks[%I64u]", stm->nano_ticks );
//	}
}

/*
 *
 */
stm_nvic_t* nvic_circuit_fabric( struct mcu_t * mcu )
{
	stm_nvic_t* p = malloc( sizeof( stm_nvic_t ));

	if( p )
	{
	  memset( p, 0x00, sizeof( stm_nvic_t ));

		p->cir_cb_ifs = malloc( sizeof( stm_circuit_ifs_t ));
		if( p->cir_cb_ifs )
		{
		  memset( p->cir_cb_ifs, 0x00, sizeof( stm_circuit_ifs_t ));
			p->cir_cb_ifs->init					= stm_nvic_init;
			p->cir_cb_ifs->reset				= stm_nvic_reset;
			p->cir_cb_ifs->release			= stm_nvic_release;
			p->cir_cb_ifs->wr_cb_reg		= stm_nvic_wr_cb_reg;
			p->cir_cb_ifs->rd_cb_reg		= stm_nvic_rd_cb_reg;
			p->cir_cb_ifs->write				= stm_nvic_write;
			p->cir_cb_ifs->read					= stm_nvic_read;
			p->cir_cb_ifs->dump					= stm_nvic_dump_state;
		}
	}

	p->mcu = mcu;
	return p;
}


void nvic_regist_interrupt( struct mcu_t * mcu, irq_number_t interrupt )
{
	switch (interrupt) {
		case SysTick_IRQN:
			mcu->nvic->pending_interrupts_ext |= ( 1UL << SysTick_Pending_Ext );
//			uint64_t uiMS = mcu->core->nano_ticks/1000000;
//		  NVIC_TRACE( mcu->core, LOG_TRACE, "\nNVIC SysTick_IRQ time[%lu] ms\n", uiMS );
		  NVIC_TRACE( mcu->core, LOG_TRACE, "\nNVIC SysTick_IRQ time[%lu] ms\n", ( mcu->core->nano_ticks/1000000 ));
		  break;

		default:

			NVIC_TRACE( mcu->core, LOG_ERROR, "\nNVIC Unknown interrupt[%d] regist\n", interrupt );
			break;
	}
}


static stm_flashaddr_t nvic_get_ihandler_address( struct mcu_t * mcu, irq_number_t inum )
{
  stm_flashaddr_t ret_val = 0x00;

  switch (inum)
  {
    case SysTick_IRQN:
      {
        uint8_t* p = ( uint8_t* )mcu->core->flash +0x0000003C;
        ret_val = p[ 3 ]<<24 | p[ 2 ]<<16 | p[ 1 ]<<8 | p[ 0 ];
        return ret_val;
      }
    default:
      mcu->core->state = cpu_Crashed;
      break;
  }

  return ret_val;
}


static uint32_t nvic_get_interrupt_for_handling( struct mcu_t * mcu )
{
//  uint32_t pending_interrupt = mcu->nvic->pending_interrupts_ext;

  if( mcu->nvic->pending_interrupts_ext )
    return SysTick_IRQN;

  return 0x00;
}

static void nvic_clear_pending_flag( struct mcu_t * mcu, irq_number_t inum )
{
  mcu->nvic->pending_interrupts_ext = 0x00;
}


static void nvic_save_prio( struct mcu_t * mcu, irq_number_t inum )
{
//   mcu->nvic->interrupr_prio
}

/**
  \brief   function handles registered interrupt and switches the core into interrupt context
  \details ...
  \param [in]      mcu  Pointer to MCU representation object
  \return          0x00 No interrupts to handle
  \return          Other Interrupt was handled
  \note    ...
 */
void nvic_handle_pending_interrupts( struct mcu_t * mcu )
{
  // TODO: Sleeping should be also added
  if( mcu->core->state == cpu_Running )
  {
    if(( mcu->nvic->pending_interrupts_ext )
      ||( mcu->nvic->pending_interrupts ))
    {
      // interrupt is occurred
      if( interrupt_context_push( mcu->core ))
      {
        // Set core to Crashed state
        mcu->core->state = cpu_Crashed;
        return;
      }
      mcu->core->state = cpu_IntPending;
    }
  }
  else if( mcu->core->state == cpu_IntPending  )
  {
    // find actual interrupt for handling
    irq_number_t interrupt = nvic_get_interrupt_for_handling( mcu );

    // Testing
    interrupt = SysTick_IRQN;

    // get address for handler related to interrupt
    stm_flashaddr_t  ih_address = nvic_get_ihandler_address( mcu, interrupt );
    mcu->core->pc = ih_address;

    NVIC_TRACE( mcu->core, LOG_TRACE, "\nNVIC ADR->PC[0x%08X]\n", ih_address );

    // Clear Pending flag
    nvic_clear_pending_flag( mcu, interrupt );

    // TODO: Save handling interrupt priority
    nvic_save_prio( mcu, interrupt );

    mcu->core->state = cpu_Running;
  }
  else if( mcu->core->state == cpu_IntLeaving )
  {
    if(( mcu->nvic->pending_interrupts_ext )
      ||( mcu->nvic->pending_interrupts ))
    {
       mcu->core->state = cpu_IntPending;
    }else{
       // Restore context
       interrupt_context_pop( mcu->core );
       mcu->core->state = cpu_IntLeave;
    }
  }
  else if( mcu->core->state == cpu_IntLeave )
  {
      if(( mcu->nvic->pending_interrupts_ext )
        ||( mcu->nvic->pending_interrupts ))
        mcu->core->state = cpu_IntPending;
      else
        mcu->core->state = cpu_Running;
  }
}

/**
  \brief   implements the logic of interrupt leaving process
  \details ...
  \param [in]      mcu  Pointer to MCU representation object
  \return          0x00 No interrupts to handle
  \return          Other Interrupt was handled
  \note    ...
 */
void nvic_run_interrupt_leaving( struct mcu_t * mcu, uint32_t leaving_code )
{
  mcu->nvic->iret_val = leaving_code;
  mcu->core->state = cpu_IntLeaving;
}
