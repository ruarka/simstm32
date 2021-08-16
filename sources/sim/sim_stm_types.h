/*
	sim_stm_types.h

 */


#ifndef __SIM_STM_TYPES_H___
#define __SIM_STM_TYPES_H___

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <inttypes.h>

typedef uint64_t	stm_cycle_count_t;
typedef uint16_t	stm_io_addr_t;


/*
 * this 'structure' is a packed representation of an IO register 'bit'
 * (or consecutive bits). This allows a way to set/get/clear them.
 * gcc is happy passing these as register value, so you don't need to
 * use a pointer when passing them along to functions.
 *
 * 9 bits ought to be enough, as it's the maximum I've seen (atmega2560)
 */
typedef struct stm_regbit_t
{
	uint32_t reg : 9, bit : 3, mask : 8;
} stm_regbit_t;

typedef struct stm_register_t
{
	uint32_t value;
	uint32_t read_msk;
	uint32_t write_msk;
} stm_register_t;


// printf() conversion specifier for avr_cycle_count_t
#define PRI_stm_cycle_count PRIu64

struct stm_t;

#ifdef __cplusplus
};
#endif

#endif /* __SIM_AVR_TYPES_H___ */
