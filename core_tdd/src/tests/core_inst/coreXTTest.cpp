/*
 * coreXTTest.cpp
 *
 *  Created on: 8 окт. 2020 г.
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

#include "sim_hex.h"

/*
  ; R(0)= 0x5a5a5a5a
  ; R(1)= 0x5a5a

  ; R(0)= 0xa5a5a5a5
  ; R(1)= 0xffffa5a5
*/
void coreSXTHTest()
{
  char pMMCU[] = "stm32l031";

  uint32_t pFlash[]={
      0x20000420,
      0x8000009,
      0x0000B201    // SXTH <Rd>,<Rm>
  };

  // stm_core_fabric
  mcu_t* pMcu = stm_mcu_circuit_fabric( pMMCU );
  ASSERTM("MCU is NULL", pMcu);

  pMcu->log = LOG_DEBUG;

  pMcu->mcu_ifs->init( pMcu );
  ASSERTM("CORE is NULL", pMcu->core);

  stm_loadcode( pMcu, (uint8_t*)pFlash, sizeof(pFlash), 0x00000000 );

  pMcu->mcu_ifs->reset( pMcu, RESET_POWER_PWR );
  uint32_t pc_val = pMcu->core->pc;

  //  ; R(0)= 0xa5a5a5a5
  //  ; R(1)= 0x5a5a
  pMcu->core->R[ 0 ] = 0x5a5a5a5a;

  // One command step
  stm_callback_run_raw( pMcu );

  ASSERT( pMcu->core->state == cpu_Running );
  ASSERT( pMcu->core->R[ 1 ] == 0x5a5a );
  ASSERT( pMcu->core->pc == pc_val+2 );

  //  ; R(0)= 0xa5a5a5a5
  //  ; R(1)= 0xffffa5a5
  pMcu->mcu_ifs->reset( pMcu, RESET_POWER_PWR );

  pMcu->core->R[ 0 ] = 0xa5a5a5a5;

  // One command step
  stm_callback_run_raw( pMcu );

  ASSERT( pMcu->core->state == cpu_Running );
  ASSERT( pMcu->core->R[ 1 ] == 0xffffa5a5 );
  ASSERT( pMcu->core->pc == pc_val+2 );
}

/*
  ; R(0)= 0x5a5a5a5a
  ; R(0)= 0x5a

  ; R(0)= 0xa5a5a5a5
  ; R(0)= 0xffffffa5
*/
void coreSXTBTest()
{
  char pMMCU[] = "stm32l031";

  uint32_t pFlash[]={
      0x20000420,
      0x8000009,
      0x0000B241    // SXTB <Rd>,<Rm>
  };

  // stm_core_fabric
  mcu_t* pMcu = stm_mcu_circuit_fabric( pMMCU );
  ASSERTM("MCU is NULL", pMcu);

  pMcu->log = LOG_DEBUG;

  pMcu->mcu_ifs->init( pMcu );
  ASSERTM("CORE is NULL", pMcu->core);

  stm_loadcode( pMcu, (uint8_t*)pFlash, sizeof(pFlash), 0x00000000 );

  pMcu->mcu_ifs->reset( pMcu, RESET_POWER_PWR );
  uint32_t pc_val = pMcu->core->pc;

  //  ; R(0)= 0xa5a5a5a5
  //  ; R(1)= 0x5a
  pMcu->core->R[ 0 ] = 0x5a5a5a5a;

  // One command step
  stm_callback_run_raw( pMcu );

  ASSERT( pMcu->core->state == cpu_Running );
  ASSERT( pMcu->core->R[ 1 ] == 0x5a );
  ASSERT( pMcu->core->pc == pc_val+2 );

  //  ; R(0)= 0xa5a5a5a5
  //  ; R(1)= 0xffffffa5
  pMcu->mcu_ifs->reset( pMcu, RESET_POWER_PWR );

  pMcu->core->R[ 0 ] = 0xa5a5a5a5;

  // One command step
  stm_callback_run_raw( pMcu );

  ASSERT( pMcu->core->state == cpu_Running );
  ASSERT( pMcu->core->R[ 1 ] == 0xffffffa5 );
  ASSERT( pMcu->core->pc == pc_val+2 );
}

/*
  ; R(0)= 0x5a5a5a5a
  ; R(1)= 0x5a5a

  ; R(0)= 0xa5a5a5a5
  ; R(1)= 0xa5a5
*/
void coreUXTHTest()
{
  char pMMCU[] = "stm32l031";

  uint32_t pFlash[]={
      0x20000420,
      0x8000009,
      0x0000B281    // UXTH <Rd>,<Rm>
  };

  // stm_core_fabric
  mcu_t* pMcu = stm_mcu_circuit_fabric( pMMCU );
  ASSERTM("MCU is NULL", pMcu);

  pMcu->log = LOG_DEBUG;

  pMcu->mcu_ifs->init( pMcu );
  ASSERTM("CORE is NULL", pMcu->core);

  stm_loadcode( pMcu, (uint8_t*)pFlash, sizeof(pFlash), 0x00000000 );

  pMcu->mcu_ifs->reset( pMcu, RESET_POWER_PWR );
  uint32_t pc_val = pMcu->core->pc;

  //  ; R(0)= 0x5a5a5a5a
  //  ; R(1)= 0x5a5a
  pMcu->core->R[ 0 ] = 0x5a5a5a5a;

  // One command step
  stm_callback_run_raw( pMcu );

  ASSERT( pMcu->core->state == cpu_Running );
  ASSERT( pMcu->core->R[ 1 ] == 0x5a5a );
  ASSERT( pMcu->core->pc == pc_val+2 );

  //  ; R(0)= 0xa5a5a5a5
  //  ; R(1)= 0xa5a5
  pMcu->mcu_ifs->reset( pMcu, RESET_POWER_PWR );

  pMcu->core->R[ 0 ] = 0xa5a5a5a5;

  // One command step
  stm_callback_run_raw( pMcu );

  ASSERT( pMcu->core->state == cpu_Running );
  ASSERT( pMcu->core->R[ 1 ] == 0xa5a5 );
  ASSERT( pMcu->core->pc == pc_val+2 );
}
