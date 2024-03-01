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

#include <errno.h>
#include <limits.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <sys/timeb.h>
#include <unistd.h>

#include <math.h>

#include "GPI2.h"
#include "GPI2_Dev.h"
#include "GPI2_PORTALS.h"

int pgaspi_dev_init_core(gaspi_context_t* const gctx) {
	int ret, i;
	ptl_ni_limits_t ni_req_limits;
	ptl_ni_limits_t ni_limits;
	ptl_interface_t iface;
	ptl_uid_t uid = PTL_UID_ANY;
	ptl_process_t phys_address;
	gctx->device = calloc(1, sizeof(gctx->device));
	if (gctx->device == NULL) {
		GASPI_DEBUG_PRINT_ERROR("Device allocation failed");
		return GASPI_ERROR;
	}

	gctx->device->ctx = calloc(1, sizeof(gaspi_portals4_ctx));
	if (gctx->device->ctx == NULL) {
		GASPI_DEBUG_PRINT_ERROR("Device ctx allocation failed");
		free(gctx->device);
		return GASPI_ERROR;
	}

	gaspi_portals4_ctx* const portals4_dev_ctx =
	    (gaspi_portals4_ctx*) gctx->device->ctx;

	portals4_dev_ctx->ni_h = PTL_INVALID_HANDLE;
	portals4_dev_ctx->eq_h = PTL_INVALID_HANDLE;
	portals4_dev_ctx->group_atomic_ct_h = PTL_INVALID_HANDLE;
	portals4_dev_ctx->passive_comm_eq_h = PTL_INVALID_HANDLE;
	portals4_dev_ctx->passive_comm_ct_h = PTL_INVALID_HANDLE;
	portals4_dev_ctx->passive_comm_le_h = PTL_INVALID_HANDLE;
	portals4_dev_ctx->passive_comm_pt_idx = PTL_PT_ANY;
	portals4_dev_ctx->data_le_h = PTL_INVALID_HANDLE;
	portals4_dev_ctx->data_pt_idx = PTL_PT_ANY;
	portals4_dev_ctx->local_info = NULL;
	portals4_dev_ctx->remote_info = NULL;
	portals4_dev_ctx->passive_comm_msg_buf = NULL;
	portals4_dev_ctx->group_atomic_ct_cnt = 0;
	portals4_dev_ctx->passive_comm_ct_cnt = 0;

	for (i = 0; i < GASPI_MAX_QP; ++i) {
		portals4_dev_ctx->comm_notif_ct_h[i] = PTL_INVALID_HANDLE;
		portals4_dev_ctx->comm_notif_ct_cnt[i] = 0;
	}

	iface = gctx->config->dev_config.params.portals4.iface;

	ret = PtlInit();

	if (ret != PTL_OK) {
		GASPI_DEBUG_PRINT_ERROR("Failed to initialize Portls library!");
		free(gctx->device->ctx);
		free(gctx->device);
		return GASPI_ERROR;
	}

	// prelim defaults
	ni_req_limits.max_entries = INT_MAX;
	ni_req_limits.max_unexpected_headers = 16319;
	ni_req_limits.max_mds = INT_MAX;
	ni_req_limits.max_eqs = INT_MAX;
	ni_req_limits.max_cts = INT_MAX;
	ni_req_limits.max_pt_index = 255;
	ni_req_limits.max_iovecs = 0;
	ni_req_limits.max_list_size = INT_MAX;
	ni_req_limits.max_triggered_ops = INT_MAX;
	ni_req_limits.max_msg_size = LONG_MAX;
	ni_req_limits.max_atomic_size = LONG_MAX;
	ni_req_limits.max_fetch_atomic_size = LONG_MAX;
	ni_req_limits.max_waw_ordered_size = LONG_MAX;
	ni_req_limits.max_war_ordered_size = LONG_MAX;
	ni_req_limits.max_volatile_size = LONG_MAX;
	ni_req_limits.features = PTL_TARGET_BIND_INACCESSIBLE;

	ret = PtlNIInit(iface,
	                PTL_NI_NO_MATCHING | PTL_NI_PHYSICAL,
	                PTL_PID_ANY,
	                &ni_req_limits,
	                &ni_limits,
	                &portals4_dev_ctx->ni_h);

	if (ret != PTL_OK) {
		GASPI_DEBUG_PRINT_ERROR(
		    "Failed to initialize Network Interface! Code %d on interface %d",
		    ret,
		    iface);
		pgaspi_dev_cleanup_core(gctx);
		return GASPI_ERROR;
	}

	if (ni_limits.max_atomic_size < sizeof(gaspi_atomic_value_t)) {
		GASPI_DEBUG_PRINT_ERROR("Bad atomic size!");
		pgaspi_dev_cleanup_core(gctx);
		return GASPI_ERROR;
	}

	if (ni_limits.max_fetch_atomic_size < sizeof(gaspi_atomic_value_t)) {
		GASPI_DEBUG_PRINT_ERROR("Bad atomic fetch size!");
		pgaspi_dev_cleanup_core(gctx);
		return GASPI_ERROR;
	}

	if (ni_limits.max_msg_size < gctx->config->transfer_size_max) {
		GASPI_DEBUG_PRINT_ERROR("Bad msg size!");
		pgaspi_dev_cleanup_core(gctx);
		return GASPI_ERROR;
	}

	if (ni_limits.features & PTL_TARGET_BIND_INACCESSIBLE == 0) {
		GASPI_DEBUG_PRINT_ERROR(
		    "Interface does not support PTL_TARGET_BIND_INACCESSIBLE feature");
		pgaspi_dev_cleanup_core(gctx);
		return GASPI_ERROR;
	}

	portals4_dev_ctx->local_info = (struct portals4_ctx_info*) calloc(
	    gctx->tnc, sizeof(struct portals4_ctx_info));

	if (portals4_dev_ctx->local_info == NULL) {
		GASPI_DEBUG_PRINT_ERROR("Failed to allocate memory!");
		pgaspi_dev_cleanup_core(gctx);
		return GASPI_ERROR;
	}

	portals4_dev_ctx->remote_info = (struct portals4_ctx_info*) calloc(
	    gctx->tnc, sizeof(struct portals4_ctx_info));

	if (portals4_dev_ctx->remote_info == NULL) {
		GASPI_DEBUG_PRINT_ERROR("Failed to allocate memory!");
		pgaspi_dev_cleanup_core(gctx);
		return GASPI_ERROR;
	}

	ret = PtlGetPhysId(portals4_dev_ctx->ni_h, &phys_address);

	if (ret != PTL_OK) {
		GASPI_DEBUG_PRINT_ERROR("Failed to read physical address from NI!");
		pgaspi_dev_cleanup_core(gctx);
		return GASPI_ERROR;
	}

	for (i = 0; i < gctx->tnc; ++i) {
		portals4_dev_ctx->local_info[i].phys_address = phys_address;
	}

	ret = PtlEQAlloc(
	    portals4_dev_ctx->ni_h, PORTALS4_EVENT_SLOTS, &portals4_dev_ctx->eq_h);

	if (ret != PTL_OK) {
		GASPI_DEBUG_PRINT_ERROR("PtlEQAlloc failed with %d", ret);
		pgaspi_dev_cleanup_core(gctx);
		return GASPI_ERROR;
	}

	ret = PtlEQAlloc(portals4_dev_ctx->ni_h,
	                 PORTALS4_EVENT_SLOTS,
	                 &portals4_dev_ctx->passive_comm_eq_h);

	if (ret != PTL_OK) {
		GASPI_DEBUG_PRINT_ERROR("PtlEQAlloc failed with %d", ret);
		pgaspi_dev_cleanup_core(gctx);
		return GASPI_ERROR;
	}

	ret = PtlPTAlloc(portals4_dev_ctx->ni_h,
	                 0,
	                 portals4_dev_ctx->eq_h,
	                 PORTALS4_DATA_PT_INDEX,
	                 &portals4_dev_ctx->data_pt_idx);

	if (ret != PTL_OK) {
		GASPI_DEBUG_PRINT_ERROR("PtlPTAlloc failed with %d", ret);
		pgaspi_dev_cleanup_core(gctx);
		return GASPI_ERROR;
	}

	ret = PtlPTAlloc(portals4_dev_ctx->ni_h,
	                 0,
	                 portals4_dev_ctx->passive_comm_eq_h,
	                 PORTALS4_PASSIVE_PT_INDEX,
	                 &portals4_dev_ctx->passive_comm_pt_idx);

	if (ret != PTL_OK) {
		GASPI_DEBUG_PRINT_ERROR("PtlPTAlloc failed with %d", ret);
		pgaspi_dev_cleanup_core(gctx);
		return GASPI_ERROR;
	}

	ptl_le_t data_le = {
	    .start = NULL,
	    .length = PTL_SIZE_MAX,
	    .uid = PTL_UID_ANY,
	    .options = PTL_LE_OP_PUT | PTL_LE_OP_GET |
	               PTL_LE_EVENT_SUCCESS_DISABLE | PTL_LE_EVENT_COMM_DISABLE,
	    .ct_handle = PTL_CT_NONE,
	};

	ret = PtlLEAppend(portals4_dev_ctx->ni_h,
	                  portals4_dev_ctx->data_pt_idx,
	                  &data_le,
	                  PTL_PRIORITY_LIST,
	                  NULL,
	                  &portals4_dev_ctx->data_le_h);
	if (ret != PTL_OK) {
		GASPI_DEBUG_PRINT_ERROR("PtlLEAppend failded with %d", ret);
		return GASPI_ERROR;
	}

	long page_size = sysconf(_SC_PAGESIZE);

	if (page_size == -1) {
		return GASPI_ERROR;
	}

	void* ptr = malloc(gctx->config->passive_transfer_size_max);
	if (ptr == NULL) {
		return GASPI_ERR_MEMALLOC;
	}

	ret = posix_memalign(
	    (void**) ptr, page_size, gctx->config->passive_transfer_size_max);
	if (ret != 0) {
		return GASPI_ERROR;
	}

	portals4_dev_ctx->passive_comm_msg_buf = ptr;
	portals4_dev_ctx->passive_comm_msg_buf_addr = (ptl_size_t) ptr;
	portals4_dev_ctx->passive_comm_msg_buf_size =
	    gctx->config->passive_transfer_size_max;
	ptr = NULL;

	ptl_le_t passive_le = {
	    .start = portals4_dev_ctx->passive_comm_msg_buf,
	    .length = portals4_dev_ctx->passive_comm_msg_buf_size,
	    .uid = PTL_UID_ANY,
	    .options = PTL_LE_OP_PUT,
	    .ct_handle = PTL_CT_NONE,
	};

	ret = PtlLEAppend(portals4_dev_ctx->ni_h,
	                  portals4_dev_ctx->passive_comm_pt_idx,
	                  &passive_le,
	                  PTL_PRIORITY_LIST,
	                  NULL,
	                  &portals4_dev_ctx->passive_comm_le_h);
	if (ret != PTL_OK) {
		GASPI_DEBUG_PRINT_ERROR("PtlLEAppend failded with %d", ret);
		return GASPI_ERROR;
	}

	ptl_event_t event;
	ret = PtlEQWait(portals4_dev_ctx->eq_h, &event);
	if (ret != PTL_OK) {
		GASPI_DEBUG_PRINT_ERROR("PtlEQWait failed with %d", ret);
		return GASPI_ERROR;
	}
	if (event.type != PTL_EVENT_LINK) {
		GASPI_DEBUG_PRINT_ERROR("Event type does not match");
		return GASPI_ERROR;
	}
	if (event.ni_fail_type != PTL_NI_OK) {
		GASPI_DEBUG_PRINT_ERROR("Linking of LE failed");
		return GASPI_ERROR;
	}

	ret = PtlEQWait(portals4_dev_ctx->passive_comm_eq_h, &event);
	if (ret != PTL_OK) {
		GASPI_DEBUG_PRINT_ERROR("PtlEQWait failed with %d", ret);
		return GASPI_ERROR;
	}
	if (event.type != PTL_EVENT_LINK) {
		GASPI_DEBUG_PRINT_ERROR("Event type does not match");
		return GASPI_ERROR;
	}
	if (event.ni_fail_type != PTL_NI_OK) {
		GASPI_DEBUG_PRINT_ERROR("Linking of LE failed");
		return GASPI_ERROR;
	}

	// Allocate CT Event queues

	for (int i = 0; i < gctx->num_queues; ++i) {
		ret = PtlCTAlloc(portals4_dev_ctx->ni_h,
		                 &portals4_dev_ctx->comm_notif_ct_h[i]);
		if (ret != PTL_OK) {
			GASPI_DEBUG_PRINT_ERROR("PtlCTAlloc failed with %d", ret);
			pgaspi_dev_cleanup_core(gctx);
			return GASPI_ERROR;
		}
	}

	ret = PtlCTAlloc(portals4_dev_ctx->ni_h,
	                 &portals4_dev_ctx->group_atomic_ct_h);

	if (ret != PTL_OK) {
		GASPI_DEBUG_PRINT_ERROR("PtlCTAlloc failed with %d", ret);
		pgaspi_dev_cleanup_core(gctx);
		return GASPI_ERROR;
	}
	ret = PtlCTAlloc(portals4_dev_ctx->ni_h,
	                 &portals4_dev_ctx->passive_comm_ct_h);

	if (ret != PTL_OK) {
		GASPI_DEBUG_PRINT_ERROR("PtlCTAlloc failed with %d", ret);
		pgaspi_dev_cleanup_core(gctx);
		return GASPI_ERROR;
	}

	// Create MDs
	unsigned long md_options = PTL_MD_EVENT_SUCCESS_DISABLE |
	                           PTL_MD_EVENT_CT_REPLY | PTL_MD_EVENT_CT_ACK |
	                           PTL_MD_VOLATILE;
	ptl_md_t md = {
	    .start = NULL,
	    .length = PTL_SIZE_MAX,
	    .options = md_options,
	    .eq_handle = PTL_EQ_NONE,
	};

	for (int i = 0; i < gctx->num_queues; ++i) {
		md.ct_handle = portals4_dev_ctx->comm_notif_ct_h[i];
		ret = PtlMDBind(
		    portals4_dev_ctx->ni_h, &md, &portals4_dev_ctx->comm_notif_md_h[i]);
		if (ret != PTL_OK) {
			GASPI_DEBUG_PRINT_ERROR("PtlMDBind failed with %d", ret);
			return GASPI_ERROR;
		}
	}

	md.ct_handle = portals4_dev_ctx->group_atomic_ct_h;
	ret = PtlMDBind(
	    portals4_dev_ctx->ni_h, &md, &portals4_dev_ctx->group_atomic_md_h);
	if (ret != PTL_OK) {
		GASPI_DEBUG_PRINT_ERROR("PtlMDBind failed with %d", ret);
		return GASPI_ERROR;
	}

	md.ct_handle = portals4_dev_ctx->passive_comm_ct_h;
	ret = PtlMDBind(
	    portals4_dev_ctx->ni_h, &md, &portals4_dev_ctx->passive_comm_md_h);
	if (ret != PTL_OK) {
		GASPI_DEBUG_PRINT_ERROR("PtlMDBind failed with %d", ret);
		return GASPI_ERROR;
	}

	return GASPI_SUCCESS;
}

int pgaspi_dev_comm_queue_delete(gaspi_context_t const* const gctx,
                                 const unsigned int id) {

	gaspi_portals4_ctx* dev = (gaspi_portals4_ctx*) gctx->device->ctx;
	int ret = PTL_OK;
	ret = PtlMDRelease(dev->comm_notif_md_h[id]);
	if (ret != PTL_OK) {
		return GASPI_ERROR;
	}
	dev->comm_notif_md_h[id] = PTL_INVALID_HANDLE;

	ret = PtlCTFree(dev->comm_notif_ct_h[id]);
	if (ret != PTL_OK) {
		return GASPI_ERROR;
	}
	dev->comm_notif_ct_h[id] = PTL_INVALID_HANDLE;
	return GASPI_SUCCESS;
}

int pgaspi_dev_comm_queue_create(
    const gaspi_context_t* const gctx,
    const unsigned int id,
    const unsigned short GASPI_UNUSED(remote_node)) {
	gaspi_portals4_ctx* dev = (gaspi_portals4_ctx*) gctx->device->ctx;
	int ret = PTL_OK;

	ret = PtlCTAlloc(dev->ni_h, &dev->comm_notif_ct_h[id]);
	if (ret != PTL_OK) {
		return GASPI_ERROR;
	}
	ptl_md_t md = {
	    .start = NULL,
	    .length = PTL_SIZE_MAX,
	    .options = PTL_MD_EVENT_SUCCESS_DISABLE | PTL_MD_EVENT_CT_REPLY |
	               PTL_MD_EVENT_CT_ACK | PTL_MD_VOLATILE,
	    .eq_handle = PTL_EQ_NONE,
	    .ct_handle = dev->comm_notif_ct_h[id],
	};

	ret = PtlMDBind(dev->ni_h, &md, &dev->comm_notif_md_h[id]);
	if (ret != PTL_OK) {
		return GASPI_ERROR;
	}

	return GASPI_SUCCESS;
}

int pgaspi_dev_comm_queue_is_valid(gaspi_context_t const* const gctx,
                                   const unsigned int id) {
	gaspi_portals4_ctx* const dev = (gaspi_portals4_ctx*) gctx->device->ctx;

	if (PtlHandleIsEqual(dev->comm_notif_ct_h[id], PTL_INVALID_HANDLE)) {
		return GASPI_ERR_INV_QUEUE;
	}
	return GASPI_SUCCESS;
}

int pgaspi_dev_create_endpoint(gaspi_context_t const* const gctx,
                               const int i,
                               void** info,
                               void** remote_info,
                               size_t* info_size) {
	gaspi_portals4_ctx* const portals4_dev_ctx =
	    (gaspi_portals4_ctx*) gctx->device->ctx;
	*info = &portals4_dev_ctx->local_info[i];
	*remote_info = &portals4_dev_ctx->remote_info[i];
	*info_size = sizeof(struct portals4_ctx_info);
	return GASPI_SUCCESS;
}

int pgaspi_dev_disconnect_context(gaspi_context_t* const GASPI_UNUSED(gctx),
                                  const int GASPI_UNUSED(i)) {
	return GASPI_SUCCESS;
}

int pgaspi_dev_comm_queue_connect(
    gaspi_context_t const* const GASPI_UNUSED(gctx),
    const unsigned short GASPI_UNUSED(q),
    const int GASPI_UNUSED(i)) {
	return GASPI_SUCCESS;
}

int pgaspi_dev_connect_context(gaspi_context_t const* const gctx,
                               const int GASPI_UNUSED(i)) {
	return GASPI_SUCCESS;
}

int pgaspi_dev_cleanup_core(gaspi_context_t* const gctx) {
	int ret;
	gaspi_portals4_ctx* dev = (gaspi_portals4_ctx*) gctx->device->ctx;

	if (!PtlHandleIsEqual(dev->group_atomic_ct_h, PTL_INVALID_HANDLE)) {
		ret = PtlCTFree(dev->group_atomic_ct_h);
		if (ret != PTL_OK) {
			GASPI_DEBUG_PRINT_ERROR("PtlCTFree failed with %d", ret);
			return GASPI_ERROR;
		}
	}

	for (int i = 0; i < GASPI_MAX_QP; ++i) {
		if (!PtlHandleIsEqual(dev->comm_notif_ct_h[i], PTL_INVALID_HANDLE)) {
			ret = PtlCTFree(dev->comm_notif_ct_h[i]);
			if (ret != PTL_OK) {
				GASPI_DEBUG_PRINT_ERROR("PtlCTFree failed with %d", ret);
				return GASPI_ERROR;
			}
		}
	}

	if (!PtlHandleIsEqual(dev->data_le_h, PTL_INVALID_HANDLE)) {
		ret = PtlLEUnlink(dev->data_le_h);
		if (ret != PTL_OK) {
			GASPI_DEBUG_PRINT_ERROR("PtlLEUnlink failed with %d", ret);
			return GASPI_ERROR;
		}
	}

	if (!PtlHandleIsEqual(dev->passive_comm_le_h, PTL_INVALID_HANDLE)) {
		ret = PtlLEUnlink(dev->passive_comm_le_h);
		if (ret != PTL_OK) {
			GASPI_DEBUG_PRINT_ERROR("PtlLEUnlink failed with %d", ret);
			return GASPI_ERROR;
		}
	}

	if (!PtlHandleIsEqual(dev->passive_comm_ct_h, PTL_INVALID_HANDLE)) {
		ret = PtlCTFree(dev->passive_comm_ct_h);
		if (ret != PTL_OK) {
			GASPI_DEBUG_PRINT_ERROR("PtlCTFree failed with %d", ret);
			return GASPI_ERROR;
		}
	}

	if (!PtlHandleIsEqual(dev->eq_h, PTL_INVALID_HANDLE)) {
		ret = PtlEQFree(dev->eq_h);
		if (ret != PTL_OK) {
			GASPI_DEBUG_PRINT_ERROR("PtlEQFree failed with %d", ret);
			return GASPI_ERROR;
		}
	}

	if (!PtlHandleIsEqual(dev->passive_comm_eq_h, PTL_INVALID_HANDLE)) {
		ret = PtlEQFree(dev->passive_comm_eq_h);
		if (ret != PTL_OK) {
			GASPI_DEBUG_PRINT_ERROR("PtlEQFree failed with %d", ret);
			return GASPI_ERROR;
		}
	}

	if (!dev->passive_comm_pt_idx != PTL_PT_ANY) {
		ret = PtlPTFree(dev->ni_h, dev->passive_comm_pt_idx);
		if (ret != PTL_OK) {
			GASPI_DEBUG_PRINT_ERROR("PtlPTFree failed with %d", ret);
			return GASPI_ERROR;
		}
	}

	if (!dev->data_pt_idx != PTL_PT_ANY) {
		ret = PtlPTFree(dev->ni_h, dev->data_pt_idx);
		if (ret != PTL_OK) {
			GASPI_DEBUG_PRINT_ERROR("PtlPTFree failed with %d", ret);
			return GASPI_ERROR;
		}
	}

	if (!PtlHandleIsEqual(dev->ni_h, PTL_INVALID_HANDLE)) {
		ret = PtlNIFini(dev->ni_h);
		if (ret != PTL_OK) {
			GASPI_DEBUG_PRINT_ERROR("PtlNIFini failed with %d", ret);
			return GASPI_ERROR;
		}
	}

	if (dev->passive_comm_msg_buf != NULL) {
		free(dev->passive_comm_msg_buf);
		dev->passive_comm_msg_buf = NULL;
	}

	if (dev->remote_info != NULL) {
		free(dev->remote_info);
		dev->remote_info = NULL;
	}

	if (dev->local_info != NULL) {
		free(dev->local_info);
		dev->local_info = NULL;
	}

	if (gctx->device->ctx != NULL) {
		free(gctx->device->ctx);
		gctx->device->ctx = NULL;
	}

	if (gctx->device != NULL) {
		free(gctx->device);
		gctx->device = NULL;
	}

	PtlFini();
	return GASPI_SUCCESS;
}
