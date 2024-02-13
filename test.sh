#!/bin/sh
result=success
./bin/test_cpu_features > log/test_cpu_features.log || result=fail
./bin/test_get_cache_ids_test >log/test_get_cache_ids_test.log || result=fail
./bin/test_is_cache_line >log/test_is_cache_line.log || result=fail
./bin/test_max_contiguos_mem_avail >log/test_max_contiguos_mem_avail.log || result=fail
./bin/test_parse_hex >log/test_parse_hex.log || result=fail
./bin/test_parse_cacheid >log/test_parse_cacheid.log || result=fail
echo $result
