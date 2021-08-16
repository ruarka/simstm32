/*! \file      stm_rcc.c
    \version   0.1
    \date      2019 05-25 17:54
    \brief
    \author    ruarka

	\bug

	\details
*/
/* ------------------------------------------------------------------------------------------------
 *                                 Includes
 * ------------------------------------------------------------------------------------------------
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "sim_inc.h"

/* ------------------------------------------------------------------------------------------------
 *                                 Type definitions
 * ------------------------------------------------------------------------------------------------
 */
/* ------------------------------------------------------------------------------------------------
 *                                     Macros
 * ------------------------------------------------------------------------------------------------
 */
#define RCC_REG_UNDEF					0xa5a5a5a5

#define PERIPH_BASE           (0x40000000UL) /*!< Peripheral base address in the alias region */
#define AHBPERIPH_BASE        (PERIPH_BASE + 0x00020000UL)
#define RCC_BASE              (AHBPERIPH_BASE + 0x00001000UL)

/* ------------------------------------------------------------------------------------------------
 *                                 Global Variables
 * ------------------------------------------------------------------------------------------------
 */
/*
 * "Pretty" register names
 */
const char * reg_names[255] =
{
		// RCC
		[ RCC_ICSCR ]			= "ICSCR",
		[ RCC_CFGR ]			= "CFGR",
		[ RCC_CIER ]			= "CIER",
		[ RCC_CIFR ]			= "CIFR",
		[ RCC_CICR ]			= "CICR",
		[ RCC_IOPRSTR ]		= "IOPRSTR",
		[ RCC_AHBRSTR ]		= "AHBRSTR",
		[ RCC_APB2RSTR ]	= "APB2RSTR",
		[ RCC_APB1RSTR ]	= "APB1RSTR",
		[ RCC_IOPENR ]		= "IOPENR",
		[ RCC_AHBENR ]		= "AHBENR",
		[ RCC_APB2ENR ]		= "APB2ENR",
		[ RCC_APB1ENR ]		= "APB1ENR",
		[ RCC_IOPSMENR ]	= "IOPSMENR",
		[ RCC_AHBSMENR ]	= "AHBSMENR",
		[ RCC_APB2SMENR ]	= "APB2SMENR",
		[ RCC_APB1SMENR ]	= "APB1SMENR",
		[ RCC_CCIPR ]			= "CCIPR",
		[ RCC_CSR ]				= "CSR",
};


/* ------------------------------------------------------------------------------------------------
 *                                 Local Variables
 * ------------------------------------------------------------------------------------------------
 */


/* ------------------------------------------------------------------------------------------------
 *                                 Local Functions
 * ------------------------------------------------------------------------------------------------
 */

void stm_rcc_recalculate_clock( void );
void stm_rcc_clock_update( mcu_t* mcu,
													 uint32_t clkid,
													 uint64_t ticks_per_nsec,
													 uint64_t frequency);


const char * stm_regname(uint8_t reg)
{
	if (!reg_names[reg])
	{
		char tt[16];
		sprintf(tt, "io:%02x", reg);
		reg_names[reg] = strdup(tt);
	}
	return reg_names[reg];
}

static void read_pool_callbacks( cb_read_cell_t* cb_rd_pool, struct mcu_t* mcu, uint32_t addr, uint32_t val )
{
	uint32_t rcc_evt = addr;
	uint32_t* param;

	while ( cb_rd_pool )
	{
		param = cb_rd_pool->param;
		cb_rd_pool->cb( mcu, param, rcc_evt, val );
		cb_rd_pool = cb_rd_pool->next;
	}
}

static void write_pool_callbacks( cb_write_cell_t* cb_wr_pool, struct mcu_t* mcu, uint32_t addr, uint32_t val )
{
	uint32_t rcc_evt = addr;
	uint32_t* param;

	while ( cb_wr_pool )
	{
		param = (uint32_t*) cb_wr_pool->param;
		cb_wr_pool->cb( mcu, param, rcc_evt, val );
		cb_wr_pool = cb_wr_pool->next;
	}
}

void stm_rcc_read( struct mcu_t* mcu, uint32_t addr, uint32_t* val, void* param )
{
// 	uint32_t rcc_addr =( addr - RCC_BASE )& 0xfffffffc;
//	uint32_t rcc_addr = addr & 0x3ff;
	uint32_t rcc_idx  = (( addr & 0x3ff )>>2);

	if( rcc_idx < sizeof( RCC_TypeDef )/4)
	{
		*val = mcu->rcc->rcc_regs[ rcc_idx ].val;
		read_pool_callbacks(
				mcu->rcc->rcc_regs[ rcc_idx ].cbs.cb_read_pool,
				mcu,
				addr,
				*val );
	}
	else
	{
		// TODO - throw rcc access exception
	}
}

void stm_rcc_write( struct mcu_t* mcu, uint32_t addr, uint32_t val, void* param )
{
//	uint32_t rcc_addr = addr & 0x3ff;
	uint32_t rcc_idx  = (( addr & 0x3ff )>>2);

	if( rcc_idx < sizeof( RCC_TypeDef )/4)
	{
		mcu->rcc->rcc_regs[ rcc_idx ].val = val;
		write_pool_callbacks(
				mcu->rcc->rcc_regs[ rcc_idx ].cbs.cb_write_pool,
				mcu,
				addr,
				val );
	}
	else
	{
		// TODO - throw rcc access exception
	}

	// Check if launching clock value update is needed
	stm_rcc_recalculate_clock();
}

//static stm_cycle_count_t stm_progen_clear(struct stm_t * stm, stm_cycle_count_t when, void * param)
//{
//	stm_flash_t * p = (stm_flash_t *)param;
//	stm_regbit_clear(p->io.stm, p->selfprgen);
//	STM_LOG( stm, LOG_WARNING, "FLASH: stm_progen_clear - SPM not received, clearing PRGEN bit\n");
//	return 0;
//}

void stm_rcc_rd_cb_reg( struct mcu_t * mcu, void* cb_rd, uint32_t addr, void* param )
{
//	cb_read_cell_t* rd_pool = stm->rcc.rcc_regs[reg].cbs.cb_read_pool;
//
//	if( !rd_pool )
//	{
//		stm->rcc.rcc_regs->cbs.cb_read_pool 				= malloc( sizeof( cb_read_cell_t ));
//		stm->rcc.rcc_regs->cbs.cb_read_pool->next 	= NULL;
//		stm->rcc.rcc_regs->cbs.cb_read_pool->cb 		= cb_rd;
//		stm->rcc.rcc_regs->cbs.cb_read_pool->param 	= param;
//		return;
//	}
//
//	while( !rd_pool->next )
//		rd_pool = rd_pool->next;
//
//	rd_pool = malloc( sizeof( cb_read_cell_t ));
//	rd_pool->next = NULL;
//	rd_pool->cb = cb_rd;
//	rd_pool->param = param;
}

void stm_rcc_wr_cb_reg( struct mcu_t * mcu, void* cb_wr, uint32_t addr, void* param )
{
//	cb_write_cell_t* wr_pool = stm->rcc.rcc_regs[reg].cbs.cb_write_pool;
//
//	if( !wr_pool )
//	{
//		stm->rcc.rcc_regs->cbs.cb_write_pool 				= malloc( sizeof( cb_write_cell_t ));
//		stm->rcc.rcc_regs->cbs.cb_write_pool->next 	= NULL;
//		stm->rcc.rcc_regs->cbs.cb_write_pool->cb 		= cb_wr;
//		stm->rcc.rcc_regs->cbs.cb_write_pool->param = param;
//		return;
//	}
//
//	while( !wr_pool->next )
//		wr_pool = wr_pool->next;
//
//	wr_pool 				= malloc( sizeof( cb_write_cell_t ));
//	wr_pool->next 	= NULL;
//	wr_pool->cb 		= cb_wr;
//	wr_pool->param 	= param;
}

void stm_rcc_updt_cb_reg( struct mcu_t * mcu, cb_rcc_onupdate_t cb_updt, void* param )
{
	cb_onupdate_cell_t* updt_pool = mcu->rcc->rcc_update_pool;

	if( !updt_pool )
	{
		mcu->rcc->rcc_update_pool 				= malloc( sizeof( cb_onupdate_cell_t ));
		mcu->rcc->rcc_update_pool->cb 		= cb_updt;
		mcu->rcc->rcc_update_pool->next 	= NULL;
		mcu->rcc->rcc_update_pool->param  = param;
		return;
	}

	while( updt_pool->next )
		updt_pool = updt_pool->next;

	updt_pool->next 	      = malloc( sizeof( cb_onupdate_cell_t ));
	updt_pool->next->next 	= NULL;
	updt_pool->next->cb 		= cb_updt;
	updt_pool->param 	= param;
}

static uint32_t stm_rcc_init( mcu_t * mcu )
{
	// LSI initiation
	mcu->rcc->lsi.frequency 			= 0;
	mcu->rcc->lsi.nano_per_tick 	= 0;
	mcu->rcc->lsi.pretty_name 		= "LSI";

	// LSE initiation
	mcu->rcc->lse.frequency 			= 0;
	mcu->rcc->lse.nano_per_tick 	= 0;
	mcu->rcc->lse.pretty_name 		= "LSE";

	// MSI initiation
	mcu->rcc->msi.frequency 			= 2100000;		// MHz
	mcu->rcc->msi.nano_per_tick 	= 476;				// nano ticks per second
	mcu->rcc->msi.pretty_name 		= "MSI";

	// HSI16 initiation
	mcu->rcc->hsi16.frequency 			= 0;
	mcu->rcc->hsi16.nano_per_tick 	= 0;
	mcu->rcc->hsi16.pretty_name 		= "HSI16";

	// HSE initiation
	mcu->rcc->hse.frequency 				= 0;
	mcu->rcc->hse.nano_per_tick 		= 0;
	mcu->rcc->hse.pretty_name 			= "HSE";


	for (uint32_t i = 0; i < sizeof( RCC_TypeDef )/sizeof(uint32_t); ++i)
	{
		mcu->rcc->rcc_regs[ i ].cbs.cb_read_pool 						= NULL;
		mcu->rcc->rcc_regs[ i ].cbs.cb_write_pool 					= NULL;
	}

	mcu->rcc->rcc_update_pool = NULL;

	// Power on reset value
	mcu->rcc->rcc_regs[ RCC_CR_idx ].val 				= 0x00000300;
	mcu->rcc->rcc_regs[ RCC_ICSCR_idx ].val 		= 0x0000b000;
	mcu->rcc->rcc_regs[ RCC_CRRCR_idx ].val 		= 0x00;
	mcu->rcc->rcc_regs[ RCC_CFGR_idx ].val 			= 0x00;
	mcu->rcc->rcc_regs[ RCC_CIER_idx ].val 			= RCC_REG_UNDEF;
	mcu->rcc->rcc_regs[ RCC_CIFR_idx ].val 			= RCC_REG_UNDEF;
	mcu->rcc->rcc_regs[ RCC_CICR_idx ].val 			= RCC_REG_UNDEF;
	mcu->rcc->rcc_regs[ RCC_IOPRSTR_idx ].val 	= RCC_REG_UNDEF;
	mcu->rcc->rcc_regs[ RCC_AHBRSTR_idx ].val 	= RCC_REG_UNDEF;
	mcu->rcc->rcc_regs[ RCC_APB2RSTR_idx ].val 	= RCC_REG_UNDEF;
	mcu->rcc->rcc_regs[ RCC_APB1RSTR_idx ].val 	= RCC_REG_UNDEF;
	mcu->rcc->rcc_regs[ RCC_IOPENR_idx ].val 		= RCC_REG_UNDEF;
	mcu->rcc->rcc_regs[ RCC_AHBENR_idx ].val 		= RCC_REG_UNDEF;
	mcu->rcc->rcc_regs[ RCC_APB2ENR_idx ].val 	= RCC_REG_UNDEF;
	mcu->rcc->rcc_regs[ RCC_APB1ENR_idx ].val 	= RCC_REG_UNDEF;
	mcu->rcc->rcc_regs[ RCC_IOPSMENR_idx ].val 	= RCC_REG_UNDEF;
	mcu->rcc->rcc_regs[ RCC_AHBSMENR_idx ].val 	= RCC_REG_UNDEF;
	mcu->rcc->rcc_regs[ RCC_APB2SMENR_idx ].val = RCC_REG_UNDEF;
	mcu->rcc->rcc_regs[ RCC_APB1SMENR_idx ].val = RCC_REG_UNDEF;
	mcu->rcc->rcc_regs[ RCC_CCIPR_idx ].val 		= RCC_REG_UNDEF;
	mcu->rcc->rcc_regs[ RCC_CSR_idx ].val 			= 0x0c000004;

	// Frequency setting
	mcu->rcc->freq_val = 0xffffffff; // 2.1 Mhz

	return 0x01;
}

// System Reset value for RCC block
void stm_rcc_reset(mcu_t * mcu, uint32_t type )
{
	// Common part of reset
	mcu->rcc->rcc_regs[ RCC_CR_idx ].val 				= 0x00000300;
	mcu->rcc->rcc_regs[ RCC_ICSCR_idx ].val 		= 0x0000b000;
	mcu->rcc->rcc_regs[ RCC_CRRCR_idx ].val 		= 0x00;
	mcu->rcc->rcc_regs[ RCC_CFGR_idx ].val 			= 0x00;
	mcu->rcc->rcc_regs[ RCC_CIER_idx ].val 			= 0x00;
	mcu->rcc->rcc_regs[ RCC_CIFR_idx ].val 			= 0x00;
	mcu->rcc->rcc_regs[ RCC_CICR_idx ].val 			= 0x00;
	mcu->rcc->rcc_regs[ RCC_IOPRSTR_idx ].val 	= 0x00;
	mcu->rcc->rcc_regs[ RCC_AHBRSTR_idx ].val 	= 0x00;
	mcu->rcc->rcc_regs[ RCC_APB2RSTR_idx ].val 	= 0x00;
	mcu->rcc->rcc_regs[ RCC_APB1RSTR_idx ].val 	= 0x00;
	mcu->rcc->rcc_regs[ RCC_IOPENR_idx ].val 		= 0x00;
	mcu->rcc->rcc_regs[ RCC_AHBENR_idx ].val 		= 0x00000100;
	mcu->rcc->rcc_regs[ RCC_APB2ENR_idx ].val 	= 0x00;
	mcu->rcc->rcc_regs[ RCC_APB1ENR_idx ].val 	= 0x00;

//  TODO: Value defined by hw
//	mcu->rcc.rcc_regs[ RCC_IOPSMENR ].val = ;
//  TODO: Value defined by hw
//	mcu->rcc.rcc_regs[ RCC_AHBSMENR ].val = ;
//  TODO: Value defined by hw
//	mcu->rcc.rcc_regs[ RCC_APB2SMENR ].val = ;
//  TODO: Value defined by hw
//	mcu->rcc.rcc_regs[ RCC_APB1SMENR ].val = ;

	mcu->rcc->rcc_regs[ RCC_CCIPR_idx ].val = 0x00;

	mcu->rcc->rcc_regs[ RCC_CSR_idx ].val = 0x0c000004;

	switch (type)
	{
		case RESET_POWER_PWR:
		{
			/* It is not really needed to wait for
			 * SYSCLK freq stabilization because of POWER RESET
			 * and all starts from clean */
			// MSI initiation - again
//			mcu->rcc->msi.frequency 			= 2100000;		// MHz
//			mcu->rcc->msi.nano_per_tick 	= 476;				// nano ticks per second
//			mcu->rcc->msi.pretty_name 		= "MSI";

			mcu->rcc->SYSCLK.frequency 		= mcu->rcc->msi.frequency;
			mcu->rcc->SYSCLK.nano_per_tick = mcu->rcc->msi.nano_per_tick;
			mcu->rcc->SYSCLK.enable  		  = 0x01;


			stm_rcc_clock_update( mcu,
					SYSCLK_UPDT,
					mcu->rcc->msi.nano_per_tick,
					mcu->rcc->msi.frequency	);
		}
		break;

		default:
			break;
	}
}
/*
 * Checks and updates clock values for clock consumers
 * for new registers values if it is needed
 */
void stm_rcc_recalculate_clock( void )
{

}
/*
 *
 */
void stm_rcc_clock_update( mcu_t* mcu,
													 uint32_t clkid,
													 uint64_t ticks_per_nsec,
													 uint64_t frequency)
{
	/* callbacks call */
	cb_onupdate_cell_t* p = mcu->rcc->rcc_update_pool;
	while( p != NULL )
	{
		cb_rcc_onupdate_t fn = p->cb;
		fn( mcu, SYSCLK_UPDT, ticks_per_nsec, frequency );

		p = p->next;
	}
}
/*
 *
 */
void stm_rcc_release( mcu_t * mcu, void* obj )
{
  if( !mcu->rcc )
  return;

  // release callback objects
	for (uint32_t i = 0; i < sizeof( RCC_TypeDef )/sizeof(uint32_t); ++i)
	{
		cb_read_cell_t* cb_read_pool = mcu->rcc->rcc_regs[ i ].cbs.cb_read_pool;
		while( cb_read_pool != NULL )
		{
			cb_read_cell_t* tmp = cb_read_pool;
			cb_read_pool = cb_read_pool->next;
			free( tmp );
		}

		mcu->rcc->rcc_regs[ i ].cbs.cb_read_pool = NULL;


		cb_write_cell_t* cb_write_pool = mcu->rcc->rcc_regs[ i ].cbs.cb_write_pool;
		while( cb_write_pool != NULL )
		{
			cb_write_cell_t* tmp = cb_write_pool;
			cb_write_pool = cb_write_pool->next;
			free( tmp );
		}

		mcu->rcc->rcc_regs[ i ].cbs.cb_write_pool = NULL;
	}

	cb_onupdate_cell_t* cb_onupdate_pool = mcu->rcc->rcc_update_pool;
	while( cb_onupdate_pool != NULL )
	{
		cb_onupdate_cell_t* tmp = cb_onupdate_pool;
		cb_onupdate_pool = cb_onupdate_pool->next;
		free( tmp );
	}

	mcu->rcc->rcc_update_pool = NULL;


  // release interface circuit object
  if( mcu->rcc->cir_cb_ifs )
  {
      free( mcu->rcc->cir_cb_ifs );
      mcu->rcc->cir_cb_ifs = NULL;
  }

	// release rcc object
	free( mcu->rcc );
	mcu->rcc = NULL;
}

void stm_rcc_dump_state( void* prcc )
{
	stm_rcc_t * rcc =( stm_rcc_t* ) prcc;
	stm_t * stm = rcc->mcu->core ;
	if( stm )
	{
		STM_LOG( stm, LOG_TRACE, "\n\nRCC Circuit Dumping\n ..." );
//	STM_LOG( stm, LOG_TRACE, "\ncycle[%lu] run_cycle_count[%lu]", stm->cycle, stm->run_cycle_count );
//	STM_LOG( stm, LOG_TRACE, "\nASPR[N.Z.C.V]=[%01d.%01d.%01d.%01d]", stm->APSR.bits.N, stm->APSR.bits.Z, stm->APSR.bits.C, stm->APSR.bits.V );
//
//	for (uint32_t var = 0; var < 13; ++var)
//	{
//		STM_LOG( stm, LOG_TRACE, "\nR[%d]=%08x", var, stm->R[var] );
//	}
//	STM_LOG( stm, LOG_TRACE, "\nSP,MSP,PSP=[%08x,%08x,%08x]", stm->R[ 13 ], stm->msp, stm->psp );
//	STM_LOG( stm, LOG_TRACE, "\nR[14](LR)=%08x", stm->R[ 14 ]);
//	STM_LOG( stm, LOG_TRACE, "\nR[15](PC)=[%08x]", stm->pc );
//
//	STM_LOG( stm, LOG_TRACE, "\nTicks[%I64u]", stm->nano_ticks );
	}
}
/*
 *
 */
stm_rcc_t* rcc_circuit_fabric( mcu_t * mcu )
{
	if( !mcu ) return NULL;

	stm_rcc_t* p = malloc( sizeof( stm_rcc_t ));
	if( p )
	{
		p->cir_cb_ifs = malloc( sizeof( stm_circuit_ifs_t ));
		if( p->cir_cb_ifs )
		{
			p->cir_cb_ifs->init					= stm_rcc_init;
			p->cir_cb_ifs->reset				= stm_rcc_reset;
			p->cir_cb_ifs->release			= stm_rcc_release;
//			p->cir_cb_ifs->updt_cb_reg 	= stm_rcc_updt_cb_reg;
			p->cir_cb_ifs->wr_cb_reg		= stm_rcc_wr_cb_reg;
			p->cir_cb_ifs->rd_cb_reg		= stm_rcc_rd_cb_reg;
			p->cir_cb_ifs->write				= stm_rcc_write;
			p->cir_cb_ifs->read					= stm_rcc_read;
			p->cir_cb_ifs->dump					= stm_rcc_dump_state;
		}

		p->mcu = mcu;
	}

	return p;
}

