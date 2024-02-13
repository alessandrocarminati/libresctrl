#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <inttypes.h>
#include <string.h>

#include "../include/resctrl_util.h"

struct max_avail_ptn_test {
	char		pattern[16];
	uint64_t	mem_size;
	uint64_t	expected;
};

struct best_fitting_block_ptn_test {
	char		pattern[16];
	uint64_t	mem_size;
	uint64_t	expected;
	uint64_t	requested;
};

void main(){
	uint64_t				num, i, max_size_blk, bit_mask;
	struct max_avail_ptn_test		max_avail_ptn[]={
						{"FC7E", 2000, 0x7e}, {"fc7e", 2000, 0x7e}, {"F00C", 2000, 0xf000}, {"800C", 2000, 0xc}, {"c073", 2000, 0x0070}, {"FE7F", 2000, 0x7f}, {"F0FC", 2000, 0xfc}, {"BC1C", 2000, 0x3c00}
						};
	struct best_fitting_block_ptn_test	best_fit_ptn[]={
						{"FC7E", 2000, 0x60, 200},
						{"fc7e", 2000, 0x60, 200},
						{"F00C", 2000, 0x0c, 200},
						{"800C", 2000, 0x0c, 200},
						{"c073", 2000, 0x03, 200},
						{"FE7F", 2000, 0x60, 200},
						{"F0FC", 2000, 0xc000, 200},
						{"BC1C", 2000, 0x18, 200},
						{"0020", 2000, 0x0, 200}
						};


	printf("test max_contiguos_mem_avail() with %lu patterns\n",sizeof(max_avail_ptn)/sizeof(max_avail_ptn[0]));
	for (i=0; i<sizeof(max_avail_ptn)/sizeof(max_avail_ptn[0]); i++){
		max_size_blk = max_contiguos_mem_avail(max_avail_ptn[i].mem_size, max_avail_ptn[i].pattern, &num);
		printf("[%s] - For max_avail_ptn = 0x%s (memsize=%lu), larger block is:%lu with mask=0x%04lx(expected 0x%04lx) \n", 
			num==max_avail_ptn[i].expected?"OK":"KO", max_avail_ptn[i].pattern, max_avail_ptn[i].mem_size, max_size_blk, num, max_avail_ptn[i].expected);
	}



	printf("test best_fitting_block() with %lu patterns\n",sizeof(best_fit_ptn)/sizeof(best_fit_ptn[0]));
	for (i=0; i<sizeof(best_fit_ptn)/sizeof(best_fit_ptn[0]); i++){
		printf("%02ld => ", i);
		bit_mask = best_fitting_block(best_fit_ptn[i].mem_size, best_fit_ptn[i].pattern, best_fit_ptn[i].requested);
		printf("[%s] - Request %ld and memory state=%s ==> expect bitmask 0x%04lx and current bitmask= 0x%04lx \n", 
			bit_mask==best_fit_ptn[i].expected?"OK":"KO", best_fit_ptn[i].requested, best_fit_ptn[i].pattern, best_fit_ptn[i].expected, bit_mask);
	}



}
