#ifndef SIM_CIR_IFS_H_
#define SIM_CIR_IFS_H_

/*! \file      cir_ifs.h
    \version   0.0
    \date      29 dec 2019 y 21:47:38
    \brief     
    \details   
    \bug
    \copyright 
    \author
*/



/* ------------------------------------------------------------------------------------------------
 *                                     Macros
 * ------------------------------------------------------------------------------------------------
 */

/* ------------------------------------------------------------------------------------------------
 *                                 Type definitions
 * ------------------------------------------------------------------------------------------------
 */

struct stm_t;
struct mcu_t;
struct stm_rcc_t;

// call back on update
//typedef void (*cb_onupdate_t)(struct mcu_t * mcu, void* param, uint32_t rcc_evt, uint32_t val);
//
//typedef struct cb_onupdate_cell_t
//{
//	struct cb_onupdate_cell_t* next;
//	cb_onupdate_t cb;
//	void * param;
//} cb_onupdate_cell_t;

// call back on read
typedef void (*cb_reg_read_t)(struct mcu_t * mcu, void* param, uint32_t rcc_evt, uint32_t val);

typedef struct cb_read_cell_t
{
	struct cb_read_cell_t* next;
	cb_reg_read_t cb;
	void * param;
} cb_read_cell_t;

// call back on write
typedef void (*cb_reg_write_t)(struct mcu_t * mcu, void* param, uint32_t rcc_evt, uint32_t val);

typedef struct cb_write_cell_t
{
	struct cb_write_cell_t* next;
	cb_reg_write_t cb;
	void * param;
} cb_write_cell_t;

typedef uint32_t ( *stm_cir_init_t )( struct mcu_t * mcu );
//typedef void ( *stm_cir_reset_t )(struct mcu_t * mcu );
typedef void ( *stm_cir_reset_t )(struct mcu_t * mcu, uint32_t type );
// typedef void ( *stm_cir_release_t )(struct mcu_t * mcu );
typedef void ( *stm_cir_release_t )(struct mcu_t * mcu, void* obj );
typedef void ( *stm_cir_updt_cb_reg_t )(struct mcu_t * mcu, void* cb_updt, uint32_t addr, void* param );
typedef void ( *stm_cir_wr_cb_reg_t )(struct mcu_t * mcu, void* cb_wr, uint32_t addr, void* param );
typedef void ( *stm_cir_rd_cb_reg_t )(struct mcu_t * mcu, void* cb_rd, uint32_t addr, void* param );
typedef void ( *stm_cir_write_t )(struct mcu_t* mcu, uint32_t addr, uint32_t val, void* param );
typedef void ( *stm_cir_read_t )(struct mcu_t* mcu, uint32_t addr, uint32_t* val, void* param );
typedef void ( *stm_dump_cir_state_t )( void* circuit );

/*
 */
typedef struct stm_circuit_ifs_t
{
	stm_cir_init_t 				init;
	stm_cir_reset_t				reset;
	stm_cir_release_t 		release;
	stm_dump_cir_state_t  dump;
	stm_cir_updt_cb_reg_t updt_cb_reg;
	stm_cir_wr_cb_reg_t		wr_cb_reg;
	stm_cir_rd_cb_reg_t		rd_cb_reg;
	stm_cir_write_t				write;
	stm_cir_read_t				read;


	cb_read_cell_t* 			cb_read_pool;
	cb_write_cell_t* 			cb_write_pool;
//	cb_onupdate_cell_t* 	cb_onupdate_pool;
} stm_circuit_ifs_t;


/* ------------------------------------------------------------------------------------------------
 *                                    Functions declarations
 * ------------------------------------------------------------------------------------------------
 */
#ifdef __cplusplus
extern "C"
{
#endif


#ifdef __cplusplus
}
#endif


#endif /* SIM_CIR_IFS_H_ */
