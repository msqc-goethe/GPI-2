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
	gaspi_portals4_ctx* const portals4_dev_ctx = gctx->device->ctx;
	portals4_mr* local_mr_ptr =
	    (portals4_mr*) gctx->rrmd[segment_id_local][gctx->rank].mr[0];
	gaspi_cycles_t s0;

	const int byte_id = rank >> 3;
	const int bit_pos rank - (byte_id * 8);
	const unsigned char bit_cmp = 1 << bit_pos;

	ret = PtlPut(local_mr_ptr->passive_md,
	             offset_local,
	             size,
	             PORTALS4_ACK_TYPE,
	             portals4_dev_ctx->remote_info[rank].phys_address,
	             portals4_dev_ctx->me_pt_index,
	             0,
	             0,
	             NULL,
	             0);

	if (PTL_OK != ret) {
		GASPI_DEBUG_PRINT_ERROR("PtlPut failed with %d", ret);
		return GASPI_ERROR;
	}

	gctx->ne_count_c[queue]++;

	return GASPI_SUCCESS;
}

gaspi_return_t pgaspi_dev_passive_receive(
    gaspi_context_t* const GASPI_UNUSED(gctx),
    const gaspi_segment_id_t GASPI_UNUSED(segment_id_local),
    const gaspi_offset_t GASPI_UNUSED(offset_local),
    gaspi_rank_t* const GASPI_UNUSED(rank),
    const gaspi_size_t GASPI_UNUSED(size),
    const gaspi_timeout_t GASPI_UNUSED(timeout_ms)) {
	return GASPI_SUCCESS;
}
