/*
 * sim_stm.c
 *
 *  Created on: 6 февр. 2019 г.
 *      Author: dell
 */
#include <unistd.h>  //Для getwd
#include <limits.h>  //Для PATH_MAX

#include <sys/types.h>
#include <sys/stat.h>
#include <libgen.h>

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "sim_inc.h"

FILE* flog = NULL;

char flog_name[] = "d:\\tmp\\1\\stm.log";


static void std_logger( stm_t * stm, const int level, const char * format, 		va_list ap);
static stm_logger_p _stm_global_logger = std_logger;

void stm_global_logger( struct stm_t* stm, const int level, const char * format, ... )
{
	va_list args;
	va_start(args, format);
	if (_stm_global_logger)
		_stm_global_logger(stm, level, format, args);
	va_end(args);
}

void stm_global_logger_set( stm_logger_p logger )
{
	_stm_global_logger = logger ? logger : std_logger;
}

stm_logger_p stm_global_logger_get( void )
{
	return _stm_global_logger;
}

/* */
void stm_loadcode( mcu_t * mcu, uint8_t * code, uint32_t size, stm_flashaddr_t address)
{
  if( mcu->core )
  {
    if(( mcu->core->flash_start_adr +address +size )>= mcu->core->flash_end_adr )
    {
      STM_LOG( mcu->core, LOG_ERROR, "stm_loadcode(): Attempted to load code of size %d but flash size is only %d.\n", mcu->core->flash_end_adr, mcu->core->flash_end_adr + 1);
      abort();
    }

    memcpy( mcu->core->flash + address, code, size);
  }
}

/**
 * Accumulates sleep requests (and returns a sleep time of 0) until
 * a minimum count of requested sleep microseconds are reached
 * (low amounts cannot be handled accurately).
 */
/*
uint32_t
avr_pending_sleep_usec(
		stm_t * avr,
		avr_cycle_count_t howLong)
{
	avr->sleep_usec += avr_cycles_to_usec(avr, howLong);
	uint32_t usec = avr->sleep_usec;
	if (usec > 200) {
		avr->sleep_usec = 0;
		return usec;
	}
	return 0;
}
*/

/*
void
avr_callback_sleep_gdb(
		stm_t * avr,
		avr_cycle_count_t howLong)
{
	uint32_t usec = avr_pending_sleep_usec(avr, howLong);
	while (avr_gdb_processor(avr, usec))
		;
}
*/

/*
void
avr_callback_run_gdb(
		stm_t * avr)
{
	avr_gdb_processor(avr, avr->state == cpu_Stopped);

	if (avr->state == cpu_Stopped)
		return ;

	// if we are stepping one instruction, we "run" for one..
	int step = avr->state == cpu_Step;
	if (step)
		avr->state = cpu_Running;

	stm_flashaddr_t new_pc = avr->pc;

	if (avr->state == cpu_Running) {
		new_pc = avr_run_one(avr);
#if CONFIG_SIMAVR_TRACE
		avr_dump_state(avr);
#endif
	}

	// run the cycle timers, get the suggested sleep time
	// until the next timer is due
	avr_cycle_count_t sleep = avr_cycle_timer_process(avr);

	avr->pc = new_pc;

	if (avr->state == cpu_Sleeping) {
		if (!avr->sreg[S_I]) {
			if (avr->log)
				STM_LOG(avr, LOG_TRACE, "simavr: sleeping with interrupts off, quitting gracefully\n");
			avr->state = cpu_Done;
			return;
		}

		 * try to sleep for as long as we can (?)

		avr->sleep(avr, sleep);
		avr->cycle += 1 + sleep;
	}
	// Interrupt servicing might change the PC too, during 'sleep'
	if (avr->state == cpu_Running || avr->state == cpu_Sleeping)
		avr_service_interrupts(avr);

	// if we were stepping, use this state to inform remote gdb
	if (step)
		avr->state = cpu_StepDone;

}
*/

/*
To avoid simulated time and wall clock time to diverge over time
this function tries to keep them in sync (roughly) by sleeping
for the time required to match the expected sleep deadline
in wall clock time.
*/
/*
void
avr_callback_sleep_raw(
		stm_t *avr,
		avr_cycle_count_t how_long)
{
	 figure out how long we should wait to match the sleep deadline
	uint64_t deadline_ns = avr_cycles_to_nsec(avr, avr->cycle + how_long);
	uint64_t runtime_ns = avr_get_time_stamp(avr);
	if (runtime_ns >= deadline_ns)
		return;
	uint64_t sleep_us = (deadline_ns - runtime_ns) / 1000;
	usleep(sleep_us);
	return;
}
*/

//void
//avr_callback_run_raw(
//		stm_t * avr)
//{
//	stm_flashaddr_t new_pc = avr->pc;
//
//	if (avr->state == cpu_Running) {
//		new_pc = avr_run_one(avr);
//#if CONFIG_SIMAVR_TRACE
//		avr_dump_state(avr);
//#endif
//	}
//
//	// run the cycle timers, get the suggested sleep time
//	// until the next timer is due
//	avr_cycle_count_t sleep = avr_cycle_timer_process(avr);
//
//	avr->pc = new_pc;
//
//	if (avr->state == cpu_Sleeping) {
//		if (!avr->sreg[S_I]) {
//			if (avr->log)
//				STM_LOG(avr, LOG_TRACE, "simavr: sleeping with interrupts off, quitting gracefully\n");
//			avr->state = cpu_Done;
//			return;
//		}
//
//		/*
//		 * try to sleep for as long as we can (?)
//		 */
//		avr->sleep(avr, sleep);
//		avr->cycle += 1 + sleep;
//	}
//	// Interrupt servicing might change the PC too, during 'sleep'
//	if (avr->state == cpu_Running || avr->state == cpu_Sleeping) {
//		/* Note: checking interrupt_state here is completely superfluous, however
//			as interrupt_state tells us all we really need to know, here
//			a simple check here may be cheaper than a call not needed. */
//		if (avr->interrupt_state)
//			avr_service_interrupts(avr);
//	}
//}


int stm_run( stm_t * stm )
{
	stm->run( stm );
	return stm->state;
}

stm_t * stm_core_allocate( const stm_t * core, uint32_t coreLen )
{
	uint8_t * b = malloc(coreLen);
	memcpy(b, core, coreLen);
	return (stm_t *)b;
}

mcu_t * stm_make_mcu_by_name( const char *name )
{
//	stm_kind_t * maker = NULL;
//	for (int i = 0; avr_kind[i] && !maker; i++) {
//		for (int j = 0; avr_kind[i]->names[j]; j++)
//			if (!strcmp(avr_kind[i]->names[j], name)) {
//				maker = avr_kind[i];
//				break;
//			}
//	}
//	if (!maker) {
//		STM_LOG(((stm_t*)0), LOG_ERROR, "%s: AVR '%s' not known\n", __FUNCTION__, name);
//		return NULL;
//	}

	// stm_t * stm = maker->make();
	//	STM_LOG(avr, LOG_TRACE, "Starting %s - flashend %04x ramend %04x e2end %04x\n",
//			avr->mmcu, avr->flashend, avr->ramend, avr->e2end);

//	avr_core_allocate(&mcu_32l031k6t6.core, sizeof(struct mcu_t));


	mcu_t * mcu  = malloc(sizeof(struct mcu_t));
	memcpy(mcu, &SIM_CORENAME.core, sizeof(struct mcu_t));

//	uint8_t * b = malloc(coreLen);
//	memcpy(b, &mcu_32l031k6t6.core, sizeof(struct mcu_t));
//	return (avr_t *)b;

	return mcu;
}
/*
 *
 */
static void std_logger(
		stm_t * stm,
		const int level,
		const char * format,
		va_list ap)
{
	if (!stm || stm->log >= level)
	{
	    char pBuff[1000];

 	    vfprintf((level > LOG_ERROR) ?  stdout : stderr , format, ap);

		if( !flog )
			init_log( flog_name );

		if( flog )
		{
//  	    fprintf( flog,"cly[[%I64u] p[%p] format[%s]\n", stm->cycle, &format, format );
//        vfprintf( flog, format, ap);
//		    sprintf( pBuff, "cly[[%I64u] p[%p] format[%s]\n", stm->cycle, &format, format );
//        fputs( pBuff, flog );
//		    sprintf( pBuff, format, ap );
		    vsprintf( pBuff, format, ap );
		    fputs( pBuff, flog );
		}
	}
}

uint32_t init_log( char* file_log_name )
{
  char cwd[ PATH_MAX+1 ] = "";

  if( !flog )
  {
    if( file_log_name == NULL )
    {
      if (NULL == getcwd(cwd, sizeof(cwd))){
        return 0;
      }else{
        strncat( cwd, "\\logs", PATH_MAX );

        struct stat st = {0};

        if( stat(cwd, &st )== -1 )
            mkdir( cwd );

        strncat( cwd, "\\stm.log", PATH_MAX );

        flog = fopen( cwd, "w" );

        return 1;
      }
    }else{
      char* bname;
      strcpy( cwd, file_log_name );
      bname = basename( cwd );

      if( strcmp( bname, file_log_name )== 0 )
      {
        if (NULL == getcwd(cwd, sizeof(cwd)))
          return 0;
        else
        {
          strncat( cwd, "\\logs", PATH_MAX );

          struct stat st = {0};

          if( stat(cwd, &st )== -1 )
              mkdir( cwd );

          strncat( cwd, "\\", PATH_MAX );
          strncat( cwd, file_log_name, PATH_MAX );

          flog = fopen( cwd, "w" );
        }
      }else{
        strcpy( cwd, file_log_name );

        char* dir = dirname( cwd );

        struct stat st = {0};

        if( stat(dir, &st )== -1 )
            mkdir( dir );

        flog = fopen( file_log_name, "w" );
      }
    }
  }
  return 0x1;
}

/*
 *
 */
void close_log( void )
{
  if( flog )
  	fclose( flog );
}

//void close_log_file( FILE* f )
//{
//  if( f )
//    fclose( f );
//}



/*
 * MCU initialization part
 */

static uint32_t stm_mcu_init( struct mcu_t * mcu )
{
  char pMMCU[] = "stm32l031";

	// RCC init
	mcu->rcc = rcc_circuit_fabric( mcu );
	if( mcu->rcc )
		mcu->rcc->cir_cb_ifs->init( mcu );

  // Core init
	stm_t* stm_i = stm_core_fabric( pMMCU );
	if( !stm_i )
	{
  	fprintf(stderr, "%s: STM '%s' not known\n", "stmsim", pMMCU );
  	return 0x00;
	}

	stm_i->mcu = mcu; // set owner mcu link
	mcu->core = stm_i;
	stm_i->circuit_ifs->init( mcu );

	// flash init
	mcu->flash = flash_circuit_fabric( mcu );
	if( mcu->flash )
		mcu->flash->cir_cb_ifs->init( mcu );

	// SCB
	mcu->scb = scb_circuit_fabric( mcu );
	if( mcu->scb )
		mcu->scb->cir_cb_ifs->init( mcu );

	// nvic init
	mcu->nvic = nvic_circuit_fabric( mcu );
	if( mcu->nvic )
		mcu->nvic->cir_cb_ifs->init( mcu );

	// systick init
	mcu->systick = sysctick_circuit_fabric( mcu );
	if( mcu->systick )
		mcu->systick->cir_cb_ifs->init( mcu );

	// gpio
	mcu->portB = gpio_circuit_fabric( mcu );

	mcu->portB->cir_cb_ifs->init( mcu );

	// Address region binding for read/write operations

	// portB
	stm_core_rw_handler_reg( mcu->core, mcu->portB->cir_cb_ifs, 0x50000400, sizeof( stm_gpio_port_regs_t ));

	// flash
	stm_core_rw_handler_reg( mcu->core, mcu->flash->cir_cb_ifs, 0x40022000, 0X1000 );

	// rcc
	stm_core_rw_handler_reg( mcu->core, mcu->rcc->cir_cb_ifs, 0X40021000, 0X400 );

	// nvic
	stm_core_rw_handler_reg( mcu->core,mcu->nvic->cir_cb_ifs, NVIC_BASE, sizeof(NVIC_Type ));

	// systicks
	stm_core_rw_handler_reg( mcu->core,mcu->systick->cir_cb_ifs, SysTick_BASE, sizeof(SysTick_Type ));

	// sbc
	stm_core_rw_handler_reg(mcu->core, mcu->scb->cir_cb_ifs, SCB_BASE, sizeof( SCB_Type ));

	return 0x01;
}


static void stm_mcu_reset( struct mcu_t * mcu, uint32_t type )
{
	// Reset core part
	if( mcu->core )
		mcu->core->circuit_ifs->reset( mcu, type );

	if( mcu->scb )
		mcu->scb->cir_cb_ifs->reset( mcu, type );

	if( mcu->flash )
	  mcu->flash->cir_cb_ifs->reset( mcu, type );

	if( mcu->nvic )
		mcu->nvic->cir_cb_ifs->reset( mcu, type );

	if( mcu->systick )
		mcu->systick->cir_cb_ifs->reset( mcu, type );

	if( mcu->portB )
	  mcu->portB->cir_cb_ifs->reset( mcu, type );

	if( mcu->rcc )
		mcu->rcc->cir_cb_ifs->reset( mcu, RESET_POWER_PWR );
}

static void stm_mcu_dump_state( void* circuit )
{
	mcu_t* mcu =( mcu_t* )circuit;

	if( mcu )
	{
		stm_t* stm = mcu->core;

		STM_LOG( stm, LOG_TRACE, "\n\nMCU Dumping----------------" );

		if( mcu->core->circuit_ifs->dump )
			mcu->core->circuit_ifs->dump( stm );

		if(mcu->rcc->cir_cb_ifs->dump)
			mcu->rcc->cir_cb_ifs->dump( mcu->rcc );

		if( mcu->flash->cir_cb_ifs->dump )
			mcu->flash->cir_cb_ifs->dump( mcu->flash );

		if( mcu->scb->cir_cb_ifs->dump )
			mcu->scb->cir_cb_ifs->dump( mcu->scb );

		if( mcu->portB->cir_cb_ifs->dump)
			mcu->portB->cir_cb_ifs->dump( mcu->portB );

		if( mcu->systick->cir_cb_ifs->dump )
			mcu->systick->cir_cb_ifs->dump( mcu->systick );

		if( mcu->nvic )
		  mcu->nvic->cir_cb_ifs->dump( mcu->nvic );
	}
}

static void stm_mcu_release( struct mcu_t * mcu, void* obj )
{
	if( mcu )
	{
	  // release rcc
	  if( mcu->rcc )
	    mcu->rcc->cir_cb_ifs->release( mcu, mcu->rcc );

		// release  core
	  if( mcu->core )
	    mcu->core->circuit_ifs->release( mcu, mcu->core );

		//release scb
		if( mcu->scb )
		  mcu->scb->cir_cb_ifs->release( mcu, mcu->scb );

		// release flash
		if( mcu->flash )
		  mcu->flash->cir_cb_ifs->release( mcu, mcu->flash );

		// release systick
		if( mcu->systick )
		  mcu->systick->cir_cb_ifs->release( mcu, mcu->systick );

		// release nvic
		if( mcu->nvic )
		  mcu->nvic->cir_cb_ifs->release( mcu, mcu->nvic );

		if( mcu->portB )
		  mcu->portB->cir_cb_ifs->release( mcu, mcu->portB );

	  if( mcu->mcu_ifs )
	    free( mcu->mcu_ifs );

	  mcu->mcu_ifs = NULL;

		free( mcu );
	}
	else
	{
		// TODO: Add warnings & comment for MCU null
	}
}

mcu_t* stm_mcu_circuit_fabric( const char *name )
{
	if( !name ) return NULL;

	mcu_t* p = malloc( sizeof( mcu_t ));
	if( p )
	{
	  // Assign all to null
	  memset( p, 0x00, sizeof( mcu_t ) );

		p->mcu_ifs = malloc( sizeof( stm_circuit_ifs_t ));

		memset( p->mcu_ifs, 0x00, sizeof( stm_circuit_ifs_t ));

		p->mcu_ifs->init				= stm_mcu_init;
		p->mcu_ifs->reset				= stm_mcu_reset;
		p->mcu_ifs->dump				= stm_mcu_dump_state;
		p->mcu_ifs->release			= stm_mcu_release;
	}
	return p;
}

/**
 *
 */
void stm_callback_run_raw( mcu_t * mcu )
{
	// perform an instruction
	if (mcu->core->state == cpu_Running)
		stm_run_one(mcu->core);

	// run the cycle timers, get the suggested sleep time
	// until the next timer is due
	stm_cycle_timer_process( mcu->core );

	// handle pending interrupts
	nvic_handle_pending_interrupts( mcu );

//	if( nvic_handle_interrupts( mcu ))
//	{
//		// handle cycle timer again
//		stm_cycle_timer_process( mcu->core );
//
//		// handle possible interrupt
//		nvic_handle_interrupts( mcu );
//	}

//	mcu->core->pc = new_pc;

	if (mcu->core->state == cpu_Sleeping)
	{
//		if (!avr->sreg[S_I]) {
//			if (avr->log)
//				AVR_LOG(avr, LOG_TRACE, "simavr: sleeping with interrupts off, quitting gracefully\n");
//			avr->state = cpu_Done;
//			return;
//		}
//		/*
//		 * try to sleep for as long as we can (?)
//		 */
//		avr->sleep(avr, sleep);
//		avr->cycle += 1 + sleep;
	}

//	// Interrupt servicing might change the PC too, during 'sleep'
//	if (avr->state == cpu_Running || avr->state == cpu_Sleeping)
//	{
//		/* Note: checking interrupt_state here is completely superfluous, however
//			as interrupt_state tells us all we really need to know, here
//			a simple check here may be cheaper than a call not needed. */
//		if (stm->interrupt_state)
//			stm_service_interrupts(stm);
//	}
}




