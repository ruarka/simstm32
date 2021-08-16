#ifndef SIM_SIM_NVIC_H_
#define SIM_SIM_NVIC_H_

/*! \file      sim_nvic.h
    \version   0.0
    \date      8 July 2020 22:40:00
    \brief     
    \details   
    \bug
    \copyright 
    \author ruarka
*/



/* ------------------------------------------------------------------------------------------------
 *                                     Macros
 * ------------------------------------------------------------------------------------------------
 */
#define SysTick_Pending_Ext				0


/* ------------------------------------------------------------------------------------------------
 *                                 Type definitions
 * ------------------------------------------------------------------------------------------------
 */
/**
 * NVIC vectors definition
 */
//typedef enum
//{
//	Reset = -3,								/*!< Handler: 0x0000_0004  */
//	NMI_Handler = -2,					/*!< Handler: 0x0000_0008  */
//	HardFault_Handler = -1,		/*!< Handler: 0x0000_000C  */
//	SVC_Handler = 3,					/*!< Handler: 0x0000_002C  */
//	PendSV_Handler = 5, 			/*!< Handler: 0x0000_0038  */
//	SysTick_Handler = 6, 			/*!< Handler: 0x0000_003C  */
//	WWDG = 7,									/*!< Handler: 0x0000_0040  */
//	PVD = 8,									/*!< Handler: 0x0000_0044  */
//	RTC = 9,									/*!< Handler: 0x0000_0048  */
//	FLASH = 10,								/*!< Handler: 0x0000_004C  */
//	RCC_CRS= 11,							/*!< Handler: 0x0000_0050  */
//	EXTI_1_0 = 12,						/*!< Handler: 0x0000_0054  */
//	EXTI_3_2 = 13,						/*!< Handler: 0x0000_0058  */
//	EXTI_15_4 = 14,						/*!< Handler: 0x0000_005C  */
//	DMA1_Channel1 = 16,				/*!< Handler: 0x0000_0064  */
//	DMA1_Channel_3_2 = 17,		/*!< Handler: 0x0000_0068  */
//	DMA1_Channel_7_4 = 18,		/*!< Handler: 0x0000_006C  */
//	ADC_COMP = 19,						/*!< Handler: 0x0000_0070  */
//	LPTIM1 = 20,							/*!< Handler: 0x0000_0074  */
//	USART_4_5 = 21, 					/*!< Handler: 0x0000_0078  */
//	TIM2 = 22, 								/*!< Handler: 0x0000_007C  */
//	TIM3 = 23, 								/*!< Handler: 0x0000_0080  */
//	TIM6 = 24, 								/*!< Handler: 0x0000_0084  */
//	TIM7 = 25, 								/*!< Handler: 0x0000_0088  */
//	TIM21 = 27, 							/*!< Handler: 0x0000_0090  */
//	I2C3 = 28, 								/*!< Handler: 0x0000_0094  */
//	TIM22 = 29, 							/*!< Handler: 0x0000_0098  */
//	I2C1 = 30, 								/*!< Handler: 0x0000_009C  */
//	I2C2 = 31, 								/*!< Handler: 0x0000_00A0  */
//	SPI1 = 32, 								/*!< Handler: 0x0000_00A4  */
//	SPI2 = 33, 								/*!< Handler: 0x0000_00A8  */
//	USART1 = 34, 							/*!< Handler: 0x0000_00AC  */
//	USART2 = 35, 							/*!< Handler: 0x0000_00B0  */
//	LPUART1_AES = 36,  				/*!< Handler: 0x0000_00B4  */
//	count = 37+3
//} stm_nvic_intvect_t;

/*!< Interrupt Number Definition */
typedef enum
{
/******  Cortex-M0 Processor Exceptions Numbers ******************************************************/
  NonMaskableInt_IRQN         = -14,    /*!< 2 Non Maskable Interrupt                                */
  HardFault_IRQN              = -13,    /*!< 3 Cortex-M0+ Hard Fault Interrupt                       */
  SVC_IRQN                    = -5,     /*!< 11 Cortex-M0+ SV Call Interrupt                         */
  PendSV_IRQN                 = -2,     /*!< 14 Cortex-M0+ Pend SV Interrupt                         */
  SysTick_IRQN                = -1,     /*!< 15 Cortex-M0+ System Tick Interrupt                     */

/******  STM32L-0 specific Interrupt Numbers *********************************************************/
  WWDG_IRQN                   = 0,      /*!< Window WatchDog Interrupt                               */
  PVD_IRQN                    = 1,      /*!< PVD through EXTI Line detect Interrupt                  */
  RTC_IRQN                    = 2,      /*!< RTC through EXTI Line Interrupt                         */
  FLASH_IRQN                  = 3,      /*!< FLASH Interrupt                                         */
  RCC_IRQN                    = 4,      /*!< RCC Interrupt                                           */
  EXTI0_1_IRQN                = 5,      /*!< EXTI Line 0 and 1 Interrupts                            */
  EXTI2_3_IRQN                = 6,      /*!< EXTI Line 2 and 3 Interrupts                            */
  EXTI4_15_IRQN               = 7,      /*!< EXTI Line 4 to 15 Interrupts                            */
  DMA1_Channel1_IRQN          = 9,      /*!< DMA1 Channel 1 Interrupt                                */
  DMA1_Channel2_3_IRQN        = 10,     /*!< DMA1 Channel 2 and Channel 3 Interrupts                 */
  DMA1_Channel4_5_6_7_IRQN    = 11,     /*!< DMA1 Channel 4, Channel 5, Channel 6 and Channel 7 Interrupts */
  ADC1_COMP_IRQN              = 12,     /*!< ADC1, COMP1 and COMP2 Interrupts                        */
  LPTIM1_IRQN                 = 13,     /*!< LPTIM1 Interrupt                                        */
  TIM2_IRQN                   = 15,     /*!< TIM2 Interrupt                                          */
  TIM21_IRQN                  = 20,     /*!< TIM21 Interrupt                                         */
  TIM22_IRQN                  = 22,     /*!< TIM22 Interrupt                                         */
  I2C1_IRQN                   = 23,     /*!< I2C1 Interrupt                                          */
  SPI1_IRQN                   = 25,     /*!< SPI1 Interrupt                                          */
  USART2_IRQN                 = 28,     /*!< USART2 Interrupt                                        */
  LPUART1_IRQN                = 29,     /*!< LPUART1 Interrupt                                       */
} irq_number_t;


/*
 *
 */
//typedef enum
//{
//	None = 0,
//	Pending = 1
//} stm_nvic_int_state_t;

/*
 * System NVIC
 */
typedef struct stm_nvic_t
{
	struct stm_circuit_ifs_t* cir_cb_ifs;

	struct mcu_t* mcu;

	uint64_t frequency;
	uint64_t nano_ticks_per_cycle;

	NVIC_Type 			regs;

	uint32_t				pending_interrupts;
	uint32_t				pending_interrupts_ext;

	uint32_t        interrupr_prio;

	uint32_t        iret_val;

} stm_nvic_t;

/* ------------------------------------------------------------------------------------------------
 *                                    Functions declarations
 * ------------------------------------------------------------------------------------------------
 */
#ifdef __cplusplus
extern "C"
{
#endif

extern stm_nvic_t* nvic_circuit_fabric( struct mcu_t * mcu );

extern void nvic_regist_interrupt( struct mcu_t * mcu, irq_number_t interrupt );

extern uint32_t nvic_handle_interrupts( struct mcu_t * mcu );

extern void nvic_handle_pending_interrupts( struct mcu_t * mcu );

extern void nvic_run_interrupt_leaving( struct mcu_t * mcu, uint32_t leaving_code );

#ifdef __cplusplus
}
#endif


#endif /* SIM_SIM_NVIC_H_ */
