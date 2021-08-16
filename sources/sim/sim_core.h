/*
	sim_core.h

 */

#ifndef __SIM_CORE_H__
#define __SIM_CORE_H__

#ifdef __cplusplus
extern "C" {
#endif

#ifdef NO_COLOR
	#define FONT_GREEN
	#define FONT_RED
	#define FONT_DEFAULT
#else
	#define FONT_GREEN		"\e[32m"
	#define FONT_RED		"\e[31m"
	#define FONT_DEFAULT	"\e[0m"
#endif


void stm_core_watch_write(stm_t *stm, uint32_t addr, uint8_t v);

/*
 * Instruction decoder, run ONE instruction
 */
uint32_t stm_run_one_no_bp(stm_t * stm);

uint32_t stm_run_one(stm_t * stm);

/*
 * These are for internal access to the stack (for interrupts)
 */
uint32_t _stm_sp_get(stm_t * stm);
void _stm_sp_set(stm_t * stm, uint32_t sp);
//int _avr_push_addr(avr_t * avr, stm_flashaddr_t addr);

#if CONFIG_SIMAVR_TRACE

/*
 * Get a "pretty" register name
 */
const char * avr_regname(uint8_t reg);

/*
 * DEBUG bits follow
 * These will disappear when gdb arrives
 */
void avr_dump_state(avr_t * avr);

#define DUMP_REG() { \
				for (int i = 0; i < 32; i++) printf("%s=%02x%c", avr_regname(i), avr->data[i],i==15?'\n':' ');\
				printf("\n");\
				uint16_t y = avr->data[R_YL] | (avr->data[R_YH]<<8);\
				for (int i = 0; i < 20; i++) printf("Y+%02d=%02x ", i, avr->data[y+i]);\
				printf("\n");\
		}


#if AVR_STACK_WATCH
#define DUMP_STACK() \
		for (int i = avr->trace_data->stack_frame_index; i; i--) {\
			int pci = i-1;\
			printf(FONT_RED "*** %04x: %-25s sp %04x\n" FONT_DEFAULT,\
					avr->trace_data->stack_frame[pci].pc, \
					avr->trace_data->codeline ? avr->trace_data->codeline[avr->trace_data->stack_frame[pci].pc>>1]->symbol : "unknown", \
							avr->trace_data->stack_frame[pci].sp);\
		}
#else
#define DUMP_STACK()
#endif

#else /* CONFIG_SIMAVR_TRACE */

#define DUMP_STACK()
#define DUMP_REG();

#endif

/**
 * Reconstructs the SREG value from avr->sreg into dst.
 */
#define READ_SREG_INTO(avr, dst) { \
			dst = 0; \
			for (int i = 0; i < 8; i++) \
				if (avr->sreg[i] > 1) { \
					printf("** Invalid SREG!!\n"); \
				} else if (avr->sreg[i]) \
					dst |= (1 << i); \
		}

static inline void stm_sreg_set( stm_t * stm, uint8_t flag, uint8_t ival)
{
	/*
	 *	clear interrupt_state if disabling interrupts.
	 *	set wait if enabling interrupts.
	 *	no change if interrupt flag does not change.
	 */

//	if (flag == S_I) {
//		if (ival) {
//			if (!stm->sreg[S_I])
//				stm->interrupt_state = -2;
//		} else
//			stm->interrupt_state = 0;
//	}
//
//	stm->sreg[flag] = ival;
}

/**
 * Splits the SREG value from src into the avr->sreg array.
 */
#define SET_SREG_FROM(stm, src) { \
			for (int i = 0; i < 8; i++) \
				stm_sreg_set(stm, i, (src & (1 << i)) != 0); \
		}

/*
 * Opcode is sitting at the end of the flash to catch PC overflows.
 * Apparently it's used by some code to simulate soft reset?
 */
#define STM_OVERFLOW_OPCODE 0xf1f1


union _ARMv6mThum2Instruction
{
	uint16_t opcode;
	// LDR literal
	struct LDR_literal
	{
		unsigned imm8 	: 8;
		unsigned Rt			: 3;
		unsigned inst  	: 5;
	}LDR_literal;
	// LDR immediate
	struct LDR_immediate
	{
		unsigned Rt 		: 3;
		unsigned Rn 		: 3;
		unsigned imm5 	: 5;
		unsigned inst 	: 5;
	}LDR_immediate;
	// LDR SP immediate T2
	struct LDR_sp_imm8_t2
	{
		unsigned imm8 	: 8;
		unsigned Rt 		: 3;
		unsigned inst 	: 5;
	}LDR_sp_imm8_t2;
	// LDRB immediate
	struct LDRB_imm5_t1
	{
		unsigned Rt 		: 3;
		unsigned Rn 		: 3;
		unsigned imm5 	: 5;
		unsigned inst 	: 5;
	}LDRB_imm5_t1;
	// LDR register t1
	struct LDR_regs_t1
	{
		unsigned Rt 		: 3;
		unsigned Rn 		: 3;
		unsigned Rm   	: 3;
		unsigned opcode : 7;
	}LDR_regs_t1;
  // LDRSB register t1
  struct LDRSB_regs_t1
  {
    unsigned Rt     : 3;
    unsigned Rn     : 3;
    unsigned Rm     : 3;
    unsigned opcode : 7;
  }LDRSB_regs_t1;
  // LDRSH register t1
  struct LDRSH_regs_t1
  {
    unsigned Rt     : 3;
    unsigned Rn     : 3;
    unsigned Rm     : 3;
    unsigned opcode : 7;
  }LDRSH_regs_t1;
  // LDRH register t1
  struct LDRH_regs_t1
  {
    unsigned Rt     : 3;
    unsigned Rn     : 3;
    unsigned Rm     : 3;
    unsigned opcode : 7;
  }LDRH_regs_t1;
  // LDRH immediate t1
  struct LDRH_imm_t1
  {
    unsigned Rt     : 3;
    unsigned Rn     : 3;
    unsigned imm5   : 5;
    unsigned opcode : 5;
  }LDRH_imm_t1;
  // LDRB register t1
	struct LDRB_regs_t1
	{
		unsigned Rt 		: 3;
		unsigned Rn 		: 3;
		unsigned Rm   	: 3;
		unsigned opcode : 7;
	}LDRB_regs_t1;
	// LDM t1
	struct LDM_t1
	{
		unsigned rlist 	: 8;
		unsigned Rn			: 3;
		unsigned inst		: 5;
	}LDM_t1;
	// POP_t1
	struct POP_t1
	{
		unsigned rlist 	: 8;
		unsigned P			: 1;
		unsigned inst		: 7;
	}POP_t1;
	// STM_t1
	struct STM_t1
	{
		unsigned rlist 	: 8;
		unsigned Rn			: 3;
		unsigned inst		: 5;
	}STM_t1;
	// PUSH
	struct PUSH_t1
	{
		unsigned rlist 	: 8;
		unsigned M			: 1;
		unsigned inst		: 7;
	}PUSH_t1;
	// BLX
	struct BLX
	{
		unsigned rest : 3;
		unsigned Rm : 4;
		unsigned inst : 9;
	} BLX;
	// BX
	struct BX_t1
	{
		unsigned zeros 	: 3;
		unsigned Rm			: 4;
		unsigned opcode	: 9;
	}BX_t1;
	// BL part1
	struct BL_t1_part1
	{
		unsigned imm10 	: 10;
		unsigned S			: 1;
		unsigned opcode	: 5;
	}BL_t1_part1;
	// BL part2
	struct BL_t1_part2
	{
		unsigned imm11 	: 11;
		unsigned J2			: 1;
		unsigned one 		: 1;
		unsigned J1			: 1;
		unsigned opcode	: 2;
	}BL_t1_part2;
	// B t2
	struct B_t2
	{
		unsigned imm11 	: 11;
		unsigned opcode	: 5;
	}B_t2;
	// B t1
	struct B_t1
	{
		unsigned imm8		: 8;
		unsigned cond		: 4;
		unsigned opcode	: 4;
	}B_t1 ;
	// MOVS
	struct MOVS
	{
		unsigned imm8 	: 8;
		unsigned Rd 		: 3;
		unsigned opcode : 5;
	} MOVS;
	// MOV
	struct MOV_t1
	{
		unsigned Rd 		: 3;
		unsigned Rm 		: 4;
		unsigned D 			: 1;
		unsigned opcode	: 8;
	}MOV_t1;
	// ADDS_t1
	struct ADDS_t1
	{
		unsigned Rd			: 3;
		unsigned Rn			: 3;
		unsigned imm3 	: 3;
		unsigned opcode : 7;
	}ADDS_t1;
	// ADDS_t2
	struct ADDS_t2
	{
		unsigned imm8 	: 8;
		unsigned Rdn 		: 3;
		unsigned opcode : 5;
	}ADDS_t2;
	// ADDS_sp_imm_t1
	struct ADDS_sp_imm_t1
	{
		unsigned imm8 	: 8;
		unsigned Rd 		: 3;
		unsigned opcode : 5;
	}ADDS_sp_imm_t1;
	// ADDS_sp_imm_t2
	struct ADDS_sp_imm_t2
	{
		unsigned imm7 	: 7;
		unsigned opcode : 8;
	}ADDS_sp_imm_t2;
  // ADR_imm8_t1
  struct ADR_imm_t1
  {
    unsigned imm8   : 8;
    unsigned Rd     : 3;
    unsigned opcode : 5;
  }ADR_imm_t1;
	// ADDS_reg
	struct ADDS_regs_t1
	{
		unsigned Rd		 	: 3;
		unsigned Rn 		: 3;
		unsigned Rm			: 3;
		unsigned opcode : 7;
	}ADDS_regs_t1;
  // ADD_reg_T2
  struct ADD_regs_t2
  {
    unsigned Rdn    : 3;
    unsigned Rm     : 4;
    unsigned DN     : 1;
    unsigned opcode : 8;
  }ADD_regs_t2;
	// ADCS_reg
  struct ADCS_regs_t1
  {
    unsigned Rdn    : 3;
    unsigned Rm     : 3;
    unsigned opcode : 10;
  }ADCS_regs_t1;

	// SUB_immediate_t1
	struct SUB_immediate_t1
	{
		unsigned Rd			: 3;
		unsigned Rn			: 3;
		unsigned imm3		: 3;
		unsigned opcode : 7;
	}SUBS_t1;
  // RSB_immediate_t1
  struct RSB_immediate_t1
  {
    unsigned Rd     : 3;
    unsigned Rn     : 3;
    unsigned opcode : 10;
  }RSBS_t1;
	// SUB_immediate_t2
	struct SUB_immediate
	{
		unsigned imm8 	: 8;
		unsigned Rdn 		: 3;
		unsigned opcode : 5;
	}SUBS_t2;
	// SUB_sp_immediate_t2
	struct SUB_SP_imm_t1
	{
		unsigned imm7 	: 7;
		unsigned opcode : 9;
	}SUB_SP_imm_t1;
	//SUB_regs_t1
	struct SUB_regs_t1
	{
		unsigned Rd			: 3;
		unsigned Rn			: 3;
		unsigned Rm			: 3;
		unsigned opcode : 7;
	}SUB_regs_t1;
  //SBC_regs_t1
  struct SBC_regs_t1
  {
    unsigned Rdn    : 3;
    unsigned Rm     : 3;
    unsigned opcode : 10;
  }SBC_regs_t1;
	// ORRS
	struct ORRS_t1
	{
		unsigned Rdn 		: 3;
		unsigned Rm 		: 3;
		unsigned opcode : 10;
	}ORRS_t1;
	// EORS
	struct EORS_reg_t1
	{
		unsigned Rn 		: 3;
		unsigned Rm 		: 3;
		unsigned opcode : 10;
	}EORS_t1;
	// ANDS
	struct ANDS_t1
	{
		unsigned Rdn 		: 3;
		unsigned Rm 		: 3;
		unsigned opcode : 10;
	}ANDS_t1;
  // TST
  struct TST_register_t1
  {
    unsigned Rn     : 3;
    unsigned Rm     : 3;
    unsigned opcode : 10;
  }TST_register_t1;
	// STR immediate T1
	struct STR_immediate_t1
	{
		unsigned Rt 		: 3;
		unsigned Rn 		: 3;
		unsigned imm5		: 5;
		unsigned opcode : 5;
	}STR_immediate_t1;
  // STRH immediate T1
  struct STRH_immediate_t1
  {
    unsigned Rt     : 3;
    unsigned Rn     : 3;
    unsigned imm5   : 5;
    unsigned opcode : 5;
  }STRH_immediate_t1;
	// STR immediate T2
	struct STR_immediate_t2
	{
		unsigned imm8		: 8;
		unsigned Rt 		: 3;
		unsigned opcode : 5;
	}STR_immediate_t2;
	// STR T1
	struct STR_t1
	{
		unsigned Rt 		: 3;
		unsigned Rn 		: 3;
		unsigned Rm 		: 3;
		unsigned opcode : 7;
	}STR_t1;
  // STRH_T1
  struct STRH_t1
  {
    unsigned Rt     : 3;
    unsigned Rn     : 3;
    unsigned Rm     : 3;
    unsigned opcode : 7;
  }STRH_t1;
  // STRB_T1
  struct STRB_t1
  {
    unsigned Rt     : 3;
    unsigned Rn     : 3;
    unsigned Rm     : 3;
    unsigned opcode : 7;
  }STRB_t1;
	// ASRS_immediate_t1
  struct ASRS_immediate_t1
  {
    unsigned Rd     : 3;
    unsigned Rm     : 3;
    unsigned imm5   : 5;
    unsigned opcode : 5;
  }ASRS_immediate_t1;
  // ASRS_register_t1
  struct ASRS_t1
  {
    unsigned Rdn    : 3;
    unsigned Rm     : 3;
    unsigned opcode : 10;
  }ASRS_t1;
	// LSRS_immediate_t1
	struct LSRS_immediate_t1
	{
		unsigned Rd 		: 3;
		unsigned Rm 		: 3;
		unsigned imm5		: 5;
		unsigned opcode : 5;
	}LSRS_immediate_t1;
	// LSRS_register_t1
	struct LSRS_t1
	{
		unsigned Rdn 		: 3;
		unsigned Rm 		: 3;
		unsigned opcode : 10;
	}LSRS_t1;
  // RORS_register_t1
  struct RORS_register_t1
  {
    unsigned Rdn    : 3;
    unsigned Rm     : 3;
    unsigned opcode : 10;
  }RORS_register_t1;
	// LSLS_immediate_t1
	struct LSLS_immediate_t1
	{
		unsigned Rd 		: 3;
		unsigned Rm 		: 3;
		unsigned imm5		: 5;
		unsigned opcode : 5;
	}LSLS_immediate_t1;
	// LSLS_t1
	struct LSLS_reg_t1
	{
		unsigned Rdn 		: 3;
		unsigned Rm 		: 3;
		unsigned opcode : 10;
	}LSLS_reg_t1;
	struct BICS_t1
	{
		unsigned Rdn 		: 3;
		unsigned Rm 		: 3;
		unsigned opcode : 10;
	}BICS_t1;
	// CMP_reg_t1
	struct CMP_t1
	{
		unsigned Rn 		: 3;
		unsigned Rm 		: 3;
		unsigned opcode : 10;
	}CMP_t1;
  // CMP_reg_t2
  struct CMP_reg_t2
  {
    unsigned Rn     : 3;
    unsigned Rm     : 4;
    unsigned DN     : 1;
    unsigned opcode : 8;
  }CMP_reg_t2;
  // CMN_reg_t1
  struct CMN_reg_t1
  {
    unsigned Rn     : 3;
    unsigned Rm     : 3;
    unsigned opcode : 10;
  }CMN_t1;
	// CMP_imm8_t1
	struct CMP_imm_t1
	{
		unsigned imm8		: 8;
		unsigned Rn 		: 3;
		unsigned opcode : 5;
	}CMP_imm_t1;
  // SXTH
  struct SXTH_t1
  {
    unsigned Rd     : 3;
    unsigned Rm     : 3;
    unsigned opcode : 10;
  }SXTH_t1;
  // SXTB
  struct SXTB_t1
  {
    unsigned Rd     : 3;
    unsigned Rm     : 3;
    unsigned opcode : 10;
  }SXTB_t1;
  // UXTH
  struct UXTH_t1
  {
    unsigned Rd     : 3;
    unsigned Rm     : 3;
    unsigned opcode : 10;
  }UXTH_t1;
	// UXTB
	struct UXTB_t1
	{
		unsigned Rd 		: 3;
		unsigned Rm 		: 3;
		unsigned opcode : 10;
	}UXTB_t1;
  // MULS
  struct MULS_t1
  {
    unsigned Rdm    : 3;
    unsigned Rn     : 3;
    unsigned opcode : 10;
  }MULS_t1;
  //CPS T1
  struct CPS_t1
  {
    unsigned BITs   : 4;
    unsigned IM     : 1;
    unsigned opcode : 11;
  }CPS_t1;
  // REV t1
  struct REV_t1
  {
    unsigned Rd     : 3;
    unsigned Rm     : 3;
    unsigned opcode : 10;
  }REV_t1;
  // REV16 t1
  struct REV16_t1
  {
    unsigned Rd     : 3;
    unsigned Rm     : 3;
    unsigned opcode : 10;
  }REV16_t1;
  // REVSH t1
  struct REVSH_t1
  {
    unsigned Rd     : 3;
    unsigned Rm     : 3;
    unsigned opcode : 10;
  }REVSH_t1;
};

static inline uint16_t _stm_flash_read16le( stm_t * stm, stm_flashaddr_t addr)
{
	stm_flashaddr_t normalized_adr = addr - stm->flash_start_adr;
	normalized_adr &= 0xfffffffe;
	return(stm->flash[normalized_adr] | (stm->flash[normalized_adr + 1] << 8));
}

//static inline uint32_t _stm_flash_read32le( stm_t * stm, stm_flashaddr_t addr)
//{
//	stm_flashaddr_t normalized_adr = addr - stm->flash_start_adr;
//	normalized_adr &= 0xfffffffe;
//	return(()stm->flash[normalized_adr]
//			| (stm->flash[normalized_adr + 1] << 8)
//			| (stm->flash[normalized_adr + 2] << 16)
//			| (stm->flash[normalized_adr + 3] << 32));
//}

extern uint32_t stm_addr_read( struct stm_t * stm, uint32_t addr, uint32_t* val );
extern uint32_t stm_addr_readh( struct stm_t * stm, uint32_t addr, uint16_t* val );
extern uint32_t stm_addr_readb( struct stm_t * stm, uint32_t addr, uint8_t* val );
extern uint32_t stm_addr_write( struct stm_t * stm, uint32_t addr, uint32_t val );
extern uint32_t stm_addr_writeh( struct stm_t * stm, uint32_t addr, uint16_t val );
extern uint32_t stm_addr_writeb( struct stm_t * stm, uint32_t addr, uint8_t val );
extern void stm_core_rw_handler_reg( stm_t *stm, stm_circuit_ifs_t* cifs, uint32_t address, uint32_t size  );

extern stm_t * stm_core_fabric( const char *name );

#ifdef __cplusplus
};
#endif

#endif /*__SIM_CORE_H__*/
