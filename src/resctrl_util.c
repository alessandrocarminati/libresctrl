#define __USE_GNU
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <sched.h>
#include <inttypes.h>
#include <string.h>
#include <limits.h>
#include <ctype.h>

#include "../include/resctrl_util.h"

int nproc(void){
	unsigned long mask[MAX_CPUS];
	unsigned long m;
	int count = 0;
	int i;

	if (sched_getaffinity(0, sizeof(mask), (void*)mask) == 0) {
		for (i = 0; i < MAX_CPUS; i++) {
			m = mask[i];
			while (m) {
				if (m & 1) count++;
				m >>= 1;
			}
		}
	}
	return count;
}

int parse_cpu_features(void) {
	char line[LINE_BUF_SIZE];
	int features = 0;
	FILE* file;

	file = fopen("/proc/cpuinfo", "r");
	if (file == NULL) {
		return features;
	}

	while (fgets(line, sizeof(line), file)) {
		if (strstr(line, "rdt_a")) {
			features |= RTLA_CPUF_RDT_A;
		}
		if (strstr(line, "cat_l3")) {
			features |= RTLA_CPUF_CAT_L3;
		}
		if (strstr(line, "cat_l2")) {
			features |= RTLA_CPUF_CAT_L2;
		}
		if (strstr(line, "cdp_l3")) {
			features |= RTLA_CPUF_CDP_L3;
		}
		if (strstr(line, "cdp_l2")) {
			features |= RTLA_CPUF_CDP_L2;
		}
		if (strstr(line, "cqm_llc")) {
			features |= RTLA_CPUF_CQM_LLC;
		}
		if (strstr(line, "cqm_occup_llc")) {
			features |= RTLA_CPUF_CQM_OCCUP_LLC;
		}
		if (strstr(line, "cqm_mbm_total")) {
			features |= RTLA_CPUF_CQM_MBM_TOTAL;
		}
		if (strstr(line, "cqm_mbm_local")) {
			features |= RTLA_CPUF_CQM_MBM_LOCAL;
		}
		if (strstr(line, "mba")) {
			features |= RTLA_CPUF_MBA;
		}
	}

	fclose(file);
	return features;
}

int get_cache_ids(struct resctrl_info *r, char *fn_fmt_l2, char *fn_fmt_l3) {

	r->cache_id_map_l3 = (int16_t*)malloc(r->ncpu * sizeof(int16_t));
	r->cache_id_map_l2 = (int16_t*)malloc(r->ncpu * sizeof(int16_t));
	if (r->cache_id_map_l3 == NULL || r->cache_id_map_l2 == NULL)
		return 0;

	for (int idx = 0; idx < r->ncpu; ++idx) {
		char path_l3[256];
		char path_l2[256];
		snprintf(path_l3, sizeof(path_l3), fn_fmt_l2?fn_fmt_l3:CACHE_ID_PATH_FMT(3), idx);
		snprintf(path_l2, sizeof(path_l2), fn_fmt_l2?fn_fmt_l2:CACHE_ID_PATH_FMT(2), idx);

		FILE *file_l3 = fopen(path_l3, "r");
		FILE *file_l2 = fopen(path_l2, "r");

		if (file_l3 != NULL) {
			fscanf(file_l3, "%" SCNd16, &(r->cache_id_map_l3[idx]));
			fclose(file_l3);
		} else {
			r->cache_id_map_l3[idx] = -1;
		}

		if (file_l2 != NULL) {
			fscanf(file_l2, "%" SCNd16, &(r->cache_id_map_l2[idx]));
			fclose(file_l2);
		} else {
			r->cache_id_map_l2[idx] = -1;
		}
	}
	return 1;
}

uint64_t make_bitmask(int max_sequence_end, int max_contiguous, int size) {
	if (max_sequence_end < 0 || max_contiguous <= 0 || max_sequence_end < max_contiguous - 1|| max_sequence_end > size) {
		return 0;
	}
	uint64_t bitmask = ((1UL << max_contiguous) - 1) << (max_sequence_end - max_contiguous );
	return bitmask;
}

int max_contiguos_mem_avail(int mem_size, char* bitmask, uint64_t *outmask) {
	int num_bits = strlen(bitmask) * 4;
	int bin_size = mem_size / num_bits;
	int max_contiguous = 0;
	int current_contiguous = 0;
	int max_sequence_end = 0;
	int bitmask_num;
	char *endptr;
	int i;

	bitmask_num = strtol(bitmask, &endptr, 16);

	for (i = 0; i < num_bits; i++) {
		if ((bitmask_num) & 0x1) {
			current_contiguous++;
			if (current_contiguous > max_contiguous) {
				max_contiguous = current_contiguous;
				max_sequence_end = i+1;
			}
		} else {
			current_contiguous = 0;
		}
		bitmask_num >>= 1;
	}

	*outmask = make_bitmask(max_sequence_end, max_contiguous, num_bits);

	return max_contiguous * bin_size;
}

uint64_t best_fitting_block(int mem_size, char* bitmask, int requested) {
	int num_bits = strlen(bitmask) * 4;
	int bin_size = mem_size / num_bits;
	int best_fit = num_bits + 1;
	int current_sequence_end = 0;
	int best_sequence_end = 0;
	int current_fit = 0;
	int bitmask_num;
	int cursor = 0;
	char *endptr;
	int i;

	bitmask_num = strtol(bitmask, &endptr, 16);

	for (i = 0; i < num_bits; i++) {
		if ((bitmask_num) & 0x1) {
			cursor++;
			if (cursor * bin_size >= requested) {
				current_fit = cursor;
				current_sequence_end = i+1;
			}
		} else {
			if ((cursor * bin_size >= requested)&&(current_fit * bin_size < best_fit * bin_size)) {
				best_fit = current_fit;
				best_sequence_end = current_sequence_end;
			}
			cursor = 0;
		}
		bitmask_num >>= 1;
	}
	if (current_fit * bin_size < best_fit * bin_size) {
		best_fit = current_fit;
		best_sequence_end = current_sequence_end;
	}

	return make_bitmask(best_sequence_end, requested / bin_size + 1, num_bits);
}

uint64_t parse_hex(char *hex) {
	return (uint64_t)strtol(hex, NULL, 16);
}

int is_cache_line(char *line) {
	char *tmp;

	while (*line && isspace(*line))
		line++;

	tmp = strchr(line, ':');
	if (tmp == NULL)
		return NO_CACHE_LINE;


	if (strncmp(line, "L3:", 3) == 0)
		return L3_LINE;

	if (strncmp(line, "L3CODE:", 7) == 0)
		return L3CODE_LINE;

	if (strncmp(line, "L2:", 3) == 0)
		return L2_LINE;

	if (strncmp(line, "L2CODE:", 7) == 0)
		return L2CODE_LINE;

	return NO_CACHE_LINE;
}

static int count_items(char *line) {
	int i=0, res=0;
	while (*(line+i++))
		if (line[i]==';')
			res++;
	return res + 1;
}

static char *fetch_item(char *line, struct cache_info *c) {
	char *next_item;
	char *left, *right=line;
	int index;

	next_item = strchr(line, ';');
	if (next_item) {
		*next_item='\0';
		next_item++;
	}

	left = strchr(line, '=') + 1;
	*(left-1)='\0';
	index = atoi(right);
	c->bitmask[index] = parse_hex(left);

	return next_item;
}

void parse_cacheid(char *input, struct cache_info *c) {
	char *current, *line;

	line = (char *) malloc(strlen(input)+1);
	strcpy(line, input);

	c->number = count_items(line);
	if (c->number <= 0)
		return;

	c->bitmask = (unsigned int*)malloc(sizeof(unsigned int)*c->number);
	current=line;
	while (*current!=':')
		current++;
	current++;
	while (current=fetch_item(current, c));
	free(line);
}

// Function to parse the cache info from a file
struct resctrl_info *parse_cache(char *fn, int ncpu, char *l2cacheid_path_fmt, char *l3cacheid_path_fmt) {
	struct resctrl_info *r;
	struct cache_info *c;
	char line[LINE_BUF_SIZE];
	FILE *f;
	int tmp;

	r=NULL;
	f = fopen(fn, "r");
	if (f == NULL)
		goto cleanup;

	while (fgets(line, sizeof(line), f)) {
		tmp = is_cache_line(line);
		if (!tmp)
			continue;
		if (!r) {
			r =(struct resctrl_info*)malloc(sizeof(struct resctrl_info));
			if (r == NULL)
				goto cleanup;
			memset(r, 0, sizeof(struct resctrl_info) );
			strcpy(r->path, fn);
			r->ncpu = ncpu;
		}

		if ((tmp & LINE_MASK)==L3_LINE) {
			r->cache_l3 = (struct cache_info*)malloc(sizeof(struct cache_info));
			c=r->cache_l3;
		} else {
			r->cache_l2 = (struct cache_info*)malloc(sizeof(struct cache_info));
			c=r->cache_l2;
		}
		c->type=tmp & TYPE_MASK;
		if (!get_cache_ids(r, l2cacheid_path_fmt, l3cacheid_path_fmt))
			goto cleanup;

		line[strlen(line)-1]='\0';
		parse_cacheid(line, c);
	}
	fclose(f);
	return r;
cleanup:
	if (f)
		fclose(f);

	printf("parse_cache - file closed\n");
	dispose_resctrl_info(r);
	return NULL;
}

void dispose_resctrl_info(struct resctrl_info *r) {
	if (r) {
		//check caches and free
		if (r->cache_l3){
			if (r->cache_l3->bitmask)
				free(r->cache_l3->bitmask);
			free(r->cache_l3);
		}

		if (r->cache_l2){
			if (r->cache_l2->bitmask)
				free(r->cache_l2->bitmask);
			free(r->cache_l2);
		}

		//check maps and free
		if (r->cache_id_map_l3)
			 free(r->cache_id_map_l3);

		if (r->cache_id_map_l2)
			 free(r->cache_id_map_l2);

		// free main object
		free(r);
	}
}

int convert_size(char *size_str) {
	int multiplier = 1;

	if (isalpha(size_str[strlen(size_str)-1])) {
		switch (tolower(size_str[strlen(size_str)-1])) {
			case 'g':
				multiplier *= 1024;
			case 'm':
				multiplier *= 1024;
			case 'k':
				multiplier *= 1024;
		}
		size_str[strlen(size_str)]='\0';
	}
	int size = atoi(size_str);
	return size * multiplier;
}

int get_cache_size(int cpun, int level){
	char fn[50];
	char buffer[50];
	FILE *f;

	if ((level==3) || (level==2)) {
		snprintf(fn, sizeof(fn), "/sys/devices/system/cpu/cpu%d/cache/index%d/size", cpun, level);
		f = fopen(fn, "r");
		 if (f == NULL)
			return -1;

		if (fscanf(f, "%19s", buffer) != 1) {
			fclose(f);
			return -1;
		}

		fclose(f);
		return convert_size(buffer);
	}
	return -1;
}

int cpulevel2id(int cpu, int level, struct resctrl_info *r) {
	int16_t                 *c;

	if ((level!=2) && (level!=3))
		return -1;

	c=(level==2)?r->cache_id_map_l2:r->cache_id_map_l3;
	if (!c)
		return -1;

	if (cpu > r->ncpu)
		return -1;

	return c[cpu];
}

