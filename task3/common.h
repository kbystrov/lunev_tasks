#ifndef COMMON_H_GUARD
#define COMMON_H_GUARD

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "err_codes.h"
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

/** @file */

#define PERM_MODE 0666
#define PAGE_SIZE 4096

extern const char * key_name;

#define SEM_STRUCT_SIZE(arg) (sizeof(arg) / sizeof(struct sembuf)) 

//!@enum Enumeration of semaphores
enum Sems{
    //! Error code is unknown
    SEM_MUTEX = 0,
    SEM_EMPTY,
    SEM_FULL,
    SEM_WRITER,
    SEM_READER,
    SEM_FINISH,
    SEM_NUM
};

typedef struct shm_buf{
    size_t len;
    char buf[PAGE_SIZE];
} shm_buf;

union semun {
    int              val;    /* Value for SETVAL */
    struct semid_ds *buf;    /* Buffer for IPC_STAT, IPC_SET */
    unsigned short  *array;  /* Array for GETALL, SETALL */
    struct seminfo  *__buf;  /* Buffer for IPC_INFO (Linux-specific) */
};


extern struct sembuf sem_wr_start[2];
extern struct sembuf sem_rd_start[2];
extern struct sembuf sem_wr_end[1];
extern struct sembuf sem_rd_end[1];
extern struct sembuf sem_wr_wait_rd[2];
extern struct sembuf sem_rd_wait_wr[2];
extern struct sembuf sem_wr_finish[1];
extern struct sembuf sem_rd_finish[1];
extern struct sembuf sem_p_empty[1];
extern struct sembuf sem_v_empty[1];
extern struct sembuf sem_p_mutex[1];
extern struct sembuf sem_v_mutex[1];
extern struct sembuf sem_p_full[1];
extern struct sembuf sem_v_full[1];
extern struct sembuf sem_p_full[1];
extern struct sembuf sem_wr_check_rd_alive[2];
extern struct sembuf sem_rd_check_wr_alive[2];

int init_shmem(key_t key, char ** shm_addr);

#endif //! COMMON_H_GUARD