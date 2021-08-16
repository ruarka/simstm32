/*
	sim_core.c
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "sim_inc.h"

/**
 * Breakpoints part should be moved into own module
 */

stm_flashaddr_t bp_instruction_access[]=
{
//    0x080001D1,
//    0x08000F5B,
		0x00,
};

uint32_t bp_address_access[]=
{
//		0xe000ed08, // SCB->VTOR
//  0x40022000,		// FLASH
		0x00,
};

// stm_cycle_count_t bp_cycle = 0xffffffff;
stm_cycle_count_t bp_cycle = 1176;


uint32_t check_instruction_bp( stm_flashaddr_t addr )
{
//	uint32_t i = 0;
//
//	while( bp_instruction_access[ i ]!= 0x00 )
//		if( bp_instruction_access[ i++ ] == addr )
//			return 0x01;

	return 0x00;
}



uint32_t check_address_bp( uint32_t addr )
{
//	uint32_t i = 0;

//	while( bp_address_access[ i ]!= 0x00 )
//		if( bp_address_access[ i++ ] == addr )
//			return 0x01;

	return 0x00;
}

uint32_t check_cycle_bp( stm_cycle_count_t  stm_cycle )
{
//  if ( stm_cycle == bp_cycle )
//    return 0x01;

  return 0x00;
}

uint32_t stm_addr_read( struct stm_t * stm, uint32_t addr, uint32_t* val )
{
	if( check_address_bp( addr ))
	{
		stm->state = cpu_BpAddress;
		return 0x00;
	}

	switch (addr & 0xf0000000)
	{
	// flash code
	case 0:
	case 0x10000000:
		if( addr >= 0x08000000 )
		{
			uint32_t given_addr = addr & 0x00ffffff;
			if( given_addr < stm->flash_end_adr )
			{
				*val = *( uint32_t* )(&stm->flash[given_addr]);
				return 0x00;
			}
		}

		break;

	// SRAM
	case 0x20000000:
	if( addr < stm->ramend )
	{
		uint32_t* p = (uint32_t*)( stm->data +( addr -0x20000000 ));
		*val =  *p;
	}
	else
	{
		// throw exception - wrong sram address
		STM_LOG( stm, LOG_ERROR, "SRAM[0x%08x] out off size\n", addr );
//		stm->dump(stm);
		stm->circuit_ifs->dump( stm );
		exit(1);
	}

	break;

	default:
		{
			hw_device_address_cell_t* dp = stm->devs_pool;

			while( dp )
			{
				if(( addr >= dp->start_address )
						&&( addr < dp->end_address ))
				{
					stm_circuit_ifs_t* ci = dp->cir_cb_ifs;
					ci->read(stm->mcu, addr-dp->start_address, val, 0);
					return 0x00;
				}
				dp = dp->next;
			}
		}
		// TODO throw exception for access fault
		break;
	}
	return 0x0;
}

uint32_t stm_addr_readh( struct stm_t * stm, uint32_t addr, uint16_t* val )
{
    if( check_address_bp( addr ))
    {
      stm->state = cpu_BpAddress;
      return 0x00;
    }

    switch (addr & 0xf0000000)
    {
    // flash code
    case 0:
    case 0x10000000:
      if( addr >= 0x08000000 )
      {
        uint32_t given_addr = addr & 0x00ffffff;
        if( given_addr < stm->flash_end_adr )
        {
          *val = stm->flash[given_addr];
          return 0x00;
        }
      }

      break;
    // SRAM
    case 0x20000000:
      if( addr < stm->ramend )
      {
        uint16_t* p = (uint16_t*)( stm->data +( addr -0x20000000 ));
        *val =  *p;
      }
      else
      {
        // throw exception - wrong sram address
        STM_LOG( stm, LOG_ERROR, "SRAM[0x%08x] out off size\n", addr );
        // stm->dump(stm);
        stm->circuit_ifs->dump( stm );
        exit(1);
      }
      break;

    default:
  //    {
  //      hw_device_address_cell_t* dp = stm->devs_pool;
  //
  //      while( dp )
  //      {
  //        if(( addr >= dp->start_address )
  //            &&( addr < dp->end_address ))
  //        {
  //          stm_circuit_ifs_t* ci = dp->cir_cb_ifs;
  //          ci->write(stm->mcu, addr-dp->start_address, val, 0);
  //          return 0x00;
  //        }
  //        dp = dp->next;
  //      }
  //    }
      // TODO throw exception for access fault
      break;
    }

  return 0x00;
}

uint32_t stm_addr_readb( struct stm_t * stm, uint32_t addr, uint8_t* val )
{
		if( check_address_bp( addr ))
		{
			stm->state = cpu_BpAddress;
			return 0x00;
		}

		switch (addr & 0xf0000000)
		{
		// flash code
		case 0:
		case 0x10000000:
			if( addr >= 0x08000000 )
			{
				uint32_t given_addr = addr & 0x00ffffff;
				if( given_addr < stm->flash_end_adr )
				{
					*val = stm->flash[given_addr];
					return 0x00;
				}
			}

			break;
		// SRAM
		case 0x20000000:
			if( addr < stm->ramend )
			{
				*val = stm->data[ addr -0x20000000 ];
			}
			else
			{
				// throw exception - wrong sram address
				STM_LOG( stm, LOG_ERROR, "SRAM[0x%08x] out off size\n", addr );
				// stm->dump(stm);
				stm->circuit_ifs->dump( stm );
				exit(1);
			}
			break;

		default:
	//		{
	//  		hw_device_address_cell_t* dp = stm->devs_pool;
	//
	//			while( dp )
	//			{
	//				if(( addr >= dp->start_address )
	//						&&( addr < dp->end_address ))
	//				{
	//					stm_circuit_ifs_t* ci = dp->cir_cb_ifs;
	//					ci->write(stm->mcu, addr-dp->start_address, val, 0);
	//					return 0x00;
	//				}
	//				dp = dp->next;
	//			}
	//		}
			// TODO throw exception for access fault
			break;
		}

	return 0x00;
}
/*
 *
 */
uint32_t stm_addr_write( struct stm_t * stm, uint32_t addr, uint32_t val )
{
	if( check_address_bp( addr ))
	{
		stm->state = cpu_BpAddress;
		return 0x00;
	}

	switch (addr & 0xf0000000)
	{
	// flash code
	case 00:

		break;

	// SRAM
	case 0x20000000:
		if( addr < stm->ramend )
		{
			uint32_t* p = (uint32_t*)( stm->data +( addr -0x20000000 ));
			*p = val;
		}
		else
		{
			// throw exception - wrong sram address
			STM_LOG( stm, LOG_ERROR, "SRAM[0x%08x] out off size\n", addr );
			// stm->dump(stm);
			stm->circuit_ifs->dump( stm );
			exit(1);
		}
		break;

	default:
		{
  		hw_device_address_cell_t* dp = stm->devs_pool;

			while( dp )
			{
				if(( addr >= dp->start_address )
						&&( addr < dp->end_address ))
				{
					stm_circuit_ifs_t* ci = dp->cir_cb_ifs;
					ci->write(stm->mcu, addr-dp->start_address, val, 0);
					return 0x00;
				}
				dp = dp->next;
			}
		}
		// TODO throw exception for access fault
		break;
	}

	return 0x0;
}

/*
 *
 */
uint32_t stm_addr_writeh( struct stm_t * stm, uint32_t addr, uint16_t val )
{
  if( check_address_bp( addr ))
  {
    stm->state = cpu_BpAddress;
    return 0x00;
  }

  switch (addr & 0xf0000000)
  {
  // flash code
  // TODO: flash code write
  case 00:
    break;

  // SRAM
  case 0x20000000:
    if( addr < stm->ramend )
    {
      uint16_t* p = (uint16_t*)( stm->data +( addr -0x20000000 ));
      *p = val;
    }
    else
    {
      // throw exception - wrong sram address
      STM_LOG( stm, LOG_ERROR, "SRAM[0x%08x] out off size\n", addr );
      stm->circuit_ifs->dump( stm );
      exit(1);
    }
    break;

  default:
//    {
//      hw_device_address_cell_t* dp = stm->devs_pool;
//
//      while( dp )
//      {
//        if(( addr >= dp->start_address )
//            &&( addr < dp->end_address ))
//        {
//          stm_circuit_ifs_t* ci = dp->cir_cb_ifs;
//          ci->write(stm->mcu, addr-dp->start_address, val, 0);
//          return 0x00;
//        }
//        dp = dp->next;
//      }
//    }
    // TODO throw exception for access fault
    break;
  }

  return 0x00;
}

/*
 *
 */
uint32_t stm_addr_writeb( struct stm_t * stm, uint32_t addr, uint8_t val )
{
	if( check_address_bp( addr ))
	{
		stm->state = cpu_BpAddress;
		return 0x00;
	}

	switch (addr & 0xf0000000)
	{
	// flash code
	case 00:
		break;

	// SRAM
	case 0x20000000:
		if( addr < stm->ramend ){
			stm->data[ addr -0x20000000 ]= val;
		}else{
			// throw exception - wrong sram address
			STM_LOG( stm, LOG_ERROR, "SRAM[0x%08x] out off size\n", addr );
			stm->circuit_ifs->dump( stm );
			exit(1);
		}
		break;

	default:
//		{
//  		hw_device_address_cell_t* dp = stm->devs_pool;
//
//			while( dp )
//			{
//				if(( addr >= dp->start_address )
//						&&( addr < dp->end_address ))
//				{
//					stm_circuit_ifs_t* ci = dp->cir_cb_ifs;
//					ci->write(stm->mcu, addr-dp->start_address, val, 0);
//					return 0x00;
//				}
//				dp = dp->next;
//			}
//		}
		// TODO throw exception for access fault
		break;
	}

	return 0x00;
}

/*
 *
 */
void stm_core_rw_handler_reg( stm_t *stm, stm_circuit_ifs_t* cifs, uint32_t address, uint32_t size  )
{
	hw_device_address_cell_t* dp = stm->devs_pool;
	if( !dp )
	{
		dp 								= malloc( sizeof( hw_device_address_cell_t ));
		dp->next 					= NULL;
		dp->start_address	= address;
		dp->end_address		= address+size-1;
		dp->cir_cb_ifs		= cifs;
		stm->devs_pool 		= dp;
		return;
	}

	while( dp->next )
		dp = dp->next;

	dp->next 	      			= malloc( sizeof( hw_device_address_cell_t ));
	dp->next->next 				= NULL;
	dp->next->cir_cb_ifs 	= cifs;
	dp->next->end_address 	= address +size-1;
	dp->next->start_address	= address;
}

/****************************************************************************\
 *
 * Helper functions for calculating the status register bit values.
 * See the ST data sheet for the instruction set for more info.
 *
\****************************************************************************/

/*
 * Main opcode decoder
 */
#if 0
uint32_t stm_run_one(stm_t * stm)
{
	// check breakpoint for flash address
	if( check_instruction_bp( stm->pc ))
	{
		stm->state = cpu_BpFlash;
		return 0x00;
	}

	/* Ensure we don't crash simstm due to a bad instruction reading past
	 * the end of the flash.
	 */
	if (unlikely(stm->pc >= stm->flash_end_adr))
	{
		STM_LOG( stm, LOG_TRACE, "\n\nCRASH PC[%08x]", stm->pc );
		stm->state = cpu_Crashed;
//		crash(stm);
		return 0;
	}

	// cpu cycles update
	stm->cycle++;

	// nano ticks update
	stm->nano_ticks += stm->nano_ticks_per_cycle;

	uint16_t		opcode = _stm_flash_read16le( stm, stm->pc);

	stm_flashaddr_t ( *pInstFunc )(struct stm_t * stm, uint16_t opcode ) = instruction_matrix[ opcode ];

	stm_flashaddr_t pc_new;
	if( pInstFunc )
	{
		pc_new = pInstFunc( stm, opcode );
		stm->pc = pc_new;

//		// stack spoiling trup
//		uint32_t val;
//		stm_addr_read( stm, 0x20000418, &val );
//		STM_LOG( stm, LOG_WARNING, "!!!0x20000418[%08X]\n", val );
	}
	else
	{
		// TODO - unknown code
		STM_LOG( stm, LOG_ERROR, "Unknown ins[%04x] at flash[%x]\n", opcode, stm->pc );
		// stm->dump(stm);
		stm->circuit_ifs->dump( stm );
		exit(1);
	}

	return 0x00;
}
#endif

#if 1
uint32_t stm_run_one_no_bp(stm_t * stm);

uint32_t stm_run_one(stm_t * stm)
{
  // check breakpoint for flash address
  if( check_instruction_bp( stm->pc ))
  {
    stm->state = cpu_BpFlash;
    return 0x00;
  }

  if( check_cycle_bp( stm->cycle ))
  {
      stm->state = cpu_BpCycle;
      return 0x00;
  }

  return stm_run_one_no_bp( stm );
}
#endif


/*
 *
 */
uint32_t stm_run_one_no_bp(stm_t * stm)
{
  /* Ensure we don't crash simstm due to a bad instruction reading past
   * the end of the flash.
   */
  if (unlikely(stm->pc >= stm->flash_end_adr))
  {
    STM_LOG( stm, LOG_TRACE, "\n\nCRASH PC[%08x]", stm->pc );
    stm->state = cpu_Crashed;
//    crash(stm);
    return 0;
  }

  // cpu cycles update
  stm->cycle++;

  // nano ticks update
  stm->nano_ticks += stm->nano_ticks_per_cycle;

  uint16_t    opcode = _stm_flash_read16le( stm, stm->pc);

  stm_flashaddr_t ( *pInstFunc )(struct stm_t * stm, uint16_t opcode ) = instruction_matrix[ opcode ];

  stm_flashaddr_t pc_new;
  if( pInstFunc )
  {
    pc_new = pInstFunc( stm, opcode );
    stm->pc = pc_new;

//    // stack spoiling trup
//    uint32_t val;
//    stm_addr_read( stm, 0x20000418, &val );
//    STM_LOG( stm, LOG_WARNING, "!!!0x20000418[%08X]\n", val );
  }
  else
  {
    // TODO - unknown code
    STM_LOG( stm, LOG_ERROR, "Unknown ins[%04x] at flash[%x]\n", opcode, stm->pc );
    // stm->dump(stm);
    stm->circuit_ifs->dump( stm );
    exit(1);
  }

  return 0x00;
}
/*
 *
 */
uint32_t stm_core_init( struct mcu_t * mcu )
{
	if( mcu )
	{
		stm32l0x_init( mcu->core );
		return 0x01;
	}
	else
	{
		// TODO: Add message for wrong MCU pointer
		return 0;
	}
}

/*
 *
 */
// typedef void ( *stm_cir_release_t )(struct mcu_t * mcu );
void stm_core_release( struct mcu_t * mcu, void* obj )
{
	if( mcu )
	{
		stm32l0x_release( mcu->core );
	}
	else
	{
		// TODO: Add message for wrong MCU pointer
		exit(1);
	}
}

/*
 *
 */
// typedef void ( *stm_cir_reset_t )(struct mcu_t * mcu, uint32_t type );
void stm_core_reset( struct mcu_t * mcu, uint32_t type )
{
	if( mcu )
	{
		stm32l0x_reset( mcu->core );

		stm_cycle_timer_reset( mcu->core );
	}
	else
	{
		// TODO: Add message for wrong MCU pointer
		exit(1);
	}
}

/*
 *
 */
// typedef void ( *stm_dump_cir_state_t )( void* mcu );
void stm_core_dump( void * circuit )
{
	stm_t * stm =( stm_t* ) circuit;
	if( stm )
	{
		stm32l0x_dump( stm );
	}
	else
	{
		// TODO: nothing to dump
	}
}

/*
 *
 */
stm_t * stm_core_fabric( const char *name )
{
	stm_t * stm = malloc(sizeof( struct stm_t ));

	if( stm )
	{
	  memset( stm, 0x00, sizeof( struct stm_t ));

	  stm_circuit_ifs_t* pcircuit =  malloc( sizeof( stm_circuit_ifs_t ));
    if( pcircuit )
    {
      memset( pcircuit, 0, sizeof( stm_circuit_ifs_t ));
      stm->circuit_ifs 					= pcircuit;
      stm->circuit_ifs->init 		= stm_core_init;
      stm->circuit_ifs->reset 	= stm_core_reset;
      stm->circuit_ifs->release = stm_core_release;
      stm->circuit_ifs->dump 		= stm_core_dump;
    }
    else
    {
      // TODO: Add Error message
      exit( 0 );
    }
	}
	return stm;
}

