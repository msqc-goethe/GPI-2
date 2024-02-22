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

/* #include <errno.h> */
/* #include <sys/mman.h> */
/* #include <sys/time.h> */
/* #include <sys/timeb.h> */
/* #include <unistd.h> */

#include "GASPI.h"
#include "GPI2.h"
#include "GPI2_PORTALS.h"

/* int _pgaspi_dev_unregister_mem(gaspi_context_t const* const GASPI_UNUSED(gctx), */
/*                                gaspi_rc_mseg_t* seg) { */
/* 	if (seg->mr[0] != NULL) { */
/* 		free(seg->mr[0]); */
/* 	} */
/* 	if (seg->mr[1] != NULL) { */
/* 		free(seg->mr[1]); */
/* 	} */
/* 	return GASPI_SUCCESS; */
/* } */

/* int register_nsrc_mem(gaspi_context_t const* const gctx, gaspi_rc_mseg_t* seg) { */
/* 	gaspi_portals4_ctx* const portals4_dev_ctx = */
/* 	    (gaspi_portals4_ctx*) gctx->device->ctx; */

/* 	portals4_dev_ctx->atomic_spc_ptr = seg->data.buf; */
/* 	portals4_dev_ctx->atomic_spc_base_addr = (ptl_size_t) seg->data.buf; */
/* 	portals4_dev_ctx->atomic_spc_length = seg->size; */

/* 	portals4_dev_ctx->notif_spc_ptr = seg->notif_spc.buf; */
/* 	portals4_dev_ctx->notif_spc_base_addr = (ptl_size_t) seg->notif_spc.buf; */
/* 	portals4_dev_ctx->notif_spc_length = seg->notif_spc_size; */

/* 	return GASPI_SUCCESS; */
/* } */

/* int register_grp_mem(gaspi_context_t const* const gctx, gaspi_rc_mseg_t* seg) { */
/* 	seg->mr[0] = calloc(1, sizeof(portals4_mr)); */
/* 	if (seg->mr[0] == NULL) { */
/* 		GASPI_DEBUG_PRINT_ERROR("Memory allocation failed!"); */
/* 		return GASPI_ERR_MEMALLOC; */
/* 	} */

/* 	seg->mr[0].ptr = seg->data.buf; */
/* 	seg->mr[0].base_addr = (ptl_size_t) seg->data.buf; */
/* 	seg->mr[0].length = seg->size; */

/* 	return GASPI_SUCCESS; */
/* } */

/* int register_data_mem(gaspi_context_t const* const gctx, gaspi_rc_mseg_t* seg) { */
/* 	seg->mr[0] = calloc(1, sizeof(portals4_mr)); */
/* 	if (seg->mr[0] == NULL) { */
/* 		GASPI_DEBUG_PRINT_ERROR("Memory allocation failed!"); */
/* 		return -1; */
/* 	} */

/* 	seg->mr[0].ptr = seg->notif_spc.buf; */
/* 	seg->mr[0].base_addr = (ptl_size_t) seg->notif_spc.buf; */
/* 	seg->mr[0].length = seg->size + NOTIFICATIONS_SPACE_SIZE; */

/* 	return GASPI_SUCCESS; */
/* } */

/* int pgaspi_dev_register_mem(gaspi_context_t const* const gctx, */
/*                             gaspi_rc_mseg_t* seg) { */
/* 	int ret; */

/* 	switch (seg->mem_kind) { */
/* 		case NSRC_MEM: */
/* 			ret = register_nsrc_mem(gctx, seg); */
/* 			break; */
/* 		case GRP_MEM: */
/* 			ret = register_grp_mem(gctx, seg); */
/* 			break; */
/* 		case DATA_MEM: */
/* 			ret = register_data_mem(gctx, seg); */
/* 			break; */
/* 		default: */
/* 			GASPI_DEBUG_PRINT_ERROR("Unknown mem kind %d", seg->mem_kind); */
/* 			return GASPI_ERR_DEVICE; */
/* 	} */
/* 	return ret; */
/* } */

/* int pgaspi_dev_unregister_mem(gaspi_context_t const* const gctx, */
/*                               gaspi_rc_mseg_t* seg) { */
/* 	return _pgaspi_dev_unregister_mem(gctx, seg); */
/* } */

int pgaspi_dev_register_mem(gaspi_context_t const* const GASPI_UNUSED(gctx),
                            gaspi_rc_mseg_t* GASPI_UNUSED(seg)) {
	return GASPI_SUCCESS;
}

int pgaspi_dev_unregister_mem(gaspi_context_t const* const GASPI_UNUSED(gctx),
                              gaspi_rc_mseg_t* GASPI_UNUSED(seg)) {
	return GASPI_SUCCESS;
}
