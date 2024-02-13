#define CACHE_LINE_TYPE_PLAIN 1
#define CACHE_LINE_TYPE_CDP 1

#define CACHE_LINE_REGEX " *L[23](CODE)*:[0-9]+=([0-9a-f]+)(.*)\n"
#define CACHE_LINE_REGEX_GRP_NUM(type)(type==CACHE_LINE_TYPE_CDP?4:3)
#define CACHE_LINE_REGEX_GRP_TAIL(type) (type==CACHE_LINE_TYPE_CDP?3:2)
#define CACHE_LINE_REGEX_GRP_BITMASK(type) (type==CACHE_LINE_TYPE_CDP?2:1)
#define CACHE_LINE_FORMAT(type) (type==CACHE_LINE_TYPE_CDP?"L%dCODE:%d=%%0%dlx%%s\n":"L%d:%d=%%0%dlx%%s\n")

//Resource Control feature related constants
#define RTLA_CPUF_RDT_A         0x1
#define RTLA_CPUF_CAT_L3        0x2
#define RTLA_CPUF_CAT_L2        0x4
#define RTLA_CPUF_CDP_L3        0x8
#define RTLA_CPUF_CDP_L2        0x10
#define RTLA_CPUF_CQM_LLC       0x20
#define RTLA_CPUF_CQM_OCCUP_LLC 0x40
#define RTLA_CPUF_CQM_MBM_TOTAL 0x80
#define RTLA_CPUF_CQM_MBM_LOCAL 0x100
#define RTLA_CPUF_MBA           0x200



struct cache_info {
        bool                    enabled;
        int                     cache_size;
        int                     number;
        int                     type;
        unsigned int            *bimask;
        int16_t                 *cache_id_map;
}

struct resctrl_info {
        char                    path[MAX_PATH];
        struct cache_info       cache_l3;
        struct cache_info       cache_l2;
        char                    resctl_name[8];

}

int parse_cpu_features(void) {
        char line[CPU_LINE_BUF_SIZE];
        int features = 0;
        FILE* file;

        file = fopen("/proc/cpuinfo", "r");
        if (file == NULL) {
                err_msg("Can not open /proc/cpuinfo");
                exit(EXIT_FAILURE);
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









// Helper function to parse a hexadecimal string into an integer
int parse_hex(char *hex) {
    return (int)strtol(hex, NULL, 16);
}

// Helper function to determine if a line is a cache line and return the cache data
char* is_cache_line(char *line) {
	char *tmp = NULL;
	while (*line && isspace(*line)) {
		line++;
	}

	if (strncmp(line, "L3", 2) == 0 || strncmp(line, "L2", 2) == 0)
		tmp = strchr(line, ':');
	return tmp?tmp+1:tmp;
}


// Helper function to parse cache IDs and values and fill cache info
struct cache_info *parse_cacheid(char *cacheid_seq) {
	struct cache_info *cache = (struct cache_info*) malloc(sizeof(struct cache_info));

	if (cache == NULL) {
        	fprintf(stderr, "Memory allocation failed\n");
		exit(EXIT_FAILURE);
	}

	// count cacheid
	cache->num = 1;
	while *(cacheid_seq + i) {
		if (*(cacheid_seq + i) == ';') {
			cache->num++;
			*(cacheid_seq + i)= '\0';
		}
	}
	

	char *token = strtok(cacheid_seq, ";");
    int max_cache_id = 0;
    while (token) {
        int cache_id;
        sscanf(token, "%d", &cache_id);
        max_cache_id = cache_id > max_cache_id ? cache_id : max_cache_id;
        token = strtok(NULL, ";");
    }
    cache->number = max_cache_id + 1;

    cache->cache_id_map = (int16_t*)malloc((max_cache_id + 1) * sizeof(int16_t));
    if (cache->cache_id_map == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }
    memset(cache->cache_id_map, -1, (max_cache_id + 1) * sizeof(int16_t));

    // Fill cache ID map
    token = strtok(cacheid_seq, ";");
    while (token) {
        int cache_id, value;
        sscanf(token, "%d=%x", &cache_id, &value);
        cache->cache_id_map[cache_id] = value;
        token = strtok(NULL, ";");
    }

    return cache;
}

// Function to parse the cache info from a file
int parse_cache(FILE *f, struct resctrl_info *r) {
    char line[MAX_LINE_LENGTH];
    while (fgets(line, sizeof(line), f)) {
        char *cache_data = is_cache_line(line);
        if (cache_data != NULL) {
            struct cache_info *cache = parse_cacheid(cache_data);
            if (strncmp(line, "L3", 2) == 0) {
                r->cache_l3 = *cache;
            } else if (strncmp(line, "L2", 2) == 0) {
                r->cache_l2 = *cache;
            }
            free(cache);
        }
    }
    return 0;
}












resctrl_info *init_resctrl(int request_features, amount int) {
	int current_system_features, cache_size;
	resctrl_info *cache_state;

	current_system_features = parse_cpu_features();
	if (current_system_features & request_features != request_features) {
		ret_code = RTLA_NOSUPFEATURE;
		goto error;
	}

	resctrl_path = malloc(MAX_PATH);
	if (!resctrl_path) {
		ret_code = RTLA_NOMEM;
		goto error;
	}

	retval = find_mount("resctrl", resctrl_path, sizeof(resctrl_path));
	if (!retval) {
		ret_code = RTLA_NORESCTRLMOUNT;
		goto cleanup1;
	}

	// len("resctrl/schemata\0") = 17
	resctrl_schemata_fn = malloc(strlen(resctrl_path) 17);
	file = fopen(resctrl_schemata_fn, "r");
	if (file == NULL) {
		ret_code = RTLA_NOFILE;
		goto cleanup1;
	}

	cache_state = malloc(sizeof(resctrl_info));
	if (!resctrl_info) {
		ret_code = RTLA_NOMEM;
		goto cleanup3;
	}

	

	while (fgets(buffer, sizeof(buffer), file)) {
		reti = regcomp(&regex, CACHE_LINE_REGEX, REG_EXTENDED);
		if (reti) {
			ret_code = RTLA_NOSTR;
			goto 
		}
	cache_state = fetch_cache_state(request_features);

	max_available_cache = max_contiguos_meme_avail(cache_size, current_mask, outmask);
	if (amount>max_available_cache.)
		return 0;
cleanup:
failure:
}
