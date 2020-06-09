#include <time.h>


#ifndef UTIL
    #include "utilities.h" 
    #define UTIL
#endif

#ifndef HD
    #include "header.h"
    #define HD
#endif

void list_all(int archive, char *header, bool vflag, bool sflag); 
void list_file(int archive, char *header, bool vflag);
void find_list_file(int archive, char* header, char *fname,
        bool vflag, bool sflag); 
void verbose_list(char* header); 
void read_perms(char* perms, char* formatted_perm); 
void read_time(char* times, char* buf); 
