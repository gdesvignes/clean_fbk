CPPFLAGS=-O3 -std=c++11

CPPFLAGS+= -DHAVE_GSL
GSL_LIBS = `/usr/bin/gsl-config --libs`
GSL_INCLUDE = `/usr/bin/gsl-config --cflags`

test:	test.C 
	g++ -O3 -std=c++11 -o test test.C 

tmp:	tmp.C 
	g++ $(CPPFLAGS) -c read_header.c write_header.c sigproc_utils.c
	g++ $(CPPFLAGS) -o tmp tmp.C read_header.o write_header.o  sigproc_utils.o

clean_fbk:	clean_fbk.C
	g++ $(CPPFLAGS) -c read_header.c write_header.c sigproc_utils.c
	g++ $(CPPFLAGS) $(GSL_LIBS) $(GSL_INCLUDE) -o clean_fbk clean_fbk.C read_header.o write_header.o  sigproc_utils.o
