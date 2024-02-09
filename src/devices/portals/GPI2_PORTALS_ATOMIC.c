/*
Copyright (c) Goethe University Frankfurt - Niklas Bartelheimer <bartelheimer@em.uni-frankfurt.de>, 2023-2026

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
#include "GASPI.h"
#include "GPI2.h"
#include "GPI2_PORTALS.h"

gaspi_return_t pgaspi_dev_atomic_fetch_add(gaspi_context_t* const gctx,
                                           const gaspi_segment_id_t segment_id,
                                           const gaspi_offset_t offset,
                                           const gaspi_rank_t rank,
                                           const gaspi_atomic_value_t val_add) {
	int ret;
	int nnr;
	ptl_ct_event_t ce;
	gaspi_portals4_ctx* const portals4_dev_ctx = gctx->device->ctx;
	portals4_mr* const local_mr_ptr = (portals4_mr*) gctx->nsrc.mr[0];
	const ptl_pt_index_t target_pt_index =
	    ((portals4_mr*) (gctx->rrmd[segment_id][gctx->rank].mr[0]))->pt_index;

	gaspi_atomic_value_t* val_arr = (gaspi_atomic_value_t*) gctx->nsrc.data.buf;
	val_arr[1] = val_add;

	ret = PtlFetchAtomic(local_mr_ptr->atomic_md,
	                     0,
	                     local_mr_ptr->atomic_md,
	                     sizeof(gaspi_atomic_value_t),
	                     sizeof(gaspi_atomic_value_t),
	                     portals4_dev_ctx->remote_info[rank].phys_address,
	                     target_pt_index,
	                     0,
	                     DATA_SEG(offset),
	                     NULL,
	                     0,
	                     PTL_SUM,
	                     PTL_UINT64_T);

	if (PTL_OK != ret) {
		GASPI_DEBUG_PRINT_ERROR("PtlFetchAtomic failed with %d", ret);
		return GASPI_ERROR;
	}

	ret = PtlAtomicSync();
	if (PTL_OK != ret) {
		GASPI_DEBUG_PRINT_ERROR("PtlAtomicSync failed with %d", ret);
		return GASPI_ERROR;
	}

	ret = PtlCTWait(portals4_dev_ctx->group_ct_handle,
	                portals4_dev_ctx->group_ct_cnt + 1,
	                &ce);

	if (PTL_OK != ret) {
		GASPI_DEBUG_PRINT_ERROR("PtlCTWait failed with %d", ret);
		return GASPI_ERROR;
	}

	if (ce.failure > 0) {
		GASPI_DEBUG_PRINT_ERROR("atomic channel might be broken");
		return GASPI_ERROR;
	}
	portals4_dev_ctx->group_ct_cnt = ce.success;

	return GASPI_SUCCESS;
}

gaspi_return_t pgaspi_dev_atomic_compare_swap(
    gaspi_context_t* const gctx,
    const gaspi_segment_id_t segment_id,
    const gaspi_offset_t offset,
    const gaspi_rank_t rank,
    const gaspi_atomic_value_t comparator,
    const gaspi_atomic_value_t val_new) {
	int ret;
	ptl_ct_event_t ce;
	gaspi_portals4_ctx* const portals4_dev_ctx = gctx->device->ctx;
	portals4_mr* const local_mr_ptr = (portals4_mr*) gctx->nsrc.mr[0];
	const ptl_pt_index_t target_pt_index =
	    ((portals4_mr*) (gctx->rrmd[segment_id][gctx->rank].mr[0]))->pt_index;

	gaspi_atomic_value_t* val_arr = (gaspi_atomic_value_t*) gctx->nsrc.data.buf;
	val_arr[1] = val_new;

	ret = PtlSwap(local_mr_ptr->atomic_md,
	              0,
	              local_mr_ptr->atomic_md,
	              sizeof(gaspi_atomic_value_t),
	              sizeof(gaspi_atomic_value_t),
	              portals4_dev_ctx->remote_info[rank].phys_address,
	              target_pt_index,
	              0,
	              DATA_SEG(offset),
	              NULL,
	              0,
	              &comparator,
	              PTL_CSWAP,
	              PTL_UINT64_T);

	if (PTL_OK != ret) {
		GASPI_DEBUG_PRINT_ERROR("PtlFetchAtomic failed with %d", ret);
		return GASPI_ERROR;
	}

	ret = PtlAtomicSync();
	if (PTL_OK != ret) {
		GASPI_DEBUG_PRINT_ERROR("PtlAtomicSync failed with %d", ret);
		return GASPI_ERROR;
	}

	ret = PtlCTWait(portals4_dev_ctx->group_ct_handle,
	                portals4_dev_ctx->group_ct_cnt + 1,
	                &ce);

	if (PTL_OK != ret) {
		GASPI_DEBUG_PRINT_ERROR("PtlCTWait failed with %d", ret);
		return GASPI_ERROR;
	}

	if (ce.failure > 0) {
		GASPI_DEBUG_PRINT_ERROR("atomic queue might be broken");
		return GASPI_ERROR;
	}
	portals4_dev_ctx->group_ct_cnt = ce.success;

	return GASPI_SUCCESS;
}
