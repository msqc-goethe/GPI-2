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
#include "GPI2.h"
#include "GPI2_PORTALS.h"

gaspi_return_t pgaspi_dev_passive_send(
    gaspi_context_t* const gctx,
    const gaspi_segment_id_t segment_id_local,
    const gaspi_offset_t offset_local,
    const gaspi_rank_t rank,
    const gaspi_size_t size,
    const gaspi_timeout_t timeout_ms) {
	int ret;
	const int byte_id = rank >> 3;
	const int bit_pos = rank - (byte_id * 8);
	const unsigned char bit_cmp = 1 << bit_pos;
	gaspi_portals4_ctx * const dev = gctx->device->ctx;
	const ptl_size_t local_offset =
	    gctx->rrmd[segment_id_local][gctx->rank].data.addr + offset_local;
	ptl_ct_event_t ce;

	const ptl_size_t nnr = dev->passive_comm_ct_cnt + 1;

	if (gctx->ne_count_p[byte_id] & bit_cmp) {
		goto checkL;
	}

	gctx->ne_count_p[byte_id] |= bit_cmp;

	ret = PtlPut(dev->passive_comm_md_h,
	             local_offset,
	             size,
	             PORTALS4_PASSIVE_ACK_TYPE,
	             dev->remote_info[rank].phys_address,
	             dev->passive_comm_pt_idx,
	             0,
	             0,
	             NULL,
	             0);

	if (ret != PTL_OK) {
		GASPI_DEBUG_PRINT_ERROR("PtlPut failed with %d", ret);
		return GASPI_ERROR;
	}

checkL:
	unsigned int ce_index;
	ret =
	    PtlCTPoll(&dev->passive_comm_ct_h, &nnr, 1, timeout_ms, &ce, &ce_index);

	if (ret != PTL_OK) {
		GASPI_DEBUG_PRINT_ERROR("PtlCTPoll failed with %d", ret);
		return ret == PTL_EQ_EMPTY ? GASPI_TIMEOUT : GASPI_ERROR;
	}

	gctx->ne_count_p[byte_id] &= (~bit_cmp);
	dev->passive_comm_ct_cnt = ce.success;
	return GASPI_SUCCESS;
}

gaspi_return_t pgaspi_dev_passive_receive(
    gaspi_context_t* const gctx,
    const gaspi_segment_id_t segment_id_local,
    const gaspi_offset_t offset_local,
    gaspi_rank_t* const rank,
    const gaspi_size_t size,
    const gaspi_timeout_t timeout_ms) {
	ptl_handle_me_t me_handle;
	int ret;
	gaspi_portals4_ctx * const dev = gctx->device->ctx;
	ptl_event_t event;
	unsigned int event_index;

	ret =
	    PtlEQPoll(&dev->passive_comm_eq_h, 1, timeout_ms, &event, &event_index);

	if (ret != PTL_OK) {
		GASPI_DEBUG_PRINT_ERROR("PtlEQPoll failed with %d", ret);
		return ret == PTL_EQ_EMPTY ? GASPI_TIMEOUT : GASPI_ERROR;
	}

	if (PTL_NI_OK != event.ni_fail_type) {
		GASPI_DEBUG_PRINT_ERROR("Passive receive event error %d",
		                        event.ni_fail_type);
		return GASPI_ERROR;
	}

	*rank = 0xffff;
	for (int i = 0; i < gctx->tnc; ++i) {
		ptl_process_t initiator = dev->remote_info[i].phys_address;
		if (event.initiator.phys.nid == initiator.phys.nid &&
		    event.initiator.phys.pid == initiator.phys.pid) {
			*rank = i;
			break;
		}
	}

	if (*rank == 0xffff) {
		return GASPI_ERROR;
	}
	void* dest = (void*) (gctx->rrmd[segment_id_local][gctx->rank].data.addr +
	                      offset_local);
	memcpy(dest, (void*) event.start, event.mlength);
	return GASPI_SUCCESS;
}
