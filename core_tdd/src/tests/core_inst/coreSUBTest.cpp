/*
 * coreSUBTest.cpp
 *
 *  Created on: 17 окт. 2020 г.
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
 * SUBS <Rd>,<Rn>,#<imm3>
 */
void coreSUB_imm3_T1Test()
{
  ASSERTM("SUBS IMM3 T1:start writing tests", false);
}

/*
 * SUBS <Rdn>,#<imm8>
 */
void coreSUB_imm8_T2Test()
{
  ASSERTM("SUBS IMM8 T2:start writing tests", false);
}
/*
 * SUBS <Rd>,<Rn>,<Rm>
 */
void coreSUB_reg_T1Test()
{
  ASSERTM("SUBS REG T1:start writing tests", false);
}

/*
 * SBCS <Rdn>,<Rm>
 */
void coreSBC_reg_T1Test()
{
  ASSERTM("SBC REG T1:start writing tests", false);
}
