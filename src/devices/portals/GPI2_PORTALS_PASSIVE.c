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
#include "GPI2.h"
#include "GPI2_PORTALS.h"
static const char* event_to_str(ptl_event_t event) {
	static char buf[100];

	switch (event.type) {
		case PTL_EVENT_GET:
			return "PTL_EVENT_GET";
		case PTL_EVENT_GET_OVERFLOW:
			return "PTL_EVENT_GET_OVERFLOW";

		case PTL_EVENT_PUT:
			return "PTL_EVENT_PUT";
		case PTL_EVENT_PUT_OVERFLOW:
			return "PTL_EVENT_PUT_OVERFLOW";

		case PTL_EVENT_ATOMIC:
			return "PTL_EVENT_ATOMIC";
		case PTL_EVENT_ATOMIC_OVERFLOW:
			return "PTL_EVENT_ATOMIC_OVERFLOW";

		case PTL_EVENT_FETCH_ATOMIC:
			return "PTL_EVENT_FETCH_ATOMIC";
		case PTL_EVENT_FETCH_ATOMIC_OVERFLOW: {
			return "PTL_EVENT_FETCH_ATOMIC_OVERFLOW";
		}
		case PTL_EVENT_REPLY:
			return "PTL_EVENT_REPLY";
		case PTL_EVENT_SEND:
			return "PTL_EVENT_SEND";
		case PTL_EVENT_ACK:
			return "PTL_EVENT_ACK";

		case PTL_EVENT_PT_DISABLED:
			return "PTL_EVENT_PT_DISABLED";
		case PTL_EVENT_LINK:
			return "PTL_EVENT_LINK";
		case PTL_EVENT_AUTO_UNLINK:
			return "PTL_EVENT_AUTO_UNLINK";
		case PTL_EVENT_AUTO_FREE:
			return "PTL_EVENT_AUTO_FREE";
		case PTL_EVENT_SEARCH:
			return "PTL_EVENT_SEARCH";
	}

	snprintf(buf, sizeof(buf), "event %d", event.type);
	return buf;
}

gaspi_return_t pgaspi_dev_passive_send(
    gaspi_context_t* const gctx,
    const gaspi_segment_id_t segment_id_local,
    const gaspi_offset_t offset_local,
    const gaspi_rank_t rank,
    const gaspi_size_t size,
    const gaspi_timeout_t timeout_ms) {
	int ret, nnr;
	const int byte_id = rank >> 3;
	const int bit_pos = rank - (byte_id * 8);
	const unsigned char bit_cmp = 1 << bit_pos;
	gaspi_portals4_ctx* const portals4_dev_ctx = gctx->device->ctx;
	portals4_mr* local_mr_ptr =
	    (portals4_mr*) gctx->rrmd[segment_id_local][gctx->rank].mr[0];
	gaspi_cycles_t s0;
	ptl_ct_event_t ce, nce;

	ret = PtlCTGet(portals4_dev_ctx->passive_snd_ct_handle, &ce);

	if (PTL_OK != ret) {
		GASPI_DEBUG_PRINT_ERROR("PtlCTGet failed with %d", ret);
		return GASPI_ERROR;
	}

	nnr = ce.success + 1;

	if (gctx->ne_count_p[byte_id] & bit_cmp) {
		goto checkL;
	}

	gctx->ne_count_p[byte_id] |= bit_cmp;

	GASPI_DEBUG_PRINT_ERROR(
	    "MEPT index: %d nnr = %d", portals4_dev_ctx->me_pt_index, nnr);
	ret = PtlPut(local_mr_ptr->passive_md,
	             offset_local,
	             size,
	             PORTALS4_PASSIVE_ACK_TYPE,
	             portals4_dev_ctx->remote_info[rank].phys_address,
	             portals4_dev_ctx->me_pt_index,
	             PORTALS4_ME_MATCH_BITS,
	             offset_local,
	             NULL,
	             0);

	if (PTL_OK != ret) {
		GASPI_DEBUG_PRINT_ERROR("PtlPut failed with %d", ret);
		return GASPI_ERROR;
	}

checkL:
	ptl_event_t event;
	unsigned int event_index;
	ret = PtlEQPoll(&portals4_dev_ctx->eq_handle,
	                1,
	                timeout_ms,
	                &event,
	                &event_index);

	if (PTL_OK != ret) {
		GASPI_DEBUG_PRINT_ERROR("PtlEQPoll failed with %d", ret);
		return GASPI_ERROR;
	}


	if (PTL_NI_OK != event.ni_fail_type) {
		GASPI_DEBUG_PRINT_ERROR("Passive send event error %d",
		                        event.ni_fail_type);
		return GASPI_ERROR;
	}
	GASPI_DEBUG_PRINT_ERROR("Send: Event type %s", event_to_str(event));
	/* s0 = gaspi_get_cycles(); */

	/* do { */
	/* 	ret = PtlCTGet(portals4_dev_ctx->passive_snd_ct_handle, &ce); */
	/* 	if (PTL_OK != ret) { */
	/* 		GASPI_DEBUG_PRINT_ERROR("PtlCTGet failed with %d", ret); */
	/* 		return GASPI_ERROR; */
	/* 	} */
	/* 	if (ce.failure > 0) { */
	/* 		GASPI_DEBUG_PRINT_ERROR("Passive queue might be broken"); */
	/* 		return GASPI_ERROR; */
	/* 	} */
	/* 	if (ce.success == 0) { */
	/* 		const gaspi_cycles_t s1 = gaspi_get_cycles(); */
	/* 		const gaspi_cycles_t tdelta = s1 - s0; */
	/* 		const float ms = (float) tdelta * gctx->cycles_to_msecs; */
	/* 		if (ms > timeout_ms) { */
	/* 			return GASPI_TIMEOUT; */
	/* 		} */
	/* 	} */
	/* } while (ce.success != nnr); */
	/* GASPI_DEBUG_PRINT_ERROR("Send ce.succecss = %d", ce.success); */
	//PtlCTSet(portals4_dev_ctx->passive_snd_ct_handle, nce);
	gctx->ne_count_p[byte_id] &= (~bit_cmp);
	return GASPI_SUCCESS;
}

gaspi_return_t pgaspi_dev_passive_receive(
    gaspi_context_t* const gctx,
    const gaspi_segment_id_t segment_id_local,
    const gaspi_offset_t offset_local,
    gaspi_rank_t* const rank,
    const gaspi_size_t size,
    const gaspi_timeout_t timeout_ms) {
	ptl_handle_me_t me_handle;
	int ret;
	gaspi_portals4_ctx* const portals4_dev_ctx = gctx->device->ctx;
	portals4_mr* local_mr_ptr =
	    (portals4_mr*) gctx->rrmd[segment_id_local][gctx->rank].mr[0];
	gaspi_cycles_t s0;

	ptl_me_t me = {
	    .match_id.phys.pid = PTL_PID_ANY,
	    .match_id.phys.nid = PTL_NID_ANY,
	    .start = (void*) (gctx->rrmd[segment_id_local][gctx->rank].data.addr +
	                      offset_local),
	    .length = size,
	    .ct_handle = PTL_CT_NONE,
	    .uid = PTL_UID_ANY,
	    .min_free = 0,
	    .options =
	        PTL_ME_OP_PUT | PTL_ME_MANAGE_LOCAL | PTL_ME_NO_TRUNCATE |
	        PTL_ME_USE_ONCE | PTL_ME_IS_ACCESSIBLE |
	        PTL_ME_EVENT_LINK_DISABLE | /*PTL_ME_EVENT_FLOWCTRL_DISABLE |*/
	        PTL_ME_EVENT_UNLINK_DISABLE,
	    .match_bits = PORTALS4_ME_MATCH_BITS,
	    .ignore_bits = PORTALS4_ME_IGNORE_BITS,
	};

	ret = PtlMEAppend(portals4_dev_ctx->match_ni_handle,
	                  portals4_dev_ctx->me_pt_index,
	                  &me,
	                  PTL_PRIORITY_LIST,
	                  NULL,
	                  &me_handle);

	if (PTL_OK != ret) {
		GASPI_DEBUG_PRINT_ERROR("PtlMEAppend failed with %d", ret);
		return GASPI_ERROR;
	}
	static int f_cnt = 0;
	GASPI_DEBUG_PRINT_ERROR("MEPT index: %d function cnt: %d",
	                        portals4_dev_ctx->me_pt_index,
	                        f_cnt);
	++f_cnt;
	ptl_event_t event;
	unsigned int event_index;

	ret = PtlEQPoll(&portals4_dev_ctx->passive_rcv_eq_handle,
	                1,
	                timeout_ms,
	                &event,
	                &event_index);

	if (PTL_OK != ret) {
		GASPI_DEBUG_PRINT_ERROR("PtlEQPoll failed with %d", ret);
		return GASPI_ERROR;
	}

	GASPI_DEBUG_PRINT_ERROR("Recv: Event type %s", event_to_str(event));

	if (PTL_NI_OK != event.ni_fail_type) {
		GASPI_DEBUG_PRINT_ERROR("Passive receive event error %d",
		                        event.ni_fail_type);
		return GASPI_ERROR;
	}

	*rank = 0xffff;
	for (int i = 0; i < gctx->tnc; ++i) {
		ptl_process_t initiator = portals4_dev_ctx->remote_info[i].phys_address;
		if (event.initiator.phys.nid == initiator.phys.nid &&
		    event.initiator.phys.pid == initiator.phys.pid) {
			*rank = i;
			break;
		}
	}

	if (*rank == 0xffff) {
		return GASPI_ERROR;
	}

	return GASPI_SUCCESS;
}
