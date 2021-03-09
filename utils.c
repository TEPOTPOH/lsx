/*
*   file: utils.c
*   lsx: short analog of linux program 'ls'. supports only option '-l'
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <stdbool.h>

#include "utils.h"

#define RESET_COLOR "\e[m" 
#define MAKE_GREEN "\e[32m" 
#define MAKE_BLUE "\e[34m" 
#define MAKE_YELLOW  "\e[33m" 
#define MAKE_MAGNETA  "\e[35m" 
#define MAKE_CYAN  "\e[36m" 


static char* get_date(char* buff, struct stat* st)
{
    time_t        file_date;
    struct tm     tm_file_date;

    file_date = st->st_mtime;
    tm_file_date = *localtime(&file_date);
    strftime(buff, 18, "%b %d %H:%M", &tm_file_date);
    return buff;
}

/* Display letters and indicators for each filetype.
   Keep these in sync with enum filetype.  */
static char const filetype_letter[] = "?pcdb-lswd";

static char get_file_type_char(filetype_t type)
{
    return filetype_letter[type];
}

static char* get_rwx_rights(const struct stat *st, char buf[4], int rmask, int wmask, int xmask)
{
    buf[0] = '-';
    buf[1] = '-';
    buf[2] = '-';

    if (st->st_mode & rmask)
        buf[0] = 'r';

    if (st->st_mode & wmask)
        buf[1] = 'w';

    if (st->st_mode & xmask)
        buf[2] = 'x';

    return buf;
}

filetype_t get_file_type(const struct stat* st)
{
    filetype_t type = unknown;
    mode_t mode = st->st_mode;

    if (S_ISREG (mode))
        type = normal;
    else if (S_ISDIR (mode))
        type = directory;
    else if (S_ISLNK (mode))
        type = symbolic_link;
    else if (S_ISFIFO (mode))
        type = fifo;
    else if (S_ISSOCK (mode))
        type = sock;
    else if (S_ISBLK (mode))
        type = blockdev;
    else if (S_ISCHR (mode))
        type = chardev;

    return type;
}

void print_color_name(filetype_t filetype, const char* fullpath, const char* f_name)
{
    bool colored = true;

    if ((chardev == filetype) || (blockdev == filetype))
    {
        printf(MAKE_YELLOW);
    }
    else if (symbolic_link == filetype)
    {
        printf(MAKE_CYAN);
    }
    else if (sock == filetype)
    {
        printf(MAKE_MAGNETA);
    }
    // Check if the file/folder is executable
    else if(!access((const char*)fullpath,X_OK))
    { 
        if (directory == filetype)
        { 
            // If folder, print in blue 
            printf(MAKE_BLUE);
        }
        else
        {
            // If executable file, print in green
            printf(MAKE_GREEN); 
        }
    }
    else 
    { 
        // If normal file, print by the default way(black color) 
        printf("%s",f_name);
        colored = false;
    }

    if (colored)
        printf("%s"RESET_COLOR,f_name);
}

void print_long_format(struct stat* st, filetype_t filetype)
{
    char ft_c = get_file_type_char(filetype);

    char u_rights[4] = "";
    get_rwx_rights(st, u_rights, S_IRUSR, S_IWUSR, S_IXUSR);

    char g_rights[4] = "";
    get_rwx_rights(st, g_rights, S_IRGRP, S_IWGRP, S_IXGRP);

    char o_rights[4] = "";
    get_rwx_rights(st, o_rights, S_IROTH, S_IWOTH, S_IXOTH);
    printf("%c%s%s%s", ft_c, u_rights, g_rights, o_rights);

    // Print the number of hard links 
    printf(" %d ", (int)st->st_nlink); 

    // Get the user name 
    struct passwd *pt = getpwuid(st->st_uid); 
    printf("%s ",pt->pw_name); 

    // Get the group name 
    struct group *p = getgrgid(st->st_gid); 
    printf("%s ",p->gr_name); 

    // Get the file size 
    printf("%lld ",(long long) st->st_size); 

    // Get the date and time 
    char date_time[64]; 
    memset(date_time,0,sizeof(date_time));
    get_date(date_time, st);
    printf("%s ", date_time); 
}

char* make_fullpath(char* dir_path, char* f_name)
{
    char* fullpath;
    size_t size = strlen(dir_path) + strlen(f_name) + 3;

    fullpath = malloc(size);
    if (NULL == fullpath) {
        return NULL;
    }
    
    strcpy(fullpath, dir_path);
    int pos = strlen(fullpath);
    if ('/' != fullpath[pos-1]) {
        fullpath[pos] = '/';
        fullpath[pos+1] = '\0';
    }
    fullpath = strcat(fullpath, f_name);
    return fullpath;
}

// Sorting the names alphabetically
// Using bubble sorting
void absort(long *ptr, int num_files)
{
    int j;
    unsigned int count;

    for(count = 0; count< num_files-1;count++) 
    { 
       for(j=count+1; j< (num_files);j++) 
       { 
           char *c = (char*)ptr[count]; 
           char *d = (char*)ptr[j]; 
            
           // Check that the two characters should be from same set 
           if( ((*c >= 'a') && (*d >= 'a')) || ((*c <='Z') && (*d <='Z')) ) 
           { 
               int i = 0; 
               // If initial characters are same, continue comparing 
               // the characters until a difference is found 
               if(*c == *d) 
               { 
                   while(*(c+i)==*(d+i)) 
                   { 
                       i++; 
                   } 
               } 
               // Check if the earlier stored value is alphabetically 
               // higher than the next value 
               if(*(c+i) > *(d+i)) 
               { 
                   // If yes, then swap the values 
                   long temp = 0; 
                   temp = ptr[count]; 
                   ptr[count] = ptr[j]; 
                   ptr[j] = temp; 
               } 
 
           } 
           else 
           { 
               // if the two beginning characters are not from 
               // the same ascii set then make them same and then 
               // compare. 
               int off_1=0, off_2=0; 
               if(*c <= 'Z') 
               { 
                   off_1 = 32; 
               } 
               if(*d <= 'Z') 
               { 
                   off_2 = 32; 
               } 
 
               int i = 0; 
               // After the character set are made same, check if the 
               // beginning characters are same. If yes, then continue  
               // searching until we find some difference. 
               if(*c+ off_1 == *d + off_2) 
               { 
                   while(*(c+off_1+i)==*(d+off_2+i)) 
                   { 
                       i++; 
                   } 
               } 
               // After difference is found, check if a swap is required. 
               if((*c + off_1+i) > (*d + off_2+i)) 
               { 
                   // If yes, go ahead and do the swap 
                   long temp = 0; 
                   temp = ptr[count]; 
                   ptr[count] = ptr[j]; 
                   ptr[j] = temp; 
               } 
           } 
       }
    }
}
