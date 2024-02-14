#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>

#include "../include/resctrl_util.h"

struct test_pattern {
	char		*cache_path_fmt_l2;
	char		*cache_path_fmt_l3;
	int16_t		*expected_cache_ids_l2;
	int16_t		*expected_cache_ids_l3;
	int		nproc;
	int		expected_state;
};


int verify_data(int16_t *a, int16_t *b, int n) {
	int i;
	for (i=0; i<n; i++)
		if (a[i]!=b[i])
			return 0;
	return 1;
}

int main(){
	int16_t *cache_ids_l3;
	int16_t *cache_ids_l2;
	struct test_pattern ptn[]={
	{"test_files/scenario1/cpu%dl2", "test_files/scenario1/cpu%dl3", (int16_t[]){0,0,1,1,2,2,3,3,4,4,5,5,6,6,7,7}, (int16_t[]){0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, 12, 1},
	{"test_files/scenario2/cpu%dl2", "test_files/scenario2/cpu%dl3", (int16_t[]){0,0,1,1,2,2,3,3,4,4,5,5,6,6,7,7}, (int16_t[]){0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1}, 12, 1},
	{"test_files/scenario3/cpu%dl2", "test_files/scenario3/cpu%dl3", (int16_t[]){0,0,0,0,1,1,1,1,2,2,2,2,3,3,3,3}, (int16_t[]){0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, 12, 1},
	{"test_files/scenario4/cpu%dl2", "test_files/scenario4/cpu%dl3", (int16_t[]){0,0,1,1,2,2,3,3,4,4,5,5,6,6,7,7}, (int16_t[]){0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1}, 12, 1},
	};
	int i;
	int test_state, l2data_state, l3data_state, ret=1;

	for (i=0; i<sizeof(ptn)/sizeof(struct test_pattern); i++) {
		test_state = get_cache_ids(&cache_ids_l3, &cache_ids_l2, ptn[i].nproc, ptn[i].cache_path_fmt_l2, ptn[i].cache_path_fmt_l3) == ptn[i].expected_state;
		l2data_state = verify_data(ptn[i].expected_cache_ids_l2, cache_ids_l2, ptn[i].nproc);
		l3data_state = verify_data(ptn[i].expected_cache_ids_l2, cache_ids_l2, ptn[i].nproc);
		printf("[%s] - test result =>%s, cacheids: l2 =>%s l3=>%s\n", (test_state && l2data_state && l3data_state)?"OK":"KO", test_state?"OK":"KO", l2data_state?"OK":"KO", l3data_state?"OK":"KO");
		free(cache_ids_l3);
		free(cache_ids_l2);
		ret = ret && (test_state && l2data_state && l3data_state);
	}
	return !ret;
}
