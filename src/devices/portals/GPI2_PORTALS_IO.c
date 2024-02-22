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
#include "GPI2_PORTALS.h"

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
	gaspi_portals4_ctx* const dev = gctx->device->ctx;
	const ptl_size_t local_offset =
	    gctx->rrmd[segment_id_local][gctx->rank].data.addr + offset_local;
	const ptl_size_t remote_offset =
	    gctx->rrmd[segment_id_remote][rank].data.addr + offset_remote;

	if (gctx->ne_count_c[queue] == gctx->config->queue_size_max) {
		GASPI_DEBUG_PRINT_ERROR(
		    "pgaspi_dev_write GASPI_QUEUE_FULL is: %d max: %d",
		    gctx->ne_count_c[queue],
		    gctx->config->queue_size_max);
		return GASPI_QUEUE_FULL;
	}

	ret = PtlPut(dev->comm_notif_md_h[queue],
	             local_offset,
	             size,
	             PORTALS4_ACK_TYPE,
	             dev->remote_info[rank].phys_address,
	             dev->data_pt_idx,
	             0,
	             remote_offset,
	             NULL,
	             0);

	if (ret != PTL_OK) {
		GASPI_DEBUG_PRINT_ERROR("PtlPut failed with %d for size %d", ret, size);
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
	gaspi_portals4_ctx* const dev = gctx->device->ctx;
	const ptl_size_t local_offset =
	    gctx->rrmd[segment_id_local][gctx->rank].data.addr + offset_local;
	const ptl_size_t remote_offset =
	    gctx->rrmd[segment_id_remote][rank].data.addr + offset_remote;

	if (gctx->ne_count_c[queue] == gctx->config->queue_size_max) {
		GASPI_DEBUG_PRINT_ERROR(
		    "pgaspi_dev_read GASPI_QUEUE_FULL is: %d max: %d",
		    gctx->ne_count_c[queue],
		    gctx->config->queue_size_max);
		return GASPI_QUEUE_FULL;
	}

	ret = PtlGet(dev->comm_notif_md_h[queue],
	             local_offset,
	             size,
	             dev->remote_info[rank].phys_address,
	             dev->data_pt_idx,
	             0,
	             remote_offset,
	             NULL);

	if (ret != PTL_OK) {
		GASPI_DEBUG_PRINT_ERROR("PtlGet failed with %d for size %d", ret, size);
		return GASPI_ERROR;
	}

	gctx->ne_count_c[queue]++;

	return GASPI_SUCCESS;
}

gaspi_return_t pgaspi_dev_purge(gaspi_context_t* const gctx,
                                const gaspi_queue_id_t queue,
                                const gaspi_timeout_t timeout_ms) {
	int ret;
	unsigned int which;
	ptl_ct_event_t ce;
	gaspi_portals4_ctx* const dev = (gaspi_portals4_ctx*) gctx->device->ctx;
	const ptl_size_t nr =
	    (ptl_size_t) gctx->ne_count_c[queue] + dev->comm_notif_ct_cnt[queue];

	ret = PtlCTPoll(
	    &dev->comm_notif_ct_h[queue], &nr, 1, timeout_ms, &ce, &which);

	if (ret != PTL_OK) {
		GASPI_DEBUG_PRINT_ERROR("PtlCTPoll failed with %d", ret);
		return ret == PTL_CT_NONE_REACHED ? GASPI_TIMEOUT : GASPI_ERROR;
	}

	gctx->ne_count_c[queue] = 0;
	dev->comm_notif_ct_cnt[queue] = ce.success;
	return GASPI_SUCCESS;
}

gaspi_return_t pgaspi_dev_wait(gaspi_context_t* const gctx,
                               const gaspi_queue_id_t queue,
                               const gaspi_timeout_t timeout_ms) {
	int ret;
	ptl_ct_event_t ce;
	unsigned int which;
	gaspi_portals4_ctx* const dev = (gaspi_portals4_ctx*) gctx->device->ctx;
	const ptl_size_t nr =
	    (ptl_size_t) gctx->ne_count_c[queue] + dev->comm_notif_ct_cnt[queue];

	ret = PtlCTPoll(
	    &dev->comm_notif_ct_h[queue], &nr, 1, timeout_ms, &ce, &which);

	if (ret != PTL_OK) {
		GASPI_DEBUG_PRINT_ERROR("PtlCTPoll failed with %d", ret);
		return ret == PTL_CT_NONE_REACHED ? GASPI_TIMEOUT : GASPI_ERROR;
	}

	if (ce.failure > 0) {
		GASPI_DEBUG_PRINT_ERROR("Comm queue %d might be broken!", queue);
		return GASPI_ERROR;
	}
	gctx->ne_count_c[queue] = 0;
	dev->comm_notif_ct_cnt[queue] = ce.success;
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
	gaspi_portals4_ctx* const dev = (gaspi_portals4_ctx*) gctx->device->ctx;

	if (gctx->ne_count_c[queue] + num > gctx->config->queue_size_max) {
		GASPI_DEBUG_PRINT_ERROR(
		    "pgaspi_dev_write_list GASPI_QUEUE_FULL is: %d max: %d",
		    gctx->ne_count_c[queue],
		    gctx->config->queue_size_max);
		return GASPI_QUEUE_FULL;
	}

	ret = PtlStartBundle(dev->ni_h);
	if (ret != PTL_OK) {
		GASPI_DEBUG_PRINT_ERROR("PtlBundleStart failed with %d", ret);
		return GASPI_ERROR;
	}

	for (gaspi_number_t i = 0; i < num; ++i) {
		ret = pgaspi_dev_write(gctx,
		                       segment_id_local[i],
		                       offset_local[i],
		                       rank,
		                       segment_id_remote[i],
		                       offset_remote[i],
		                       size[i],
		                       queue);
		if (ret != GASPI_SUCCESS) {
			return ret;
		}
	}

	ret = PtlEndBundle(dev->ni_h);
	if (ret != PTL_OK) {
		GASPI_DEBUG_PRINT_ERROR("PtlEndBundle failed with %d", ret);
		return GASPI_ERROR;
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
	gaspi_portals4_ctx* const dev = (gaspi_portals4_ctx*) gctx->device->ctx;

	if (gctx->ne_count_c[queue] + num > gctx->config->queue_size_max) {
		GASPI_DEBUG_PRINT_ERROR(
		    "pgaspi_dev_read_list GASPI_QUEUE_FULL is: %d max: %d",
		    gctx->ne_count_c[queue],
		    gctx->config->queue_size_max);
		return GASPI_QUEUE_FULL;
	}

	ret = PtlStartBundle(dev->ni_h);
	if (ret != PTL_OK) {
		GASPI_DEBUG_PRINT_ERROR("PtlBundleStart failed with %d", ret);
		return GASPI_ERROR;
	}

	for (gaspi_number_t i = 0; i < num; ++i) {
		ret = pgaspi_dev_read(gctx,
		                      segment_id_local[i],
		                      offset_local[i],
		                      rank,
		                      segment_id_remote[i],
		                      offset_remote[i],
		                      size[i],
		                      queue);
		if (ret != GASPI_SUCCESS) {
			return ret;
		}
	}

	ret = PtlEndBundle(dev->ni_h);
	if (ret != PTL_OK) {
		GASPI_DEBUG_PRINT_ERROR("PtlEndBundle failed with %d", ret);
		return GASPI_ERROR;
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
	gaspi_portals4_ctx* const dev = gctx->device->ctx;
	((gaspi_notification_t*) gctx->nsrc.notif_spc.buf)[notification_id] =
	    notification_value;
	const ptl_size_t local_offset =
	    gctx->nsrc.notif_spc.addr +
	    notification_id * sizeof(gaspi_notification_t);
	const ptl_size_t remote_offset =
	    gctx->rrmd[segment_id_remote][rank].notif_spc.addr +
	    notification_id * sizeof(gaspi_notification_t);

	if (gctx->ne_count_c[queue] == gctx->config->queue_size_max) {
		GASPI_DEBUG_PRINT_ERROR("pgaspi_dev_ GASPI_QUEUE_FULL is: %d max: %d",
		                        gctx->ne_count_c[queue],
		                        gctx->config->queue_size_max);
		return GASPI_QUEUE_FULL;
	}

	ret = PtlPut(dev->comm_notif_md_h[queue],
	             local_offset,
	             sizeof(gaspi_notification_t),
	             PORTALS4_ACK_TYPE,
	             dev->remote_info[rank].phys_address,
	             dev->data_pt_idx,
	             0,
	             remote_offset,
	             NULL,
	             0);

	if (ret != PTL_OK) {
		GASPI_DEBUG_PRINT_ERROR("PtlPut failed with %d", ret);
		return GASPI_ERROR;
	}

	gctx->ne_count_c[queue]++;

	return GASPI_SUCCESS;
}

gaspi_return_t pgaspi_dev_write_notify(
    gaspi_context_t* const gctx,
    const gaspi_segment_id_t segment_id_local,
    const gaspi_offset_t offset_local,
    const gaspi_rank_t rank,
    const gaspi_segment_id_t segment_id_remote,
    const gaspi_offset_t offset_remote,
    const gaspi_size_t size,
    const gaspi_notification_id_t notification_id,
    const gaspi_notification_t notification_value,
    const gaspi_queue_id_t queue) {
	int ret;

	if (gctx->ne_count_c[queue] + 2 > gctx->config->queue_size_max) {
		GASPI_DEBUG_PRINT_ERROR(
		    "pgaspi_dev_write_notify GASPI_QUEUE_FULL is: %d max: %d",
		    gctx->ne_count_c[queue],
		    gctx->config->queue_size_max);
		return GASPI_QUEUE_FULL;
	}

	ret = pgaspi_dev_write(gctx,
	                       segment_id_local,
	                       offset_local,
	                       rank,
	                       segment_id_remote,
	                       offset_remote,
	                       size,
	                       queue);
	if (ret != GASPI_SUCCESS) {
		return ret;
	}

	ret = pgaspi_dev_notify(gctx,
	                        segment_id_remote,
	                        rank,
	                        notification_id,
	                        notification_value,
	                        queue);
	if (ret != GASPI_SUCCESS) {
		return ret;
	}

	return GASPI_SUCCESS;
}

gaspi_return_t pgaspi_dev_write_list_notify(
    gaspi_context_t* const gctx,
    const gaspi_number_t num,
    gaspi_segment_id_t* const segment_id_local,
    gaspi_offset_t* const offset_local,
    const gaspi_rank_t rank,
    gaspi_segment_id_t* const segment_id_remote,
    gaspi_offset_t* const offset_remote,
    gaspi_size_t* const size,
    const gaspi_segment_id_t segment_id_notification,
    const gaspi_notification_id_t notification_id,
    const gaspi_notification_t notification_value,
    const gaspi_queue_id_t queue) {
	int ret;

	if (gctx->ne_count_c[queue] + num + 1 > gctx->config->queue_size_max) {
		GASPI_DEBUG_PRINT_ERROR(
		    "pgaspi_dev_write_list_notify GASPI_QUEUE_FULL is: %d max: %d",
		    gctx->ne_count_c[queue],
		    gctx->config->queue_size_max);
		return GASPI_QUEUE_FULL;
	}

	ret = pgaspi_dev_write_list(gctx,
	                            num,
	                            segment_id_local,
	                            offset_local,
	                            rank,
	                            segment_id_remote,
	                            offset_remote,
	                            size,
	                            queue);
	if (ret != GASPI_SUCCESS) {
		return ret;
	}

	ret = pgaspi_dev_notify(gctx,
	                        segment_id_notification,
	                        rank,
	                        notification_id,
	                        notification_value,
	                        queue);
	if (ret != GASPI_SUCCESS) {
		return ret;
	}

	return GASPI_SUCCESS;
}

gaspi_return_t pgaspi_dev_read_notify(
    gaspi_context_t* const gctx,
    const gaspi_segment_id_t segment_id_local,
    const gaspi_offset_t offset_local,
    const gaspi_rank_t rank,
    const gaspi_segment_id_t segment_id_remote,
    const gaspi_offset_t offset_remote,
    const gaspi_size_t size,
    const gaspi_notification_id_t notification_id,
    const gaspi_queue_id_t queue) {
	int ret;
	gaspi_portals4_ctx* const dev = gctx->device->ctx;

	if (gctx->ne_count_c[queue] + 2 > gctx->config->queue_size_max) {
		GASPI_DEBUG_PRINT_ERROR(
		    "pgaspi_dev_read_notify GASPI_QUEUE_FULL is: %d max: %d",
		    gctx->ne_count_c[queue],
		    gctx->config->queue_size_max);
		return GASPI_QUEUE_FULL;
	}

	ret = pgaspi_dev_read(gctx,
	                      segment_id_local,
	                      offset_local,
	                      rank,
	                      segment_id_remote,
	                      offset_remote,
	                      size,
	                      queue);
	if (ret != GASPI_SUCCESS) {
		return ret;
	}

	const ptl_size_t local_offset =
	    gctx->rrmd[segment_id_local][gctx->rank].notif_spc.addr +
	    notification_id * sizeof(gaspi_notification_t);
	const ptl_size_t remote_offset =
	    gctx->rrmd[segment_id_remote][rank].notif_spc.addr +
	    NOTIFICATIONS_SPACE_SIZE - sizeof(gaspi_notification_t);

	ret = PtlGet(dev->comm_notif_md_h[queue],
	             local_offset,
	             sizeof(gaspi_notification_t),
	             dev->remote_info[rank].phys_address,
	             dev->data_pt_idx,
	             0,
	             remote_offset,
	             NULL);

	if (ret != PTL_OK) {
		GASPI_DEBUG_PRINT_ERROR("PtlGet failed with %d", ret);
		return GASPI_ERROR;
	}

	gctx->ne_count_c[queue]++;

	return GASPI_SUCCESS;
}

gaspi_return_t pgaspi_dev_read_list_notify(
    gaspi_context_t* const gctx,
    const gaspi_number_t num,
    gaspi_segment_id_t* const segment_id_local,
    gaspi_offset_t* const offset_local,
    const gaspi_rank_t rank,
    gaspi_segment_id_t* const segment_id_remote,
    gaspi_offset_t* const offset_remote,
    gaspi_size_t* const size,
    const gaspi_segment_id_t segment_id_notification,
    const gaspi_notification_id_t notification_id,
    const gaspi_queue_id_t queue) {
	int ret;
	gaspi_portals4_ctx* const dev = gctx->device->ctx;

	if (gctx->ne_count_c[queue] + num + 1 > gctx->config->queue_size_max) {
		GASPI_DEBUG_PRINT_ERROR(
		    "pgaspi_dev_read_list_notify GASPI_QUEUE_FULL is: %d max: %d",
		    gctx->ne_count_c[queue],
		    gctx->config->queue_size_max);
		return GASPI_QUEUE_FULL;
	}

	ret = pgaspi_dev_read_list(gctx,
	                           num,
	                           segment_id_local,
	                           offset_local,
	                           rank,
	                           segment_id_remote,
	                           offset_remote,
	                           size,
	                           queue);
	if (ret != GASPI_SUCCESS) {
		return ret;
	}

	const ptl_size_t local_offset =
	    gctx->rrmd[segment_id_notification][gctx->rank].notif_spc.addr +
	    notification_id * sizeof(gaspi_notification_t);
	const ptl_size_t remote_offset =
	    gctx->rrmd[segment_id_notification][rank].notif_spc.addr +
	    NOTIFICATIONS_SPACE_SIZE - sizeof(gaspi_notification_t);

	ret = PtlGet(dev->comm_notif_md_h[queue],
	             local_offset,
	             sizeof(gaspi_notification_t),
	             dev->remote_info[rank].phys_address,
	             dev->data_pt_idx,
	             0,
	             remote_offset,
	             NULL);

	if (ret != PTL_OK) {
		GASPI_DEBUG_PRINT_ERROR("PtlGet failed with %d", ret);
		return GASPI_ERROR;
	}

		gctx->ne_count_c[queue]++;
	return GASPI_SUCCESS;
}
