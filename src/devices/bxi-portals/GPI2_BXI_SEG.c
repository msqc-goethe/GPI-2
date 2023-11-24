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

int ptl_le_factory(gaspi_portals4_ctx* dev,
                   void* start,
                   ptl_size_t length,
                   ptl_le_handle_t* ptl_handle,
                   unsigned int options,
                   ptl_le_index_t* pt_index,
                   ptl_eq_handle_t eq_handle,
                   ptl_ct_handle_t ct_handle) {
	int ret;
	ptl_le_t le;
	ptl_uid_t uid;

	ret = PtlGetUid(dev->ni_handle, &uid);

	if (PTL_OK != ret) {
		GASPI_DEBUG_PRINT_ERROR("PtlGetUid failed with %d", ret);
		return -1;
	}

	ret = PtlPTAlloc(dev->ni_handle, 0, eq_handle, dev->seg_cnt, pt_index);
	le.start = start;
	le.length = length;
	le.uid = uid;
	le.options = options;
	le.ct_handle = ct_handle;
	ret = PtlLEAppend(
	    dev->ni_handle, pt_index, &le, PTL_PRIORITY_LIST, NULL, le_handle);

	if (PTL_OK != ret) {
		GASPI_DEBUG_PRINT_ERROR("PtlLEAppend failed with %d", ret);
		return -1;
	}

	dev->seg_cnt++;
	return 0;
}

int _pgaspi_dev_unregister_mem(gaspi_context_t const* const gctx,
                               gaspi_rc_mseg_t* seg) {
	int ret, i, u;
	gaspi_portals4_ctx* const portals4_dev_ctx =
	    (gaspi_portals4_ctx*) gctx->device->ctx;
	portals4_mr* mr_ptr = NULL;

	for (i = 0; i < 2; ++i) {
		mr_ptr = (portals4_mr*) seg->mr[i];
		if (mr_ptr == NULL)
			break;
		if (!PtlHandleIsEqual(mr_ptr->group_md, PTL_INVALID_HANDLE)) {
			ret = PtlMDRelease(mr_ptr->group_md);
			if (PTL_OK != ret) {
				GASPI_DEBUG_PRINT_ERROR("PtlMDRelease failed with %d", ret);
				return -1;
			}
		}
		if (!PtlHandleIsEqual(mr_ptr->passive_md, PTL_INVALID_HANDLE)) {
			ret = PtlMDRelease(mr_ptr->passive_md);
			if (PTL_OK != ret) {
				GASPI_DEBUG_PRINT_ERROR("PtlMDRelease failed with %d", ret);
				return -1;
			}
		}
		for (u = 0; u < GASPI_MAX_QP; ++u) {
			if (!PtlHandleIsEqual(mr_ptr->comm_md[u], PTL_INVALID_HANDLE)) {
				ret = PtlMDRelease(mr_ptr->comm_md[u]);
				if (PTL_OK != ret) {
					GASPI_DEBUG_PRINT_ERROR("PtlMDRelease failed with %d", ret);
					return -1;
				}
			}
		}
		if (!PtlHandleIsEqual(mr_ptr->le_handle, PTL_INVALID_HANDLE)) {
			ret = PtlLEUnlink(mr_ptr->le_handle);
			if (PTL_OK != ret) {
				GASPI_DEBUG_PRINT_ERROR("PtlMDRelease failed with %d", ret);
				return -1;
			}
		}
		if (mr_ptr->pt_index) {
			ret = PtlPTFree(portals4_dev_ctx->ni_handle, mr_ptr->pt_index);
			if (PTL_OK != ret) {
				GASPI_DEBUG_PRINT_ERROR("PtlPTFree failed with %d", ret);
				return -1;
			}
		}
		free(mr_ptr);
		mr_ptr = NULL;
	}
	return 0;
}

int pgaspi_dev_register_mem(gaspi_context_t const* const gctx,
                            gaspi_rc_mseg_t* seg) {
	int ret, i;
	unsigned int le_options = 0;
	ptl_md_t md;
	ptl_le_t le;
	ptl_uid_t uid = PTL_UID_ANY;
	gaspi_portals4_ctx* const portals4_dev_ctx =
	    (gaspi_portals4_ctx*) gctx->device->ctx;
	portals4_mr* mr_ptr = NULL;

	if (seg->data.buf != NULL) {
		memset(&md, 0, sizeof(ptl_md_t));
		memset(&le, 0, sizeof(ptl_le_t));

		seg->mr[0] = calloc(1, sizeof(portals4_mr));
		if (seg->mr[0] == NULL) {
			GASPI_DEBUG_PRINT_ERROR("Memory allocation failed!");
			return -1;
		}
		mr_ptr = (portals4_mr*) seg->mr[0];
		md.start = seg->data.buf;
		md.length = seg->size;
		md.options = PTL_MD_EVENT_SUCCESS_DISABLE | PTL_MD_EVENT_CT_REPLY |
		             PTL_MD_EVENT_CT_ACK;
		md.eq_handle = portals4_dev_ctx->eq_handle;

		if (seg->mem_kind == GASPI_GROUP_MEM) {
			// register MD to group communication counting events
			md.ct_handle = portals4_dev_ctx->group_ct_handle;

			ret =
			    PtlMDBind(portals4_dev_ctx->ni_handle, &md, &mr_ptr->group_md);
			if (PTL_OK != ret) {
				GASPI_DEBUG_PRINT_ERROR("PtlMDBind failed with %d", ret);
				_pgaspi_dev_unregister_mem(gctx, seg);
				return -1;
			}
			//GASPI_DEBUG_PRINT_ERROR("Allocating DATA LE with index %d at addr: %lu with length %lu",portals4_dev_ctx->seg_cnt,(unsigned long)seg->data.buf,seg->size);
		}
		else {
			// register MD for passive communication counting events
			md.ct_handle = PTL_CT_NONE;
			md.eq_handle = portals4_dev_ctx->passive_snd_eq_handle;

			ret = PtlMDBind(
			    portals4_dev_ctx->ni_handle, &md, &mr_ptr->passive_md);
			if (PTL_OK != ret) {
				GASPI_DEBUG_PRINT_ERROR("PtlMDBind failed with %d", ret);
				_pgaspi_dev_unregister_mem(gctx, seg);
				return -1;
			}

			// register MD for one-sided communication counting events
			md.eq_handle = PTL_EQ_NONE;
			for (i = 0; i < gctx->tnc; ++i) {
				md.ct_handle = portals4_dev_ctx->comm_ct_handle[i];

				ret = PtlMDBind(
				    portals4_dev_ctx->ni_handle, &md, &mr_ptr->comm_md[i]);
				if (PTL_OK != ret) {
					GASPI_DEBUG_PRINT_ERROR("PtlMDBind failed with %d", ret);
					_pgaspi_dev_unregister_mem(gctx, seg);
					return -1;
				}
			}
		}

		le_options = PTL_LE_OP_PUT | PTL_LE_OP_GET |
		             PTL_LE_EVENT_SUCCESS_DISABLE | PTL_LE_EVENT_LINK_DISABLE;
		if (seg->mem_kind != GASPI_GROUP_MEM)
			le_options |= PTL_LE_IS_ACCESSIBLE;

		ret = ptl_le_factory(portals4_dev_ctx,
		                     seg->data.buf,
		                     seg->size,
		                     &mr_ptr->le_handle,
		                     le_options,
		                     &mr_ptr->pt_index,
		                     portals4_dev_ctx->eq_handle,
		                     PTL_CT_NONE);

		if (ret < 0) {
			GASPI_DEBUG_PRINT_ERROR("ptl_le_factory failed with %d", ret);
			_pgaspi_dev_unregister_mem(gctx, seg);
			return -1;
		}

		ret = PtlPTAlloc(portals4_dev_ctx->ni_handle,
		                 0,
		                 portals4_dev_ctx->eq_handle,
		                 portals4_dev_ctx->seg_cnt,
		                 &mr_ptr->pt_index);
		le.start = seg->data.buf;
		le.length = seg->size;
		le.uid = uid;

		ret = PtlLEAppend(portals4_dev_ctx->ni_handle,
		                  mr_ptr->pt_index,
		                  &le,
		                  PTL_PRIORITY_LIST,
		                  NULL,
		                  &mr_ptr->le_handle);
		if (PTL_OK != ret) {
			GASPI_DEBUG_PRINT_ERROR("PtlLEAppend failed with %d", ret);
			_pgaspi_dev_unregister_mem(gctx, seg);
			return -1;
		}
		portals4_dev_ctx->seg_cnt++;
	}

	if (seg->notif_spc.buf != NULL) {
		memset(&md, 0, sizeof(ptl_md_t));
		memset(&le, 0, sizeof(ptl_le_t));

		seg->mr[1] = (portals4_mr*) calloc(1, sizeof(portals4_mr));
		if (seg->mr[1] == NULL) {
			GASPI_DEBUG_PRINT_ERROR("Memory allocation failed!");
			return -1;
		}
		mr_ptr = (portals4_mr*) seg->mr[1];
		md.start = seg->notif_spc.buf;
		md.length = seg->notif_spc_size;
		md.options = PTL_MD_EVENT_SUCCESS_DISABLE | PTL_MD_EVENT_CT_REPLY |
		             PTL_MD_EVENT_CT_ACK;
		md.eq_handle = portals4_dev_ctx->eq_handle;
		if (seg->mem_kind == GASPI_GROUP_MEM) {
			// register MD to group communication counting events
			md.ct_handle = portals4_dev_ctx->group_ct_handle;

			ret =
			    PtlMDBind(portals4_dev_ctx->ni_handle, &md, &mr_ptr->group_md);
			if (PTL_OK != ret) {
				GASPI_DEBUG_PRINT_ERROR("PtlMDBind failed with %d", ret);
				_pgaspi_dev_unregister_mem(gctx, seg);
				return -1;
			}
		}
		else {
			// register MD for passive communication counting events
			md.ct_handle = portals4_dev_ctx->passive_ct_handle;

			ret = PtlMDBind(
			    portals4_dev_ctx->ni_handle, &md, &mr_ptr->passive_md);
			if (PTL_OK != ret) {
				GASPI_DEBUG_PRINT_ERROR("PtlMDBind failed with %d", ret);
				_pgaspi_dev_unregister_mem(gctx, seg);
				return -1;
			}

			// register MD for one-sided communication counting events
			for (i = 0; i < gctx->tnc; ++i) {
				md.ct_handle = portals4_dev_ctx->comm_ct_handle[i];

				ret = PtlMDBind(
				    portals4_dev_ctx->ni_handle, &md, &mr_ptr->comm_md[i]);
				if (PTL_OK != ret) {
					GASPI_DEBUG_PRINT_ERROR("PtlMDBind failed with %d", ret);
					_pgaspi_dev_unregister_mem(gctx, seg);
					return -1;
				}
			}
		}
		ret = PtlPTAlloc(portals4_dev_ctx->ni_handle,
		                 0,
		                 portals4_dev_ctx->eq_handle,
		                 portals4_dev_ctx->seg_cnt,
		                 &mr_ptr->pt_index);

		le.start = seg->notif_spc.buf;
		le.length = seg->notif_spc_size;
		le.uid = uid;
		le.options = PTL_LE_OP_PUT | PTL_LE_OP_GET | PTL_LE_EVENT_LINK_DISABLE |
		             PTL_LE_EVENT_SUCCESS_DISABLE;

		ret = PtlLEAppend(portals4_dev_ctx->ni_handle,
		                  mr_ptr->pt_index,
		                  &le,
		                  PTL_PRIORITY_LIST,
		                  NULL,
		                  &mr_ptr->le_handle);
		if (PTL_OK != ret) {
			GASPI_DEBUG_PRINT_ERROR("PtlLEAppend failed with %d", ret);
			_pgaspi_dev_unregister_mem(gctx, seg);
			return -1;
		}
		portals4_dev_ctx->seg_cnt++;
	}
	return 0;
}

int pgaspi_dev_unregister_mem(gaspi_context_t const* const gctx,
                              gaspi_rc_mseg_t* seg) {
	return _pgaspi_dev_unregister_mem(gctx, seg);
}
