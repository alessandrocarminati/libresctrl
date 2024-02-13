#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include "../include/resctrl_util.h"
int main(){
	int16_t *cache_ids_l3;
	int16_t *cache_ids_l2;
	int i, n;

	n = nproc();
	get_cache_ids(&cache_ids_l3, &cache_ids_l2, n);
	for (i=1; i<n; i++) {
		printf("Cache id %d l3=0x%02x, l2=0x%02x\n", i, cache_ids_l3[i], cache_ids_l2[i]);
		}
	return 0;
}
