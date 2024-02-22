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
int pgaspi_dev_poll_groups(gaspi_context_t* const gctx) {
	int ret;
	ptl_ct_event_t ce;
	const ptl_size_t nr = gctx->ne_count_grp;
	gaspi_portals4_ctx* const dev = gctx->device->ctx;

	ret = PtlCTWait(dev->group_atomic_ct_h, dev->group_atomic_ct_cnt + nr, &ce);
	if (ret != PTL_OK) {
		GASPI_DEBUG_PRINT_ERROR("PtlCTPoll failed with %d", ret);
		return GASPI_ERROR;
	}

	if (ce.failure > 0) {
		GASPI_DEBUG_PRINT_ERROR(
		    "Res: %d Success: %d Failure %d", nr, ce.success, ce.failure);
		return GASPI_ERROR;
	}
	gctx->ne_count_grp -= nr;
	dev->group_atomic_ct_cnt = ce.success;
	return nr;
}

int pgaspi_dev_post_group_write(gaspi_context_t* const gctx,
                                void* local_addr,
                                int length,
                                int dst,
                                void* remote_addr,
                                unsigned char group) {
	int ret;
	gaspi_portals4_ctx* const dev = (gaspi_portals4_ctx*) gctx->device->ctx;

	ret = PtlPut(dev->group_atomic_md_h,
	             (ptl_size_t) local_addr,
	             length,
	             PORTALS4_ACK_TYPE,
	             dev->remote_info[dst].phys_address,
	             dev->data_pt_idx,
	             0,
	             (ptl_size_t) remote_addr,
	             NULL,
	             0);

	if (ret != PTL_OK) {
		GASPI_DEBUG_PRINT_ERROR("PtlPut failed with %d", ret);
		return -1;
	}

	gctx->ne_count_grp++;
	return 0;
}
