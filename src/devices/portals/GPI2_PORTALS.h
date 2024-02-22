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

#define PORTALS4_DATA_PT_INDEX 0
#define PORTALS4_PASSIVE_PT_INDEX 1

#define PORTALS4_EVENT_SLOTS (1024)
#define PORTALS4_ACK_TYPE PTL_CT_ACK_REQ
#define PORTALS4_PASSIVE_ACK_TYPE PTL_ACK_REQ

struct portals4_ctx_info {
	ptl_process_t phys_address;
};

typedef struct {
	//Interface
	ptl_handle_ni_t ni_h;

	//Passive Comm Internal Memory
	void* passive_comm_msg_buf;
	ptl_size_t passive_comm_msg_buf_addr;
	ptl_size_t passive_comm_msg_buf_size;

	//Memory Descriptors
	ptl_handle_md_t group_atomic_md_h;
	ptl_handle_md_t passive_comm_md_h;
	ptl_handle_md_t comm_notif_md_h[GASPI_MAX_QP];

	//Data Space Portal
	ptl_handle_le_t data_le_h;
	ptl_pt_index_t data_pt_idx;

	//Passive Comm Portal
	ptl_handle_le_t passive_comm_le_h;
	ptl_pt_index_t passive_comm_pt_idx;

	//Counting Event Queues
	ptl_handle_ct_t group_atomic_ct_h;
	ptl_handle_ct_t passive_comm_ct_h;
	ptl_handle_ct_t comm_notif_ct_h[GASPI_MAX_QP];

	//Event Queues
	ptl_handle_eq_t eq_h;
	ptl_handle_eq_t passive_comm_eq_h;

	//CT Event Counter
	ptl_size_t group_atomic_ct_cnt;
	ptl_size_t passive_comm_ct_cnt;
	ptl_size_t comm_notif_ct_cnt[GASPI_MAX_QP];

	//Exchange
	struct portals4_ctx_info* local_info;
	struct portals4_ctx_info* remote_info;
} gaspi_portals4_ctx;

#endif // _GPI2_BXI_H_
