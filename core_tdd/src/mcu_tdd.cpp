/*
 * mcu_tdd.cpp
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

void mcuCreateAndInitTest()
{
  // mcu init
  char pMMCU[] = "stm32l031";

  // stm_core_fabric
  mcu_t* pMcu = stm_mcu_circuit_fabric( pMMCU );
  ASSERT(pMcu);

  pMcu->log = LOG_DEBUG;

  // Init MCU
  pMcu->mcu_ifs->init( pMcu );
  // Check all part are created
  ASSERT(pMcu->core);
  ASSERT(pMcu->flash);
  ASSERT(pMcu->nvic);
  ASSERT(pMcu->scb);
  ASSERT(pMcu->sram);
  ASSERT(pMcu->systick);
  ASSERT(pMcu->rcc);
  ASSERT(pMcu->portB);

  pMcu->mcu_ifs->reset( pMcu, RESET_POWER_PWR );

  pMcu->mcu_ifs->release( pMcu, pMcu );
}





