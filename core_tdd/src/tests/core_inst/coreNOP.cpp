/*
 * coreNOP.cpp
 *
 *  Created on: 22 сент. 2020 г.
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


void coreNOPTest()
{
  char pMMCU[] = "stm32l031";

  uint32_t pFlash[]={
      0x20000420,
      0x8000009,
      0x0000BF00
  };

  // stm_core_fabric
  mcu_t* pMcu = stm_mcu_circuit_fabric( pMMCU );
  ASSERTM("MCU is NULL", pMcu);

  pMcu->log = LOG_DEBUG;

  pMcu->mcu_ifs->init( pMcu );
  ASSERTM("CORE is NULL", pMcu->core);

  stm_loadcode( pMcu, (uint8_t*)pFlash, sizeof(pFlash), 0x00000000 );

  pMcu->mcu_ifs->reset( pMcu, RESET_POWER_PWR );
  uint32_t aspr_val = pMcu->core->APSR.val;
  uint32_t pc_val = pMcu->core->pc;

  stm_callback_run_raw( pMcu );

  ASSERT( pMcu->core->state == cpu_Running );
  ASSERT( pMcu->core->APSR.val == aspr_val );
  ASSERT( pMcu->core->pc == pc_val+2 );
}


