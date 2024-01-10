#include <stdio.h>
#include "portals4.h"

int main(int argc, char* argv[]) {
	int ret;
	ptl_ni_limits_t ni_limits;
	ptl_handle_ni_t ni_handle;

	ret = PtlInit();
	if (PTL_OK != ret)
		return -1;

	ret = PtlNIInit(PTL_IFACE_DEFAULT,
	                PTL_NI_NO_MATCHING | PTL_NI_PHYSICAL,
	                PTL_PID_ANY,
	                NULL,
	                &ni_limits,
	                &ni_handle);
	if (PTL_OK != ret)
		return -1;

	printf("max_entries: %d\n",ni_limits.max_entries);
	printf("max_unexpected_headers: %d\n",ni_limits.max_unexpected_headers);
	printf("max_mds: %d\n",ni_limits.max_mds);
	printf("max_eqs: %d\n",ni_limits.max_eqs);
	printf("max_cts: %d\n",ni_limits.max_cts);
	printf("max_pt_index: %d\n",ni_limits.max_pt_index);
	printf("max_iovecs: %d\n",ni_limits.max_iovecs);
	printf("max_list_size: %d\n",ni_limits.max_list_size);
	printf("max_triggered_ops: %d\n",ni_limits.max_triggered_ops);
	printf("max_msg_size: %d\n",ni_limits.max_msg_size);
	printf("max_atomic_size: %d\n",ni_limits.max_atomic_size);
	printf("max_fetch_atomic_size: %d\n",ni_limits.max_fetch_atomic_size);
	printf("max_waw_ordered_size: %d\n",ni_limits.max_waw_ordered_size);
	printf("max_war_ordered_size: %d\n",ni_limits.max_war_ordered_size);
	printf("max_volatile_size: %d\n",ni_limits.max_volatile_size);
	printf("features: %d\n",ni_limits.features);

	PtlNIFini(ni_handle);
	PtlFini();

	return 0;
}
