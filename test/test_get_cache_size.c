#include <stdio.h>

#include "../include/resctrl_util.h"

struct cache_size_test_ptn {
	int	cpu;
	int	level;
	int	expect; // 1 stands for a number, 0 for a error
};

int ge0(int n){
	return n>=0;
}

int l0(int n){
	return n<0;
}

int main(){
	int cache_size, i, res=1;
	struct cache_size_test_ptn ptn[]={
		{0, 2, 1},
		{0, 3, 1},
		{0, 1, 0},	// only chache level 2 and 3 are in scope
		{0, 4, 0},	// only chache level 2 and 3 are in scope
		{32768, 2, 0}
	};

	for (i=0; i<sizeof(ptn)/sizeof(struct cache_size_test_ptn); i++) {
		cache_size = get_cache_size(ptn[i].cpu, ptn[i].level);
		printf("[%s] - test cache size cpu %d level %d == %d\n", (ptn[i].expect?&ge0:&l0)(cache_size)?"OK":"KO", ptn[i].cpu, ptn[i].level, cache_size);
		res = res && (ptn[i].expect?&ge0:&l0)(cache_size);
	}
	return !res;
}
