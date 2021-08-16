/*
 * coreCMP_T2Test.cpp
 *
 *  Created on: 1 окт. 2020 г.
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
; R8( |80..1| ) R0( |80..1| )
; ASPR(NZCV)_0011
; 0x00000002

; R8( 2 ) R0( 0xFFFFFFFF )
; ASPR(NZCV)_0010

; R8( 0 ) R0( 1 )
; ASPR(NZCV)_0000

; R8(0xFFFFFFFF) R0( 1 )
; ASPR(NZCV)_0110
; 0x00000000

; R8(1) R0(1)
; ASPR(NZCV)_0000
; 0x00000002

; R18( 0xFFFFFFFF ) R0( 0xFFFFFFFF )
; ASPR(NZCV)_1010
; 0xFFFFFFFE

; R8(0xFFFFFFFF) R0( 0x7FFFFFFF )
; ASPR(NZCV)_0010
; 0x7FFFFFFE

; R8(0x7FFFFFFF) R0(0x7FFFFFFF)
; ASPR(NZCV)_1001
; 0xFFFFFFFE
*/


void coreCMP_T2Test()
{
  // 118 - A6.7.18 CMP (register) T2
  char pMMCU[] = "stm32l031";

  uint32_t pFlash[]={
      0x20000420,
      0x8000009,
      0x00004540    // CMP <Rn>,<Rm>
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

//  ; R8( |80..1| ) R0( |80..1| )
//  ; ASPR(NZCV)_
//  ; 0x00000002
  pMcu->core->APSR.val = 0x00;
  pMcu->core->R[ 0 ] = 0x80000001;
  pMcu->core->R[ 8 ] = 0x80000001;

  // One command step
  stm_callback_run_raw( pMcu );

  ASSERT( pMcu->core->state == cpu_Running );
  ASSERT( pMcu->core->APSR.bits.N == 0 );
  ASSERT( pMcu->core->APSR.bits.Z == 1 );
  ASSERT( pMcu->core->APSR.bits.C == 1 );
  ASSERT( pMcu->core->APSR.bits.V == 0 );
  ASSERT( pMcu->core->pc == pc_val+2 );

//  ; R8( 2 ) R0( 0xFFFFFFFF )
//  ; ASPR(NZCV)_0010
  pMcu->mcu_ifs->reset( pMcu, RESET_POWER_PWR );

  pMcu->core->R[ 0 ] = 0xFFFFFFFF;
  pMcu->core->R[ 8 ] = 2;

  // One command step
  stm_callback_run_raw( pMcu );

  ASSERT( pMcu->core->state == cpu_Running );
  ASSERT( pMcu->core->APSR.bits.N == 1 );
  ASSERT( pMcu->core->APSR.bits.Z == 0 );
  ASSERT( pMcu->core->APSR.bits.C == 1 );
  ASSERT( pMcu->core->APSR.bits.V == 0 );
  ASSERT( pMcu->core->pc == pc_val+2 );

//  ; R8( 0 ) R0( 1 )
//  ; ASPR(NZCV)_0000
  pMcu->mcu_ifs->reset( pMcu, RESET_POWER_PWR );

  pMcu->core->R[ 0 ] = 1;
  pMcu->core->R[ 8 ] = 0;

  // One command step
  stm_callback_run_raw( pMcu );

  ASSERT( pMcu->core->state == cpu_Running );
  ASSERT( pMcu->core->APSR.bits.N == 0 );
  ASSERT( pMcu->core->APSR.bits.Z == 0 );
  ASSERT( pMcu->core->APSR.bits.C == 1 );
  ASSERT( pMcu->core->APSR.bits.V == 0 );
  ASSERT( pMcu->core->pc == pc_val+2 );

//  ; R8(0xFFFFFFFF) R0( 1 )
//  ; ASPR(NZCV)_0110
//  ; 0x00000000
  pMcu->mcu_ifs->reset( pMcu, RESET_POWER_PWR );

  pMcu->core->R[ 8 ] = 0xFFFFFFFF;
  pMcu->core->R[ 0 ] = 1;

  // One command step
  stm_callback_run_raw( pMcu );

  ASSERT( pMcu->core->state == cpu_Running );
  ASSERT( pMcu->core->APSR.bits.N == 0 );
  ASSERT( pMcu->core->APSR.bits.Z == 0 );
  ASSERT( pMcu->core->APSR.bits.C == 0 );
  ASSERT( pMcu->core->APSR.bits.V == 0 );
  ASSERT( pMcu->core->pc == pc_val+2 );

//  ; R8(1) R0(1)
//  ; ASPR(NZCV)_0000
//  ; 0x00000002
  pMcu->mcu_ifs->reset( pMcu, RESET_POWER_PWR );

  pMcu->core->R[ 0 ] = 1;
  pMcu->core->R[ 8 ] = 1;

  // One command step
  stm_callback_run_raw( pMcu );

  ASSERT( pMcu->core->state == cpu_Running );
  ASSERT( pMcu->core->APSR.bits.N == 0 );
  ASSERT( pMcu->core->APSR.bits.Z == 1 );
  ASSERT( pMcu->core->APSR.bits.C == 1 );
  ASSERT( pMcu->core->APSR.bits.V == 0 );
  ASSERT( pMcu->core->pc == pc_val+2 );

//  ;  R8( 0xFFFFFFFF ) R0( 0xFFFFFFFF )
//  ; ASPR(NZCV)_1010
//  ; 0xFFFFFFFE
  pMcu->mcu_ifs->reset( pMcu, RESET_POWER_PWR );

  pMcu->core->R[ 0 ] = 0xFFFFFFFF;
  pMcu->core->R[ 8 ] = 0xFFFFFFFF;

  // One command step
  stm_callback_run_raw( pMcu );

  ASSERT( pMcu->core->state == cpu_Running );
  ASSERT( pMcu->core->APSR.bits.N == 0 );
  ASSERT( pMcu->core->APSR.bits.Z == 1 );
  ASSERT( pMcu->core->APSR.bits.C == 1 );
  ASSERT( pMcu->core->APSR.bits.V == 0 );
  ASSERT( pMcu->core->pc == pc_val+2 );

//  ; R8(0xFFFFFFFF) R0( 0x7FFFFFFF )
//  ; ASPR(NZCV)_0010
//  ; 0x7FFFFFFE
  pMcu->mcu_ifs->reset( pMcu, RESET_POWER_PWR );

  pMcu->core->R[ 0 ] = 0x7FFFFFFF;
  pMcu->core->R[ 8 ] = 0xFFFFFFFF;

  // One command step
  stm_callback_run_raw( pMcu );

  ASSERT( pMcu->core->state == cpu_Running );
  ASSERT( pMcu->core->APSR.bits.N == 1 );
  ASSERT( pMcu->core->APSR.bits.Z == 0 );
  ASSERT( pMcu->core->APSR.bits.C == 0 );
  ASSERT( pMcu->core->APSR.bits.V == 1 );
  ASSERT( pMcu->core->pc == pc_val+2 );

//  ; R8(0x7FFFFFFF) R0(0x7FFFFFFF)
//  ; ASPR(NZCV)_1001
//  ; 0xFFFFFFFE
  pMcu->mcu_ifs->reset( pMcu, RESET_POWER_PWR );

  pMcu->core->R[ 0 ] = 0x7FFFFFFF;
  pMcu->core->R[ 8 ] = 0x7FFFFFFF;

  // One command step
  stm_callback_run_raw( pMcu );

  ASSERT( pMcu->core->state == cpu_Running );
  ASSERT( pMcu->core->APSR.bits.N == 0 );
  ASSERT( pMcu->core->APSR.bits.Z == 1 );
  ASSERT( pMcu->core->APSR.bits.C == 1 );
  ASSERT( pMcu->core->APSR.bits.V == 0 );
  ASSERT( pMcu->core->pc == pc_val+2 );
}

/*
 * CMP <Rn>,<Rm>
 */
void coreCMP_reg_T1Test()
{
  ASSERTM("CMP REG T1:start writing tests", false);
}

/*
 * CMN <Rn>,<Rm>
 */
void coreCMN_reg_T1Test()
{
  ASSERTM("CMN REG T1:start writing tests", false);
}

/*
 *
 */
