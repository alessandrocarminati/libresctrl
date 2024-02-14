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
#define LINE_BUF_SIZE 		2048
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
	unsigned int		*bitmask;
	int16_t			*cache_id_map;
};

struct resctrl_info {
	char			path[PATH_MAX];
	struct cache_info	*cache_l3;
	struct cache_info	*cache_l2;
	char			resctl_name[8];
};

/**
 * @brief [nproc] Returns the number of CPUs in the system using sched_getaffinity.
 *
 * This function retrieves the number of CPUs in the system by querying the system's
 * CPU affinity settings using the sched_getaffinity function. It provides an efficient
 * way to determine the number of available CPUs in the system.
 *
 * @return The number of CPUs in the system. Returns -1 if an error occurs during retrieval.
 */
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

//VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVto review
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
 * @return 0 on failure
 */
int get_cache_ids(int16_t** cache_ids_l3, int16_t** cache_ids_l2, int num_cpus);

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

/**
 * @brief [is_cache_line] Determines if a line from the res_ctrl schemata file in sysfs describes a code cache line.
 *
 * This function processes lines from the res_ctrl schemata file in sysfs. It detects lines
 * that are relevant to the particular use case of code cache, which may vary based on how
 * the res-ctrlfs is mounted.
 *
 * @param line A pointer to the line to be processed.
 * @return An integer value representing the type of cache line:
 *         - L3_LINE if the line describes an L3 code cache.
 *         - L2_LINE if the line describes an L2 code cache.
 *         - NO_CACHE_LINE if the line does not describe a cache line.
 */
int is_cache_line(char *line);

/**
 * @brief [parse_hex] Converts a hexadecimal string to an unsigned 64-bit integer.
 *
 * This internal helper function is used to convert a hexadecimal string into
 * the corresponding unsigned 64-bit integer value.
 *
 * @param hex A pointer to the hexadecimal string to be converted.
 * @return The unsigned 64-bit integer value corresponding to the hexadecimal string.
 */
uint64_t parse_hex(char *hex);

//VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV needs review
/**
 * @brief [parse_cache] Parses a res_ctrl schemata file to fill a struct resctrl_info object.
 *
 * This function reads a given res_ctrl schemata file and parses its contents to
 * populate a struct resctrl_info object with relevant information. The function
 * dynamically allocates the resctrl_info object and all its subobjects.
 *
 * @param fn The filename of the res_ctrl schemata file to be parsed.
 * @return A pointer to the dynamically allocated struct resctrl_info object
 *         containing the parsed information. Returns NULL if there is an error
 *         during parsing or memory allocation.
 * @note The returned object and all its subobjects must be freed using the
 *       dispose_resctrl_info function when no longer needed to prevent memory leaks.
 */
struct resctrl_info *parse_cache(char *fn, char *l2cacheid_path_fmt, char *l3cacheid_path_fmt);

/**
 * @brief [parse_cacheid] Parses a cache line string and fills a struct cache_info object.
 *
 * This internal function is used by the parse_cache function. It takes a string
 * known to be a cache line and extracts its contents into a dynamically allocated
 * struct cache_info object. The size of the cache_info object is calculated based
 * on the contents of the cache line before allocation.
 *
 * @param line A pointer to the cache line string to be parsed.
 * @param c A pointer to the struct cache_info object where the parsed information
 *          will be stored. The memory for the cache_info object is allocated within
 *          this function.
 */
void parse_cacheid(char *line, struct cache_info *c);

/**
 * @brief [dispose_resctrl_info] Frees memory allocated for a struct resctrl_info object and
 * its subobjects.
 *
 * This function frees the memory allocated for a struct resctrl_info object and
 * all its subobjects. It should be used when the resctrl_info object and its
 * subobjects are no longer needed to prevent memory leaks.
 *
 * @param r A pointer to the struct resctrl_info object to be freed.
 */
void dispose_resctrl_info(struct resctrl_info *r);

/**
 * @brief [get_cache_size] Retrieves the cache size for a given CPU and cache level using sysfs.
 *
 * This function accesses sysfs to determine the cache size for a specified CPU and cache level.
 * The supported cache levels are L3 and L2; for all other levels, the function reports an error,
 * indicated by the return value -1. The sysfs filesystem must be mounted for the function to work.
 *
 * @param cpun The CPU number for which the cache size is to be retrieved.
 * @param level The cache level (L3 or L2) for which the size is to be retrieved.
 * @return The size of the specified cache level for the given CPU in bytes. Returns -1 if an error occurs
 *         or if the specified cache level is not supported.
 */
int get_cache_size(int cpun, int level);

