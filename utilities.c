#include "utilities.h"

char *int_to_ostr(unsigned int val, size_t size, char *buf) {
    int i;
    if (val > MAX_OSTR) {
        fprintf(stderr, "Value too large\n");
        exit(EXIT_FAILURE);
    }
    for (i=0; i<size; i++) {
        buf[i] = '\0';
    }
    
    snprintf(buf, size, "%o",  val);
    return buf;
}

uint32_t ostr_to_int(char *chksum_str) {
    uint32_t sum;
    char *nptr;
    char chksum_str_cpy[BLK] = {'\0'};

    strcpy(chksum_str_cpy, chksum_str);

    sum = strtol(chksum_str_cpy, &nptr, 8); 

    /*
    if (!sum && !strcmp(chksum_str, nptr)) {
        fprintf(stderr, "string %s has no numbers!\n", chksum_str);
        return 0;
    }
    */

    return sum;
}

bool check_end(int archive) {
    ssize_t size;
    char buf[2*BLK] = {'\0'};
    char zero[2*BLK] = {'\0'};

    if ((size = read(archive, buf, 2*BLK)) < 0) {
        fprintf(stderr, "Failed to read archive to check_end\n");
        perror("read");
        exit(EXIT_FAILURE);
    }
    /*
    printf("size read in for check_end: %lu\n", size);
    */

    if (lseek(archive, -(size), SEEK_CUR) < 0) {
        fprintf(stderr, "Failed to seek in check_end\n");
        perror("lseek");
        exit(EXIT_FAILURE);
    }

    if (!strncmp(buf, zero, 2*BLK)) {
        /* They are equal */
        /*
        printf("Found end of archive\n");
        */
        return false;
    }
    else {
        return true;
    }

}

void seek_to_header(int archive, char* header) {
    ssize_t filesize;
    ssize_t seeksize;

    filesize = ostr_to_int(&header[OFF_SZ]);
    /*
    printf("Seeking %lu\n", filesize + (BLK-filesize%BLK));
    printf("filesize mod BLK: %lu\n", filesize%BLK);
    */
    if (filesize && filesize%BLK) {
        if ((seeksize = lseek(archive, filesize + (BLK - filesize%BLK), 
                        SEEK_CUR)) < 0) {
            perror("lseek");
            exit(EXIT_FAILURE);
        }
    }
    else if (filesize) {
        if ((seeksize = lseek(archive, filesize, SEEK_CUR)) < 0) {
            perror("lseek");
            exit(EXIT_FAILURE);
        }
    }

    /*
    printf("seeked %lu to next header\n", filesize + (BLK - filesize%BLK));
    printf("filsize: %lu\n", filesize);
    */

}
void *safe_calloc(size_t nmemb, size_t size) {
    void *ptr;
    if (!(ptr = calloc(nmemb, size))) {
        perror("calloc");
        exit(EXIT_FAILURE);
    }
    return ptr;
}
