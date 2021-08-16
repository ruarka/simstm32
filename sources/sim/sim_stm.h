/*
 * sim_stm.h
 *
 *  Created on: 6 февр. 2019 г.
 *      Author: dell
 */

#ifndef SIM_SIM_STM_H_
#define SIM_SIM_STM_H_

/* TRACES & LOGS control */
#define STM_LOG_ON        1
#define STM_TRACE_ON      1
//#define CORE_TRACE_ON   1
//#define NVIC_TRACE_ON   1
// #define SYSTICK_TRACE_ON  1


#ifdef __cplusplus
extern "C" {
#endif

#ifndef __has_attribute
	#define __has_attribute(x) 0
#endif

//#if __has_attribute(fallthrough)
//	#define FALLTHROUGH __attribute__((fallthrough));
//#else
//	#define FALLTHROUGH
//#endif

#include <stdint.h>
#include "sim_stm_types.h"


typedef uint32_t stm_flashaddr_t;

struct stm_t;
typedef uint8_t (*stm_io_read_t)(	struct stm_t * stm, stm_io_addr_t addr,	void * param);
typedef void (*stm_io_write_t)(	struct stm_t * stm, stm_io_addr_t addr,	uint8_t v, void * param);

/**
 * Logging macros and associated log levels.
 * The current log level is kept in avr->log.
 */
enum {
	LOG_NONE = 0,
	LOG_OUTPUT,
	LOG_ERROR,
	LOG_WARNING,
	LOG_TRACE,
	LOG_DEBUG,
};


#ifdef STM_LOG_ON
#ifndef STM_LOG
#define STM_LOG(stm, level, ...) \
	do { \
		stm_global_logger(stm, level, __VA_ARGS__); \
	} while(0)
#endif
#else
#define STM_LOG(stm, level, ...)
#endif

#ifdef STM_TRACE_ON
#ifndef STM_TRACE
#define STM_TRACE(stm, level, ... ) \
    do { \
      stm_global_logger(stm, level, __VA_ARGS__); \
    } while(0)
#endif
#else
#define STM_TRACE(stm, level, ...)
#endif

#ifdef CORE_TRACE_ON
#ifndef CORE_TRACE
#define CORE_TRACE(stm, level, ... ) \
    do { \
      stm_global_logger(stm, level, __VA_ARGS__); \
    } while(0)
#endif
#else
#define CORE_TRACE(stm, level, ...)
#endif

#ifdef NVIC_TRACE_ON
#ifndef NVIC_TRACE
#define NVIC_TRACE(stm, level, ... ) \
    do { \
      stm_global_logger(stm, level, __VA_ARGS__); \
    } while(0)
#endif
#else
#define NVIC_TRACE(stm, level, ...)
#endif

#ifdef SYSTICK_TRACE_ON
#ifndef SYSTICK_TRACE
#define SYSTICK_TRACE(stm, level, ... ) \
    do { \
      stm_global_logger(stm, level, __VA_ARGS__); \
    } while(0)
#endif
#else
#define SYSTICK_TRACE(stm, level, ...)
#endif

/*
 * Core states.
 */
enum {
	cpu_Limbo = 0,	// before initialization is finished
	cpu_Stopped,		// all is stopped, timers included

	cpu_Running,		// we're free running

	cpu_Sleeping,		// we're now sleeping until an interrupt

	cpu_Step,				// run ONE instruction, then...
	cpu_StepDone,		// tell gdb it's all OK, and give it registers
	cpu_Done,       // stm software stopped gracefully
	cpu_Crashed,    // stm software crashed (watchdog fired)
	cpu_BpFlash,		// breakpoint onto flash address access
	cpu_BpAddress,	// breakpoint onto address access
	cpu_BpCycle,    // breakpoint onto cycle number is met
	cpu_IntPending, // pending to interrupt
	cpu_IntLeaving, // initiate interrupt leaving process
	cpu_IntLeave    // interrupt leaving process
};

// this is only ever used if CONFIG_SIMAVR_TRACE is defined
struct avr_trace_data_t {
	struct avr_symbol_t ** codeline;

	/* DEBUG ONLY
	 * this keeps track of "jumps" ie, call,jmp,ret,reti and so on
	 * allows dumping of a meaningful data even if the stack is
	 * munched and so on
	 */
//	#define OLD_PC_SIZE	32
//	struct {
//		uint32_t pc;
//		uint16_t sp;
//	} old[OLD_PC_SIZE]; // catches reset..
//	int			old_pci;
//
//#if AVR_STACK_WATCH
//	#define STACK_FRAME_SIZE	32
//	// this records the call/ret pairs, to try to catch
//	// code that munches the stack -under- their own frame
//	struct {
//		uint32_t	pc;
//		uint16_t 	sp;
//	} stack_frame[STACK_FRAME_SIZE];
//	int			stack_frame_index;
//#endif
//
//	// DEBUG ONLY
//	// keeps track of which registers gets touched by instructions
//	// reset before each new instructions. Allows meaningful traces
//	uint32_t	touched[256 / 32];	// debug
};

typedef void (*stm_run_t)(struct stm_t * stm);
//
//#define AVR_FUSE_LOW	0
//#define AVR_FUSE_HIGH	1
//#define AVR_FUSE_EXT	2


/*
 *
 */
typedef struct hw_device_address_cell_t
{
	uint32_t 	start_address;
	uint32_t  end_address;
	struct hw_device_address_cell_t* next;
	struct stm_circuit_ifs_t* cir_cb_ifs;
} hw_device_address_cell_t;



struct mcu_t;

/*
 * Main STM32 core instance.
 */
typedef struct stm_t
{
	const char * 		mmcu;	// name of the STM32

	struct mcu_t* 	mcu;  // pointer to owner mcu

//	// these are filled by sim_core_declare from constants in /usr/lib/avr/include/avr/io*.h
//	uint16_t			ioend;
//  uint32_t* 			sram;
//  uint32_t				sram_start;
//  uint32_t				sram_end;
	uint32_t 				ramstart;
	uint32_t 				ramend;

	uint32_t				flash_start_adr;
	uint32_t				flash_end_adr;
	uint32_t				e2end;
	uint32_t 				start_linear_address;
//	uint8_t				vector_size;
//	uint8_t				signature[3];
//	uint8_t				fuse[6];
//	uint8_t				lockbits;
//	avr_io_addr_t		rampz;	// optional, only for ELPM/SPM on >64Kb cores
//	avr_io_addr_t		eind;	// optional, only for EIJMP/EICALL on >64Kb cores
//	uint8_t				address_size;	// 2, or 3 for cores >128KB in flash
//	struct {
//		avr_regbit_t		porf;
//		avr_regbit_t		extrf;
//		avr_regbit_t		borf;
//		avr_regbit_t		wdrf;
//	} reset_flags;
//

	// APSR
	union APSR
	{
		uint32_t val;
		// Bits
		struct bits_apsr_t
		{
			unsigned reserved : 28;
			unsigned V				: 1;
			unsigned C  			: 1;
			unsigned Z				: 1;
			unsigned N				: 1;
		}bits;
	}APSR;

  // IPSR
  union IPSR
  {
    uint32_t val;
    // Bits
    struct bits_ipsr_t
    {
      unsigned inum     : 6;
      unsigned reserved : 26;
    }bits;
  }IPSR;

  // EPSR
  union EPSR
  {
    uint32_t val;
    // Bits
    struct bits_espr_t
    {
      unsigned reserved0 : 24;
      unsigned T         : 1;
      unsigned reserved1 : 7;
    }bits;
  }EPSR;


	// internal registers R0 ... R12, SP=13, LR=14
	uint32_t R[ 15 ];

	// filled by the ELF data, this allow tracking of invalid jumps
	uint32_t			codeend;
//
	int					state;		// stopped, running, sleeping
	uint64_t		frequency;	// frequency we are running at
  // mostly used by the ADC for now
	uint32_t			vcc,avcc,aref; // (optional) voltages in millivolts

	// cycles gets incremented when sleeping and when running; it corresponds
	// not only to "cycles that runs" but also "cycles that might have run"
	// like, sleeping.
	stm_cycle_count_t	cycle;		// current cycle

	// these next two allow the core to freely run between cycle timers and also allows
	// for a maximum run cycle limit... run_cycle_count is set during cycle timer processing.
	stm_cycle_count_t	run_cycle_count;	// cycles to run before next timer
	stm_cycle_count_t	run_cycle_limit;	// maximum run cycle interval limit

	/**
	 *
	 */
	uint64_t nano_ticks;
	uint64_t nano_ticks_per_cycle;
	uint64_t nano_ticks_limit;

	/**
	 * Sleep requests are accumulated in sleep_usec until the minimum sleep value
	 * is reached, at which point sleep_usec is cleared and the sleep request
	 * is passed on to the operating system.
	 */
	uint32_t 			sleep_usec;
	uint64_t			time_base;	// for avr_get_time_stamp()

//	// called at init time
//	void (*init)(struct stm_t * stm);
//	// called at reset time
//	void (*reset)(struct stm_t * stm);
//	// called at release time
//	void (*release)(struct stm_t * stm);
//	// core dump
//	void (*dump)(struct stm_t * stm);

//	struct {
//		// called at init time (for special purposes like using a
//		// memory mapped file as flash see: simduino)
//		void (*init)(struct stm_t * stm, void * data);
//		// called at termination time ( to clean special initializations)
//		void (*deinit)(struct stm_t * stm, void * data);
//		// value passed to init() and deinit()
//		void *data;
//	} custom;

	/*!
	 * Default STM core run function.
	 * Two modes are available, a "raw" run that goes as fast as
	 * it can, and a "gdb" mode that also watchouts for gdb events
	 * and is a little bit slower.
	 */
	stm_run_t	run;

	/*!
	 * Sleep default behaviour.
	 * In "raw" mode, it calls usleep, in gdb mode, it waits
	 * for howLong for gdb command on it's sockets.
	 */
	void (*sleep)(struct stm_t * stm, stm_cycle_count_t howLong);

	/*!
	 * Every IRQs will be stored in this pool. It is not
	 * mandatory (yet) but will allow listing IRQs and their connections
	 */
		stm_irq_pool_t	irq_pool;
//
//	// Mirror of the SREG register, to facilitate the access to bits
//	// in the opcode decoder.
//	// This array is re-synthesized back/forth when SREG changes
//	uint8_t		sreg[8];
//
//	/* Interrupt state:
//		00: idle (no wait, no pending interrupts) or disabled
//		<0: wait till zero
//		>0: interrupt pending */
//	int8_t			interrupt_state;	// interrupt state

  /*
	 * PC
   */
	stm_flashaddr_t	pc;
	stm_flashaddr_t	reset_pc; 	/* Reset value for PC */

	/*
	 * SP(R13)
	 */
	stm_flashaddr_t msp;				/* Current Master SP 	*/
	stm_flashaddr_t psp;				/* Current Process SP */
	stm_flashaddr_t reset_sp; 	/* Reset value for 		*/

	/*
	 *  SCR -System Control Register
	 */
	struct
	{
		stm_regbit_t		bt0;
		stm_regbit_t		SLEEPONEXIT;
		stm_regbit_t		SLEEPDEEP;
		stm_regbit_t		bt3;
		stm_regbit_t		SEVEONPEND;
	} SCR;

//	/*
//	 * callback when specific IO registers are read/written.
//	 * There is one drawback here, there is in way of knowing what is the
//	 * "beginning of useful sram" on a core, so there is no way to deduce
//	 * what is the maximum IO register for a core, and thus, we can't
//	 * allocate this table dynamically.
//	 * If you wanted to emulate the BIG AVRs, and XMegas, this would need
//	 * work.
//	 */
//	struct {
//		struct stm_irq_t * irq;	// optional, used only if asked for with avr_iomem_getirq()
//		struct {
//			void * param;
//			stm_io_read_t c;
//		} r;
//		struct {
//			void * param;
//			stm_io_write_t c;
//		} w;
//	} io[MAX_IOs];

	/*
	 * This block allows sharing of the IO write/read on addresses between
	 * multiple callbacks. In 99% of case it's not needed, however on the tiny*
	 * (tiny85 at last) some registers have bits that are used by different
	 * IO modules.
	 * If this case is detected, a special "dispatch" callback is installed that
	 * will handle this particular case, without impacting the performance of the
	 * other, normal cases...
	 */
	int				io_shared_io_count;
	struct {
		int used;
		struct {
			void * param;
			void * c;
		} io[4];
	} io_shared_io[4];

	// flash memory (initialized to 0xff, and code loaded into it)
	uint8_t *		flash;
	// this is the general purpose registers, IO registers, and SRAM
	uint8_t *		data;


	// queue of io modules
	struct stm_io_t * io_port;

//	// Builtin and user-defined commands
//	avr_cmd_table_t commands;
	// cycle timers tracking & delivery
	stm_cycle_timer_pool_t	cycle_timers;
//	// interrupt vectors and delivery fifo
//	avr_int_table_t	interrupts;
//
//	// DEBUG ONLY -- value ignored if CONFIG_SIMAVR_TRACE = 0
	uint8_t	trace : 1,
			log : 4; // log level, default to 1
//
//	// Only used if CONFIG_SIMAVR_TRACE is defined
//	struct avr_trace_data_t *trace_data;
//
//	// VALUE CHANGE DUMP file (waveforms)
//	// this is the VCD file that gets allocated if the
//	// firmware that is loaded explicitly asks for a trace
//	// to be generated, and allocates it's own symbols
//	// using AVR_MMCU_TAG_VCD_TRACE (see avr_mcu_section.h)
//	struct avr_vcd_t * vcd;
//
	// gdb hooking structure. Only present when gdb server is active
	struct avr_gdb_t * gdb;

//	// if non-zero, the gdb server will be started when the core
//	// crashed even if not activated at startup
//	// if zero, the simulator will just exit() in case of a crash
//	int		gdb_port;
//
//	// buffer for console debugging output from register
//	struct {
//		char *	 buf;
//		uint32_t size;
//		uint32_t len;
//	} io_console_buffer;

	//
	stm_circuit_ifs_t* circuit_ifs;

	/*  */
	hw_device_address_cell_t* devs_pool;

} stm_t;

struct stm_rcc_t;

/*
 * Main STM32 instance.
 *
 * TODO: Add PRIMASK, FAULTMASK
 */
typedef struct mcu_t
{
		stm_t*					core;
    stm_rcc_t* 			rcc;
    stm_flash_t* 		flash;
    stm_systick_t*  systick;
    stm_scb_t*			scb;
    stm_nvic_t*			nvic;
//    stm_gpio_t*			portA;
    stm_gpio_t*			portB;
//    stm_gpio_t*			portC;
//    stm_gpio_t*			portD;
//    stm_gpio_t*			portE;
//    stm_gpio_t*			portH;

    stm_circuit_ifs_t* mcu_ifs;

  	uint8_t	trace : 1,
  			log : 4; // log level, default to 1

  	// flash memory (initialized to 0xff, and code loaded into it)
//  	uint8_t *				flash_data;
//  	uint32_t				flash_start_adr;
//  	uint32_t				flash_end_adr;

    // SRAM
    uint8_t* 				sram;
    uint32_t				sram_start;
    uint32_t				sram_end;

//    avr_eeprom_t     eeprom;
//    avr_watchdog_t    watchdog;
//    avr_extint_t    extint;
//    avr_ioport_t    porta, portb;
//    avr_acomp_t		acomp;
//    avr_adc_t        adc;
//    avr_timer_t    timer0, timer1;
}mcu_t;


// this is a static constructor for each of the AVR devices
typedef struct stm_kind_t
{
	const char * names[4];	// name aliases
	stm_t * (*make)(void);
} stm_kind_t;

// a symbol loaded from the .elf file
//typedef struct avr_symbol_t {
//	uint32_t	addr;
//	const char  symbol[0];
//} avr_symbol_t;

// locate the maker for mcu "name" and allocates a new avr instance
mcu_t * stm_make_mcu_by_name( const char *name );

stm_t * stm_core_fabric( const char *name );

// initializes a new AVR instance. Will call the IO registers init(), and then reset()
//int
//avr_init(
//		stm_t * avr);
// Used by the cores, allocated a mutable stm_t from the const global
stm_t* stm_core_allocate( const stm_t * core, uint32_t coreLen);

// resets the AVR, and the IO modules
//void
//avr_reset(
//		stm_t * avr);
// run one cycle of the AVR, sleep if necessary
//int
//avr_run(
//		stm_t * avr);
// finish any pending operations
//void
//avr_terminate(
//		stm_t * avr);

// set an IO register to receive commands from the AVR firmware
// it's optional, and uses the ELF tags
//void
//avr_set_command_register(
//		stm_t * avr,
//		avr_io_addr_t addr);

// specify the "console register" -- output sent to this register
// is printed on the simulator console, without using a UART
//void
//avr_set_console_register(
//		stm_t * avr,
//		avr_io_addr_t addr);

// load code in the "flash"
void stm_loadcode( mcu_t * mcu, uint8_t * code, uint32_t size, stm_flashaddr_t address);

/*
 * These are accessors for avr->data but allows watchpoints to be set for gdb
 * IO modules use that to set values to registers, and the AVR core decoder uses
 * that to register "public" read by instructions.
 */
//void
//stm_core_watch_write(
//		stm_t *avr,
//		uint16_t addr,
//		uint8_t v);
//uint8_t
//avr_core_watch_read(
//		stm_t *avr,
//		uint16_t addr);

// called when the core has detected a crash somehow.
// this might activate gdb server
//void
//avr_sadly_crashed(
//		stm_t *avr,
//		uint8_t signal);

/*
 * Logs a message using the current logger
 */
void stm_global_logger( struct stm_t* stm, const int level, const char * format, 	... );

#ifndef AVR_CORE
#include <stdarg.h>
/*
 * Type for custom logging functions
 */
typedef void (*stm_logger_p)(struct stm_t* stm, const int level, const char * format, va_list ap);

/* Sets a global logging function in place of the default */
//void
//avr_global_logger_set(
//		avr_logger_p logger);
/* Gets the current global logger function */
//avr_logger_p
//stm_global_logger_get(void);
#endif

/*
 * These are callbacks for the two 'main' behaviour in simavr
 */
//void avr_callback_sleep_gdb(stm_t * avr, avr_cycle_count_t howLong);
//void avr_callback_run_gdb(stm_t * avr);
//void avr_callback_sleep_raw(stm_t * avr, avr_cycle_count_t howLong);
//void avr_callback_run_raw(stm_t * avr);
void stm_callback_run_raw( mcu_t * mcu );

/**
 * Accumulates sleep requests (and returns a sleep time of 0) until
 * a minimum count of requested sleep microseconds are reached
 * (low amounts cannot be handled accurately).
 * This function is an utility function for the sleep callbacks
 */
//uint32_t
//avr_pending_sleep_usec(
//		stm_t * avr,
//		avr_cycle_count_t howLong);
///* Return the number of 'real time' spent since sim started, in uS */
//uint64_t
//avr_get_time_stamp(
//		stm_t * avr );
//

/*
 *
 */
extern uint32_t init_log( char* file_log_name );
/*
 *
 */
extern void close_log( void );

/**
 *
 */
mcu_t* stm_mcu_circuit_fabric( const char *name );


#ifdef __cplusplus
};
#endif

//#include "sim_io.h"
//#include "sim_regbit.h"

#ifdef __GNUC__

# ifndef likely
#  define likely(x) __builtin_expect(!!(x), 1)
# endif

# ifndef unlikely
#  define unlikely(x) __builtin_expect(!!(x), 0)
# endif

#else /* ! __GNUC__ */

# ifndef likely
#  define likely(x) x
# endif

# ifndef unlikely
#  define unlikely(x) x
# endif


#endif /* __GNUC__ */

#endif /* SIM_SIM_STM_H_ */
