/*
 * cmdshellcb.h
 *
 *  Created on: 18 окт. 2020 г.
 *      Author: art_m
 */

#ifndef CMDSHELLCB_H_
#define CMDSHELLCB_H_

extern uint32_t cmd_new_cb( int args, char** params );
extern uint32_t cmd_nir_cb( int args, char** params );
extern uint32_t cmd_fl_cb( int args, char** params );
extern uint32_t cmd_dump_mcu_cb( int args, char** params );
extern uint32_t cmd_dump_ram_cb( int args, char** params );
extern uint32_t cmd_stm_run_cb( int args, char** params );
extern uint32_t cmd_stm_run_until_address_cb( int args, char** params );


#endif /* CMDSHELLCB_H_ */
