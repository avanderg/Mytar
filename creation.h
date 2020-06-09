#ifndef UTIL
    #include "utilities.h" 
    #define UTIL
#endif

#ifndef HD
    #include "header.h"
    #define HD
#endif 

#include <errno.h>
#include <dirent.h>

int add_to_archive(char *arch_name, int archive, char* fname, bool vlfag, 
        bool sflag); 
bool cmp(char *arch_name, char *fname); 
