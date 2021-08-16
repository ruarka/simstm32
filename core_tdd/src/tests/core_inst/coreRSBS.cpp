/*
 * coreRSBS.cpp
 *
 *  Created on: 21 сент. 2020 г.
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
 * RSBS r0,r0, #0
 *
; ASPR(NZCV)_0000
        EORS    r0,r0
        MSR   APSR,r0
; 1) #0 -1
        LDR     R0,|One|
        RSBS  R0, R0, #0
; result ASPR(NZCV)_1000

; 2) #0 - 0xFFFFFFFF
        LDR     R0,|mOne|
        RSBS  R0, R0, #0
; result ASPR(NZCV)_0000

; 3) #0 -0
        LDR     R0,|Zero|
        RSBS  R0, R0, #0
; result ASPR(NZCV)_0110


; ASPR(NZCV)_1111
        LDR     R0,|APSR_1111|
        MSR   APSR,r0
; 4) #0 -1
        LDR     R0,|One|
        RSBS  R0, R0, #0
; result ASPR(NZCV)_1000

; ASPR(NZCV)_1111
        LDR     R0,|APSR_1111|
        MSR   APSR,r0
; 5) #0 - 0xFFFFFFFF
        LDR     R0,|mOne|
        RSBS  R0, R0, #0
; result ASPR(NZCV)_0000

; ASPR(NZCV)_1111
        LDR     R0,|APSR_1111|
        MSR   APSR,r0
; 6) #0 -0
        LDR     R0,|Zero|
        RSBS  R0, R0, #0
; result ASPR(NZCV)_0110
*/

void coreRSBSTest()
{
  // stm_core_fabric
  mcu_t* pMcu = coreInitWithInstruction( 0x4240 );  // RSBS r0,r0,#0

   uint32_t pc_val = pMcu->core->pc;

//  ; ASPR(NZCV)_0000
//          EORS    r0,r0
//          MSR   APSR,r0
//  ; 1) #0 -1
//          LDR     R0,|One|
//          RSBS  R0, R0, #0
//  ; result ASPR(NZCV)_1000
  pMcu->core->APSR.val = 0x00;
  pMcu->core->R[ 0 ] = 1;

  // One command step
  stm_callback_run_raw( pMcu );

  ASSERT( pMcu->core->state == cpu_Running );
  ASSERT( pMcu->core->APSR.bits.N == 1 );
  ASSERT( pMcu->core->APSR.bits.Z == 0 );
  ASSERT( pMcu->core->APSR.bits.C == 0 );
  ASSERT( pMcu->core->APSR.bits.V == 0 );
  ASSERT( pMcu->core->R[ 0 ] == 0xFFFFFFFF );
  ASSERT( pMcu->core->pc == pc_val+2 );

//  ; 2) #0 - 0xFFFFFFFF
//          LDR     R0,|mOne|
//          RSBS  R0, R0, #0
//  ; result ASPR(NZCV)_0000
  pMcu->mcu_ifs->reset( pMcu, RESET_POWER_PWR );

  pMcu->core->R[ 0 ] = 0xFFFFFFFF;

  // One command step
  stm_callback_run_raw( pMcu );

  ASSERT( pMcu->core->state == cpu_Running );
  ASSERT( pMcu->core->APSR.bits.N == 0 );
  ASSERT( pMcu->core->APSR.bits.Z == 0 );
  ASSERT( pMcu->core->APSR.bits.C == 0 );
  ASSERT( pMcu->core->APSR.bits.V == 0 );
  ASSERT( pMcu->core->R[ 0 ] == 0x01 );
  ASSERT( pMcu->core->pc == pc_val+2 );

//  ; 3) #0 -0
//          LDR     R0,|Zero|
//          RSBS  R0, R0, #0
//  ; result ASPR(NZCV)_0110
  pMcu->mcu_ifs->reset( pMcu, RESET_POWER_PWR );

  pMcu->core->R[ 0 ] = 0x00;

  // One command step
  stm_callback_run_raw( pMcu );

  ASSERT( pMcu->core->state == cpu_Running );
  ASSERT( pMcu->core->APSR.bits.N == 0 );
  ASSERT( pMcu->core->APSR.bits.Z == 1 );
  ASSERT( pMcu->core->APSR.bits.C == 1 );
  ASSERT( pMcu->core->APSR.bits.V == 0 );
  ASSERT( pMcu->core->R[ 0 ] == 0x00 );
  ASSERT( pMcu->core->pc == pc_val+2 );

//  ; ASPR(NZCV)_1111
//          LDR     R0,|APSR_1111|
//          MSR   APSR,r0
//  ; 4) #0 -1
//          LDR     R0,|One|
//          RSBS  R0, R0, #0
//  ; result ASPR(NZCV)_1000
  pMcu->mcu_ifs->reset( pMcu, RESET_POWER_PWR );

  pMcu->core->APSR.val = 0xF0000000;
  pMcu->core->R[ 0 ] = 1;

  // One command step
  stm_callback_run_raw( pMcu );

  ASSERT( pMcu->core->state == cpu_Running );
  ASSERT( pMcu->core->APSR.bits.N == 1 );
  ASSERT( pMcu->core->APSR.bits.Z == 0 );
  ASSERT( pMcu->core->APSR.bits.C == 0 );
  ASSERT( pMcu->core->APSR.bits.V == 0 );
  ASSERT( pMcu->core->R[ 0 ] == 0xFFFFFFFF );
  ASSERT( pMcu->core->pc == pc_val+2 );

//  ; ASPR(NZCV)_1111
//          LDR     R0,|APSR_1111|
//          MSR   APSR,r0
//  ; 5) #0 - 0xFFFFFFFF
//          LDR     R0,|mOne|
//          RSBS  R0, R0, #0
//  ; result ASPR(NZCV)_0000
  pMcu->mcu_ifs->reset( pMcu, RESET_POWER_PWR );

  pMcu->core->APSR.val = 0xF0000000;
  pMcu->core->R[ 0 ] = 0xFFFFFFFF;

  // One command step
  stm_callback_run_raw( pMcu );

  ASSERT( pMcu->core->state == cpu_Running );
  ASSERT( pMcu->core->APSR.bits.N == 0 );
  ASSERT( pMcu->core->APSR.bits.Z == 0 );
  ASSERT( pMcu->core->APSR.bits.C == 0 );
  ASSERT( pMcu->core->APSR.bits.V == 0 );
  ASSERT( pMcu->core->R[ 0 ] == 0x01 );
  ASSERT( pMcu->core->pc == pc_val+2 );

//  ; ASPR(NZCV)_1111
//          LDR     R0,|APSR_1111|
//          MSR   APSR,r0
//  ; 6) #0 -0
//          LDR     R0,|Zero|
//          RSBS  R0, R0, #0
//  ; result ASPR(NZCV)_0110
  pMcu->mcu_ifs->reset( pMcu, RESET_POWER_PWR );

  pMcu->core->APSR.val = 0xF0000000;
  pMcu->core->R[ 0 ] = 0x00;

  // One command step
  stm_callback_run_raw( pMcu );

  ASSERT( pMcu->core->state == cpu_Running );
  ASSERT( pMcu->core->APSR.bits.N == 0 );
  ASSERT( pMcu->core->APSR.bits.Z == 1 );
  ASSERT( pMcu->core->APSR.bits.C == 1 );
  ASSERT( pMcu->core->APSR.bits.V == 0 );
  ASSERT( pMcu->core->R[ 0 ] == 0x00 );
  ASSERT( pMcu->core->pc == pc_val+2 );

  // Free mcu
  if( pMcu )
  {
    pMcu->mcu_ifs->release( pMcu, pMcu );
    free( pMcu );
  }

}
