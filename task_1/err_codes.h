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
    //! Error in writer in main: wrong input arguments number
    ERR_WRITER_ARGC = 1,
    //! Error in writer in make_file(const char * argv): argv is NULL
    ERR_WRITER_MAKE_FILE_ARG,
    //! Error in writer in make_file(const char * argv): strdup has return NULL
    ERR_WRITER_MAKE_FILE_STRDUP, 
    //! Error in writer in make_file(const char * argv): error while opening file
    ERR_WRITER_MAKE_FILE_OPEN,
    //! Error in reader in make_global_fifo(): error while creating FIFO
    ERR_READER_MAKE_GLOBAL_FIFO_MKFIFO,
    //! Error in reader in make_global_fifo(): error while opening file
    ERR_READER_MAKE_GLOBAL_FIFO_OPEN,
    //! Error in reader in make_uniq_fifo(char * fifo_name): fifo_name is NULL
    ERR_READER_MAKE_UNIQ_FIFO_ARG,
    //! Error in reader in make_uniq_fifo(char * fifo_name): FIFO with yje same name exists and couldn't be removed
    ERR_READER_MAKE_UNIQ_FIFO_MKFIFO_RMV,
    //! Error in reader in make_uniq_fifo(char * fifo_name): error while creating unique FIFO
    ERR_READER_MAKE_UNIQ_FIFO_MKFIFO_OTHER,
    //! Error in reader in main: error while opening unique FIFO in non-blocking mode
    ERR_READER_UNIQFIFO_OPEN_NBLK,
    //! Error in reader in main: error while changing unique FIFO flags
    ERR_READER_FCNTL,
    //! Error in reader in send_pid(int fd): error while writting PID to fd
	ERR_READER_SEND_PID_WRITE,
    //! Error in reader in send_pid(int fd): wrong number of bytes was written to fd
	ERR_READER_SEND_PID_WSIZE,
    //! Error in writer in get_rd_pid(): error while opening glogal FIFO
    ERR_WRITER_GET_RD_PID_OPEN, 
    //! Error in writer in get_rd_pid(): error while reading from global FIFO
    ERR_WRITER_GET_RD_PID_READ,
    //! Error in writer in get_rd_pid(): wrong number of bytes was read from global FIFO
    ERR_WRITER_GET_RD_PID_RSIZE,
    //! Error in writer in make_uniq_fifo_name(char * fifo_name, int rd_pid): fifo_name is NULL
    ERR_WRITER_MAKE_UNIQ_FIFO_NAME_ARG,
    //! Error in writer in main: error while opening unique FIFO in non-blocking mode
    ERR_WRITER_OPEN_UNIQ_FIFO_NBLK,
    //! Error in writer in main: error while changing unique FIFO flags
    ERR_WRITER_FCNTL,
    //! Error in writer in main: error while removing unique FIFO
    ERR_WRITER_UNIQ_FIFO_RMV,
    //! Error in reader in main: error while removing unique FIFO
    ERR_READER_UNIQ_FIFO_RMV,

};


#endif //! ERROR_H_GUARD