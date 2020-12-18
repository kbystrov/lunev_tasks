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
                        return (-err_code);    \
                    }

#define CHECK_ERR_NULL(arg, err_code)   \
                    if(arg == NULL){   \
                        fprintf(stderr, "Input arg %s is NULL at %s: %s in %d line\n", (#arg) , __FILE__, __PRETTY_FUNCTION__, __LINE__);   \
                        return (-err_code);    \
                    }

//!@enum Enumeration of error codes
enum Errors{
    //! Error code is unknown
    ERR_UNKNOWN = -1,
    ERR_SHMEM_SEM_ARGC = 1,
    ERR_SHMEM_SEM_MAKE_FILE_ARG, 
    ERR_SHMEM_SEM_MAKE_FILE_STRDUP,
    ERR_SHMEM_SEM_MAKE_FILE_OPEN,
    ERR_SHMEM_SEM_FTOK,
    ERR_SHMEM_SEM_SHMEGET,
    ERR_SHMEM_SEM_SHMDT,
    ERR_SHMEM_SEM_SHMAT,
    ERR_SHMEM_SEM_INIT_SHMEM_INPUT,
    ERR_SHMEM_SEM_SEMGET,
    ERR_SHMEM_WR_CHECK,
    ERR_SHMEM_RD_CHECK,
    ERR_SHMEM_WR_END,
    ERR_SHMEM_RD_END,
};


#endif //! ERROR_H_GUARD