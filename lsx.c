/*
*   file: lsx.c
*   lsx: short analog of linux program 'ls'. supports only option '-l'
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <stdbool.h>

#include "utils.h"

static bool long_format = false;
static char* curr_dir = NULL;

static void parce_args(int argc, char **argv)
{
    if (argc > 1) {
        for (int n = 1; n < argc; ++n) {
            char* string = argv[n];
            if (NULL == string) {
                break;
            }

            // options
            if ('-' == string[0]) {
                if (2 == strlen(string) && ('l' == string[1])) {
                    long_format = true;
                } else {
                    printf(" command %s not supported\n", string);
                }
            } else {
                // path
                curr_dir = string;
            }
        }
    }
}

// return num_files
static int collect_files(long** file_list)
{
    DIR *dp = NULL; 
    struct dirent *dptr = NULL; 
    // number of files inside the directory
    int num_files = 0; 

    // open the directory 
    dp = opendir((const char*)curr_dir);

    if (NULL == dp) {
        printf(" cannot access %s: No such file or directory\n", curr_dir);
        return -1; 
    }

    // Start reading the directory contents 
    while(NULL != (dptr = readdir(dp)))
    { 
        // Do not count the files beginning with '.' 
        if(dptr->d_name[0] != '.') 
        num_files++; 
    }
    // Our aim was to count the number of files/folders  
    // inside the current working directory. Since its  
    // done so close the directory. 
    closedir(dp); 

    // Restore the values back as we will be using them 
    // later again 
    dp = NULL; 
    dptr = NULL; 

    // Check that we should have at least one file/folder 
    // inside the current working directory 
    if(!num_files) 
    { 
        return 0; 
    } 
    
    // Allocate memory to hold the addresses of the  
    // names of contents in current working directory 
    *file_list = malloc(num_files*8); 
    if(NULL == *file_list) 
    { 
        printf("\n Memory allocation failed\n"); 
        return -1; 
    } 
    else 
    { 
        // Initialize the memory by zeros 
        memset(*file_list,0,num_files*8); 
    }
 
   // Open the directory again 
   dp = opendir((const char*)curr_dir);
   if(NULL == dp) 
   { 
       printf("\n ERROR : Could not open the working directory\n"); 
       free(*file_list); 
       return -1; 
   } 

   // Start iterating the directory and read all its contents 
   // inside an array allocated above. 
   unsigned int j = 0; 
   for(unsigned int count = 0; NULL != (dptr = readdir(dp)); count++)
   { 
       if(dptr->d_name[0] != '.') 
       { 
          (*file_list)[j] = (long)dptr->d_name;
          j++;
       } 
   }

   return num_files;
}

int main(int argc, char **argv)
{
    unsigned int count = 0; 
    long *file_list = NULL; 
    
    parce_args(argc, argv);
    
    if (NULL == curr_dir) {
        // Fetch the environment variable PWD so as to get the
        // Current working directory 
        curr_dir = getenv("PWD"); 
        if (NULL == curr_dir) 
        {
            printf("\n Could not get the working directory\n");
            return -1; 
        }
    }
    
    int num_files = collect_files(&file_list);
    
    if (num_files <= 0) {
        // return error code
        return num_files;
    }
   
    // Start sorting the names alphabetically
    absort(file_list, num_files);

    // Start displaying on console. 
    for(count = 0; count< num_files; count++)
    {
        struct stat st;
        
        char* f_name = (char*)file_list[count];

        char* fullpath = make_fullpath(curr_dir, f_name);

        if(lstat(fullpath, &st)) {
            // If stat() fails 
            printf("\n stat() failed: %s\n", fullpath);
            if (NULL != file_list)
                free(file_list); 
            if (NULL != fullpath)
                free(fullpath);
            return -1; 
        }

        filetype_t filetype = get_file_type((const struct stat*)&st);

        if (long_format) {
            print_long_format(&st, filetype);
        }

        print_color_name(filetype, fullpath, f_name);
        
        if (long_format)
            printf("\n");
        else
            printf("  ");

        if (NULL != fullpath)
            free(fullpath);
    }
    printf("\n");
 
    if (NULL != file_list)
        free(file_list);

    return 0;
}

