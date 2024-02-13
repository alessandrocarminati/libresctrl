

bin/resctrl_util.o: src/resctrl_util.c include/resctrl_util.h
	gcc -c src/resctrl_util.c  -o bin/resctrl_util.o

test: bin/test_get_cache_ids_test bin/test_max_contiguos_mem_avail

bin/test_get_cache_ids_test: test/test_get_cache_ids_test.c bin/resctrl_util.o
	gcc test/test_get_cache_ids_test.c bin/resctrl_util.o -o bin/test_get_cache_ids_test

bin/test_max_contiguos_mem_avail: test/test_max_contiguos_mem_avail.c bin/resctrl_util.o
	gcc test/test_max_contiguos_mem_avail.c bin/resctrl_util.o -o bin/test_max_contiguos_mem_avail
clean:
	rm -rf bin/*.o test_*

