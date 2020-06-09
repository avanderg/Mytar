#include "mytar.h"
/*
 TODO: (For whole program!) 
       * More intensive tests
       * Duplicate entries (This might work already, needs to be tested)
       * Make sure perms are implemented properly for extraction 
         (read spec)
       * Remove unneeded #includes, #defines, and comments
       * Documentation (Comments and README)
       * Need to read spec to double check everything is met
       * Looks like long files are not interoperable with gnu tar atm
            * Need to check offsets and where file is writing, but
              looks like gnu tar doesn't want to find the second part
       Optional:
            * Support for integers not fitting in allowed num of octal digits
       
       Intensive tests performed on: 
            * very large files (2gb movies)
            * empty files
            * valgrind tests looking good on normal stuff and tests done
            * Looks to be perfoming pretty well except on archive with
              an absolute stupid amount of files (JavaProjects)
                * Works for mytar creation and extraction, but not compatible
                  with tar creation and extraction
       Need to test: 
            * Very deep directories (deep_dir is ok, but JavaProjects is not)
              JavaProjects now works with mytar itself, but not with GNU
              tar. I think their handling of long files differs from 
              this program.
            * Test on sparse files and files filled with null bytes
*/



int main(int argc, char* argv[]) {

    int i;
    bool cflag = false; /* Creation Flag */
    bool tflag = false; /* Listing Flag */
    bool xflag = false; /* Extraction Flag */
    bool vflag = false; /* Verbose Flag */
    bool sflag = false; /* Strict Flag */

    /* Require at least 3 arguments: progname, flags (cxtvSf), archive name */
    if (argc < 3) {
        fprintf(stderr, "Too few arguments\n");
        print_usage();
        exit(EXIT_FAILURE);
    }

    /* Require fflag, no support for stdin/stdout as f */
    if (argv[1][strlen(argv[1])-1] != 'f') {
        fprintf(stderr, "Need f flag\n");
        print_usage();
        exit(EXIT_FAILURE);
    }

    /* Parse input to get mode */
    for (i=0; i<strlen(argv[1]); i++) {
        switch (argv[1][i]) {
            case('c'):
                cflag = true;
                break;
            case('t'):
                tflag = true;
                break;
            case('x'):
                xflag = true;
                break;
            case ('v'):
                vflag = true;
                break;
            case('S'):
                sflag = true;
                break;
            case('f'):
                break;
            default:
                fprintf(stderr, "Invalid flag: %c\n", argv[1][i]);
                print_usage();
                exit(EXIT_FAILURE);
        }
    }

    /* Require only 1 running mode. Can't do any two of creation, listing,
       and extraction.
    */
    check_flags(cflag, tflag, xflag);

    /* Run appropriate mode */
    if (cflag)
        creation_mode(argc, argv, vflag, sflag);
    else if (tflag)
        listing_mode(argc, argv, vflag, sflag);
    else if (xflag)
        extraction_mode(argc, argv, vflag, sflag);
    else {
        fprintf(stderr, "I wasn't given a mission to perform :(\n");
        print_usage();
        exit(EXIT_FAILURE);
    }

}

void print_usage(void) {
    fprintf(stderr, "Usage: mytar [ctxvS] tarfile [ path [ ... ] ] \n");
    fprintf(stderr, "[-c] Create an archive\n");
    fprintf(stderr, "[-t] Print the table of contents of an archive\n");
    fprintf(stderr, "[-x] Extract the contents of an archive\n");
    fprintf(stderr, "[-f] Specifies archive filename\n");
    fprintf(stderr, "[-S] Be strict about standards compliance\n");
}


/* Require only 1 running mode. Can't do any two of creation, listing, 
   and extraction.
*/
void check_flags(bool cflag, bool tflag, bool xflag) {
    if (cflag && xflag) {
        fprintf(stderr, 
                "Can't create an archive and extract at the same time\n");
        exit(EXIT_FAILURE);
    }
    if (cflag && tflag) {
        fprintf(stderr, 
                "Can't make an archive and list at the same time\n");
        exit(EXIT_FAILURE);
    }
    if (xflag && tflag) {
        fprintf(stderr, 
                "Can't extract an archive and list at the same time\n");
        exit(EXIT_FAILURE);
    }
}


/* Perform creation mode operations */
void creation_mode(int argc, char *argv[], bool vflag, bool sflag) {
    int i;
    int fd_out;

    /* Require an archive and at least 1 file to add to it */
    if (argc < 4) {
        fprintf(stderr, "No files given to add to archive, quitting.\n");
        exit(EXIT_FAILURE);
    }
    /* Open archive */
    if ((fd_out = open(argv[2], O_CREAT|O_WRONLY|O_TRUNC, 
                    S_IRUSR|S_IWUSR)) < 0) {
        fprintf(stderr, "%s failed to open\n", argv[2]);
        perror("open for write");
        exit(EXIT_FAILURE);
    }
    /* Loop through all given files to add to archive */
    for (i=3; i<argc; i++) {
        add_to_archive(argv[2], fd_out, argv[i], vflag, sflag);
    }

    /* Write double NULL block to mark end of archive */
    write_end_archive(fd_out);

    close(fd_out);
    
}

/* Write double NULL block to mark end of archive */
int write_end_archive(int fd_out) {
    ssize_t size;
    char end_buf[BLK*2] = {'\0'};

    if ((size = write(fd_out, end_buf, BLK*2)) < 0) {
        perror("write");
        exit(EXIT_FAILURE);
    }
    if (size != BLK*2) {
#ifdef __arm__
            fprintf(stderr, "Expected size to be %d, but it was %d\n",
                    BLK*2, size);
#else
            fprintf(stderr, "Expected size to be %d, but it was %lu\n",
                    BLK*2, size);
#endif
        exit(EXIT_FAILURE);
    }
    return 0;
}

/* Perform list mode operations */
void listing_mode(int argc, char *argv[], bool vflag, bool sflag) {
    
    int archive;
    char *header;
    int i;

    if ((archive = open(argv[2], O_RDONLY)) < 0) {
        fprintf(stderr, "%s failed to open\n", argv[2]);
        perror("open for read");
        exit(EXIT_FAILURE);
    }

    if (!(header = calloc(BLK, sizeof(char)))) {
        perror("calloc");
        exit(EXIT_FAILURE);
    }
    
    /* If no file is given after archive, list the whole archive */
    if (argc < 4) {
        list_all(archive, header, vflag, sflag);
    }
    /* Otherwise, look for the given files in the archive and list 
       them if found, or report they aren't in the archive.
    */
    else if (argc >= 4){
        for (i=3; i<argc; i++) {
            find_list_file(archive, header, argv[i], vflag, sflag);
        }
    }
    else {
        fprintf(stderr, "Invalid value for argc: %d\n", argc);
        print_usage();
    }

    free(header);
    close(archive);

}

/* Perform extract mode operations */
void extraction_mode (int argc, char* argv[], bool vflag, bool sflag) {
    int archive;
    int i;

    if ((archive = open(argv[2], O_RDONLY)) < 0) {
        fprintf(stderr, "%s failed to open\n", argv[2]);
        perror("open for read");
        exit(EXIT_FAILURE);
    }

    /* If no file is given after archive, extract the whole archive */
    if (argc < 4) {
        extract_archive(archive, vflag, sflag);
    }
    /* Otherwise, look for the given files in the archive and extract 
       them if found, or report they aren't in the archive.
    */
    else if (argc >= 4) {
        for (i=3; i<argc; i++) {
            find_extract_file(archive, argv[i], vflag, sflag);
        }
    }
    /* Shouldn't be able to get here, but just in case */
    else {
        fprintf(stderr, "Invalid value for argc: %d\n", argc);
        print_usage();
    }
    close(archive);
}
