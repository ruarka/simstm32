/*
	stm_flash.h

 */


#ifndef __STM_FLASH_H___
#define __STM_FLASH_H___

#ifdef __cplusplus
extern "C" {
#endif


/**
  * @brief FLASH Registers
  */
typedef struct FLASH_TypeDef
{
  uint32_t ACR;           /*!< Access control register,          Address offset: 0x00 */
  uint32_t PECR;          /*!< Program/erase control register,   Address offset: 0x04 */
  uint32_t PDKEYR;        /*!< Power down key register,          Address offset: 0x08 */
  uint32_t PEKEYR;        /*!< Program/erase key register,       Address offset: 0x0c */
  uint32_t PRGKEYR;       /*!< Program memory key register,      Address offset: 0x10 */
  uint32_t OPTKEYR;       /*!< Option byte key register,         Address offset: 0x14 */
  uint32_t SR;            /*!< Status register,                  Address offset: 0x18 */
  uint32_t OPTR;          /*!< Option byte register,             Address offset: 0x1c */
  uint32_t WRPR1;         /*!< Write protection register 1,      Address offset: 0x20 */
  uint32_t WRPR2;         /*!< Write protection register 2,      Address offset: 0x80 */
} FLASH_TypeDef;



/*
 * Handles self-programming subsystem if the core
 * supports it.
 */
typedef struct stm_flash_t
{
	struct stm_circuit_ifs_t* cir_cb_ifs;

	FLASH_TypeDef* flash_regs;

//	stm_io_t	io;
//	uint16_t	flags;
//	uint16_t	*tmppage;
//	uint8_t	*tmppage_used;
//	uint16_t	spm_pagesize;
//	uint8_t r_spm;
//	stm_regbit_t selfprgen;
//	stm_regbit_t pgers;		// page erase
//	stm_regbit_t pgwrt;		// page write
//	stm_regbit_t blbset;	// lock bit set
//	stm_regbit_t rwwsre;    // read while write section read enable
//	stm_regbit_t rwwsb;		// read while write section busy

//	stm_int_vector_t flash;	// Interrupt vector

	struct mcu_t* mcu;
} stm_flash_t;

stm_flash_t* flash_circuit_fabric( struct mcu_t * mcu );

#ifdef __cplusplus
};
#endif

#endif /* __STM_FLASH_H___ */
