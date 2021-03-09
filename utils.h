/*
*   file: utils.h
*   lsx: short analog of linux program 'ls'. supports only option '-l'
*/

#ifndef	UTILS_H
#define	UTILS_H

typedef enum {
    unknown,
    fifo,
    chardev,
    directory,
    blockdev,
    normal,
    symbolic_link,
    sock,
    whiteout,
    arg_directory
} filetype_t;

void absort(long *ptr, int num_files);
char* make_fullpath(char* dir_path, char* f_name);
void print_long_format(struct stat* st, filetype_t filetype);
void print_color_name(filetype_t filetype, const char* fullpath, const char* f_name);
filetype_t get_file_type(const struct stat* st);

#endif
