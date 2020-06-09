CC = gcc
CFLAGS = -Wall -g
UNITY = Unity/src
HEADERS = $(UNITY)/unity.h $(UNITY)/unity_internals.h
PROGS = mytar test test_header
FILES = out.txt

all: mytar 

mytar: mytar.o creation.o listing.o extraction.o header.o utilities.o
	$(CC) $(CFLAGS) -o mytar mytar.o \
		creation.o listing.o extraction.o header.o utilities.o \
		mytar.h creation.h listing.h extraction.h header.h utilities.h
test_extract:
	./mytar cvf archive.tar normal_dir
	mv normal_dir normal_dir_orig
	./mytar xvf archive.tar

test_extract_r:
	rm -rf normal_dir
	mv normal_dir_orig normal_dir

mytar.o: mytar.c mytar.h
	$(CC) $(CFLAGS) -c mytar.c 

run_test: test_header
	./test_header

run_test_v: test_header
	valgrind ./test_header

test_tar: test_tar.o unity.o mytar.o
	$(CC) $(CFLAGS) -o test_tar test_tar.o unity.o test_tar.h  \
		$(HEADERS) $(UNITY)/unity.h mytar.h

test_tar.o: test_tar.c test_tar.h 
	$(CC) $(CFLAGS) -c test_tar.c 

test_header: test_header.o unity.o header.o utilities.o
	$(CC) $(CFLAGS) -o test_header test_header.o unity.o header.o utilities.o\
		test_header.h $(HEADERS) $(UNITY)/unity.h header.h utilities.h
test_header.o: test_header.c test_header.h
	$(CC) $(CFLAGS) -c test_header.c 

unity.o: $(UNITY)/unity.c $(HEADERS)
	$(CC) $(CFLAGS) -c $(UNITY)/unity.c

header.o: header.c header.h utilities.o
	$(CC) $(CFLAGS) -c header.c

creation.o: creation.c creation.h utilities.o
	$(CC) $(CFLAGS) -c creation.c

listing.o: listing.c listing.h utilities.o
	$(CC) $(CFLAGS) -c listing.c

extraction.o: extraction.c extraction.h utilities.o
	$(CC) $(CFLAGS) -c extraction.c

utilities.o: utilities.c utilities.h
	$(CC) $(CFLAGS) -c utilities.c

install:
	sudo cp mytar /usr/local/bin

clean:
	rm -f *.o $(PROGS) $(FILES)
