#include <stdio.h>

#include "../include/resctrl_util.h"

int main(){
	int features;

	features = parse_cpu_features();

	if (features & RTLA_CPUF_RDT_A)		printf("RDT (Resource Director Technology) Allocation - rdt_a\n");
	if (features & RTLA_CPUF_CAT_L3)	printf("CAT (Cache Allocation Technology) - cat_l3\n");
	if (features & RTLA_CPUF_CAT_L2)	printf("CAT (Cache Allocation Technology) - cat_l2\n");
	if (features & RTLA_CPUF_CDP_L3)	printf("CDP (Code and Data Prioritization ) - cdp_l3\n");
	if (features & RTLA_CPUF_CDP_L2)	printf("CDP (Code and Data Prioritization ) - cdp_l2\n");
	if (features & RTLA_CPUF_CQM_LLC)	printf("CQM (Cache QoS Monitoring) - cqm_llc\n");
	if (features & RTLA_CPUF_CQM_OCCUP_LLC)	printf("CQM (Cache QoS Monitoring) - cqm_occup_llc\n");
	if (features & RTLA_CPUF_CQM_MBM_TOTAL)	printf("MBM (Memory Bandwidth Monitoring) - cqm_mbm_total\n");
	if (features & RTLA_CPUF_CQM_MBM_LOCAL)	printf("MBM (Memory Bandwidth Monitoring) - cqm_mbm_local\n");
	if (features & RTLA_CPUF_MBA)		printf("MBA (Memory Bandwidth Allocation) - mba\n");
}

