/*
	stm_flash.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "stdint.h"

#include "sim_inc.h"

//static stm_cycle_count_t stm_progen_clear(struct stm_t * stm, stm_cycle_count_t when, void * param)
//{
//	stm_flash_t * p = (stm_flash_t *)param;
//	stm_regbit_clear(p->io.stm, p->selfprgen);
//	STM_LOG( stm, LOG_WARNING, "FLASH: stm_progen_clear - SPM not received, clearing PRGEN bit\n");
//	return 0;
//}


//static void stm_flash_write(stm_t * stm, stm_io_addr_t addr, uint8_t v, void * param)
//{
//	stm_flash_t * p = (stm_flash_t *)param;
//
//	stm_core_watch_write(avr, addr, v);
//
////	printf("** stm_flash_write %02x\n", v);
//
//	if (stm_regbit_get(avr, p->selfprgen))
//		stm_cycle_timer_register(avr, 4, stm_progen_clear, p); // 4 cycles is very little!
//}

//static void stm_flash_clear_temppage(stm_flash_t *p)
//{
//	for (int i = 0; i < p->spm_pagesize / 2; i++) {
//		p->tmppage[i] = 0xff;
//		p->tmppage_used[i] = 0;
//	}
//}

//static int stm_flash_ioctl(struct stm_io_t * port, uint32_t ctl, void * io_param)
//{
//	if (ctl != STM_IOCTL_FLASH_SPM)
//		return -1;

//	stm_flash_t * p = (stm_flash_t *)port;
//	stm_t * stm = p->io.stm;
//
//	stm_flashaddr_t z = stm->data[R_ZL] | (stm->data[R_ZH] << 8);
//	if (stm->rampz)
//		z |= stm->data[stm->rampz] << 16;
//	uint16_t r01 = stm->data[0] | (stm->data[1] << 8);
//
////	printf("AVR_IOCTL_FLASH_SPM %02x Z:%04x R01:%04x\n", avr->data[p->r_spm], z,r01);
//	if (stm_regbit_get(stm, p->selfprgen)) {
//		stm_cycle_timer_cancel(stm, stm_progen_clear, p);
//
//		if (stm_regbit_get(stm, p->pgers)) {
//			z &= ~1;
//			STM_LOG(stm, LOG_TRACE, "FLASH: Erasing page %04x (%d)\n", (z / p->spm_pagesize), p->spm_pagesize);
//			for (int i = 0; i < p->spm_pagesize; i++)
//				stm->flash[z++] = 0xff;
//		} else if (stm_regbit_get(stm, p->pgwrt)) {
//			z &= ~(p->spm_pagesize - 1);
//			STM_LOG(stm, LOG_TRACE, "FLASH: Writing page %04x (%d)\n", (z / p->spm_pagesize), p->spm_pagesize);
//			for (int i = 0; i < p->spm_pagesize / 2; i++) {
//				stm->flash[z++] = p->tmppage[i];
//				stm->flash[z++] = p->tmppage[i] >> 8;
//			}
//			stm_flash_clear_temppage(p);
//		} else if (stm_regbit_get(stm, p->blbset)) {
//			STM_LOG(stm, LOG_TRACE, "FLASH: Setting lock bits (ignored)\n");
//		} else if (p->flags & STM_SELFPROG_HAVE_RWW && stm_regbit_get(stm, p->rwwsre)) {
//			stm_flash_clear_temppage(p);
//		} else {
//			STM_LOG(stm, LOG_TRACE, "FLASH: Writing temppage %08x (%04x)\n", z, r01);
//			z >>= 1;
//			if (!p->tmppage_used[z % (p->spm_pagesize / 2)]) {
//				p->tmppage[z % (p->spm_pagesize / 2)] = r01;
//				p->tmppage_used[z % (p->spm_pagesize / 2)] = 1;
//			}
//		}
//	}
//	stm_regbit_clear(stm, p->selfprgen);
//	return 0;
//}

//static void stm_flash_reset( stm_io_t * port)
//{
//	stm_flash_t * p = (stm_flash_t *) port;
//
//	stm_flash_clear_temppage(p);
//}

//static void stm_flash_dealloc(struct stm_io_t * port)
//{
//	stm_flash_t * p = (stm_flash_t *) port;
//
//	if (p->tmppage)
//		free(p->tmppage);
//
//	if (p->tmppage_used)
//		free(p->tmppage_used);
//}

//static	stm_io_t	_io =
//{
//	.kind = "flash",
//	.ioctl = stm_flash_ioctl,
//	.reset = stm_flash_reset,
//	.dealloc = stm_flash_dealloc,
//};

// void stm_flash_init(stm_t * stm, stm_flash_t * p)
// {
//	p->io = _io;
//	printf("%s init SPM %04x\n", __FUNCTION__, p->r_spm);

//	if (!p->tmppage)
//		p->tmppage = malloc(p->spm_pagesize);
//
//	if (!p->tmppage_used)
//		p->tmppage_used = malloc(p->spm_pagesize / 2);
//
//	stm_register_io(stm, &p->io);
//	stm_register_vector(stm, &p->flash);
//
//	stm_register_io_write(stm, p->r_spm, stm_flash_write, p);
//}

/*
 *
 */
static uint32_t stm_flash_init( mcu_t * mcu )
{
	stm_flash_t* p = mcu->flash;

	if( p )
	{
		p->flash_regs->ACR 			= 0;
		p->flash_regs->PECR 		= 7;
		p->flash_regs->PDKEYR 	= 0;
		p->flash_regs->PEKEYR 	= 0;
		p->flash_regs->PRGKEYR 	= 0;
		p->flash_regs->OPTKEYR 	= 0;
		p->flash_regs->SR 			= 0x0c;
		p->flash_regs->OPTR 		= 0;
//		p->flash_regs->WRPR1 		= 0;
//		p->flash_regs->WRPR2 		= 0;
	}

	return 0x00;
}

/*
 *
 */
void stm_flash_reset(mcu_t * mcu, uint32_t type )
{
	stm_flash_t* p = mcu->flash;

	if( p )
	{
		p->flash_regs->ACR 			= 0;
		p->flash_regs->PECR 		= 7;
		p->flash_regs->PDKEYR 	= 0;
		p->flash_regs->PEKEYR 	= 0;
		p->flash_regs->PRGKEYR 	= 0;
		p->flash_regs->OPTKEYR 	= 0;
		p->flash_regs->SR 			= 0x0c;
		p->flash_regs->OPTR 		= 0;
//		p->flash_regs->WRPR1 		= 0;
//		p->flash_regs->WRPR2 		= 0;
	}
}

/*
 *
 */
void stm_flash_release( mcu_t * mcu, void* obj )
{
	if( mcu )
	{
		if( mcu->flash->cir_cb_ifs )
			free( mcu->flash->cir_cb_ifs );
		mcu->flash->cir_cb_ifs = NULL;

		if( mcu->flash->flash_regs )
			free( mcu->flash->flash_regs);
		mcu->flash->flash_regs = NULL;

		if( mcu->flash )
			free( mcu->flash );
		mcu->flash = NULL;
	}
}

/*
 *
 */
void stm_flash_wr_cb_reg( struct mcu_t * mcu, void* cb_wr, uint32_t addr, void* param )
{
}

/*
 *
 */
void stm_flash_rd_cb_reg( struct mcu_t * mcu, void* cb_rd, uint32_t addr, void* param )
{
}

/*
 * uint32_t word writing
 */
void stm_flash_write( struct mcu_t* mcu, uint32_t addr, uint32_t val, void* param )
{
//	switch(addr-0x40022000)
	switch(addr)
	{
		case 0x00:
			mcu->flash->flash_regs->ACR = val;
			break;
		case 0x04:
			mcu->flash->flash_regs->PECR = val;
			break;
		case 0x08:
			mcu->flash->flash_regs->PDKEYR = val;
			break;
		case 0x0c:
			mcu->flash->flash_regs->PEKEYR = val;
			break;
		case 0x10:
			mcu->flash->flash_regs->PRGKEYR = val;
			break;
		case 0x14:
			mcu->flash->flash_regs->OPTKEYR = val;
			break;
		case 0x18:
			mcu->flash->flash_regs->SR = val;
			break;
		case 0x1c:
			mcu->flash->flash_regs->OPTR = val;
			break;
		case 0x20:
//			mcu->flash->flash_regs->WRPR1 = val;
			break;
		case 0x80:
//			mcu->flash->flash_regs->WRPR = val;
			break;

		default:
			break;
	}
}

/*
 * uint32_t word reading
 */
void stm_flash_read( struct mcu_t* mcu, uint32_t addr, uint32_t* val, void* param )
{
//	switch(addr-0x40022000)
	switch(addr)
	{
		case 0x00:
			*val = mcu->flash->flash_regs->ACR;
			break;
		case 0x04:
			*val = mcu->flash->flash_regs->PECR;
			break;
		case 0x08:
			*val = mcu->flash->flash_regs->PDKEYR;
			break;
		case 0x0c:
			*val = mcu->flash->flash_regs->PEKEYR;
			break;
		case 0x10:
			*val = mcu->flash->flash_regs->PRGKEYR;
			break;
		case 0x14:
			*val = mcu->flash->flash_regs->OPTKEYR;
			break;
		case 0x18:
			*val = mcu->flash->flash_regs->SR;
			break;
		case 0x1c:
			*val = mcu->flash->flash_regs->OPTR;
			break;
		case 0x20:
//			*val = mcu->flash->flash_regs->WRPR1;
			break;
		case 0x80:
//			*val = mcu->flash->flash_regs->WRPR2;
			break;

		default:
			*val = 0x00;
			break;
	}
}

/*
 *
 */
void stm_flash_dump_state( void* pflash )
{
	stm_flash_t * flash =( stm_flash_t* ) pflash;
	stm_t * stm = flash->mcu->core ;
	if( stm )
	{
		STM_LOG( stm, LOG_TRACE, "\n\nFLASH Circuit Dumping\n ..." );
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
stm_flash_t* flash_circuit_fabric( struct mcu_t * mcu )
{
	if( !mcu )
	  return 0x00;

	stm_flash_t* p = malloc( sizeof( stm_flash_t ));
	if( p )
	{
	  memset( p, 0x00, sizeof( stm_flash_t ));

		p->cir_cb_ifs = malloc( sizeof( stm_circuit_ifs_t ));
		if( p->cir_cb_ifs )
		{
		  memset( p->cir_cb_ifs, 0x00, sizeof( stm_circuit_ifs_t ));
			p->cir_cb_ifs->init					= stm_flash_init;
			p->cir_cb_ifs->reset				= stm_flash_reset;
			p->cir_cb_ifs->release			= stm_flash_release;
			p->cir_cb_ifs->wr_cb_reg		= stm_flash_wr_cb_reg;
			p->cir_cb_ifs->rd_cb_reg		= stm_flash_rd_cb_reg;
			p->cir_cb_ifs->write				= stm_flash_write;
			p->cir_cb_ifs->read					= stm_flash_read;
			p->cir_cb_ifs->dump					= stm_flash_dump_state;
		}
	}

	p->flash_regs = malloc( sizeof( FLASH_TypeDef ));
	if( p->flash_regs )
	  memset( p->flash_regs, 0x00, sizeof( FLASH_TypeDef ));

	p->mcu = mcu;
	return p;
}


