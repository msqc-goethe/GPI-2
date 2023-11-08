/*
Copyright (c) Goethe University Frankfurt MSQC - Niklas Bartelheimer <bartelheimer@em.uni-frankfurt.de>, 2023-2026

This file is part of GPI-2.

GPI-2 is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License
version 3 as published by the Free Software Foundation.

GPI-2 is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with GPI-2. If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef _GPI2_BXI_H_
#define _GPI2_BXI_H_

#include <portals4.h>
#include <portals4_bxiext.h>

#include "GPI2_Dev.h"

#define GPI2_PORTALS4_DATA_PT (0)
#define GPI2_PORTALS4_NOTIFY_SPC_PT (1)
#define GPI2_PORTALS4_EVENT_SLOTS (1024)

#ifdef DEBUG
#define PORTALS4_DEBUG_PRINT_MSG(msg, ...)       \
	{                                            \
		fprintf(stdout,                          \
		        "Message at (%s:%d): " msg "\n", \
		        __FILE__,                        \
		        __LINE__,                        \
		        ##__VA_ARGS__);                  \
	}
#endif

struct portals4_ctx_info {
	ptl_process_t phys_address;
	ptl_process_t logical_address;
	int nGroup;
	int nP;
	int nC;
};

typedef struct {
	ptl_handle_ni_t ni_handle;
	ptl_handle_eq_t data_eq_handle;
	ptl_handle_eq_t notify_spc_eq_handle;
	ptl_handle_ct_t data_ct_handle;
	ptl_handle_ct_t notify_spc_ct_handle;
	ptl_pt_index_t data_pt_index;
	ptl_pt_index_t notify_spc_pt_index;
	struct portals4_ctx_info* local_info;
	struct portals4_ctx_info* remote_info;
} gaspi_portals4_ctx;

#endif // _GPI2_BXI_H_
