
#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/wait.h>
#include "cft_types.h"
#include "cft_io.h"

/* Содержимое для исходных файлов */ 
const StatData case_1_in_a[2] =  
    {{.id = 90889, .count = 13, .cost = 3.567, .primary = 0, .mode=3 }, 
    {.id = 90089, .count = 1, .cost = 88.90, .primary = 1, .mode=0 }}; 
const StatData case_1_in_b[2] =  
    {{.id = 90089, .count = 13, .cost = 0.011, .primary = 0, .mode=2 }, 
    {.id = 90189, .count = 1000, .cost = 1.00003, .primary = 1, .mode=2}}; 

/* Ожидаемый результат обработки */ 
const StatData case_1_out[3] =  
    {{.id = 90189, .count = 1000, .cost = 1.00003, .primary = 1, .mode = 2 }, 
    {.id = 90889, .count = 13, .cost = 3.567, .primary = 0, .mode = 3 }, 
    {.id = 90089, .count = 14, .cost = 88.911, .primary = 0, .mode = 2 }}; 

int main(int argc, char* argv[])
{
    if (argc != 5)
    {
        printf("Usage:\n  %s <proc_util_path> <input_path1> <input_path2> <output_path>\n", argv[0]);
        return EXIT_FAILURE;
    }

    char* path_in_a = argv[2];
    char* path_in_b = argv[3];
    char* path_out = argv[4];

    if (StoreDump(path_in_a, case_1_in_a, sizeof(case_1_in_a)/sizeof(*case_1_in_a)))
    {
        fprintf(stderr, "Ошибка записи итогового массива в файл '%s': %s\n", path_in_a, strerror(errno));
        return EXIT_FAILURE;
    }

    if (StoreDump(path_in_b, case_1_in_b, sizeof(case_1_in_b)/sizeof(*case_1_in_b)))
    {
        fprintf(stderr, "Ошибка записи итогового массива в файл '%s': %s\n", path_in_b, strerror(errno));
        return EXIT_FAILURE;
    }

    pid_t pid = fork();

    if (pid < 0)
    {
        perror("fork");
        return EXIT_FAILURE;
    }
    else if (pid == 0)
    {
        // Это дочерний процесс
        execl(argv[1], argv[1], path_in_a, path_in_b, path_out, (char *) NULL); 
        perror("execl");
        exit(EXIT_FAILURE);
    }
    else
    {
        // Это родительский процесс, ждем завершения дочернего процесса
        waitpid(pid, NULL, 0);
    }

    StatData *data_out = NULL;
    size_t count_out = 0;
    if (LoadDump(path_out, &data_out, &count_out))
    {
        fprintf(stderr, "Ошибка чтения массива записей из файла '%s': %s\n", path_out, strerror(errno));
        return EXIT_FAILURE;
    }

    if (sizeof(case_1_out)/sizeof(*case_1_out) == count_out &&
        memcmp(case_1_out, data_out, count_out) == 0)
    {
        printf("Тесты выполнены успешно!\n");
    }
    else
    {
        fprintf(stderr, "Ошибка прохождения тестов!\n");
    }

    free(data_out);

    return EXIT_SUCCESS;
}
