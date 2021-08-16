/*! \file      sim_gpio.c
    \version   0.0
    \date      19 èþë. 2020 ã. 22:58:09
    \brief     
    \details   
    \bug
    \copyright 
    \author ruarka

    \note Pin speed is not implemented yet.
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
pfn_pins_custom_layout_names_init* custom_pins_layout = NULL;

/* ------------------------------------------------------------------------------------------------
 *                         Local Variables & Functions
 * ------------------------------------------------------------------------------------------------
 */

void pin_reset( struct stm_pin_t* pin, uint32_t reset_type )
{
}
void pin_release( struct stm_pin_t* pin )
{
}
void pin_set_pin_value( struct stm_pin_t* pin, uint32_t val, stm_pin_mode_t pins_type )
{
}
void pin_get_pin_state( struct stm_pin_t* pin, uint32_t* val, stm_pin_mode_t* pins_type )
{
}
void pin_set_pin_cb( struct stm_pin_t* pin, pfn_pin_change_state_cb_t fn )
{
}

void pin_set_pin_ext_cb( struct stm_pin_t* pin, pfn_pin_change_state_cb_t fn )
{
  ext_pin_cb_cell_t* ptcell = pin->ext_cb_pool;

  // find the last
  if( !ptcell ){
    ptcell              = malloc( sizeof( ext_pin_cb_cell_t ));
    ptcell->next        = NULL;
    ptcell->ext_pin_cb  = fn;
    pin->ext_cb_pool    = ptcell;
    return;
  }

  while( ptcell->next )
    ptcell = ptcell->next;

  ptcell->next = malloc( sizeof( ext_pin_cb_cell_t ));
  ptcell->next->next = NULL;
  ptcell->next->ext_pin_cb = fn;
}

uint32_t pin_analog_to_digital( uint32_t val )
{
  if( val > 0x01000 )
    return 1;
  return 0;
}

uint32_t pin_floating_to_digital( uint32_t val )
{
  if( val > 0xfffff )
    return 1;
  return 0;
}

static
void pin_send_val_to_all_cbs( struct stm_pin_t* pin )
{
  /* Send new value to all callbacks */
  sim_an_pin_cb_cell_t* pAn = pin->an_cb_pool;
  while( pAn ){
    pAn->sim_pin_cb( pin );
    pAn = pAn->next;
  }

  sim_al_pin_cb_cell_t* pAl = pin->al_cb_pool;
  while( pAl ){
    pAl->sim_pin_cb( pin );
    pAl = pAl->next;
  }

  ext_pin_cb_cell_t* pEx = pin->ext_cb_pool;
  while( pEx ){
    pEx->ext_pin_cb( pin );
    pEx = pEx->next;
  }
}

void pin_IDR_update_value( struct stm_pin_t* pin )
{
  switch( pin->val_type ){
    case vtFloating:
      if( pin_floating_to_digital( ( uint32_t )rand())){
        /* Write 1 to IDR bit */
        pin->port->regs.IDR |=( 1<<pin->pin_num );
      }else{
        /* Write 0 to IDR bit */
          pin->port->regs.IDR &= ~( 1<<pin->pin_num );
      }
      break;

    case vtDigital:
      if( pin->val ){
        /* Write 1 to IDR bit */
        pin->port->regs.IDR |=( 1<<pin->pin_num );
      }else{
        /* Write 0 to IDR bit */
          pin->port->regs.IDR &= ~( 1<<pin->pin_num );
      }
      break;

    case vtAnalog:
      if( pin_analog_to_digital( pin->val )){
        /* Write 1 to IDR bit */
        pin->port->regs.IDR |=( 1<<pin->pin_num );
      }else{
        /* Write 0 to IDR bit */
          pin->port->regs.IDR &= ~( 1<<pin->pin_num );
      }
      break;

    case vtUndefined:
    case vtCollision:
      if( pin_analog_to_digital( ( uint32_t )rand() )){
        /* Write 1 to IDR bit */
        pin->port->regs.IDR |=( 1<<pin->pin_num );
      }else{
        /* Write 0 to IDR bit */
          pin->port->regs.IDR &= ~( 1<<pin->pin_num );
      }
      break;

    default:
      break;
  }
}

void pin_MCU_sources_update_val( struct stm_pin_t* pin )
{
  if( pin->analog.is_active ){
    pin->val      = pin->analog.val;
    pin->val_type = pin->analog.val_type;
    return;
  }

  if( pin->alternate.is_active ){
    pin->val      = pin->alternate.val;
    pin->val_type = pin->alternate.val_type;
    return;
  }

  if( pin->odr.is_active ){
    pin->val      = pin->odr.val;
    pin->val_type = pin->odr.val_type;
    return;
  }

  pin->val        = PIN_FLOATING_VALUE;
  pin->val_type   = vtFloating;
}

void pin_ODR_update_val( struct stm_pin_t* pin, uint32_t val, stm_pin_value_type_t val_type )
{
  uint32_t old_val = pin->val;
  stm_pin_value_type_t old_val_t = pin->val_type;

  // Keep signal source data
  pin->odr.val       = val;
  pin->odr.val_type  = val_type;

  if( pin->ext.is_active )
  {
    pin->val        = PIN_COLLISION_VALUE;
    pin->val_type   = vtCollision;
  }else{
    pin->val        = val;
    pin->val_type   = val_type;
  }

  pin_IDR_update_value( pin );

  if(( old_val != pin->val )
    ||( old_val_t != pin->val_type ))
      pin_send_val_to_all_cbs(pin);
}

void pin_External_update_val( struct stm_pin_t* pin, uint32_t val, stm_pin_value_type_t val_type )
{
  uint32_t old_val = pin->val;
  stm_pin_value_type_t old_val_t = pin->val_type;

  pin->ext.val      = val;
  pin->ext.val_type = val_type;

  if(( val_type == vtDigital )
    ||( val_type == vtAnalog )){

    if(( pin->odr.is_active )
        ||( pin->analog.is_active )
        ||( pin->alternate.is_active ))
    {
      pin->val        = PIN_COLLISION_VALUE;
      pin->val_type   = vtCollision;
    }else{
      pin->val        = val;
      pin->val_type   = val_type;
    }
    pin->ext.is_active = 1;  // ext used as signal source

  }else{
    pin->ext.is_active = 0;  // ext in not a signal source
    pin_MCU_sources_update_val( pin );
  }

  pin_IDR_update_value( pin );


  if(( old_val != pin->val )
    ||( old_val_t != pin->val_type ))
      pin_send_val_to_all_cbs(pin);
}

void pin_Analog_update_val( struct stm_pin_t* pin, uint32_t val, stm_pin_value_type_t val_type )
{
  uint32_t old_val = pin->val;
  stm_pin_value_type_t old_val_t = pin->val_type;

  // Keep signal source data
  pin->analog.val       = val;
  pin->analog.val_type  = val_type;

  if( pin->ext.is_active )
  {
    pin->val        = pin->ext.val;
    pin->val_type   = pin->ext.val_type;
    return;
  } else {
    pin->val        = val;
    pin->val_type   = val_type;
  }

  pin_IDR_update_value( pin );

  if(( old_val != pin->val )
    ||( old_val_t != pin->val_type ))
      pin_send_val_to_all_cbs(pin);
}

void pin_Alternative_update_val( struct stm_pin_t* pin, uint32_t val, stm_pin_value_type_t val_type )
{
}

void pin_update_value( struct stm_pin_t* pin, uint32_t val, stm_pin_value_type_t val_type )
{
  uint32_t output_active_number = 0;
  uint32_t tmpVal;

  if( pin->odr.is_active ){
    output_active_number++;
    pin->odr.val = tmpVal =val;
  }

  if( pin->alternate.is_active ){
    if( output_active_number > 0  ){
      // Error
      GPIO_TRACE( pin->mcu->core, LOG_ERROR, "PIN[%s]:GPIO:Output&Alt are active", pin->name );
      pin->state = stColision;
      pin->val = PIN_COLLISION_VALUE;
      pin->val_type = vtCollision;
      return;
    } else{
      output_active_number++;
      pin->alternate.val = tmpVal = val;
    }
  }

  if( pin->ext.is_active ){
    if( output_active_number > 0  ){
      // Error
      pin->state = stColision;
      pin->val = PIN_COLLISION_VALUE;
      pin->val_type = vtCollision;
      return;
    } else{
      output_active_number++;
      pin->ext.val = tmpVal = val;
    }
  }

  if( pin->analog.is_active ){
    if( output_active_number > 0 ){
      pin->val = val;
      pin->val = PIN_COLLISION_VALUE;
      pin->val_type = vtCollision;
      return;
    } else{
      pin->analog.val = val;
    }
  }

  GPIO_TRACE( pin->mcu->core, LOG_ERROR, "PIN[%s]:GPIO:No one src is active", pin->name );
}

void pin_set_mode( struct stm_pin_t* pin, stm_pin_mode_t set_pin_mode )
{
  uint32_t old_val    = pin->val;
  uint32_t old_val_t  = pin->val_type;

  if(( pin->state == stUnused )
     ||( pin->mode == set_pin_mode ))
    return;

  pin->mode = set_pin_mode;

  switch( set_pin_mode ) {
    case Input:
      pin->odr.is_active        = 0;
      pin->alternate.is_active  = 0;
      pin->analog.is_active     = 0;

      if( pin->ext.is_active )
      {
        pin->val = pin->ext.val;
        pin->val_type = pin->ext.val_type;
      }else{

        pin->val      = PIN_FLOATING_VALUE;
        pin->val_type = vtFloating;
      }

      pin_IDR_update_value( pin );

      if(( old_val != pin->val )
        ||( old_val_t != pin->val_type ))
          pin_send_val_to_all_cbs(pin);
      break;

    case Output:
      pin->odr.is_active        = 1;
      pin->alternate.is_active  = 0;
      pin->analog.is_active     = 0;

      if( pin->port->regs.ODR &( 1<<pin->pin_num ))
        pin->data.pfn_ODR_pin_value( pin, PIN_DIGITAL_ONE_VALUE, vtDigital );
      else
        pin->data.pfn_ODR_pin_value( pin, PIN_DIGITAL_ZERO_VALUE, vtDigital );

      break;

    case Alternate:
// TODO:
//      switch( pin->pin_type ) {
//        case Undefined: pin_Undefined_Alternate_mode(pin, set_pin_mode); break;
//        case Unused:    pin_Unused_Alternate_mode(pin, set_pin_mode); break;
//        case Input:     pin_Input_Alternate_mode(pin, set_pin_mode); break;
//        case Output:    pin_Output_Alternate_mode(pin, set_pin_mode); break;
//        case Alternate: pin_Alternate_Alternate_mode(pin, set_pin_mode); break;
//        case Analog:    pin_Analog_Alternate_mode(pin, set_pin_mode); break;
//        case Layout:    pin_Layout_Alternate_mode(pin, set_pin_mode); break;
//          break;
//
//        default:
//          break;
//      }
      break;

    case Analog:
      pin->odr.is_active        = 0;  /* set false */
      pin->alternate.is_active  = 0;
      pin->analog.is_active     = 1;

      /* Write 0 to IDR bit */
      pin->data.pfn_Analog_pin_value( pin, PIN_FLOATING_VALUE, vtFloating );
      break;

    default:
      break;
  }
}

void pin_rd( struct stm_pin_t* pin, uint32_t* val, stm_pin_value_type_t* val_type )
{
}

void pin_wr( struct stm_pin_t* pin, uint32_t val, stm_pin_value_type_t val_type )
{
}

/* ................................................................................................
 *  GPIO part definition
 */

/*
 * RCC Callback function
 */
void gpio_rcc_onupdate_cb(struct mcu_t * mcu, uint32_t param, uint64_t ticks_per_nsec, uint64_t frequency)
{
	mcu->portB->nano_ticks_per_cycle = ticks_per_nsec;
	mcu->portB->frequency 					 = frequency;
}

/*
 *
 */
void stm_gpio_MODER_wr( stm_gpio_t* p, uint32_t val )
{
  uint32_t tMODER;
  p->regs.MODER = tMODER = val;

  uint32_t idx = tMODER & 0x03;

  GPIO_TRACE( p->mcu->core, LOG_TRACE, "PORT[%s] MODER[%04x]", p->gpioPortName, val );

  for( uint32_t j=0; j<16; j++ ){
    /* Pin state update */
    switch ( idx ) {
      case 0: // Input
        pin_set_mode( &(p->pins[ j ]), Input );
        break;

      case 1: // Output
        pin_set_mode( &(p->pins[ j ]), Output );
        break;

      case 2: // Alternative
        pin_set_mode( &(p->pins[ j ]), Alternate );
        break;

      case 3: // Analog
         pin_set_mode( &(p->pins[ j ]), Analog );
         break;
      }

    tMODER =( tMODER >> 2 );
    idx = tMODER & 0x03;
  }
}

/*
 * There is no deference between push-pull and open-drain
 */
void stm_gpio_OTYPER_wr( stm_gpio_t* p, uint32_t val )
{
  GPIO_TRACE( p->mcu->core, LOG_TRACE, "PORT[%s] OTYPER[%04x]", p->gpioPortName, val );

  p->regs.OTYPER = val;
}
/*
 * No time delay is supported for simulation
 */
void stm_gpio_OSPEEDR_wr( stm_gpio_t* p, uint32_t val )
{
  GPIO_TRACE( p->mcu->core, LOG_TRACE, "\nPORT[%s] OSPEEDR[%04x]", p->gpioPortName, val );

  p->regs.OSPEEDR = val;
}
/*
 * It is not of scope for digital simulation
 */
void stm_gpio_PUPDR_wr( stm_gpio_t* p, uint32_t val )
{
  GPIO_TRACE( p->mcu->core, LOG_TRACE, "\nPORT[%s] PUPDR[%04x]", p->gpioPortName, val );

  p->regs.PUPDR = val;
}

void stm_gpio_IDR_wr( stm_gpio_t* p, uint32_t val )
{
  GPIO_TRACE( p->mcu->core, LOG_TRACE, "\nPORT[%s] IDR[%04x] -Err", p->gpioPortName, val );
}

void stm_gpio_ODR_wr( stm_gpio_t* p, uint32_t val )
{
  GPIO_TRACE( p->mcu->core, LOG_TRACE, "\nPORT[%s] ODR[%04x]", p->gpioPortName, val );

  uint32_t ones = p->regs.ODR ^ val;
  p->regs.ODR = val;

  for( int i = 0; ones; i++ ){
    if( ones & 1 )
      if(( p->pins[ i ].state != stUnused )
        &&( p->pins[ i ].odr.is_active )){

      if( val &( 1<<i ))
        p->pins[ i ].data.pfn_ODR_pin_value( &( p->pins[ i ]), PIN_DIGITAL_ONE_VALUE, vtDigital );
      else
        p->pins[ i ].data.pfn_ODR_pin_value( &( p->pins[ i ]), PIN_DIGITAL_ZERO_VALUE, vtDigital );


//      pin_IDR_update_value( &( p->pins[ i ]));
//
//      pin_send_val_to_all_cbs( &( p->pins[ i ]));
    }
    ones =( ones>>1 );
  }
}

void stm_gpio_BSRR_wr( stm_gpio_t* p, uint32_t val )
{
  uint32_t ones;
  uint32_t oldODR = p->regs.ODR;

  // bit set
  p->regs.ODR |=( val & 0xffff );

  ones = oldODR ^ p->regs.ODR;

  for( int i = 0; ones; i++ ){
    if( ones & 1 )
      if(( p->pins[ i ].state != stUnused )
        &&( p->pins[ i ].odr.is_active )){
      {
        p->pins[ i ].data.pfn_ODR_pin_value(&( p->pins[ i ]), PIN_DIGITAL_ONE_VALUE, vtDigital );

//        pin_IDR_update_value( &( p->pins[ i ]));
//
//        pin_send_val_to_all_cbs( &( p->pins[ i ] ));
      }
    }
    ones = ones >> 1;
  }

  // bit clear
  oldODR = p->regs.ODR;
  p->regs.ODR &=(( ~val )>>16 );

  ones = oldODR ^ p->regs.ODR;

  for( int i = 0; ones; i++ ){
    if( ones & 1 )
      if(( p->pins[ i ].state != stUnused )
        &&( p->pins[ i ].odr.is_active )){
      {
        p->pins[ i ].data.pfn_ODR_pin_value(&( p->pins[ i ]), PIN_DIGITAL_ZERO_VALUE, vtDigital );

//        pin_IDR_update_value( &( p->pins[ i ]));
//
//        pin_send_val_to_all_cbs( &( p->pins[ i ] ));
      }
    }
    ones = ones >> 1;
  }

  GPIO_TRACE( p->mcu->core, LOG_TRACE, "\nPORT[%s] BSRR[%04x] ODR[%08x]", p->gpioPortName, val, p->mcu->portB->regs.ODR );
}

void stm_gpio_LCKR_wr( stm_gpio_t* p, uint32_t val )
{
  GPIO_TRACE( p->mcu->core, LOG_TRACE, "\nPORT[%s] LCKR[%04x]", p->gpioPortName, val );

  p->regs.LCKR = val;
}

void stm_gpio_AFR0_wr( stm_gpio_t* p, uint32_t val )
{
  GPIO_TRACE( p->mcu->core, LOG_TRACE, "\nPORT[%s] AFR0[%04x]", p->gpioPortName, val );

  p->regs.AFR[ 0 ]= val;
}

void stm_gpio_AFR1_wr( stm_gpio_t* p, uint32_t val )
{
  GPIO_TRACE( p->mcu->core, LOG_TRACE, "\nPORT[%s] AFR1[%04x]", p->gpioPortName, val );

  p->regs.AFR[ 1 ]= val;
}

void stm_gpio_BRR_wr( stm_gpio_t* p, uint32_t val )
{
  GPIO_TRACE( p->mcu->core, LOG_TRACE, "\nPORT[%s] BRR[%04x]", p->gpioPortName, val );

  // bit clear
  uint32_t oldODR = p->regs.ODR;
  p->regs.ODR &=( ~val>>16 );

  uint32_t ones = oldODR ^ p->regs.ODR;

  for( int i = 0; ones; ++i ){
    if( ones & 1 )
      if(( p->pins[ i ].state != stUnused )
        &&( p->pins[ i ].odr.is_active )){
      {
        p->pins[ i ].data.pfn_ODR_pin_value( &( p->pins[ i ]), PIN_DIGITAL_ZERO_VALUE, vtDigital );

//        pin_IDR_update_value( &( p->pins[ i ]));
//
//        pin_send_val_to_all_cbs( &( p->pins[ i ] ));
      }
      ones = ones>>1;
    }
  }
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
static uint32_t stm_gpio_init( mcu_t * mcu )
{
  char pinNameBuf[ 10 ];
	stm_gpio_t* p = mcu->portB;

  /* pins initialization */
	for( int i=0; i<16; ++i )	{

	  // assign ctrl ifs functions
	  p->pins[ i ].ctrl.pfn_reset           = pin_reset;
	  p->pins[ i ].ctrl.pfn_release         = pin_release;
	  p->pins[ i ].ctrl.pfn_set_pin_value   = pin_set_pin_value;
	  p->pins[ i ].ctrl.pfn_get_pin_state   = pin_get_pin_state;
	  p->pins[ i ].ctrl.pfn_set_pin_cb      = pin_set_pin_cb;
	  p->pins[ i ].ctrl.pfn_set_pin_ext_cb  = pin_set_pin_ext_cb;

	  // assign data ifs functions
	  p->pins[ i ].data.pfn_ODR_pin_value         = pin_ODR_update_val;
	  p->pins[ i ].data.pfn_External_pin_value    = pin_External_update_val;
	  p->pins[ i ].data.pfn_Alternative_pin_value = pin_Alternative_update_val;
	  p->pins[ i ].data.pfn_Analog_pin_value      = pin_Analog_update_val;

	  p->pins[ i ].state                    = stInUse;

	  sprintf( pinNameBuf, "%s%02i", p->gpioPortName, i );
	  strncpy( p->pins[ i ].name, pinNameBuf, 4 );

	  p->pins[ i ].al_cb_pool = NULL;
	  p->pins[ i ].an_cb_pool = NULL;
	  p->pins[ i ].ext_cb_pool = NULL;
  }

	// bind core to rcc
	stm_rcc_updt_cb_reg( mcu, gpio_rcc_onupdate_cb, NULL );

	return 0x00;
}

/*
 *
 */
void stm_gpio_reset(mcu_t * mcu, uint32_t type )
{
	stm_gpio_t* p = mcu->portB;

	if( p )
	{
	  /* Pins reset */
	  for( int i=0; i<16 ; ++i )
	    if( p->pins[ i ].state != stUnused )
	      p->pins[ i ].ctrl.pfn_reset(&( p->pins[ i ]), type );

		stm_gpio_MODER_wr( p, 0xFFFFFFFF );
		stm_gpio_OTYPER_wr( p, 0x00000000 );
		stm_gpio_OSPEEDR_wr( p, 0x00000000 );
		stm_gpio_PUPDR_wr( p, 0x00000000 );
		stm_gpio_IDR_wr( p, 0x0000a5a5 );
//		stm_gpio_ODR_wr( p, 0x00000000 );
		p->regs.ODR = 0x00000000;
//		stm_gpio_BSRR_wr( p, 0x00000000 );
		p->regs.BSRR = 0x00000000;
		stm_gpio_LCKR_wr( p, 0x00000000 );
		stm_gpio_AFR0_wr( p, 0x00000000 );
		stm_gpio_AFR1_wr( p, 0x00000000 );
		stm_gpio_BRR_wr( p, 0x00000000 );
	}
}

/*
 *
 */
void stm_gpio_release( mcu_t * mcu, void* obj )
{
	if( mcu )
	{
	  struct stm_gpio_t* p =( struct stm_gpio_t* )obj;

	  /* Release Pins */
	  for( int i = 0; i<16; ++i ){
	      p->pins[ i ].ctrl.pfn_release(&( p->pins[ i ]));
	  }

		if( mcu->portB->cir_cb_ifs )
			free( mcu->portB->cir_cb_ifs );
		mcu->portB->cir_cb_ifs = NULL;

		/* In,Out pin release */


		/*   */
		free( mcu->portB );
		mcu->portB = NULL;
	}
}

/*
 *
 */
void stm_gpio_wr_cb_reg( struct mcu_t * mcu, void* cb_wr, uint32_t addr, void* param )
{
}

/*
 *
 */
void stm_gpio_rd_cb_reg( struct mcu_t * mcu, void* cb_rd, uint32_t addr, void* param )
{
}

/*
 * uint32_t word writing
 */
void stm_gpio_write( struct mcu_t* mcu, uint32_t addr, uint32_t val, void* param )
{
	switch (addr) {
		case 0x00:
			stm_gpio_MODER_wr( mcu->portB, val );
			break;

		case 0x04:
			stm_gpio_OTYPER_wr( mcu->portB, val );
			break;

		case 0x08:
			stm_gpio_OSPEEDR_wr( mcu->portB, val );
			break;

		case 0x0C:
		  stm_gpio_PUPDR_wr( mcu->portB, val );
			break;

		case 0x10:
	    stm_gpio_IDR_wr( mcu->portB, val );
			break;

		case 0x14:
        stm_gpio_ODR_wr( mcu->portB, val );
			break;

		case 0x18:
      stm_gpio_BSRR_wr( mcu->portB, val );
			break;

		case 0x1C:
			stm_gpio_LCKR_wr( mcu->portB, val );
			break;

		case 0x20:
			stm_gpio_AFR0_wr( mcu->portB, val );
			break;

		case 0x24:
			stm_gpio_AFR1_wr( mcu->portB, val );
			break;

		case 0x28:
			stm_gpio_BRR_wr( mcu->portB, val );
			break;

		default:
			break;
	}
}

/*
 * uint32_t word reading
 */
void stm_gpio_read( struct mcu_t* mcu, uint32_t addr, uint32_t* val, void* param )
{
	switch (addr) {
		case 0x00:
			*val = mcu->portB->regs.MODER;
			break;

		case 0x04:
			*val = mcu->portB->regs.OTYPER;
			break;

		case 0x08:
			*val = mcu->portB->regs.OSPEEDR;
			break;

		case 0x0C:
			*val = mcu->portB->regs.PUPDR;
			break;

		case 0x10:
			*val = mcu->portB->regs.IDR;
			break;

		case 0x14:
			*val = mcu->portB->regs.ODR;
			break;

		case 0x18:
			*val = mcu->portB->regs.BSRR;
			break;

		case 0x1C:
			*val = mcu->portB->regs.LCKR;
			break;

		case 0x20:
			*val = mcu->portB->regs.AFR[ 0 ];
			break;

		case 0x24:
			*val = mcu->portB->regs.AFR[ 1 ];
			break;

		case 0x28:
			*val = mcu->portB->regs.BRR;
			break;

		default:
			break;
	}
}

/*
 *
 */
void stm_gpio_dump_state( void* pgpio )
{
	stm_gpio_t* p = pgpio;
	stm_t * stm = p->mcu->core;
	if( stm )
	{
		GPIO_TRACE( stm, LOG_TRACE, "\n\nGPIO PORTB Circuit Dumping\n--------------" );

		// MODER print
  	GPIO_TRACE( stm, LOG_TRACE, "\nMODER[%08X]   15[%d] 14[%d]", p->regs.MODER, (p->regs.MODER>>30)&0x03, (p->regs.MODER>>28)&0x03 );
  	GPIO_TRACE( stm, LOG_TRACE, " 13[%d] 12[%d] 11[%d] 10[%d]", (p->regs.MODER>>26)&0x03, (p->regs.MODER>>24)&0x03, (p->regs.MODER>>22)&0x03, (p->regs.MODER>>20)&0x03 );
  	GPIO_TRACE( stm, LOG_TRACE, " 9[%d] 8[%d] 7[%d] 6[%d]", (p->regs.MODER>>18)&0x03, (p->regs.MODER>>16)&0x03, (p->regs.MODER>>14)&0x03, (p->regs.MODER>>12)&0x03 );
  	GPIO_TRACE( stm, LOG_TRACE, " 5[%d] 4[%d] 3[%d] 2[%d]", (p->regs.MODER>>10)&0x03, (p->regs.MODER>>8)&0x03, (p->regs.MODER>>6)&0x03, (p->regs.MODER>>4)&0x03 );
  	GPIO_TRACE( stm, LOG_TRACE, " 1[%d] 0[%d]", (p->regs.MODER>>2)&0x03, (p->regs.MODER)&0x03 );

  	// OTYPER
  	GPIO_TRACE( stm, LOG_TRACE, "\nOTYPER[%08X]  15[%d] 14[%d]", p->regs.OTYPER, (p->regs.OTYPER>>15)&0x01, (p->regs.OTYPER>>14)&0x01 );
  	GPIO_TRACE( stm, LOG_TRACE, " 13[%d] 12[%d] 11[%d] 10[%d]", (p->regs.OTYPER>>13)&0x01, (p->regs.OTYPER>>12)&0x01, (p->regs.OTYPER>>11)&0x01, (p->regs.OTYPER>>10)&0x01 );
  	GPIO_TRACE( stm, LOG_TRACE, " 9[%d] 8[%d] 7[%d] 6[%d]", (p->regs.OTYPER>>9)&0x01, (p->regs.OTYPER>>8)&0x01, (p->regs.OTYPER>>7)&0x01, (p->regs.OTYPER>>6)&0x01 );
  	GPIO_TRACE( stm, LOG_TRACE, " 5[%d] 4[%d] 3[%d] 2[%d]", (p->regs.OTYPER>>5)&0x01, (p->regs.OTYPER>>4)&0x01, (p->regs.OTYPER>>3)&0x01, (p->regs.OTYPER>>2)&0x01 );
  	GPIO_TRACE( stm, LOG_TRACE, " 1[%d] 0[%d]", (p->regs.OTYPER>>1)&0x01, (p->regs.OTYPER)&0x01 );

  	// OSPEEDR
  	GPIO_TRACE( stm, LOG_TRACE, "\nOSPEEDR[%08X] 15[%d] 14[%d]", p->regs.OSPEEDR, (p->regs.OSPEEDR>>30)&0x03, (p->regs.OSPEEDR>>28)&0x03 );
  	GPIO_TRACE( stm, LOG_TRACE, " 13[%d] 12[%d] 11[%d] 10[%d]", (p->regs.OSPEEDR>>26)&0x03, (p->regs.OSPEEDR>>24)&0x03, (p->regs.OSPEEDR>>22)&0x03, (p->regs.OSPEEDR>>20)&0x03 );
  	GPIO_TRACE( stm, LOG_TRACE, " 9[%d] 8[%d] 7[%d] 6[%d]", (p->regs.OSPEEDR>>18)&0x03, (p->regs.OSPEEDR>>16)&0x03, (p->regs.OSPEEDR>>14)&0x03, (p->regs.OSPEEDR>>12)&0x03 );
  	GPIO_TRACE( stm, LOG_TRACE, " 5[%d] 4[%d] 3[%d] 2[%d]", (p->regs.OSPEEDR>>10)&0x03, (p->regs.OSPEEDR>>8)&0x03, (p->regs.OSPEEDR>>6)&0x03, (p->regs.OSPEEDR>>4)&0x03 );
  	GPIO_TRACE( stm, LOG_TRACE, " 1[%d] 0[%d]", (p->regs.OSPEEDR>>2)&0x03, (p->regs.OSPEEDR)&0x03 );

  	// PUPDR
  	GPIO_TRACE( stm, LOG_TRACE, "\nPUPDR[%08X]   15[%d] 14[%d]", p->regs.PUPDR, (p->regs.PUPDR>>30)&0x03, (p->regs.PUPDR>>28)&0x03 );
  	GPIO_TRACE( stm, LOG_TRACE, " 13[%d] 12[%d] 11[%d] 10[%d]", (p->regs.PUPDR>>26)&0x03, (p->regs.PUPDR>>24)&0x03, (p->regs.PUPDR>>22)&0x03, (p->regs.PUPDR>>20)&0x03 );
  	GPIO_TRACE( stm, LOG_TRACE, " 9[%d] 8[%d] 7[%d] 6[%d]", (p->regs.PUPDR>>18)&0x03, (p->regs.PUPDR>>16)&0x03, (p->regs.PUPDR>>14)&0x03, (p->regs.PUPDR>>12)&0x03 );
  	GPIO_TRACE( stm, LOG_TRACE, " 5[%d] 4[%d] 3[%d] 2[%d]", (p->regs.PUPDR>>10)&0x03, (p->regs.PUPDR>>8)&0x03, (p->regs.PUPDR>>6)&0x03, (p->regs.PUPDR>>4)&0x03 );
  	GPIO_TRACE( stm, LOG_TRACE, " 1[%d] 0[%d]", (p->regs.PUPDR>>2)&0x03, (p->regs.PUPDR)&0x03 );

  	// IDR
  	GPIO_TRACE( stm, LOG_TRACE, "\nIDR[%08X]     15[%d] 14[%d]", p->regs.IDR, (p->regs.IDR>>15)&0x01, (p->regs.IDR>>14)&0x01 );
  	GPIO_TRACE( stm, LOG_TRACE, " 13[%d] 12[%d] 11[%d] 10[%d]", (p->regs.IDR>>13)&0x01, (p->regs.IDR>>12)&0x01, (p->regs.IDR>>11)&0x01, (p->regs.IDR>>10)&0x01 );
  	GPIO_TRACE( stm, LOG_TRACE, " 9[%d] 8[%d] 7[%d] 6[%d]", (p->regs.IDR>>9)&0x01, (p->regs.IDR>>8)&0x01, (p->regs.IDR>>7)&0x01, (p->regs.IDR>>6)&0x01 );
  	GPIO_TRACE( stm, LOG_TRACE, " 5[%d] 4[%d] 3[%d] 2[%d]", (p->regs.IDR>>5)&0x01, (p->regs.IDR>>4)&0x01, (p->regs.IDR>>3)&0x01, (p->regs.IDR>>2)&0x01 );
  	GPIO_TRACE( stm, LOG_TRACE, " 1[%d] 0[%d]", (p->regs.IDR>>1)&0x01, (p->regs.IDR)&0x01 );

  	// ODR
  	GPIO_TRACE( stm, LOG_TRACE, "\nODR[%08X]     15[%d] 14[%d]", p->regs.ODR, (p->regs.ODR>>15)&0x01, (p->regs.ODR>>14)&0x01 );
  	GPIO_TRACE( stm, LOG_TRACE, " 13[%d] 12[%d] 11[%d] 10[%d]", (p->regs.ODR>>13)&0x01, (p->regs.ODR>>12)&0x01, (p->regs.ODR>>11)&0x01, (p->regs.ODR>>10)&0x01 );
  	GPIO_TRACE( stm, LOG_TRACE, " 9[%d] 8[%d] 7[%d] 6[%d]", (p->regs.ODR>>9)&0x01, (p->regs.ODR>>8)&0x01, (p->regs.ODR>>7)&0x01, (p->regs.ODR>>6)&0x01 );
  	GPIO_TRACE( stm, LOG_TRACE, " 5[%d] 4[%d] 3[%d] 2[%d]", (p->regs.ODR>>5)&0x01, (p->regs.ODR>>4)&0x01, (p->regs.ODR>>3)&0x01, (p->regs.ODR>>2)&0x01 );
  	GPIO_TRACE( stm, LOG_TRACE, " 1[%d] 0[%d]", (p->regs.ODR>>1)&0x01, (p->regs.ODR)&0x01 );

  	// BSRR
  	GPIO_TRACE( stm, LOG_TRACE, "\nBSRR[%08X]    31[%d] 30[%d]", p->regs.BSRR, (p->regs.BSRR>>31)&0x01, (p->regs.BSRR>>30)&0x01 );
  	GPIO_TRACE( stm, LOG_TRACE, " 29[%d] 28[%d] 27[%d] 26[%d]", (p->regs.BSRR>>29)&0x01, (p->regs.BSRR>>28)&0x01, (p->regs.BSRR>>27)&0x01, (p->regs.BSRR>>26)&0x01 );
  	GPIO_TRACE( stm, LOG_TRACE, " 25[%d] 24[%d] 23[%d] 22[%d]", (p->regs.BSRR>>25)&0x01, (p->regs.BSRR>>24)&0x01, (p->regs.BSRR>>23)&0x01, (p->regs.BSRR>>22)&0x01 );
  	GPIO_TRACE( stm, LOG_TRACE, " 21[%d] 20[%d] 19[%d] 18[%d]", (p->regs.BSRR>>21)&0x01, (p->regs.BSRR>>20)&0x01, (p->regs.BSRR>>19)&0x01, (p->regs.BSRR>>18)&0x01 );
  	GPIO_TRACE( stm, LOG_TRACE, " 17[%d] 16[%d]", (p->regs.BSRR>>17)&0x01, (p->regs.BSRR>>16)&0x01 );

  	GPIO_TRACE( stm, LOG_TRACE, " 15[%d] 14[%d]", p->regs.BSRR, (p->regs.BSRR>>15)&0x01, (p->regs.BSRR>>14)&0x01 );
  	GPIO_TRACE( stm, LOG_TRACE, " 13[%d] 12[%d] 11[%d] 10[%d]", (p->regs.BSRR>>13)&0x01, (p->regs.BSRR>>12)&0x01, (p->regs.BSRR>>11)&0x01, (p->regs.BSRR>>10)&0x01 );
  	GPIO_TRACE( stm, LOG_TRACE, " 9[%d] 8[%d] 7[%d] 6[%d]", (p->regs.BSRR>>9)&0x01, (p->regs.BSRR>>8)&0x01, (p->regs.BSRR>>7)&0x01, (p->regs.BSRR>>6)&0x01 );
  	GPIO_TRACE( stm, LOG_TRACE, " 5[%d] 4[%d] 3[%d] 2[%d]", (p->regs.BSRR>>5)&0x01, (p->regs.BSRR>>4)&0x01, (p->regs.BSRR>>3)&0x01, (p->regs.BSRR>>2)&0x01 );
  	GPIO_TRACE( stm, LOG_TRACE, " 1[%d] 0[%d]", (p->regs.BSRR>>1)&0x01, (p->regs.BSRR)&0x01 );

  	// LCKR
  	GPIO_TRACE( stm, LOG_TRACE, "\nLCKR[%08X]    16[%d] 15[%d] 14[%d]", p->regs.LCKR, (p->regs.LCKR>>16)&0x01, (p->regs.LCKR>>15)&0x01, (p->regs.LCKR>>14)&0x01 );
  	GPIO_TRACE( stm, LOG_TRACE, " 13[%d] 12[%d] 11[%d] 10[%d]", (p->regs.LCKR>>13)&0x01, (p->regs.LCKR>>12)&0x01, (p->regs.LCKR>>11)&0x01, (p->regs.LCKR>>10)&0x01 );
  	GPIO_TRACE( stm, LOG_TRACE, " 9[%d] 8[%d] 7[%d] 6[%d]", (p->regs.LCKR>>9)&0x01, (p->regs.LCKR>>8)&0x01, (p->regs.LCKR>>7)&0x01, (p->regs.LCKR>>6)&0x01 );
  	GPIO_TRACE( stm, LOG_TRACE, " 5[%d] 4[%d] 3[%d] 2[%d]", (p->regs.LCKR>>5)&0x01, (p->regs.LCKR>>4)&0x01, (p->regs.LCKR>>3)&0x01, (p->regs.LCKR>>2)&0x01 );
  	GPIO_TRACE( stm, LOG_TRACE, " 1[%d] 0[%d]", (p->regs.LCKR>>1)&0x01, (p->regs.LCKR)&0x01 );

  	// AFRL
  	GPIO_TRACE( stm, LOG_TRACE, "\nAFRL[%08X]    A7[%d] A6[%d]", p->regs.AFR[0], (p->regs.AFR[0]>>28)&0x0f, (p->regs.AFR[0]>>24)&0x0f );
  	GPIO_TRACE( stm, LOG_TRACE, " A5[%d] A4[%d] A3[%d] A2[%d]", (p->regs.AFR[0]>>20)&0x0f, (p->regs.AFR[0]>>16)&0x0f, (p->regs.AFR[0]>>12)&0x0f, (p->regs.AFR[0]>>8)&0x0f );
  	GPIO_TRACE( stm, LOG_TRACE, " A1[%d] A0[%d]", (p->regs.AFR[0]>>4)&0x0f, (p->regs.AFR[0])&0x0f );

  	// AFRH
  	GPIO_TRACE( stm, LOG_TRACE, "\nAFRH[%08X]    A15[%d] A14[%d]", p->regs.AFR[1], (p->regs.AFR[1]>>28)&0x0f, (p->regs.AFR[1]>>24)&0x0f );
  	GPIO_TRACE( stm, LOG_TRACE, " A13[%d] A12[%d] A11[%d] A10[%d]", (p->regs.AFR[1]>>20)&0x0f, (p->regs.AFR[1]>>16)&0x0f, (p->regs.AFR[1]>>12)&0x0f, (p->regs.AFR[1]>>8)&0x0f );
  	GPIO_TRACE( stm, LOG_TRACE, " A9[%d] A8[%d]", (p->regs.AFR[1]>>4)&0x0f, (p->regs.AFR[1])&0x0f );

  	// BRR
  	GPIO_TRACE( stm, LOG_TRACE, "\nBRR[%08X]     15[%d] 14[%d]", p->regs.BRR, (p->regs.BRR>>15)&0x01, (p->regs.BRR>>14)&0x01 );
  	GPIO_TRACE( stm, LOG_TRACE, " 13[%d] 12[%d] 11[%d] 10[%d]", (p->regs.BRR>>13)&0x01, (p->regs.BRR>>12)&0x01, (p->regs.BRR>>11)&0x01, (p->regs.BRR>>10)&0x01 );
  	GPIO_TRACE( stm, LOG_TRACE, " 9[%d] 8[%d] 7[%d] 6[%d]", (p->regs.BRR>>9)&0x01, (p->regs.BRR>>8)&0x01, (p->regs.BRR>>7)&0x01, (p->regs.BRR>>6)&0x01 );
  	GPIO_TRACE( stm, LOG_TRACE, " 5[%d] 4[%d] 3[%d] 2[%d]", (p->regs.BRR>>5)&0x01, (p->regs.BRR>>4)&0x01, (p->regs.BRR>>3)&0x01, (p->regs.BRR>>2)&0x01 );
  	GPIO_TRACE( stm, LOG_TRACE, " 1[%d] 0[%d]", (p->regs.BRR>>1)&0x01, (p->regs.BRR)&0x01 );
	}
}


void pins_l031K6_LQFP32_layout_init( struct mcu_t * mcu, struct stm_gpio_t* port, char* port_name )
{
  char pinNameBuf[ 5 ];

  if( strcmp( port_name, "PA" )== 0 )
  {

  }else if( strcmp( port_name, "PB" )== 0 )
  {
    // Pins
    for( uint32_t i=0; i<16; i++ )
    {
      sprintf( pinNameBuf, "PB%02i", i );
      strncpy( port->pins[ i ].name, pinNameBuf, 4 );
      port->pins[ i ].mcu                 = mcu;
      port->pins[ i ].port                = port;
      port->pins[ i ].pin_num             = i;
      port->pins[ i ].ext_cb_pool         = NULL;
      port->pins[ i ].al_cb_pool          = NULL;
      port->pins[ i ].an_cb_pool          = NULL;
      port->pins[ i ].state               = stLayout;
      port->pins[ i ].mode                = Undefined;
      port->pins[ i ].val                 = 0;
      port->pins[ i ].val_type            = vtUndefined;
      // ODR source value  preset
      port->pins[ i ].odr.val             = 0;
      port->pins[ i ].odr.val_type        = vtDigital;
      port->pins[ i ].odr.is_active       = 0;
      // External source value  preset
      port->pins[ i ].ext.val             = 0;
      port->pins[ i ].ext.val_type        = vtUndefined;
      port->pins[ i ].ext.is_active       = 0;
      // Alternate source value preset
      port->pins[ i ].alternate.val       = 0;
      port->pins[ i ].alternate.val_type  = vtUndefined;
      port->pins[ i ].alternate.is_active = 0;
      // Analog source preset
      port->pins[ i ].analog.val          = 0;
      port->pins[ i ].analog.val_type     = vtUndefined;
      port->pins[ i ].analog.is_active    = 0;
    }

    // Set Unused pins
    for( uint32_t i=8; i<16; i++ )
      port->pins[ i ].state               = stUnused;

  }else if( strcmp( port_name, "PC" )== 0 )
  {

  }
}

struct stm_pin_t* get_pin_by_name( struct stm_gpio_t* port, char* name )
{
  for (int idx = 0; idx <16 ; ++idx) {
    if( port->pins[ idx ].state != stUnused )
      if( strcmp( port->pins[ idx ].name, name )== 0 )
        return &( port->pins[ idx ] );
  }
  return NULL;
}

/*
 *
 */
struct stm_gpio_t* gpio_circuit_fabric( struct mcu_t * mcu )
{
	if( !mcu ) return 0x00;

	struct stm_gpio_t* p = malloc( sizeof( stm_gpio_t ));
	if( p )
	{
	  memset( p, 0x00, sizeof( stm_gpio_t ));

	  strcpy( p->gpioPortName, "PB" );

		p->cir_cb_ifs = malloc( sizeof( stm_circuit_ifs_t ));
		if( p->cir_cb_ifs )
		{
		  memset( p->cir_cb_ifs, 0x00, sizeof( stm_circuit_ifs_t ));
			p->cir_cb_ifs->init					= stm_gpio_init;
			p->cir_cb_ifs->reset				= stm_gpio_reset;
			p->cir_cb_ifs->release			= stm_gpio_release;
			p->cir_cb_ifs->wr_cb_reg		= stm_gpio_wr_cb_reg;
			p->cir_cb_ifs->rd_cb_reg		= stm_gpio_rd_cb_reg;
			p->cir_cb_ifs->write				= stm_gpio_write;
			p->cir_cb_ifs->read					= stm_gpio_read;
			p->cir_cb_ifs->dump					= stm_gpio_dump_state;
		}

		custom_pins_layout = pins_l031K6_LQFP32_layout_init;

    custom_pins_layout( mcu, p, "PB" );
	}

	p->mcu = mcu;
	return p;
}


