#ifndef COMMON_H_GUARD
#define COMMON_H_GUARD

#include <stdio.h>
#include <errno.h>

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

#endif //! COMMON_H_GUARD