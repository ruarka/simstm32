/*
 * sim_32l0x.h
 *
 *  Created on: 31 окт. 2019 г.
 *      Author: dell
 */

#ifndef CORES_SIM_32L0X_H_
#define CORES_SIM_32L0X_H_

//#include "sim_core_declare.h"
//#include "avr_eeprom.h"
//#include "avr_watchdog.h"
//#include "avr_extint.h"
//#include "avr_ioport.h"
//#include "avr_adc.h"
//#include "avr_timer.h"
//#include "avr_acomp.h"

void stm32l0x_init(struct stm_t * stm);
void stm32l0x_reset(struct stm_t * stm);
void stm32l0x_release(struct stm_t * stm);
void stm32l0x_dump( struct stm_t * stm );
void core_rcc_onupdate_cb(struct mcu_t * mcu, uint32_t param, uint64_t ticks_per_nsec, uint64_t frequency);
/*
 * This is a template for all of the stm32l031 devices, hopefully
 */
//typedef struct mcu_t
//{
//    stm_t core;
////    stm_rcc_t rcc;
//    stm_circuit_ifs_t* rcc_ifs;
//
//
////    avr_eeprom_t     eeprom;
////    avr_watchdog_t    watchdog;
////    avr_extint_t    extint;
////    avr_ioport_t    porta, portb;
////    avr_acomp_t		acomp;
////    avr_adc_t        adc;
////    avr_timer_t    timer0, timer1;
//}mcu_t;

#define SIM_VECTOR_SIZE    2
#define SIM_MMCU        "32l031k6t6"

#ifndef SIM_CORENAME
#define SIM_CORENAME    mcu_32l031k6t6
#endif

#ifndef SIM_VECTOR_SIZE
#error SIM_VECTOR_SIZE is not declared
#endif
#ifndef SIM_MMCU
#error SIM_MMCU is not declared
#endif

extern const struct mcu_t SIM_CORENAME;

extern stm_flashaddr_t (*instruction_matrix[ 65536 ])(struct stm_t * stm, uint16_t opcode);

#endif /* CORES_SIM_32L0X_H_ */
