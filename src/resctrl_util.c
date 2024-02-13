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

void get_cache_ids(int16_t** cache_ids_l3, int16_t** cache_ids_l2, int num_cpus) { int cpu_count = num_cpus;

	*cache_ids_l3 = (int16_t*)malloc(cpu_count * sizeof(int16_t));
	*cache_ids_l2 = (int16_t*)malloc(cpu_count * sizeof(int16_t));
	if (*cache_ids_l3 == NULL || *cache_ids_l2 == NULL) {
		perror("Memory allocation error");
		exit(EXIT_FAILURE);
	}

	for (int idx = 0; idx < cpu_count; ++idx) {
		char path_l3[256];
		char path_l2[256];
		snprintf(path_l3, sizeof(path_l3), "/sys/devices/system/cpu/cpu%d/cache/index3/id", idx);
		snprintf(path_l2, sizeof(path_l2), "/sys/devices/system/cpu/cpu%d/cache/index2/id", idx);

		FILE *file_l3 = fopen(path_l3, "r");
		FILE *file_l2 = fopen(path_l2, "r");

		if (file_l3 != NULL) {
			fscanf(file_l3, "%" SCNd16, &((*cache_ids_l3)[idx]));
			fclose(file_l3);
		} else {
			(*cache_ids_l3)[idx] = -1;
		}

		if (file_l2 != NULL) {
			fscanf(file_l2, "%" SCNd16, &((*cache_ids_l2)[idx]));
			fclose(file_l2);
		} else {
			(*cache_ids_l2)[idx] = -1;
		}
	}
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


/**
 * cache_alloc_check_prerequisite - verifies prerequisites for cache-alloc
 * are met, if anything's missing stops execution.
 *
 * on success, returns the path where the resctrl filesystem is mounted.
 */
/*
resctrl_info *cache_alloc_check_prerequisite(int requested){
	int cpu_features;
	char *resctrl_path;

	resctrl_path = malloc(MAX_PATH);
	if (!resctrl_path) {
		err_msg("Can't allocate memory\n");
		exit(EXIT_FAILURE);
	}
	cpu_features = parse_cpu_features();
	if (!(cpu_features & RTLA_CPUF_CAT_L3)) {
		err_msg("Resource Control features not supported on this system\n");
		exit(EXIT_FAILURE);
	}
	retval = find_mount("resctrl", resctrl_path, sizeof(resctrl_path));
	if (!retval) {
		err_msg("Did not find resctrl mount point\n");
		exit(EXIT_FAILURE);
	}

	file = fopen("/sys/fs/resctrl/schemata", "r");
	if (file == NULL) {
		err_msg("Failed to access /sys/fs/resctrl/schemata\n");
		exit(EXIT_FAILURE);
	}
	while (fgets(buffer, sizeof(buffer), file)) {
		reti = regcomp(&regex, CACHE_LINE_REGEX, REG_EXTENDED);
		if (reti) {
			err_msg("Could not compile regex\n");
			exit(EXIT_FAILURE);
		}


	return resctrl_path;
}
*/

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

	if ((strncmp(line, "L3:", 3) == 0) || (strncmp(line, "L3CODE:", 7) == 0))
		return L3_LINE;

	if ((strncmp(line, "L2:", 3) == 0) || (strncmp(line, "L2CODE:", 7) == 0))
		return L2_LINE;

	return NO_CACHE_LINE;
}

int count_items(char *line) {
	int i=0, res=0;
//	printf("count_items - line='%s'\n", line);
	while (*(line+i++))
		if (line[i]==';')
			res++;
//	printf("count_items - res=%d\n", res + 1);
	return res + 1;
}

char *fetch_item(char *line, struct cache_info *c) {
	char *next_item;
	char *left, *right=line;
	int index;

//	printf("fetch_item - line=\"%s\", line=%08lx, c=%08lx\n", line, line, c);
	next_item = strchr(line, ';');
//	printf("fetch_item - next_item=%08lx\n", next_item);
	if (next_item) {
		*next_item='\0';
		next_item++;
	}

	left = strchr(line, '=') + 1;
//	printf("fetch_item - left=\"%s\", left=%08lx, left-1=%08lx -> %c\n", left, left, left-1, *(left-1));
//	printf("fetch_item - right=\"%s\", right=%08lx,  left=\"%s\", left=%08lx\n", right, right, left, left);
	*(left-1)='\0';
//	printf("fetch_item - right=\"%s\", right=%08lx,  left=\"%s\", left=%08lx\n", right, right, left, left);
	index = atoi(right);
	c->bitmask[index] = parse_hex(left);

	return next_item;
}

void parse_cacheid(char *input, struct cache_info *c) {
	char *current, *line;

	line = (char *) malloc(strlen(input)+1);
	strcpy(line, input);

//	printf("parse_cacheid - input=%08lx, line=%08lx\n", input, line);
//	printf("parse_cacheid - c=%08lx\n", c);
	c->number = count_items(line);
//	printf("parse_cacheid - c->number=%d\n", c->number);
	if (c->number <= 0)
		return;

	c->bitmask = (unsigned int*)malloc(sizeof(unsigned int)*c->number);
//	printf("parse_cacheid - c->bitmask=%08lx\n", c->bitmask);
	current=line;
	while (*current!=':')
		current++;
	current++;
//	printf("parse_cacheid - line=%08lx, current=%08lx\n", line, current);
	while (current=fetch_item(current, c));
	free(line);
}

// Function to parse the cache info from a file
struct resctrl_info *parse_cache(char *fn, struct resctrl_info *r) {
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
			goto cleanup;

		if (!r) {
			r =(struct resctrl_info*)malloc(sizeof(struct resctrl_info));
			if (r == NULL)
				goto cleanup;
			memset(r, 0, sizeof(struct resctrl_info) );
		}

		if (tmp==L3_LINE) {
			r->cache_l3 = (struct cache_info*)malloc(sizeof(struct cache_info));
			c=r->cache_l3;
		} else {
			r->cache_l2 = (struct cache_info*)malloc(sizeof(struct cache_info));
			c=r->cache_l2;
		}
		parse_cacheid(line, c);
	}
	fclose(f);
	return r;
cleanup:
	if (f)
		fclose(f);
	if (r) {
		if (r->cache_l3)
			free(r->cache_l3);
		if (r->cache_l2)
			free(r->cache_l2);
	free(r);
	}
	return NULL;
}
