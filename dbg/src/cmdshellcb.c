/*
 * cmdshellcb.c
 *
 *  Created on: 18 oct 2020 y
 *      Author: art_m
 */
#include <stdio.h>
#include <stdlib.h>
#include <libgen.h>
#include <string.h>

#include "sim_inc.h"

#include "sim_hex.h"
#include "cmdshell.h"

extern char* pTestHexFile;

mcu_t* mcu = 0;
stm_t* stm = 0;
uint8_t* flash = 0;
uint32_t fsize, fstart, fstart_linear_address;

/*
 * Commands Shell Callbacks
 */
/*
 *
 */
uint32_t cmd_new_cb( int args, char** params )
{
  char pMMCU[] = "stm32l031";

  if( mcu )
  {
    printf( "\nTry to release mcu %s", pMMCU );
    mcu->mcu_ifs->release( mcu, mcu );
    printf( "\nmcu was released" );
    free( mcu );
  }

  // stm_core_fabric
  mcu = stm_mcu_circuit_fabric( pMMCU );
  if (!mcu )
  {
    fprintf(stderr, "\nErr:new: not created-%s\n", pMMCU );
//    exit(1);
  }else
  {
    printf( "\nnew MCU for %s was created", pMMCU );
  }

  return 0x00;
}
/*
 *
 */
uint32_t cmd_nir_cb( int args, char** params )
{
  cmd_new_cb( args, params );

  mcu->mcu_ifs->init( mcu );

  if( !flash )
  {
    flash =  read_ihex_file(
        pTestHexFile,
        &fsize,
        &fstart,
        &fstart_linear_address);

    printf("\nFLASH buffer was loaded form %s", pTestHexFile);
  }

  stm_loadcode( mcu, flash, fsize, 0x00000000 );

  mcu->mcu_ifs->reset( mcu, RESET_SYSTEM_NSRT );

  printf( "\n\nflash_start=%x", mcu->core->flash_start_adr );
  printf( "\nflash_end=%x", mcu->core->flash_end_adr );
  printf( "\nstart_linear_address=%x", mcu->core->start_linear_address );

  printf( "\n\nreset_pc=%08x", mcu->core->reset_pc );
  printf( "\nreset_sp=%08x\n\n", mcu->core->reset_sp );

  //stm_flashaddr_t new_pc;
  stm = mcu->core;
  stm->log = LOG_TRACE;

  return 0x00;
}
/*
 *
 */
uint32_t cmd_fl_cb( int args, char** params )
{
  if( flash )
  {
    free( flash );
    flash = 0x00;
  }

  if( args < 2 )
  {
    flash=  read_ihex_file(
        pTestHexFile,
        &fsize,
        &fstart,
        &fstart_linear_address);
  } else
  {
    flash=  read_ihex_file(
        pTestHexFile,
        &fsize,
        &fstart,
        &fstart_linear_address);
  }

  printf("\nFLASH buffer was loaded form %s", pTestHexFile);

  return 0x00;
}
/*
 *
 */
uint32_t cmd_dump_mcu_cb( int args, char** params )
{
  if( mcu )
  {
    mcu->mcu_ifs->dump( mcu );
  }
  else
    printf( "\nMCU couldnt be dumped because mcu[NULL]" );

  return 0x00;
}
/*
 *
 */
uint32_t cmd_dump_ram_cb( int args, char** params )
{
  if( mcu )
  {
    hdump("\n\n[SRAM:HEX]", mcu->sram, mcu->sram_end - mcu->sram_start);
  }
  else
    printf( "\nMCU SRAM couldnt be dumped because mcu[NULL]" );

  return 0x00;
}
/*
 *
 */
uint32_t cmd_stm_run_cb( int args, char** params )
{
  if( mcu )
  {
    int run_cycles = 1;
    if( args >= 1 )
    {
      run_cycles = atoi( params[0]);
      if( !run_cycles )
      {
        printf("\nWARN:Strange number of cycles-%s\n", params[0]);
      }
    }

    for (uint64_t var = 0; var < run_cycles; ++var)
    {
      if( stm->state == cpu_Running )
      {
        stm_run_one( stm );
      }
    }
  }
  else
    printf( "\nMCU isnt INIT. Run is  senseless" );

  return 0x00;
}

uint32_t cmd_stm_run_until_address_cb( int args, char** params )
{
  if( mcu )
  {
//    uint32_t until_addr = 0xffffffff;
//    if( args >= 1 )
//    {
//      char *ptr;
//      until_addr = strtol( params[0], &ptr, 16 );
//    }
//
//    //until_addr = 0x080000f9;
//
//    until_addr = until_addr | 0x01;
//    printf("\nrua at %08x", until_addr);

//    while( stm->pc != until_addr )
//    {
//      if( stm->state == cpu_Running )
//      {
//        stm_run_one( stm );
//      }
//    }
//
//    printf("\nrua address %08x met\n", until_addr);

    uint32_t until_cycles = 1000;
    if( args >= 1 )
    {
      char *ptr;
      until_cycles = strtol( params[0], &ptr, 16 );
    }

    printf("\nrua cycles %08x", until_cycles);

    while( stm->cycle != until_cycles )
    {
      if( stm->state == cpu_Running )
      {
        stm_run_one( stm );
      }
    }

    printf("\nrua cycle %08x met\n", until_cycles);

  }
  else
    printf( "\nMCU isnt INIT. Run is  senseless" );

  return 0x00;
}
