#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <string.h>

#include "../include/resctrl_util.h"

#define BOOL2STR(x) ((x)?"OK":"KO")

struct test_ptn {
	char		*fn;
	char		*l2cacheid_fmt;
	char		*l3cacheid_fmt;
	int		cpu;
	int		level;
	int		expected_caches;
	int		ncpu;
	};


int main() {
	int res = 1, i, tmp;
	struct resctrl_info *r;
	struct test_ptn ptn[] ={
	{"test_files/test1", "test_files/scenario1/cpu%dl2", "test_files/scenario1/cpu%dl3", 0, 2, 0, 16},
	{"test_files/test1", "test_files/scenario1/cpu%dl2", "test_files/scenario1/cpu%dl3", 3, 2, 1, 16},
	{"test_files/test1", "test_files/scenario1/cpu%dl2", "test_files/scenario1/cpu%dl3", 12, 2, 6, 16},
	{"test_files/test1", "test_files/scenario1/cpu%dl2", "test_files/scenario1/cpu%dl3", 15, 3, 0, 16},
	{"test_files/test1", "test_files/scenario1/cpu%dl2", "test_files/scenario1/cpu%dl3", 8, 3, 0, 16},

	{"test_files/test1", "test_files/scenario2/cpu%dl2", "test_files/scenario2/cpu%dl3", 0, 2, 0, 16},
	{"test_files/test1", "test_files/scenario2/cpu%dl2", "test_files/scenario2/cpu%dl3", 3, 2, 1, 16},
	{"test_files/test1", "test_files/scenario2/cpu%dl2", "test_files/scenario2/cpu%dl3", 10, 3, 1, 16},

	{"test_files/test1", "test_files/scenario3/cpu%dl2", "test_files/scenario3/cpu%dl3", 4, 2, 1, 16},
	{"test_files/test1", "test_files/scenario3/cpu%dl2", "test_files/scenario3/cpu%dl3", 6, 3, 0, 16},
	{"test_files/test1", "test_files/scenario3/cpu%dl2", "test_files/scenario3/cpu%dl3", 15, 3, 0, 16},

	{"test_files/test1", "test_files/scenario3/cpu%dl2", "test_files/scenario3/cpu%dl3", 15, 4, -1, 16},
	};

	for (i=0; i< sizeof(ptn)/sizeof(struct test_ptn); i++) {
		r = parse_cache(ptn[i].fn, ptn[i].ncpu, ptn[i].l2cacheid_fmt, ptn[i].l3cacheid_fmt);
		tmp = cpulevel2id(ptn[i].cpu, ptn[i].level, r, ptn[i].ncpu);
		printf("[%s] - cpu=%d, level=%d => cacheid=%d expected=%d\n", BOOL2STR(tmp==ptn[i].expected_caches), ptn[i].cpu, ptn[i].level, tmp, ptn[i].expected_caches);
//		for (int j=0; j<ptn[i].ncpu; j++) printf("%d ", r->cache_id_map_l2[j]); printf("\n");
//		for (int j=0; j<ptn[i].ncpu; j++) printf("%d ", r->cache_id_map_l3[j]); printf("\n");
		dispose_resctrl_info(r);
		res = res && (tmp==ptn[i].expected_caches);
		}

	return !res;
}
