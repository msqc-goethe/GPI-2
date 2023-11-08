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
gaspi_return_t pgaspi_dev_write(
    gaspi_context_t* const GASPI_UNUSED(gctx),
    const gaspi_segment_id_t GASPI_UNUSED(segment_id_local),
    const gaspi_offset_t GASPI_UNUSED(offset_local),
    const gaspi_rank_t GASPI_UNUSED(rank),
    const gaspi_segment_id_t GASPI_UNUSED(segment_id_remote),
    const gaspi_offset_t GASPI_UNUSED(offset_remote),
    const gaspi_size_t GASPI_UNUSED(size),
    const gaspi_queue_id_t GASPI_UNUSED(queue)) {
	return GASPI_SUCCESS;
}

gaspi_return_t pgaspi_dev_read(
    gaspi_context_t* const GASPI_UNUSED(gctx),
    const gaspi_segment_id_t GASPI_UNUSED(segment_id_local),
    const gaspi_offset_t GASPI_UNUSED(offset_local),
    const gaspi_rank_t GASPI_UNUSED(rank),
    const gaspi_segment_id_t GASPI_UNUSED(segment_id_remote),
    const gaspi_offset_t GASPI_UNUSED(offset_remote),
    const gaspi_size_t GASPI_UNUSED(size),
    const gaspi_queue_id_t GASPI_UNUSED(queue)) {
	return GASPI_SUCCESS;
}

gaspi_return_t pgaspi_dev_purge(
    gaspi_context_t* const GASPI_UNUSED(gctx),
    const gaspi_queue_id_t GASPI_UNUSED(queue),
    const gaspi_timeout_t GASPI_UNUSED(timeout_ms)) {
	return GASPI_SUCCESS;
}

gaspi_return_t pgaspi_dev_wait(gaspi_context_t* const GASPI_UNUSED(gctx),
                               const gaspi_queue_id_t GASPI_UNUSED(queue),
                               const gaspi_timeout_t GASPI_UNUSED(timeout_ms)) {
	return GASPI_SUCCESS;
}

gaspi_return_t pgaspi_dev_write_list(
    gaspi_context_t* const GASPI_UNUSED(gctx),
    const gaspi_number_t GASPI_UNUSED(num),
    gaspi_segment_id_t* const GASPI_UNUSED(segment_id_local),
    gaspi_offset_t* const GASPI_UNUSED(offset_local),
    const gaspi_rank_t GASPI_UNUSED(rank),
    gaspi_segment_id_t* const GASPI_UNUSED(segment_id_remote),
    gaspi_offset_t* const GASPI_UNUSED(offset_remote),
    gaspi_size_t* const GASPI_UNUSED(size),
    const gaspi_queue_id_t GASPI_UNUSED(queue)) {
	return GASPI_SUCCESS;
}

gaspi_return_t pgaspi_dev_read_list(
    gaspi_context_t* const GASPI_UNUSED(gctx),
    const gaspi_number_t GASPI_UNUSED(num),
    gaspi_segment_id_t* const GASPI_UNUSED(segment_id_local),
    gaspi_offset_t* const GASPI_UNUSED(offset_local),
    const gaspi_rank_t GASPI_UNUSED(rank),
    gaspi_segment_id_t* const GASPI_UNUSED(segment_id_remote),
    gaspi_offset_t* const GASPI_UNUSED(offset_remote),
    gaspi_size_t* const GASPI_UNUSED(size),
    const gaspi_queue_id_t GASPI_UNUSED(queue)) {
	return GASPI_SUCCESS;
}

gaspi_return_t pgaspi_dev_notify(
    gaspi_context_t* const GASPI_UNUSED(gctx),
    const gaspi_segment_id_t GASPI_UNUSED(segment_id_remote),
    const gaspi_rank_t GASPI_UNUSED(rank),
    const gaspi_notification_id_t GASPI_UNUSED(notification_id),
    const gaspi_notification_t GASPI_UNUSED(notification_value),
    const gaspi_queue_id_t GASPI_UNUSED(queue)) {
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
