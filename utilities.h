#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define DSK_BLK 4096
#define BLK 512
#define MAX_PATH 256
#define MAGIC "ustar"
#define VERSION "00"
#define VERSION_CHAR '0'
#define SPACE 32 
#define ZERO_SZ "0"
#define SZ_OBUF 16
#define OCTAL 8



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

#define MAX_OSTR 68719476735 /* octal 77777777777 */

typedef enum bool {false, true} bool;

uint32_t ostr_to_int(char *chksum_str); 
char* int_to_ostr(unsigned int val, size_t size, char *buf); 
bool check_end(int archive);
void seek_to_header(int archive, char* header); 
