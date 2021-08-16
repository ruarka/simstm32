/*
 ============================================================================
 Name        : dbg.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================

  TODO: Add --[gpio_o_stream "<file.txt>"] creates file with signal outputs for
        GPIO bits

 */
#include <unistd.h>  //Äëÿ getwd
#include <limits.h>  //Äëÿ PATH_MAX

#include <stdio.h>
#include <stdlib.h>
#include <libgen.h>
#include <string.h>

#include "sim_inc.h"

#include "sim_hex.h"
#include "cmdshellcb.h"
#include "cmdshell.h"

#include <time.h>

/*
 * Debug shell commands
 */
SHELL_CMDS_BEGIN
	SHELL_CMD("r","r - run 1 instruction by sim instance", "err_r",cmd_stm_run_cb)
	SHELL_CMD("rua","rua - run until <address> met, <address> omitted run continuously", "err_rua",cmd_stm_run_until_address_cb)
	SHELL_CMD("fl","fl - flash load from file", "err_fl",cmd_fl_cb)
	SHELL_CMD("nir","nir - new sim instance, init, reset","err nir",cmd_nir_cb)
	SHELL_CMD("new","new - new sim instance", "err new",cmd_new_cb)
	SHELL_CMD("dm","dm - dump state sim instance", "err dmcu",cmd_dump_mcu_cb)
	SHELL_CMD("dr","dr - dr <sadr> <lines>- dump mcu ram sim instance.\n\t<sadr> start address\n\t<lines> printed lines", "err dmcu",cmd_dump_ram_cb)
SHELL_CMDS_END


//mcu_t* mcu = 0;
//stm_t* stm = 0;
//uint8_t* flash = 0;
//uint32_t fsize, fstart, fstart_linear_address;

char* pTestHexFile = "D:\\Projects\\ecplise\\simstm\\tdata\\20.03.21.v.01.hex";

// Global session profile
// uint64_t session_cycles = 1169;

/*
 *
// __asm {
//  mov eax, ecx
//  }
//  __asm("movl %eax, %ebx");
 */

//uint32_t f_cpu = 0;
//
//static void  display_usage( const char * app )
//{
//	printf("Usage: %s [...] <firmware>\n", app);
//	printf( "		[--freq|-f <freq>]  Sets the frequency for an .hex firmware\n"
//			"		[--mcu|-m <device>] Sets the MCU type for an .hex firmware\n"
//			"       [--list-cores]      List all supported AVR cores and exit\n"
//			"       [--help|-h]         Display this usage message and exit\n"
//			"       [--trace, -t]       Run full scale decoder trace\n"
//			"       [-ti <vector>]      Add traces for IRQ vector <vector>\n"
//			"       [--gdb|-g]          Listen for gdb connection on port 1234\n"
//			"       [-ff <.hex file>]   Load next .hex file as flash\n"
//			"       [-ee <.hex file>]   Load next .hex file as eeprom\n"
//			"       [--input|-i <file>] A .vcd file to use as input signals\n"
//			"       [-v]                Raise verbosity level\n"
//			"                           (can be passed more than once)\n"
//			"       <firmware>          A .hex or an ELF file. ELF files are\n"
//			"                           prefered, and can include debugging syms\n");
//	exit(1);
//}

//static void list_cores( void )
//{
//	printf( "Supported STM cores:\n stm32l031 ");
////	for (int i = 0; avr_kind[i]; i++) {
////		printf("       ");
////		for (int ti = 0; ti < 4 && avr_kind[i]->names[ti]; ti++)
////			printf("%s ", avr_kind[i]->names[ti]);
////		printf("\n");
////	}
//	exit(1);
//}

FILE* f_ext_log = NULL;

void init_ext_log( char* ext_file_log_name )
{
  if( !f_ext_log )
    f_ext_log = fopen( ext_file_log_name, "w" );
}

void close_gpio_log( void )
{
  if( f_ext_log )
    fclose( f_ext_log );
}


stm_cycle_count_t stm_cycle_timer_test(	struct stm_t * stm, stm_cycle_count_t when,	void * param)
{
	return 0x00;
}

//void pin_change_state_cb(
//    struct stm_pin_t* pin,
//    uint32_t val,
//    pin_source_t val_type )
void pin_change_state_cb( struct stm_pin_t* pin )
{
  if( f_ext_log ){

    uint64_t uTimeTmp = pin->mcu->core->cycle * pin->mcu->core->nano_ticks_per_cycle;
    uint32_t uNanoSeconds = uTimeTmp%1000;
    uTimeTmp /= 1000;
    uint32_t uMicroSeconds = uTimeTmp%1000;
    uTimeTmp /= 1000;
    uint32_t uMiliSeconds = uTimeTmp%1000;
    uint64_t uSeconds = uTimeTmp/1000;

    char pBuff[1000];

    switch( pin->val_type ) {

      case vtUndefined:
        sprintf( pBuff, "[%I64u.%03d.%03d.%03d] %s[ UNDEF ]\n", uSeconds, uMiliSeconds, uMicroSeconds, uNanoSeconds ,pin->name );
        break;

      case vtFloating:
        sprintf( pBuff, "[%I64u.%03d.%03d.%03d] %s[ FLOAT ]\n", uSeconds, uMiliSeconds, uMicroSeconds, uNanoSeconds ,pin->name );
        break;

      case vtDigital:
        if( pin->val )
          sprintf( pBuff, "[%I64u.%03d.%03d.%03d] %s[ 1 ]\n", uSeconds, uMiliSeconds, uMicroSeconds, uNanoSeconds ,pin->name );
        else
          sprintf( pBuff, "[%I64u.%03d.%03d.%03d] %s[ 0 ]\n", uSeconds, uMiliSeconds, uMicroSeconds, uNanoSeconds ,pin->name );

        break;

      case vtCollision:
        sprintf( pBuff, "[%I64u.%03d.%03d.%03d] %s[ COLISION ]\n", uSeconds, uMiliSeconds, uMicroSeconds, uNanoSeconds ,pin->name );
        break;

      case vtAnalog:
      default:
        break;
    }

//    switch( pin->val ){
//      case PIN_FLOATING_VALUE:
//        sprintf( pBuff, "[%I64u.%03d.%03d.%03d] %s[ UNDEF ]\n", uSeconds, uMiliSeconds, uMicroSeconds, uNanoSeconds ,pin->name );
//        break;
//
//      case PIN_COLLISION_VALUE:
//        sprintf( pBuff, "[%I64u.%03d.%03d.%03d] %s[ COLISION ]\n", uSeconds, uMiliSeconds, uMicroSeconds, uNanoSeconds ,pin->name );
//        break;
//
//      case PIN_DIGITAL_ONE_VALUE:
//        sprintf( pBuff, "[%I64u.%03d.%03d.%03d] %s[ 1 ]\n", uSeconds, uMiliSeconds, uMicroSeconds, uNanoSeconds ,pin->name );
//        break;
//
//      case PIN_DIGITAL_ZERO_VALUE:
//        sprintf( pBuff, "[%I64u.%03d.%03d.%03d] %s[ 0 ]\n", uSeconds, uMiliSeconds, uMicroSeconds, uNanoSeconds ,pin->name );
//        break;
//
//      default:
//        break;
//    }

    fputs( pBuff, f_ext_log );
  }
}

#if 1
/*
 *  Master debug branch
 */

#include <libgen.h>


int main_dbg(int argc, char *argv[])
{
  clock_t clock_start, clock_end;
  double elapsed;

//  init_log( "d:\\tmp\\1\\stm.log" );
  init_log( "stm.log" );

  init_ext_log( "d:\\tmp\\1\\gpio.log" );

//  char path[ MAX_PATH ];
//  strcpy( path, argv[0] );
//  char *dirc, *basec, *bname, *dname;
//  dname = dirname( path );

  uint32_t dsize, start, start_linear_address;
//  uint8_t* pFlashData =  read_ihex_file(
////      "D:\\Projects\\eclipse\\simstm\\tdata\\STM32L031K6_1.hex",
//      "D:\\Projects\\eclipse\\simstm\\tdata\\STM32L031K6_NUCLEO.01.hex",
//      &dsize,
//      &start,
//      &start_linear_address);

//  uint8_t* pFlashData =  read_ihex_file(
//      "D:\\Projects\\eclipse\\simstm\\tdata\\20.03.21.v.01.hex",
//      &dsize,
//      &start,
//      &start_linear_address);

  uint8_t* pFlashData =  read_ihex_file(
      "D:\\Projects\\eclipse\\simstm\\tdata\\20.11.12.v.01.hex",
      &dsize,
      &start,
      &start_linear_address);


//  hdump("[ART:HEX]", pFlashData, dsize);

  // mcu init
  char pMMCU[] = "stm32l031";

  // stm_core_fabric
  mcu_t* pMcu = stm_mcu_circuit_fabric( pMMCU );
  if (!pMcu )
  {
    fprintf(stderr, "%s: STM '%s' not known\n", "stmsim", pMMCU );
    exit(1);
  }

  pMcu->log = LOG_DEBUG;

  pMcu->mcu_ifs->init( pMcu );

  // Testing external cb setting
  // ( struct stm_pin_t* port, pfn_pin_change_state_cb_t fn )
  //pin_change_state_cb
  pMcu->portB->pins[ 3 ].ctrl.pfn_set_pin_ext_cb( &(pMcu->portB->pins[ 3 ]), pin_change_state_cb );


  stm_loadcode( pMcu, pFlashData, dsize, 0x00000000 );

  pMcu->mcu_ifs->reset( pMcu, RESET_POWER_PWR );

  printf( "\n\nflash_start=%x", pMcu->core->flash_start_adr );
  printf( "\nflash_end=%x", pMcu->core->flash_end_adr );
  printf( "\nstart_linear_address=%x", pMcu->core->start_linear_address );


  printf( "\n\nreset_pc=%08x", pMcu->core->reset_pc );
  printf( "\nreset_sp=%08x\n\n", pMcu->core->reset_sp );

  //stm_flashaddr_t new_pc;
  stm_t* pStm = pMcu->core;
  pStm->log = LOG_DEBUG;

  printf( "\n\nPress Any key to start...\n" );
  getchar();

  clock_start = clock();

  for (int var = 0; var < 2000; ++var)
//  for (int var = 0; var < 1; ++var)
  {
//        while(( pStm->cycle < (1000 +(var*1000 )))
//            &&(  pStm->pc != 0xffffffff ))
          while( pStm->cycle < (1000 +(var*1000 )))
          {
            stm_callback_run_raw( pMcu );

//            if( pStm->pc >= 0xf0000000 )
//            {
//                printf( "\nPC_OLD[0x%08X] PC_NEW[0x%08X]\n", old_pc, pStm->pc );
//                exit(1);
//            }

            if( pStm->state != cpu_Running )
            {
              if( pStm->state == cpu_Crashed )
              {
                 STM_LOG( pStm, LOG_TRACE, "\nERROR:Core had Crash!!!\n" );
                 exit( 1 );
              }

              if( pStm->state == cpu_BpAddress )
              {
                STM_LOG( pStm, LOG_TRACE, "\nDEBUG:BP met\n" );

                pStm->mcu->core->state = cpu_Running;

                stm_run_one_no_bp(pMcu->core);
              }else if( pStm->state == cpu_BpFlash )
              {
                STM_LOG( pStm, LOG_TRACE, "\nDEBUG:BP met\n" );

                pStm->mcu->core->state = cpu_Running;

                stm_run_one_no_bp(pMcu->core);
              } else if( pStm->state == cpu_BpCycle )
              {
                  STM_LOG( pStm, LOG_TRACE, "\nDEBUG:BP Cycle met\n" );

                  pStm->mcu->core->state = cpu_Running;

                  stm_run_one_no_bp(pMcu->core);
              }

              if(( pStm->state != cpu_Running )
                &&( pStm->state != cpu_IntPending )
                &&( pStm->state != cpu_IntLeaving )
                &&( pStm->state != cpu_IntLeave ))
              {
                  STM_LOG( pStm, LOG_TRACE, "\nERROR:Core Crash with code[%d]\n", pStm->state );
                  exit( 1 );
              }

          }
        }
  }

//  stm_run_one_no_bp(pMcu->core);

  clock_end = clock();
  elapsed=((double)(clock_end-clock_start))/CLOCKS_PER_SEC;
  STM_LOG( pStm, LOG_DEBUG, "\nProcTime sec:%lf\n", elapsed );

  if( pMcu->mcu_ifs->dump )
  {
    pMcu->mcu_ifs->dump( pMcu );
  }

  if( pMcu->mcu_ifs->release )
  {
    pMcu->mcu_ifs->release( pMcu, pMcu );
  }

  close_gpio_log();

  close_log();

  printf( "\nProcTime sec:%lf\n", elapsed );

  printf( "\nPress Any key to  finish..." );
  getchar();

  return EXIT_SUCCESS;
}
#endif


int main_cmd_shell(int argc, char *argv[])
{
  shell_run( 0x01 );

  return EXIT_SUCCESS;
}


int main(int argc, char *argv[])
{
  for (int i = 0; i < argc; ++i) {
    if( argv[ i ]!= NULL  )
      if(( strcmp( argv[ i ], "dbg" )== 0 )
         ||( strcmp( argv[ i ], "-dbg" )== 0 )
         ||( strcmp( argv[ i ], "--dbg" )== 0 ))
        return main_dbg( argc, argv );
  }

  return main_cmd_shell( argc, argv );
}



#if 0

int main(int argc, char *argv[])
{
  clock_t clock_start, clock_end;
  double elapsed;

//  if( !menu( argc, argv ))
//    exit( 1 );

//  if( !session_cycles )
//  {
//    char ch;
//    printf( "\n\nEnter cycles number:" );
//    scanf("%c", &ch);
//  }

  init_log( "d:\\tmp\\1\\stm.log" );

  uint32_t dsize, start, start_linear_address;
//  uint8_t* pFlashData =  read_ihex_file(
////      "D:\\Projects\\ecplise\\simstm\\tdata\\STM32L031K6_1.hex",
//      "D:\\Projects\\ecplise\\simstm\\tdata\\STM32L031K6_NUCLEO.01.hex",
//      &dsize,
//      &start,
//      &start_linear_address);

  uint8_t* pFlashData =  read_ihex_file(
      "D:\\Projects\\ecplise\\simstm\\tdata\\20.03.21.v.01.hex",
      &dsize,
      &start,
      &start_linear_address);


//  hdump("[ART:HEX]", pFlashData, dsize);

  // mcu init
  char pMMCU[] = "stm32l031";

  // stm_core_fabric
  mcu_t* pMcu = stm_mcu_circuit_fabric( pMMCU );
  if (!pMcu )
  {
  //    fprintf(stderr, "%s: STM '%s' not known\n", argv[0], pMMCU );
    fprintf(stderr, "%s: STM '%s' not known\n", "stmsim", pMMCU );
    exit(1);
  }

  pMcu->log = LOG_DEBUG;

  pMcu->mcu_ifs->init( pMcu );

  stm_loadcode( pMcu, pFlashData, dsize, 0x00000000 );

  pMcu->mcu_ifs->reset( pMcu, RESET_POWER_PWR );

  printf( "\n\nflash_start=%x", pMcu->core->flash_start_adr );
  printf( "\nflash_end=%x", pMcu->core->flash_end_adr );
  printf( "\nstart_linear_address=%x", pMcu->core->start_linear_address );


  printf( "\n\nreset_pc=%08x", pMcu->core->reset_pc );
  printf( "\nreset_sp=%08x\n\n", pMcu->core->reset_sp );

  //stm_flashaddr_t new_pc;
  stm_t* pStm = pMcu->core;
  pStm->log = LOG_TRACE;

  clock_start = clock();

  for (int var = 0; var < 200; ++var)
  {
      //  while(( pStm->cycle < 3200 )
        while(( pStm->cycle < (10000 +(var*10000 )))
            &&(  pStm->pc != 0xffffffff ))
        {
            stm_callback_run_raw( pMcu );

            if(( pStm->state == cpu_BpAddress )
              ||(  pStm->state == cpu_BpFlash ) )
          {
            STM_LOG( pStm, LOG_TRACE, "\nDEBUG:BP met\n" );
            pStm->mcu->mcu_ifs->dump( pStm->mcu );
            exit(1);
          }
        }
  }

  clock_end = clock();
  elapsed=((double)(clock_end-clock_start))/CLOCKS_PER_SEC;

//  STM_LOG( stm, LOG_DEBUG, "\nProcTime sec:%lf\n", elapsed );

  STM_LOG( pStm, LOG_DEBUG, "\nProcTime sec:%lf\n", elapsed );

  printf( "\nProcTime sec:%lf\n", elapsed );


  if( pMcu->mcu_ifs->dump )
  {
    pMcu->mcu_ifs->dump( pMcu );
  }

//  hdump("\n\n[SRAM:HEX]", pMcu->sram, pMcu->sram_end - pMcu->sram_start);

  if( pMcu->mcu_ifs->release )
  {
    pMcu->mcu_ifs->release( pMcu );
  }

  close_log();

  return EXIT_SUCCESS;
}
#endif

/**
  * @brief  Initializes the CPU, AHB and APB buses clocks according to the specified
  *         parameters in the RCC_ClkInitStruct.
  * @param  RCC_ClkInitStruct pointer to an RCC_OscInitTypeDef structure that
  *         contains the configuration information for the RCC peripheral.
  * @param  FLatency FLASH Latency
  *          The value of this parameter depend on device used within the same series
  * @note   The SystemCoreClock CMSIS variable is used to store System Clock Frequency
  *         and updated by @ref HAL_RCC_GetHCLKFreq() function called within this function
  *
  * @note   The MSI is used (enabled by hardware) as system clock source after
  *         start-up from Reset, wake-up from STOP and STANDBY mode, or in case
  *         of failure of the HSE used directly or indirectly as system clock
  *         (if the Clock Security System CSS is enabled).
  *
  * @note   A switch from one clock source to another occurs only if the target
  *         clock source is ready (clock stable after start-up delay or PLL locked).
  *         If a clock source which is not yet ready is selected, the switch will
  *         occur when the clock source will be ready.
  *         You can use @ref HAL_RCC_GetClockConfig() function to know which clock is
  *         currently used as system clock source.
  * @note   Depending on the device voltage range, the software has to set correctly
  *         HPRE[3:0] bits to ensure that HCLK not exceed the maximum allowed frequency
  *         (for more details refer to section above "Initialization/de-initialization functions")
  * @retval HAL status
  */


