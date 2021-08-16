/*
	sim_io.c

	Copyright 2008, 2009 Michel Pollet <buserror@gmail.com>

 	This file is part of simstm.

	simstm is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	simstm is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with simstm.  If not, see <http://www.gnu.org/licenses/>.
 */


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>

#include "sim_inc.h"


int stm_ioctl( stm_t *stm, uint32_t ctl, void * io_param )
{
	stm_io_t * port = stm->io_port;
	int res = -1;
	while (port && res == -1) {
		if (port->ioctl)
			res = port->ioctl(port, ctl, io_param);
		port = port->next;
	}
	return res;
}

void stm_register_io( stm_t *stm, stm_io_t * io )
{
	io->next = stm->io_port;
	io->stm = stm;
	stm->io_port = io;
}

void stm_register_io_read( stm_t *stm, stm_io_addr_t addr, stm_io_read_t readp, void * param)
{
//	stm_io_addr_t a = STM_DATA_TO_IO(addr);
//	if (stm->io[a].r.param || stm->io[a].r.c) {
//		if (stm->io[a].r.param != param || stm->io[a].r.c != readp)
//		{
//			STM_LOG(stm, LOG_ERROR,
//					"IO: %s(): Already registered, refusing to override.\n",
//					__func__);
//			STM_LOG(stm, LOG_ERROR,
//					"IO: %s(%04x : %p/%p): %p/%p\n",
//					__func__, a,
//					stm->io[a].r.c, stm->io[a].r.param, readp, param);
//			abort();
//		}
//	}
//	stm->io[a].r.param = param;
//	stm->io[a].r.c = readp;
}

//static void _stm_io_mux_write( stm_t * stm, stm_io_addr_t addr, uint8_t v, void * param)
//{
//	int io = (intptr_t)param;
//	for (int i = 0; i < stm->io_shared_io[io].used; i++) {
//		stm_io_write_t c = stm->io_shared_io[io].io[i].c;
//		if (c)
//			c(stm, addr, v, stm->io_shared_io[io].io[i].param);
//	}
//}

void stm_register_io_write( stm_t *stm, stm_io_addr_t addr, stm_io_write_t writep, void * param )
{
//	stm_io_addr_t a = STM_DATA_TO_IO(addr);
//
//	if (a >= MAX_IOs) {
//		STM_LOG(stm, LOG_ERROR,
//				"IO: %s(): IO address 0x%04x out of range (max 0x%04x).\n",
//				__func__, a, MAX_IOs);
//		abort();
//	}
//	/*
//	 * Verifying that some other piece of code is not installed to watch write
//	 * on this address. If there is, this code installs a "dispatcher" callback
//	 * instead to handle multiple clients, otherwise, it continues as usual
//	 */
//	if (stm->io[a].w.param || stm->io[a].w.c) {
//		if (stm->io[a].w.param != param || stm->io[a].w.c != writep) {
//			// if the muxer not already installed, allocate a new slot
//			if (stm->io[a].w.c != _stm_io_mux_write) {
//				int no = stm->io_shared_io_count++;
//				if (stm->io_shared_io_count > ARRAY_SIZE(stm->io_shared_io)) {
//					STM_LOG(stm, LOG_ERROR,
//							"IO: %s(): Too many shared IO registers.\n", __func__);
//					abort();
//				}
//				STM_LOG(stm, LOG_TRACE,
//						"IO: %s(%04x): Installing muxer on register.\n",
//						__func__, addr);
//				stm->io_shared_io[no].used = 1;
//				stm->io_shared_io[no].io[0].param = stm->io[a].w.param;
//				stm->io_shared_io[no].io[0].c = stm->io[a].w.c;
//				stm->io[a].w.param = (void*)(intptr_t)no;
//				stm->io[a].w.c = _stm_io_mux_write;
//			}
//			int no = (intptr_t)stm->io[a].w.param;
//			int d = stm->io_shared_io[no].used++;
//			if (stm->io_shared_io[no].used > ARRAY_SIZE(stm->io_shared_io[0].io)) {
//				STM_LOG(stm, LOG_ERROR,
//						"IO: %s(): Too many callbacks on %04x.\n",
//						__func__, addr);
//				abort();
//			}
//			stm->io_shared_io[no].io[d].param = param;
//			stm->io_shared_io[no].io[d].c = writep;
//			return;
//		}
//	}
//
//	stm->io[a].w.param = param;
//	stm->io[a].w.c = writep;
}

stm_irq_t * stm_io_getirq( stm_t * stm, uint32_t ctl, int index)
{
	stm_io_t * port = stm->io_port;
	while (port) {
		if (port->irq && port->irq_ioctl_get == ctl && port->irq_count > index)
			return port->irq + index;
		port = port->next;
	}
	return NULL;
}

stm_irq_t * stm_iomem_getirq( stm_t * stm, stm_io_addr_t addr, const char * name, int index)
{
//	if (index > 8)
//		return NULL;
//	stm_io_addr_t a = STM_DATA_TO_IO(addr);
//	if (stm->io[a].irq == NULL) {
//		/*
//		 * Prepare an array of names for the io IRQs. Ideally we'd love to have
//		 * a proper name for these, but it's not possible at this time.
//		 */
//		char names[9 * 20];
//		char * d = names;
//		const char * namep[9];
//		for (int ni = 0; ni < 9; ni++) {
//			if (ni < 8)
//				sprintf(d, "=stm.io.%04x.%d", addr, ni);
//			else
//				sprintf(d, "8=stm.io.%04x.all", addr);
//			namep[ni] = d;
//			d += strlen(d) + 1;
//		}
//		stm->io[a].irq = stm_alloc_irq(&stm->irq_pool, 0, 9, namep);
//		// mark the pin ones as filtered, so they only are raised when changing
//		for (int i = 0; i < 8; i++)
//			stm->io[a].irq[i].flags |= IRQ_FLAG_FILTERED;
//	}
//	// if given a name, replace the default one...
//	if (name) {
//		int l = strlen(name);
//		char n[l + 10];
//		sprintf(n, "stm.io.%s", name);
//		free((void*)stm->io[a].irq[index].name);
//		stm->io[a].irq[index].name = strdup(n);
//	}
//	return stm->io[a].irq + index;
	return 0x00;
}

stm_irq_t * stm_io_setirqs( stm_io_t * io, uint32_t ctl, int count, stm_irq_t * irqs )
{
	// allocate this module's IRQ
	io->irq_count = count;

	if (!irqs) {
		const char ** irq_names = NULL;

		if (io->irq_names) {
			irq_names = malloc(count * sizeof(char*));
			memset(irq_names, 0, count * sizeof(char*));
			char buf[64];
			for (int i = 0; i < count; i++) {
				/*
				 * this bit takes the io module 'kind' ("port")
				 * the IRQ name ("=0") and the last character of the ioctl ('p','o','r','A')
				 * to create a full name "=porta.0"
				 */
				char * dst = buf;
				// copy the 'flags' of the name out
				const char * kind = io->irq_names[i];
				while (isdigit(*kind))
					*dst++ = *kind++;
				while (!isalpha(*kind))
					*dst++ = *kind++;
				// add stm name
//				strcpy(dst, io->stm->mmcu);
				strcpy(dst, "stm");
				dst += strlen(dst);
				*dst ++ = '.';
				// add module 'kind'
				strcpy(dst, io->kind);
				dst += strlen(dst);
				// add port name, if any
				if ((ctl & 0xff) > ' ')
					*dst ++ = tolower(ctl & 0xff);
				*dst ++ = '.';
				// add the rest of the irq name
				strcpy(dst, kind);
				dst += strlen(dst);
				*dst = 0;

//				printf("%s\n", buf);
				irq_names[i] = strdup(buf);
			}
		}
		irqs = stm_alloc_irq(&io->stm->irq_pool, 0,
						count, irq_names);
		if (irq_names) {
			for (int i = 0; i < count; i++)
				free((char*)irq_names[i]);
			free((char*)irq_names);
		}
	}

	io->irq = irqs;
	io->irq_ioctl_get = ctl;
	return io->irq;
}

static void stm_deallocate_io( stm_io_t * io )
{
	if (io->dealloc)
		io->dealloc(io);
	stm_free_irq(io->irq, io->irq_count);
	io->irq_count = 0;
	io->irq_ioctl_get = 0;
	io->stm = NULL;
	io->next = NULL;
}

void stm_deallocate_ios( struct stm_t * stm )
{
	stm_io_t * port = stm->io_port;
	while (port) {
		stm_io_t * next = port->next;
		stm_deallocate_io(port);
		port = next;
	}
	stm->io_port = NULL;
}
