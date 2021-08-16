/*
	sim_cycle_timers.c

	Copyright 2008-2012 Michel Pollet <buserror@gmail.com>

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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "sim_inc.h"

#include "sim_time.h"


#define QUEUE(__q, __e) { \
		(__e)->next = (__q); \
		(__q) = __e; \
	}
#define DETACH(__q, __l, __e) { \
		if (__l) \
			(__l)->next = (__e)->next; \
		else \
			(__q) = (__e)->next; \
	}
#define INSERT(__q, __l, __e) { \
		if (__l) { \
			(__e)->next = (__l)->next; \
			(__l)->next = (__e); \
		} else { \
			(__e)->next = (__q); \
			(__q) = (__e); \
		} \
	}

#define DEFAULT_SLEEP_CYCLES 1000

/**
 *
 */
void stm_cycle_timer_reset( struct stm_t * stm )
{
	stm_cycle_timer_pool_t * pool = &stm->cycle_timers;
	memset(pool, 0, sizeof(*pool));

	// queue all slots into the free queue
	for (int i = 0; i < MAX_CYCLE_TIMERS; i++)
	{
		stm_cycle_timer_slot_p t = &pool->timer_slots[i];
		QUEUE(pool->timer_free, t);
	}
	stm->run_cycle_count = 1;
	stm->run_cycle_limit = 1;
}

static stm_cycle_count_t
stm_cycle_timer_return_sleep_run_cycles_limited(
	stm_t *stm,
	stm_cycle_count_t sleep_cycle_count)
{
	// run_cycle_count is bound to run_cycle_limit but NOT less than 1 cycle...
	//	this is not an error!..  unless you like deadlock.
	stm_cycle_count_t run_cycle_count = ((stm->run_cycle_limit >= sleep_cycle_count) ?
		sleep_cycle_count : stm->run_cycle_limit);
	stm->run_cycle_count = run_cycle_count ? run_cycle_count : 1;

	// sleep cycles are returned unbounded thus preserving original behavior.
	return(sleep_cycle_count);
}

static void
stm_cycle_timer_reset_sleep_run_cycles_limited(
	stm_t *stm)
{
	stm_cycle_timer_pool_t * pool = &stm->cycle_timers;
	stm_cycle_count_t sleep_cycle_count = DEFAULT_SLEEP_CYCLES * stm->nano_ticks_per_cycle;

	if(pool->timer) {
		if(pool->timer->when > stm->nano_ticks) {
			sleep_cycle_count = pool->timer->when - stm->nano_ticks;
		} else {
			sleep_cycle_count = 0;
		}
	}

	stm_cycle_timer_return_sleep_run_cycles_limited(stm, sleep_cycle_count);
}

// no sanity checks checking here, on purpose
static void
stm_cycle_timer_insert(
		stm_t * stm,
		stm_cycle_count_t when,
		stm_cycle_timer_t timer,
		void * param)
{
	stm_cycle_timer_pool_t * pool = &stm->cycle_timers;

	when += stm->nano_ticks;

	stm_cycle_timer_slot_p t = pool->timer_free;

	if (!t) {
		STM_LOG(stm, LOG_ERROR, "CYCLE: %s: ran out of timers (%d)!\n", __func__, MAX_CYCLE_TIMERS);
		return;
	}
	// detach head
	pool->timer_free = t->next;
	t->next = NULL;
	t->timer = timer;
	t->param = param;
	t->when = when;

	// find its place in the list
	stm_cycle_timer_slot_p loop = pool->timer, last = NULL;
	while (loop) {
		if (loop->when > when)
			break;
		last = loop;
		loop = loop->next;
	}
	INSERT(pool->timer, last, t);
}

void
stm_cycle_timer_register(
		struct stm_t * stm,
		stm_cycle_count_t when,
		stm_cycle_timer_t timer,
		void * param)
{
	stm_cycle_timer_pool_t * pool = &stm->cycle_timers;

	// remove it if it was already scheduled
	stm_cycle_timer_cancel(stm, timer, param);

	if (!pool->timer_free) {
		STM_LOG(stm, LOG_ERROR, "CYCLE: %s: pool is full (%d)!\n", __func__, MAX_CYCLE_TIMERS);
		return;
	}
	stm_cycle_timer_insert(stm, when, timer, param);
	stm_cycle_timer_reset_sleep_run_cycles_limited(stm);
}

void
stm_cycle_timer_register_usec(
		stm_t * stm,
		uint32_t when,
		stm_cycle_timer_t timer,
		void * param)
{
	stm_cycle_timer_register(stm, stm_usec_to_cycles(stm, when), timer, param);
}

void
stm_cycle_timer_cancel(
		stm_t * stm,
		stm_cycle_timer_t timer,
		void * param)
{
	stm_cycle_timer_pool_t * pool = &stm->cycle_timers;

	// find its place in the list
	stm_cycle_timer_slot_p t = pool->timer, last = NULL;
	while (t) {
		if (t->timer == timer && t->param == param) {
			DETACH(pool->timer, last, t);
			QUEUE(pool->timer_free, t);
			break;
		}
		last = t;
		t = t->next;
	}
	stm_cycle_timer_reset_sleep_run_cycles_limited(stm);
}

/*
 * Check to see if a timer is present, if so, return the number (+1) of
 * cycles left for it to fire, and if not present, return zero
 */
stm_cycle_count_t
stm_cycle_timer_status(
		stm_t * stm,
		stm_cycle_timer_t timer,
		void * param)
{
	stm_cycle_timer_pool_t * pool = &stm->cycle_timers;

	// find its place in the list
	stm_cycle_timer_slot_p t = pool->timer;
	while (t) {
		if (t->timer == timer && t->param == param) {
			return 1 + (t->when - stm->nano_ticks);
		}
		t = t->next;
	}
	return 0;
}

/*
 * run through all the timers, call the ones that needs it,
 * clear the ones that wants it, and calculate the next
 * potential cycle we could sleep for...
 */
stm_cycle_count_t stm_cycle_timer_process( stm_t * stm )
{
	stm_cycle_timer_pool_t * pool = &stm->cycle_timers;

	if (pool->timer) do
	{
		stm_cycle_timer_slot_p t = pool->timer;
		stm_cycle_count_t when = t->when;

		if (when > stm->nano_ticks)
			return stm_cycle_timer_return_sleep_run_cycles_limited(stm, when - stm->nano_ticks);

		// detach from active timers
		pool->timer = t->next;
		t->next = NULL;
		do {
			stm_cycle_count_t w = t->timer(stm, when, t->param);
			// make sure the return value is either zero, or greater
			// than the last one to prevent infinite loop here
			when = w > when ? w : 0;
		} while (when && when <= stm->nano_ticks);
		
		if (when) // reschedule then
			stm_cycle_timer_insert(stm, when - stm->nano_ticks, t->timer, t->param);
		
		// requeue this one into the free ones
		QUEUE(pool->timer_free, t);
	} while (pool->timer);

	// original behavior was to return 1000 cycles when no timers were present...
	// run_cycles are bound to at least one cycle but no more than requested limit...
	//	value passed here is returned unbounded, thus preserving original behavior.
	return stm_cycle_timer_return_sleep_run_cycles_limited(stm, DEFAULT_SLEEP_CYCLES * stm->nano_ticks_per_cycle);
}


/////////////////////////////////////////////////////////////////////////////
//void stm_cycle_timer_reset( struct stm_t * stm )
//{
//	stm_cycle_timer_pool_t * pool = &stm->cycle_timers;
//	memset(pool, 0, sizeof(*pool));
//
//	// queue all slots into the free queue
//	for (int i = 0; i < MAX_CYCLE_TIMERS; i++)
//	{
//		stm_cycle_timer_slot_p t = &pool->timer_slots[i];
//		QUEUE(pool->timer_free, t);
//	}
//	stm->run_cycle_count = 1;
//	stm->run_cycle_limit = 1;
//}
//
//static stm_cycle_count_t
//stm_cycle_timer_return_sleep_run_cycles_limited(
//	stm_t *stm,
//	stm_cycle_count_t sleep_cycle_count)
//{
//	// run_cycle_count is bound to run_cycle_limit but NOT less than 1 cycle...
//	//	this is not an error!..  unless you like deadlock.
//	stm_cycle_count_t run_cycle_count = ((stm->run_cycle_limit >= sleep_cycle_count) ?
//		sleep_cycle_count : stm->run_cycle_limit);
//	stm->run_cycle_count = run_cycle_count ? run_cycle_count : 1;
//
//	// sleep cycles are returned unbounded thus preserving original behavior.
//	return(sleep_cycle_count);
//}
//
//static void
//stm_cycle_timer_reset_sleep_run_cycles_limited(
//	stm_t *stm)
//{
//	stm_cycle_timer_pool_t * pool = &stm->cycle_timers;
//	stm_cycle_count_t sleep_cycle_count = DEFAULT_SLEEP_CYCLES;
//
//	if(pool->timer) {
//		if(pool->timer->when > stm->cycle) {
//			sleep_cycle_count = pool->timer->when - stm->cycle;
//		} else {
//			sleep_cycle_count = 0;
//		}
//	}
//
//	stm_cycle_timer_return_sleep_run_cycles_limited(stm, sleep_cycle_count);
//}
//
//// no sanity checks checking here, on purpose
//static void
//stm_cycle_timer_insert(
//		stm_t * stm,
//		stm_cycle_count_t when,
//		stm_cycle_timer_t timer,
//		void * param)
//{
//	stm_cycle_timer_pool_t * pool = &stm->cycle_timers;
//
//	when += stm->cycle;
//
//	stm_cycle_timer_slot_p t = pool->timer_free;
//
//	if (!t) {
//		STM_LOG(stm, LOG_ERROR, "CYCLE: %s: ran out of timers (%d)!\n", __func__, MAX_CYCLE_TIMERS);
//		return;
//	}
//	// detach head
//	pool->timer_free = t->next;
//	t->next = NULL;
//	t->timer = timer;
//	t->param = param;
//	t->when = when;
//
//	// find its place in the list
//	stm_cycle_timer_slot_p loop = pool->timer, last = NULL;
//	while (loop) {
//		if (loop->when > when)
//			break;
//		last = loop;
//		loop = loop->next;
//	}
//	INSERT(pool->timer, last, t);
//}
//
//void
//stm_cycle_timer_register(
//		struct stm_t * stm,
//		stm_cycle_count_t when,
//		stm_cycle_timer_t timer,
//		void * param)
//{
//	stm_cycle_timer_pool_t * pool = &stm->cycle_timers;
//
//	// remove it if it was already scheduled
//	stm_cycle_timer_cancel(stm, timer, param);
//
//	if (!pool->timer_free) {
//		STM_LOG(stm, LOG_ERROR, "CYCLE: %s: pool is full (%d)!\n", __func__, MAX_CYCLE_TIMERS);
//		return;
//	}
//	stm_cycle_timer_insert(stm, when, timer, param);
//	stm_cycle_timer_reset_sleep_run_cycles_limited(stm);
//}
//
//void
//stm_cycle_timer_register_usec(
//		stm_t * stm,
//		uint32_t when,
//		stm_cycle_timer_t timer,
//		void * param)
//{
//	stm_cycle_timer_register(stm, stm_usec_to_cycles(stm, when), timer, param);
//}
//
//void
//stm_cycle_timer_cancel(
//		stm_t * stm,
//		stm_cycle_timer_t timer,
//		void * param)
//{
//	stm_cycle_timer_pool_t * pool = &stm->cycle_timers;
//
//	// find its place in the list
//	stm_cycle_timer_slot_p t = pool->timer, last = NULL;
//	while (t) {
//		if (t->timer == timer && t->param == param) {
//			DETACH(pool->timer, last, t);
//			QUEUE(pool->timer_free, t);
//			break;
//		}
//		last = t;
//		t = t->next;
//	}
//	stm_cycle_timer_reset_sleep_run_cycles_limited(stm);
//}
//
///*
// * Check to see if a timer is present, if so, return the number (+1) of
// * cycles left for it to fire, and if not present, return zero
// */
//stm_cycle_count_t
//stm_cycle_timer_status(
//		stm_t * stm,
//		stm_cycle_timer_t timer,
//		void * param)
//{
//	stm_cycle_timer_pool_t * pool = &stm->cycle_timers;
//
//	// find its place in the list
//	stm_cycle_timer_slot_p t = pool->timer;
//	while (t) {
//		if (t->timer == timer && t->param == param) {
//			return 1 + (t->when - stm->cycle);
//		}
//		t = t->next;
//	}
//	return 0;
//}
//
///*
// * run through all the timers, call the ones that needs it,
// * clear the ones that wants it, and calculate the next
// * potential cycle we could sleep for...
// */
//stm_cycle_count_t stm_cycle_timer_process( stm_t * stm )
//{
//	stm_cycle_timer_pool_t * pool = &stm->cycle_timers;
//
//	if (pool->timer) do
//	{
//		stm_cycle_timer_slot_p t = pool->timer;
//		stm_cycle_count_t when = t->when;
//
//		if (when > stm->cycle)
//			return stm_cycle_timer_return_sleep_run_cycles_limited(stm, when - stm->cycle);
//
//		// detach from active timers
//		pool->timer = t->next;
//		t->next = NULL;
//		do {
//			stm_cycle_count_t w = t->timer(stm, when, t->param);
//			// make sure the return value is either zero, or greater
//			// than the last one to prevent infinite loop here
//			when = w > when ? w : 0;
//		} while (when && when <= stm->cycle);
//
//		if (when) // reschedule then
//			stm_cycle_timer_insert(stm, when - stm->cycle, t->timer, t->param);
//
//		// requeue this one into the free ones
//		QUEUE(pool->timer_free, t);
//	} while (pool->timer);
//
//	// original behavior was to return 1000 cycles when no timers were present...
//	// run_cycles are bound to at least one cycle but no more than requested limit...
//	//	value passed here is returned unbounded, thus preserving original behavior.
//	return stm_cycle_timer_return_sleep_run_cycles_limited(stm, DEFAULT_SLEEP_CYCLES);
//}

