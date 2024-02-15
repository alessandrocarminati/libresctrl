#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <string.h>

#include "../include/resctrl_util.h"

#define BOOL2STR(x) ((x)?"OK":"KO")

struct test_ptn {
	char		*fn;
	uint64_t	request_bitmask; //<<check type
	int		level;
	int		cacheid;
	char		*expect_str;
	};

//char *create_req_str(struct resctrl_info *r, uint64_t bitmask, int level, int cacheid){

int main() {
	int res = 1, i, tmp;
	struct resctrl_info *r;
	char *req_str;
	struct test_ptn ptn[] ={
	{"test_files/test1", 0xe3ff, 3, 0, "L3:0=e3ff"},
	{"test_files/test1", 0xf000, 3, 1, NULL},
	{"test_files/test1", 0xf000, 2, 0, NULL},

	{"test_files/test2", 0xffc3, 3, 0, "L3CODE:0=ffc3"},

	{"test_files/test3", 0xffc3, 3, 1, "L3CODE:0=ffff;1=ffc3"},

	{"test_files/test4", 0x0c00, 3, 2, "L3CODE:0=ffff;1=ffff;2=0c00"},

	{"test_files/test5", 0xfff0, 2, 0, "L2CODE:0=fff0;1=ffff;2=fc00"},
	{"test_files/test5", 0xfff0, 3, 0, "L3CODE:0=fff0;1=ffff;2=fc00"},

	{"test_files/test6", 0xfff0, 3, 0, "L3:0=fff0"},
	{"test_files/test6", 0xfff0, 2, 0, "L2:0=fff0"},

	{"test_files/test7", 0xfff0, 3, 7, "L3:0=ffff;1=ffff;2=ffff;3=ffff;4=ffff;5=ffff;6=ffff;7=fff0"},
	{"test_files/test7", 0xfff0, 2, 7, "L2:0=ffff;1=ffff;2=ffff;3=ffff;4=ffff;5=ffff;6=ffff;7=fff0"},
	};

	for (i=0; i< sizeof(ptn)/sizeof(struct test_ptn); i++) {
		r = parse_cache(ptn[i].fn, nproc(), NULL, NULL);
		req_str = create_req_str(r, ptn[i].request_bitmask,  ptn[i].level,  ptn[i].cacheid);
		tmp = (req_str)?strcmp(req_str, ptn[i].expect_str)==0?1:0:(req_str==ptn[i].expect_str);
		printf("[%s] generated=\"%s\" expected=\"%s\"\n", BOOL2STR(tmp), req_str, ptn[i].expect_str);
		if (req_str)
			free(req_str);
		dispose_resctrl_info(r);
		res = res && tmp;
		}

	return !res;
}
