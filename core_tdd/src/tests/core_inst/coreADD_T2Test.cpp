/*
 * coreADD_T2Test.cpp
 *
 *  Created on: 28 сент. 2020 г.
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
; R1(2) R0(1)
; R1(3)

; R1(1) R0(0xFFFFFFFF)
; R1(0)
*/

/*
 * ADD <Rdn>,<Rm>
 * page 102
 */
void coreADD_T2Test()
{
  char pMMCU[] = "stm32l031";

  uint32_t pFlash[]={
      0x20000420,
      0x8000009,
      0x00004401    // ADD R1,R0
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

  //  ; R1(2) R0(1)
  //  ; R1(3)
  pMcu->core->R[ 0 ] = 1;
  pMcu->core->R[ 1 ] = 2;

  // One command step
  stm_callback_run_raw( pMcu );

  ASSERT( pMcu->core->state == cpu_Running );
  ASSERT( pMcu->core->R[ 1 ] == 3 );
  ASSERT( pMcu->core->pc == pc_val+2 );

  //  ; R1(1) R0(0xFFFFFFFF)
  //  ; R1(0)
  pMcu->mcu_ifs->reset( pMcu, RESET_POWER_PWR );

  pMcu->core->R[ 0 ] = 0xFFFFFFFF;
  pMcu->core->R[ 1 ] = 1;

  // One command step
  stm_callback_run_raw( pMcu );

  ASSERT( pMcu->core->state == cpu_Running );
  ASSERT( pMcu->core->R[ 1 ] == 0 );
  ASSERT( pMcu->core->pc == pc_val+2 );

  //  ; R14(0x08000200) R0(2)
  //  ; R0(0x08000200)
  pMcu->mcu_ifs->reset( pMcu, RESET_POWER_PWR );

  uint32_t pFlash1[]={
      0x20000420,
      0x8000009,
      0x00004470    // ADD R0,LR
  };

  stm_loadcode( pMcu, (uint8_t*)pFlash1, sizeof(pFlash), 0x00000000 );

  pMcu->core->R[ 14 ] = 0x08000200;
  pMcu->core->R[ 0 ] = 2;

  // One command step
  stm_callback_run_raw( pMcu );

  ASSERT( pMcu->core->state == cpu_Running );
  ASSERT( pMcu->core->R[ 0 ] ==( 0x08000200+2 ));
  ASSERT( pMcu->core->pc == pc_val+2 );

  //  ; SP/R13(0x20000420) R0(2)
  //  ; SP(0x20000422)
  pMcu->mcu_ifs->reset( pMcu, RESET_POWER_PWR );

  uint32_t pFlash2[]={
      0x20000420,
      0x8000009,
      0x00004485    // ADD SP,R0
  };
  stm_loadcode( pMcu, (uint8_t*)pFlash2, sizeof(pFlash), 0x00000000 );

  pMcu->core->R[ 0 ]= 2;

  // One command step
  stm_callback_run_raw( pMcu );

  ASSERT( pMcu->core->state == cpu_Running );
  ASSERT( pMcu->core->R[ 13 ] ==( 0x20000420+2 ));
  ASSERT( pMcu->core->pc == pc_val+2 );
}

/*
 ; PC imm8(0x01)
 ; R0( PC +0x04 )
 */
void coreADR_T1Test()
{
  char pMMCU[] = "stm32l031";

  uint32_t pFlash[]={
      0x20000420,
      0x8000009,
      0x0000A001    // ADR R0,imm8(0x01)
  };

  // stm_core_fabric
  mcu_t* pMcu = stm_mcu_circuit_fabric( pMMCU );
  ASSERTM("MCU is NULL", pMcu);

  pMcu->log = LOG_DEBUG;

  pMcu->mcu_ifs->init( pMcu );
  ASSERTM("CORE is NULL", pMcu->core);

  stm_loadcode( pMcu, (uint8_t*)pFlash, sizeof(pFlash), 0x00000000 );

  pMcu->mcu_ifs->reset( pMcu, RESET_POWER_PWR );

  // ; PC imm8(0x01)
  // ; R0( PC +0x04 )
  uint32_t pc_val = pMcu->core->pc;

  // One command step
  stm_callback_run_raw( pMcu );

  ASSERT( pMcu->core->state == cpu_Running );
  ASSERT( pMcu->core->R[ 0 ] == (( pc_val &0xFFFFFFFC )+( 0x01<<2 )));
  ASSERT( pMcu->core->pc == pc_val+2 );
}

/*
 * Opcode(1C40) ADDS r0,r0,#1
 */
void coreADDS_imm3_T1Test()
{
  ASSERTM("ADDS T1:start writing tests", false);
}

void coreADDS_imm8_T2Test()
{
  ASSERTM("ADDS T2:start writing tests", false);
}

/*
 * ADCS <Rdn>,<Rm>
 */
void coreADCS_T1Test()
{
  ASSERTM("ADCS T1:start writing tests", false);
}
