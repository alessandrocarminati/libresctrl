#include <limits.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <linux/limits.h>

#define CACHE_LINE_TYPE_PLAIN 1
#define CACHE_LINE_TYPE_CDP 1

#define CACHE_LINE_REGEX " *L[23](CODE)*:[0-9]+=([0-9a-f]+)(.*)\n"
#define CACHE_LINE_REGEX_GRP_NUM(type)(type==CACHE_LINE_TYPE_CDP?4:3)
#define CACHE_LINE_REGEX_GRP_TAIL(type) (type==CACHE_LINE_TYPE_CDP?3:2)
#define CACHE_LINE_REGEX_GRP_BITMASK(type) (type==CACHE_LINE_TYPE_CDP?2:1)
#define CACHE_LINE_FORMAT(type) (type==CACHE_LINE_TYPE_CDP?"L%dCODE:%d=%%0%dlx%%s\n":"L%d:%d=%%0%dlx%%s\n")

#define MAX_CPUS 2048
#define CPU_LINE_BUF_SIZE 	2048
#define RTLA_CPUF_RDT_A		0x1
#define RTLA_CPUF_CAT_L3	0x2
#define RTLA_CPUF_CAT_L2	0x4
#define RTLA_CPUF_CDP_L3	0x8
#define RTLA_CPUF_CDP_L2	0x10
#define RTLA_CPUF_CQM_LLC	0x20
#define RTLA_CPUF_CQM_OCCUP_LLC	0x40
#define RTLA_CPUF_CQM_MBM_TOTAL	0x80
#define RTLA_CPUF_CQM_MBM_LOCAL	0x100
#define RTLA_CPUF_MBA		0x200


#define NO_CACHE_LINE		0
#define L2_LINE			2
#define L3_LINE			3

struct cache_info {
	bool			enabled;
	int			cache_size;
	int			number;
	int			type;
	unsigned int		*bimask;
	int16_t			*cache_id_map;
};

struct resctrl_info {
	char			path[PATH_MAX];
	struct cache_info	cache_l3;
	struct cache_info	cache_l2;
	char			resctl_name[8];

};


int nproc(void);

/**
 * @brief [parse_cpu_features] Parses cache-related CPU features into a bitwise 32-bit integer.
 *
 * This function retrieves cache-related CPU features and packs them into a bitwise 32-bit integer.
 * The definition of the features is coupled with this function.
 * It does not take any input parameters and does not depend on any CPU feature to work.
 * However, it requires the procfs filesystem to be mounted to access CPU-related information.
 *
 * @return A 32-bit integer representing cache-related CPU features packed bitwise.
 */
int parse_cpu_features(void);

/**
 * @brief [get_cache_ids] Retrieves cache IDs for L3 and L2 caches for each CPU in the system.
 *
 * This function allocates memory for two arrays of int16_t pointers, one for L3 cache IDs
 * and one for L2 cache IDs, and fills them with cache IDs corresponding to each CPU in the system.
 *
 * Before calling this function, it's required to ensure that the 'resctl' feature is checked
 * and the sysfs filesystem is mounted.
 * The 'resctl' feature can be checked by verifying:
 * (parse_cpu_features() | RTLA_CPUF_CAT_L3) || (parse_cpu_features() | RTLA_CPUF_CAT_L2)
 *
 * @param cache_ids_l3 Pointer to an int16_t pointer array for storing L3 cache IDs.
 *                   This array will be allocated by the function and filled with cache IDs.
 * @param cache_ids_l2 Pointer to an int16_t pointer array for storing L2 cache IDs.
 *                   This array will be allocated by the function and filled with cache IDs.
 * @param num_cpus     An integer specifying the number of CPUs in the system.
 *                   This determines the size of the arrays to allocate.
 */
void get_cache_ids(int16_t** cache_ids_l3, int16_t** cache_ids_l2, int num_cpus);

/**
 * @brief [make_bitmask] Generates a bitmask for a specific request.
 *
 * This function is a helper function used by other functions such as 'max_contiguous_mem_avail'
 * and 'best_fitting_block'. It is not intended for standalone use.
 *
 * It generates a bitmask corresponding to a precise memory request based on the maximum
 * sequence end, maximum contiguous memory, and size specified.
 *
 * @param max_sequence_end Maximum sequence end value.
 * @param max_contiguous  Maximum contiguous memory value.
 * @param size      Size of the memory request.
 *
 * @return A 64-bit unsigned integer representing the generated bitmask.
 */
uint64_t make_bitmask(int max_sequence_end, int max_contiguous, int size);

/**
 * @brief [max_contiguos_mem_avail] Calculates the size of the largest contiguous memory block available.
 *
 * This function takes a bitmask pattern string, typically representing the content of
 * /sys/fs/resctrl/schemata, and determines the size of the largest contiguous memory block available.
 *
 * It takes a mem_size parameter representing the cache size, a bitmask parameter
 * representing the cache state, and a pointer outmask where the mask for the suggested size is reported.
 *
 * @param mem_size  Size of the memory block.
 * @param bitmask   A string representing the cache state (bitmask pattern).
 * @param outmask   Pointer to a 64-bit unsigned integer where the mask for the suggested size will be reported.
 *
 * @return The size of the largest contiguous memory block available.
 */
int max_contiguos_mem_avail(int mem_size, char* bitmask, uint64_t *outmask);

/**
 * @brief [best_fitting_block] Finds the best fitting memory block size.
 *
 * This function attempts to generate a bitmask for the specified size based on the current cache state,
 * represented by the provided bitmask pattern string.
 *
 * It searches for a contiguous space in memory to allocate the requested size.
 *
 * If successful, it returns the bitmask for the specified amount of memory.
 * If no suitable block is found, it returns 0.
 *
 * @param mem_size   Size of the memory.
 * @param bitmask    A string representing the current cache state.
 * @param requested  The size of memory requested.
 *
 * @return The bitmask for the specified amount of memory if a suitable block is found; otherwise, 0.
 */
uint64_t best_fitting_block(int mem_size, char* bitmask, int requested);



int is_cache_line(char *line);
uint64_t parse_hex(char *hex);
