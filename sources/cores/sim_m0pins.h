/*
 * sim_m0pins.h
 *
 *  Created on: 18 nov 2019
 *      Author: dell
 */

#ifndef CORES_SIM_M0PINS_H_
#define CORES_SIM_M0PINS_H_

extern stm_flashaddr_t adr_T1(struct stm_t * stm, uint16_t opcode );            // ADR <Rd>,<label>
extern stm_flashaddr_t blx(struct stm_t * stm, uint16_t opcode );
extern stm_flashaddr_t bx_t1(struct stm_t * stm, uint16_t opcode );							// BX_t1
//extern stm_flashaddr_t bl_t1(struct stm_t * stm, uint16_t opcode );						// BL_t1
extern stm_flashaddr_t fb01_block(struct stm_t * stm, uint16_t opcode );				// f01 block
extern stm_flashaddr_t b_t2(struct stm_t * stm, uint16_t opcode ); 							// B_t2
extern stm_flashaddr_t b_t1(struct stm_t * stm, uint16_t opcode );							// B_t1
extern stm_flashaddr_t udf_t1( struct stm_t * stm, uint16_t opcode );           // UDF t1
extern stm_flashaddr_t udf_t2( struct stm_t * stm, uint16_t opcode );           // UDF t2
extern stm_flashaddr_t svc_t1( struct stm_t * stm, uint16_t opcode );           // SVC t1
extern stm_flashaddr_t ldr_literal(struct stm_t * stm, uint16_t opcode );
extern stm_flashaddr_t ldr_immediate(struct stm_t * stm, uint16_t opcode );
extern stm_flashaddr_t ldr_sp_imm_t2(struct stm_t * stm, uint16_t opcode );			// LDR_sp_imm_t2
extern stm_flashaddr_t ldrb_immediate(struct stm_t * stm, uint16_t opcode );		// LDRB_t1
extern stm_flashaddr_t ldr_reg_t1(struct stm_t * stm, uint16_t opcode );				// LDR_reg_t1
extern stm_flashaddr_t ldrsb_reg_t1(struct stm_t * stm, uint16_t opcode );      //
extern stm_flashaddr_t ldrb_reg_t1(struct stm_t * stm, uint16_t opcode );				// LDRB_reg_t1
extern stm_flashaddr_t ldrh_reg_t1(struct stm_t * stm, uint16_t opcode );
extern stm_flashaddr_t ldrsh_reg_t1(struct stm_t * stm, uint16_t opcode );
extern stm_flashaddr_t ldrh_immediate_T1(struct stm_t * stm, uint16_t opcode ); //
extern stm_flashaddr_t ldm_t1(struct stm_t * stm, uint16_t opcode );						// LDM_t1
extern stm_flashaddr_t pop_t1(struct stm_t * stm, uint16_t opcode );						// POP_t1
extern stm_flashaddr_t stm_t1(struct stm_t * stm, uint16_t opcode );						// STM_t1
extern stm_flashaddr_t push_t1(struct stm_t * stm, uint16_t opcode );						// PUSH_t1
extern stm_flashaddr_t mov_immediate(struct stm_t * stm, uint16_t opcode ); 		// MOVS
extern stm_flashaddr_t mov_register_t1(struct stm_t * stm, uint16_t opcode );		// MOV_t1
extern stm_flashaddr_t add_immediate_T1(struct stm_t * stm, uint16_t opcode );	// adds_t1
extern stm_flashaddr_t add_immediate_T2(struct stm_t * stm, uint16_t opcode ); 	// adds_t2
extern stm_flashaddr_t add_sp_immediate_T1(struct stm_t * stm, uint16_t opcode ); // add_sp_t1
extern stm_flashaddr_t add_sp_immediate_T2(struct stm_t * stm, uint16_t opcode ); // ass_sp_t2
extern stm_flashaddr_t add_regs_T1(struct stm_t * stm, uint16_t opcode );				// adds_reg_t1
extern stm_flashaddr_t add_register_T2(struct stm_t * stm, uint16_t opcode );   // add_register_T2
extern stm_flashaddr_t adc_regs_T1(struct stm_t * stm, uint16_t opcode );       // ADC (register) Encoding T1
extern stm_flashaddr_t sub_immediate_T1(struct stm_t * stm, uint16_t opcode );	// subs_imm_t1
extern stm_flashaddr_t rsbs_immediate_T1(struct stm_t * stm, uint16_t opcode ); // rsbs_imm_t1
extern stm_flashaddr_t sub_immediate_T2(struct stm_t * stm, uint16_t opcode );	// subs_imm_t2
extern stm_flashaddr_t sub_sp_immediate_T1(struct stm_t * stm, uint16_t opcode ); // sub_sp_imm_t1
extern stm_flashaddr_t sub_regs_T1(struct stm_t * stm, uint16_t opcode );				// subs_regs_t1
extern stm_flashaddr_t sbc_regs_T1(struct stm_t * stm, uint16_t opcode );       // SBC (register) Encoding T1
extern stm_flashaddr_t orr_register_T1(struct stm_t * stm, uint16_t opcode );  	// orrs_t1
extern stm_flashaddr_t mvns_register_T1(struct stm_t * stm, uint16_t opcode );   // mvns_reg_t1
extern stm_flashaddr_t eors_register_T1(struct stm_t * stm, uint16_t opcode );  // eors_t1
extern stm_flashaddr_t ands_register_T1(struct stm_t * stm, uint16_t opcode );  //
extern stm_flashaddr_t tst_register_T1(struct stm_t * stm, uint16_t opcode );   // TST (register) Encoding T1
extern stm_flashaddr_t str_T1(struct stm_t * stm, uint16_t opcode );            //
extern stm_flashaddr_t strh_T1(struct stm_t * stm, uint16_t opcode );           //
extern stm_flashaddr_t strb_T1(struct stm_t * stm, uint16_t opcode );           //
extern stm_flashaddr_t str_immediate_T1(struct stm_t * stm, uint16_t opcode ); 	// str_imm_t1
extern stm_flashaddr_t str_sp_immediate_T2(struct stm_t * stm, uint16_t opcode );	//str_sp_imm_t2
extern stm_flashaddr_t strb_immediate_T1(struct stm_t * stm, uint16_t opcode );	//
extern stm_flashaddr_t strh_immediate_T1(struct stm_t * stm, uint16_t opcode ); //
extern stm_flashaddr_t asr_immediate_T1(struct stm_t * stm, uint16_t opcode );   // ASR (immediate) Encoding T1
extern stm_flashaddr_t asr_register_T1(struct stm_t * stm, uint16_t opcode );    // ASR (register) Encoding T1
extern stm_flashaddr_t lsr_immediate_T1(struct stm_t * stm, uint16_t opcode );  // lsr_t1
extern stm_flashaddr_t lsr_T1(struct stm_t * stm, uint16_t opcode );
extern stm_flashaddr_t ror_register_T1(struct stm_t * stm, uint16_t opcode );   // RORS (register) Encoding T1
extern stm_flashaddr_t lsl_T1(struct stm_t * stm, uint16_t opcode );
extern stm_flashaddr_t lsl_immediate_T1(struct stm_t * stm, uint16_t opcode );  // lsl_t1
extern stm_flashaddr_t bics_register_T1(struct stm_t * stm, uint16_t opcode );  // bics_t1
extern stm_flashaddr_t cmp_register_T1(struct stm_t * stm, uint16_t opcode );		// cmp_reg_t1
extern stm_flashaddr_t cmp_register_T2(struct stm_t * stm, uint16_t opcode );   // cmp_reg_t2
extern stm_flashaddr_t cmn_register_T1(struct stm_t * stm, uint16_t opcode );   // cmn_reg_t1
extern stm_flashaddr_t cmp_immediate_T1(struct stm_t * stm, uint16_t opcode );  // cmp_imm_t1
extern stm_flashaddr_t nop_T1(struct stm_t * stm, uint16_t opcode );            // nop_t1
extern stm_flashaddr_t uxtb_T1(struct stm_t * stm, uint16_t opcode );           // uxbt_t1
extern stm_flashaddr_t muls_register_T1(struct stm_t * stm, uint16_t opcode );  // muls_reg_t1
extern stm_flashaddr_t sxth_T1(struct stm_t * stm, uint16_t opcode );
extern stm_flashaddr_t uxth_T1(struct stm_t * stm, uint16_t opcode );
extern stm_flashaddr_t sxtb_T1(struct stm_t * stm, uint16_t opcode );
extern stm_flashaddr_t cps_T1(struct stm_t * stm, uint16_t opcode );
extern stm_flashaddr_t rev_T1(struct stm_t * stm, uint16_t opcode );
extern stm_flashaddr_t rev16_T1(struct stm_t * stm, uint16_t opcode );
extern stm_flashaddr_t revsh_T1(struct stm_t * stm, uint16_t opcode );
extern stm_flashaddr_t bkpt_T1(struct stm_t * stm, uint16_t opcode );
extern stm_flashaddr_t yield_T1(struct stm_t * stm, uint16_t opcode );
extern stm_flashaddr_t wfe_T1(struct stm_t * stm, uint16_t opcode );
extern stm_flashaddr_t wfi_T1(struct stm_t * stm, uint16_t opcode );
extern stm_flashaddr_t sev_T1(struct stm_t * stm, uint16_t opcode );
extern stm_flashaddr_t unpredictable(struct stm_t * stm, uint16_t opcode );     // unpredictable

extern uint32_t interrupt_context_push(struct stm_t * stm );
extern uint32_t interrupt_set_handler_adr(struct stm_t * stm, stm_flashaddr_t interrupt_handler_adr );
extern uint32_t interrupt_context_pop(struct stm_t * stm );

#endif /* CORES_SIM_M0PINS_H_ */
