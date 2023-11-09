/*
Copyright (c) Goethe University Frankfurt MSQC - Niklas Bartelheimer<bartelheimer@em.uni-frankfurt.de>, 2023-2026

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
#include <sys/mman.h>
#include <sys/time.h>
#include <sys/timeb.h>
#include <unistd.h>

#include "GASPI.h"
#include "GPI2.h"
#include "GPI2_BXI.h"

int _pgaspi_dev_unregister_mem(gaspi_rc_mseg_t* seg){
	int ret;

	if (!PtlHandleIsEqual(seg->data_md_handle, PTL_INVALID_HANDLE)) {
		ret = PtlMDRelease(seg->data_md_handle);
		if (PTL_OK != ret) {
			GASPI_DEBUG_PRINT_ERROR("PtlMDRelease failed with %d", ret);
			return -1;
		}
	}

	if (!PtlHandleIsEqual(seg->notify_spc_md_handle, PTL_INVALID_HANDLE)) {
		ret = PtlMDRelease(seg->notify_spc_md_handle);
		if (PTL_OK != ret) {
			GASPI_DEBUG_PRINT_ERROR("PtlMDRelease failed with %d", ret);
			return -1;
		}
	}

	if (PtlHandleIsEqual(seg->data_le_handle, PTL_INVALID_HANDLE)) {
		ret = PtlLEUnlink(seg->data_le_handle);
		if (PTL_OK != ret) {
			GASPI_DEBUG_PRINT_ERROR("PtlLEUnlink failed with %d", ret);
			return -1;
		}
	}

	if (PtlHandleIsEqual(seg->notify_spc_le_handle, PTL_INVALID_HANDLE)) {
		ret = PtlLEUnlink(seg->notify_spc_le_handle);
		if (PTL_OK != ret) {
			GASPI_DEBUG_PRINT_ERROR("PtlLEUnlink failed with %d", ret);
			return -1;
		}
	}
	return 0;
}

int pgaspi_dev_register_mem(gaspi_context_t const* const gctx,
                            gaspi_rc_mseg_t* seg) {
	int ret;
	ptl_md_t md;
	ptl_le_t le;
	ptl_uid_t uid = PTL_UID_ANY;
	gaspi_portals4_ctx* const portals4_dev_ctx =
	    (gaspi_portals4_ctx*) gctx->device->ctx;

	seg->data_md_handle = PTL_INVALID_HANDLE;
	seg->data_le_handle = PTL_INVALID_HANDLE;
	seg->notify_spc_md_handle = PTL_INVALID_HANDLE;
	seg->notify_spc_le_handle = PTL_INVALID_HANDLE;

	PtlGetUid(portals4_dev_ctx->ni_handle,&uid);

	if (PTL_OK != ret) {
		GASPI_DEBUG_PRINT_ERROR("PtlGetUid failed with %d", ret);
		_pgaspi_dev_unregister_mem(seg);
		return -1;
	}

	if(seg->data.buf == NULL){
	PORTALS4_DEBUG_PRINT_MSG("Allocating Data Segment at with size %d",seg->size);

	memset(&md,0,sizeof(ptl_md_t));
	memset(&le,0,sizeof(ptl_le_t));

	md.start = seg->data.buf;
	md.length = seg->size;
	md.options = PTL_MD_EVENT_SUCCESS_DISABLE | PTL_MD_EVENT_CT_SEND |
	             PTL_MD_EVENT_CT_REPLY | PTL_MD_EVENT_CT_ACK;
	md.eq_handle = portals4_dev_ctx->data_eq_handle;
	md.ct_handle = portals4_dev_ctx->data_ct_handle;

	ret = PtlMDBind(portals4_dev_ctx->ni_handle, &md, &seg->data_md_handle);

	if (PTL_OK != ret) {
		GASPI_DEBUG_PRINT_ERROR("PtlMDBind failed with %d", ret);
		_pgaspi_dev_unregister_mem(seg);
		return -1;
	}

	le.start = seg->data.buf;
	le.length = seg->size;
	le.options = PTL_LE_OP_PUT | PTL_LE_OP_GET | PTL_LE_EVENT_SUCCESS_DISABLE |
	             PTL_LE_EVENT_LINK_DISABLE;
	le.uid = uid;

	ret = PtlLEAppend(portals4_dev_ctx->ni_handle,
	                  portals4_dev_ctx->data_pt_index,
	                  &le,
	                  PTL_PRIORITY_LIST,
	                  NULL,
	                  &seg->data_le_handle);
	if (PTL_OK != ret) {
		GASPI_DEBUG_PRINT_ERROR("PtlLEAppend failed with %d", ret);
		_pgaspi_dev_unregister_mem(seg);
		return -1;
	}
	}
	if (seg->notif_spc.buf != NULL) {
		PORTALS4_DEBUG_PRINT_MSG("Allocating Notify Segment at with size %d",seg->size);

		memset(&md,0,sizeof(ptl_md_t));
		memset(&le,0,sizeof(ptl_le_t));
		
		PORTALS4_DEBUG_PRINT_MSG("UID is %d",uid);

		md.start = seg->notif_spc.buf;
		md.length = seg->notif_spc_size;
		md.options = PTL_MD_EVENT_SUCCESS_DISABLE | PTL_MD_EVENT_CT_SEND |
		             PTL_MD_EVENT_CT_REPLY | PTL_MD_EVENT_CT_ACK;
		md.eq_handle = portals4_dev_ctx->notify_spc_eq_handle;
		md.ct_handle = portals4_dev_ctx->notify_spc_ct_handle;

		ret = PtlMDBind(
		    portals4_dev_ctx->ni_handle, &md, &seg->notify_spc_md_handle);

		if (PTL_OK != ret) {
			GASPI_DEBUG_PRINT_ERROR("PtlMDBind failed with %d", ret);
			_pgaspi_dev_unregister_mem(seg);
			return -1;
		}

		le.start = seg->notif_spc.buf;
		le.length = seg->notif_spc_size;
		le.options = PTL_LE_OP_PUT | PTL_LE_OP_GET |
		             PTL_LE_EVENT_SUCCESS_DISABLE | PTL_LE_EVENT_LINK_DISABLE;
		le.uid = uid;
		ret = PtlLEAppend(portals4_dev_ctx->ni_handle,
		                  portals4_dev_ctx->notify_spc_pt_index,
		                  &le,
		                  PTL_PRIORITY_LIST,
		                  NULL,
		                  &seg->notify_spc_le_handle);

		if (PTL_OK != ret) {
			GASPI_DEBUG_PRINT_ERROR("PtlLEAppend failed with %d", ret);
			_pgaspi_dev_unregister_mem(seg);
			return -1;
		}
	}
	PORTALS4_DEBUG_PRINT_MSG("Leaving pgaspi_dev_register_mem");
	return 0;
}

int pgaspi_dev_unregister_mem(gaspi_context_t const* const GASPI_UNUSED(gctx),
                              gaspi_rc_mseg_t* seg) {
	return _pgaspi_dev_unregister_mem(seg);
}
