#include "common.h"

const char * key_name = "err_codes.h";

struct sembuf sem_wr_start[2] = {
    {SEM_WRITER, 0, 0},
    {SEM_WRITER, 1, SEM_UNDO},
}; 

struct sembuf sem_rd_start[2] = {
    {SEM_READER, 0, 0},
    {SEM_READER, 1, SEM_UNDO},
}; 

struct sembuf sem_wr_end[1] = {
    {SEM_WRITER, -1, 0},
}; 

struct sembuf sem_rd_end[1] = {
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

struct sembuf sem_wr_finish[1] = {
    {SEM_FINISH, 1, 0},
};

struct sembuf sem_rd_finish[1] = {
    {SEM_FINISH, 0, IPC_NOWAIT},
};

struct sembuf sem_p_empty[1] = {
    {SEM_EMPTY, -1, 0},
};

struct sembuf sem_v_empty[1] = {
    {SEM_EMPTY, 1, 0},
};

struct sembuf sem_p_mutex[1] = {
    {SEM_MUTEX, -1, 0},
};

struct sembuf sem_v_mutex[1] = {
    {SEM_MUTEX, 1, 0},
};

struct sembuf sem_p_full[1] = {
    {SEM_FULL, -1, 0},
};

struct sembuf sem_v_full[1] = {
    {SEM_FULL, 1, 0},
};

struct sembuf sem_rd_check_wr_alive[2] = {
    {SEM_WRITER, -1, IPC_NOWAIT},
    {SEM_WRITER, 1, 0},
}; 

struct sembuf sem_wr_check_rd_alive[2] = {
    {SEM_READER, -1, IPC_NOWAIT},
    {SEM_READER, 1, 0},
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