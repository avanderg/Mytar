#include "extraction.h"

void extract_archive(int archive, bool vflag, bool sflag) {

    char type; /* To store typeflag */
    int i; /* Generic loop index */

    char name[MAX_PATH] = {'\0'}; /* The name of the file with ../ if needed */
    char header[BLK] = {'\0'}; /* Header buffer */

    while (check_end(archive)) {
        read_header(archive, header); /* Read header into header buf */
        if (!check_header(header, sflag)) {
            exit(EXIT_FAILURE);
        }
        type = header[OFF_TYPEFLAG];
        get_name(header, name);
        extract_item(type, header, name, archive, vflag);
        for (i=0; i<MAX_PATH; i++) {
            name[i] = '\0';
        }
    }
}

void extract_item(char type, char* header, char *fname, 
        int archive, bool vflag) {

    switch(type) {
        case '0':
            extract_file(header, fname, archive, vflag);
            break;
        case '5':
            extract_dir(header, fname, archive, vflag);
            break;
        case '2':
            extract_link(header, fname, archive, vflag);
            break;
        default:
            fprintf(stderr, "Invalid typeflag: %c\n", type);
            break;
    }
}


void extract_file(char* header, char* fname, int archive, bool vflag) {
    
    uint32_t perms;
    int fd_out;

    perms = ostr_to_int(&header[OFF_MODE]);
    /*errno = 0;*/
    if ((perms & S_IXUSR) | (perms & S_IXGRP) | (perms & S_IXOTH)) {
        
        if ((fd_out = open(fname, O_CREAT|O_WRONLY|O_TRUNC, 
                        S_IRUSR|S_IWUSR|S_IXUSR|
                        S_IRGRP|S_IWGRP|S_IXGRP|
                        S_IROTH|S_IWOTH|S_IXOTH)) < 0) {
            fprintf(stderr, "%s failed to open in extract_file\n",
                    fname);
            perror("open");
            exit(EXIT_FAILURE);
        }
    }

    else {
        
        if ((fd_out = open(fname, O_CREAT|O_WRONLY|O_TRUNC, 
                        S_IRUSR|S_IWUSR|
                        S_IRGRP|S_IWGRP|
                        S_IROTH|S_IWOTH)) < 0) {
            fprintf(stderr, "%s failed to open in extract_file\n",
                    fname);
            perror("open");
            exit(EXIT_FAILURE);
        }

    }

    write_file(archive, fd_out, header, vflag);
    close(fd_out);

}

void write_file(int archive, int fd_out, char* header, bool vflag) {

    ssize_t size;
    ssize_t rsize;
    ssize_t wsize;
    ssize_t counter;

    char write_buf[DSK_BLK] = {'\0'};
    char name[MAX_PATH] = {'\0'};

    size = ostr_to_int(&header[OFF_SZ]);

    if (vflag) {
        printf("%s\n", get_name(header, name));
    }

    counter = 0;
    while (counter < size) {
        if (size-counter < DSK_BLK) {
            if ((rsize = read(archive, write_buf, size-counter)) < 0) {
                fprintf(stderr, "archive failed read in extract_file\n");
                perror("read");
                exit(EXIT_FAILURE);
            }
        }

        else {
            if ((rsize = read(archive, write_buf, DSK_BLK)) < 0) {
                fprintf(stderr, "archive failed read in extract_file\n");
                perror("read");
                exit(EXIT_FAILURE);
            }
        }
        if (rsize == 0) {
            #ifdef __arm__
                fprintf(stderr, "no bytes read. Expected %d more bytes\n", 
                        counter-size);
            #else
                fprintf(stderr, "no bytes read. Expected %lu more bytes\n", 
                        counter-size);
            #endif
        }

        if ((wsize = write(fd_out, write_buf, rsize)) < 0) {  
            fprintf(stderr, "archive failed write in extract_file\n");
            perror("write");
            exit(EXIT_FAILURE);
        }
        if (rsize != wsize) {
            #ifdef __arm__
                fprintf(stderr, "%d bytes read, but %d bytes written\n", 
                        rsize, wsize);
            #else
                fprintf(stderr, "%lu bytes read, but %lu bytes written\n", 
                        rsize, wsize);
            #endif
            exit(EXIT_FAILURE);
        }
        counter += wsize;
    }
    if (counter != 0 && counter%512) {
        if (lseek(archive, BLK-(counter%BLK), SEEK_CUR) < 0) {
            fprintf(stderr, "Failed to seek to counter mod 512 on fd %d\n", 
                    archive);
            perror("lseek");
            exit(EXIT_FAILURE);
        }
    }
}

void extract_dir(char* header, char* fname, int archive, bool vflag) {

    DIR* cur_dir = NULL;
    mode_t mode;
    char name[MAX_PATH] = {'\0'};

    if (!(cur_dir = opendir(fname))) {
        if (errno == ENOENT) {
            mode = ostr_to_int(&header[OFF_MODE]);
            if (mkdir(fname, mode)) {
                fprintf(stderr, "Failed to mkdir: %s\n", 
                        get_name(header, name));
                perror("mkdir");
                exit(EXIT_FAILURE);
            }
            if (!(cur_dir = opendir(fname))) {
                fprintf(stderr, "Failed to open directory 1: %s\n",
                        fname);
                perror("opendir");
                exit(EXIT_FAILURE);
            }
            errno = 0;
        }

        else {
            fprintf(stderr, "Failed to open directory 2: %s\n",
                    fname);
            perror("opendir");
            exit(EXIT_FAILURE);
        }
    }

    if (vflag) {
        printf("%s\n", get_name(header, name));
    }
    closedir(cur_dir);
}

void extract_link(char* header, char* fname, int archive, bool vflag) {
   
   char name[MAX_PATH] = {'\0'};

   if (symlink(&header[OFF_LINKNAME], get_name(header, name))  < 0) {
       perror("symlink");
       exit(EXIT_FAILURE);
   }
   if (vflag) {
       printf("%s\n", get_name(header, name));
   }
} 


void find_extract_file(int archive, char *fname, 
        bool vflag, bool sflag) {
    /*printf("Finding and extracting file: %s\n", fname); */

    char name_cpy[BLK];
    char *tok;
    bool extracted_flag;
    bool seek_flag;
    char type;
    int index;
    int i;
    
    char header[BLK] = {'\0'}; 
    char name[MAX_PATH] = {'\0'};
    extracted_flag = false;
    seek_flag = false;

    while(check_end(archive)) {
        read_header(archive, header);
        if (!check_header(header, sflag)) {
            exit(EXIT_FAILURE);
        }
        type = header[OFF_TYPEFLAG];


        strcpy(name_cpy, get_name(header, name));
        tok = strtok(name_cpy, "/");
        index = 0;
        while (tok) {
            if (!strcmp(tok, fname)) {
                extracted_flag = true;
                seek_flag = true;
                break;
            }
            index += strlen(tok)+1;
            tok = strtok(NULL, "/");
        }
        if (seek_flag) {
            for (i=0; i<BLK; i++) {
                name[i] = '\0';
            }
            extract_item(type, header, get_name_at_index(header, name, index),
                    archive, vflag);
        }
        else { 
            seek_to_header(archive, header);
        }

        seek_flag = false;
    }

    if (lseek(archive, 0, SEEK_SET) < 0) {
        perror("lseek");
        exit(EXIT_FAILURE);
    }
    if (!extracted_flag) {
        printf("File: %s not found in archive\n", fname);
    }
}

char* get_name_at_index(char *header, char* name, int index) {
    if (index < SZ_NAME) {
        strncpy(name, &header[OFF_NAME+index], 
                strlen(&header[OFF_NAME+index]));
    }
    else if (index < SZ_NAME + SZ_PREF) {
        strncpy(name, &header[OFF_PREF+(index-SZ_NAME)],
                strlen(&header[OFF_PREF+(index-SZ_NAME)]));
    }
    else {
        fprintf(stderr, "Index: %d larger than max file length\n", index);
        exit(EXIT_FAILURE);
    }
    return name;
}
    
