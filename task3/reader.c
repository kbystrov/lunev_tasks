#include "common.h"

int main() {

    int res = 0;

    key_t key = ftok(key_name, 0);
    CHECK_ERR(key, ERR_SHMEM_SEM_FTOK, "Error while getting key_t in reader", 1);

    //! Получаем разделяемую память и отображаем ее в адресное пространство процесса
    char * shm_addr = NULL;
    int shm_id = init_shmem(key, &shm_addr);
    if(shm_id < 0){
        return shm_id;
    }

    //! Получаем семафоры
    int sem_id = semget(key, SEM_NUM, PERM_MODE | IPC_CREAT);
    CHECK_ERR(sem_id, ERR_SHMEM_SEM_SEMGET, "Error while getting semaphores set", 1);

    //! Ждем, пока не займем семафор для читателя (Гарантирует единственного активного читателя)
    res = semop(sem_id, sem_rd_start, SEM_STRUCT_SIZE(sem_rd_start) );
    CHECK_ERR(res, ERR_SHMEM_RD_CHECK, "Error while starting writer", 1);

    //! Ждем готовность писателя
    res = semop(sem_id, sem_rd_wait_wr, SEM_STRUCT_SIZE(sem_rd_wait_wr) );
    CHECK_ERR(res, ERR_SHMEM_RD_WAIT_WR, "Error while waiting writer in reader", 1);

    //! Освобождаем ресурсы и "отпускаем" семафоры читателя
    res = semop(sem_id, sem_rd_finish, SEM_STRUCT_SIZE(sem_rd_finish) );
    CHECK_ERR(res, ERR_SHMEM_RD_END, "Error while free reader semaphores", 1);

    res = shmdt(shm_addr);
    CHECK_ERR(res, ERR_SHMEM_SEM_SHMDT, "Error while detaching shared memory in reader", 1);

    return 0;
}