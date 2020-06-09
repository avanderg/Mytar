#ifndef UTIL
    #include "utilities.h" 
    #define UTIL
#endif

#ifndef HD
    #include "header.h"
    #define HD
#endif 

#ifndef ARRAYS_ON_STACK
    #define ARRAYS_ON_STACK
#endif

/*
#ifndef ARRAYS_ON_HEAP
    #define ARRAYS_ON_HEAP
#endif
*/

#include <sys/types.h>
#include <dirent.h>
#include <errno.h>

void extract_archive(int archive, bool vflag, bool sflag);
void extract_item(char type, char* header, char* fname, 
        int archive, bool vflag); 
void extract_file(char* header, char* fname, int archive, 
        bool vflag);
void write_file(int archive, int fd_out, char* header,
        bool vflag); 
void extract_dir(char* header, char* fname, int archive, 
        bool vflag); 
void extract_link(char* header, char* fname, int archive, 
        bool vflag); 
void find_extract_file(int archive, char *fname, 
        bool vflag, bool sflag);
char* get_name_at_index(char *header, char* name, int index); 
