#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#ifndef UTIL
    #include "utilities.h" 
    #define UTIL
#endif

#ifndef HD
    #include "header.h"
    #define HD
#endif

#ifndef CREATE 
    #include "creation.h"
    #define CREATE 
#endif

#ifndef EXTRACT
    #include "extraction.h"
    #define EXTRACT
#endif

#ifndef LIST 
    #include "listing.h"
    #define LIST 
#endif

#ifndef ARRAYS_ON_STACK
    #define ARRAYS_ON_STACK
#endif

/*
#ifndef ARRAYS_ON_HEAP
    #define ARRAYS_ON_HEAP
#endif
*/

void print_usage(void); 
void check_flags(bool cflag, bool tflag, bool xflag);
void creation_mode(int argc, char *argv[], bool vflag, bool sflag); 
int write_end_archive(int fd_out);
void listing_mode(int argc, char *argv[], bool vflag, bool sflag);
void extraction_mode(int argc, char *argv[], bool vflag, bool sflag);
