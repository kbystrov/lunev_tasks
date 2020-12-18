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
#include "err_codes.h"
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

/** @file */

#define PERM_MODE 0666
#define PAGE_SIZE 4096

const char * key_name = "err_codes.h";

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


struct sembuf sem_wr_start[2] = {
    {SEM_WRITER, 0, 0},
    {SEM_WRITER, 1, SEM_UNDO},
}; 

struct sembuf sem_rd_start[2] = {
    {SEM_READER, 0, 0},
    {SEM_READER, 1, SEM_UNDO},
}; 

struct sembuf sem_wr_finish[1] = {
    {SEM_WRITER, -1, 0},
}; 

struct sembuf sem_rd_finish[1] = {
    {SEM_READER, -1, 0},
}; 

struct sembuf sem_wr_wait_rd[2] = {
    {SEM_READER, -1, SEM_UNDO},
    {SEM_READER, 1, SEM_UNDO},
};

struct sembuf sem_rd_wait_wr[2] = {
    {SEM_WRITER, -1, SEM_UNDO},
    {SEM_WRITER, 1, SEM_UNDO},
};


int init_shmem(key_t key, char ** shm_addr){

    CHECK_ERR_NULL(shm_addr, ERR_SHMEM_SEM_INIT_SHMEM_INPUT);

    int shm_id = shmget(key, sizeof(shm_buf), PERM_MODE | IPC_CREAT);
    CHECK_ERR(shm_id, ERR_SHMEM_SEM_SHMEGET, "Error while getting shared memory", 1);

    char * tmp_addr = shmat(shm_id, NULL, 0);
    if (tmp_addr == (void *) -1){
        perror("Error while attaching shared memory");
        return (-ERR_SHMEM_SEM_SHMAT);
    }
    CHECK_ERR_NULL(tmp_addr, ERR_SHMEM_SEM_SHMAT);

    *shm_addr = tmp_addr;

    return shm_id;
}

#endif //! COMMON_H_GUARD