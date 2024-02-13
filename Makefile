

bin/resctrl_util.o: src/resctrl_util.c include/resctrl_util.h
	gcc -c src/resctrl_util.c  -o bin/resctrl_util.o

test: bin/test_get_cache_ids_test bin/test_max_contiguos_mem_avail bin/test_cpu_features bin/test_is_cache_line bin/test_parse_hex
	./test.sh

bin/test_get_cache_ids_test: test/test_get_cache_ids_test.c bin/resctrl_util.o
	gcc test/test_get_cache_ids_test.c bin/resctrl_util.o -o bin/test_get_cache_ids_test

bin/test_max_contiguos_mem_avail: test/test_max_contiguos_mem_avail.c bin/resctrl_util.o
	gcc test/test_max_contiguos_mem_avail.c bin/resctrl_util.o -o bin/test_max_contiguos_mem_avail

bin/test_cpu_features: test/test_cpu_features.c bin/resctrl_util.o
	gcc test/test_cpu_features.c bin/resctrl_util.o -o bin/test_cpu_features

bin/test_is_cache_line: test/test_is_cache_line.c bin/resctrl_util.o
	gcc test/test_is_cache_line.c bin/resctrl_util.o -o bin/test_is_cache_line

bin/test_parse_hex: test/test_parse_hex.c bin/resctrl_util.o
	gcc test/test_parse_hex.c bin/resctrl_util.o -o bin/test_parse_hex

clean:
	rm -rf bin/*.o bin/test_*

