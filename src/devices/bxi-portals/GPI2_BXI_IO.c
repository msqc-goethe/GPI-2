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
#include "GASPI.h"
#include "GPI2.h"
#include "GPI2_BXI.h"

/* Communication functions */
gaspi_return_t pgaspi_dev_write(gaspi_context_t* const gctx,
                                const gaspi_segment_id_t segment_id_local,
                                const gaspi_offset_t offset_local,
                                const gaspi_rank_t rank,
                                const gaspi_segment_id_t segment_id_remote,
                                const gaspi_offset_t offset_remote,
                                const gaspi_size_t size,
                                const gaspi_queue_id_t queue) {
	int ret;
	gaspi_portals4_ctx* const portals4_dev_ctx = gctx->device->ctx;
	portals4_mr* local_mr_ptr =
	    (portals4_mr*) gctx->rrmd[segment_id_local][gctx->rank].mr[0];
	// Ugly hack: pt_index of remote segment should be the same index as the local counter part
	portals4_mr* remote_mr_ptr =
	    (portals4_mr*) gctx->rrmd[segment_id_remote][gctx->rank].mr[0];

	if(local_mr_ptr == NULL){
		GASPI_DEBUG_PRINT_ERROR("nullptr");
		return -1;
	}
	if(remote_mr_ptr == NULL){
		GASPI_DEBUG_PRINT_ERROR("nullptr");
		return -1;
	}

	if (gctx->ne_count_c[queue] == gctx->config->queue_size_max) {
		return GASPI_QUEUE_FULL;
	}

	ret = PtlPut(local_mr_ptr->comm_md[queue],
	             offset_local,
	             size,
	             PORTALS4_ACK_TYPE,
	             portals4_dev_ctx->remote_info[rank].phys_address,
	             remote_mr_ptr->pt_index,
	             0,
	             offset_remote,
	             NULL,
	             0);

	if (PTL_OK != ret) {
		GASPI_DEBUG_PRINT_ERROR("PtlPut failed with %d", ret);
		return GASPI_ERROR;
	}
	gctx->ne_count_c[queue]++;
	return GASPI_SUCCESS;
}

gaspi_return_t pgaspi_dev_read(gaspi_context_t* const gctx,
                               const gaspi_segment_id_t segment_id_local,
                               const gaspi_offset_t offset_local,
                               const gaspi_rank_t rank,
                               const gaspi_segment_id_t segment_id_remote,
                               const gaspi_offset_t offset_remote,
                               const gaspi_size_t size,
                               const gaspi_queue_id_t queue) {
	int ret;
	gaspi_portals4_ctx* const portals4_dev_ctx = gctx->device->ctx;
	portals4_mr* const local_mr_ptr =
	    (portals4_mr*) gctx->rrmd[segment_id_local][gctx->rank].mr[0];
	portals4_mr* const remote_mr_ptr =
	    (portals4_mr*) gctx->rrmd[segment_id_remote][gctx->rank].mr[0];

	if (gctx->ne_count_c[queue] == gctx->config->queue_size_max) {
		return GASPI_QUEUE_FULL;
	}

	ret = PtlGet(local_mr_ptr->comm_md[queue],
	             offset_local,
	             size,
	             portals4_dev_ctx->remote_info[rank].phys_address,
	             remote_mr_ptr->pt_index,
	             0,
	             offset_remote,
	             NULL );

	if (PTL_OK != ret) {
		GASPI_DEBUG_PRINT_ERROR("PtlGet failed with %d", ret);
		return GASPI_ERROR;
	}

	gctx->ne_count_c[queue]++;

	return GASPI_SUCCESS;
}

gaspi_return_t pgaspi_dev_purge(gaspi_context_t* const gctx,
                                const gaspi_queue_id_t queue,
                                const gaspi_timeout_t timeout_ms) {
	int ret;
	ptl_ct_event_t ce,nce;
	int nr = gctx->ne_count_c[queue];
	const int nnr = nr;
	const gaspi_cycles_t s0 = gaspi_get_cycles();
	gaspi_portals4_ctx* const portals4_dev_ctx =
	    (gaspi_portals4_ctx*) gctx->device->ctx;

	memset(&nce,0,sizeof(ptl_ct_event_t));
	do {
		ret = PtlCTGet(portals4_dev_ctx->comm_ct_handle[queue], &ce);
		if (PTL_OK != ret) {
			GASPI_DEBUG_PRINT_ERROR("PtlCTGet failed with %d", ret);
			return GASPI_ERROR;
		}

		if (ce.success == 0) {
			const gaspi_cycles_t s1 = gaspi_get_cycles();
			const gaspi_cycles_t tdelta = s1 - s0;

			const float ms = (float) tdelta * gctx->cycles_to_msecs;

			if (ms > timeout_ms) {
				GASPI_DEBUG_PRINT_ERROR("GASPI_TIMEOUT");
				return GASPI_TIMEOUT;
			}
		}
		else {
			nr -= ce.success;

		}
	} while (nr > 0);

	gctx->ne_count_c[queue] -= nnr;
	PtlCTSet(portals4_dev_ctx->comm_ct_handle[queue],nce);

	return GASPI_SUCCESS;
}

gaspi_return_t pgaspi_dev_wait(gaspi_context_t* const gctx,
                               const gaspi_queue_id_t queue,
                               const gaspi_timeout_t timeout_ms) {
	int ret;
	ptl_ct_event_t ce,nce;
	int nr = gctx->ne_count_c[queue];
	const int nnr = nr;
	const gaspi_cycles_t s0 = gaspi_get_cycles();
	gaspi_portals4_ctx* const portals4_dev_ctx =
	    (gaspi_portals4_ctx*) gctx->device->ctx;

	memset(&nce,0,sizeof(ptl_ct_event_t));
	do {
		ret = PtlCTGet(portals4_dev_ctx->comm_ct_handle[queue], &ce);
		if (PTL_OK != ret) {
			GASPI_DEBUG_PRINT_ERROR("PtlCTGet failed with %d", ret);
			return GASPI_ERROR;
		}
		else if (ce.failure > 0) {
			GASPI_DEBUG_PRINT_ERROR("Comm queue %d might be broken!", queue);
			return GASPI_ERROR;
		}

		if (ce.success == 0) {
			const gaspi_cycles_t s1 = gaspi_get_cycles();
			const gaspi_cycles_t tdelta = s1 - s0;

			//GASPI_DEBUG_PRINT_ERROR("ce.success == 0");
			const float ms = (float) tdelta * gctx->cycles_to_msecs;

			if (ms > timeout_ms) {
				GASPI_DEBUG_PRINT_ERROR("GASPI_TIMEOUT");
				return GASPI_TIMEOUT;
			}
		}
		else {
			//GASPI_DEBUG_PRINT_ERROR("received %d events",ce.success);
			nr -= ce.success;
		}
	} while (nr > 0);

	gctx->ne_count_c[queue] -= nnr;
	PtlCTSet(portals4_dev_ctx->comm_ct_handle[queue],nce);
	return GASPI_SUCCESS;
}

gaspi_return_t pgaspi_dev_write_list(
    gaspi_context_t* const gctx,
    const gaspi_number_t num,
    gaspi_segment_id_t* const segment_id_local,
    gaspi_offset_t* const offset_local,
    const gaspi_rank_t rank,
    gaspi_segment_id_t* const segment_id_remote,
    gaspi_offset_t* const offset_remote,
    gaspi_size_t* const size,
    const gaspi_queue_id_t queue) {
	int ret;
	gaspi_portals4_ctx* const portals4_dev_ctx = gctx->device->ctx;
	gaspi_number_t i;

	if (gctx->ne_count_c[queue] + num > gctx->config->queue_size_max) {
		return GASPI_QUEUE_FULL;
	}

	for (i = 0; i < num; ++i) {
		portals4_mr* const local_mr_ptr =
		    (portals4_mr*) gctx->rrmd[segment_id_local[i]][gctx->rank].mr[0];
		portals4_mr* const remote_mr_ptr =
		    (portals4_mr*) gctx->rrmd[segment_id_remote[i]][gctx->rank].mr[0];

		ret = PtlPut(local_mr_ptr->comm_md[queue],
		             offset_local[i],
		             size[i],
		             PORTALS4_ACK_TYPE,
		             portals4_dev_ctx->remote_info[rank].phys_address,
		             remote_mr_ptr->pt_index,
		             0,
		             offset_remote[i],
		             NULL,
		             0);

		if (PTL_OK != ret) {
			GASPI_DEBUG_PRINT_ERROR("PtlPut failed with %d", ret);
			return GASPI_ERROR;
		}
	}
	if (num > 0) {
		gctx->ne_count_c[queue] += num;
	}

	return GASPI_SUCCESS;
}

gaspi_return_t pgaspi_dev_read_list(gaspi_context_t* const gctx,
                                    const gaspi_number_t num,
                                    gaspi_segment_id_t* const segment_id_local,
                                    gaspi_offset_t* const offset_local,
                                    const gaspi_rank_t rank,
                                    gaspi_segment_id_t* const segment_id_remote,
                                    gaspi_offset_t* const offset_remote,
                                    gaspi_size_t* const size,
                                    const gaspi_queue_id_t queue) {
	int ret;
	gaspi_portals4_ctx* const portals4_dev_ctx = gctx->device->ctx;
	gaspi_number_t i;

	if (gctx->ne_count_c[queue] + num > gctx->config->queue_size_max) {
		return GASPI_QUEUE_FULL;
	}

	for (i = 0; i < num; ++i) {
		portals4_mr* const local_mr_ptr =
		    (portals4_mr*) gctx->rrmd[segment_id_local[i]][gctx->rank].mr[0];
		portals4_mr* const remote_mr_ptr =
		    (portals4_mr*) gctx->rrmd[segment_id_remote[i]][gctx->rank].mr[0];

		ret = PtlGet(local_mr_ptr->comm_md[queue],
		             offset_local[i],
		             size[i],
		             portals4_dev_ctx->remote_info[rank].phys_address,
		             remote_mr_ptr->pt_index,
		             0,
		             offset_remote[i],
		             NULL);

		if (PTL_OK != ret) {
			GASPI_DEBUG_PRINT_ERROR("PtlPut failed with %d", ret);
			return GASPI_ERROR;
		}
	}
	if (num > 0) {
		gctx->ne_count_c[queue] += num;
	}
	return GASPI_SUCCESS;
}

gaspi_return_t pgaspi_dev_notify(gaspi_context_t* const gctx,
                                 const gaspi_segment_id_t segment_id_remote,
                                 const gaspi_rank_t rank,
                                 const gaspi_notification_id_t notification_id,
                                 const gaspi_notification_t notification_value,
                                 const gaspi_queue_id_t queue) {
	int ret;
	gaspi_portals4_ctx* const portals4_dev_ctx = gctx->device->ctx;
	portals4_mr* const local_mr_ptr = (portals4_mr*) gctx->nsrc.mr[1];
	portals4_mr* const remote_mr_ptr =
	    (portals4_mr*) gctx->rrmd[segment_id_remote][gctx->rank].mr[1];

	if (gctx->ne_count_c[queue] == gctx->config->queue_size_max) {
		return GASPI_QUEUE_FULL;
	}
	
	((gaspi_notification_t*)gctx->nsrc.notif_spc.buf)[notification_id] = notification_value;
	ret = PtlPut(local_mr_ptr->comm_md[queue],
	             notification_id * sizeof(gaspi_notification_t),
	             sizeof(gaspi_notification_t),
	             PORTALS4_ACK_TYPE,
	             portals4_dev_ctx->remote_info[rank].phys_address,
	             remote_mr_ptr->pt_index,
	             0,
	             notification_id * sizeof(gaspi_notification_t),
	             NULL,
	             0);

	if (PTL_OK != ret) {
		GASPI_DEBUG_PRINT_ERROR("PtlPut failed with %d", ret);
		return GASPI_ERROR;
	}

	gctx->ne_count_c[queue]++;

	return GASPI_SUCCESS;
}

gaspi_return_t pgaspi_dev_write_notify(
    gaspi_context_t* const GASPI_UNUSED(gctx),
    const gaspi_segment_id_t GASPI_UNUSED(segment_id_local),
    const gaspi_offset_t GASPI_UNUSED(offset_local),
    const gaspi_rank_t GASPI_UNUSED(rank),
    const gaspi_segment_id_t GASPI_UNUSED(segment_id_remote),
    const gaspi_offset_t GASPI_UNUSED(offset_remote),
    const gaspi_size_t GASPI_UNUSED(size),
    const gaspi_notification_id_t GASPI_UNUSED(notification_id),
    const gaspi_notification_t GASPI_UNUSED(notification_value),
    const gaspi_queue_id_t GASPI_UNUSED(queue)) {
	return GASPI_SUCCESS;
}

gaspi_return_t pgaspi_dev_write_list_notify(
    gaspi_context_t* const GASPI_UNUSED(gctx),
    const gaspi_number_t GASPI_UNUSED(num),
    gaspi_segment_id_t* const GASPI_UNUSED(segment_id_local),
    gaspi_offset_t* const GASPI_UNUSED(offset_local),
    const gaspi_rank_t GASPI_UNUSED(rank),
    gaspi_segment_id_t* const GASPI_UNUSED(segment_id_remote),
    gaspi_offset_t* const GASPI_UNUSED(offset_remote),
    gaspi_size_t* const GASPI_UNUSED(size),
    const gaspi_segment_id_t GASPI_UNUSED(segment_id_notification),
    const gaspi_notification_id_t GASPI_UNUSED(notification_id),
    const gaspi_notification_t GASPI_UNUSED(notification_value),
    const gaspi_queue_id_t GASPI_UNUSED(queue)) {
	return GASPI_SUCCESS;
}

gaspi_return_t pgaspi_dev_read_notify(
    gaspi_context_t* const GASPI_UNUSED(gctx),
    const gaspi_segment_id_t GASPI_UNUSED(segment_id_local),
    const gaspi_offset_t GASPI_UNUSED(offset_local),
    const gaspi_rank_t GASPI_UNUSED(rank),
    const gaspi_segment_id_t GASPI_UNUSED(segment_id_remote),
    const gaspi_offset_t GASPI_UNUSED(offset_remote),
    const gaspi_size_t GASPI_UNUSED(size),
    const gaspi_notification_id_t GASPI_UNUSED(notification_id),
    const gaspi_queue_id_t GASPI_UNUSED(queue)) {
	return GASPI_SUCCESS;
}

gaspi_return_t pgaspi_dev_read_list_notify(
    gaspi_context_t* const GASPI_UNUSED(gctx),
    const gaspi_number_t GASPI_UNUSED(num),
    gaspi_segment_id_t* const GASPI_UNUSED(segment_id_local),
    gaspi_offset_t* const GASPI_UNUSED(offset_local),
    const gaspi_rank_t GASPI_UNUSED(rank),
    gaspi_segment_id_t* const GASPI_UNUSED(segment_id_remote),
    gaspi_offset_t* const GASPI_UNUSED(offset_remote),
    gaspi_size_t* const GASPI_UNUSED(size),
    const gaspi_segment_id_t GASPI_UNUSED(segment_id_notification),
    const gaspi_notification_id_t GASPI_UNUSED(notification_id),
    const gaspi_queue_id_t GASPI_UNUSED(queue)) {
	return GASPI_SUCCESS;
}
