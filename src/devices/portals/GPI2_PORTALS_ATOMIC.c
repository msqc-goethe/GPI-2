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
	gaspi_portals4_ctx* const dev = gctx->device->ctx;
	const ptl_size_t remote_offset =
	    gctx->rrmd[segment_id][rank].data.addr + offset;

	ret = PtlFetchAtomic(dev->group_atomic_md_h,
	                     gctx->nsrc.data.addr,
	                     dev->group_atomic_md_h,
			     (ptl_size_t)&val_add,
	                     sizeof(gaspi_atomic_value_t),
	                     dev->remote_info[rank].phys_address,
	                     dev->data_pt_idx,
	                     0,
	                     remote_offset,
	                     NULL,
	                     0,
	                     PTL_SUM,
	                     PTL_UINT64_T);

	if (ret != PTL_OK) {
		GASPI_DEBUG_PRINT_ERROR("PtlFetchAtomic failed with %d", ret);
		return GASPI_ERROR;
	}

	ret = PtlAtomicSync();
	if (ret != PTL_OK) {
		GASPI_DEBUG_PRINT_ERROR("PtlAtomicSync failed with %d", ret);
		return GASPI_ERROR;
	}

	ret = PtlCTWait(dev->group_atomic_ct_h, dev->group_atomic_ct_cnt + 1, &ce);

	if (ret != PTL_OK) {
		GASPI_DEBUG_PRINT_ERROR("PtlCTWait failed with %d", ret);
		return GASPI_ERROR;
	}

	if (ce.failure > 0) {
		GASPI_DEBUG_PRINT_ERROR("atomic channel might be broken");
		return GASPI_ERROR;
	}
	dev->group_atomic_ct_cnt = ce.success;

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
	gaspi_portals4_ctx* const dev = gctx->device->ctx;
	const ptl_size_t remote_offset =
	    gctx->rrmd[segment_id][rank].data.addr + offset;

	ret = PtlSwap(dev->group_atomic_md_h,
	              gctx->nsrc.data.addr,
	              dev->group_atomic_md_h,
		      (ptl_size_t)&val_new,
	              sizeof(gaspi_atomic_value_t),
	              dev->remote_info[rank].phys_address,
	              dev->data_pt_idx,
	              0,
	              remote_offset,
	              NULL,
	              0,
	              &comparator,
	              PTL_CSWAP,
	              PTL_UINT64_T);

	if (ret != PTL_OK) {
		GASPI_DEBUG_PRINT_ERROR("PtlFetchAtomic failed with %d", ret);
		return GASPI_ERROR;
	}

	ret = PtlAtomicSync();
	if (ret != PTL_OK) {
		GASPI_DEBUG_PRINT_ERROR("PtlAtomicSync failed with %d", ret);
		return GASPI_ERROR;
	}

	ret = PtlCTWait(dev->group_atomic_ct_h, dev->group_atomic_ct_cnt + 1, &ce);

	if (ret != PTL_OK) {
		GASPI_DEBUG_PRINT_ERROR("PtlCTWait failed with %d", ret);
		return GASPI_ERROR;
	}

	if (ce.failure > 0) {
		GASPI_DEBUG_PRINT_ERROR("atomic queue might be broken");
		return GASPI_ERROR;
	}
	dev->group_atomic_ct_cnt = ce.success;

	return GASPI_SUCCESS;
}
