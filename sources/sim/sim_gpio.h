#ifndef SIM_SIM_GPIO_H_
#define SIM_SIM_GPIO_H_

/*! \file      sim_gpio.h
    \version   0.0
    \date      19 july 2020 year 22:58:35
    \brief     
    \details   
    \bug
    \copyright 
    \author ruarka
*/

struct stm_pin_t;
// struct stm_port_t;
struct stm_gpio_t;

/* ------------------------------------------------------------------------------------------------
 *                                     Macros
 * ------------------------------------------------------------------------------------------------
 */
#define PIN_COLLISION_VALUE      0xfffe
#define PIN_FLOATING_VALUE      0xffff
#define PIN_DIGITAL_ONE_VALUE   0xfffd
#define PIN_DIGITAL_ZERO_VALUE  0x0000

#define MODER_ADR               0x00
#define OTYPER_ADR              0x04
#define OSPEEDR_ADR             0x08
#define PUPDR_ADR               0x0c
#define IDR_ADR                 0x10
#define ODR_ADR                 0x14
#define BSRR_ADR                0x18
#define LCKR_ADR                0x1c
#define AFR0_ADR                0x20
#define AFR1_ADR                0x24
#define BRR_ADR                 0x28

#define GPIO_TRACE_ON     1


#ifdef GPIO_TRACE_ON
#ifndef GPIO_TRACE
#define GPIO_TRACE(stm, level, ... ) \
    do { \
      stm_global_logger(stm, level, __VA_ARGS__); \
    } while(0)
#endif
#else
#define GPIO_TRACE(stm, level, ...)
#endif

/* ------------------------------------------------------------------------------------------------
 *                                 Type definitions
 * ------------------------------------------------------------------------------------------------
 */

typedef void ( pfn_pins_custom_layout_names_init )(
    struct mcu_t * mcu,
    struct stm_gpio_t* port,
    char* port_name
    );

/* ............................... Pins Definitions ................................................
 *
 */
/*
 * -------------- New version of pin sim object ------------
 */
typedef enum pin_source_t
{
  SrcUndefined = 0,
  SrcODR,
  SrcAlternate,
  SrcAnalog,
  SrcExternal
} pin_source_t;

typedef enum stm_pin_mode_t
{
  Undefined = 0,
  Unused,
  Input,
  Output,
  Alternate,
  Analog,
  Layout
} stm_pin_mode_t;


typedef enum stm_pin_state_t
{
  stUndefined = 0,
  stUnused,
  stInUse,
  stColision,
  stLayout
} stm_pin_state_t;


typedef enum stm_pin_value_type_t
{
  vtUndefined,
  vtFloating,
  vtDigital,
  vtAnalog,
  vtCollision
} stm_pin_value_type_t;

typedef struct stm_pin_source_t
{
  uint32_t              val;
  stm_pin_value_type_t  val_type;
  uint32_t              is_active;
} stm_pin_source_t;

/**
 * @brief Input port object definition
 * Assigned for external module to set port values
 */

typedef void( pfn_pin_change_state_cb_t )( struct stm_pin_t* pin );

typedef struct sim_al_pin_cb_cell_t
{
  struct sim_al_pin_cb_cell_t* next;
  pfn_pin_change_state_cb_t* sim_pin_cb;
} sim_al_pin_cb_cell_t;

typedef struct sim_an_pin_cb_cell_t
{
  struct sim_an_pin_cb_cell_t* next;
  pfn_pin_change_state_cb_t* sim_pin_cb;
} sim_an_pin_cb_cell_t;


typedef struct ext_pin_cb_cell_t
{
  struct ext_pin_cb_cell_t*   next;
  pfn_pin_change_state_cb_t*  ext_pin_cb;
} ext_pin_cb_cell_t;

typedef struct ctrl_pin_ifs_t
{
//  void( *pfn_init )( struct stm_gpio_t* port, struct stm_pin_t* pin );
  void( *pfn_reset )( struct stm_pin_t* pin, uint32_t reset_type );
  void( *pfn_release )( struct stm_pin_t* pin );
  void( *pfn_set_pin_value )( struct stm_pin_t* pin, uint32_t val, stm_pin_mode_t pins_type );
  void( *pfn_get_pin_state )( struct stm_pin_t* pin, uint32_t* val, stm_pin_mode_t* pins_type );
  void( *pfn_set_pin_cb )( struct stm_pin_t* pin, pfn_pin_change_state_cb_t fn );
  void( *pfn_set_pin_ext_cb )( struct stm_pin_t* pin, pfn_pin_change_state_cb_t fn );
} ctrl_pin_ifs_t;

typedef struct data_pin_ifs_t
{
  void( *pfn_ODR_pin_value )( struct stm_pin_t* pin, uint32_t val, stm_pin_value_type_t val_type );
  void( *pfn_External_pin_value )( struct stm_pin_t* pin, uint32_t val, stm_pin_value_type_t val_type );
  void( *pfn_Alternative_pin_value )( struct stm_pin_t* pin, uint32_t val, stm_pin_value_type_t val_type );
  void( *pfn_Analog_pin_value )( struct stm_pin_t* pin, uint32_t val, stm_pin_value_type_t val_type );
} data_pin_ifs_t;


typedef struct stm_pin_t
{
  uint32_t                val;
  stm_pin_value_type_t    val_type;
  stm_pin_state_t         state;
  stm_pin_mode_t          mode;

  stm_pin_source_t        odr;
  stm_pin_source_t        ext;
  stm_pin_source_t        alternate;
  stm_pin_source_t        analog;

  char                    name[ 5 ];
  uint32_t                pin_num;
  ctrl_pin_ifs_t          ctrl;
  data_pin_ifs_t          data;
  struct stm_gpio_t*      port;
  struct mcu_t*           mcu;
  sim_an_pin_cb_cell_t*   an_cb_pool;
  sim_al_pin_cb_cell_t*   al_cb_pool;
  ext_pin_cb_cell_t*      ext_cb_pool;
} stm_pin_t;

/* ............................... GPIO Definitions ...............................................
 *
 */

/**
 * @brief STM GPIO bit object description
 */

/**
  * @brief General Purpose IO
  */
typedef struct stm_gpio_port_regs
{
  uint32_t MODER;        /*!< GPIO port mode register,                     Address offset: 0x00 */
  uint32_t OTYPER;       /*!< GPIO port output type register,              Address offset: 0x04 */
  uint32_t OSPEEDR;      /*!< GPIO port output speed register,             Address offset: 0x08 */
  uint32_t PUPDR;        /*!< GPIO port pull-up/pull-down register,        Address offset: 0x0C */
  uint32_t IDR;          /*!< GPIO port input data register,               Address offset: 0x10 */
  uint32_t ODR;          /*!< GPIO port output data register,              Address offset: 0x14 */
  uint32_t BSRR;         /*!< GPIO port bit set/reset registerBSRR,        Address offset: 0x18 */
  uint32_t LCKR;         /*!< GPIO port configuration lock register,       Address offset: 0x1C */
  uint32_t AFR[2];       /*!< GPIO alternate function register,            Address offset: 0x20-0x24 */
  uint32_t BRR;          /*!< GPIO bit reset register,                     Address offset: 0x28 */
}stm_gpio_port_regs_t;


/*
 * @brief STM GPIO port object description
 */
typedef struct stm_gpio_t
{
	struct stm_circuit_ifs_t*   cir_cb_ifs;
	struct mcu_t*               mcu;
	stm_gpio_port_regs_t        regs;
	char                        gpioPortName[ 3 ];
	stm_pin_t     	            pins[ 16 ];
	uint64_t                    frequency;			    	// frequency rcc feed with
	uint64_t                    nano_ticks_per_cycle;	// to calculate cycletimer gap
} stm_gpio_t;
/* ------------------------------------------------------------------------------------------------
 *                                    Functions declarations
 * ------------------------------------------------------------------------------------------------
 */
#ifdef __cplusplus
extern "C"
{

#endif

extern void stm_gpio_write( struct mcu_t* mcu, uint32_t addr, uint32_t val, void* param );
extern void stm_gpio_read( struct mcu_t* mcu, uint32_t addr, uint32_t* val, void* param );

/* On RCC update callback function  */
extern void gpio_rcc_onupdate_cb(struct mcu_t * mcu, uint32_t param, uint64_t ticks_per_nsec, uint64_t frequency);

/*  */
extern struct stm_pin_t* get_pin_by_name( struct stm_gpio_t* port, char* name );

/* GPIO fabric function */
stm_gpio_t* gpio_circuit_fabric( struct mcu_t * mcu );


#ifdef __cplusplus
}
#endif


#endif /* SIM_SIM_GPIO_H_ */
