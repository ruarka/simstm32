/*! \file      sim_scb.c
    \version   0.0
    \date      27 θών. 2020 γ. 21:37:54
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
//void scb_rcc_onupdate_cb(struct mcu_t * mcu, uint32_t param, uint64_t ticks_per_nsec, uint64_t frequency)
//{
//	mcu->scb->nano_ticks_per_cycle = ticks_per_nsec;
//	mcu->scb->frequency 					 = frequency;
//}

/**
  \brief   ...
  \details ...
  \param [in]      mcu  Pointer to MCU representation object
  \return             0  Was successfully initialized
  \return             Other Error
  \note    IRQn must not be negative.
 */
/*
 *
 */
static uint32_t stm_scb_init( mcu_t * mcu )
{
	stm_scb_t* p = mcu->scb;

	if( p )
	{
		p->scb_regs->CPUID 	= 0x410CC601;
		p->scb_regs->ICSR 	= 0x00000000;
		p->scb_regs->VTOR 	= 0x00000000;
		p->scb_regs->AIRCR	= 0xFA050000;
		p->scb_regs->SCR		= 0x00000000;
		p->scb_regs->CCR		= 0x00000204;
		p->scb_regs->SHP[0] = 0x00000000;
		p->scb_regs->SHP[1] = 0x00000000;
		p->scb_regs->SHCSR  = 0x00000000;
	}

	// bind core to rcc
//	stm_rcc_updt_cb_reg( mcu, scb_rcc_onupdate_cb, NULL );

	return 0x00;
}

/*
 *
 */
void stm_scb_reset(mcu_t * mcu, uint32_t type )
{
	stm_scb_t* p = mcu->scb;

	if( p )
	{
		p->scb_regs->CPUID 	= 0x410CC601;
		p->scb_regs->ICSR 	= 0x00000000;
		p->scb_regs->VTOR 	= 0x00000000;
		p->scb_regs->AIRCR 	= 0xFA050000;
		p->scb_regs->SCR		= 0x00000000;
		p->scb_regs->CCR		= 0x00000204;
		p->scb_regs->SHP[0] = 0x00000000;
		p->scb_regs->SHP[1] = 0x00000000;
		p->scb_regs->SHCSR  = 0x00000000;
	}
}

/*
 *
 */
void stm_scb_release( mcu_t * mcu, void* obj )
{
	if( mcu )
	{
	  if( mcu->scb )
	  {
      if( mcu->scb->scb_regs )
          free( mcu->scb->scb_regs );
      mcu->scb->scb_regs = NULL;

      if( mcu->scb->cir_cb_ifs )
        free( mcu->scb->cir_cb_ifs );
      mcu->scb->cir_cb_ifs = NULL;

      free( mcu->scb );
      mcu->scb = NULL;
	  }
	}
}

/*
 *
 */
void stm_scb_wr_cb_reg( struct mcu_t * mcu, void* cb_wr, uint32_t addr, void* param )
{
}

/*
 *
 */
void stm_scb_rd_cb_reg( struct mcu_t * mcu, void* cb_rd, uint32_t addr, void* param )
{
}

/*
 * uint32_t word writing
 */
void stm_scb_write( struct mcu_t* mcu, uint32_t addr, uint32_t val, void* param )
{
	switch( addr>>2 )
	{
		case 0x01:
			mcu->scb->scb_regs->ICSR = val;
			break;

		case 0x02:
			mcu->scb->scb_regs->VTOR = val;
			break;

		case 0x03:
			mcu->scb->scb_regs->AIRCR = val;
			break;

		case 0x04:
			mcu->scb->scb_regs->SCR = val;
			break;

		case 0x05:
			mcu->scb->scb_regs->CCR = val;
			break;

		case 0x06:
			mcu->scb->scb_regs->RESERVED1 = val;
			break;

		case 0x07:
			mcu->scb->scb_regs->SHP[0] = val;
			break;

		case 0x08:
			mcu->scb->scb_regs->SHP[1] = val;
			break;

		case 0x09:
			mcu->scb->scb_regs->SHCSR = val;
			break;

		default:
			break;
	}
}

/*
 * uint32_t word reading
 */
void stm_scb_read( struct mcu_t* mcu, uint32_t addr, uint32_t* val, void* param )
{
	switch( addr>>2 )
	{
		case 0x00:
			*val = mcu->scb->scb_regs->CPUID;
			break;

		case 0x01:
			*val = mcu->scb->scb_regs->ICSR;
			break;

		case 0x02:
			*val = mcu->scb->scb_regs->VTOR;
			break;

		case 0x03:
			*val = mcu->scb->scb_regs->AIRCR;
			break;

		case 0x04:
			*val = mcu->scb->scb_regs->SCR;
			break;

		case 0x05:
			*val = mcu->scb->scb_regs->CCR;
			break;

		case 0x06:
			*val = mcu->scb->scb_regs->RESERVED1;
			break;

		case 0x07:
			*val = mcu->scb->scb_regs->SHP[0];
			break;

		case 0x08:
			*val = mcu->scb->scb_regs->SHP[1];
			break;

		case 0x09:
			*val = mcu->scb->scb_regs->SHCSR;
			break;

		default:
			break;
	}
}

/*
 *
 */
void stm_scb_dump_state( void* pscb )
{
	stm_scb_t * scb =( stm_scb_t* ) pscb;
	stm_t * stm = scb->mcu->core;
	if( stm )
	{
		STM_LOG( stm, LOG_TRACE, "\n\nSCB Circuit Dumping\n--------------" );

		// LOG CPUID
  	STM_LOG( stm, LOG_TRACE, "\nCPUID[%08x] IMPL[%02x]", scb->scb_regs->CPUID, (scb->scb_regs->CPUID)>>24 );
  	STM_LOG( stm, LOG_TRACE, " VAR[%01x]", ((scb->scb_regs->CPUID)>>20)& 0xf );
  	STM_LOG( stm, LOG_TRACE, " ARCH[%01x]", ((scb->scb_regs->CPUID)>>16)& 0xf );
  	STM_LOG( stm, LOG_TRACE, " PART[%03x]", ((scb->scb_regs->CPUID)>>4)& 0xfff );
  	STM_LOG( stm, LOG_TRACE, " REV[%01x]", (scb->scb_regs->CPUID)& 0xf );


//  	STM_LOG( stm, LOG_TRACE, "\nASPR[N.Z.C.V]=[%01d.%01d.%01d.%01d]", stm->APSR.bits.N, stm->APSR.bits.Z, stm->APSR.bits.C, stm->APSR.bits.V );

  	// LOG ICSR
  	STM_LOG( stm, LOG_TRACE, "\nICSR[%08x] ", scb->scb_regs->ICSR );
  	STM_LOG( stm, LOG_TRACE, " NDSET[%01x]", ((scb->scb_regs->CPUID)>>20)& 0xf );
//  	STM_LOG( stm, LOG_TRACE, " ARCH[%01x]", ((scb->scb_regs->CPUID)>>16)& 0xf );
//
  	////	STM_LOG( stm, LOG_TRACE, "\nASPR[N.Z.C.V]=[%01d.%01d.%01d.%01d]", stm->APSR.bits.N, stm->APSR.bits.Z, stm->APSR.bits.C, stm->APSR.bits.V );

  	// LOG
  	STM_LOG( stm, LOG_TRACE, "\nVTOR[%08x] ", scb->scb_regs->VTOR );

  	// LOG
  	STM_LOG( stm, LOG_TRACE, "\nAIRCR[%08x] ", scb->scb_regs->AIRCR );

  	// LOG
  	STM_LOG( stm, LOG_TRACE, "\nSCR[%08x] ", scb->scb_regs->SCR );

  	// LOG
  	STM_LOG( stm, LOG_TRACE, "\nCCR[%08x] ", scb->scb_regs->CCR );

  	// LOG
  	STM_LOG( stm, LOG_TRACE, "\nRESERVED1[%08x] ", scb->scb_regs->RESERVED1 );

  	// LOG
  	STM_LOG( stm, LOG_TRACE, "\nSHP[0][%08x] ", scb->scb_regs->SHP[0] );

  	// LOG
  	STM_LOG( stm, LOG_TRACE, "\nSHP[1][%08x] ", scb->scb_regs->SHP[1] );

		// LOG
  	STM_LOG( stm, LOG_TRACE, "\nSHCSR[%08x] ", scb->scb_regs->SHCSR );

	}
}

/*
 *
 */
stm_scb_t* scb_circuit_fabric( struct mcu_t * mcu )
{
	if( !mcu ) return 0x00;

	stm_scb_t* p = malloc( sizeof( stm_scb_t ));
	if( p )
	{
	  memset( p, 0x00, sizeof( stm_scb_t ));

		p->cir_cb_ifs = malloc( sizeof( stm_circuit_ifs_t ));
		memset( p->cir_cb_ifs, 0x00, sizeof( stm_circuit_ifs_t ));

		if( p->cir_cb_ifs )
		{
			p->cir_cb_ifs->init					= stm_scb_init;
			p->cir_cb_ifs->reset				= stm_scb_reset;
			p->cir_cb_ifs->release			= stm_scb_release;
			p->cir_cb_ifs->wr_cb_reg		= stm_scb_wr_cb_reg;
			p->cir_cb_ifs->rd_cb_reg		= stm_scb_rd_cb_reg;
			p->cir_cb_ifs->write				= stm_scb_write;
			p->cir_cb_ifs->read					= stm_scb_read;
			p->cir_cb_ifs->dump					= stm_scb_dump_state;
		}

    p->scb_regs         = malloc( sizeof( SCB_Type ));
    memset( p->scb_regs, 0x00, sizeof( SCB_Type ) ) ;
	}

	p->mcu = mcu;
	return p;
}
