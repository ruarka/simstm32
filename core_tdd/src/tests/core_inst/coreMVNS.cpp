/*
 * coreMVNS.cpp
 *
 *  Created on: 27 сент. 2020 г.
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
; R0( |80..1| ) CF(0) NF(0) Z(0)
; ASPR(NZCV)_0000
; res( R1 )= 0x7FFFFFFE

; R0( |0xFFFFFFFF| ) CF(0) NF(0) Z(0)
; ASPR(NZCV)_0100
; res( R1 )= 0

; R0( |0x0| ) CF(1) NF(1) Z(1)
; ASPR(NZCV)_1011
; res( R1 )= 0xFFFFFFFF

; R0(1) CF(1) NF(1) Z(1)
; ASPR(NZCV)_1011
; res( R1 )= 0xFFFFFFFE
*/

void coreMVNSTest()
{
  char pMMCU[] = "stm32l031";

  uint32_t pFlash[]={
      0x20000420,
      0x8000009,
      0x000043C1    // MVNS R1,R0
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

  //  ; R0( |80..1| ) CF(0) NF(0) Z(0)
  //  ; ASPR(NZCV)_0000
  //  ; res( R1 )= 0x7FFFFFFE
  pMcu->core->APSR.val = 0x00;
  pMcu->core->R[ 0 ] = 0x80000001;

  // One command step
  stm_callback_run_raw( pMcu );

  ASSERT( pMcu->core->state == cpu_Running );
  ASSERT( pMcu->core->APSR.bits.N == 0 );
  ASSERT( pMcu->core->APSR.bits.Z == 0 );
  ASSERT( pMcu->core->APSR.bits.C == 0 );
  ASSERT( pMcu->core->APSR.bits.V == 0 );
  ASSERT( pMcu->core->R[ 1 ] == 0x7FFFFFFE );
  ASSERT( pMcu->core->pc == pc_val+2 );

  //  ; R0( |0xFFFFFFFF| ) CF(0) NF(0) Z(0)
  //  ; ASPR(NZCV)_0100
  //  ; res( R1 )= 0
  pMcu->mcu_ifs->reset( pMcu, RESET_POWER_PWR );

  pMcu->core->APSR.val = 0x00;
  pMcu->core->R[ 0 ] = 0xFFFFFFFF;

  // One command step
  stm_callback_run_raw( pMcu );

  ASSERT( pMcu->core->state == cpu_Running );
  ASSERT( pMcu->core->APSR.bits.N == 0 );
  ASSERT( pMcu->core->APSR.bits.Z == 1 );
  ASSERT( pMcu->core->APSR.bits.C == 0 );
  ASSERT( pMcu->core->APSR.bits.V == 0 );
  ASSERT( pMcu->core->R[ 1 ] == 0 );
  ASSERT( pMcu->core->pc == pc_val+2 );

  //  ; R0( |0x0| ) CF(1) NF(1) Z(1)
  //  ; ASPR(NZCV)_1011
  //  ; res( R1 )= 0xFFFFFFFF
  pMcu->mcu_ifs->reset( pMcu, RESET_POWER_PWR );

  pMcu->core->APSR.bits.C = 1;
  pMcu->core->APSR.bits.Z = 1;
  pMcu->core->APSR.bits.N = 1;
  pMcu->core->APSR.bits.V = 1;
  pMcu->core->R[ 0 ] = 0;

  // One command step
  stm_callback_run_raw( pMcu );

  ASSERT( pMcu->core->state == cpu_Running );
  ASSERT( pMcu->core->APSR.bits.N == 1 );
  ASSERT( pMcu->core->APSR.bits.Z == 0 );
  ASSERT( pMcu->core->APSR.bits.C == 1 );
  ASSERT( pMcu->core->APSR.bits.V == 1 );
  ASSERT( pMcu->core->R[ 1 ] == 0xFFFFFFFF );
  ASSERT( pMcu->core->pc == pc_val+2 );


  //  ; R0(1) CF(1) NF(1) Z(1)
  //  ; ASPR(NZCV)_1011
  //  ; res( R1 )= 0xFFFFFFFE
  pMcu->mcu_ifs->reset( pMcu, RESET_POWER_PWR );

  pMcu->core->APSR.bits.C = 1;
  pMcu->core->APSR.bits.Z = 1;
  pMcu->core->APSR.bits.N = 1;
  pMcu->core->APSR.bits.V = 1;
  pMcu->core->R[ 0 ] = 1;

  // One command step
  stm_callback_run_raw( pMcu );

  ASSERT( pMcu->core->state == cpu_Running );
  ASSERT( pMcu->core->APSR.bits.N == 1 );
  ASSERT( pMcu->core->APSR.bits.Z == 0 );
  ASSERT( pMcu->core->APSR.bits.C == 1 );
  ASSERT( pMcu->core->APSR.bits.V == 1 );
  ASSERT( pMcu->core->R[ 1 ] == 0xFFFFFFFE );
  ASSERT( pMcu->core->pc == pc_val+2 );
}
