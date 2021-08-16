/*
 * sim_m0pins.c
 *
 *  Created on: 18 nov 2019 y
 *      Author: ruarka

   TODO: APSR(V) - It doesn't work properly.
                   Instead of assigning APSR(V) to 0 in instructions which have influence
                   on V but an assigning is not done because of mistake.
                   Should be corrected.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sim_inc.h"

/* ------------------------------------------------------------------------------------------------
 *                                 Type definitions
 * ------------------------------------------------------------------------------------------------
 */

/* ------------------------------------------------------------------------------------------------
 *                                     Macros
 * ------------------------------------------------------------------------------------------------
 */
#define UPDATE_Z_N( R, ZF, NF )                 \
  do{                                           \
    if( R & 0x80000000 )                        \
      NF = 1;                                   \
    else                                        \
      NF = 0;                                   \
                                                \
    if( R )                                     \
      ZF = 0;                                   \
    else                                        \
      ZF = 1;                                   \
  }while(0)

#define ADD_WITH_C_V( R, CF, OF, X, Y )         \
  do{                                           \
    union result                                \
    {                                           \
      uint64_t  val;                            \
      struct hilo                               \
      {                                         \
        uint32_t  lo;                           \
        uint32_t  hi;                           \
      }hilo;                                    \
    }res64;                                     \
                                                \
    res64.val = 0;                              \
    res64.hilo.lo = X;                          \
    res64.val += Y;                             \
                                                \
    R = res64.hilo.lo;                          \
                                                \
    if(( X & 0x80000000 )!=( Y & 0x80000000 ))  \
      OF = 0;                                   \
    else if(( res64.hilo.lo & 0x80000000 )      \
        ==( Y & 0x80000000 ))                   \
      OF = 0;                                   \
    else                                        \
      OF = 1;                                   \
                                                \
    if( res64.hilo.hi )                         \
      CF = 1;                                   \
    else                                        \
      CF = 0;                                   \
  }while(0)


#define ADD_WITH_CARRY( R, CF, OF, X, Y, Cin )  \
  do{                                           \
    union result                                \
    {                                           \
      uint64_t  val;                            \
      struct hilo                               \
      {                                         \
        uint32_t  lo;                           \
        uint32_t  hi;                           \
      }hilo;                                    \
    }res64;                                     \
                                                \
    res64.val = 0;                              \
    res64.hilo.lo = X;                          \
    res64.val += Y;                             \
    res64.val += Cin;                           \
                                                \
    R = res64.hilo.lo;                          \
                                                \
    if(( X & 0x80000000 )!=( Y & 0x80000000 ))  \
      OF = 0;                                   \
    else if(( res64.hilo.lo & 0x80000000 )      \
        ==( Y & 0x80000000 ))                   \
      OF = 0;                                   \
    else                                        \
      OF = 1;                                   \
                                                \
    if( res64.hilo.hi )                         \
      CF = 1;                                   \
    else                                        \
      CF = 0;                                   \
  }while(0)

#define TRACE_ASPR() CORE_TRACE( stm, LOG_TRACE, "\tASPR[N.Z.C.V]=[%01d.%01d.%01d.%01d]\n", stm->APSR.bits.N, stm->APSR.bits.Z, stm->APSR.bits.C, stm->APSR.bits.V )

/* ------------------------------------------------------------------------------------------------
 *                                 Global Variables
 * ------------------------------------------------------------------------------------------------
 */

/* ------------------------------------------------------------------------------------------------
 *                         Local Variables & Functions
 * ------------------------------------------------------------------------------------------------
 */
uint32_t uiSubroutineLvl = 0x00;


/*
 * FIRST PACK OF FUNC
 */
stm_flashaddr_t blx(struct stm_t * stm, uint16_t opcode )
{
  union _ARMv6mThum2Instruction inst;
  inst.opcode = opcode;

  CORE_TRACE( stm, LOG_TRACE, "0x%08X BLX[%04x] Rm[%d] rest[%d]\n", stm->pc &0xFFFFFFFE, inst.opcode, inst.BLX.Rm, inst.BLX.rest );

  // save return address into LR
  stm->R[14] = (stm->pc + 2)| 0x00000001;

  CORE_TRACE( stm, LOG_TRACE, "\tLR[0x%08X] PC[0x%08X]\n", stm->R[14], stm->R[inst.BLX.Rm] & 0xfffffffe );

  // new pc value
  return stm->R[inst.BLX.Rm] | 0x00000001;
}

stm_flashaddr_t bx_t1(struct stm_t * stm, uint16_t opcode )
{
  union _ARMv6mThum2Instruction inst;
  inst.opcode = opcode;

  CORE_TRACE( stm, LOG_TRACE, "0x%08X BX[%04x] Rm[%d]\n", stm->pc &0xFFFFFFFE, inst.opcode, inst.BX_t1.Rm );
  CORE_TRACE( stm, LOG_TRACE, "\tPC[0x%08X]\n", stm->R[inst.BX_t1.Rm ] & 0xfffffffe );

  // check for interrupt leaving
  if( inst.BX_t1.Rm == 14 )
  {
      // todo : nvic interrupt return
      uint32_t val = stm->R[inst.BX_t1.Rm ];
      if(( val & 0xfffffff0 )== 0xfffffff0 )
      {
        // Try to leave interrupt
        nvic_run_interrupt_leaving( stm->mcu, val );
      }
  }

  // new pc value
  return stm->R[inst.BX_t1.Rm ];
}
/*
 *
 */
stm_flashaddr_t bl_t1(struct stm_t * stm, uint16_t opcode, uint16_t opcode2 )
{
  union _ARMv6mThum2Instruction inst_w1;
  inst_w1.opcode = opcode;

  union _ARMv6mThum2Instruction inst_w2;
  inst_w2.opcode = _stm_flash_read16le( stm, stm->pc+2);

  union imm32_offset
  {
    uint32_t          val;
    struct imm32
    {
      unsigned zero   : 1;
      unsigned imm11  : 11;
      unsigned imm10  : 10;
      unsigned I2     : 1;
      unsigned I1     : 1;
      unsigned S      : 1;
      unsigned signExt: 7;
    }imm32;
  }imm32_offset;

  imm32_offset.val = 0;
  imm32_offset.imm32.imm11  = inst_w2.BL_t1_part2.imm11;
  imm32_offset.imm32.imm10  = inst_w1.BL_t1_part1.imm10;
  imm32_offset.imm32.I2     = ~(inst_w1.BL_t1_part1.S ^ inst_w2.BL_t1_part2.J2);
  imm32_offset.imm32.I1     = ~(inst_w1.BL_t1_part1.S ^ inst_w2.BL_t1_part2.J1);
  imm32_offset.imm32.S      = inst_w1.BL_t1_part1.S;

  if( imm32_offset.imm32.S )
    imm32_offset.imm32.signExt = 0x7f ;

  CORE_TRACE( stm, LOG_TRACE, "0x%08X BL[%04x] OFS[0x%08X] \n", stm->pc &0xFFFFFFFE, inst_w1.opcode, imm32_offset );

  uint32_t next_instr_addr = stm->pc +4;

  // save return address into LR
  stm->R[14] = next_instr_addr | 0x00000001;

  CORE_TRACE( stm, LOG_TRACE, "\tLR[0x%08X] PC[0x%08X]\n", stm->R[14], (next_instr_addr +imm32_offset.val) & 0xfffffffe );

  // new pc value
  return next_instr_addr +imm32_offset.val;
}
/*
 *
 */
stm_flashaddr_t msr_t1(struct stm_t * stm, uint16_t opcode, uint16_t opcode2 )
{
  union msr_hw1
  {
    uint16_t val;
    struct fisrt_hw
    {
      unsigned Rn     : 4;
      unsigned opcode : 12;
    }fisrt_hw;
  }msr_hw1;

  union msr_hw2
  {
    uint16_t val;
    struct second_hw
    {
      unsigned SYSm   : 8;
      unsigned opcode : 8;
    }second_hw;
  }msr_hw2;

  msr_hw1.val = opcode;
  msr_hw2.val = opcode2;

  CORE_TRACE( stm, LOG_TRACE, "0x%08X MSR[%04x][%04x] \n", stm->pc &0xFFFFFFFE, opcode, opcode2 );

  switch (msr_hw2.second_hw.SYSm & 0xf8) {
    case 0:
      {
        if(!( msr_hw2.second_hw.SYSm & 0x04 ))
        {
          stm->APSR.val = stm->R[ msr_hw1.fisrt_hw.Rn ] & 0xf8000000;

          TRACE_ASPR();
        }
      }
      break;

    case 0x08:
      {
          if(( msr_hw2.second_hw.SYSm & 0x07) == 0x00 )
          {
            // TODO: SP_main
            //stm->s
          }else if(( msr_hw2.second_hw.SYSm & 0x07) == 0x01 )
          {
            //TODO: SP_proccess
          }
      }
      break;

    case 0x10:
      {

      }
      break;

    default:
      break;
  }

  return stm->pc+4;
}
/*
 *
 */
stm_flashaddr_t fb01_block(struct stm_t * stm, uint16_t opcode )
{
  stm_flashaddr_t next_pc;
  uint16_t opcode2 = _stm_flash_read16le( stm, stm->pc+2 );

  switch(  opcode2 & 0xc000  )
  {
    case 0x8000:
      {
        switch (opcode & 0x70 )
        {
          case 0:
            // MSR
            next_pc = msr_t1(stm, opcode, opcode2 );
            break;

          case 0x60:
            // MRS
            CORE_TRACE( stm, LOG_ERROR, "Unknown ins[%04x][%04x] at flash[%x]\n", opcode, opcode2, stm->pc );
            next_pc = stm->pc;
            break;

          default:
            CORE_TRACE( stm, LOG_ERROR, "Unknown ins[%04x][%04x] at flash[%x]\n", opcode, opcode2, stm->pc );
            next_pc = stm->pc;
            break;
        }
      }
      break;

    case 0xc000:
      next_pc = bl_t1(stm, opcode, opcode2 );
      break;

    default:
      CORE_TRACE( stm, LOG_ERROR, "Unknown ins[%04x][%04x] at flash[%x]\n", opcode, opcode2, stm->pc );
      next_pc = stm->pc;
      break;
  }

  return next_pc;
}
/*
 *
 */
stm_flashaddr_t b_t2(struct stm_t * stm, uint16_t opcode )
{
  union _ARMv6mThum2Instruction inst;
  inst.opcode = opcode;

  union imm32_offset
  {
    uint32_t          val;
    struct imm32
    {
      unsigned zero   : 1;
      unsigned imm11  : 11;
      unsigned signExt: 20;
    }imm32;
  }imm32_offset;

  imm32_offset.val = 0;

  imm32_offset.imm32.imm11  = inst.B_t2.imm11;

  if( imm32_offset.val & 0x800 )
    imm32_offset.imm32.signExt = 0xfffff;

  CORE_TRACE( stm, LOG_TRACE, "0x%08X B[%04x] OFS[0x%08X] \n", stm->pc &0xFFFFFFFE, inst.opcode, imm32_offset );

  uint32_t next_instr_addr = stm->pc +4 +imm32_offset.val;

  CORE_TRACE( stm, LOG_TRACE, "\tPC[0x%08X]\n", next_instr_addr );

  // new pc value
  return next_instr_addr;
}
/*
 *
 */
stm_flashaddr_t b_t1(struct stm_t * stm, uint16_t opcode )
{
  union _ARMv6mThum2Instruction inst;
  inst.opcode = opcode;

  CORE_TRACE( stm, LOG_TRACE, "0x%08X B<C>[%04x] CON[%04x] IMM8[0x%04x]\n", stm->pc &0xFFFFFFFE, inst.opcode, inst.B_t1.cond, inst.B_t1.imm8 );

  // stm->APSR.bits
  switch (inst.B_t1.cond) {
    case 0: // Z == 1
      if( !stm->APSR.bits.Z )
        return stm->pc+2;
      break;

    case 1:
      if( stm->APSR.bits.Z )
        return stm->pc+2;
      break;

    case 2:
      if( !stm->APSR.bits.C )
        return stm->pc+2;
      break;

    case 3:
      if( stm->APSR.bits.C )
        return stm->pc+2;
      break;

    case 4:
      if( !stm->APSR.bits.N )
        return stm->pc+2;
      break;

    case 5:
      if( stm->APSR.bits.N )
        return stm->pc+2;
      break;

    case 6:
      if( !stm->APSR.bits.V )
        return stm->pc+2;
      break;

    case 7:
      if( stm->APSR.bits.V )
        return stm->pc+2;
      break;

    case 8:
      if( !stm->APSR.bits.C || stm->APSR.bits.Z )
        return stm->pc+2;
      break;

    case 9:
      if( stm->APSR.bits.C && !stm->APSR.bits.Z )
        return stm->pc+2;
      break;

    case 10:
      if( stm->APSR.bits.N != stm->APSR.bits.V )
        return stm->pc+2;
      break;

    case 11:
      if( stm->APSR.bits.N == stm->APSR.bits.V )
        return stm->pc+2;
      break;

    case 12:
      if( stm->APSR.bits.Z || (stm->APSR.bits.N != stm->APSR.bits.V ))
        return stm->pc+2;
      break;

    case 13:
      if( !stm->APSR.bits.Z && (stm->APSR.bits.N == stm->APSR.bits.V ))
        return stm->pc+2;
      break;

    case 14:
      // UDF?
    case 15:
      // SVC
    default:
      break;
  }

  uint32_t res_pc = inst.B_t1.imm8<<1;

  if( res_pc & 0x100 )
    res_pc |= 0xfffffe00;

  res_pc += stm->pc;
  res_pc += 4;

  CORE_TRACE( stm, LOG_TRACE, "\tPC[0x%08X]\n", res_pc );

  return res_pc;
}

stm_flashaddr_t ldr_literal(struct stm_t * stm, uint16_t opcode )
{
  // 0b 0100 1xxx xxxx xxxx -LDR <Rt>,<label>
  union _ARMv6mThum2Instruction inst;
  inst.opcode = opcode;

  CORE_TRACE( stm, LOG_TRACE, "0x%08X LDR[%04x] Rt[%d] imm8[0x%x]\n", stm->pc &0xFFFFFFFE, inst.opcode, inst.LDR_literal.Rt, inst.LDR_literal.imm8 );

  stm_flashaddr_t given_pc =( stm->pc & 0xfffffffc ) +0x04 +( inst.LDR_literal.imm8 << 2 );

  stm->R[ inst.LDR_literal.Rt ]= *((uint32_t*)( stm->flash +( given_pc - stm->flash_start_adr)));

  CORE_TRACE( stm, LOG_TRACE, "\tR[%d]=0x%08X\n", inst.LDR_literal.Rt, stm->R[ inst.LDR_literal.Rt ]);

  return stm->pc + 2;
}

stm_flashaddr_t ldr_immediate(struct stm_t * stm, uint16_t opcode )
{
  union _ARMv6mThum2Instruction inst;
  inst.opcode = opcode;

  CORE_TRACE( stm, LOG_TRACE, "0x%08X LDR[%04x] Rt[%d] Rn[%d] imm5[%d]\n", stm->pc &0xFFFFFFFE, inst.opcode, inst.LDR_immediate.Rt, inst.LDR_immediate.Rn, inst.LDR_immediate.imm5 );

  uint32_t addr = stm->R[ inst.LDR_immediate.Rn ] +(inst.LDR_immediate.imm5<<2 );
  uint32_t val;

  stm_addr_read( stm, addr, &val );

  stm->R[ inst.LDR_immediate.Rt ] = val;

  CORE_TRACE( stm, LOG_TRACE, "\tR[%d]=0x%08X\n", inst.LDR_immediate.Rt, val);

  return stm->pc+2;
}
/*
 * Opcode[9802] LDR r0,[sp,#0x08]
 */
stm_flashaddr_t ldr_sp_imm_t2(struct stm_t * stm, uint16_t opcode )
{
  union _ARMv6mThum2Instruction inst;
  inst.opcode = opcode;

  CORE_TRACE( stm, LOG_TRACE, "0x%08X LDRSP[%04x] Rt[%d] imm32[0x%08X]\n", stm->pc &0xFFFFFFFE, inst.opcode, inst.LDR_sp_imm8_t2.Rt, inst.LDR_sp_imm8_t2.imm8<<2 );

  uint32_t addr = stm->R[ 13 ] +(inst.LDR_sp_imm8_t2.imm8<<2 );
  uint32_t val;

  stm_addr_read( stm, addr, &val );

  stm->R[ inst.LDR_sp_imm8_t2.Rt ] = val;

  CORE_TRACE( stm, LOG_TRACE, "\tR[%d]=0x%08X\n", inst.LDR_sp_imm8_t2.Rt, val);

  return stm->pc+2;
}
/*
 * Opcode(7820) LDRB r0,[r4,#0x00]
 */
stm_flashaddr_t ldrb_immediate(struct stm_t * stm, uint16_t opcode )
{
  union _ARMv6mThum2Instruction inst;
  inst.opcode = opcode;

  CORE_TRACE( stm, LOG_TRACE, "0x%08X LDRB[%04x] Rt[%d] Rn[%d] imm5[%d]\n", stm->pc &0xFFFFFFFE, inst.opcode, inst.LDRB_imm5_t1.Rt, inst.LDRB_imm5_t1.Rn, inst.LDRB_imm5_t1.imm5 );

//  uint32_t addr = stm->R[ inst.LDRB_imm5_t1.Rn ] +(inst.LDRB_imm5_t1.imm5<<2 );
  uint32_t addr = stm->R[ inst.LDRB_imm5_t1.Rn ] +(inst.LDRB_imm5_t1.imm5 );
  uint8_t val;

  stm_addr_readb( stm, addr, &val );

  stm->R[ inst.LDRB_imm5_t1.Rt ] = val;

  CORE_TRACE( stm, LOG_TRACE, "\tR[%d]=0x%x\n", inst.LDRB_imm5_t1.Rt, val);

  return stm->pc+2;
}
/*
 * opcode[58D2] LDR r2,[r2,r3]
 */
stm_flashaddr_t ldr_reg_t1(struct stm_t * stm, uint16_t opcode )
{
  union _ARMv6mThum2Instruction inst;
  inst.opcode = opcode;

  CORE_TRACE( stm, LOG_TRACE, "0x%08X LDR[%04x] Rt[%d] Rn[%d] Rm[%d]\n", stm->pc &0xFFFFFFFE, inst.opcode, inst.LDR_regs_t1.Rt, inst.LDR_regs_t1.Rn, inst.LDR_regs_t1.Rm );

  uint32_t addr = stm->R[ inst.LDR_regs_t1.Rn ] +stm->R[inst.LDR_regs_t1.Rm];
  uint32_t val;

  stm_addr_read( stm, addr, &val );

  stm->R[ inst.LDR_regs_t1.Rt ] = val;

  CORE_TRACE( stm, LOG_TRACE, "\tR[%d]=0x%08X\n", inst.LDR_regs_t1.Rt, val);

  return stm->pc+2;
}
/*
 * opcode[5C51] LDRB r1,[r2,r1]
 */
stm_flashaddr_t ldrb_reg_t1(struct stm_t * stm, uint16_t opcode )
{
  union _ARMv6mThum2Instruction inst;
  inst.opcode = opcode;

  CORE_TRACE( stm, LOG_TRACE, "0x%08X LDRB[%04x] Rt[%d] Rn[%d] Rm[%d]\n", stm->pc &0xFFFFFFFE, inst.opcode, inst.LDRB_regs_t1.Rt, inst.LDRB_regs_t1.Rn, inst.LDRB_regs_t1.Rm );

  uint32_t addr = stm->R[ inst.LDRB_regs_t1.Rn ] +stm->R[inst.LDRB_regs_t1.Rm];
  uint8_t val;

  stm_addr_readb( stm, addr, &val );

  stm->R[ inst.LDRB_regs_t1.Rt ] = val;

  return stm->pc+2;
}

/*
 *
 */
stm_flashaddr_t ldm_t1(struct stm_t * stm, uint16_t opcode )
{
  union _ARMv6mThum2Instruction inst;
  inst.opcode     = opcode;
  uint32_t Rn     = inst.LDM_t1.Rn;
  uint32_t rlist  = inst.LDM_t1.rlist;

  CORE_TRACE( stm, LOG_TRACE, "0x%08X LDM[%04x] Rn[%d] rl[%x]\n", stm->pc &0xFFFFFFFE, inst.opcode, Rn, rlist );

  uint32_t addr = stm->R[Rn];
  uint32_t val;

  if( rlist & 0x01 )
  {
    stm_addr_read( stm, addr, &val );
    stm->R[0] = val;
    addr +=4;

    CORE_TRACE( stm, LOG_TRACE, "\tR[0]=0x%08X\n", val);
  }

  if( rlist & 0x02 )
  {
    stm_addr_read( stm, addr, &val );
    stm->R[1] = val;
    addr +=4;

    CORE_TRACE( stm, LOG_TRACE, "\tR[1]=0x%08X\n", val);
  }

  if( rlist & 0x04 )
  {
    stm_addr_read( stm, addr, &val );
    stm->R[2] = val;
    addr +=4;

    CORE_TRACE( stm, LOG_TRACE, "\tR[2]=0x%08X\n", val);
  }

  if( rlist & 0x08 )
  {
    stm_addr_read( stm, addr, &val );
    stm->R[3] = val;
    addr +=4;

    CORE_TRACE( stm, LOG_TRACE, "\tR[3]=0x%08X\n", val);
  }

  if( rlist & 0x10 )
  {
    stm_addr_read( stm, addr, &val );
    stm->R[4] = val;
    addr +=4;

    CORE_TRACE( stm, LOG_TRACE, "\tR[4]=0x%08X\n", val);
  }

  if( rlist & 0x20 )
  {
    stm_addr_read( stm, addr, &val );
    stm->R[5] = val;
    addr +=4;

    CORE_TRACE( stm, LOG_TRACE, "\tR[5]=0x%08X\n", val);
  }

  if( rlist & 0x40 )
  {
    stm_addr_read( stm, addr, &val );
    stm->R[6] = val;
    addr +=4;

    CORE_TRACE( stm, LOG_TRACE, "\tR[6]=0x%08X\n", val);
  }

  if( rlist & 0x80 )
  {
    stm_addr_read( stm, addr, &val );
    stm->R[7] = val;

    if(!( rlist &( 1<<Rn )))
      stm->R[Rn] = addr;

    CORE_TRACE( stm, LOG_TRACE, "\tR[7]=0x%08X\n", val);
//  }else{
//    addr -=4;
//
//    if(!( rlist &( 1<<Rn )))
//      stm->R[Rn] = addr;
//  }
  }

  if(!( rlist &( 1<<Rn )))
    stm->R[Rn] = addr;

  CORE_TRACE( stm, LOG_TRACE, "\tR%d(@0x%08X)\n", Rn,  addr);

  return stm->pc+2;
}
/*
 * opcode[BD30] POP {r4-r5,pc}
 */
stm_flashaddr_t pop_t1(struct stm_t * stm, uint16_t opcode )
{
  union _ARMv6mThum2Instruction inst;
  inst.opcode     = opcode;
  uint32_t P      = inst.POP_t1.P;
  uint32_t rlist  = inst.POP_t1.rlist;

  CORE_TRACE( stm, LOG_TRACE, "0x%08X lvl[%d] POP[%04x] P[%d] rl[%x]\n", stm->pc &0xFFFFFFFE, --uiSubroutineLvl, inst.opcode, P, rlist );

  uint32_t addr = stm->R[13];
  uint32_t val;

  if( rlist & 0x01 )
  {
    addr +=4;
    stm_addr_read( stm, addr, &val );
    stm->R[0] = val;

    CORE_TRACE( stm, LOG_TRACE, "\tR[0]=0x%08X\n", val);
  }

  if( rlist & 0x02 )
  {
    addr +=4;
    stm_addr_read( stm, addr, &val );
    stm->R[1] = val;

    CORE_TRACE( stm, LOG_TRACE, "\tR[1]=0x%08X\n", val);
  }

  if( rlist & 0x04 )
  {
    addr +=4;
    stm_addr_read( stm, addr, &val );
    stm->R[2] = val;

    CORE_TRACE( stm, LOG_TRACE, "\tR[2]=0x%08X\n", val);
  }

  if( rlist & 0x08 )
  {
    addr +=4;
    stm_addr_read( stm, addr, &val );
    stm->R[3] = val;

    CORE_TRACE( stm, LOG_TRACE, "\tR[3]=0x%08X\n", val);
  }

  if( rlist & 0x10 )
  {
    addr +=4;
    stm_addr_read( stm, addr, &val );
    stm->R[4] = val;

    CORE_TRACE( stm, LOG_TRACE, "\tR[4]=0x%08X\n", val);
  }

  if( rlist & 0x20 )
  {
    addr +=4;
    stm_addr_read( stm, addr, &val );
    stm->R[5] = val;

    CORE_TRACE( stm, LOG_TRACE, "\tR[5]=0x%08X\n", val);
  }

  if( rlist & 0x40 )
  {
    addr +=4;
    stm_addr_read( stm, addr, &val );
    stm->R[6] = val;

    CORE_TRACE( stm, LOG_TRACE, "\tR[6]=0x%08X\n", val);
  }
  if( rlist & 0x80 )
  {
    addr +=4;
    stm_addr_read( stm, addr, &val );
    stm->R[7] = val;

    CORE_TRACE( stm, LOG_TRACE, "\tR[7]=0x%08X\n", val);
  }
  if( P )
  {
    addr +=4;
    stm_addr_read( stm, addr, &val );

    if(( val & 0xfffffff0 )== 0xfffffff0 )
    {
      // todo: nvic
      // initiate interrupt leaving process
        nvic_run_interrupt_leaving( stm->mcu, val );
    }else{
      stm->pc = val;
    }

    CORE_TRACE( stm, LOG_TRACE, "\tPC=0x%08X\n", val);
  }
  else
    val = stm->pc+2;

  stm->R[13] = addr;

  CORE_TRACE( stm, LOG_TRACE, "\tSP(@0x%08X)\n", addr);

  return val;
}
/*
 *
 */
stm_flashaddr_t stm_t1(struct stm_t * stm, uint16_t opcode )
{
  union _ARMv6mThum2Instruction inst;
  inst.opcode     = opcode;
  uint32_t Rn     = inst.STM_t1.Rn;
  uint32_t rlist  = inst.STM_t1.rlist;

  CORE_TRACE( stm, LOG_TRACE, "0x%08X STM[%04x] Rn[%d] rl[%x]\n", stm->pc &0xFFFFFFFE, opcode, Rn, rlist );

  uint32_t addr = stm->R[Rn];
  uint32_t val;

  if( rlist & 0x01 )
  {
    val  = stm->R[0];
    stm_addr_write( stm, addr, val );

    CORE_TRACE( stm, LOG_TRACE, "\t@0x%08X=R0(0x%08X)\n", addr, val);
    addr +=4;
  }

  if( rlist & 0x02 )
  {
    val  = stm->R[1];
    stm_addr_write( stm, addr, val );

    CORE_TRACE( stm, LOG_TRACE, "\t@0x%08X=R1(0x%08X)\n", addr, val);
    addr +=4;
  }

  if( rlist & 0x04 )
  {
    val  = stm->R[2];
    stm_addr_write( stm, addr, val );

    CORE_TRACE( stm, LOG_TRACE, "\t@0x%08X=R2(0x%08X)\n", addr, val);

    addr +=4;
  }

  if( rlist & 0x08 )
  {
    val  = stm->R[3];
    stm_addr_write( stm, addr, val );

    CORE_TRACE( stm, LOG_TRACE, "\t@0x%08X=R3(0x%08X)\n", addr, val);

    addr +=4;
  }

  if( rlist & 0x10 )
  {
    val  = stm->R[4];
    stm_addr_write( stm, addr, val );

    CORE_TRACE( stm, LOG_TRACE, "\t@0x%08X=R4(0x%08X)\n", addr, val);

    addr +=4;
  }

  if( rlist & 0x20 )
  {
    val  = stm->R[5];
    stm_addr_write( stm, addr, val );

    CORE_TRACE( stm, LOG_TRACE, "\t@0x%08X=R5(0x%08X)\n", addr, val);

    addr +=4;
  }

  if( rlist & 0x40 )
  {
    val  = stm->R[6];
    stm_addr_write( stm, addr, val );

    CORE_TRACE( stm, LOG_TRACE, "\t@0x%08X=R6(0x%08X)\n", addr, val);

    addr +=4;
  }

  if( rlist & 0x80 )
  {
    val  = stm->R[7];
    stm_addr_write( stm, addr, val );

    if(!( rlist &( 1<<Rn )))
      stm->R[Rn] = addr;

    CORE_TRACE( stm, LOG_TRACE, "\t@0x%08X=R7(0x%08X)\n", addr, val);

    addr +=4;
  }

  if(!( rlist &( 1<<Rn )))
      stm->R[Rn] = addr;

  CORE_TRACE( stm, LOG_TRACE, "\tR%d(@0x%08X)\n", Rn,  addr);

  return stm->pc+2;
}

/*
 * PUSH
 */
stm_flashaddr_t push_t1(struct stm_t * stm, uint16_t opcode )
{
  union _ARMv6mThum2Instruction inst;
  inst.opcode     = opcode;
  uint32_t M    = inst.PUSH_t1.M;
  uint32_t rlist  = inst.PUSH_t1.rlist;

  CORE_TRACE( stm, LOG_TRACE, "0x%08X lvl[%d] PUSH[%04x] M[%d] rl[%x]\n", stm->pc &0xFFFFFFFE, uiSubroutineLvl++, opcode, M, rlist );

  uint32_t addr = stm->R[13];

  if( M )
  {
    stm_addr_write( stm, addr, stm->R[ 14 ]);
    CORE_TRACE( stm, LOG_TRACE, "\t@0x%08X=LR(0x%08X)\n", addr, stm->R[ 14 ]);
    addr -=4;
  }
  if( rlist & 0x80 )
  {
    stm_addr_write( stm, addr, stm->R[7] );

    CORE_TRACE( stm, LOG_TRACE, "\t@0x%08X=R7(0x%08X)\n", addr, stm->R[ 7 ]);

    addr -=4;
  }
  if( rlist & 0x40 )
  {
    stm_addr_write( stm, addr, stm->R[ 6 ] );

    CORE_TRACE( stm, LOG_TRACE, "\t@0x%08X=R6(0x%08X)\n", addr, stm->R[ 6 ]);

    addr -=4;
  }
  if( rlist & 0x20 )
  {
    stm_addr_write( stm, addr, stm->R[5] );

    CORE_TRACE( stm, LOG_TRACE, "\t@0x%08X=R5(0x%08X)\n", addr, stm->R[ 5 ]);

    addr -=4;
  }
  if( rlist & 0x10 )
  {
    stm_addr_write( stm, addr, stm->R[ 4 ]);

    CORE_TRACE( stm, LOG_TRACE, "\t@0x%08X=R4(0x%08X)\n", addr, stm->R[ 4 ]);

    addr -=4;
  }
  if( rlist & 0x08 )
  {
    stm_addr_write( stm, addr, stm->R[3] );

    CORE_TRACE( stm, LOG_TRACE, "\t@0x%08X=R3(0x%08X)\n", addr, stm->R[ 3 ]);

    addr -=4;
  }
  if( rlist & 0x04 )
  {
    stm_addr_write( stm, addr, stm->R[2] );

    CORE_TRACE( stm, LOG_TRACE, "\t@0x%08X=R2(0x%08X)\n", addr, stm->R[ 2 ]);

    addr -=4;
  }
  if( rlist & 0x02 )
  {
    stm_addr_write( stm, addr, stm->R[ 1 ]);

    CORE_TRACE( stm, LOG_TRACE, "\t@0x%08X=R1(0x%08X)\n", addr, stm->R[ 1 ]);
    addr -=4;
  }
  if( rlist & 0x01 )
  {
    stm_addr_write( stm, addr, stm->R[ 0 ]);

    CORE_TRACE( stm, LOG_TRACE, "\t@0x%08X=R0(0x%08X)\n", stm->R[ 0 ]);
    addr -=4;
  }

  stm->R[ 13 ]= addr;

  CORE_TRACE( stm, LOG_TRACE, "\tSP(@0x%08X)\n", addr);

  return stm->pc+2;
}

stm_flashaddr_t mov_register_t1(struct stm_t * stm, uint16_t opcode )
{
  union _ARMv6mThum2Instruction inst;
  inst.opcode   = opcode;
  uint32_t Rm   = inst.MOV_t1.Rm;
  uint32_t Rd   = inst.MOV_t1.Rd;
  Rd += (inst.MOV_t1.D << 3);

  CORE_TRACE( stm, LOG_TRACE, "0x%08X MOV[%04x] Rd[%d] Rm[%d]\n", stm->pc &0xFFFFFFFE, opcode, Rd, Rm );

  if( Rd == 15 )
  {
    stm->pc = stm->R[ Rm ];

    CORE_TRACE( stm, LOG_TRACE, "\tPC=0x%08X\n", Rd, stm->R[ Rm ]);
    return stm->pc;
  }
  else
  {
    stm->R[ Rd ] = stm->R[ Rm ];

    CORE_TRACE( stm, LOG_TRACE, "\tR[%d]=0x%08X\n", Rd, stm->R[ Rm ]);
    return stm->pc+2;
  }
}

/*
 * Opcode(A80E) ADD r0,sp,#0x38
 */
stm_flashaddr_t add_sp_immediate_T1(struct stm_t * stm, uint16_t opcode )
{
  union _ARMv6mThum2Instruction inst;
  inst.opcode   = opcode;
  uint32_t Rd = inst.ADDS_sp_imm_t1.Rd;
  uint32_t imm32 = inst.ADDS_sp_imm_t1.imm8<<2;

  CORE_TRACE( stm, LOG_TRACE, "0x%08X ADD SP[%04x] Rd[%d] imm32[0x%0x]\n", stm->pc &0xFFFFFFFE, opcode, Rd, imm32 );

  stm->R[ Rd ]= stm->R[ 13 ] +imm32;

  CORE_TRACE( stm, LOG_TRACE, "\t@SP(0x%08X)\n", stm->R[ 13 ]);
  CORE_TRACE( stm, LOG_TRACE, "\tR[%d]=0x%08X\n", Rd, stm->R[ Rd ]);

  return stm->pc+2;
}
/*
 * Opcode(XXXX) ADD SP,SP,#<imm7>
 */
stm_flashaddr_t add_sp_immediate_T2(struct stm_t * stm, uint16_t opcode )
{
  union _ARMv6mThum2Instruction inst;
  inst.opcode   = opcode;
  uint32_t imm32 = inst.ADDS_sp_imm_t2.imm7<<2;

  CORE_TRACE( stm, LOG_TRACE, "0x%08X ADD[%04x] SP imm32[0x%0x]\n", stm->pc &0xFFFFFFFE, opcode, imm32 );

  stm->R[ 13 ]= stm->R[ 13 ] +imm32;

  CORE_TRACE( stm, LOG_TRACE, "\t@SP(0x%08X)\n", stm->R[ 13 ]);

  return stm->pc+2;
}

/*
 * Opcode[B093] SUB sp,sp,#0x4C
 */
stm_flashaddr_t sub_sp_immediate_T1(struct stm_t * stm, uint16_t opcode )
{
  union _ARMv6mThum2Instruction inst;
  inst.opcode     = opcode;
  uint32_t imm32    = ( inst.SUB_SP_imm_t1.imm7 )<<2;

  CORE_TRACE( stm, LOG_TRACE, "0x%08X SUBS[%04x] SP imm7[0x%x]\n", stm->pc &0xFFFFFFFE, opcode, inst.SUB_SP_imm_t1.imm7 );

  stm->R[13] = stm->R[13] -imm32;

  CORE_TRACE( stm, LOG_TRACE, "\tSP=0x%08X\n", stm->R[ 13 ]);
  TRACE_ASPR();

  return stm->pc+2;
}

/*
 *
 */
stm_flashaddr_t orr_register_T1(struct stm_t * stm, uint16_t opcode )
{
  union _ARMv6mThum2Instruction inst;
  inst.opcode   = opcode;
  uint32_t val  = stm->R[ inst.ORRS_t1.Rm ];
  uint32_t Rdn  = inst.ORRS_t1.Rdn;

  CORE_TRACE( stm, LOG_TRACE, "0x%08X ORRS[%04x] Rdn[%d] Rm[%d]\n", stm->pc &0xFFFFFFFE, opcode, Rdn, inst.ORRS_t1.Rm );

  val |=  stm->R[ Rdn ];
  stm->R[ Rdn ] = val;

  // APSR update
  if( val & 0x80000000 )
    stm->APSR.bits.N = 1;
  else
    stm->APSR.bits.N = 0;

  if( val )
    stm->APSR.bits.Z = 0;
  else
    stm->APSR.bits.Z = 1;

//  stm->APSR.bits.C = 0;

  CORE_TRACE( stm, LOG_TRACE, "\tR[%d]=0x%08X\n", Rdn, val );
  TRACE_ASPR();

  return stm->pc+2;
}

#if 0
/*
 *
 */
stm_flashaddr_t eors_register_T1(struct stm_t * stm, uint16_t opcode )
{
  union _ARMv6mThum2Instruction inst;
  inst.opcode   = opcode;
  uint32_t val  = stm->R[ inst.ORRS_t1.Rm ];
  uint32_t Rn = inst.EORS_t1.Rn;

  CORE_TRACE( stm, LOG_TRACE, "0x%08X EORS[%04x] Rn[%d] Rm[%d]\n", stm->pc &0xFFFFFFFE, opcode, Rn, inst.EORS_t1.Rm );

  val ^=  stm->R[ Rn ];
  stm->R[ Rn ] = val;

  // APSR update
  if( val & 0x80000000 )
    stm->APSR.bits.N = 1;
  else
    stm->APSR.bits.N = 0;

  if( val )
    stm->APSR.bits.Z = 0;
  else
    stm->APSR.bits.Z = 1;

//  stm->APSR.bits.C = 0;

  TRACE_ASPR();
  // CORE_TRACE( stm, LOG_TRACE, "\tASPR[N.Z.C.V]=[%01d.%01d.%01d.%01d]\n", stm->APSR.bits.N, stm->APSR.bits.Z, stm->APSR.bits.C, stm->APSR.bits.V );

  return stm->pc+2;
}
#endif

#if 0
stm_flashaddr_t ands_register_T1(struct stm_t * stm, uint16_t opcode )
{
  union _ARMv6mThum2Instruction inst;
  inst.opcode   = opcode;
  uint32_t val  = stm->R[ inst.ANDS_t1.Rm ];
  uint32_t Rdn  = inst.ANDS_t1.Rdn;

  CORE_TRACE( stm, LOG_TRACE, "0x%08X ANDS[%04x] Rn[%d] Rm[%d]\n", stm->pc &0xFFFFFFFE, opcode, Rdn, inst.ANDS_t1.Rm );

  val &=  stm->R[ Rdn ];
  stm->R[ Rdn ] = val;

  // APSR update
  if( val & 0x80000000 )
    stm->APSR.bits.N = 1;
  else
    stm->APSR.bits.N = 0;

  if( val )
    stm->APSR.bits.Z = 0;
  else
    stm->APSR.bits.Z = 1;

//  stm->APSR.bits.C = 0;

  CORE_TRACE( stm, LOG_TRACE, "\tR[%d]=0x%08X\n", Rdn, val);
  TRACE_ASPR();
  // CORE_TRACE( stm, LOG_TRACE, "\tASPR[N.Z.C.V]=[%01d.%01d.%01d.%01d]\n", stm->APSR.bits.N, stm->APSR.bits.Z, stm->APSR.bits.C, stm->APSR.bits.V );

  return stm->pc+2;
}
#endif

/*
 *
 */
stm_flashaddr_t str_immediate_T1(struct stm_t * stm, uint16_t opcode )
{
  union _ARMv6mThum2Instruction inst;
  inst.opcode = opcode;

  CORE_TRACE( stm, LOG_TRACE, "0x%08X STR[%04x] Rt[%d] Rn[%d] imm5[%d]\n", stm->pc &0xFFFFFFFE, inst.opcode, inst.STR_immediate_t1.Rt, inst.STR_immediate_t1.Rn, inst.STR_immediate_t1.imm5 );

  uint32_t addr = stm->R[ inst.STR_immediate_t1.Rn ] +(inst.STR_immediate_t1.imm5<<2 );
  uint32_t val = stm->R[ inst.STR_immediate_t1.Rt ];

  if( addr == 0x50000418 )
  {
      TRACE_ASPR();
  }

  stm_addr_write( stm, addr, val );

  CORE_TRACE( stm, LOG_TRACE, "\tSRAM[0x%08X]=0x%08X\n", addr, val);

  return stm->pc+2;
}
/*
 * Opcode(9001) STR r0,[sp,#0x04]
 */
stm_flashaddr_t str_sp_immediate_T2(struct stm_t * stm, uint16_t opcode )
{
  union _ARMv6mThum2Instruction inst;
  inst.opcode = opcode;

  CORE_TRACE( stm, LOG_TRACE, "0x%08X STR[%04x] Rt[%d] SP imm32[0x%x]\n", stm->pc &0xFFFFFFFE, inst.opcode, inst.STR_immediate_t2.Rt, inst.STR_immediate_t2.imm8<<2 );

  uint32_t addr = stm->R[ 13 ] +(inst.STR_immediate_t2.imm8<<2 );
  uint32_t val = stm->R[ inst.STR_immediate_t2.Rt ];

  stm_addr_write( stm, addr, val );

  CORE_TRACE( stm, LOG_TRACE, "\tSRAM[0x%08X]=0x%08X\n", addr, val);

  return stm->pc+2;
}
/*
 * Opcode(7002) STRB r2,[r0,#0x00]
 */
stm_flashaddr_t strb_immediate_T1(struct stm_t * stm, uint16_t opcode )
{
  union _ARMv6mThum2Instruction inst;
  inst.opcode = opcode;

  CORE_TRACE( stm, LOG_TRACE, "0x%08X STRB[%04x] Rt[%d] Rn[%d] imm5[%d]\n", stm->pc &0xFFFFFFFE, inst.opcode, inst.STR_immediate_t1.Rt, inst.STR_immediate_t1.Rn, inst.STR_immediate_t1.imm5 );

  uint32_t addr = stm->R[ inst.STR_immediate_t1.Rn ] +(inst.STR_immediate_t1.imm5 );
  uint32_t val = stm->R[ inst.STR_immediate_t1.Rt ];

  stm_addr_writeb( stm, addr, val );

  CORE_TRACE( stm, LOG_TRACE, "\tSRAM[0x%08X]=0x%02x\n", addr, val);

  return stm->pc+2;
}

/*
 *
 */
stm_flashaddr_t str_T1(struct stm_t * stm, uint16_t opcode )
{
  union _ARMv6mThum2Instruction inst;
  inst.opcode = opcode;

  CORE_TRACE( stm, LOG_TRACE, "0x%08X STR[%04x] !!Untested Rt[%d] Rn[%d] Rm[%d]\n", stm->pc &0xFFFFFFFE, inst.opcode, inst.STR_t1.Rt, inst.STR_t1.Rn, inst.STR_t1.Rm );

  uint32_t addr = stm->R[ inst.STR_t1.Rn ] +(inst.STR_t1.Rm );
  uint32_t val = stm->R[ inst.STR_t1.Rt ];

  stm_addr_write( stm, addr, val );

  CORE_TRACE( stm, LOG_TRACE, "\tSRAM[0x%08X]=0x%08X\n", addr, val);

  return stm->pc+2;
}

/*
 *
 */
stm_flashaddr_t lsr_immediate_T1(struct stm_t * stm, uint16_t opcode )
{
  union _ARMv6mThum2Instruction inst;
  inst.opcode = opcode;
  uint32_t Rd = inst.LSRS_immediate_t1.Rd;
  uint32_t Rm = inst.LSRS_immediate_t1.Rm;
  uint32_t imm5 = inst.LSRS_immediate_t1.imm5;
  uint32_t val = stm->R[ Rm ];

  CORE_TRACE( stm, LOG_TRACE, "0x%08X LSR[%04x] Rd[%d] Rm[%d] imm5[%d]\n", stm->pc &0xFFFFFFFE, inst.opcode, Rd, Rm, imm5 );

  if( 0 < imm5  )
  {
    uint32_t res =( val>>imm5 );
    stm->R[ Rd ] = res;

    // APSR update
    stm->APSR.bits.N = 0;

    if( res )
      stm->APSR.bits.Z = 0;
    else
      stm->APSR.bits.Z = 1;

    if( val & ( 0x01 << (imm5-1) ) )
      stm->APSR.bits.C = 1;
    else
      stm->APSR.bits.C = 0;
  }
  else
  {
    // TODO Undescriber behaviour for imm5 == 0
    stm->R[ Rd ] = val;
  }

  CORE_TRACE( stm, LOG_TRACE, "\tR[%d]=0x%08X\n", Rd, val);
  TRACE_ASPR();
  // CORE_TRACE( stm, LOG_TRACE, "\tASPR[N.Z.C.V]=[%01d.%01d.%01d.%01d]\n", stm->APSR.bits.N, stm->APSR.bits.Z, stm->APSR.bits.C, stm->APSR.bits.V );

  return stm->pc+2;
}
/*
 *
 */
// TODO:Not tested well
stm_flashaddr_t lsr_T1(struct stm_t * stm, uint16_t opcode )
{
  union _ARMv6mThum2Instruction inst;
  inst.opcode       = opcode;
  uint32_t Rdn      = inst.LSRS_t1.Rdn;
  uint32_t Rm       = inst.LSRS_t1.Rm;
  uint32_t Rm_val   = stm->R[ Rm ];
  uint32_t val      = stm->R[ Rdn ];

  CORE_TRACE( stm, LOG_TRACE, "0x%08X LSR[%04x] Rdn[%d] Rm[%d] imm5[%d]\n", stm->pc &0xFFFFFFFE, inst.opcode, Rdn, Rm, inst.LSRS_t1.opcode );

  if( 0 < Rm_val  )
  {
    uint32_t res        =( val>>Rm_val );
    stm->R[ Rdn ]       = res;

    // APSR update
    stm->APSR.bits.N    = 0;

    if( res )
      stm->APSR.bits.Z  = 0;
    else
      stm->APSR.bits.Z  = 1;

    if( val & ( 0x01 << (Rm_val-1) ) )
      stm->APSR.bits.C  = 1;
    else
      stm->APSR.bits.C  = 0;
  }
  else
  {
    stm->R[ Rdn ]       = val;
  }

  CORE_TRACE( stm, LOG_TRACE, "\tR[%d]=0x%08X\n", Rdn, Rm_val);
  TRACE_ASPR();
  // CORE_TRACE( stm, LOG_TRACE, "\tASPR[N.Z.C.V]=[%01d.%01d.%01d.%01d]\n", stm->APSR.bits.N, stm->APSR.bits.Z, stm->APSR.bits.C, stm->APSR.bits.V );

  return stm->pc+2;
}
/*
 *
 */
stm_flashaddr_t lsl_immediate_T1(struct stm_t * stm, uint16_t opcode )
{
  union _ARMv6mThum2Instruction inst;
  inst.opcode = opcode;
  uint32_t Rd = inst.LSLS_immediate_t1.Rd;
  uint32_t Rm = inst.LSLS_immediate_t1.Rm;
  uint32_t imm5 = inst.LSLS_immediate_t1.imm5;
  uint32_t val = stm->R[ Rm ];

  CORE_TRACE( stm, LOG_TRACE, "0x%08X LSL[%04x] Rd[%d] Rm[%d] imm5[%d]\n", stm->pc &0xFFFFFFFE, inst.opcode, Rd, Rm, imm5 );

  if( 0 < imm5 )
  {
    uint32_t res =( val<<imm5 );
    stm->R[ Rd ] = res;

    // APSR update
    if( res & 0x80000000 )
      stm->APSR.bits.N = 1;
    else
      stm->APSR.bits.N = 0;

    if( !res )
      stm->APSR.bits.Z = 1;
    else
      stm->APSR.bits.Z = 0;

    if( val & ( 0x01 << (32-imm5) ) )
      stm->APSR.bits.C = 1;
    else
      stm->APSR.bits.C = 0;
  }
  else
  {
    // TODO Undescribed behaviour for imm5 == 0
    stm->R[ Rd ] = val;

    // APSR update
    if( val & 0x80000000 )
      stm->APSR.bits.N = 1;
    else
      stm->APSR.bits.N = 0;

    if( val )
      stm->APSR.bits.Z = 0;
    else
      stm->APSR.bits.Z = 1;

    stm->APSR.bits.C = 0;
  }

  CORE_TRACE( stm, LOG_TRACE, "\tR[%d]=0x%08X\n", Rd, stm->R[ Rd ]);
  TRACE_ASPR();
  // CORE_TRACE( stm, LOG_TRACE, "\tASPR[N.Z.C.V]=[%01d.%01d.%01d.%01d]\n", stm->APSR.bits.N, stm->APSR.bits.Z, stm->APSR.bits.C, stm->APSR.bits.V );

  return stm->pc+2;
}
/*
 * opcode[4095] LSLS r5,r5,r2
 */
stm_flashaddr_t lsl_T1(struct stm_t * stm, uint16_t opcode )
{
  union _ARMv6mThum2Instruction inst;
  inst.opcode = opcode;
  uint32_t Rdn = inst.LSLS_reg_t1.Rdn;
  uint32_t Rm = inst.LSLS_reg_t1.Rm;
  uint32_t val = stm->R[ Rdn ];
  uint32_t Rm_val = stm->R[ Rm ];

  CORE_TRACE( stm, LOG_TRACE, "0x%08X LSLS[%04x] Rdn[%d] Rm[%d]\n", stm->pc &0xFFFFFFFE, inst.opcode, Rdn, Rm );

  if( 0 < Rm_val )
  {
    uint32_t res =( val<<Rm_val );
    stm->R[ Rdn ] = res;

    // APSR update
    if( res & 0x80000000 )
      stm->APSR.bits.N = 1;
    else
      stm->APSR.bits.N = 0;

    if( !res )
      stm->APSR.bits.Z = 1;
    else
      stm->APSR.bits.Z = 0;

    if( val & ( 0x01 << (32-Rm_val) ) )
      stm->APSR.bits.C = 1;
    else
      stm->APSR.bits.C = 0;
  }
  else
  {
    // TODO Undescribed behaviour for imm5 == 0
    stm->R[ Rdn ] = val;

    // APSR update
    if( val & 0x80000000 )
      stm->APSR.bits.N = 1;
    else
      stm->APSR.bits.N = 0;

    if( val )
      stm->APSR.bits.Z = 0;
    else
      stm->APSR.bits.Z = 1;

    stm->APSR.bits.C = 0;
  }

  CORE_TRACE( stm, LOG_TRACE, "\tR[%d]=0x%08X\n", Rdn, stm->R[ Rdn ]);
  TRACE_ASPR();
  // CORE_TRACE( stm, LOG_TRACE, "\tASPR[N.Z.C.V]=[%01d.%01d.%01d.%01d]\n", stm->APSR.bits.N, stm->APSR.bits.Z, stm->APSR.bits.C, stm->APSR.bits.V );

  return stm->pc+2;
}
/*
 *
 */
stm_flashaddr_t bics_register_T1(struct stm_t * stm, uint16_t opcode )
{
  union _ARMv6mThum2Instruction inst;
  inst.opcode   = opcode;
  uint32_t Rdn  = inst.BICS_t1.Rdn;
  uint32_t Rm   = inst.BICS_t1.Rm;
  uint32_t res  = stm->R[ Rdn ];

  CORE_TRACE( stm, LOG_TRACE, "0x%08X BICS[%04x] Rn[%d] Rm[%d]\n", stm->pc &0xFFFFFFFE, opcode, Rdn, Rm );

  res &=  ~(stm->R[ Rm ]);
  stm->R[ Rdn ] = res;

  // APSR update
  if( res & 0x80000000 )
    stm->APSR.bits.N = 1;
  else
    stm->APSR.bits.N = 0;

  if( res )
    stm->APSR.bits.Z = 0;
  else
    stm->APSR.bits.Z = 1;

//  stm->APSR.bits.C = 0;

  CORE_TRACE( stm, LOG_TRACE, "\tR[%d]=0x%08X\n", Rdn, res);
  TRACE_ASPR();
  // CORE_TRACE( stm, LOG_TRACE, "\tASPR[N.Z.C.V]=[%01d.%01d.%01d.%01d]\n", stm->APSR.bits.N, stm->APSR.bits.Z, stm->APSR.bits.C, stm->APSR.bits.V );

  return stm->pc+2;
}

/*
 *
 */
stm_flashaddr_t cmp_register_T1(struct stm_t * stm, uint16_t opcode )
{
  union _ARMv6mThum2Instruction inst;
  inst.opcode   = opcode;
  uint32_t Rn   = inst.CMP_t1.Rn;
  uint32_t Rm   = inst.CMP_t1.Rm;
  uint32_t Rm_val = stm->R[ Rm ];

  CORE_TRACE( stm, LOG_TRACE, "0x%08X CMP[%04x] Rn[%d] Rm[%d]\n", stm->pc &0xFFFFFFFE, opcode, Rn, Rm );

  uint32_t result;

  ADD_WITH_CARRY( result, stm->APSR.bits.C, stm->APSR.bits.V, stm->R[ Rn ], ~Rm_val, 1 );

  // APSR update
  UPDATE_Z_N( result, stm->APSR.bits.Z, stm->APSR.bits.N );

  TRACE_ASPR();

  return stm->pc+2;
}

/*
 *
 */
stm_flashaddr_t cmp_immediate_T1(struct stm_t * stm, uint16_t opcode )
{
  union _ARMv6mThum2Instruction inst;
  inst.opcode   = opcode;
  uint32_t Rn     = inst.CMP_imm_t1.Rn;
  uint32_t imm8   = inst.CMP_imm_t1.imm8;
//  uint32_t Rn_val = stm->R[ Rn ];

  CORE_TRACE( stm, LOG_TRACE, "0x%08X CMP[%04x] Rn[%d] IM8[0x%x]\n", stm->pc &0xFFFFFFFE, opcode, Rn, imm8 );

  uint32_t result;

  ADD_WITH_CARRY( result, stm->APSR.bits.C, stm->APSR.bits.V, stm->R[ Rn ], ~imm8, 1 );

  // APSR update
  UPDATE_Z_N( result, stm->APSR.bits.Z, stm->APSR.bits.N );

  TRACE_ASPR();

  return stm->pc+2;
}

/**
 *
 */
stm_flashaddr_t nop_T1(struct stm_t * stm, uint16_t opcode )
{
  CORE_TRACE( stm, LOG_TRACE, "0x%08X NOP[%04x] <-NEW\n", stm->pc &0xFFFFFFFE, opcode );

  return stm->pc+2;
}
/*
 * Opcode(B2D2) UXTB r2,r2
 */
stm_flashaddr_t uxtb_T1(struct stm_t * stm, uint16_t opcode )
{
  union _ARMv6mThum2Instruction inst;
  inst.opcode   = opcode;
  uint32_t Rd = inst.UXTB_t1.Rd;

  CORE_TRACE( stm, LOG_TRACE, "0x%08X UXTB[%04x] Rd[%d] Rm[%d]\n", stm->pc &0xFFFFFFFE, opcode, Rd, inst.UXTB_t1.Rm );

  stm->R[ Rd ]= (stm->R[ inst.UXTB_t1.Rm ])&0x0f;

  CORE_TRACE( stm, LOG_TRACE, "\tR[%d]=0x%08X\n", Rd, stm->R[ Rd ]);

  return stm->pc+2;
}




/*
 * SECOND PACK of FUNC
 */
//stm_flashaddr_t blx(struct stm_t * stm, uint16_t opcode )
//{
//	union _ARMv6mThum2Instruction inst;
//	inst.opcode = opcode;
//
//	CORE_TRACE( stm, LOG_TRACE, "0x%08X BLX[%04x] Rm[%d] rest[%d]\n", stm->pc &0xFFFFFFFE, inst.opcode, inst.BLX.Rm, inst.BLX.rest );
//
//	// save return address into LR
//	stm->R[14] = (stm->pc + 2)| 0x00000001;
//
//	CORE_TRACE( stm, LOG_TRACE, "\tLR[0x%08X] PC[0x%08X]\n", stm->R[14], stm->R[inst.BLX.Rm] & 0xfffffffe );
//
//	// new pc value
//	return stm->R[inst.BLX.Rm] | 0x00000001;
//}
//
//stm_flashaddr_t bx_t1(struct stm_t * stm, uint16_t opcode )
//{
//	union _ARMv6mThum2Instruction inst;
//	inst.opcode = opcode;
//
//	CORE_TRACE( stm, LOG_TRACE, "0x%08X BX[%04x] Rm[%d]\n", stm->pc &0xFFFFFFFE, inst.opcode, inst.BX_t1.Rm );
//  CORE_TRACE( stm, LOG_TRACE, "\tPC[0x%08X]\n", stm->R[inst.BX_t1.Rm ] & 0xfffffffe );
//
//  // check for interrupt leaving
//  if( inst.BX_t1.Rm == 14 )
//  {
//      // todo : nvic interrupt return
//      uint32_t val = stm->R[inst.BX_t1.Rm ];
//      if(( val & 0xfffffff0 )== 0xfffffff0 )
//      {
//        // Try to leave interrupt
//        nvic_run_interrupt_leaving( stm->mcu, val );
//      }
//  }
//
//	// new pc value
//	return stm->R[inst.BX_t1.Rm ];
//}
/*
 *
 */
//stm_flashaddr_t bl_t1(struct stm_t * stm, uint16_t opcode, uint16_t opcode2 )
//{
//	union _ARMv6mThum2Instruction inst_w1;
//	inst_w1.opcode = opcode;
//
//	union _ARMv6mThum2Instruction inst_w2;
//	inst_w2.opcode = _stm_flash_read16le( stm, stm->pc+2);
//
//	union imm32_offset
//	{
//		uint32_t 					val;
//		struct imm32
//		{
//			unsigned zero 	: 1;
//			unsigned imm11	: 11;
//			unsigned imm10	: 10;
//			unsigned I2			: 1;
//			unsigned I1			: 1;
//			unsigned S			: 1;
//			unsigned signExt: 7;
//		}imm32;
//	}imm32_offset;
//
//	imm32_offset.val = 0;
//	imm32_offset.imm32.imm11 	= inst_w2.BL_t1_part2.imm11;
//	imm32_offset.imm32.imm10	= inst_w1.BL_t1_part1.imm10;
//	imm32_offset.imm32.I2			= ~(inst_w1.BL_t1_part1.S ^ inst_w2.BL_t1_part2.J2);
//	imm32_offset.imm32.I1			= ~(inst_w1.BL_t1_part1.S ^ inst_w2.BL_t1_part2.J1);
//	imm32_offset.imm32.S			= inst_w1.BL_t1_part1.S;
//
//	if( imm32_offset.imm32.S )
//		imm32_offset.imm32.signExt = 0x7f ;
//
//	CORE_TRACE( stm, LOG_TRACE, "0x%08X BL[%04x] OFS[0x%08X] \n", stm->pc &0xFFFFFFFE, inst_w1.opcode, imm32_offset );
//
//	uint32_t next_instr_addr = stm->pc +4;
//
//	// save return address into LR
//	stm->R[14] = next_instr_addr | 0x00000001;
//
//  CORE_TRACE( stm, LOG_TRACE, "\tLR[0x%08X] PC[0x%08X]\n", stm->R[14], (next_instr_addr +imm32_offset.val) & 0xfffffffe );
//
//	// new pc value
//	return next_instr_addr +imm32_offset.val;
//}
/*
 * UDF #<imm8>
 */
stm_flashaddr_t udf_t1( struct stm_t * stm, uint16_t opcode )
{
  CORE_TRACE( stm, LOG_TRACE, "0x%08X UDF T1-Not implemented [%04x]\n", stm->pc &0xFFFFFFFE, opcode );

  stm->state = cpu_Crashed;

  return stm->pc;
}

/*
 * UDF.W #<imm16>
 */
stm_flashaddr_t udf_t2( struct stm_t * stm, uint16_t opcode )
{
  CORE_TRACE( stm, LOG_TRACE, "0x%08X UDF T2-Not implemented [%04x]\n", stm->pc &0xFFFFFFFE, opcode );

  stm->state = cpu_Crashed;

  return stm->pc;
}

/*
 * SVC #<imm8>
 */
stm_flashaddr_t svc_t1( struct stm_t * stm, uint16_t opcode )
{
  CORE_TRACE( stm, LOG_TRACE, "0x%08X SVC T1-Not implemented [%04x]\n", stm->pc &0xFFFFFFFE, opcode );

  stm->state = cpu_Crashed;

  return stm->pc;
}

/*
 *
 */
//stm_flashaddr_t msr_t1(struct stm_t * stm, uint16_t opcode, uint16_t opcode2 )
//{
//	union msr_hw1
//	{
//		uint16_t val;
//		struct fisrt_hw
//		{
//			unsigned Rn 		: 4;
//			unsigned opcode	: 12;
//		}fisrt_hw;
//	}msr_hw1;
//
//	union msr_hw2
//	{
//		uint16_t val;
//		struct second_hw
//		{
//			unsigned SYSm   : 8;
//			unsigned opcode	: 8;
//		}second_hw;
//	}msr_hw2;
//
//	msr_hw1.val = opcode;
//	msr_hw2.val = opcode2;
//
//	CORE_TRACE( stm, LOG_TRACE, "0x%08X MSR[%04x][%04x] \n", stm->pc &0xFFFFFFFE, opcode, opcode2 );
//
//	switch (msr_hw2.second_hw.SYSm & 0xf8) {
//		case 0:
//			{
//				if(!( msr_hw2.second_hw.SYSm & 0x04 ))
//				{
//					stm->APSR.val = stm->R[ msr_hw1.fisrt_hw.Rn ] & 0xf8000000;
//					  TRACE_ASPR();
//            // CORE_TRACE( stm, LOG_TRACE, "\tASPR[N.Z.C.V]=[%01d.%01d.%01d.%01d]\n", stm->APSR.bits.N, stm->APSR.bits.Z, stm->APSR.bits.C, stm->APSR.bits.V );
//				}
//			}
//			break;
//
//		case 0x08:
//			{
//					if(( msr_hw2.second_hw.SYSm & 0x07) == 0x00 )
//					{
//						// TODO: SP_main
//						//stm->s
//					}else if(( msr_hw2.second_hw.SYSm & 0x07) == 0x01 )
//					{
//						//TODO: SP_proccess
//					}
//			}
//			break;
//
//		case 0x10:
//			{
//
//			}
//			break;
//
//		default:
//			break;
//	}
//
//	return stm->pc+4;
//}
/*
 *
 */
//stm_flashaddr_t fb01_block(struct stm_t * stm, uint16_t opcode )
//{
//	stm_flashaddr_t next_pc;
//	uint16_t opcode2 = _stm_flash_read16le( stm, stm->pc+2 );
//
//	switch(  opcode2 & 0xc000  )
//	{
//		case 0x8000:
//			{
//				switch (opcode & 0x70 )
//				{
//					case 0:
//						// MSR
//						next_pc = msr_t1(stm, opcode, opcode2 );
//						break;
//
//					case 0x60:
//						// MRS
//						CORE_TRACE( stm, LOG_ERROR, "Unknown ins[%04x][%04x] at flash[%x]\n", opcode, opcode2, stm->pc );
//						next_pc = stm->pc;
//						break;
//
//					default:
//						CORE_TRACE( stm, LOG_ERROR, "Unknown ins[%04x][%04x] at flash[%x]\n", opcode, opcode2, stm->pc );
//						next_pc = stm->pc;
//						break;
//				}
//			}
//			break;
//
//		case 0xc000:
//			next_pc = bl_t1(stm, opcode, opcode2 );
//			break;
//
//		default:
//			CORE_TRACE( stm, LOG_ERROR, "Unknown ins[%04x][%04x] at flash[%x]\n", opcode, opcode2, stm->pc );
//			next_pc = stm->pc;
//			break;
//	}
//
//	return next_pc;
//}
/*
 *
 */
//stm_flashaddr_t b_t2(struct stm_t * stm, uint16_t opcode )
//{
//	union _ARMv6mThum2Instruction inst;
//	inst.opcode = opcode;
//
//	union imm32_offset
//	{
//		uint32_t 					val;
//		struct imm32
//		{
//			unsigned zero 	: 1;
//			unsigned imm11	: 11;
//			unsigned signExt: 20;
//		}imm32;
//	}imm32_offset;
//
//	imm32_offset.val = 0;
//
//	imm32_offset.imm32.imm11 	= inst.B_t2.imm11;
//
//	if( imm32_offset.val & 0x800 )
//		imm32_offset.imm32.signExt = 0xfffff;
//
//	CORE_TRACE( stm, LOG_TRACE, "0x%08X B[%04x] OFS[0x%08X] \n", stm->pc &0xFFFFFFFE, inst.opcode, imm32_offset );
//
//	uint32_t next_instr_addr = stm->pc +4 +imm32_offset.val;
//
//  CORE_TRACE( stm, LOG_TRACE, "\tPC[0x%08X]\n", next_instr_addr );
//
//	// new pc value
//	return next_instr_addr;
//}
/*
 *
 */
//stm_flashaddr_t b_t1(struct stm_t * stm, uint16_t opcode )
//{
//	union _ARMv6mThum2Instruction inst;
//	inst.opcode = opcode;
//
//	CORE_TRACE( stm, LOG_TRACE, "0x%08X B<C>[%04x] CON[%04x] IMM8[0x%04x]\n", stm->pc &0xFFFFFFFE, inst.opcode, inst.B_t1.cond, inst.B_t1.imm8 );
//
//	// stm->APSR.bits
//	switch (inst.B_t1.cond) {
//		case 0: // Z == 1
//			if( !stm->APSR.bits.Z )
//				return stm->pc+2;
//			break;
//
//		case 1:
//			if( stm->APSR.bits.Z )
//				return stm->pc+2;
//			break;
//
//		case 2:
//			if( !stm->APSR.bits.C )
//				return stm->pc+2;
//			break;
//
//		case 3:
//			if( stm->APSR.bits.C )
//				return stm->pc+2;
//			break;
//
//		case 4:
//			if( !stm->APSR.bits.N )
//				return stm->pc+2;
//			break;
//
//		case 5:
//			if( stm->APSR.bits.N )
//				return stm->pc+2;
//			break;
//
//		case 6:
//			if( !stm->APSR.bits.V )
//				return stm->pc+2;
//			break;
//
//		case 7:
//			if( stm->APSR.bits.V )
//				return stm->pc+2;
//			break;
//
//		case 8:
//			if( !stm->APSR.bits.C || stm->APSR.bits.Z )
//				return stm->pc+2;
//			break;
//
//		case 9:
//			if( stm->APSR.bits.C && !stm->APSR.bits.Z )
//				return stm->pc+2;
//			break;
//
//		case 10:
//			if( stm->APSR.bits.N != stm->APSR.bits.V )
//				return stm->pc+2;
//			break;
//
//		case 11:
//			if( stm->APSR.bits.N == stm->APSR.bits.V )
//				return stm->pc+2;
//			break;
//
//		case 12:
//			if( stm->APSR.bits.Z || (stm->APSR.bits.N != stm->APSR.bits.V ))
//				return stm->pc+2;
//			break;
//
//		case 13:
//			if( !stm->APSR.bits.Z && (stm->APSR.bits.N == stm->APSR.bits.V ))
//				return stm->pc+2;
//			break;
//
//		case 14:
//			// UDF?
//		case 15:
//			// SVC
//		default:
//			break;
//	}
//
//	uint32_t res_pc = inst.B_t1.imm8<<1;
//
//	if( res_pc & 0x100 )
//		res_pc |= 0xfffffe00;
//
//	res_pc += stm->pc;
//	res_pc += 4;
//
//  CORE_TRACE( stm, LOG_TRACE, "\tPC[0x%08X]\n", res_pc );
//
//	return res_pc;
//}

//stm_flashaddr_t ldr_literal(struct stm_t * stm, uint16_t opcode )
//{
//	// 0b 0100 1xxx xxxx xxxx -LDR <Rt>,<label>
//	union _ARMv6mThum2Instruction inst;
//	inst.opcode = opcode;
//
//	CORE_TRACE( stm, LOG_TRACE, "0x%08X LDR[%04x] Rt[%d] imm8[0x%x]\n", stm->pc &0xFFFFFFFE, inst.opcode, inst.LDR_literal.Rt, inst.LDR_literal.imm8 );
//
//	stm_flashaddr_t given_pc =( stm->pc & 0xfffffffc ) +0x04 +( inst.LDR_literal.imm8 << 2 );
//
//	stm->R[ inst.LDR_literal.Rt ]= *((uint32_t*)( stm->flash +( given_pc - stm->flash_start_adr)));
//
//	CORE_TRACE( stm, LOG_TRACE, "\tR[%d]=0x%08X\n", inst.LDR_literal.Rt, stm->R[ inst.LDR_literal.Rt ]);
//
//	return stm->pc + 2;
//}

//stm_flashaddr_t ldr_immediate(struct stm_t * stm, uint16_t opcode )
//{
//	union _ARMv6mThum2Instruction inst;
//	inst.opcode = opcode;
//	uint32_t Rt = inst.LDR_immediate.Rt;
//	uint32_t Rn = inst.LDR_immediate.Rn;
//	uint32_t imm5 = inst.LDR_immediate.imm5;
//
////	CORE_TRACE( stm, LOG_TRACE, "0x%08X LDR[%04x] Rt[%d] Rn[%d] imm5[%d]\n", (stm->pc &0xFFFFFFFE), opcode, Rt, Rn, imm5 );
//  CORE_TRACE( stm, LOG_TRACE, "0x%08X LDR[%04x]\n", (stm->pc &0xFFFFFFFE), opcode );
//
//
//	uint32_t addr = stm->R[ Rn ] +(imm5<<2 );
//	uint32_t val;
//
//	stm_addr_read( stm, addr, &val );
//
//	stm->R[ Rt ] = val;
//
//	CORE_TRACE( stm, LOG_TRACE, "\tR[%d]=0x%08X\n", inst.LDR_immediate.Rt, val);
//
//	return stm->pc+2;
//}
/*
 * Opcode[9802] LDR r0,[sp,#0x08]
 */
//stm_flashaddr_t ldr_sp_imm_t2(struct stm_t * stm, uint16_t opcode )
//{
//	union _ARMv6mThum2Instruction inst;
//	inst.opcode = opcode;
//
//	CORE_TRACE( stm, LOG_TRACE, "0x%08X LDRSP[%04x] Rt[%d] imm32[0x%08X]\n", stm->pc &0xFFFFFFFE, inst.opcode, inst.LDR_sp_imm8_t2.Rt, inst.LDR_sp_imm8_t2.imm8<<2 );
//
//	uint32_t addr = stm->R[ 13 ] +(inst.LDR_sp_imm8_t2.imm8<<2 );
//	uint32_t val;
//
//	stm_addr_read( stm, addr, &val );
//
//	stm->R[ inst.LDR_sp_imm8_t2.Rt ] = val;
//
//	CORE_TRACE( stm, LOG_TRACE, "\tR[%d]=0x%08X\n", inst.LDR_sp_imm8_t2.Rt, val);
//
//	return stm->pc+2;
//}
/*
 * Opcode(7820) LDRB r0,[r4,#0x00]
 */
//stm_flashaddr_t ldrb_immediate(struct stm_t * stm, uint16_t opcode )
//{
//	union _ARMv6mThum2Instruction inst;
//	inst.opcode = opcode;
//
//	CORE_TRACE( stm, LOG_TRACE, "0x%08X LDRB[%04x] Rt[%d] Rn[%d] imm5[%d]\n", stm->pc &0xFFFFFFFE, inst.opcode, inst.LDRB_imm5_t1.Rt, inst.LDRB_imm5_t1.Rn, inst.LDRB_imm5_t1.imm5 );
//
//	uint32_t addr = stm->R[ inst.LDRB_imm5_t1.Rn ] +(inst.LDRB_imm5_t1.imm5<<2 );
//	uint8_t val;
//
//	stm_addr_readb( stm, addr, &val );
//
//	stm->R[ inst.LDRB_imm5_t1.Rt ] = val;
//
//	CORE_TRACE( stm, LOG_TRACE, "\tR[%d]=0x%x\n", inst.LDRB_imm5_t1.Rt, val);
//
//	return stm->pc+2;
//}
/*
 * opcode[58D2] LDR r2,[r2,r3]
 */
//stm_flashaddr_t ldr_reg_t1(struct stm_t * stm, uint16_t opcode )
//{
//	union _ARMv6mThum2Instruction inst;
//	inst.opcode = opcode;
//
//	CORE_TRACE( stm, LOG_TRACE, "0x%08X LDR[%04x] Rt[%d] Rn[%d] Rm[%d]\n", stm->pc &0xFFFFFFFE, inst.opcode, inst.LDR_regs_t1.Rt, inst.LDR_regs_t1.Rn, inst.LDR_regs_t1.Rm );
//
//	uint32_t addr = stm->R[ inst.LDR_regs_t1.Rn ] +stm->R[inst.LDR_regs_t1.Rm];
//	uint32_t val;
//
//	stm_addr_read( stm, addr, &val );
//
//	stm->R[ inst.LDR_regs_t1.Rt ] = val;
//
//	CORE_TRACE( stm, LOG_TRACE, "\tR[%d]=0x%08X\n", inst.LDR_regs_t1.Rt, val);
//
//	return stm->pc+2;
//}
/*
 * LDRSB <Rt>,[<Rn>,<Rm>]
 */
stm_flashaddr_t ldrsb_reg_t1(struct stm_t * stm, uint16_t opcode )
{
  union _ARMv6mThum2Instruction inst;
  inst.opcode = opcode;

  CORE_TRACE( stm, LOG_TRACE, "0x%08X LDRSB[%04x] Rt[%d] Rn[%d] Rm[%d]\n", stm->pc &0xFFFFFFFE, inst.opcode, inst.LDRSB_regs_t1.Rt, inst.LDRSB_regs_t1.Rn, inst.LDRSB_regs_t1.Rm );

  uint32_t addr = stm->R[ inst.LDRSB_regs_t1.Rn ] +stm->R[inst.LDRSB_regs_t1.Rm];
  uint8_t val;

  stm_addr_readb( stm, addr, &val );

  stm->R[ inst.LDRSB_regs_t1.Rt ]= val;

  if( val & 0x80 )
    stm->R[ inst.LDRSB_regs_t1.Rt ] |= 0xffffff00;
  else
    stm->R[ inst.LDRSB_regs_t1.Rt ] &= 0x7f;

  CORE_TRACE( stm, LOG_TRACE, "\tR[%d]=0x%08X\n", inst.LDRSB_regs_t1.Rt, stm->R[ inst.LDRSB_regs_t1.Rt ]);

  return stm->pc+2;
}

/*
 * opcode[5C51] LDRB r1,[r2,r1]
 */
//stm_flashaddr_t ldrb_reg_t1(struct stm_t * stm, uint16_t opcode )
//{
//	union _ARMv6mThum2Instruction inst;
//	inst.opcode = opcode;
//
//	CORE_TRACE( stm, LOG_TRACE, "0x%08X LDRB[%04x] Rt[%d] Rn[%d] Rm[%d]\n", stm->pc &0xFFFFFFFE, inst.opcode, inst.LDRB_regs_t1.Rt, inst.LDRB_regs_t1.Rn, inst.LDRB_regs_t1.Rm );
//
//	uint32_t addr = stm->R[ inst.LDRB_regs_t1.Rn ] +stm->R[inst.LDRB_regs_t1.Rm];
//	uint8_t val;
//
//	stm_addr_readb( stm, addr, &val );
//
//	stm->R[ inst.LDRB_regs_t1.Rt ] = val;
//
//  CORE_TRACE( stm, LOG_TRACE, "\tR[%d]=0x%08X\n", inst.LDRB_regs_t1.Rt, stm->R[ inst.LDRB_regs_t1.Rt ]);
//
//	return stm->pc+2;
//}
/*
 * LDRH <Rt>,[<Rn>,<Rm>]
 */
stm_flashaddr_t ldrh_reg_t1(struct stm_t * stm, uint16_t opcode )
{
  union _ARMv6mThum2Instruction inst;
  inst.opcode = opcode;

  CORE_TRACE( stm, LOG_TRACE, "0x%08X LDRH[%04x] Rt[%d] Rn[%d] Rm[%d]\n", stm->pc &0xFFFFFFFE, inst.opcode, inst.LDRH_regs_t1.Rt, inst.LDRH_regs_t1.Rn, inst.LDRH_regs_t1.Rm );

  uint32_t addr = stm->R[ inst.LDRH_regs_t1.Rn ] +stm->R[inst.LDRH_regs_t1.Rm];
  uint16_t val;

  stm_addr_readh( stm, addr, &val );

  stm->R[ inst.LDRH_regs_t1.Rt ]= val;

  CORE_TRACE( stm, LOG_TRACE, "\tR[%d]=0x%08X\n", inst.LDRH_regs_t1.Rt, stm->R[ inst.LDRH_regs_t1.Rt ]);

  return stm->pc+2;
}
/*
 * LDRSH <Rt>,[<Rn>,<Rm>]
 */
stm_flashaddr_t ldrsh_reg_t1(struct stm_t * stm, uint16_t opcode )
{
  union _ARMv6mThum2Instruction inst;
  inst.opcode = opcode;

  CORE_TRACE( stm, LOG_TRACE, "0x%08X LDRSH[%04x] Rt[%d] Rn[%d] Rm[%d]\n", stm->pc &0xFFFFFFFE, inst.opcode, inst.LDRSH_regs_t1.Rt, inst.LDRSH_regs_t1.Rn, inst.LDRSH_regs_t1.Rm );

  uint32_t addr = stm->R[ inst.LDRSH_regs_t1.Rn ] +stm->R[inst.LDRSH_regs_t1.Rm];
  uint16_t val;

  stm_addr_readh( stm, addr, &val );

  stm->R[ inst.LDRSH_regs_t1.Rt ]= val;

  if( val & 0x80 )
    stm->R[ inst.LDRSH_regs_t1.Rt ] |= 0xffff0000;
  else
    stm->R[ inst.LDRSH_regs_t1.Rt ] &= 0x7fff;

  CORE_TRACE( stm, LOG_TRACE, "\tR[%d]=0x%08X\n", inst.LDRSH_regs_t1.Rt, stm->R[ inst.LDRSH_regs_t1.Rt ]);

  return stm->pc+2;
}

/*
 * LDRH{<q>} <Rt>, [<Rn> {, #+/-<imm>}]
 */
stm_flashaddr_t ldrh_immediate_T1(struct stm_t * stm, uint16_t opcode )
{
  union _ARMv6mThum2Instruction inst;
  inst.opcode = opcode;

  CORE_TRACE( stm, LOG_TRACE, "0x%08X LDRH[%04x] Rt[%d] Rn[%d] imm5[0x%02X]\n", stm->pc &0xFFFFFFFE, inst.opcode, inst.LDRH_imm_t1.Rt, inst.LDRH_imm_t1.Rn, inst.LDRH_imm_t1.imm5 );

  uint32_t addr = stm->R[ inst.LDRH_imm_t1.Rn ] +(inst.LDRH_imm_t1.imm5<<1);
  uint16_t val;

  stm_addr_readh( stm, addr, &val );

  stm->R[ inst.LDRH_imm_t1.Rt ]= val;

  CORE_TRACE( stm, LOG_TRACE, "\tR[%d]=0x%08X\n", inst.LDRH_imm_t1.Rt, stm->R[ inst.LDRH_imm_t1.Rt ]);

  return stm->pc+2;
}

/*
 *
 */
#if 0
stm_flashaddr_t ldm_t1(struct stm_t * stm, uint16_t opcode )
{
	union _ARMv6mThum2Instruction inst;
	inst.opcode 		= opcode;
	uint32_t Rn 		= inst.LDM_t1.Rn;
	uint32_t rlist 	= inst.LDM_t1.rlist;

	CORE_TRACE( stm, LOG_TRACE, "0x%08X LDM[%04x] Rn[%d] rl[%x]\n", stm->pc &0xFFFFFFFE, inst.opcode, Rn, rlist );

	uint32_t addr = stm->R[Rn];
	uint32_t val;

	if( rlist & 0x01 )
	{
		stm_addr_read( stm, addr, &val );
		stm->R[0] = val;
		addr +=4;

		CORE_TRACE( stm, LOG_TRACE, "\tR[0]=0x%08X\n", val);
	}

	if( rlist & 0x02 )
	{
		stm_addr_read( stm, addr, &val );
		stm->R[1] = val;
		addr +=4;

		CORE_TRACE( stm, LOG_TRACE, "\tR[1]=0x%08X\n", val);
	}

	if( rlist & 0x04 )
	{
		stm_addr_read( stm, addr, &val );
		stm->R[2] = val;
		addr +=4;

		CORE_TRACE( stm, LOG_TRACE, "\tR[2]=0x%08X\n", val);
	}

	if( rlist & 0x08 )
	{
		stm_addr_read( stm, addr, &val );
		stm->R[3] = val;
		addr +=4;

		CORE_TRACE( stm, LOG_TRACE, "\tR[3]=0x%08X\n", val);
	}

	if( rlist & 0x10 )
	{
		stm_addr_read( stm, addr, &val );
		stm->R[4] = val;
		addr +=4;

		CORE_TRACE( stm, LOG_TRACE, "\tR[4]=0x%08X\n", val);
	}

	if( rlist & 0x20 )
	{
		stm_addr_read( stm, addr, &val );
		stm->R[5] = val;
		addr +=4;

		CORE_TRACE( stm, LOG_TRACE, "\tR[5]=0x%08X\n", val);
	}

	if( rlist & 0x40 )
	{
		stm_addr_read( stm, addr, &val );
		stm->R[6] = val;
		addr +=4;

		CORE_TRACE( stm, LOG_TRACE, "\tR[6]=0x%08X\n", val);
	}

	if( rlist & 0x80 )
	{
		stm_addr_read( stm, addr, &val );
		stm->R[7] = val;

		if(!( rlist &( 1<<Rn )))
			stm->R[Rn] = addr;

		CORE_TRACE( stm, LOG_TRACE, "\tR[7]=0x%08X\n", val);
//	}else{
//		addr -=4;
//
//		if(!( rlist &( 1<<Rn )))
//			stm->R[Rn] = addr;
//	}
	}

	if(!( rlist &( 1<<Rn )))
		stm->R[Rn] = addr;

	CORE_TRACE( stm, LOG_TRACE, "\tR%d(@0x%08X)\n", Rn,  addr);

	return stm->pc+2;
}
#endif


#if 0
/*
 * opcode[BD30] POP {r4-r5,pc}
 */
stm_flashaddr_t pop_t1(struct stm_t * stm, uint16_t opcode )
{
	union _ARMv6mThum2Instruction inst;
	inst.opcode 		= opcode;
	uint32_t P 			= inst.POP_t1.P;
	uint32_t rlist 	= inst.POP_t1.rlist;

	CORE_TRACE( stm, LOG_TRACE, "0x%08X lvl[%d] POP[%04x] P[%d] rl[%x]\n", stm->pc &0xFFFFFFFE, --uiSubroutineLvl, inst.opcode, P, rlist );

	uint32_t addr = stm->R[13];
	uint32_t val;

	if( rlist & 0x01 )
	{
		addr +=4;
		stm_addr_read( stm, addr, &val );
		stm->R[0] = val;

		CORE_TRACE( stm, LOG_TRACE, "\tR[0]=0x%08X\n", val);
	}

	if( rlist & 0x02 )
	{
		addr +=4;
		stm_addr_read( stm, addr, &val );
		stm->R[1] = val;

		CORE_TRACE( stm, LOG_TRACE, "\tR[1]=0x%08X\n", val);
	}

	if( rlist & 0x04 )
	{
		addr +=4;
		stm_addr_read( stm, addr, &val );
		stm->R[2] = val;

		CORE_TRACE( stm, LOG_TRACE, "\tR[2]=0x%08X\n", val);
	}

	if( rlist & 0x08 )
	{
		addr +=4;
		stm_addr_read( stm, addr, &val );
		stm->R[3] = val;

		CORE_TRACE( stm, LOG_TRACE, "\tR[3]=0x%08X\n", val);
	}

	if( rlist & 0x10 )
	{
		addr +=4;
		stm_addr_read( stm, addr, &val );
		stm->R[4] = val;

		CORE_TRACE( stm, LOG_TRACE, "\tR[4]=0x%08X\n", val);
	}

	if( rlist & 0x20 )
	{
		addr +=4;
		stm_addr_read( stm, addr, &val );
		stm->R[5] = val;

		CORE_TRACE( stm, LOG_TRACE, "\tR[5]=0x%08X\n", val);
	}

	if( rlist & 0x40 )
	{
		addr +=4;
		stm_addr_read( stm, addr, &val );
		stm->R[6] = val;

		CORE_TRACE( stm, LOG_TRACE, "\tR[6]=0x%08X\n", val);
	}
	if( rlist & 0x80 )
	{
		addr +=4;
		stm_addr_read( stm, addr, &val );
		stm->R[7] = val;

		CORE_TRACE( stm, LOG_TRACE, "\tR[7]=0x%08X\n", val);
	}
	if( P )
	{
		addr +=4;
		stm_addr_read( stm, addr, &val );

		if(( val & 0xfffffff0 )== 0xfffffff0 )
		{
		  // todo: nvic
		  // initiate interrupt leaving process
		    nvic_run_interrupt_leaving( stm->mcu, val );
		}else{
		  stm->pc = val;
		}

		CORE_TRACE( stm, LOG_TRACE, "\tPC=0x%08X\n", val);
	}
	else
		val = stm->pc+2;

	stm->R[13] = addr;

	CORE_TRACE( stm, LOG_TRACE, "\tSP(@0x%08X)\n", addr);

	return val;
}
#endif

#if 0
/*
 *
 */
stm_flashaddr_t stm_t1(struct stm_t * stm, uint16_t opcode )
{
	union _ARMv6mThum2Instruction inst;
	inst.opcode 		= opcode;
	uint32_t Rn 		= inst.STM_t1.Rn;
	uint32_t rlist 	= inst.STM_t1.rlist;

	CORE_TRACE( stm, LOG_TRACE, "0x%08X STM[%04x] Rn[%d] rl[%x]\n", stm->pc &0xFFFFFFFE, opcode, Rn, rlist );

	uint32_t addr = stm->R[Rn];
	uint32_t val;

	if( rlist & 0x01 )
	{
		val  = stm->R[0];
		stm_addr_write( stm, addr, val );

		CORE_TRACE( stm, LOG_TRACE, "\t@0x%08X=R0(0x%08X)\n", addr, val);
		addr +=4;
	}

	if( rlist & 0x02 )
	{
		val  = stm->R[1];
		stm_addr_write( stm, addr, val );

		CORE_TRACE( stm, LOG_TRACE, "\t@0x%08X=R1(0x%08X)\n", addr, val);
		addr +=4;
	}

	if( rlist & 0x04 )
	{
		val  = stm->R[2];
		stm_addr_write( stm, addr, val );

		CORE_TRACE( stm, LOG_TRACE, "\t@0x%08X=R2(0x%08X)\n", addr, val);

		addr +=4;
	}

	if( rlist & 0x08 )
	{
		val  = stm->R[3];
		stm_addr_write( stm, addr, val );

		CORE_TRACE( stm, LOG_TRACE, "\t@0x%08X=R3(0x%08X)\n", addr, val);

		addr +=4;
	}

	if( rlist & 0x10 )
	{
		val  = stm->R[4];
		stm_addr_write( stm, addr, val );

		CORE_TRACE( stm, LOG_TRACE, "\t@0x%08X=R4(0x%08X)\n", addr, val);

		addr +=4;
	}

	if( rlist & 0x20 )
	{
		val  = stm->R[5];
		stm_addr_write( stm, addr, val );

		CORE_TRACE( stm, LOG_TRACE, "\t@0x%08X=R5(0x%08X)\n", addr, val);

		addr +=4;
	}

	if( rlist & 0x40 )
	{
		val  = stm->R[6];
		stm_addr_write( stm, addr, val );

		CORE_TRACE( stm, LOG_TRACE, "\t@0x%08X=R6(0x%08X)\n", addr, val);

		addr +=4;
	}

	if( rlist & 0x80 )
	{
		val  = stm->R[7];
		stm_addr_write( stm, addr, val );

		if(!( rlist &( 1<<Rn )))
			stm->R[Rn] = addr;

		CORE_TRACE( stm, LOG_TRACE, "\t@0x%08X=R7(0x%08X)\n", addr, val);

		addr +=4;
	}

	if(!( rlist &( 1<<Rn )))
			stm->R[Rn] = addr;

	CORE_TRACE( stm, LOG_TRACE, "\tR%d(@0x%08X)\n", Rn,  addr);

	return stm->pc+2;
}
#endif

#if 0
/*
 * PUSH
 */
stm_flashaddr_t push_t1(struct stm_t * stm, uint16_t opcode )
{
	union _ARMv6mThum2Instruction inst;
	inst.opcode 		= opcode;
	uint32_t M 		= inst.PUSH_t1.M;
	uint32_t rlist 	= inst.PUSH_t1.rlist;

	CORE_TRACE( stm, LOG_TRACE, "0x%08X lvl[%d] PUSH[%04x] M[%d] rl[%x]\n", stm->pc &0xFFFFFFFE, uiSubroutineLvl++, opcode, M, rlist );

	uint32_t addr = stm->R[13];

	if( M )
	{
		stm_addr_write( stm, addr, stm->R[ 14 ]);
		CORE_TRACE( stm, LOG_TRACE, "\t@0x%08X=LR(0x%08X)\n", addr, stm->R[ 14 ]);
		addr -=4;
	}
	if( rlist & 0x80 )
	{
		stm_addr_write( stm, addr, stm->R[7] );

		CORE_TRACE( stm, LOG_TRACE, "\t@0x%08X=R7(0x%08X)\n", addr, stm->R[ 7 ]);

		addr -=4;
	}
	if( rlist & 0x40 )
	{
		stm_addr_write( stm, addr, stm->R[ 6 ] );

		CORE_TRACE( stm, LOG_TRACE, "\t@0x%08X=R6(0x%08X)\n", addr, stm->R[ 6 ]);

		addr -=4;
	}
	if( rlist & 0x20 )
	{
		stm_addr_write( stm, addr, stm->R[5] );

		CORE_TRACE( stm, LOG_TRACE, "\t@0x%08X=R5(0x%08X)\n", addr, stm->R[ 5 ]);

		addr -=4;
	}
	if( rlist & 0x10 )
	{
		stm_addr_write( stm, addr, stm->R[ 4 ]);

		CORE_TRACE( stm, LOG_TRACE, "\t@0x%08X=R4(0x%08X)\n", addr, stm->R[ 4 ]);

		addr -=4;
	}
	if( rlist & 0x08 )
	{
		stm_addr_write( stm, addr, stm->R[3] );

		CORE_TRACE( stm, LOG_TRACE, "\t@0x%08X=R3(0x%08X)\n", addr, stm->R[ 3 ]);

		addr -=4;
	}
	if( rlist & 0x04 )
	{
		stm_addr_write( stm, addr, stm->R[2] );

		CORE_TRACE( stm, LOG_TRACE, "\t@0x%08X=R2(0x%08X)\n", addr, stm->R[ 2 ]);

		addr -=4;
	}
	if( rlist & 0x02 )
	{
		stm_addr_write( stm, addr, stm->R[ 1 ]);

		CORE_TRACE( stm, LOG_TRACE, "\t@0x%08X=R1(0x%08X)\n", addr, stm->R[ 1 ]);
		addr -=4;
	}
	if( rlist & 0x01 )
	{
		stm_addr_write( stm, addr, stm->R[ 0 ]);

		CORE_TRACE( stm, LOG_TRACE, "\t@0x%08X=R0(0x%08X)\n", stm->R[ 0 ]);
		addr -=4;
	}

	stm->R[ 13 ]= addr;

	CORE_TRACE( stm, LOG_TRACE, "\tSP(@0x%08X)\n", addr);

	return stm->pc+2;
}
#endif

#if 1
stm_flashaddr_t mov_immediate(struct stm_t * stm, uint16_t opcode )
{
	union _ARMv6mThum2Instruction inst;
	inst.opcode 	= opcode;
	uint32_t val 	= inst.MOVS.imm8;
	uint32_t Rd		= inst.MOVS.Rd;

	CORE_TRACE( stm, LOG_TRACE, "0x%08X MOVS[%04x] Rd[%d] imm8[0x%x]\n", stm->pc &0xFFFFFFFE, opcode, Rd, val );

	stm->R[ Rd ] = val;

  UPDATE_Z_N( val, stm->APSR.bits.Z, stm->APSR.bits.N );

	CORE_TRACE( stm, LOG_TRACE, "\tR[%d]=0x%08X\n", Rd, val);
  TRACE_ASPR();

	return stm->pc+2;
}
#endif

/*
 * Opcode(1C40) ADDS r0,r0,#1
 */
stm_flashaddr_t add_immediate_T1(struct stm_t * stm, uint16_t opcode )
{
  union _ARMv6mThum2Instruction inst;
  inst.opcode     = opcode;
  uint32_t imm3   = inst.ADDS_t1.imm3;
  uint32_t Rd     = inst.ADDS_t1.Rd;
  uint32_t Rn     = inst.ADDS_t1.Rn;
//  uint32_t Rd_val = stm->R[ Rd ];
  uint32_t Rn_val = stm->R[ Rn ];
  uint32_t res;

  CORE_TRACE( stm, LOG_TRACE, "0x%08X ADDS[%04x] Rd[%d] Rn[%d] imm3[0x%x]\n", stm->pc &0xFFFFFFFE, opcode, Rd, Rn, imm3 );

  ADD_WITH_C_V( res, stm->APSR.bits.C, stm->APSR.bits.V, imm3, Rn_val );

  // APSR update
  UPDATE_Z_N( res, stm->APSR.bits.Z, stm->APSR.bits.N );

  stm->R[ Rd ]= res;

  CORE_TRACE( stm, LOG_TRACE, "\tR[%d]=0x%08X\n", Rd, res );
  TRACE_ASPR();
  // CORE_TRACE( stm, LOG_TRACE, "\tASPR[N.Z.C.V]=[%01d.%01d.%01d.%01d]\n", stm->APSR.bits.N, stm->APSR.bits.Z, stm->APSR.bits.C, stm->APSR.bits.V );

  return stm->pc+2;
}

#if 1
/*
 *
 */
stm_flashaddr_t add_immediate_T2(struct stm_t * stm, uint16_t opcode )
{
  union _ARMv6mThum2Instruction inst;
  inst.opcode   = opcode;
  uint32_t Rdn  = inst.ADDS_t2.Rdn;

  uint32_t res;

  CORE_TRACE( stm, LOG_TRACE, "0x%08X ADDS[%04x] Rdn[%d] imm8[0x%x]\n", stm->pc &0xFFFFFFFE, opcode, Rdn, inst.ADDS_t2.imm8 );

  // ADD_WITH_C_V( R, CF, OF, X, Y )
  ADD_WITH_C_V( res, stm->APSR.bits.C, stm->APSR.bits.V, stm->R[ Rdn ], inst.ADDS_t2.imm8 );

  // APSR update
  UPDATE_Z_N( res, stm->APSR.bits.Z, stm->APSR.bits.N );

  stm->R[ Rdn ]= res;

  CORE_TRACE( stm, LOG_TRACE, "\tR[%d]=0x%08X\n", Rdn, res );
  TRACE_ASPR();

  return stm->pc+2;
}
#endif

#if 1
/*
 * Opcode[1940] ADDS r0,r0,r5
 */
stm_flashaddr_t add_regs_T1(struct stm_t * stm, uint16_t opcode )
{
  union _ARMv6mThum2Instruction inst;
  inst.opcode   = opcode;
  uint32_t Rd   = inst.ADDS_regs_t1.Rd;
  uint32_t Rn   = inst.ADDS_regs_t1.Rn;
  uint32_t Rm   = inst.ADDS_regs_t1.Rm;
  uint32_t res;

  CORE_TRACE( stm, LOG_TRACE, "0x%08X ADDS[%04x] Rd[%d] Rn[%d] Rm[%d]\n", stm->pc &0xFFFFFFFE, opcode, Rd, Rn, Rm );

  ADD_WITH_C_V( res, stm->APSR.bits.C, stm->APSR.bits.V, stm->R[ Rn ], stm->R[ Rm ]);

  // APSR update
  UPDATE_Z_N( res, stm->APSR.bits.Z, stm->APSR.bits.N );

  stm->R[ Rd ]= res;

  CORE_TRACE( stm, LOG_TRACE, "\tR[%d]=0x%08X\n", Rd, res );
  TRACE_ASPR();

  return stm->pc+2;
}
#endif

/*
 * ADD <Rdn>,<Rm>
 */
stm_flashaddr_t add_register_T2(struct stm_t * stm, uint16_t opcode )
{
  union _ARMv6mThum2Instruction inst;
  inst.opcode   = opcode;
  uint32_t Rdn  = inst.ADD_regs_t2.Rdn;
  uint32_t Rm   = inst.ADD_regs_t2.Rm;

  CORE_TRACE( stm, LOG_TRACE, "0x%08X ADD[%04x] Rdn[%d] DN[%d] Rm[%d]\n", stm->pc &0xFFFFFFFE, opcode, inst.ADD_regs_t2.Rdn, inst.ADD_regs_t2.DN, Rm );

  Rdn += inst.ADD_regs_t2.DN<<3;

  if(( Rm < 0x0f )&&( Rdn < 0x0f ))
  {
    stm->R[ Rdn ] += stm->R[ Rm ];

    CORE_TRACE( stm, LOG_TRACE, "\tR[%d]=0x%08X\n", Rdn, stm->R[ Rdn ] );

  }else {
      if( Rm == 0x0f )
      {
          if( Rdn == 0x0f )
          {
              stm->state = cpu_Crashed;
              CORE_TRACE( stm, LOG_TRACE, "0x%08X ADD[%04x] Crashed Rdn,Rm ==0x1111\n", stm->pc &0xFFFFFFFE, opcode );

              return stm->pc;

          }else{
              stm->R[ Rdn ] += stm->pc;
              stm->R[ Rdn ] += 2;
          }
      }else{
          stm->pc = stm->R[ Rm ];
          stm->R[ 15 ]= stm->pc;
          CORE_TRACE( stm, LOG_TRACE, "\tPC=0x%08X\n", stm->pc );
          return stm->pc;
      }

  }
  return stm->pc+2;
}

/*
 * ADCS <Rdn>,<Rm>
 */
// TODO: NOT TESTED
stm_flashaddr_t adc_regs_T1(struct stm_t * stm, uint16_t opcode )
{
  union _ARMv6mThum2Instruction inst;
  inst.opcode   = opcode;
  uint32_t Rdn  = inst.ADCS_regs_t1.Rdn;
  uint32_t Rm   = inst.ADCS_regs_t1.Rm;
  uint32_t res;

  CORE_TRACE( stm, LOG_TRACE, "0x%08X ADCS[%04x] Rdn[%d] Rm[%d]\n", stm->pc &0xFFFFFFFE, opcode, Rdn, Rm );

  ADD_WITH_CARRY( res, stm->APSR.bits.C, stm->APSR.bits.V, stm->R[ Rdn ], stm->R[ Rm ], stm->APSR.bits.C );

  // APSR update
  UPDATE_Z_N( res, stm->APSR.bits.Z, stm->APSR.bits.N );

  stm->R[ Rdn ]= res;

  CORE_TRACE( stm, LOG_TRACE, "\tR[%d]=0x%08X\n", Rdn, res );
  TRACE_ASPR();

  return stm->pc+2;
}

#if 0

/*
 * Opcode(A80E) ADD r0,sp,#0x38
 */
stm_flashaddr_t add_sp_immediate_T1(struct stm_t * stm, uint16_t opcode )
{
	union _ARMv6mThum2Instruction inst;
	inst.opcode 	= opcode;
	uint32_t Rd	= inst.ADDS_sp_imm_t1.Rd;
	uint32_t imm32 = inst.ADDS_sp_imm_t1.imm8<<2;

	CORE_TRACE( stm, LOG_TRACE, "0x%08X ADD SP[%04x] Rd[%d] imm32[0x%0x]\n", stm->pc &0xFFFFFFFE, opcode, Rd, imm32 );

	stm->R[ Rd ]= stm->R[ 13 ] +imm32;

	CORE_TRACE( stm, LOG_TRACE, "\t@SP(0x%08X)\n", stm->R[ 13 ]);
	CORE_TRACE( stm, LOG_TRACE, "\tR[%d]=0x%08X\n", Rd, stm->R[ Rd ]);

  return stm->pc+2;
}
#endif

#if 0
/*
 * Opcode(XXXX) ADD SP,SP,#<imm7>
 */
stm_flashaddr_t add_sp_immediate_T2(struct stm_t * stm, uint16_t opcode )
{
	union _ARMv6mThum2Instruction inst;
	inst.opcode 	= opcode;
	uint32_t imm32 = inst.ADDS_sp_imm_t2.imm7<<2;

	CORE_TRACE( stm, LOG_TRACE, "0x%08X ADD[%04x] SP imm32[0x%0x]\n", stm->pc &0xFFFFFFFE, opcode, imm32 );

	stm->R[ 13 ]= stm->R[ 13 ] +imm32;

	CORE_TRACE( stm, LOG_TRACE, "\t@SP(0x%08X)\n", stm->R[ 13 ]);

  return stm->pc+2;
}
#endif

/*
 * ADR <Rd>,<label>
 */
stm_flashaddr_t adr_T1(struct stm_t * stm, uint16_t opcode )
{
  union _ARMv6mThum2Instruction inst;
  inst.opcode     = opcode;
  uint32_t imm32  = inst.ADR_imm_t1.imm8<<2;
  uint32_t Rn     = inst.ADR_imm_t1.Rd;

  CORE_TRACE( stm, LOG_TRACE, "0x%08X ADR[%04x] imm32[0x%0x]\n", stm->pc &0xFFFFFFFE, opcode, imm32 );

  stm->R[ Rn ]=( stm->pc & 0xFFFFFFFC )+ imm32;

  CORE_TRACE( stm, LOG_TRACE, "\tR%d(0x%08X)\n", Rn, stm->R[ Rn ]);

  return stm->pc+2;
}

/*
 *
 */
stm_flashaddr_t sub_immediate_T1(struct stm_t * stm, uint16_t opcode )
{
	union _ARMv6mThum2Instruction inst;
	inst.opcode 		= opcode;
	uint32_t imm32 	= inst.SUBS_t1.imm3;
	uint32_t Rd 		= inst.SUBS_t1.Rd;
	uint32_t Rn			= inst.SUBS_t1.Rn;
  uint32_t Rn_val = stm->R[Rn];
	uint32_t res;

	CORE_TRACE( stm, LOG_TRACE, "0x%08X SUBS[%04x] Rd[%d] Rn[%d] imm3[%d]\n", stm->pc &0xFFFFFFFE, opcode, Rd, Rn, imm32 );

  ADD_WITH_CARRY( res, stm->APSR.bits.C, stm->APSR.bits.V, Rn_val, ~imm32,  1 );

  // APSR update
  UPDATE_Z_N( res, stm->APSR.bits.Z, stm->APSR.bits.N );

	stm->R[Rd] = res;

	CORE_TRACE( stm, LOG_TRACE, "\tR[%d]=0x%08X\n", Rn, res );
  TRACE_ASPR();
  // CORE_TRACE( stm, LOG_TRACE, "\tASPR[N.Z.C.V]=[%01d.%01d.%01d.%01d]\n", stm->APSR.bits.N, stm->APSR.bits.Z, stm->APSR.bits.C, stm->APSR.bits.V );

	return stm->pc+2;
}

/*
 *  Opcode[1B49] RSBS <Rd>,<Rn>
 */
stm_flashaddr_t rsbs_immediate_T1(struct stm_t * stm, uint16_t opcode )
{
  union _ARMv6mThum2Instruction inst;
  inst.opcode         = opcode;
  uint32_t Rn_val     = stm->R[ inst.RSBS_t1.Rn ];
  uint32_t res;
  uint32_t imm32      = 0;

  CORE_TRACE( stm, LOG_TRACE, "0x%08X RSBS[%04x] Rd[%d] Rn[%d]\n", stm->pc &0xFFFFFFFE, opcode, inst.RSBS_t1.Rd, inst.RSBS_t1.Rn );

  ADD_WITH_CARRY( res, stm->APSR.bits.C, stm->APSR.bits.V, ~Rn_val, imm32, 1 );

  UPDATE_Z_N( res, stm->APSR.bits.Z, stm->APSR.bits.N );

  stm->R[inst.RSBS_t1.Rd ]= res;

  CORE_TRACE( stm, LOG_TRACE, "\tR[%d]=0x%08X\n", inst.RSBS_t1.Rd, stm->R[ inst.RSBS_t1.Rd ]);
  TRACE_ASPR();
  // CORE_TRACE( stm, LOG_TRACE, "\tASPR[N.Z.C.V]=[%01d.%01d.%01d.%01d]\n", stm->APSR.bits.N, stm->APSR.bits.Z, stm->APSR.bits.C, stm->APSR.bits.V );

  return stm->pc+2;
}

#if 1
/*
 *
 */
stm_flashaddr_t sub_immediate_T2(struct stm_t * stm, uint16_t opcode )
{
	union _ARMv6mThum2Instruction inst;
	inst.opcode 		= opcode;
	uint32_t imm32 	= inst.SUBS_t2.imm8;
	uint32_t Rdn		= inst.SUBS_t2.Rdn;
	uint32_t Rn_val 	= stm->R[Rdn];
	uint32_t res;

	CORE_TRACE( stm, LOG_TRACE, "0x%08X SUBS[%04x] Rdn[%d] imm8[0x%x]\n", stm->pc &0xFFFFFFFE, opcode, Rdn, imm32 );

  ADD_WITH_CARRY( res, stm->APSR.bits.C, stm->APSR.bits.V, Rn_val, ~imm32,  1 );

  // APSR update
  UPDATE_Z_N( res, stm->APSR.bits.Z, stm->APSR.bits.N );

  stm->R[ Rdn ]= res;

	CORE_TRACE( stm, LOG_TRACE, "\tR[%d]=0x%08X\n", Rdn, res );
  TRACE_ASPR();

	return stm->pc+2;
}
#endif

/*
 * Opcode[1B49] SUBS     r1,r1,r5
 */
stm_flashaddr_t sub_regs_T1(struct stm_t * stm, uint16_t opcode )
{
	union _ARMv6mThum2Instruction inst;
	inst.opcode 		= opcode;
	uint32_t Rd 		= inst.SUB_regs_t1.Rd;
	uint32_t Rn			= inst.SUB_regs_t1.Rn;
	uint32_t Rm			= inst.SUB_regs_t1.Rm;
	uint32_t R_Rn 	= stm->R[ Rn ];
	uint32_t Rm_val = stm->R[ Rm ];
	uint32_t res;

	CORE_TRACE( stm, LOG_TRACE, "0x%08X SUBS[%04x] Rd[%d] Rn[%d] Rm[%d]\n", stm->pc &0xFFFFFFFE, opcode, Rd, Rn, Rm );

  ADD_WITH_CARRY( res, stm->APSR.bits.C, stm->APSR.bits.V, R_Rn, ~Rm_val,  1 );

  // APSR update
  UPDATE_Z_N( res, stm->APSR.bits.Z, stm->APSR.bits.N );

	stm->R[Rd] = res;

	CORE_TRACE( stm, LOG_TRACE, "\tR[%d]=0x%08X\n", Rd, res );
  TRACE_ASPR();

	return stm->pc+2;
}

/*
 * SBC (register) Encoding T1
 * SBCS <Rdn>,<Rm>
 */
// TODO: NOT TESTED
stm_flashaddr_t sbc_regs_T1(struct stm_t * stm, uint16_t opcode )
{
  union _ARMv6mThum2Instruction inst;
  inst.opcode     = opcode;
  uint32_t Rdn    = inst.SBC_regs_t1.Rdn;
  uint32_t Rm     = inst.SBC_regs_t1.Rm;
  uint32_t Rm_val = stm->R[ Rm ];
  uint32_t res;

  CORE_TRACE( stm, LOG_TRACE, "0x%08X SBCS[%04x] Rdn[%d] Rm[%d]-NOT TESTED\n", stm->pc &0xFFFFFFFE, opcode, Rdn, Rm );

  ADD_WITH_CARRY( res, stm->APSR.bits.C, stm->APSR.bits.V, stm->R[ Rdn ], ~Rm_val,  1 );

  // APSR update
  UPDATE_Z_N( res, stm->APSR.bits.Z, stm->APSR.bits.N );

  stm->R[Rdn] = res;

  CORE_TRACE( stm, LOG_TRACE, "\tR[%d]=0x%08X\n", Rdn, res );
  TRACE_ASPR();
  // CORE_TRACE( stm, LOG_TRACE, "\tASPR[N.Z.C.V]=[%01d.%01d.%01d.%01d]\n", stm->APSR.bits.N, stm->APSR.bits.Z, stm->APSR.bits.C, stm->APSR.bits.V );

  return stm->pc+2;
}

/*
 * MVNS <Rd>,<Rm>
 */
stm_flashaddr_t mvns_register_T1(struct stm_t * stm, uint16_t opcode )
{
  union _ARMv6mThum2Instruction inst;
  inst.opcode   = opcode;
  uint32_t val  = ~stm->R[ inst.ORRS_t1.Rm ];
  uint32_t Rdn  = inst.ORRS_t1.Rdn;

  CORE_TRACE( stm, LOG_TRACE, "0x%08X MVNS[%04x] Rdn[%d] Rm[%d]\n", stm->pc &0xFFFFFFFE, opcode, Rdn, inst.ORRS_t1.Rm );

  stm->R[ Rdn ]= val;

  // APSR update
  UPDATE_Z_N( val, stm->APSR.bits.Z, stm->APSR.bits.N );

  CORE_TRACE( stm, LOG_TRACE, "\tR[%d]=0x%08X\n", Rdn, val );
  TRACE_ASPR();
  // CORE_TRACE( stm, LOG_TRACE, "\tASPR[N.Z.C.V]=[%01d.%01d.%01d.%01d]\n", stm->APSR.bits.N, stm->APSR.bits.Z, stm->APSR.bits.C, stm->APSR.bits.V );

  return stm->pc+2;
}

/*
 *
 */
stm_flashaddr_t eors_register_T1(struct stm_t * stm, uint16_t opcode )
{
	union _ARMv6mThum2Instruction inst;
	inst.opcode 	= opcode;
	uint32_t val 	= stm->R[ inst.ORRS_t1.Rm ];
	uint32_t Rn	= inst.EORS_t1.Rn;

	CORE_TRACE( stm, LOG_TRACE, "0x%08X EORS[%04x] Rn[%d] Rm[%d]\n", stm->pc &0xFFFFFFFE, opcode, Rn, inst.EORS_t1.Rm );

	val ^=  stm->R[ Rn ];
	stm->R[ Rn ] = val;

	// APSR update
  UPDATE_Z_N( val, stm->APSR.bits.Z, stm->APSR.bits.N );

  TRACE_ASPR();

  return stm->pc+2;
}

stm_flashaddr_t ands_register_T1(struct stm_t * stm, uint16_t opcode )
{
	union _ARMv6mThum2Instruction inst;
	inst.opcode 	= opcode;
	uint32_t val 	= stm->R[ inst.ANDS_t1.Rm ];
	uint32_t Rdn	= inst.ANDS_t1.Rdn;

	CORE_TRACE( stm, LOG_TRACE, "0x%08X ANDS[%04x] Rn[%d] Rm[%d]\n", stm->pc &0xFFFFFFFE, opcode, Rdn, inst.ANDS_t1.Rm );

	val &=  stm->R[ Rdn ];
	stm->R[ Rdn ] = val;

	// APSR update
  UPDATE_Z_N( val, stm->APSR.bits.Z, stm->APSR.bits.N );

	CORE_TRACE( stm, LOG_TRACE, "\tR[%d]=0x%08X\n", Rdn, val);
  TRACE_ASPR();

  return stm->pc+2;
}

/*
 * TST (register) Encoding T1
 */
stm_flashaddr_t tst_register_T1(struct stm_t * stm, uint16_t opcode )
{
  union _ARMv6mThum2Instruction inst;
  inst.opcode   = opcode;
  uint32_t val  = stm->R[ inst.TST_register_t1.Rm ];
  uint32_t Rn   = inst.TST_register_t1.Rn;

  CORE_TRACE( stm, LOG_TRACE, "0x%08X TST[%04x] Rn[%d] Rm[%d]-NOT TESTED\n", stm->pc &0xFFFFFFFE, opcode, Rn, inst.ANDS_t1.Rm );

  val &=  stm->R[ Rn ];

  // APSR update
  UPDATE_Z_N( val, stm->APSR.bits.Z, stm->APSR.bits.N );

  TRACE_ASPR();

  return stm->pc+2;
}

#if 0
/*
 *
 */
stm_flashaddr_t str_immediate_T1(struct stm_t * stm, uint16_t opcode )
{
	union _ARMv6mThum2Instruction inst;
	inst.opcode = opcode;

	CORE_TRACE( stm, LOG_TRACE, "0x%08X STR[%04x] Rt[%d] Rn[%d] imm5[%d]\n", stm->pc &0xFFFFFFFE, inst.opcode, inst.STR_immediate_t1.Rt, inst.STR_immediate_t1.Rn, inst.STR_immediate_t1.imm5 );

	uint32_t addr = stm->R[ inst.STR_immediate_t1.Rn ] +(inst.STR_immediate_t1.imm5<<2 );
	uint32_t val = stm->R[ inst.STR_immediate_t1.Rt ];

	stm_addr_write( stm, addr, val );

	CORE_TRACE( stm, LOG_TRACE, "\tSRAM[0x%08X]=0x%08X\n", addr, val);

	return stm->pc+2;
}
#endif

#if 0
/*
 * Opcode(9001) STR r0,[sp,#0x04]
 */
stm_flashaddr_t str_sp_immediate_T2(struct stm_t * stm, uint16_t opcode )
{
	union _ARMv6mThum2Instruction inst;
	inst.opcode = opcode;

	CORE_TRACE( stm, LOG_TRACE, "0x%08X STR[%04x] Rt[%d] SP imm32[0x%x]\n", stm->pc &0xFFFFFFFE, inst.opcode, inst.STR_immediate_t1.Rt, inst.STR_immediate_t2.imm8<<2 );

	uint32_t addr = stm->R[ 13 ] +(inst.STR_immediate_t2.imm8<<2 );
	uint32_t val = stm->R[ inst.STR_immediate_t2.Rt ];

	stm_addr_write( stm, addr, val );

	CORE_TRACE( stm, LOG_TRACE, "\tSRAM[0x%08X]=0x%08X\n", addr, val);

	return stm->pc+2;
}
#endif

#if 0
/*
 * Opcode(7002) STRB r2,[r0,#0x00]
 */
stm_flashaddr_t strb_immediate_T1(struct stm_t * stm, uint16_t opcode )
{
	union _ARMv6mThum2Instruction inst;
	inst.opcode = opcode;

	CORE_TRACE( stm, LOG_TRACE, "0x%08X STRB[%04x] Rt[%d] Rn[%d] imm5[%d]\n", stm->pc &0xFFFFFFFE, inst.opcode, inst.STR_immediate_t1.Rt, inst.STR_immediate_t1.Rn, inst.STR_immediate_t1.imm5 );

	uint32_t addr = stm->R[ inst.STR_immediate_t1.Rn ] +(inst.STR_immediate_t1.imm5 );
	uint32_t val = stm->R[ inst.STR_immediate_t1.Rt ];

	stm_addr_writeb( stm, addr, val );

	CORE_TRACE( stm, LOG_TRACE, "\tSRAM[0x%08X]=0x%02x\n", addr, val);

	return stm->pc+2;
}
#endif

/*
 * STRH <Rt>,[<Rn>{,#<imm5>}]
 */
stm_flashaddr_t strh_immediate_T1(struct stm_t * stm, uint16_t opcode )
{
  union _ARMv6mThum2Instruction inst;
  inst.opcode = opcode;

  CORE_TRACE( stm, LOG_TRACE, "0x%08X STRH[%04x] Rt[%d] Rn[%d] imm5[%d]\n", stm->pc &0xFFFFFFFE, inst.opcode, inst.STRH_immediate_t1.Rt, inst.STRH_immediate_t1.Rn, inst.STRH_immediate_t1.imm5 );

  uint32_t addr = stm->R[ inst.STRH_immediate_t1.Rn ] +(inst.STRH_immediate_t1.imm5<<1 );
  uint16_t val = stm->R[ inst.STRH_immediate_t1.Rt ];

  stm_addr_writeh( stm, addr, val );

  CORE_TRACE( stm, LOG_TRACE, "\tSRAM[0x%08X]=0x%04x\n", addr, val);

  return stm->pc+2;
}

/*
 * STRH <Rt>,[<Rn>,<Rm>]
 */
stm_flashaddr_t strh_T1(struct stm_t * stm, uint16_t opcode )
{
  union _ARMv6mThum2Instruction inst;
  inst.opcode = opcode;

  CORE_TRACE( stm, LOG_TRACE, "0x%08X STRH[%04x] Rt[%d] Rn[%d] Rm[%d]\n", stm->pc &0xFFFFFFFE, inst.opcode, inst.STRH_t1.Rt, inst.STRH_t1.Rn, inst.STRH_t1.Rm );

  uint32_t addr = stm->R[ inst.STRH_t1.Rn ] +stm->R[(inst.STRH_t1.Rm )];
  uint16_t val = stm->R[ inst.STRH_t1.Rt ];

  stm_addr_writeh( stm, addr, val );

  CORE_TRACE( stm, LOG_TRACE, "\tSRAM[0x%08X]=0x%04X\n", addr, val);

  return stm->pc+2;
}

/*
 * STRB <Rt>,[<Rn>,<Rm>]
 * TODO: Implement
 */
stm_flashaddr_t strb_T1(struct stm_t * stm, uint16_t opcode )
{
  union _ARMv6mThum2Instruction inst;
  inst.opcode = opcode;

  CORE_TRACE( stm, LOG_TRACE, "0x%08X STRB[%04x] Rt[%d] Rn[%d] Rm[%d]\n", stm->pc &0xFFFFFFFE, inst.opcode, inst.STRB_t1.Rt, inst.STRB_t1.Rn, inst.STRB_t1.Rm );

  uint32_t addr = stm->R[ inst.STRB_t1.Rn ] +stm->R[(inst.STRB_t1.Rm )];
  uint8_t val = stm->R[ inst.STRB_t1.Rt ];

  stm_addr_writeb( stm, addr, val );

  CORE_TRACE( stm, LOG_TRACE, "\tSRAM[0x%08X]=0x%02X\n", addr, val);

  return stm->pc+2;
}

/*
 * ASRS (immediate) Encoding T1
 */
uint32_t signbit_spread[] =
{
    0x80000000, 0xC0000000, 0xE0000000, 0xF0000000, 0xF8000000, 0xFC000000, 0xFE000000, 0xFF000000,
    0xFF800000, 0xFFC00000, 0xFFE00000, 0xFFF00000, 0xFFF80000, 0xFFFC0000, 0xFFFE0000, 0xFFFF0000,
    0xFFFF8000, 0xFFFFC000, 0xFFFFE000, 0xFFFFF000, 0xFFFFF800, 0xFFFFFC00, 0xFFFFFE00, 0xFFFFFF00,
    0xFFFFFF80, 0xFFFFFFC0, 0xFFFFFFE0, 0xFFFFFFF0, 0xFFFFFFF8, 0xFFFFFFFC, 0xFFFFFFFE, 0xFFFFFFFF
};

uint32_t cbit_spread[]=
{
    0x00000000, 0x00000001, 0x00000002, 0x00000004, 0x00000008, 0x00000010, 0x00000020, 0x00000040,
    0x00000080, 0x00000100, 0x00000200, 0x00000400, 0x00000800, 0x00001000, 0x00002000, 0x00004000,
    0x00008000, 0x00010000, 0x00002000, 0x00040000, 0x00080000, 0x00100000, 0x00200000, 0x00400000,
    0x00800000, 0x01000000, 0x00200000, 0x04000000, 0x08000000, 0x10000000, 0x20000000, 0x40000000
};
// TODO: NOT TESTED
stm_flashaddr_t asr_immediate_T1(struct stm_t * stm, uint16_t opcode )
{
  union _ARMv6mThum2Instruction inst;
  inst.opcode = opcode;
  uint32_t Rd = inst.ASRS_immediate_t1.Rd;
  uint32_t Rm = inst.ASRS_immediate_t1.Rm;
  uint32_t imm5 = inst.ASRS_immediate_t1.imm5;
  uint32_t val = stm->R[ Rm ];
  uint32_t cbit = cbit_spread[ imm5 ];
  uint32_t sign = signbit_spread[ imm5 ];

  CORE_TRACE( stm, LOG_TRACE, "0x%08X ASR[%04x] Rd[%d] Rm[%d] imm5[%d]-Not Tested\n", stm->pc &0xFFFFFFFE, inst.opcode, Rd, Rm, imm5 );

  if( 0 < imm5  )
  {
    if( val & cbit )
      stm->APSR.bits.C = 1;
    else
      stm->APSR.bits.C = 0;

    uint32_t res =( val>>imm5 );

    if( val & 0x80000000 )
    {
        stm->APSR.bits.N = 1;
        res |= sign;
    }else{
        stm->APSR.bits.N = 0;
    }

    stm->R[ Rd ] = res;

    if( res )
      stm->APSR.bits.Z = 0;
    else
      stm->APSR.bits.Z = 1;
  }
  else
  {
    // for imm5 == 0
    stm->R[ Rd ] = val;
  }

  CORE_TRACE( stm, LOG_TRACE, "\tR[%d]=0x%08X\n", Rd, stm->R[ Rd ]);
  TRACE_ASPR();

  return stm->pc+2;
}

/*
 * ASRS (register) Encoding T1
 */
stm_flashaddr_t asr_register_T1(struct stm_t * stm, uint16_t opcode )
{
  union _ARMv6mThum2Instruction inst;
  inst.opcode = opcode;
  uint32_t Rdn = inst.ASRS_t1.Rdn;
  uint32_t Rm = inst.ASRS_t1.Rm;
  uint32_t val = stm->R[ Rm ];
  uint32_t cbit = cbit_spread[ Rm ];
  uint32_t sign = signbit_spread[ Rm ];

  CORE_TRACE( stm, LOG_TRACE, "0x%08X ASRS[%04x] Rdn[%d] Rm[%d]-Not Tested\n", stm->pc &0xFFFFFFFE, inst.opcode, Rdn, Rm );

  if( 0 < Rm  )
  {
    if( val & cbit )
      stm->APSR.bits.C = 1;
    else
      stm->APSR.bits.C = 0;

    uint32_t res =( val>>Rm );

    if( val & 0x80000000 )
    {
        stm->APSR.bits.N = 1;
        res |= sign;
    }else{
        stm->APSR.bits.N = 0;
    }

    stm->R[ Rdn ] = res;

    if( res )
      stm->APSR.bits.Z = 0;
    else
      stm->APSR.bits.Z = 1;
  }
  else
  {
    // for imm5 == 0
    stm->R[ Rdn ] = val;
  }

  CORE_TRACE( stm, LOG_TRACE, "\tRdn[%d]=0x%08X\n", Rdn, val);
  TRACE_ASPR();
  // CORE_TRACE( stm, LOG_TRACE, "\tASPR[N.Z.C.V]=[%01d.%01d.%01d.%01d]\n", stm->APSR.bits.N, stm->APSR.bits.Z, stm->APSR.bits.C, stm->APSR.bits.V );

  return stm->pc+2;
}


#if 0
/*
 *
 */
stm_flashaddr_t lsr_immediate_T1(struct stm_t * stm, uint16_t opcode )
{
	union _ARMv6mThum2Instruction inst;
	inst.opcode = opcode;
	uint32_t Rd = inst.LSRS_immediate_t1.Rd;
	uint32_t Rm = inst.LSRS_immediate_t1.Rm;
	uint32_t imm5 = inst.LSRS_immediate_t1.imm5;
	uint32_t val = stm->R[ Rm ];

	CORE_TRACE( stm, LOG_TRACE, "0x%08X LSR[%04x] Rd[%d] Rm[%d] imm5[%d]\n", stm->pc &0xFFFFFFFE, inst.opcode, Rd, Rm, imm5 );

	if( 0 < imm5  )
	{
		uint32_t res =( val>>imm5 );
		stm->R[ Rd ] = res;

		// APSR update
		stm->APSR.bits.N = 0;

		if( res )
			stm->APSR.bits.Z = 0;
		else
			stm->APSR.bits.Z = 1;

		if( val & ( 0x01 << (imm5-1) ) )
			stm->APSR.bits.C = 1;
		else
			stm->APSR.bits.C = 0;
	}
	else
	{
		// TODO Undescriber behaviour for imm5 == 0
		stm->R[ Rd ] = val;
	}

	CORE_TRACE( stm, LOG_TRACE, "\tR[%d]=0x%08X\n", Rd, val);
  TRACE_ASPR();
  // CORE_TRACE( stm, LOG_TRACE, "\tASPR[N.Z.C.V]=[%01d.%01d.%01d.%01d]\n", stm->APSR.bits.N, stm->APSR.bits.Z, stm->APSR.bits.C, stm->APSR.bits.V );

	return stm->pc+2;
}
#endif

#if 0
/*
 *
 */
// TODO:Not tested well
stm_flashaddr_t lsr_T1(struct stm_t * stm, uint16_t opcode )
{
	union _ARMv6mThum2Instruction inst;
	inst.opcode 			= opcode;
	uint32_t Rdn 			= inst.LSRS_t1.Rdn;
	uint32_t Rm 			= inst.LSRS_t1.Rm;
	uint32_t Rm_val 	= stm->R[ Rm ];
	uint32_t val 			= stm->R[ Rdn ];

	CORE_TRACE( stm, LOG_TRACE, "0x%08X LSR[%04x] Rdn[%d] Rm[%d] imm5[%d]\n", stm->pc &0xFFFFFFFE, inst.opcode, Rdn, Rm, inst.LSRS_t1.opcode );

	if( 0 < Rm_val  )
	{
		uint32_t res 				=( val>>Rm_val );
		stm->R[ Rdn ] 			= res;

		// APSR update
		stm->APSR.bits.N 		= 0;

		if( res )
			stm->APSR.bits.Z 	= 0;
		else
			stm->APSR.bits.Z 	= 1;

		if( val & ( 0x01 << (Rm_val-1) ) )
			stm->APSR.bits.C 	= 1;
		else
			stm->APSR.bits.C 	= 0;
	}
	else
	{
		stm->R[ Rdn ] 			= val;
	}

	CORE_TRACE( stm, LOG_TRACE, "\tR[%d]=0x%08X\n", Rdn, Rm_val);
  TRACE_ASPR();
  // CORE_TRACE( stm, LOG_TRACE, "\tASPR[N.Z.C.V]=[%01d.%01d.%01d.%01d]\n", stm->APSR.bits.N, stm->APSR.bits.Z, stm->APSR.bits.C, stm->APSR.bits.V );

	return stm->pc+2;
}
#endif

/*
 * RORS (register) Encoding T1
 */
// TODO:Not tested well
stm_flashaddr_t ror_register_T1(struct stm_t * stm, uint16_t opcode )
{
  union _ARMv6mThum2Instruction inst;
  inst.opcode       = opcode;
  uint32_t Rdn      = inst.RORS_register_t1.Rdn;
  uint32_t Rm       = inst.RORS_register_t1.Rm;
  uint32_t Rm_val   = stm->R[ Rm ] & 0xff;
  uint32_t val      = stm->R[ Rdn ];

  uint32_t res;

  CORE_TRACE( stm, LOG_TRACE, "0x%08X RORS[%04x] Rdn[%d] Rm[%d]-NOT TESTED\n", stm->pc &0xFFFFFFFE, inst.opcode, Rdn, Rm );

  if( 0 < Rm_val  )
  {
    uint32_t t1, t2;

    Rm_val = Rm_val % 32;
    t1 = Rdn >> Rm_val;
    t2 = Rdn <<( 32 - Rm_val );
    res = t1 | t2;

    if( val &( 0x01 <<( Rm_val-1 )))
      stm->APSR.bits.C  = 1;
    else
      stm->APSR.bits.C  = 0;
  }
  else
    res = val;

  stm->R[ Rdn ] = res;

  // APSR update
  if( res & 0x80000000 )
    stm->APSR.bits.N    = 1; // TODO: Check this
  else
    stm->APSR.bits.N    = 0; // TODO: Check this

  if( res )
    stm->APSR.bits.Z  = 0;
  else
    stm->APSR.bits.Z  = 1;

  CORE_TRACE( stm, LOG_TRACE, "\tR[%d]=0x%08X\n", Rdn, Rm_val);
  TRACE_ASPR();
  // CORE_TRACE( stm, LOG_TRACE, "\tASPR[N.Z.C.V]=[%01d.%01d.%01d.%01d]\n", stm->APSR.bits.N, stm->APSR.bits.Z, stm->APSR.bits.C, stm->APSR.bits.V );

  return stm->pc+2;
}

#if 0

/*
 *
 */
stm_flashaddr_t lsl_immediate_T1(struct stm_t * stm, uint16_t opcode )
{
	union _ARMv6mThum2Instruction inst;
	inst.opcode = opcode;
	uint32_t Rd = inst.LSLS_immediate_t1.Rd;
	uint32_t Rm = inst.LSLS_immediate_t1.Rm;
	uint32_t imm5 = inst.LSLS_immediate_t1.imm5;
	uint32_t val = stm->R[ Rm ];

	CORE_TRACE( stm, LOG_TRACE, "0x%08X LSL[%04x] Rd[%d] Rm[%d] imm5[%d]\n", stm->pc &0xFFFFFFFE, inst.opcode, Rd, Rm, imm5 );

	if( 0 < imm5 )
	{
		uint32_t res =( val<<imm5 );
		stm->R[ Rd ] = res;

		// APSR update
		if( res & 0x80000000 )
			stm->APSR.bits.N = 1;
		else
			stm->APSR.bits.N = 0;

		if( !res )
			stm->APSR.bits.Z = 1;
		else
			stm->APSR.bits.Z = 0;

		if( val & ( 0x01 << (32-imm5) ) )
			stm->APSR.bits.C = 1;
		else
			stm->APSR.bits.C = 0;
	}
	else
	{
		// TODO Undescribed behaviour for imm5 == 0
		stm->R[ Rd ] = val;

		// APSR update
		if( val & 0x80000000 )
			stm->APSR.bits.N = 1;
		else
			stm->APSR.bits.N = 0;

		if( val )
			stm->APSR.bits.Z = 0;
		else
			stm->APSR.bits.Z = 1;

		stm->APSR.bits.C = 0;
	}

	CORE_TRACE( stm, LOG_TRACE, "\tR[%d]=0x%08X\n", Rd, stm->R[ Rd ]);
  TRACE_ASPR();
  // CORE_TRACE( stm, LOG_TRACE, "\tASPR[N.Z.C.V]=[%01d.%01d.%01d.%01d]\n", stm->APSR.bits.N, stm->APSR.bits.Z, stm->APSR.bits.C, stm->APSR.bits.V );

	return stm->pc+2;
}
#endif

#if 0
/*
 * opcode[4095] LSLS r5,r5,r2
 */
stm_flashaddr_t lsl_T1(struct stm_t * stm, uint16_t opcode )
{
	union _ARMv6mThum2Instruction inst;
	inst.opcode = opcode;
	uint32_t Rdn = inst.LSLS_reg_t1.Rdn;
	uint32_t Rm = inst.LSLS_reg_t1.Rm;
	uint32_t val = stm->R[ Rdn ];
	uint32_t Rm_val = stm->R[ Rm ];

	CORE_TRACE( stm, LOG_TRACE, "0x%08X LSLS[%04x] Rdn[%d] Rm[%d]\n", stm->pc &0xFFFFFFFE, inst.opcode, Rdn, Rm );

	if( 0 < Rm_val )
	{
		uint32_t res =( val<<Rm_val );
		stm->R[ Rdn ] = res;

		// APSR update
		if( res & 0x80000000 )
			stm->APSR.bits.N = 1;
		else
			stm->APSR.bits.N = 0;

		if( !res )
			stm->APSR.bits.Z = 1;
		else
			stm->APSR.bits.Z = 0;

		if( val & ( 0x01 << (32-Rm_val) ) )
			stm->APSR.bits.C = 1;
		else
			stm->APSR.bits.C = 0;
	}
	else
	{
		// TODO Undescribed behaviour for imm5 == 0
		stm->R[ Rdn ] = val;

		// APSR update
		if( val & 0x80000000 )
			stm->APSR.bits.N = 1;
		else
			stm->APSR.bits.N = 0;

		if( val )
			stm->APSR.bits.Z = 0;
		else
			stm->APSR.bits.Z = 1;

		stm->APSR.bits.C = 0;
	}

	CORE_TRACE( stm, LOG_TRACE, "\tR[%d]=0x%08X\n", Rdn, stm->R[ Rdn ]);
  TRACE_ASPR();
  // CORE_TRACE( stm, LOG_TRACE, "\tASPR[N.Z.C.V]=[%01d.%01d.%01d.%01d]\n", stm->APSR.bits.N, stm->APSR.bits.Z, stm->APSR.bits.C, stm->APSR.bits.V );

	return stm->pc+2;
}
#endif

#if 0
/*
 *
 */
stm_flashaddr_t bics_register_T1(struct stm_t * stm, uint16_t opcode )
{
	union _ARMv6mThum2Instruction inst;
	inst.opcode 	= opcode;
	uint32_t Rdn	= inst.BICS_t1.Rdn;
	uint32_t Rm		= inst.BICS_t1.Rm;
	uint32_t res 	= stm->R[ Rdn ];

	CORE_TRACE( stm, LOG_TRACE, "0x%08X BICS[%04x] Rn[%d] Rm[%d]\n", stm->pc &0xFFFFFFFE, opcode, Rdn, Rm );

	res &=  ~(stm->R[ Rm ]);
	stm->R[ Rdn ] = res;

	// APSR update
	if( res & 0x80000000 )
		stm->APSR.bits.N = 1;
	else
		stm->APSR.bits.N = 0;

	if( res )
		stm->APSR.bits.Z = 0;
	else
		stm->APSR.bits.Z = 1;

//	stm->APSR.bits.C = 0;

	CORE_TRACE( stm, LOG_TRACE, "\tR[%d]=0x%08X\n", Rdn, res);
  TRACE_ASPR();
  // CORE_TRACE( stm, LOG_TRACE, "\tASPR[N.Z.C.V]=[%01d.%01d.%01d.%01d]\n", stm->APSR.bits.N, stm->APSR.bits.Z, stm->APSR.bits.C, stm->APSR.bits.V );

  return stm->pc+2;
}
#endif

/*
 * CMP Rn<Rm
 */
stm_flashaddr_t cmp_register_T2(struct stm_t * stm, uint16_t opcode )
{
  union _ARMv6mThum2Instruction inst;
  inst.opcode   = opcode;
  uint32_t Rn   = inst.CMP_reg_t2.Rn;
  uint32_t Rm   = inst.CMP_reg_t2.Rm;
  uint32_t Rn_val = stm->R[ Rn ];
  uint32_t Rm_val = stm->R[ Rm ];
  uint32_t res;

  CORE_TRACE( stm, LOG_TRACE, "0x%08X CMP[%04x] Rn[%d] Rm[%d]\n", stm->pc &0xFFFFFFFE, opcode, Rn, Rm );

  Rn += inst.CMP_reg_t2.DN<<3;

  if((( Rn < 8 )&&( Rm < 8 ))
      ||( Rn == 0x0f )
      ||( Rm == 0x0f ))
  {
      stm->state = cpu_Crashed;
      CORE_TRACE( stm, LOG_TRACE, "0x%08X CMN[%04x] Crashed Rn[%d] Rm[%d]\n", stm->pc &0xFFFFFFFE, opcode, Rn, Rm );

      return stm->pc;
  }

  ADD_WITH_CARRY( res, stm->APSR.bits.C, stm->APSR.bits.V, Rn_val, ~Rm_val, 1 );

  // APSR update
  UPDATE_Z_N( res, stm->APSR.bits.Z, stm->APSR.bits.N );

  TRACE_ASPR();

  return stm->pc+2;
}

/*
 * Opcode(0x42C1) CMN <Rn>,<Rm>
 */
stm_flashaddr_t cmn_register_T1(struct stm_t * stm, uint16_t opcode )
{
  union _ARMv6mThum2Instruction inst;
  inst.opcode   = opcode;
  uint32_t Rn   = stm->R[ inst.CMN_t1.Rn ];
  uint32_t Rm   = stm->R[ inst.CMN_t1.Rm ];
  uint32_t res;

  CORE_TRACE( stm, LOG_TRACE, "0x%08X CMN[%04x%d] Rm[%d]\n", stm->pc &0xFFFFFFFE, opcode, Rn, Rm );

  ADD_WITH_C_V( res, stm->APSR.bits.C, stm->APSR.bits.V, Rn, Rm );

  // APSR update
  UPDATE_Z_N( res, stm->APSR.bits.Z, stm->APSR.bits.N );

  TRACE_ASPR();

  return stm->pc+2;
}

#if 0
/*
 * Opcode(B2D2) UXTB r2,r2
 */
stm_flashaddr_t uxtb_T1(struct stm_t * stm, uint16_t opcode )
{
	union _ARMv6mThum2Instruction inst;
	inst.opcode 	= opcode;
	uint32_t Rd	= inst.UXTB_t1.Rd;

	CORE_TRACE( stm, LOG_TRACE, "0x%08X UXTB[%04x] Rd[%d] Rm[%d]\n", stm->pc &0xFFFFFFFE, opcode, Rd, inst.UXTB_t1.Rm );

  stm->R[ Rd ]= (stm->R[ inst.UXTB_t1.Rm ])&0x0f;

	CORE_TRACE( stm, LOG_TRACE, "\tR[%d]=0x%08X\n", Rd, stm->R[ Rd ]);

  return stm->pc+2;
}
#endif

/*
 * UXTH <Rd>,<Rm>
 * TODO: Implement
 */
stm_flashaddr_t uxth_T1(struct stm_t * stm, uint16_t opcode )
{
  union _ARMv6mThum2Instruction inst;
  inst.opcode   = opcode;
  uint32_t Rd = inst.UXTH_t1.Rd;

  CORE_TRACE( stm, LOG_TRACE, "0x%08X UXTH[%04x] Rd[%d] Rm[%d]\n", stm->pc &0xFFFFFFFE, opcode, Rd, inst.UXTH_t1.Rm );

  stm->R[ Rd ]= (stm->R[ inst.UXTH_t1.Rm ])&0xffff;

  CORE_TRACE( stm, LOG_TRACE, "\tR[%d]=0x%08X\n", Rd, stm->R[ Rd ]);

  return stm->pc+2;
}

/*
 * MULS <Rdm>,<Rn>,<Rdm>
 * TODO: Implement
 */
stm_flashaddr_t muls_register_T1(struct stm_t * stm, uint16_t opcode )
{
  union _ARMv6mThum2Instruction inst;
  inst.opcode   = opcode;
  // uint32_t Rdm = inst.MULS_t1.Rdm;

  uint32_t res = stm->R[inst.MULS_t1.Rn] * stm->R[inst.MULS_t1.Rdm];

  CORE_TRACE( stm, LOG_TRACE, "0x%08X MULS[%04x] Rn[%d] Rdm[%d]\n", stm->pc &0xFFFFFFFE, opcode, inst.MULS_t1.Rn, inst.MULS_t1.Rdm );

  stm->R[inst.MULS_t1.Rdm] = res;

  // APSR update
  UPDATE_Z_N( res, stm->APSR.bits.Z, stm->APSR.bits.N );

  TRACE_ASPR();

  return stm->pc+2;
}

/*
 * SXTH <Rd>,<Rm>
 */
stm_flashaddr_t sxth_T1(struct stm_t * stm, uint16_t opcode )
{
  union _ARMv6mThum2Instruction inst;
  inst.opcode   = opcode;
  uint32_t Rd = inst.SXTH_t1.Rd;

  CORE_TRACE( stm, LOG_TRACE, "0x%08X SXTH[%04x] Rd[%d] Rm[%d]\n", stm->pc &0xFFFFFFFE, opcode, Rd, inst.SXTH_t1.Rm );

  stm->R[ Rd ]= (stm->R[ inst.SXTH_t1.Rm ])&0xffff;

  if( stm->R[ Rd ] & 0x8000 )
    stm->R[ Rd ] |= 0xFFFF0000;

  CORE_TRACE( stm, LOG_TRACE, "\tR[%d]=0x%08X\n", Rd, stm->R[ Rd ]);

  return stm->pc+2;
}
/*
 * SXTB <Rd>,<Rm>
 */
stm_flashaddr_t sxtb_T1(struct stm_t * stm, uint16_t opcode )
{
  union _ARMv6mThum2Instruction inst;
  inst.opcode   = opcode;
  uint32_t Rd = inst.SXTB_t1.Rd;

  CORE_TRACE( stm, LOG_TRACE, "0x%08X SXTH[%04x] Rd[%d] Rm[%d]\n", stm->pc &0xFFFFFFFE, opcode, Rd, inst.SXTB_t1.Rm );

  stm->R[ Rd ]= (stm->R[ inst.SXTB_t1.Rm ])&0xff;

  if( stm->R[ Rd ] & 0x80 )
    stm->R[ Rd ] |= 0xFFFFFF00;

  CORE_TRACE( stm, LOG_TRACE, "\tR[%d]=0x%08X\n", Rd, stm->R[ Rd ]);

  return stm->pc+2;
}
/*
 * CPS<effect> i
 */
stm_flashaddr_t cps_T1(struct stm_t * stm, uint16_t opcode )
{
  union _ARMv6mThum2Instruction inst;
  inst.opcode   = opcode;

  do{}while(!inst.opcode);

  CORE_TRACE( stm, LOG_TRACE, "0x%08X SXTH[%04x] IM[%d] BITS[%01x]: Not Supported\n", stm->pc &0xFFFFFFFE, opcode, inst.CPS_t1.IM, inst.CPS_t1.BITs );

  stm->state = cpu_Crashed;

  // TODO: Realise

  return stm->pc;
}
/*
 * REV <Rd>,<Rm>
 */
stm_flashaddr_t rev_T1(struct stm_t * stm, uint16_t opcode )
{
  union _ARMv6mThum2Instruction inst;
  inst.opcode   = opcode;
  uint32_t Rd = inst.REV_t1.Rd;
  uint32_t Rm_val = stm->R[ inst.REV_t1.Rm ];

  CORE_TRACE( stm, LOG_TRACE, "0x%08X REV[%04x] Rd[%d] Rm[%d]\n", stm->pc &0xFFFFFFFE, opcode, Rd, inst.REV_t1.Rm );

  stm->R[ Rd ]= (( Rm_val &0xff )<<24 ) // result<31:24> = R[m]<7:0>;
      +(( Rm_val&0xff00 )<<8)           // result<23:16> = R[m]<15:8>;
      +(( Rm_val&0xff0000 )>>8)         // result<15:8> = R[m]<23:16>;
      +(( Rm_val&0xff000000 )>>24);     // result<7:0> = R[m]<31:24>;


  CORE_TRACE( stm, LOG_TRACE, "\tR[%d]=0x%08X\n", Rd, stm->R[ inst.REV_t1.Rm ]);

  stm->state = cpu_Running;

  return stm->pc+2;
}
/*
 * REV16 <Rd>,<Rm>
 */
stm_flashaddr_t rev16_T1(struct stm_t * stm, uint16_t opcode )
{
  union _ARMv6mThum2Instruction inst;
  inst.opcode   = opcode;
  uint32_t Rd = inst.REV16_t1.Rd;
  uint32_t Rm_val = stm->R[ inst.REV16_t1.Rm ];

  CORE_TRACE( stm, LOG_TRACE, "0x%08X REV16[%04x] Rd[%d] Rm[%d]\n", stm->pc &0xFFFFFFFE, opcode, Rd, inst.REV16_t1.Rm );

  stm->R[ Rd ]= (( Rm_val &0x00ff0000 )<<8 ) // result<31:24> = R[m]<23:16>;
      +(( Rm_val&0xff000000 )>>8)           // result<23:16> = R[m]<31:24>;
      +(( Rm_val&0x000000ff )<<8)                 // result<15:8> = R[m]<7:0>;
      +(( Rm_val&0x0000ff00 )>>8);              // result<7:0> = R[m]<15:8>;

  CORE_TRACE( stm, LOG_TRACE, "\tR[%d]=0x%08X\n", Rd, stm->R[ inst.REV16_t1.Rm ]);

  stm->state = cpu_Running;

  return stm->pc+2;
}
/*
 * REVSH <Rd>,<Rm>
 */
stm_flashaddr_t revsh_T1(struct stm_t * stm, uint16_t opcode )
{
  union _ARMv6mThum2Instruction inst;
  inst.opcode   = opcode;
  uint32_t Rd = inst.REVSH_t1.Rd;
  uint32_t Rm_val = stm->R[ inst.REVSH_t1.Rm ];

  CORE_TRACE( stm, LOG_TRACE, "0x%08X REVSH[%04x] Rd[%d] Rm[%d]\n", stm->pc &0xFFFFFFFE, opcode, Rd, inst.REVSH_t1.Rm );

  stm->R[ Rd ]= (( Rm_val &0xff )<<8 )    // result<31:8> = SignExtend(R[m]<7:0>, 24);
      +(( Rm_val&0xff00 )>>8);            // result<7:0> = R[m]<15:8>;

  if( stm->R[ Rd ]&0x8000 )
    stm->R[ Rd ] |= 0xffff0000;
  else
    stm->R[ Rd ] &= 0x7fff;

  CORE_TRACE( stm, LOG_TRACE, "\tR[%d]=0x%08X\n", Rd, stm->R[ inst.REVSH_t1.Rm ]);

  stm->state = cpu_Running;

  return stm->pc+2;
}
/*
 * BKPT #<imm8>
 */
stm_flashaddr_t bkpt_T1(struct stm_t * stm, uint16_t opcode )
{
  CORE_TRACE( stm, LOG_TRACE, "0x%08X BKPT[%04x]:Not implemented\n", stm->pc &0xFFFFFFFE, opcode );

  stm->state = cpu_Crashed;

  return stm->pc;
}
/*
 * BKPT #<imm8>
 */
stm_flashaddr_t yield_T1(struct stm_t * stm, uint16_t opcode )
{
  CORE_TRACE( stm, LOG_TRACE, "0x%08X YIELD[%04x]:Not Supported\n", stm->pc &0xFFFFFFFE, opcode );

  stm->state = cpu_Crashed;

  return stm->pc;
}
/*
 * WFE
 */
stm_flashaddr_t wfe_T1(struct stm_t * stm, uint16_t opcode )
{
  CORE_TRACE( stm, LOG_TRACE, "0x%08X WFE[%04x]:Not implemented\n", stm->pc &0xFFFFFFFE, opcode );

  stm->state = cpu_Crashed;

  return stm->pc;
}
/*
 * WFI
 */
stm_flashaddr_t wfi_T1(struct stm_t * stm, uint16_t opcode )
{
  CORE_TRACE( stm, LOG_TRACE, "0x%08X WFI[%04x]:Not implemented\n", stm->pc &0xFFFFFFFE, opcode );

  stm->state = cpu_Crashed;

  return stm->pc;
}
/*
 * SEV
 */
stm_flashaddr_t sev_T1(struct stm_t * stm, uint16_t opcode )
{
  CORE_TRACE( stm, LOG_TRACE, "0x%08X SEV[%04x]:Not implemented\n", stm->pc &0xFFFFFFFE, opcode );

  stm->state = cpu_Crashed;

  return stm->pc;
}

/*
 * UNPREDICTABLE
 */
stm_flashaddr_t unpredictable(struct stm_t * stm, uint16_t opcode )
{
  CORE_TRACE( stm, LOG_TRACE, "0x%08X UNPREDICTABLE[%04x]\n", stm->pc &0xFFFFFFFE, opcode );

  stm->state = cpu_Crashed;

  return stm->pc;
}

/*
 * Pushes core to interrupt context
 */
uint32_t interrupt_context_push(struct stm_t * stm )
{
  CORE_TRACE( stm, LOG_TRACE, "INT CTX into\n" );

  uint32_t sp_addr = stm->R[ 13 ];

  if( stm->mcu->scb->scb_regs->CCR &( 0x01<<9 ))
  {
    stm_addr_write( stm, sp_addr, 0xaa55aa55 );
    CORE_TRACE( stm, LOG_TRACE, "\tSTKALIGN\n" );
    sp_addr -=4;

    // cpu cycles update
    stm->cycle += 9;
    // nano ticks update
    stm->nano_ticks += stm->nano_ticks_per_cycle *9;
  }
  else
  {
    // cpu cycles update
    stm->cycle += 8;
    // nano ticks update
    stm->nano_ticks += stm->nano_ticks_per_cycle *8;
  }

  // push xPSR
  uint32_t val = stm->APSR.val;
  val |= stm->IPSR.val;
  val |= stm->EPSR.val;
  stm_addr_write( stm, sp_addr, val );
  CORE_TRACE( stm, LOG_TRACE, "\t@0x%08X=xPSR(0x%08X)\n", sp_addr, val );
  sp_addr -=4;

  // push PC
  stm_addr_write( stm, sp_addr, stm->pc );
  CORE_TRACE( stm, LOG_TRACE, "\t@0x%08X=PC(0x%08X)\n", sp_addr, stm->pc );
  sp_addr -=4;

  // push LR
  stm_addr_write( stm, sp_addr, stm->R[ 14 ] );
  CORE_TRACE( stm, LOG_TRACE, "\t@0x%08X=LR(0x%08X)\n", sp_addr, stm->R[ 14 ]);
  sp_addr -=4;

  // push R12
  stm_addr_write( stm, sp_addr, stm->R[12] );
  CORE_TRACE( stm, LOG_TRACE, "\t@0x%08X=R12(0x%08X)\n", sp_addr, stm->R[ 12 ]);
  sp_addr -=4;

  // push R3
  stm_addr_write( stm, sp_addr, stm->R[ 3 ]);
  CORE_TRACE( stm, LOG_TRACE, "\t@0x%08X=R3(0x%08X)\n", sp_addr, stm->R[ 3 ]);
  sp_addr -=4;

  // push R2
  stm_addr_write( stm, sp_addr, stm->R[ 2 ]);
  CORE_TRACE( stm, LOG_TRACE, "\t@0x%08X=R2(0x%08X)\n", sp_addr, stm->R[ 2 ]);
  sp_addr -=4;

  // push R1
  stm_addr_write( stm, sp_addr, stm->R[ 1 ]);
  CORE_TRACE( stm, LOG_TRACE, "\t@0x%08X=R1(0x%08X)\n", sp_addr, stm->R[ 1 ]);
  sp_addr -=4;

  // push R0
  stm_addr_write( stm, sp_addr, stm->R[ 0 ]);
  CORE_TRACE( stm, LOG_TRACE, "\t@0x%08X=R3(0x%08X)\n", sp_addr, stm->R[ 0 ]);
  sp_addr -=4;

  stm->R[ 13 ]= sp_addr;

  stm->R[ 14 ]= 0xFFFFFFF9;

  CORE_TRACE( stm, LOG_TRACE, "\tSP(@0x%08X)\n", sp_addr);
  CORE_TRACE( stm, LOG_TRACE, "\tcycle(@0x%08X)\n", stm->cycle);
//  CORE_TRACE( stm, LOG_TRACE, "\tticks(@0x%08X)\n", stm->cycle);

  return 0x00;
}
/*
 * if higher priority interrupt is coming update handler address
 */
uint32_t interrupt_set_handler_adr(struct stm_t * stm, stm_flashaddr_t interrupt_handler_adr )
{
  return interrupt_handler_adr;
}
/*
 * Pops core from interrupt and context restore
 */
uint32_t interrupt_context_pop(struct stm_t * stm )
{
  CORE_TRACE( stm, LOG_TRACE, "INT CTX out\n" );

  uint32_t val;

  uint32_t sp_addr = stm->R[ 13 ];

  // pop R0
  sp_addr +=4;
  stm_addr_read( stm, sp_addr, &val );
  stm->R[ 0 ]= val;
  CORE_TRACE( stm, LOG_TRACE, "\t@0x%08X=R3(0x%08X)\n", sp_addr, stm->R[ 0 ]);

  // pop R1
  sp_addr +=4;
  stm_addr_read( stm, sp_addr, &val );
  stm->R[ 1 ]= val;
  CORE_TRACE( stm, LOG_TRACE, "\t@0x%08X=R1(0x%08X)\n", sp_addr, stm->R[ 1 ]);

  // pop R2
  sp_addr +=4;
  stm_addr_read( stm, sp_addr, &val );
  stm->R[ 2 ]= val;
  CORE_TRACE( stm, LOG_TRACE, "\t@0x%08X=R2(0x%08X)\n", sp_addr, stm->R[ 2 ]);

  // pop R3
  sp_addr +=4;
  stm_addr_read( stm, sp_addr, &val );
  stm->R[ 3 ]= val;
  CORE_TRACE( stm, LOG_TRACE, "\t@0x%08X=R3(0x%08X)\n", sp_addr, stm->R[ 3 ]);

  // pop R12
  sp_addr +=4;
  stm_addr_read( stm, sp_addr, &val );
  stm->R[ 12 ]= val;
  CORE_TRACE( stm, LOG_TRACE, "\t@0x%08X=R12(0x%08X)\n", sp_addr, stm->R[ 12 ]);

  // pop LR
  sp_addr +=4;
  stm_addr_read( stm, sp_addr, &val );
  stm->R[ 14 ]= val;
  CORE_TRACE( stm, LOG_TRACE, "\t@0x%08X=LR(0x%08X)\n", sp_addr, stm->R[ 14 ]);

  // pop PC
  sp_addr +=4;
  stm_addr_read( stm, sp_addr, &val );
  stm->pc = val;
  CORE_TRACE( stm, LOG_TRACE, "\t@0x%08X=PC(0x%08X)\n", sp_addr, stm->pc );

  // pop xPSR
  sp_addr +=4;
  stm_addr_read( stm, sp_addr, &val );
  stm->APSR.val = val & 0xf0000000;
  stm->IPSR.val = val & ( 1<<24 );
  stm->EPSR.val = val & 0x0000003f;
  CORE_TRACE( stm, LOG_TRACE, "\t@0x%08X=xPSR(0x%08X)\n", sp_addr, val );

  if( stm->mcu->scb->scb_regs->CCR &( 0x01<<9 ))
  {
    sp_addr +=4;
    CORE_TRACE( stm, LOG_TRACE, "\tSTKALIGN\n" );
    // cpu cycles update
    stm->cycle += 9;
    // nano ticks update
    stm->nano_ticks += stm->nano_ticks_per_cycle *9;
  }
  else
  {
    // cpu cycles update
    stm->cycle += 8;
    // nano ticks update
    stm->nano_ticks += stm->nano_ticks_per_cycle *8;
  }

  stm->R[ 13 ]= sp_addr;

  CORE_TRACE( stm, LOG_TRACE, "\tSP(@0x%08X)\n", sp_addr);
  CORE_TRACE( stm, LOG_TRACE, "\tcycle(@0x%08X)\n", stm->cycle);
//  CORE_TRACE( stm, LOG_TRACE, "\tticks(@0x%08X)\n", stm->cycle);

  return 0x00;
}



