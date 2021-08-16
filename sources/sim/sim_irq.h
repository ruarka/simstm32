/*
 sim_irq.h

 Copyright 2008, 2009 Michel Pollet <buserror@gmail.com>

 This file is part of simavr.

 simavr is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 simavr is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with simavr.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __SIM_IRQ_H__
#define __SIM_IRQ_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

/*
 * Internal IRQ system
 *
 * This subsystem allows any piece of code to "register" a hook to be called when an IRQ is
 * raised. The IRQ definition is up to the module defining it, for example a IOPORT pin change
 * might be an IRQ in which case any piece of code can be notified when a pin has changed state
 *
 * The notify hooks are chained, and duplicates are filtered out so you can't register a
 * notify hook twice on one particular IRQ
 *
 * IRQ calling order is not defined, so don't rely on it.
 *
 * IRQ hook needs to be registered in reset() handlers, ie after all modules init() bits
 * have been called, to prevent race condition of the initialization order.
 */
struct stm_irq_t;

typedef void (*stm_irq_notify_t)(struct stm_irq_t * irq, uint32_t value,
		void * param);

enum
{
	IRQ_FLAG_NOT 				= (1 << 0),	//!< change polarity of the IRQ
	IRQ_FLAG_FILTERED 	= (1 << 1),	//!< do not "notify" if "value" is the same as previous raise
	IRQ_FLAG_ALLOC 			= (1 << 2), //!< this irq structure was malloced via stm_alloc_irq
	IRQ_FLAG_INIT 			= (1 << 3), //!< this irq hasn't been used yet
	IRQ_FLAG_FLOATING 	= (1 << 4), //!< this 'pin'/signal is floating
	IRQ_FLAG_USER 			= (1 << 5), //!< Can be used by irq users
};

/*
 * IRQ Pool structure
 */
typedef struct stm_irq_pool_t
{
	int count;						//!< number of irqs living in the pool
	struct stm_irq_t ** irq;		//!< irqs belonging in this pool
} stm_irq_pool_t;

/*!
 * Public IRQ structure
 */
typedef struct stm_irq_t
{
	struct stm_irq_pool_t * pool;
	const char * name;
	uint32_t irq;		//!< any value the user needs
	uint32_t value;		//!< current value
	uint8_t flags;		//!< IRQ_* flags
	struct stm_irq_hook_t * hook;	//!< list of hooks to be notified
} stm_irq_t;

//! allocates 'count' IRQs, initializes their "irq" starting from 'base' and increment
stm_irq_t * stm_alloc_irq(stm_irq_pool_t * pool, uint32_t base, uint32_t count,
		const char ** names /* optional */);

void stm_free_irq(stm_irq_t * irq, uint32_t count);

//! init 'count' IRQs, initializes their "irq" starting from 'base' and increment
void stm_init_irq(stm_irq_pool_t * pool, stm_irq_t * irq, uint32_t base,
		uint32_t count, const char ** names /* optional */);

//! Returns the current IRQ flags
uint8_t stm_irq_get_flags(stm_irq_t * irq);

//! Sets this irq's flags
void stm_irq_set_flags(stm_irq_t * irq, uint8_t flags);

//! 'raise' an IRQ. Ie call their 'hooks', and raise any chained IRQs, and set the new 'value'
void stm_raise_irq(stm_irq_t * irq, uint32_t value);

//! Same as avr_raise_irq(), but also allow setting the float status
void stm_raise_irq_float(stm_irq_t * irq, uint32_t value, int floating);

//! this connects a "source" IRQ to a "destination" IRQ
void stm_connect_irq(stm_irq_t * src, stm_irq_t * dst);
void stm_unconnect_irq(stm_irq_t * src, stm_irq_t * dst);

//! register a notification 'hook' for 'irq' -- 'param' is anything that your want passed back as argument
void stm_irq_register_notify(stm_irq_t * irq, stm_irq_notify_t notify, void * param);

void stm_irq_unregister_notify(stm_irq_t * irq, stm_irq_notify_t notify,
		void * param);

#ifdef __cplusplus
};
#endif

#endif /* __SIM_IRQ_H__ */
