/*
 * m0pi_tdd.h
 *
 *  Created on: 12 sep 2020
 *      Author: art_m
 */

#ifndef SRC_M0PI_TDD_H_
#define SRC_M0PI_TDD_H_

void coreWFETest();
void coreWFITest();
void coreSEVTest();

extern mcu_t* coreInitWithInstruction( uint32_t inst_code );

#endif /* SRC_M0PI_TDD_H_ */
