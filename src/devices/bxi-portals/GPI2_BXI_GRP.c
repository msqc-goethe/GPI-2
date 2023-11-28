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
#include "GPI2_BXI.h"
#include "GPI2_Coll.h"

/* Group utilities */
int pgaspi_dev_poll_groups(gaspi_context_t* const gctx) {
	int ret;
	ptl_ct_event_t ct,nct;

	gaspi_portals4_ctx* const portals4_dev_ctx = gctx->device->ctx;
	memset(&nct,0,sizeof(ptl_ct_event_t));
	ret = PtlCTGet(portals4_dev_ctx->group_ct_handle, &ct);

	if (PTL_OK != ret) {
		GASPI_DEBUG_PRINT_ERROR("PtlCTGet failed with %d", ret);
		return -1;
	}

	if (ct.failure > 0) {
		GASPI_DEBUG_PRINT_ERROR("Collectives queue might be broken");
		return -1;
	}

	gctx->ne_count_grp -= ct.success;
	PtlCTSet(portals4_dev_ctx->group_ct_handle,nct);
	return ct.success;
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
	    ((portals4_mr*)gctx->groups[group].rrcd[gctx->rank].mr[0])->group_md;
	ptl_pt_index_t pt_index = ((portals4_mr*)gctx->groups[group].rrcd[gctx->rank].mr[0])->pt_index;
	ptl_size_t local_offset = (unsigned long) local_addr -
	                          gctx->groups[group].rrcd[gctx->rank].data.addr;
	ptl_size_t remote_offset = (unsigned long) remote_addr - gctx->groups[group].rrcd[dst].data.addr;

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
