#include "listing.h"

void list_all(int archive, char *header, 
        bool vflag, bool sflag) {

    /*ssize_t size; */

    while(check_end(archive)) {

        read_header(archive, header);
        if (!check_header(header, sflag)) {
            fprintf(stderr, "Found invalid header\n");
            exit(EXIT_FAILURE);
        }

        list_file(archive, header, vflag);
        seek_to_header(archive, header);
    }
}

void list_file(int archive, char *header, bool vflag) {
    
    /*ssize_t filesize;
    off_t seeksize;
    */
    
    char name[BLK] = {'\0'};

    if (vflag) {
        verbose_list(header);
    }
    else {
        printf("%s\n", get_name(header, name));
    }

    
}


void find_list_file(int archive, char* header, char *fname,
        bool vflag, bool sflag) {

    char name_cpy[BLK]; 
    char *tok;
    bool listed_flag;
    char name[BLK] = {'\0'};
    int i;

    
    listed_flag = false;

    while(check_end(archive)) {

        read_header(archive, header);
        /*check_header(header); */
        if (!check_header(header,sflag)) {
            exit(EXIT_FAILURE);
        }


        strcpy(name_cpy, get_name(header, name));
        /*tok = name_cpy; */
        tok = strtok(name_cpy, "/");
        /*
        if (!strcmp(tok, fname)) {
            list_file(archive, header, vflag, sflag);
            listed_flag = true;
            continue;
        }
        */

        while (tok) {
            if (!strcmp(tok, fname)) {
                list_file(archive, header, vflag);
                listed_flag = true; 
                break;
            }
            tok = strtok(NULL, "/");
        }
        /*
        if (!listed_flag) {
            seek_to_header(archive, header);
        }
        */
        seek_to_header(archive, header);
        for (i=0; i<BLK; i++) {
            name[i] = '\0';
        }
    }

    if (lseek(archive, 0, SEEK_SET) < 0) {
        perror("lseek");
        exit(EXIT_FAILURE);
    }
    if (!listed_flag) {
        printf("File: %s not found in archive\n", fname);
    }
    
}


void verbose_list(char* header) {
    char print_string[BLK] = {'\0'};
    char *ptr;
    char tmp[BLK] = {'\0'};
    ssize_t size;
    char buf[BLK] = {'\0'};
    char name[BLK] = {'\0'};

    switch(header[OFF_TYPEFLAG]) {
        case '0':
            print_string[0] = '-';
            break;
        case '5':
            print_string[0] = 'd';
            break;
        case '2':
            print_string[0] = 'l';
            break;
        default:
            fprintf(stderr, "Invalid typeflag: %c\n", header[OFF_TYPEFLAG]);
            break;
    }
    ptr = print_string + 1; 
    read_perms(&header[OFF_MODE], ptr);
   
    strcat(print_string, " ");
    strcat(print_string, &header[OFF_UNAME]);
    strcat(print_string, "/");
    strcat(print_string, &header[OFF_GNAME]);
    
    size = ostr_to_int(&header[OFF_SZ]);
    #ifdef __arm__
        sprintf(tmp, "%8d", size);
    #else
        sprintf(tmp, "%8lu", size);
    #endif
    strcat(print_string, tmp);
    strcat(print_string, " ");

    read_time(&header[OFF_MTIME], buf); 
    /*
    printf("mtime: %d\n", ostr_to_int(&header[OFF_MTIME]));
    */
    strcat(print_string, buf);

    strcat(print_string, get_name(header, name));

    printf("%s\n", print_string);

}

void read_perms(char* mode, char* formatted_perm) {
        /*Takes in an octal string representing mode and formats output*/
        /*char formatted_perm[10] = {0}; */
        int i;
        char *perms;
        perms = mode;
        perms += strlen(mode)-3;
        
        for (i=0; i < 3; i++) {
            switch(perms[i])
            {
            case '0':
                formatted_perm[0+(3*i)] = '-';
                formatted_perm[1+(3*i)] = '-';
                formatted_perm[2+(3*i)] = '-';
                break;
            case '1':
                formatted_perm[0+(3*i)] = '-';
                formatted_perm[1+(3*i)] = '-';
                formatted_perm[2+(3*i)] = 'x';
                break;
            case '2':
                formatted_perm[0+(3*i)] = '-';
                formatted_perm[1+(3*i)] = 'w';
                formatted_perm[2+(3*i)] = '-';
                break;
            case '3':
                formatted_perm[0+(3*i)] = '-';
                formatted_perm[1+(3*i)] = 'w';
                formatted_perm[2+(3*i)] = 'x';
                break;
            case '4':
                formatted_perm[0+(3*i)] = 'r';
                formatted_perm[1+(3*i)] = '-';
                formatted_perm[2+(3*i)] = '-';
                break;
            case '5':
                formatted_perm[0+(3*i)] = 'r';
                formatted_perm[1+(3*i)] = '-';
                formatted_perm[2+(3*i)] = 'x';
                break;
            case '6':
                formatted_perm[0+(3*i)] = 'r';
                formatted_perm[1+(3*i)] = 'w';
                formatted_perm[2+(3*i)] = '-';
                break;
            case '7':
                formatted_perm[0+(3*i)] = 'r';
                formatted_perm[1+(3*i)] = 'w';
                formatted_perm[2+(3*i)] = 'x';
                break;
            }
        }
}

void read_time(char* times, char* buf) {
        time_t time;
        struct tm *inf;

        time = (time_t) ostr_to_int(times);
        inf = localtime(&time);
        sprintf(buf, "%d-%02d-%02d %02d:%02d ",
                1900+inf->tm_year, 1+inf->tm_mon, 
                inf->tm_mday,inf->tm_hour, inf->tm_min);

}
