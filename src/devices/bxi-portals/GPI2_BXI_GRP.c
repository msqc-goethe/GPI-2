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
#include <sys/mman.h>
#include <sys/timeb.h>
#include <unistd.h>
#include "GASPI.h"
#include "GPI2.h"
#include "GPI2_BXI.h"
#include "GPI2_Coll.h"

/* Group utilities */
int pgaspi_dev_poll_groups(gaspi_context_t* const GASPI_UNUSED(gctx)) {
	return 0;
}

int pgaspi_dev_post_group_write(gaspi_context_t* const GASPI_UNUSED(gctx),
                                void* GASPI_UNUSED(local_addr),
                                int GASPI_UNUSED(length),
                                int GASPI_UNUSED(dst),
                                void* GASPI_UNUSED(remote_addr),
                                unsigned char GASPI_UNUSED(group)) {
	return 0;
}
