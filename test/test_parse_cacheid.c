#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <string.h>
#include <sys/mman.h>

#include "../include/resctrl_util.h"

struct parse_cacheid_ptn {
	char		*line;
	int		expected_number;
	unsigned int	*expected_data;
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

int verify_parse(struct parse_cacheid_ptn *ptn, struct cache_info *c){
	int number, data;

	number = ptn->expected_number==c->number;
	data = verdata(ptn->expected_data, c->bitmask, min(ptn->expected_number, c->number));
	printf("[%s] - line=\"%s\" ==> nummber->%s(%d,%d), data->%s\n", (number && data)?"OK":"KO", ptn->line, number?"OK":"KO", ptn->expected_number, c->number, data?"OK":"KO");
	return number && data;
}

int main() {
	struct parse_cacheid_ptn ptn[] ={
	{"    L3:0=ffff", 1, (unsigned int[]){0xffffU}},
	{"    L3CODE:0=ffff", 1, (unsigned int[]){0xffff}},
	{"    L3CODE:0=ffff;1=ffff",	2,	(unsigned int[]){0xffff, 0xffff}},
	{"    L3CODE:0=ffff;1=ffff;2=fc00",	3,	(unsigned int[]){0xffff, 0xffff, 0xfc00}},
	{"    L2CODE:0=ffff;1=ffff;2=fc00",	3,	(unsigned int[]){0xffff, 0xffff, 0xfc00}},
	{"    L3:0=0001;1=0003;2=0007;3=000f;4=001f;5=003f;6=007f;7=00ff",	8,	(unsigned int[]){0x0001, 0x0003, 0x0007, 0x000f, 0x001f, 0x003f, 0x007f, 0x00ff}}
	};

	int res = 1, i, tmp;
	struct cache_info *c = (struct cache_info *) malloc(sizeof(struct cache_info));

	for (i=0; i< sizeof(ptn)/sizeof(struct parse_cacheid_ptn); i++) {
		parse_cacheid(ptn[i].line, c);
		tmp = verify_parse(&ptn[i], c);
		res= res && tmp;
		if (c->bitmask)
			free(c->bitmask);
	}
	free(c);
	return !res;
}
