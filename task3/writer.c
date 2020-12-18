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


    //! Освобождаем ресурсы и "отпускаем" семафоры писателя
    res = semop(sem_id, sem_wr_finish, SEM_STRUCT_SIZE(sem_wr_finish) );
    CHECK_ERR(res, ERR_SHMEM_WR_END, "Error while free writer semaphores", 1);

    res = shmdt(shm_addr);
    CHECK_ERR(res, ERR_SHMEM_SEM_SHMDT, "Error while detaching shared memory in writer", 1);

    return 0;
}