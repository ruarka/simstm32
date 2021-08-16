/*
 * stm_rcc.h
 *
 *  Created on: 25 november 2019 ã.
 *  Author: ruarka
 *
 *  Reset and Clock Control (RCC) module
 */

#ifndef SIM_STM_RCC_H_
#define SIM_STM_RCC_H_

#include "core_cm0plus.h"

#ifdef __cplusplus
extern "C"
{
#endif

/* ------------------------------------------------------------------------------------------------
 *                                     Macros
 * ------------------------------------------------------------------------------------------------
 */

#define RESET_UNDEF					0x00
#define RESET_SYSTEM_NSRT 	0x01
#define RESET_SYSTEM_WWDG 	0x02
#define RESET_SYSTEM_IWDG 	0x03
#define RESET_SYSTEM_SW 		0x03
#define RESET_SYSTEM_LPM 		0x04
#define RESET_SYSTEM_OBL 		0x05
#define RESET_SYSTEM_ESB 		0x06
#define RESET_SYSTEM_FW 		0x07
#define RESET_POWER_PWR 		0x08
#define RESET_POWER_BOR 		0x09
#define RESET_RTC 					0x0a
#define RESET_BACKUP_REGS 	0x0b

/**
  \ingroup    RCC_core_register
  \defgroup   STM_CM0P_core_base     Core Definitions
  \brief      Definitions for base addresses, unions, and structures.
  @{
 */
#define CLOCK_SRC_UNDEF_ID	0x00	/*!< Undefined clock source for common terms */
#define CLOCK_SRC_HSI16_ID	0x01  /*!< HSI16 clock source ID */
#define CLOCK_SRC_HSE_ID   	0x02  /*!< HSE   clock source ID */
#define CLOCK_SRC_PLL_ID		0x03	/*!< PLL   clock source ID */
#define CLOCK_SRC_MSI_ID		0x04	/*!< MSI   clock source ID */
#define CLOCK_SRC_LSI_ID		0x05	/*!< LSI   clock source ID */
#define CLOCK_SRC_LSE_ID		0x06	/*!< LsE   clock source ID */


#define SYSCLK_UPDT					0x01	/*!< Update SYSCLK     values command */
#define WatchdogLS_UPDT			0x02	/*!< Update WatchdogLS values command */
#define RTC_UPDT						0x03	/*!< Update RTC        values command */
#define ADCCLK_UPDT					0x04	/*!< Update ADCCLK     values command */
#define MCO_UPDT						0x05	/*!< Update MCO        values command */
#define CK_PWR_UPDT					0x06	/*!< Update CK_PWR     values command */
#define FCLK_UPDT						0x07	/*!< Update FCLK       values command */
#define HCLK_UPDT						0x08	/*!< Update HCLK       values command */
#define TIMxCLK_UPDT				0x09	/*!< Update TIMxCLK    values command */
#define PCLK1_UPDT					0x0a	/*!< Update PCLK1      values command */
#define APB1_TIMx_UPDT			0x0b	/*!< Update APB1_TIMx  values command */
#define APB2_TIMx_UPDT			0x0c	/*!< Update APB2_TIMx  values command */
#define PCLK2_UPDT					0x0d	/*!< Update PCLK2      values command */
#define LPTIMCLK_UPDT				0x0e	/*!< Update LPTIMCLK   values command */
#define UARTCLK_UPDT				0x0f	/*!< Update UARTCLK    values command */
#define I2C1CLK_UPDT				0x10	/*!< Update I2C1CLK    values command */

/*@} */

/**
 *
 */
#define NANO_UNDEF					0x00
#define NANO_SYSCLK_ASSIGN	0x01

/* ------------------------------------------------------------------------------------------------
 *                                 Type definitions
 * ------------------------------------------------------------------------------------------------
 */
typedef struct
{
  __IO uint32_t CR;            /*!< RCC clock control register,                                   Address offset: 0x00 */
  __IO uint32_t ICSCR;         /*!< RCC Internal clock sources calibration register,              Address offset: 0x04 */
  __IO uint32_t CRRCR;         /*!< RCC Clock recovery RC register,                               Address offset: 0x08 */
  __IO uint32_t CFGR;          /*!< RCC Clock configuration register,                             Address offset: 0x0C */
  __IO uint32_t CIER;          /*!< RCC Clock interrupt enable register,                          Address offset: 0x10 */
  __IO uint32_t CIFR;          /*!< RCC Clock interrupt flag register,                            Address offset: 0x14 */
  __IO uint32_t CICR;          /*!< RCC Clock interrupt clear register,                           Address offset: 0x18 */
  __IO uint32_t IOPRSTR;       /*!< RCC IO port reset register,                                   Address offset: 0x1C */
  __IO uint32_t AHBRSTR;       /*!< RCC AHB peripheral reset register,                            Address offset: 0x20 */
  __IO uint32_t APB2RSTR;      /*!< RCC APB2 peripheral reset register,                           Address offset: 0x24 */
  __IO uint32_t APB1RSTR;      /*!< RCC APB1 peripheral reset register,                           Address offset: 0x28 */
  __IO uint32_t IOPENR;        /*!< RCC Clock IO port enable register,                            Address offset: 0x2C */
  __IO uint32_t AHBENR;        /*!< RCC AHB peripheral clock enable register,                     Address offset: 0x30 */
  __IO uint32_t APB2ENR;       /*!< RCC APB2 peripheral enable register,                          Address offset: 0x34 */
  __IO uint32_t APB1ENR;       /*!< RCC APB1 peripheral enable register,                          Address offset: 0x38 */
  __IO uint32_t IOPSMENR;      /*!< RCC IO port clock enable in sleep mode register,              Address offset: 0x3C */
  __IO uint32_t AHBSMENR;      /*!< RCC AHB peripheral clock enable in sleep mode register,       Address offset: 0x40 */
  __IO uint32_t APB2SMENR;     /*!< RCC APB2 peripheral clock enable in sleep mode register,      Address offset: 0x44 */
  __IO uint32_t APB1SMENR;     /*!< RCC APB1 peripheral clock enable in sleep mode register,      Address offset: 0x48 */
  __IO uint32_t CCIPR;         /*!< RCC clock configuration register,                             Address offset: 0x4C */
  __IO uint32_t CSR;           /*!< RCC Control/status register,                                  Address offset: 0x50 */
} RCC_TypeDef;


/**
 * call back updates on circuit state changes
 */
typedef void (*cb_rcc_onupdate_t)(struct mcu_t * stm, uint32_t param, uint64_t ticks_per_nsec, uint64_t frequency);

typedef struct cb_onupdate_cell_t
{
	struct cb_onupdate_cell_t* next;
	cb_rcc_onupdate_t cb;
	void * param;
} cb_onupdate_cell_t;


// RCC registers
enum {
	//
	RCC_CR_idx = 0,
	RCC_ICSCR_idx,
	RCC_CRRCR_idx,
	RCC_CFGR_idx,
	RCC_CIER_idx,
	RCC_CIFR_idx,
	RCC_CICR_idx,
	RCC_IOPRSTR_idx,
	RCC_AHBRSTR_idx,
	RCC_APB2RSTR_idx,
	RCC_APB1RSTR_idx,
	RCC_IOPENR_idx,
	RCC_AHBENR_idx,
	RCC_APB2ENR_idx,
	RCC_APB1ENR_idx,
	RCC_IOPSMENR_idx,
	RCC_AHBSMENR_idx,
	RCC_APB2SMENR_idx,
	RCC_APB1SMENR_idx,
	RCC_CCIPR_idx,
	RCC_CSR_idx,
	RCC_Register_Cnt
};

enum {
	//
	RCC_CR				= 0x00,
	RCC_ICSCR 		= 0x04,
	RCC_CRRCR			= 0x08,
	RCC_CFGR 			= 0x0C,
	RCC_CIER 			= 0x10,
	RCC_CIFR 			= 0x14,
	RCC_CICR 			= 0x18,
	RCC_IOPRSTR 	= 0x1C,
	RCC_AHBRSTR 	= 0x20,
	RCC_APB2RSTR 	= 0x24,
	RCC_APB1RSTR 	= 0x28,
	RCC_IOPENR 		= 0x2C,
	RCC_AHBENR 		= 0x30,
	RCC_APB2ENR 	= 0x34,
	RCC_APB1ENR		= 0x38,
	RCC_IOPSMENR 	= 0x3C,
	RCC_AHBSMENR 	= 0x40,
	RCC_APB2SMENR = 0x44,
	RCC_APB1SMENR = 0x48,
	RCC_CCIPR 		= 0x4C,
	RCC_CSR 			= 0x50,
};

/*
 *
 */
typedef struct stm_crystal_oscilator_t
{
	uint64_t		frequency;
	uint64_t    nano_per_tick;
	char* 			pretty_name;
}stm_crystal_oscilator_t, *stm_crystal_oscilator_p;

/*
 *
 */
typedef struct stm_clock_t
{
	uint64_t		frequency;
	uint64_t    nano_per_tick;
	uint32_t    enable;
	char* 			preaty_name;
} stm_clock_t, *stm_clock_p;

/*
 \brief
 \details
 */
typedef struct stm_rcc_t
{
	struct stm_t* stm;
	struct mcu_t* mcu;

	struct stm_circuit_ifs_t* cir_cb_ifs;

	struct
	{
		uint32_t val;

		struct
		{
			cb_read_cell_t* cb_read_pool;
			cb_write_cell_t* cb_write_pool;
	//		cb_onupdate_cell_t* cb_onupdate_pool;
		} cbs;
	}rcc_regs[sizeof( RCC_TypeDef )/sizeof(uint32_t)];

	cb_onupdate_cell_t* rcc_update_pool;

	// Frequency
	uint64_t freq_val;

	struct stm_crystal_oscilator_t lsi;
	struct stm_crystal_oscilator_t lse;
	struct stm_crystal_oscilator_t msi;
	struct stm_crystal_oscilator_t hsi16;
	struct stm_crystal_oscilator_t hse;

	// Clock sources
	struct stm_clock_t 	SYSCLK;
	struct stm_clock_t  WatchdogLS;
	struct stm_clock_t  RTC;
	struct stm_clock_t  ADCCLK;
	struct stm_clock_t  MCO;
	struct stm_clock_t  CK_PWR;
	struct stm_clock_t	FCLK;
	struct stm_clock_t	HCLK;
	struct stm_clock_t	TIMxCLK;
	struct stm_clock_t	PCLK1;
	struct stm_clock_t	APB1_TIMx;
	struct stm_clock_t	APB2_TIMx;
	struct stm_clock_t	PCLK2;
	struct stm_clock_t	LPTIMCLK;
	struct stm_clock_t	UARTCLK;
	struct stm_clock_t	I2C1CLK;

} stm_rcc_t, *stm_rcc_t_p;

// class fabric for RCC  registers
stm_rcc_t* rcc_circuit_fabric( struct mcu_t * mcu );

// void stm_rcc_updt_cb_reg( struct mcu_t * mcu, void* cb_updt, void* param );
void stm_rcc_updt_cb_reg( struct mcu_t * mcu, cb_rcc_onupdate_t cb_updt, void* param );

#ifdef __cplusplus
};
#endif

#endif /* SIM_STM_RCC_H_ */
