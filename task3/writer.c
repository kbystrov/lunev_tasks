#include "common.h"

const int num_argc = 2;

int make_file(const char * argv){

	CHECK_ERR_NULL(argv, ERR_SHMEM_SEM_MAKE_FILE_ARG );

	char * input_file_name = strdup(argv);
	CHECK_ERR_NULL(input_file_name, ERR_SHMEM_SEM_MAKE_FILE_STRDUP);

	int file_fd = open(input_file_name, O_RDONLY);
	CHECK_ERR(file_fd, ERR_SHMEM_SEM_MAKE_FILE_OPEN, "Error while opening input file", 1);

	free(input_file_name);

	return file_fd;
}

int produce_item(char * buf, int fd){

    CHECK_ERR_NULL(buf, ERR_SHMEM_PRODUCE_ITEM_INPUT);

    int res = read(fd, buf, PAGE_SIZE);
    CHECK_ERR(res, ERR_SHMEM_PRODUCE_ITEM_READ, "Error while reading from file", 1);

    return res;
}

int sem_init_writer(int sem_id){

    union semun arg;

    arg.val = 1;
    int res = semctl(sem_id, SEM_FINISH, SETVAL, arg);
    CHECK_ERR(res, ERR_SHMEM_SEMCTL_FINISH, "Error while setting SEM_FINISH to 0", 1);

    arg.val = 1;
    res = semctl(sem_id, SEM_FULL, SETVAL, arg);
    CHECK_ERR(res, ERR_SHMEM_SEMINIT_FULL, "Error while initializing start values for semaphores", 1);

    res = semop(sem_id, sem_wr_init, SEM_STRUCT_SIZE(sem_wr_init) );
    CHECK_ERR(res, ERR_SHMEM_WR_INIT, "Error while initializing start values in writer", 1);

    return res;
}

int main(int argc, char ** argv) {

    if(argc < num_argc){
		printf("Wrong number of argc = %d! Should be %d!\n", argc, num_argc);
		return ERR_SHMEM_SEM_ARGC;
	}

    int res = 0;

    //! Получаем указатель на входной файл и открываем его
	int file_fd = make_file(argv[1]);
	if(file_fd < 0){
		fprintf(stderr, "Can't open input file writer");
		return file_fd;
	}

    key_t key = ftok(key_name, 0);
    CHECK_ERR(key, ERR_SHMEM_SEM_FTOK, "Error while getting key_t in writer", 1);

    //! Получаем разделяемую память и отображаем ее в адресное пространство процесса
    char * shm_addr = NULL;
    int shm_id = init_shmem(key, &shm_addr);
    if(shm_id < 0){
        return shm_id;
    }

    //! Получаем семафоры
    int sem_id = semget(key, SEM_NUM, PERM_MODE | IPC_CREAT);
    CHECK_ERR(sem_id, ERR_SHMEM_SEM_SEMGET, "Error while getting semaphores set", 1);

    //! Ждем, пока не займем семафор для писателя (Гарантирует единственного активного писателя)
    res = semop(sem_id, sem_wr_start, SEM_STRUCT_SIZE(sem_wr_start) );
    CHECK_ERR(res, ERR_SHMEM_WR_CHECK, "Error while starting writer", 1);

    //! Ждем, когда не освободится семафор на пару читатель-писатель
    res = semop(sem_id, sem_wait_pair, SEM_STRUCT_SIZE(sem_wait_pair));
    CHECK_ERR(res, ERR_SHMEM_WAIT_PAIR, "Error while waiting pair semaphore", 1);

    //! Инициализируем семафоры перед началом передачи данных
    res = sem_init_writer(sem_id);
    if(res < 0){
        return res;
    }

    //! Ждем готовность читателя
    res = semop(sem_id, sem_wr_wait_rd, SEM_STRUCT_SIZE(sem_wr_wait_rd) );
    CHECK_ERR(res, ERR_SHMEM_WR_WAIT_RD, "Error while waiting reader in writer", 1);

    shm_buf tmp_buf = {};

    //! Цикл чтения из файла и передачи данных читателю
    while(1){
        //! P(empty);
        res = semop(sem_id, sem_p_empty, SEM_STRUCT_SIZE(sem_p_empty) );
        CHECK_ERR_NORET(res, ERR_SHMEM_P_EMPTY, "Error while P(empty) in writer", 1);
        //! P(mutex);
        /*
        res = semop(sem_id, sem_p_mutex, SEM_STRUCT_SIZE(sem_p_mutex) );
        CHECK_ERR_NORET(res, ERR_SHMEM_P_MUTEX, "Error while P(mutex) in writer", 1);
        */
        //! Проверяем жив ли читатель
        res = semop(sem_id, sem_wr_check_rd_alive, SEM_STRUCT_SIZE(sem_wr_check_rd_alive) );
        if(res == -1){
            break;
        }
        //! Читаем данные из файла в буфер
        res = produce_item(tmp_buf.buf, file_fd);
        if(res < 0){ //< Ошибка при чтении файла
            perror("Error while procuding item");
            break;
        } else if (res == 0){ //< EOF. Уведомляем читателя об окончании передачи данных
        /*
            res = semop(sem_id, sem_wr_finish, SEM_STRUCT_SIZE(sem_wr_finish) );
            CHECK_ERR_NORET(res, ERR_SHMEM_WR_FINISH, "Error while notifying reader about the finish of transmitting", 1);
        */  
            break;
        }
        tmp_buf.len = res;
        //! put_item();
        memcpy(shm_addr, &tmp_buf, sizeof(tmp_buf));
        //! V(mutex);
        /*
        res = semop(sem_id, sem_v_mutex, SEM_STRUCT_SIZE(sem_v_mutex) );
        CHECK_ERR_NORET(res, ERR_SHMEM_V_MUTEX, "Error while V(mutex) in writer", 1);
        */
        //! V(full);
        res = semop(sem_id, sem_v_full, SEM_STRUCT_SIZE(sem_v_full) );
        CHECK_ERR_NORET(res, ERR_SHMEM_V_FULL, "Error while V(full) in writer", 1);
    }

    close(file_fd);

    res = shmdt(shm_addr);
    CHECK_ERR(res, ERR_SHMEM_SEM_SHMDT, "Error while detaching shared memory in writer", 1);

    res = shmctl(shm_id, IPC_RMID, NULL);
    CHECK_ERR(res, ERR_SHMEM_SEM_SEMRM, "Error while deleting shared memory in writer", 1);

    return 0;
}