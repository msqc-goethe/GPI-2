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

int pgaspi_dev_register_mem(gaspi_context_t const* const GASPI_UNUSED(gctx),
                            gaspi_rc_mseg_t* GASPI_UNUSED(seg)) {
	return GASPI_SUCCESS;
}

int pgaspi_dev_unregister_mem(gaspi_context_t const* const GASPI_UNUSED(gctx),
                              gaspi_rc_mseg_t* GASPI_UNUSED(seg)) {
	return GASPI_SUCCESS;
}
