#include <stdio.h>

#include "../include/resctrl_util.h"

struct is_cache_line_test_pattern {
	char	*line;
	int	expected;
};


int main() {
	struct is_cache_line_test_pattern test_patterns[]={
	{"    MB:0=2048", NO_CACHE_LINE},
	{"    L3:0=ffff", L3_LINE},
	{"    L2:0=ffff", L2_LINE},
	{"    L3DATA:0=ffff", NO_CACHE_LINE},
	{"    L3CODE:0=ffff", L3_LINE},
	{"    L2DATA:0=ffff", NO_CACHE_LINE},
	{"    L2CODE:0=ffff", L2_LINE},
	{"sbiriguto", NO_CACHE_LINE},
	{"    antani:pippo", NO_CACHE_LINE}
	};
	int i, tmp, res = 1, tmp2;

	for (i=0; i<sizeof(test_patterns)/sizeof(struct is_cache_line_test_pattern); i++) {
		tmp = is_cache_line(test_patterns[i].line);
		tmp2 = tmp == test_patterns[i].expected;
		printf("[%s] Test pattern=\"%s\"\t ==> expected=%d, actua=%d\n", tmp2?"OK":"KO", test_patterns[i].line, test_patterns[i].expected, tmp);
		res = res && tmp2;
	}
	return !res;
}
