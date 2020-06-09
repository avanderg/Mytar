#include "header.h"

/* TODO: Make sure overflow to prefix works*/

/* Read a header into memory */
char* read_header(int archive, char* header) {
    
    ssize_t size;

    if ((size = read(archive, header, BLK)) < 0) {
        perror("read");
        exit(EXIT_FAILURE);
    }

    /* Ensure we read in a full block */
    if (size != BLK) {
        #ifdef __arm__
            fprintf(stderr, "%d bytes read instead of %d\n", size, BLK);
        #else
            fprintf(stderr, "%lu bytes read instead of %d\n", size, BLK);
        #endif
        exit(EXIT_FAILURE);
    }

    return header;
}

/* Write a header from a char buffer to a file */
int write_header(char* arch_name, char* header) {

    int fd_out;
    ssize_t size;


    if ((fd_out = open(arch_name, O_CREAT|O_WRONLY|O_APPEND, 
                    S_IRUSR|S_IWUSR)) < 0) {
        fprintf(stderr, "%s failed to open in write_header\n",
                arch_name);
        perror("open");
        exit(EXIT_FAILURE);
    }

    if ((size = write(fd_out, header, BLK)) < 0) {
        perror("write");
        exit(EXIT_FAILURE);
    }

    /* Ensure we wrote a full block */
    if (size != BLK) {
        #ifdef __arm__
            fprintf(stderr, "%d bytes written instead of 512", size);
        #else
            fprintf(stderr, "%lu bytes written instead of 512", size);
        #endif
        exit(EXIT_FAILURE);
    }

    close(fd_out);
    return 0;
}

/* Build a header for file pathname stored in buffer header */ 
int build_header(char *pathname, char *header) {
    struct stat sb;
    struct passwd *pw;
    struct group *grp;
    uint32_t mtime;
    char typeflag;
    char ostr_buf[SZ_OBUF] = {'\0'}; /* Buffer for int_to_ostr */

    /* strtok stuff */
    char *tok;
    char path_cpy[BLK] = {'\0'};
    char dir_path[BLK] = {'\0'};
    char link_buf[BLK] = {'\0'};

    /* Flag to check if leading ./ or ../ need to be stripped */
    /* bool pflag = false; */

    /* For counting leading parents */
    int pflag;
    int index;




    if (lstat(pathname, &sb) == -1) {
       fprintf(stderr, "Error on %s\n", pathname);
       perror("lstat"); 
       exit(EXIT_FAILURE);
    }

    /* Get m time in seconds, drop the nanoseconds */
    mtime = build_mtime(&sb.st_mtim);

    typeflag = get_type(sb.st_mode);

    /* Getting filename */
    /* Strip off the leading "./" if it's given, make sure it's not a leading
       "../"
    */
    if (pathname[1] == '.' || pathname[0] != '.') {
        tok = pathname;
    }


    /* If ./ needs to be stripped, strip it */
    else {
        strcpy(path_cpy, pathname);
        tok = strtok(path_cpy, "/");
        tok = strtok(NULL, "/");
    }

    /* Strip "/" at end if need to */
    if (tok[strlen(tok)-1] == '/') {
        tok[strlen(tok)-1] = '\0';
    }
    
    /* Regular File */
    if (typeflag == '0') {
        /* Write name to header, overflow goes to prefix section */
        create_name(header, tok);
        /* Write size to header as an ostr */
        memcpy(&header[OFF_SZ], int_to_ostr(sb.st_size, SZ_SIZE, ostr_buf), 
                SZ_SIZE);
    }
    /* Directory */
    else if (typeflag == '5') {
        /* Add a "/" to the end of the name */
        strncpy(dir_path, tok, strlen(tok));
        strcat(dir_path, "/");
        /* Write name to header, overflow goes to prefix section */
        create_name(header, dir_path);
        /* Directories have size 0 */
        memcpy(&header[OFF_SZ], ZERO_SZ, 1);
    }
    else if (typeflag == '2') {
        /* linkname stuff then...*/
        memcpy(&header[OFF_SZ], ZERO_SZ, SZ_SIZE);
        create_name(header, tok);
        if (readlink(tok, link_buf, BLK) < 0) {
            perror("readlink");
            exit(EXIT_FAILURE);
        }
        memcpy(&header[OFF_LINKNAME], link_buf, strlen(link_buf)); 

    }
    else {
        fprintf(stderr, "File: %s is not a regular file, directory, or"
                "symlink. Skipping...\n", pathname);
    }


    #ifdef OLD_PARENT 
    if (header[OFF_NAME] == '.' && header[OFF_NAME+1] == '.') {
        pflag = true;
        strip_parent(header);
    }
    #endif

    /* Strip parent "../"s */
    index = 0;
    pflag = 0;
    while(header[OFF_NAME+index] == '.' && header[OFF_NAME+index+1] == '.') {
        pflag++;
        index += 3;
    }
    strip_parent(header);

    memcpy(&header[OFF_MODE],
            int_to_ostr(sb.st_mode, SZ_MODE, ostr_buf), SZ_MODE);

    memcpy(&header[OFF_UID],  
            int_to_ostr(sb.st_uid, SZ_ID, ostr_buf), SZ_ID);

    memcpy(&header[OFF_GID], 
            int_to_ostr(sb.st_gid, SZ_ID, ostr_buf), SZ_ID);

    /*
    printf("mtime: %d\n", mtime);
    */
    memcpy(&header[OFF_MTIME], 
            int_to_ostr(mtime, SZ_MTIME, ostr_buf), SZ_MTIME);
    
    memcpy(&header[OFF_TYPEFLAG], &typeflag, SZ_TYPEFLAG);
    
    /* linkname to be implemented later */ 

    memcpy(&header[OFF_MAGIC], MAGIC, SZ_MAGIC);
    memcpy(&header[OFF_VERSION], VERSION, SZ_VERSION); 

    if (!(pw = getpwuid(sb.st_uid))) {
        perror("getpwuid");
        exit(EXIT_FAILURE);
    }
    memcpy(&header[OFF_UNAME], pw->pw_name, strlen(pw->pw_name));

    if (!(grp = getgrgid(sb.st_gid))) {
        perror("getgrgid");
        exit(EXIT_FAILURE);
    }
    memcpy(&header[OFF_GNAME], grp->gr_name, strlen(grp->gr_name));

    memcpy(&header[OFF_DEVMJ], 
            int_to_ostr(major(sb.st_dev), SZ_DEV, ostr_buf), SZ_DEV);
    memcpy(&header[OFF_DEVMN], 
            int_to_ostr(minor(sb.st_rdev), SZ_DEV, ostr_buf), SZ_DEV);

    memcpy(&header[OFF_CHKSUM], 
            int_to_ostr(chksum(header), SZ_CHKSUM, ostr_buf), SZ_CHKSUM);

    return pflag; 
}

void create_name(char* header, char* name) {
    int len;
    len = (int) strlen(name);

    if (len < SZ_NAME) {
        memcpy(&header[OFF_NAME], name, SZ_NAME);
    }
    else if (len <= SZ_NAME + SZ_PREF){
        memcpy(&header[OFF_NAME], name, SZ_NAME);
        memcpy(&header[OFF_PREF], name+SZ_NAME, SZ_PREF);
    }
    else {
        #ifdef __arm__
            fprintf(stderr, "Name length, %d, too long\n", strlen(name));
        #else
            fprintf(stderr, "Name length, %lu, too long\n", strlen(name));
        #endif
        exit(EXIT_FAILURE);
    }
}

char* get_name(char *header, char *name) {
    if (header[OFF_NAME+SZ_NAME-1] == '\0') {
        strncpy(name, &header[OFF_NAME], strlen(&header[OFF_NAME]));
    }
    else {
        strncpy(name, &header[OFF_NAME], SZ_NAME);
        strncpy(name+SZ_NAME, &header[OFF_PREF], strlen(&header[OFF_PREF]));
    }
    return name;
}


int strip_parent(char *header) {
    char *tok;
    char path_cpy[BLK] = {'\0'};
    char name[BLK] = {'\0'};
    char *ptr;
    char delim[3] = "..";
    int i;
    int cnt;

    cnt = 0;
    get_name(header, name);

    strncpy(path_cpy, name, strlen(name));

    ptr = path_cpy;

    /* use strstr to split on ".." */
    /* go to where delim is */ 
    while ((tok = strstr(ptr, delim))) {
        ptr += strlen(delim); /* increment past it */
        if (*ptr == '/') {
            ptr++;
        }

        cnt++;
    }
    if (cnt) {
        /*ptr++;*/ /* increment for / */
        for (i=strlen(ptr); i<SZ_NAME; i++) {
            ptr[i] = '\0';
        }
        create_name(header, ptr);

    }

    return 0;
}

bool check_header(char* header, bool sflag) {
    /*
    printf("chksum(header): %d\n", chksum(header));
    printf("ostr_to_int(&header[OFF_CHKSUM]: %d\n", 
            ostr_to_int(&header[OFF_CHKSUM]));
            */

    /*
    printf("inside check_header\n");
    printf("ostr_to_int(&header[OFF_CHKSUM]): %d\n", 
            ostr_to_int(&header[OFF_CHKSUM]));
            */
    if (chksum(header) != ostr_to_int(&header[OFF_CHKSUM])) {
        fprintf(stderr, "Invalid header\n");
        printf("chksum was %d but read %d from file\n",
                chksum(header), ostr_to_int(&header[OFF_CHKSUM]));

        /*
        fprintf(stderr, "chksum(header): %d while %d was written in" 
               " chksum slot", chksum(header), 
               ostr_to_int(&header[OFF_CHKSUM]));
               */
        return false;
    }
    if (!sflag) {

        if (!strstr(&header[OFF_MAGIC], MAGIC)) {
            fprintf(stderr, "Invalid header\n");
            fprintf(stderr, "Magic was %s instead of %s\n", 
                    &header[OFF_MAGIC], MAGIC);
            return false;
        }
        return true;
    }
    else {

        if (!strcmp(MAGIC, &header[OFF_MAGIC])) {
            fprintf(stderr, "Invalid header\n");
            return false;
        }
        
        if (header[OFF_VERSION] != VERSION_CHAR ||
            header[OFF_VERSION+1] != VERSION_CHAR) {
            fprintf(stderr, "Invalid header\n");
            return false;
        }

    }
    return true;
}

uint32_t chksum(char *header) {
    uint32_t sum;
    int i;

    sum = 0;
    for (i=0; i<BLK; i++) {
        if (i > 147 && i < 156) {
            sum += (unsigned char) SPACE; 
        }
        else {
            sum += (unsigned char) header[i];
        }
        /*
        printf("sum: %d\n", sum);
        printf("loop count: %d\n", i);
        */
    }

    return sum;

}


uint32_t build_mtime(struct timespec *st_mtim) {
    /* Just drop the nanoseconds, should probably change to round */
    return st_mtim->tv_sec;

}

char get_type(mode_t mode) {
    if (S_ISREG(mode)) {
        return '0';
    }
    else if (S_ISLNK(mode)) {
        return '2';
    }
    else if (S_ISDIR(mode)) {
        return '5';
    }
    else {
        return '9';
    }
}

