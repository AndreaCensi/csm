#ifndef H_SM_UTILS
#define H_SM_UTILS

#include <stdio.h>

/** Tries to open a file for reading; if filename == "-" or "stdin", returns
    standard input. An error message is printed in case of error. */
FILE * open_file_for_reading(const char*filename);

/** Tries to open a file for reading; if filename == "-" | "stdout", returns
    standard output. If it's "stderr" same.  
    An error message is printed in case of error. */
FILE * open_file_for_writing(const char*filename);


#endif