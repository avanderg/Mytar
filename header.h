#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#ifdef __arm__ 
    #include <sys/sysmacros.h>
#endif
#include <fcntl.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <pwd.h>
#include <grp.h>

#ifndef UTIL
    #include "utilities.h" 
    #define UTIL
#endif


/*
#define BLK 512
#define MAGIC "ustar"
#define VERSION "00"
#define VERSION_CHAR '0'
#define SPACE 32 
#define ZERO_SZ "0"
#define SZ_OBUF 16


#define SZ_NAME 100
#define SZ_MODE 8
#define SZ_ID 8
#define SZ_SIZE 12
#define SZ_MTIME 12
#define SZ_CHKSUM 8
#define SZ_TYPEFLAG 1
#define SZ_LINKNAME 100
#define SZ_MAGIC 6
#define SZ_VERSION 2
#define SZ_UNAME 32
#define SZ_DEV 8
#define SZ_PREF 155

#define OFF_NAME 0
#define OFF_MODE 100
#define OFF_UID 108
#define OFF_GID 116
#define OFF_SZ 124
#define OFF_MTIME 136
#define OFF_CHKSUM 148
#define OFF_TYPEFLAG 156
#define OFF_LINKNAME 157
#define OFF_MAGIC 257
#define OFF_VERSION 263
#define OFF_UNAME 265 
#define OFF_GNAME 297
#define OFF_DEVMJ 329 
#define OFF_DEVMN 337
#define OFF_PREF 345
*/


/*
typedef struct header header_struct;
struct header {
    char name[100];
    char mode[8];
    char uid[8];
    char gid[8];
    char size[12];
    char mtime[12];
    char chksum[8];
    char typeflag[1];
    char linkname[100];
    char magic[6];
    char version[2];
    char uname[32];
    char gname[32];
    char devmajor[8];
    char devminor[8];
    char prefix[155];
};
*/


char* read_header(int head, char* header);
int write_header(char* fname, char* header); 
void create_name(char* header, char* name); 
char* get_name(char *header, char *name); 
int strip_parent(char *header); 
bool check_header(char* header, bool sflag); 
uint32_t chksum(char* buf); 
int build_header(char *pathname, char* header); 
uint32_t build_mtime(struct timespec *st_mtim); 
char get_type(mode_t mode); 
