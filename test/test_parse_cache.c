#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <string.h>

#include "../include/resctrl_util.h"

#define BOOL2STR(x) ((x)?"OK":"KO")

struct parse_cache_ptn {
	char		*fn;
	int		expected_caches;
	int		expected_number_l2;
	int		expected_number_l3;
	unsigned int	*expected_data_l2;
	unsigned int	*expected_data_l3;
	};

int verdata(unsigned int *l, unsigned int *r, int len){
	int i;
	int res=1;

	for (i =0;i<len;i++)
		res = res && l[i]==r[i];
	return res;
}

int min(int a, int b) {
	if (a<b) return a;
	return b;
}


int verify(struct resctrl_info *r, struct parse_cache_ptn *ptn){
	int path, caches_n, l3_num, l2_num, l3_data, l2_data, res;

	path = strcmp(r->path, ptn->fn)==0;
	caches_n = ptn->expected_caches == ((r->cache_l3!=NULL)+(r->cache_l2!=NULL));
	l3_num = r->cache_l3?ptn->expected_number_l3 == r->cache_l3->number:1;
	l2_num = r->cache_l2?ptn->expected_number_l2 == r->cache_l2->number:1;
	l3_data = r->cache_l3?verdata(ptn->expected_data_l3, r->cache_l3->bitmask, min(ptn->expected_number_l3, r->cache_l3->number)):1;
	l2_data = r->cache_l2?verdata(ptn->expected_data_l2, r->cache_l2->bitmask, min(ptn->expected_number_l2, r->cache_l2->number)):1;
	res = path && caches_n && l3_num && l2_num && l3_data && l2_data;
	printf("[%s] - Test_file='%s' ==> path->%s, caches_n->%s, l3_num=%s, l2_num->%s, l3_data->%s, l2_data->%s\n",
		BOOL2STR(res), ptn->fn, BOOL2STR(path), BOOL2STR(caches_n), BOOL2STR(l3_num), BOOL2STR(l2_num), BOOL2STR(l3_data), BOOL2STR(l2_data));
	return res;
}

int main() {
	int res = 1, i, tmp;
	struct resctrl_info *r;
	struct parse_cache_ptn ptn[] ={
	{"test_files/test1", 1, 0, 1, NULL, (unsigned int[]){0xffffU}},
	{"test_files/test2", 1, 0, 1, NULL, (unsigned int[]){0xffffU}},
	{"test_files/test3", 1, 0, 2, NULL, (unsigned int[]){0xffffU, 0xffffU}},
	{"test_files/test4", 1, 0, 3, NULL, (unsigned int[]){0xffffU, 0xffffU, 0xfc00U}},
	{"test_files/test5", 2, 3, 3, (unsigned int[]){0xffffU, 0xffffU, 0xfc00U}, (unsigned int[]){0xffffU, 0xffffU, 0xfc00U}},
	{"test_files/test6", 2, 1, 1, (unsigned int[]){0xffffU}, (unsigned int[]){0xffffU}},
	{"test_files/test7", 2, 8, 8, (unsigned int[]){0xffffU, 0xffffU, 0xffffU, 0xffffU, 0xffffU, 0xffffU, 0xffffU, 0xffffU}, (unsigned int[]){0xffffU, 0xffffU, 0xffffU, 0xffffU, 0xffffU, 0xffffU, 0xffffU, 0xffffU}},
	};

	for (i=0; i< sizeof(ptn)/sizeof(struct parse_cache_ptn); i++) {
		r = parse_cache(ptn[i].fn, NULL, NULL);
		tmp = verify(r, &ptn[i]);
		dispose_resctrl_info(r);
		res = res && tmp;
		}

	return !res;
}
