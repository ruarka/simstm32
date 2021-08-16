/*
	sim_io.h
 */

#ifndef __SIM_IO_H__
#define __SIM_IO_H__

//#include "sim_stm.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * used by the ioports to implement their own features
 * see avr_eeprom.* for an example, and stm_ioctl().
 */
#define STM_IOCTL_DEF(_a,_b,_c,_d) \
	(((_a) << 24)|((_b) << 16)|((_c) << 8)|((_d)))

/*
 * IO module base struct
 * Modules uses that as their first member in their own struct
 */
typedef struct stm_io_t
{
	struct stm_io_t * 	next;
	struct stm_t *			stm;		// avr we are attached to
	const char * 				kind;		// pretty name, for debug

	const char ** 			irq_names;	// IRQ names

	uint32_t						irq_ioctl_get;	// used to get irqs from this module
	int									irq_count;	// number of (optional) irqs
	struct stm_irq_t *	irq;		// optional external IRQs

	// called at reset time
	void (*reset)(struct stm_io_t *io);

	// called externally. allow access to io modules and so on
	int (*ioctl)(struct stm_io_t *io, uint32_t ctl, void *io_param);

	// optional, a function to free up allocated system resources
	void (*dealloc)(struct stm_io_t *io);
} stm_io_t;

/*
 * IO modules helper functions
 */

// registers an IO module, so it's run(), reset() etc are called
// this is called by the AVR core init functions, you /could/ register an external
// one after instantiation, for whatever purpose...
//void stm_register_io( struct stm_t *stm, stm_io_t * io);

// Sets an IO module "official" IRQs and the ioctl used to get to them. if 'irqs' is NULL,
// 'count' will be allocated
stm_irq_t * stm_io_setirqs( stm_io_t * io, uint32_t ctl, int count, stm_irq_t * irqs );

// register a callback for when IO register "addr" is read
//void stm_register_io_read( stm_t *stm, stm_io_addr_t addr, stm_io_read_t read, void * param);

// register a callback for when the IO register is written. callback has to set the memory itself
//void stm_register_io_write( stm_t *stm, stm_io_addr_t addr, stm_io_write_t write, void * param);

// call every IO modules until one responds to this
int stm_ioctl( struct stm_t *stm, uint32_t ctl, void * io_param);

// get the specific irq for a module, check stm_ioctl_IOPORT_GETIRQ for example
struct stm_irq_t * stm_io_getirq( struct stm_t * stm, uint32_t ctl, int index);

// get the IRQ for an absolute IO address
// this allows any code to hook an IRQ in any io address, for example
// tracing changes of values into a register
// Note that the values do not "magically" change, they change only
// when the AVR code attempt to read and write at that address
//
// the "index" is a bit number, or ALL bits if index == 8
#define STM_IOMEM_IRQ_ALL 8
stm_irq_t * stm_iomem_getirq( struct stm_t * stm, stm_io_addr_t addr, const char * name, int index);

// Terminates all IOs and remove from them from the io chain
void stm_deallocate_ios( struct stm_t *stm);

#ifdef __cplusplus
};
#endif

#endif /* __SIM_IO_H__ */
