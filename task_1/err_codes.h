#ifndef ERROR_H_GUARD
#define ERROR_H_GUARD

#include <stdio.h>
#include <errno.h>
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

/** @file */

#define CHECK_ERR(res, err_code, msg, perr_flag)   \
                    if(res == -1){   \
                        fprintf(stderr, "ERROR = %d (%s) at %s: %s in %d line\n", err_code, (msg), __FILE__, __PRETTY_FUNCTION__, __LINE__);   \
                        if(perr_flag){  \
                            perror( (msg) );    \
                        }   \
                        return err_code;    \
                    }

#define CHECK_ERR_NULL(arg, err_code)   \
                    if(arg == NULL){   \
                        fprintf(stderr, "Input arg %s is NULL at %s: %s in %d line\n", (#arg) , __FILE__, __PRETTY_FUNCTION__, __LINE__);   \
                        return err_code;    \
                    }

//!@enum Enumeration of error codes
enum Errors{
    //! Error code is unknown
    ERR_UNKNOWN = -1,
    //! Error in writer in main: wrong input arguments number
    ERR_WRITER_ARGC = 1,
    //! Error in writer in make_file(const char * argv): argv is NULL
    ERR_WRITER_MAKE_FILE_ARG,
    //! Error in writer in make_file(const char * argv): strdup has return NULL
    ERR_WRITER_MAKE_FILE_STRDUP, 
    //! Error in writer in make_file(const char * argv): error while opening file
    ERR_WRITER_MAKE_FILE_OPEN,
    //! Error in writer in make_global_fifo(): error while creating FIFO
    ERR_WRITER_MAKE_GLOBAL_FIFO_MKFIFO,
    //! Error in writer in make_global_fifo(): error while opening file
    ERR_WRITER_MAKE_GLOBAL_FIFO_OPEN,
    //! Error in writer in make_uniq_fifo(char * fifo_name): fifo_name is NULL
    ERR_WRITER_MAKE_UNIQ_FIFO_ARG,
    //! Error in writer in make_uniq_fifo(char * fifo_name): FIFO with yje same name exists and couldn't be removed
    ERR_WRITER_MAKE_UNIQ_FIFO_MKFIFO_RMV,
    //! Error in writer in make_uniq_fifo(char * fifo_name): error while creating unique FIFO
    ERR_WRITER_MAKE_UNIQ_FIFO_MKFIFO_OTHER,
};


#endif //! ERROR_H_GUARD