/*
 * coreREVTest.cpp
 *
 *  Created on: 10 окт. 2020 г.
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

/*
 * R[0] = 0x12345678
 * R[1] = 0x78563412
 */
void coreREVTest()
{
  char pMMCU[] = "stm32l031";

  uint32_t pFlash[]={
      0x20000420,
      0x8000009,
      0x0000BA01 // REV <Rd>,<Rm>
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


  //  * R[0] = 0x12345678
  //  * Res R[1] = 0x78563412

  pMcu->core->R[ 0 ]= 0x12345678;

  stm_callback_run_raw( pMcu );

  ASSERT( pMcu->core->state == cpu_Running );
  ASSERT( pMcu->core->pc == pc_val+2 );
  ASSERT( pMcu->core->R[ 1 ] == 0x78563412 );
}
/*
 *
 * R[0] = 0x12345678
 * R[1] = 0x34127856
 */
void coreREV16Test()
{
  char pMMCU[] = "stm32l031";

  uint32_t pFlash[]={
      0x20000420,
      0x8000009,
      0x0000BA41 // REV16 <Rd>,<Rm>
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


  //  * R[0] = 0x12345678
  //  * Res R[1] = 0x34127856
  pMcu->core->R[ 0 ]= 0x12345678;

  stm_callback_run_raw( pMcu );

  ASSERT( pMcu->core->state == cpu_Running );
  ASSERT( pMcu->core->pc == pc_val+2 );
  ASSERT( pMcu->core->R[ 1 ] == 0x34127856 );
}

/*
 * R[0] = 0x12345678
 * R[1] = 0x00007856

 * R[0] = 0x000056c8
 * R[1] = 0xffffc856
 */
void coreREVSHTest()
{
  char pMMCU[] = "stm32l031";

  uint32_t pFlash[]={
      0x20000420,
      0x8000009,
      0x0000bac1 // REV <Rd>,<Rm>
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


  // * R[0] = 0x12345678
  // * R[1] = 0x00007856
  pMcu->core->R[ 0 ]= 0x12345678;

  stm_callback_run_raw( pMcu );

  ASSERT( pMcu->core->state == cpu_Running );
  ASSERT( pMcu->core->pc == pc_val+2 );
  ASSERT( pMcu->core->R[ 1 ] == 0x00007856 );

  // * R[0] = 0x000056c8
  // * R[1] = 0xffffc856
  pMcu->mcu_ifs->reset( pMcu, RESET_POWER_PWR );

  pMcu->core->R[ 0 ]= 0x000056c8;

  stm_callback_run_raw( pMcu );

  ASSERT( pMcu->core->state == cpu_Running );
  ASSERT( pMcu->core->pc == pc_val+2 );
  ASSERT( pMcu->core->R[ 1 ] == 0xffffc856 );
}


