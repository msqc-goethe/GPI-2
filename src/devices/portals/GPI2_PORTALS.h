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
#include "GPI2_Dev.h"

#define PORTALS4_ME_PT_INDEX 0
#define PORTALS4_ME_MATCH_BITS 0xDEADBEEF
#define PORTALS4_ME_IGNORE_BITS 0
#define PORTALS4_EVENT_SLOTS (1024)
#define PORTALS4_ACK_TYPE PTL_CT_ACK_REQ
#define PORTALS4_PASSIVE_ACK_TYPE PTL_ACK_REQ

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

enum pt_states {PT_FREE = 0, PT_ALLOCATED = 1};

typedef struct {
	ptl_handle_md_t group_md;
	ptl_handle_md_t passive_md;
	ptl_handle_md_t comm_md[GASPI_MAX_QP];
	ptl_handle_le_t le_handle;
	ptl_pt_index_t  pt_index;
} portals4_mr;

struct portals4_ctx_info {
	ptl_process_t phys_address;
	ptl_process_t matching_phys_address;
};

typedef struct {
	ptl_handle_ni_t ni_handle;
	ptl_handle_ni_t match_ni_handle;
	ptl_handle_eq_t eq_handle;
	ptl_handle_eq_t passive_rcv_eq_handle;
	ptl_handle_ct_t group_ct_handle;
	ptl_handle_ct_t passive_snd_ct_handle;
	ptl_handle_ct_t comm_ct_handle[GASPI_MAX_QP];
	ptl_pt_index_t  me_pt_index; // passive comm matching interface
	//ptl_handle_me_t me_handle; // passive comm matching interface
	struct portals4_ctx_info* local_info;
	struct portals4_ctx_info* remote_info;
	int8_t *pte_states;
	int max_ptes;
} gaspi_portals4_ctx;

#endif // _GPI2_BXI_H_
