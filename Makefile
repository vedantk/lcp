ccflags=-Wextra -Os
cc=g++

lcp: src/*
	mkdir -p bin/
	make bin/bmp.o bin/common.o bin/elbaz_em.o bin/filter.o bin/hist.o bin/image.o bin/lung_analysis.o bin/pgm.o bin/region_growing.o
	$(cc) $(ccflags) src/main.cpp bin/*.o -o $@

bin/bmp.o: src/image.* src/bmp.cpp 
	$(cc) -c $(ccflags) src/bmp.cpp -o $@
	
bin/common.o: src/common.* 
	$(cc) -c $(ccflags) src/common.cpp -o $@

bin/elbaz_em.o: src/image.* src/elbaz_em.cpp 
	$(cc) -c $(ccflags) src/elbaz_em.cpp -o $@

bin/filter.o: src/image.* src/filter.cpp 
	$(cc) -c $(ccflags) src/filter.cpp -o $@

bin/hist.o: src/image.* src/hist.cpp 
	$(cc) -c $(ccflags) src/hist.cpp -o $@

bin/image.o: src/image.*  
	$(cc) -c $(ccflags) src/image.cpp -o $@

bin/lung_analysis.o: src/image.* src/lung_analysis.cpp
	$(cc) -c $(ccflags) src/lung_analysis.cpp -o $@

bin/pgm.o: src/image.* src/pgm.cpp
	$(cc) -c $(ccflags) src/pgm.cpp -o $@

bin/region_growing.o: src/image.* src/region_growing.cpp
	$(cc) -c $(ccflags) src/region_growing.cpp -o $@

clean:
	rm bin/*.o
	rm lcp

devel:
	make lcp ccflags="-g -Wextra -D_DEVEL"
