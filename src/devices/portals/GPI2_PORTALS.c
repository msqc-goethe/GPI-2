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

int _pgaspi_dev_cleanup_core(gaspi_portals4_ctx* const dev, int tnc) {
	int ret, i;

	if (!PtlHandleIsEqual(dev->group_ct_handle, PTL_INVALID_HANDLE)) {
		ret = PtlCTFree(dev->group_ct_handle);
		if (PTL_OK != ret) {
			GASPI_DEBUG_PRINT_ERROR("PtlCTFree failed with %d", ret);
			return -1;
		}
	}

	for (i = 0; i < GASPI_MAX_QP; ++i) {
		if (!PtlHandleIsEqual(dev->comm_ct_handle[i], PTL_INVALID_HANDLE)) {
			ret = PtlCTFree(dev->comm_ct_handle[i]);
			if (PTL_OK != ret) {
				GASPI_DEBUG_PRINT_ERROR("PtlCTFree failed with %d", ret);
				return -1;
			}
		}
	}

	if (!PtlHandleIsEqual(dev->eq_handle, PTL_INVALID_HANDLE)) {
		ret = PtlEQFree(dev->eq_handle);
		if (PTL_OK != ret) {
			GASPI_DEBUG_PRINT_ERROR("PtlEQFree failed with %d", ret);
			return -1;
		}
	}

	if (!PtlHandleIsEqual(dev->passive_comm.le_handle, PTL_INVALID_HANDLE)) {
		ret = PtlLEUnlink(dev->passive_comm.le_handle);
		if (PTL_OK != ret) {
			GASPI_DEBUG_PRINT_ERROR("PtlLEUnlink failed with %d", ret);
			return -1;
		}
	}
	if (dev->pte_states[0] == PT_ALLOCATED) {
		ret = PtlPTFree(dev->ni_handle, dev->passive_comm.pt_index);
		if (PTL_OK != ret) {
			GASPI_DEBUG_PRINT_ERROR("PtlPTFree failed with %d", ret);
			return -1;
		}
		dev->pte_states[0] = PT_FREE;
	}

	if (!PtlHandleIsEqual(dev->passive_comm.ct_handle, PTL_INVALID_HANDLE)) {
		ret = PtlCTFree(dev->passive_comm.ct_handle);
		if (PTL_OK != ret) {
			GASPI_DEBUG_PRINT_ERROR("PtlCTFree failed with %d", ret);
			return -1;
		}
	}

	if (!PtlHandleIsEqual(dev->passive_comm.eq_handle, PTL_INVALID_HANDLE)) {
		ret = PtlEQFree(dev->passive_comm.eq_handle);
		if (PTL_OK != ret) {
			GASPI_DEBUG_PRINT_ERROR("PtlEQFree failed with %d", ret);
			return -1;
		}
	}

	if (!PtlHandleIsEqual(dev->ni_handle, PTL_INVALID_HANDLE)) {
		ret = PtlNIFini(dev->ni_handle);
		if (PTL_OK != ret) {
			GASPI_DEBUG_PRINT_ERROR("PtlNIFini failed with %d", ret);
			return -1;
		}
	}

	return 0;
}

int _compare_ptl_ni_limits(ptl_ni_limits_t* const lhs,
                           ptl_ni_limits_t* const rhs) {
	if (lhs->max_entries != rhs->max_entries) {
		GASPI_DEBUG_PRINT_ERROR("max_entries mismatch! lhs: %d rhs: %d",
		                        lhs->max_entries,
		                        rhs->max_entries);
		return 0;
	}
	if (lhs->max_unexpected_headers != rhs->max_unexpected_headers) {
		GASPI_DEBUG_PRINT_ERROR(
		    "max_unexpected_headers mismatch! lhs: %d rhs: %d",
		    lhs->max_unexpected_headers,
		    rhs->max_unexpected_headers);
		return 0;
	}
	if (lhs->max_mds != rhs->max_mds) {
		GASPI_DEBUG_PRINT_ERROR(
		    "max_mds mismatch! lhs: %d rhs: %d", lhs->max_mds, rhs->max_mds);
		return 0;
	}
	if (lhs->max_eqs != rhs->max_eqs) {
		GASPI_DEBUG_PRINT_ERROR(
		    "max_eqs mismatch! lhs: %d rhs: %d", lhs->max_eqs, rhs->max_eqs);
		return 0;
	}
	if (lhs->max_cts != rhs->max_cts) {
		GASPI_DEBUG_PRINT_ERROR(
		    "max_cts mismatch! lhs: %d rhs: %d", lhs->max_cts, rhs->max_cts);
		return 0;
	}
	if (lhs->max_pt_index != rhs->max_pt_index) {
		GASPI_DEBUG_PRINT_ERROR("max_pt_index mismatch! lhs: %d rhs: %d",
		                        lhs->max_pt_index,
		                        rhs->max_pt_index);
		return 0;
	}
	if (lhs->max_iovecs != rhs->max_iovecs) {
		GASPI_DEBUG_PRINT_ERROR("max_iovec mismatch! lhs: %d rhs: %d",
		                        lhs->max_iovecs,
		                        rhs->max_iovecs);
		return 0;
	}
	if (lhs->max_list_size != rhs->max_list_size) {
		GASPI_DEBUG_PRINT_ERROR("max_list_size mismatch! lhs: %d rhs: %d",
		                        lhs->max_list_size,
		                        rhs->max_list_size);
		return 0;
	}
	if (lhs->max_triggered_ops != rhs->max_triggered_ops) {
		GASPI_DEBUG_PRINT_ERROR(
		    "max_triggered_ops mismatch! lhs: %d rhs: %d",
		    lhs->max_triggered_ops != rhs->max_triggered_ops);
		return 0;
	}
	if (lhs->max_msg_size != rhs->max_msg_size) {
		GASPI_DEBUG_PRINT_ERROR("max_msg_size mismatch! lhs: %d rhs: %d",
		                        lhs->max_msg_size,
		                        rhs->max_msg_size);
		return 0;
	}
	if (lhs->max_atomic_size != rhs->max_atomic_size) {
		GASPI_DEBUG_PRINT_ERROR("max_atomic_size mismatch! lhs: %d rhs: %d",
		                        lhs->max_atomic_size,
		                        rhs->max_atomic_size);
		return 0;
	}
	if (lhs->max_fetch_atomic_size != rhs->max_fetch_atomic_size) {
		GASPI_DEBUG_PRINT_ERROR(
		    "max_fetch_atomic_size mismatch! lhs: %d rhs: %d",
		    lhs->max_fetch_atomic_size,
		    rhs->max_fetch_atomic_size);
		return 0;
	}
	if (lhs->max_waw_ordered_size != rhs->max_waw_ordered_size) {
		GASPI_DEBUG_PRINT_ERROR(
		    "max_waw_ordered_size mismatch! lhs: %d rhs: %d",
		    lhs->max_waw_ordered_size,
		    rhs->max_waw_ordered_size);
		return 0;
	}
	if (lhs->max_war_ordered_size != rhs->max_war_ordered_size) {
		GASPI_DEBUG_PRINT_ERROR(
		    "max_war_ordered_size mismatch! lhs: %d rhs: %d",
		    lhs->max_war_ordered_size,
		    rhs->max_war_ordered_size);
		return 0;
	}
	if (lhs->max_volatile_size != rhs->max_volatile_size) {
		GASPI_DEBUG_PRINT_ERROR("max_volatile_size! lhs: %d rhs: %d",
		                        lhs->max_volatile_size,
		                        rhs->max_volatile_size);
		return 0;
	}
	if (lhs->features != rhs->features) {
		GASPI_DEBUG_PRINT_ERROR(
		    "features! lhs: %d rhs: %d", lhs->features, rhs->features);
		return 0;
	}
	return 1;
}

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
		return -1;
	}

	gctx->device->ctx = calloc(1, sizeof(gaspi_portals4_ctx));
	if (gctx->device->ctx == NULL) {
		GASPI_DEBUG_PRINT_ERROR("Device ctx allocation failed");
		free(gctx->device);
		return -1;
	}

	gaspi_portals4_ctx* const portals4_dev_ctx =
	    (gaspi_portals4_ctx*) gctx->device->ctx;

	portals4_dev_ctx->ni_handle = PTL_INVALID_HANDLE;
	portals4_dev_ctx->eq_handle = PTL_INVALID_HANDLE;
	portals4_dev_ctx->group_ct_handle = PTL_INVALID_HANDLE;
	portals4_dev_ctx->passive_comm.eq_handle = PTL_INVALID_HANDLE;
	portals4_dev_ctx->passive_comm.ct_handle = PTL_INVALID_HANDLE;
	portals4_dev_ctx->passive_comm.le_handle = PTL_INVALID_HANDLE;
	portals4_dev_ctx->passive_comm.pt_index = PTL_PT_ANY;
	portals4_dev_ctx->passive_comm.msg_buf = NULL;
	portals4_dev_ctx->local_info = NULL;
	portals4_dev_ctx->remote_info = NULL;
	portals4_dev_ctx->pte_states = NULL;
	portals4_dev_ctx->max_ptes = 0;
	portals4_dev_ctx->group_ct_cnt = 0;
	portals4_dev_ctx->passive_comm.ct_cnt = 0;

	for (i = 0; i < GASPI_MAX_QP; ++i) {
		portals4_dev_ctx->comm_ct_handle[i] = PTL_INVALID_HANDLE;
		portals4_dev_ctx->comm_ct_cnt[i] = 0;
	}

	iface = gctx->config->dev_config.params.portals4.iface;

	ret = PtlInit();

	if (PTL_OK != ret) {
		GASPI_DEBUG_PRINT_ERROR("Failed to initialize Portls library!");
		free(gctx->device->ctx);
		free(gctx->device);
		return -1;
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
	ni_req_limits.features = 0;
	//ni_req_limits.features = PTL_TOTAL_DATA_ORDERING; // PTL_TOTAL_DATA_ORDERING ...

	ret = PtlNIInit(iface,
	                PTL_NI_NO_MATCHING | PTL_NI_PHYSICAL,
	                PTL_PID_ANY,
	                &ni_req_limits,
	                &ni_limits,
	                &portals4_dev_ctx->ni_handle);

	if (ret != PTL_OK) {
		GASPI_DEBUG_PRINT_ERROR(
		    "Failed to initialize Network Interface! Code %d on interface %d",
		    ret,
		    iface);
		pgaspi_dev_cleanup_core(gctx);
		return -1;
	}

	if (ni_limits.max_atomic_size < sizeof(gaspi_atomic_value_t)) {
		GASPI_DEBUG_PRINT_ERROR("Bad atomic size!");
		return -1;
	}

	if (ni_limits.max_fetch_atomic_size < sizeof(gaspi_atomic_value_t)) {
		GASPI_DEBUG_PRINT_ERROR("Bad atomic fetch size!");
		return -1;
	}

	if (ni_limits.max_msg_size < gctx->config->transfer_size_max) {
		GASPI_DEBUG_PRINT_ERROR("Bad msg size!");
		return -1;
	}

	portals4_dev_ctx->max_ptes = ni_limits.max_pt_index;
	portals4_dev_ctx->pte_states =
	    (int8_t*) calloc(ni_limits.max_pt_index, sizeof(int8_t));

	if (portals4_dev_ctx->pte_states == NULL) {
		GASPI_DEBUG_PRINT_ERROR("Failed to allocate memory!");
		pgaspi_dev_cleanup_core(gctx);
		return -1;
	}

	portals4_dev_ctx->local_info = (struct portals4_ctx_info*) calloc(
	    gctx->tnc, sizeof(struct portals4_ctx_info));

	if (portals4_dev_ctx->local_info == NULL) {
		GASPI_DEBUG_PRINT_ERROR("Failed to allocate memory!");
		pgaspi_dev_cleanup_core(gctx);
		return -1;
	}

	portals4_dev_ctx->remote_info = (struct portals4_ctx_info*) calloc(
	    gctx->tnc, sizeof(struct portals4_ctx_info));

	if (portals4_dev_ctx->remote_info == NULL) {
		GASPI_DEBUG_PRINT_ERROR("Failed to allocate memory!");
		pgaspi_dev_cleanup_core(gctx);
		return -1;
	}

	ret = PtlGetPhysId(portals4_dev_ctx->ni_handle, &phys_address);

	if (PTL_OK != ret) {
		GASPI_DEBUG_PRINT_ERROR("Failed to read physical address from NI!");
		pgaspi_dev_cleanup_core(gctx);
		return -1;
	}

	for (i = 0; i < gctx->tnc; ++i) {
		portals4_dev_ctx->local_info[i].phys_address = phys_address;
	}

	ret = PtlEQAlloc(portals4_dev_ctx->ni_handle,
	                 PORTALS4_EVENT_SLOTS,
	                 &portals4_dev_ctx->eq_handle);

	if (PTL_OK != ret) {
		GASPI_DEBUG_PRINT_ERROR("PtlEQAlloc failed with %d", ret);
		pgaspi_dev_cleanup_core(gctx);
		return -1;
	}

	ret = PtlEQAlloc(portals4_dev_ctx->ni_handle,
	                 PORTALS4_EVENT_SLOTS,
	                 &portals4_dev_ctx->passive_comm.eq_handle);

	if (PTL_OK != ret) {
		GASPI_DEBUG_PRINT_ERROR("PtlEQAlloc failed with %d", ret);
		pgaspi_dev_cleanup_core(gctx);
		return -1;
	}

	// We assume here that the first PT index is not used at this point
	ret = PtlPTAlloc(portals4_dev_ctx->ni_handle,
	                 0,
	                 portals4_dev_ctx->passive_comm.eq_handle,
	                 0,
	                 &portals4_dev_ctx->passive_comm.pt_index);

	if (PTL_OK != ret) {
		GASPI_DEBUG_PRINT_ERROR("PtlPTAlloc failed with %d", ret);
		pgaspi_dev_cleanup_core(gctx);
		return -1;
	}
	portals4_dev_ctx->pte_states[0] = PT_ALLOCATED;
	portals4_dev_ctx->passive_comm.msg_buf =
	    malloc(gctx->config->passive_transfer_size_max);

	if (portals4_dev_ctx->passive_comm.msg_buf == NULL) {
		GASPI_DEBUG_PRINT_ERROR("Memory allocation failed!");
		return -1;
	}

	ptl_le_t passive_le = {
	    .start = portals4_dev_ctx->passive_comm.msg_buf,
	    .length = gctx->config->passive_transfer_size_max,
	    .uid = PTL_UID_ANY,
	    .options = PTL_LE_OP_PUT,
	    .ct_handle = PTL_CT_NONE,
	};

	ret = PtlLEAppend(portals4_dev_ctx->ni_handle,
	                  portals4_dev_ctx->passive_comm.pt_index,
	                  &passive_le,
	                  PTL_PRIORITY_LIST,
	                  NULL,
	                  &portals4_dev_ctx->passive_comm.le_handle);
	if (PTL_OK != ret) {
		GASPI_DEBUG_PRINT_ERROR("PtlLEAppend failded with %d", ret);
		return GASPI_ERROR;
	}

	ptl_event_t event;
	ret = PtlEQWait(portals4_dev_ctx->passive_comm.eq_handle, &event);
	if (PTL_OK != ret) {
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

	ret = PtlCTAlloc(portals4_dev_ctx->ni_handle,
	                 &portals4_dev_ctx->passive_comm.ct_handle);

	if (PTL_OK != ret) {
		GASPI_DEBUG_PRINT_ERROR("PtlCTAlloc failed with %d", ret);
		pgaspi_dev_cleanup_core(gctx);
		return -1;
	}

	ret = PtlCTAlloc(portals4_dev_ctx->ni_handle,
	                 &portals4_dev_ctx->group_ct_handle);

	if (PTL_OK != ret) {
		GASPI_DEBUG_PRINT_ERROR("PtlCTAlloc failed with %d", ret);
		pgaspi_dev_cleanup_core(gctx);
		return -1;
	}

	for (i = 0; i < GASPI_MAX_QP; ++i) {
		ret = PtlCTAlloc(portals4_dev_ctx->ni_handle,
		                 &portals4_dev_ctx->comm_ct_handle[i]);

		if (PTL_OK != ret) {
			GASPI_DEBUG_PRINT_ERROR("PtlPTAlloc failed with %d", ret);
			pgaspi_dev_cleanup_core(gctx);
			return -1;
		}
	}
	return 0;
}

int pgaspi_dev_comm_queue_delete(
    gaspi_context_t const* const GASPI_UNUSED(gctx),
    const unsigned int GASPI_UNUSED(id)) {
	return 0;
}

int pgaspi_dev_comm_queue_create(
    gaspi_context_t const* const GASPI_UNUSED(gctx),
    const unsigned int GASPI_UNUSED(id),
    const unsigned short GASPI_UNUSED(remote_node)) {
	return 0;
}

int pgaspi_dev_comm_queue_is_valid(
    gaspi_context_t const* const GASPI_UNUSED(gctx),
    const unsigned int GASPI_UNUSED(id)) {
	return 0;
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
	return 0;
}

int pgaspi_dev_disconnect_context(gaspi_context_t* const GASPI_UNUSED(gctx),
                                  const int GASPI_UNUSED(i)) {
	return 0;
}

int pgaspi_dev_comm_queue_connect(
    gaspi_context_t const* const GASPI_UNUSED(gctx),
    const unsigned short GASPI_UNUSED(q),
    const int GASPI_UNUSED(i)) {
	return 0;
}

int pgaspi_dev_connect_context(gaspi_context_t const* const gctx,
                               const int GASPI_UNUSED(i)) {
	return 0;
}

int pgaspi_dev_cleanup_core(gaspi_context_t* const gctx) {
	int ret;
	gaspi_portals4_ctx* const portals4_dev_ctx =
	    (gaspi_portals4_ctx*) gctx->device->ctx;

	ret = _pgaspi_dev_cleanup_core(portals4_dev_ctx, gctx->tnc);
	if (!ret) {
		return ret;
	}
	if (portals4_dev_ctx->remote_info != NULL)
		free(portals4_dev_ctx->remote_info);
	if (portals4_dev_ctx->local_info != NULL)
		free(portals4_dev_ctx->local_info);
	if (portals4_dev_ctx->pte_states != NULL)
		free(portals4_dev_ctx->pte_states);
	if (gctx->device->ctx != NULL)
		free(gctx->device->ctx);
	if (gctx->device != NULL)
		free(gctx->device);
	PtlFini();

	return 0;
}
