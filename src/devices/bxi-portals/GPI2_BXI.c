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
#include "GPI2_BXI.h"
#include "GPI2_Dev.h"

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

	iface = gctx->config->dev_config.params.bxi.iface;

	ret = PtlInit();

	if (PTL_OK != ret) {
		GASPI_DEBUG_PRINT_ERROR("Failed to initialize Portls library!");
		free(gctx->device->ctx);
		free(gctx->device);
		return -1;
	}

	// prelim defaults
	ni_req_limits.max_entries = 1024;
	ni_req_limits.max_unexpected_headers = 1024;
	ni_req_limits.max_mds = 1024;
	ni_req_limits.max_eqs = 1024;
	ni_req_limits.max_cts = 1024;
	ni_req_limits.max_pt_index = 64;
	ni_req_limits.max_iovecs = 1024;
	ni_req_limits.max_list_size = 1024;
	ni_req_limits.max_triggered_ops = 1024;
	ni_req_limits.max_msg_size = LONG_MAX;
	ni_req_limits.max_atomic_size = LONG_MAX;
	ni_req_limits.max_fetch_atomic_size = LONG_MAX;
	ni_req_limits.max_waw_ordered_size = LONG_MAX;
	ni_req_limits.max_war_ordered_size = LONG_MAX;
	ni_req_limits.max_volatile_size = LONG_MAX;
	ni_req_limits.features = 0; // PTL_TOTAL_DATA_ORDERING ...

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
		goto err_d;
	}

	portals4_dev_ctx->local_info = (struct portals4_ctx_info*) calloc(
	    gctx->tnc, sizeof(struct portals4_ctx_info));

	if (portals4_dev_ctx->local_info == NULL) {
		GASPI_DEBUG_PRINT_ERROR("Failed to allocate memory!");
		goto err_l;
	}

	portals4_dev_ctx->remote_info = (struct portals4_ctx_info*) calloc(
	    gctx->tnc, sizeof(struct portals4_ctx_info));

	if (portals4_dev_ctx->remote_info == NULL) {
		GASPI_DEBUG_PRINT_ERROR("Failed to allocate memory!");
		goto err_r;
	}

	ret = PtlGetPhysId(portals4_dev_ctx->ni_handle, &phys_address);

	if (PTL_OK != ret) {
		GASPI_DEBUG_PRINT_ERROR("Failed to read physical address from NI!");
		goto err_r;
	}

	for (i = 0; i < gctx->tnc; ++i) {
		portals4_dev_ctx->local_info[i].phys_address = phys_address;
	}

	ret = PtlEQAlloc(portals4_dev_ctx->ni_handle,
	                 GPI2_PORTALS4_EVENT_SLOTS,
	                 &portals4_dev_ctx->data_eq_handle);

	if (PTL_OK != ret) {
		GASPI_DEBUG_PRINT_ERROR("PtlEQAlloc failed with %d", ret);
		goto err_eq2;
	}

	ret = PtlEQAlloc(portals4_dev_ctx->ni_handle,
	                 GPI2_PORTALS4_EVENT_SLOTS,
	                 &portals4_dev_ctx->notify_spc_eq_handle);

	if (PTL_OK != ret) {
		GASPI_DEBUG_PRINT_ERROR("PtlEQAlloc failed with %d", ret);
		goto err_eq1;
	}

	ret = PtlPTAlloc(portals4_dev_ctx->ni_handle,
	                 0,
	                 portals4_dev_ctx->data_eq_handle,
	                 GPI2_PORTALS4_DATA_PT,
	                 &portals4_dev_ctx->data_pt_index);

	if (PTL_OK != ret) {
		GASPI_DEBUG_PRINT_ERROR("PtlPTAlloc failed with %d", ret);
		goto err_pt2;
	}
	PORTALS4_DEBUG_PRINT_MSG("Data PT index is %d",
	                         portals4_dev_ctx->data_pt_index);
	ret = PtlPTAlloc(portals4_dev_ctx->ni_handle,
	                 0,
	                 portals4_dev_ctx->notify_spc_eq_handle,
	                 GPI2_PORTALS4_NOTIFY_SPC_PT,
	                 &portals4_dev_ctx->notify_spc_pt_index);

	if (PTL_OK != ret) {
		GASPI_DEBUG_PRINT_ERROR("PtlPTAlloc failed with %d", ret);
		goto err_pt1;
	}

	PORTALS4_DEBUG_PRINT_MSG("Notify SPC PT index is %d",
	                         portals4_dev_ctx->notify_spc_pt_index);

	ret = PtlCTAlloc(portals4_dev_ctx->ni_handle,
	                 &portals4_dev_ctx->data_ct_handle);

	if (PTL_OK != ret) {
		GASPI_DEBUG_PRINT_ERROR("PtlPTAlloc failed with %d", ret);
		goto err_ct2;
	}

	ret = PtlCTAlloc(portals4_dev_ctx->ni_handle,
	                 &portals4_dev_ctx->notify_spc_ct_handle);

	if (PTL_OK != ret) {
		GASPI_DEBUG_PRINT_ERROR("PtlPTAlloc failed with %d", ret);
		goto err_ct1;
	}
	return 0;

err_ct1:
	PtlCTFree(portals4_dev_ctx->notify_spc_ct_handle);
err_ct2:
	PtlCTFree(portals4_dev_ctx->data_ct_handle);
err_pt1:
	PtlPTFree(portals4_dev_ctx->ni_handle,
	          portals4_dev_ctx->notify_spc_pt_index);
err_pt2:
	PtlPTFree(portals4_dev_ctx->ni_handle, portals4_dev_ctx->data_pt_index);
err_eq1:
	PtlEQFree(portals4_dev_ctx->notify_spc_eq_handle);
err_eq2:
	PtlEQFree(portals4_dev_ctx->data_eq_handle);
	PtlNIFini(portals4_dev_ctx->ni_handle);
err_r:
	free(portals4_dev_ctx->remote_info);
err_l:
	free(portals4_dev_ctx->local_info);
err_d:
	free(gctx->device->ctx);
	free(gctx->device);
	PtlFini();
	return -1;
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

int pgaspi_dev_connect_context(gaspi_context_t const* const GASPI_UNUSED(gctx),
                               const int GASPI_UNUSED(i)) {
	return 0;
}

int pgaspi_dev_cleanup_core(gaspi_context_t* const gctx) {
	gaspi_portals4_ctx* const portals4_dev_ctx =
	    (gaspi_portals4_ctx*) gctx->device->ctx;
	PtlPTFree(portals4_dev_ctx->ni_handle,
	          portals4_dev_ctx->notify_spc_pt_index);
	PtlPTFree(portals4_dev_ctx->ni_handle, portals4_dev_ctx->data_pt_index);
	PtlEQFree(portals4_dev_ctx->notify_spc_eq_handle);
	PtlEQFree(portals4_dev_ctx->data_eq_handle);
	PtlNIFini(portals4_dev_ctx->ni_handle);
	free(portals4_dev_ctx->remote_info);
	free(portals4_dev_ctx->local_info);
	free(gctx->device->ctx);
	free(gctx->device);
	PtlFini();
	return 0;
}
