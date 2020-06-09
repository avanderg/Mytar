#include "creation.h"

int add_to_archive(char* arch_name, int archive, char* fname, 
        bool vflag, bool sflag) {
    
    int fd_in; /* File descriptor for file (fname) to write to archive */
    ssize_t rsize; /* Read size */
    ssize_t wsize; /* Write size */
    ssize_t counter; /* Loop counter for read/write loop */
    int i; /* Loop index for clearing out write_buf for next read/write */
    DIR* cur_dir = NULL; /* For traversing directories */
    struct dirent* my_dirent = NULL; /* Also for traversing directories */
    bool pflag; /* Parent flag, used to check if leading  ../ needs to 
                   be stripped */
    char header[BLK] = {'\0'}; /* Header buffer */
    char write_buf[DSK_BLK] = {'\0'}; /* Buffer to read in and write out of */
    char name[MAX_PATH] = {'\0'}; /* The name of the file with ../ if needed */
    char tmp_name_buf[MAX_PATH] = {'\0'}; /* Buffer for get_name fucntion */
    char path_buf[MAX_PATH] = {'\0'}; /* Buffer for path if fname is a dir */

    /* Check if we're trying to archive the archive recursively on itself */
    if (cmp(arch_name, fname)) {
        fprintf(stderr, "Trying to archive the archive %s...Skipping\n",
                arch_name);
        return 0;
    }

    /* Build header and return pflag */
    pflag = build_header(fname, header);
    if (!check_header(header, sflag)) {
        fprintf(stderr, "Created invalid header\n");
        exit(EXIT_FAILURE);
    }
    /* Write the header to the archive */
    write_header(arch_name, header);
    /* If needed, return leading ../ */
    if (pflag) {
       for (i=0; i<pflag; i++) {
           strcat(name, "../"); 
       }
    }
    /* Name is the filename */
    strcat(name, get_name(header, tmp_name_buf));
    /* Seek past the header to write */
    if (lseek(archive, BLK, SEEK_CUR) < 0) {
        fprintf(stderr, "Couldn't seek archive\n");
        perror("lseek");
    }

    if (vflag) {
        printf("%s\n", name);
    }

    /* If file is a directory, add its children */
    if (header[OFF_TYPEFLAG] == '5') {
        /* Add its children to archive */
        /* Doing a DFS */
        if (!(cur_dir = opendir(name))) {
            fprintf(stderr, "Failed to open directory: %s\n",
                    name);
            perror("opendir");
            exit(EXIT_FAILURE);
        }
        errno = 0; /* Reset errno in case it was set somewhere*/
        while ((my_dirent = readdir(cur_dir))) {
            /* Skip self and parent */
            if (!(strcmp(my_dirent->d_name, ".")) || 
                !(strcmp(my_dirent->d_name, ".."))) {
                continue;
            }
            /* Put this directory path into path_buf */
            strcpy(path_buf, name);
            strncat(path_buf, my_dirent->d_name, strlen(my_dirent->d_name));
            /* Recurse on its children */
            add_to_archive(arch_name, archive, path_buf, 
                    vflag, sflag);
        }
        /* Check for errno b/c readdir returns NULL at the end of directory
           and if there was an error
        */
        if (errno) {
            fprintf(stderr, "Failed to read directory: %s\n",
                name);
            perror("readdir");
            exit(EXIT_FAILURE);
        }
        closedir(cur_dir);
        return 0;
    }
    
    /* If size of file is 0 (empty file or symlink), return */
    if (ostr_to_int(&header[OFF_SZ]) == 0) {
        return 0;
    }

    /* To get here, file should have typeflag 0, b/c it's not a directory
       or symlink
    */
    if (header[OFF_TYPEFLAG] != '0') {
        fprintf(stderr, "Expected a regular file, invalid typeflag for file:"
                " %s\n", fname);
        return -1;
    }

    /* File is a regular file, so write its contents */
    if ((fd_in = open(fname, O_RDONLY)) < 0) {
        fprintf(stderr, "%s failed to open\n", fname);
        perror("open");
        exit(EXIT_FAILURE);
    }
    
    counter = 0;
    /* Loop for size stored in header */
    while (counter < ostr_to_int(&header[OFF_SZ])) {
        /* Read a full disk block from the file */
        if ((rsize = read(fd_in, write_buf, DSK_BLK)) < 0) {
            fprintf(stderr, "%s failed read\n", fname);
            perror("read");
            exit(EXIT_FAILURE);
        }

        /* Expected some bytes to be read, blow up if nothing is read */
        if (rsize == 0) {
            printf("no bytes read. Expected %lu more bytes\n", 
                    counter-ostr_to_int(&header[OFF_SZ]));
        }

        /* If the amount read wasn't a multiple of 512, make it a full block.
           This means we've hit the end of file also */
        if (rsize % BLK) {
            for (i=rsize; i<rsize+(BLK - rsize%BLK); i++) {
                write_buf[i] = '\0';
            }
            rsize = rsize + (BLK - rsize%BLK);
        }

        /* Write the data that was read */
        if ((wsize = write(archive, write_buf, rsize)) < 0) {  
            fprintf(stderr, "archive failed write");
            perror("write");
            exit(EXIT_FAILURE);
        }

        /* If the written size wasn't an even block of 512, something bad
           happened */
        if (wsize%BLK) {
            fprintf(stderr, "Bytes written differs from bytes read."
                    "%lu written when %d read.\n", wsize, BLK);
            exit(EXIT_FAILURE);
        }

        counter += wsize;
    }

    close(fd_in);
    return 0;
}

/* Checks the archive name with the current filename so the program
   doesn't recursively create archives with itself inside 
*/
bool cmp(char *arch_name, char *fname) {
    char *tok;
    char path_cpy[BLK] = {'\0'};
    char *end;

    strcpy(path_cpy, fname);
    tok = strtok(path_cpy, "/");
    while (tok) {
        end = tok;
        tok = strtok(NULL, "/");
    }

    if (!(strcmp(arch_name, end))) {
        return true;
    }
    else {
        return false;
    }
}

