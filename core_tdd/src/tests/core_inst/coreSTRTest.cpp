/*
 * coreSTRH_T1Test.cpp
 *
 *  Created on: 2 окт. 2020 г.
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
; R2( 0xaa55 ) R1(0x20000000) R0( 2 )
; SRAM( 0x20000002 )= 0xaa55

 */

void coreSTRH_T1Test()
{
  char pMMCU[] = "stm32l031";

  uint32_t pFlash[]={
      0x20000420,
      0x8000009,
      0x0000520a    // STRH R2,R1,R0
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

  //  ; R2( 0xaa55 ) R1(0x20000000) R0( 2 )
  //  ; SRAM( 0x20000002 )= 0xaa55
  pMcu->core->R[ 0 ] = 2;
  pMcu->core->R[ 1 ] = 0x20000000;
  pMcu->core->R[ 2 ] = 0x1111aa55;

  // One command step
  stm_callback_run_raw( pMcu );

  ASSERT( pMcu->core->state == cpu_Running );
  ASSERT( pMcu->core->pc == pc_val+2 );

  uint16_t* pVal16 =( uint16_t* )( pMcu->core->data +pMcu->core->R[ 0 ]);
  uint32_t val=0;
  val = *pVal16;

  ASSERT( val ==( pMcu->core->R[ 2 ]&0x0000ffff ));
}

/*
; R2( 0xa5 ) R1(0x20000000) R0( 1 )
; SRAM( 0x20000001 )= 0xaa55

 */

void coreSTRB_T1Test()
{
  char pMMCU[] = "stm32l031";

  uint32_t pFlash[]={
      0x20000420,
      0x8000009,
      0x0000540a    // STRB R2,R1,R0
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

  //  ; R2( 0xa5 ) R1(0x20000000) R0( 1 )
  //  ; SRAM( 0x20000001 )= 0xa5
  pMcu->core->R[ 0 ] = 2;
  pMcu->core->R[ 1 ] = 0x20000000;
  pMcu->core->R[ 2 ] = 0x111111a5;

  // One command step
  stm_callback_run_raw( pMcu );

  ASSERT( pMcu->core->state == cpu_Running );
  ASSERT( pMcu->core->pc == pc_val+2 );

  uint8_t* pVal8 =( uint8_t* )( pMcu->core->data +pMcu->core->R[ 0 ]);
  uint32_t val=0;
  val = *pVal8;

  ASSERT( val ==( pMcu->core->R[ 2 ]&0x000000ff ));
}
/*
 *
 */
void coreSTRH_im_T1Test()
{
  char pMMCU[] = "stm32l031";

  uint32_t pFlash[]={
      0x20000420,
      0x8000009,
      0x00008041    // STRH R1,R0+imm5(1)
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

  //  ; R2( 0xaa55 ) R1(0x20000000) R0( 2 )
  //  ; SRAM( 0x20000002 )= 0xaa55
  pMcu->core->R[ 0 ] = 0x20000000;
  pMcu->core->R[ 1 ] = 0x1111aa55;

  // One command step
  stm_callback_run_raw( pMcu );

  ASSERT( pMcu->core->state == cpu_Running );
  ASSERT( pMcu->core->pc == pc_val+2 );

  uint16_t* pVal16 =( uint16_t* )( pMcu->core->data +(pMcu->core->R[ 0 ]-0x20000000)+2);
  uint32_t val=0;
  val = *pVal16;

  ASSERT( val ==( pMcu->core->R[ 1 ]&0x0000ffff ));
}
