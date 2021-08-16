/*
 * simGpioTests.cpp
 *
 *  Created on: 10 θών. 2021 γ.
 *      Author: art_m
 */
#include "cute.h"
#include "ide_listener.h"
#include "xml_listener.h"
#include "cute_runner.h"

#include "sim_inc.h"
#include "sim_hex.h"
#include <time.h>
#include "m0pi_tdd.h"


uint32_t testVal = 0;
stm_pin_value_type_t testValType = vtUndefined;


static void pin_change_state_cb( struct stm_pin_t* pin )
{
  testVal     = pin->val;
  testValType = pin->val_type;


//  if( f_ext_log ){
//
//    uint64_t uTimeTmp = pin->mcu->core->cycle * pin->mcu->core->nano_ticks_per_cycle;
//    uint32_t uNanoSeconds = uTimeTmp%1000;
//    uTimeTmp /= 1000;
//    uint32_t uMicroSeconds = uTimeTmp%1000;
//    uTimeTmp /= 1000;
//    uint32_t uMiliSeconds = uTimeTmp%1000;
//    uint64_t uSeconds = uTimeTmp/1000;
//
//    char pBuff[1000];
//
//    switch( val ){
//      case PIN_FLOATING_VALUE:
//        sprintf( pBuff, "[%I64u.%03d.%03d.%03d] %s[ UNDEF ]\n", uSeconds, uMiliSeconds, uMicroSeconds, uNanoSeconds ,pin->name );
//        break;
//
//      case PIN_COLISION_VALUE:
//        sprintf( pBuff, "[%I64u.%03d.%03d.%03d] %s[ COLISION ]\n", uSeconds, uMiliSeconds, uMicroSeconds, uNanoSeconds ,pin->name );
//        break;
//
//      case PIN_DIGITAL_ONE_VALUE:
//        sprintf( pBuff, "[%I64u.%03d.%03d.%03d] %s[ 1 ]\n", uSeconds, uMiliSeconds, uMicroSeconds, uNanoSeconds ,pin->name );
//        break;
//
//      case PIN_DIGITAL_ZERO_VALUE:
//        sprintf( pBuff, "[%I64u.%03d.%03d.%03d] %s[ 0 ]\n", uSeconds, uMiliSeconds, uMicroSeconds, uNanoSeconds ,pin->name );
//        break;
//
//      default:
//        break;
//    }
//
//    fputs( pBuff, f_ext_log );
//  }
}



void simGpioTest()
{
  // mcu init
  char pMMCU[] = "stm32l031";

  // stm_core_fabric
  mcu_t* mcu = stm_mcu_circuit_fabric( pMMCU );
  ASSERT(mcu);

  mcu->log = LOG_DEBUG;

  // Init MCU
  mcu->mcu_ifs->init( mcu );

  // Install test callback
  mcu->portB->pins[ 3 ].ctrl.pfn_set_pin_ext_cb( &(mcu->portB->pins[ 3 ]), pin_change_state_cb );

  // Check all part are created
  ASSERT(mcu->portB);

  mcu->mcu_ifs->reset( mcu, RESET_POWER_PWR );

  // Check the state after reset
  ASSERT( testVal == PIN_FLOATING_VALUE );
  ASSERT( mcu->portB->regs.MODER == 0xFFFFFFFF );
  ASSERT( mcu->portB->regs.OTYPER == 0x00000000 );
  ASSERT( mcu->portB->regs.OSPEEDR == 0x00000000 );
  ASSERT( mcu->portB->regs.PUPDR == 0x00000000 );
  ASSERT( mcu->portB->regs.IDR == 0x00000000 );
  ASSERT( mcu->portB->regs.ODR == 0x00000000 );
  ASSERT( mcu->portB->regs.BSRR == 0x00000000 );
  ASSERT( mcu->portB->regs.LCKR == 0x00000000 );
  ASSERT( mcu->portB->regs.AFR[ 0 ] == 0x00000000 );
  ASSERT( mcu->portB->regs.AFR[ 1 ] == 0x00000000 );
  ASSERT( mcu->portB->regs.BRR == 0x00000000 );

  /* Test output pin */
  // Set Pin[3] to output
  stm_gpio_write( mcu, OSPEEDR_ADR, 0x00c0, 0 );
  stm_gpio_write( mcu, PUPDR_ADR, 0x40, 0 );
  stm_gpio_write( mcu, MODER_ADR, 0xffffff7f, 0 );

  ASSERT( testVal == PIN_DIGITAL_ZERO_VALUE );

  // Set Pin[3]
  stm_gpio_write( mcu, BSRR_ADR, 0x08, 0 );
  ASSERT( mcu->portB->regs.ODR == 0x08 );
  ASSERT( testVal == PIN_DIGITAL_ONE_VALUE );
  ASSERT( mcu->portB->regs.IDR == 0x08 );

  // Clear Pin[3]
  stm_gpio_write( mcu, BSRR_ADR, 0x80000, 0 );
  ASSERT( mcu->portB->regs.ODR == 0x00 );
  ASSERT( testVal == PIN_DIGITAL_ZERO_VALUE );
  ASSERT( mcu->portB->regs.IDR == 0x00 );

  /* Test input pin */
  // Set Pin[3] to input
  stm_gpio_write( mcu, MODER_ADR, 0xffffff3f, 0 );
  ASSERT( testVal == PIN_FLOATING_VALUE );
//  ASSERT( mcu->portB->regs.IDR == 0x8000 );

  // Set pin val by external system
  struct stm_pin_t* pin = get_pin_by_name( mcu->portB, (char*)"PB03");
  ASSERT( pin != NULL );
  pin->data.pfn_External_pin_value( pin, PIN_DIGITAL_ONE_VALUE, vtDigital );
  ASSERT(( mcu->portB->regs.IDR )&0x08 );
  ASSERT( testVal == PIN_DIGITAL_ONE_VALUE );

  // Clear Pin[3]
  pin->data.pfn_External_pin_value( pin, PIN_DIGITAL_ZERO_VALUE, vtDigital );
  ASSERT( testVal == PIN_DIGITAL_ZERO_VALUE );
  ASSERT(( mcu->portB->regs.IDR & 0x08 )== 0x00 );

  mcu->mcu_ifs->release( mcu, mcu );
}


