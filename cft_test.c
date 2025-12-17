
#define _POSIX_C_SOURCE 199309L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>
#include "cft_types.h"
#include "cft_io.h"

/* Содержимое для исходных файлов */ 
const StatData case_1_in_a[] =  
    {{.id = 90889, .count = 13, .cost = 3.567, .primary = 0, .mode=3 }, 
    {.id = 90089, .count = 1, .cost = 88.90, .primary = 1, .mode=0 },
    // проверка count и cost
    {.id = 12345, .count = 35, .cost = 88.90, .primary = 1, .mode=0 },
    {.id = 12345, .count = 46, .cost = -0.9, .primary = 1, .mode=0 },
    // проверка primary и mode
    {.id = 23456, .count = 30, .cost = 88.90, .primary = 0, .mode=4 },
    {.id = 23456, .count = -45, .cost = 0.15, .primary = 1, .mode=2 },
    // неповторяющиеся id
    {.id = 34561, .count = 123, .cost = 88.90, .primary = 1, .mode=0 },
    {.id = 34562, .count = -24, .cost = -0.9, .primary = 1, .mode=3 },
    {.id = 34563, .count = 90, .cost = 5.5, .primary = 1, .mode=1 },
    }; 
const StatData case_1_in_b[] =  
    {{.id = 90089, .count = 13, .cost = 0.011, .primary = 0, .mode=2 }, 
    {.id = 90189, .count = 1000, .cost = 1.00003, .primary = 1, .mode=2},
    // проверка count и cost
    {.id = 12389, .count = 123, .cost = 88.90, .primary = 1, .mode=0 },
    {.id = 12389, .count = -24, .cost = -0.9, .primary = 1, .mode=1 },
    {.id = 12389, .count = 90, .cost = 5.5, .primary = 1, .mode=0 },
    // проверка primary и mode
    {.id = 23478, .count = 123, .cost = 88.90, .primary = 1, .mode=7 },
    {.id = 23478, .count = 500, .cost = 0.32, .primary = 0, .mode=3 },
    {.id = 23478, .count = 90, .cost = 5.51, .primary = 1, .mode=1 },
    // неповторяющиеся id
    {.id = 10000, .count = 67, .cost = 0.45, .primary = 0, .mode=0 },
    {.id = 10002, .count = 100, .cost = 0.55, .primary = 1, .mode=6 }}; 

/* Ожидаемый результат обработки */ 
const StatData case_1_out[] =  
    {{.id = 34562, .count = -24, .cost = -0.9, .primary = 1, .mode=3 },
    {.id = 10000, .count = 67, .cost = 0.45, .primary = 0, .mode=0 },
    {.id = 10002, .count = 100, .cost = 0.55, .primary = 1, .mode=6 },
    {.id = 90189, .count = 1000, .cost = 1.00003, .primary = 1, .mode = 2 }, 
    {.id = 90889, .count = 13, .cost = 3.567, .primary = 0, .mode = 3 }, 
    {.id = 34563, .count = 90, .cost = 5.5, .primary = 1, .mode=1 },
    {.id = 12345, .count = 81, .cost = 88, .primary = 1, .mode=0 },
    {.id = 34561, .count = 123, .cost = 88.90, .primary = 1, .mode=0 },
    {.id = 90089, .count = 14, .cost = 88.911, .primary = 0, .mode = 2 },
    {.id = 23456, .count = -15, .cost = 89.05, .primary = 0, .mode=4 },    
    {.id = 12389, .count = 189, .cost = 93.5, .primary = 1, .mode=1 },
    {.id = 23478, .count = 713, .cost = 94.73, .primary = 0, .mode=7 }}; 

int main(int argc, char* argv[])
{
    if (argc != 5)
    {
        printf("Использование:\n  %s <утилита_обработки_данных> <файл_1> <файл_2> <выходной_файл>\n", argv[0]);
        return EXIT_FAILURE;
    }

    char* path_in_a = argv[2];
    char* path_in_b = argv[3];
    char* path_out = argv[4];

    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    if (StoreDump(path_in_a, case_1_in_a, sizeof(case_1_in_a)/sizeof(*case_1_in_a)))
    {
        fprintf(stderr, "Ошибка формирования файла данных '%s': %s\n", path_in_a, strerror(errno));
        return EXIT_FAILURE;
    }

    if (StoreDump(path_in_b, case_1_in_b, sizeof(case_1_in_b)/sizeof(*case_1_in_b)))
    {
        fprintf(stderr, "Ошибка формирования файла данных '%s': %s\n", path_in_b, strerror(errno));
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
        memcmp(case_1_out, data_out, sizeof(case_1_out)) == 0)
    {
        clock_gettime(CLOCK_MONOTONIC, &end);
        double val = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
        printf("Тесты выполнены успешно!\nВремя выполнения: %.3f секунд\n", val);
    }
    else
    {
        fprintf(stderr, "Ошибка прохождения тестов!\n");
    }

    free(data_out);

    return EXIT_SUCCESS;
}
