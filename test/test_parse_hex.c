#include <stdio.h>

#include "../include/resctrl_util.h"

struct parse_hex_test_pattern {
	char	*line;
	int	expected;
};


int main() {
	struct parse_hex_test_pattern test_ptn[] = {
	{"FFFF",	0xffff},
	{"AAAA",	0xaaaa},
	{"01234567",	0x01234567},
	{"12121111", 0x12121111},
	{"ff", 0xff},
	{"abc", 0xabc},
	{"pippo", 0x0},
	{"123", 0x123}
	};
	int i, tmp, res=1;

	for (i=0; i<sizeof(test_ptn)/sizeof(struct parse_hex_test_pattern); i++) {
		tmp = parse_hex(test_ptn[i].line)==test_ptn[i].expected;
		printf("[%s] %s => expected=%08x\n", tmp?"OK":"KO", test_ptn[i].line, test_ptn[i].expected);
		res= res && tmp;
		}
	return !res;
}
