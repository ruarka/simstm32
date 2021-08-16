/*
 * coreLDR_T1Test.cpp
 *
 *  Created on: 3 окт. 2020 г.
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
; R1(0x20000001) R0( 1 ) SRAM( 0x20000002 )= 0x5a
; R2( 0x5a )

; R1(0x20000001) R0( 0xFFFFFFFF ) SRAM( 0x20000000 )= 0xa5
; R2( 0xffffffa5 )
 */

void coreLDRSB_T1Test()
{
  char pMMCU[] = "stm32l031";

  uint32_t pFlash[]={
      0x20000420,
      0x8000009,
      0x0000560a    // LDRSB R2,R1,R0
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

  //  ; R1(0x20000001) R0( 1 ) SRAM( 0x20000002 )= 0x5a
  //  ; R2( 0x5a )
  pMcu->core->R[ 0 ] = 1;
  pMcu->core->R[ 1 ] = 0x20000001;
  uint8_t* pVal8 =( uint8_t* )( pMcu->core->data +(pMcu->core->R[ 1 ]-0x20000000) +pMcu->core->R[ 0 ]);
  *pVal8 = 0x05a;

//  hdump("[ART:HEX]", pMcu->core->data, 16);

  // One command step
  stm_callback_run_raw( pMcu );

  ASSERT( pMcu->core->state == cpu_Running );
  ASSERT( pMcu->core->pc == pc_val+2 );
  ASSERT( pMcu->core->R[ 2 ]==( 0x5a ));

  //  ; R1(0x20000001) R0( 0xFFFFFFFF ) SRAM( 0x20000000 )= 0xa5
  //  ; R2( 0xffffffa5 )
  pMcu->mcu_ifs->reset( pMcu, RESET_POWER_PWR );
  pc_val = pMcu->core->pc;

  pMcu->core->R[ 0 ] = 0xFFFFFFFF;
  pMcu->core->R[ 1 ] = 0x20000001;
  pVal8 =( uint8_t* )( pMcu->core->data );
  *pVal8 = 0x0a5;

  // One command step
  stm_callback_run_raw( pMcu );

  ASSERT( pMcu->core->state == cpu_Running );
  ASSERT( pMcu->core->pc == pc_val+2 );
  ASSERT( pMcu->core->R[ 2 ]==( 0xffffffa5 ));
}

/*
 *
 */
void coreLDRH_T1Test()
{
  char pMMCU[] = "stm32l031";

  uint32_t pFlash[]={
      0x20000420,
      0x8000009,
      0x00005a0a    // LDRH R2,R1,R0
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

  //  ; R1(0x20000001) R0( 1 ) SRAM( 0x20000002 )= 0x5a
  //  ; R2( 0x5a )
  pMcu->core->R[ 0 ] = 1;
  pMcu->core->R[ 1 ] = 0x20000001;
  uint16_t* pVal16 =( uint16_t* )( pMcu->core->data +(pMcu->core->R[ 1 ]-0x20000000) +pMcu->core->R[ 0 ]);
  *pVal16 = 0x5a5a;

  //  hdump("[ART:HEX]", pMcu->core->data, 16);

  // One command step
  stm_callback_run_raw( pMcu );

  ASSERT( pMcu->core->state == cpu_Running );
  ASSERT( pMcu->core->pc == pc_val+2 );
  ASSERT( pMcu->core->R[ 2 ]==( 0x5a5a ));

  //  ; R1(0x20000001) R0( 0xFFFFFFFF ) SRAM( 0x20000000 )= 0xa5
  //  ; R2( 0xffffffa5 )
  pMcu->mcu_ifs->reset( pMcu, RESET_POWER_PWR );
  pc_val = pMcu->core->pc;

  pMcu->core->R[ 0 ] = 0xFFFFFFFF;
  pMcu->core->R[ 1 ] = 0x20000001;
  pVal16 =( uint16_t* )( pMcu->core->data );
  *pVal16 = 0x0a5a5;

  // One command step
  stm_callback_run_raw( pMcu );

  ASSERT( pMcu->core->state == cpu_Running );
  ASSERT( pMcu->core->pc == pc_val+2 );
  ASSERT( pMcu->core->R[ 2 ]==( 0x0a5a5 ));
}

void coreLDRSH_T1Test()
{
  char pMMCU[] = "stm32l031";

  uint32_t pFlash[]={
      0x20000420,
      0x8000009,
      0x00005e0a    // LDRSH R2,R1,R0
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

  //  ; R1(0x20000001) R0( 1 ) SRAM( 0x20000002 )= 0x5a5a
  //  ; R2( 0x5a5a )
  pMcu->core->R[ 0 ] = 1;
  pMcu->core->R[ 1 ] = 0x20000001;
  uint16_t* pVal16 =( uint16_t* )( pMcu->core->data +(pMcu->core->R[ 1 ]-0x20000000) +pMcu->core->R[ 0 ]);
  *pVal16 = 0x5a5a;

  //  hdump("[ART:HEX]", pMcu->core->data, 16);

  // One command step
  stm_callback_run_raw( pMcu );

  ASSERT( pMcu->core->state == cpu_Running );
  ASSERT( pMcu->core->pc == pc_val+2 );
  ASSERT( pMcu->core->R[ 2 ]==( 0x5a5a ));

  //  ; R1(0x20000001) R0( 0xFFFFFFFF ) SRAM( 0x20000000 )= 0x0a5a5
  //  ; R2( 0xffffa5a5 )
  pMcu->mcu_ifs->reset( pMcu, RESET_POWER_PWR );
  pc_val = pMcu->core->pc;

  pMcu->core->R[ 0 ] = 0xFFFFFFFF;
  pMcu->core->R[ 1 ] = 0x20000001;
  pVal16 =( uint16_t* )( pMcu->core->data );
  *pVal16 = 0x0a5a5;

  // One command step
  stm_callback_run_raw( pMcu );

  ASSERT( pMcu->core->state == cpu_Running );
  ASSERT( pMcu->core->pc == pc_val+2 );
  ASSERT( pMcu->core->R[ 2 ]==( 0xffffa5a5 ));
}

/*
 *
 */
void coreLDRH_im_T1Test()
{
  char pMMCU[] = "stm32l031";

  uint32_t pFlash[]={
      0x20000420,
      0x8000009,
      0x00008841    // LDRH R1,(R0+imm5)
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

  //  ; R1(0x20000001) R0( 0x20000000 ) SRAM( 0x20000002 )= 0x5a5a
  //  ; R2( 0x5a5a )
  pMcu->core->R[ 0 ] = 0x20000000;
  //  pMcu->core->R[ 1 ] = 0x5a5a;
  uint16_t* pVal16 =( uint16_t* )( pMcu->core->data +(pMcu->core->R[ 0 ]-0x20000000) +2);
  *pVal16 = 0x5a5a;

  //hdump("[ART:HEX]", pMcu->core->data, 16);

  // One command step
  stm_callback_run_raw( pMcu );

  ASSERT( pMcu->core->state == cpu_Running );
  ASSERT( pMcu->core->pc == pc_val+2 );
  ASSERT( pMcu->core->R[ 1 ]==( 0x5a5a ));
}
