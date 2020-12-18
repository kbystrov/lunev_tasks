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

    shm_buf * tmp_buf = (shm_buf *) shm_addr;

    //! Цикл чтения из разделяемой памяти и печать в stdout
    while(1){
        //! Проверяем SEM_FINISH - выходим если конец передачи
        res = semop(sem_id, sem_rd_finish, SEM_STRUCT_SIZE(sem_rd_finish) );
        if(res == -1){
            break;
        }
        //! Проверяем жив ли gbcfnt
        res = semop(sem_id, sem_rd_check_wr_alive, SEM_STRUCT_SIZE(sem_rd_check_wr_alive) );
        if(res == -1){
            break;
        }
        //! P(full);
        res = semop(sem_id, sem_p_full, SEM_STRUCT_SIZE(sem_p_empty) );
        CHECK_ERR_NORET(res, ERR_SHMEM_P_FULL, "Error while P(full) in wrreaderiter", 1);
        //! P(mutex);
        res = semop(sem_id, sem_p_mutex, SEM_STRUCT_SIZE(sem_p_mutex) );
        CHECK_ERR_NORET(res, ERR_SHMEM_P_MUTEX, "Error while P(mutex) in reader", 1);
        //! get_item()
        write(STDOUT_FILENO, tmp_buf->buf, tmp_buf->len);
        //! V(mutex);
        res = semop(sem_id, sem_v_mutex, SEM_STRUCT_SIZE(sem_v_mutex) );
        CHECK_ERR_NORET(res, ERR_SHMEM_V_MUTEX, "Error while V(mutex) in reader", 1);
        //! V(empty);
        res = semop(sem_id, sem_v_empty, SEM_STRUCT_SIZE(sem_v_full) );
        CHECK_ERR_NORET(res, ERR_SHMEM_V_EMPTY, "Error while V(empty) in reader", 1);
    }

    //! Освобождаем ресурсы и "отпускаем" семафоры читателя
    res = semop(sem_id, sem_rd_end, SEM_STRUCT_SIZE(sem_rd_end) );
    CHECK_ERR(res, ERR_SHMEM_RD_END, "Error while free reader semaphores", 1);

    res = shmdt(shm_addr);
    CHECK_ERR(res, ERR_SHMEM_SEM_SHMDT, "Error while detaching shared memory in reader", 1);

    return 0;
}