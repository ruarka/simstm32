/*
 * m0pi_tdd.cpp
 *
 *  Created on: 12 сент. 2020 г.
 *      Author: art_m
 */
#include "cute.h"
#include "ide_listener.h"
#include "xml_listener.h"
#include "cute_runner.h"

#include "sim_inc.h"
#include "sim_hex.h"
#include <time.h>

/* ------------------------------------------------------------------------------------------------
 *                                 Local Variables
 * ------------------------------------------------------------------------------------------------
 */

// TODO: NEG should be implemented also

void coreWFETest()
{
  ASSERTM("WFE:start writing tests", false);
}

void coreWFITest()
{
  ASSERTM("WFE:start writing tests", false);
}

void coreSEVTest()
{
  ASSERTM("SEV:start writing tests", false);
}


/* ------------------------------------------------------------------------------------------------
 *                                 Local Functions
 * ------------------------------------------------------------------------------------------------
 */
mcu_t* coreInitWithInstruction( uint32_t inst_code )
{
  char pMMCU[] = "stm32l031";

  // TODO: Correct this
  uint32_t pFlash[ 3 ]={
      0x20000420,
      0x8000009,
   };

  pFlash[ 2 ]= inst_code;

  // stm_core_fabric
  mcu_t* pMcu = stm_mcu_circuit_fabric( pMMCU );
  ASSERTM("MCU is NULL", pMcu);

  pMcu->log = LOG_DEBUG;

  pMcu->mcu_ifs->init( pMcu );
  ASSERTM("CORE is NULL", pMcu->core);

  stm_loadcode( pMcu, (uint8_t*)pFlash, sizeof(pFlash), 0x00000000 );

  pMcu->mcu_ifs->reset( pMcu, RESET_POWER_PWR );

  return pMcu;
}
