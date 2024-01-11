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
#include <sys/mman.h>
#include <sys/timeb.h>
#include <unistd.h>
#include "GASPI.h"
#include "GPI2.h"
#include "GPI2_Coll.h"
#include "GPI2_PORTALS.h"

/* Group utilities */
int pgaspi_dev_poll_groups(gaspi_context_t* const gctx,
                           const gaspi_timeout_t timeout_ms) {
	int ret;
	unsigned int which;
	ptl_ct_event_t ce, nct;
	const gaspi_cycles_t s0 = gaspi_get_cycles();
	const ptl_size_t nr = gctx->ne_count_grp;
	gaspi_portals4_ctx* const portals4_dev_ctx = gctx->device->ctx;
	memset(&nct, 0, sizeof(ptl_ct_event_t));

	ret = PtlCTPoll(
	    &portals4_dev_ctx->group_ct_handle, &nr, 1, timeout_ms, &ce, &which);

	if (PTL_OK != ret) {
		GASPI_DEBUG_PRINT_ERROR("PtlCTPoll failed with %d", ret);
		return GASPI_ERROR;
	}

	/* do { */
	/* 	ret = PtlCTGet(portals4_dev_ctx->group_ct_handle, &ce); */

	/* 	if (PTL_OK != ret) { */
	/* 		GASPI_DEBUG_PRINT_ERROR("PtlCTGet failed with %d", ret); */
	/* 		return GASPI_ERROR; */
	/* 	} */

	/* 	if (ce.failure > 0) { */
	/* 		GASPI_DEBUG_PRINT_ERROR("Collectives queue might be broken"); */
	/* 		return GASPI_ERROR; */
	/* 	} */

	/* 	if (ce.success == 0) { */
	/* 		const gaspi_cycles_t s1 = gaspi_get_cycles(); */
	/* 		const gaspi_cycles_t tdelta = s1 - s0; */

	/* 		const float ms = (float) tdelta * gctx->cycles_to_msecs; */

	/* 		if (ms > timeout_ms) { */
	/* 			return GASPI_TIMEOUT; */
	/* 		} */
	/* 	} */
	/* } while (ce.success != nnr); */

	if (ce.failure > 0) {
		GASPI_DEBUG_PRINT_ERROR(
		    "Res: %d Success: %d Failure %d", nr, ce.success, ce.failure);
		return GASPI_ERROR;
	}
	gctx->ne_count_grp -= nr;

	if (gctx->ne_count_grp != 0) {
		GASPI_DEBUG_PRINT_ERROR("group count error, count is %d",
		                        gctx->ne_count_grp);
		return -1;
	}

	PtlCTSet(portals4_dev_ctx->group_ct_handle, nct);
	return nr;
}

int pgaspi_dev_post_group_write(gaspi_context_t* const gctx,
                                void* local_addr,
                                int length,
                                int dst,
                                void* remote_addr,
                                unsigned char group) {
	int ret;
	gaspi_portals4_ctx* const portals4_dev_ctx =
	    (gaspi_portals4_ctx*) gctx->device->ctx;
	ptl_handle_md_t md =
	    ((portals4_mr*) gctx->groups[group].rrcd[gctx->rank].mr[0])->group_md;
	ptl_pt_index_t pt_index =
	    ((portals4_mr*) gctx->groups[group].rrcd[gctx->rank].mr[0])->pt_index;
	ptl_size_t local_offset = (unsigned long) local_addr -
	                          gctx->groups[group].rrcd[gctx->rank].data.addr;
	ptl_size_t remote_offset =
	    (unsigned long) remote_addr - gctx->groups[group].rrcd[dst].data.addr;

	ret = PtlPut(md,
	             local_offset,
	             length,
	             PORTALS4_ACK_TYPE,
	             portals4_dev_ctx->remote_info[dst].phys_address,
	             pt_index,
	             0,
	             remote_offset,
	             NULL,
	             0);

	if (PTL_OK != ret) {
		GASPI_DEBUG_PRINT_ERROR("PtlPut failed with %d", ret);
		return -1;
	}

	gctx->ne_count_grp++;
	return 0;
}