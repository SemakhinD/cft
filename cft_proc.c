
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "cft_types.h"
#include "cft_io.h"

void JoinDumpInternal(StatData *data, size_t count, StatData *out_data, size_t *out_count)
{
    if (!data || !count || !out_data || !out_count)
        return; 

    for (size_t i = 0; i < count; i++)
    {
        BOOL exist = FALSE;
        StatData *cur = data + i;
        size_t j = 0;
        for (; j < *out_count; j++)
        {
            if (out_data[j].id == cur->id)
            {
                exist = TRUE;
                break;
            }
        }
        if (!exist)
        {
            memcpy(out_data + j, cur, sizeof(*cur));
            (*out_count)++;
        }
        else
        {
            // поля count и cost должны складываться
            out_data[j].count += cur->count;
            out_data[j].cost += cur->cost;
            // поле primary должно иметь значение 0 если хотя бы в одном из элементов оно 0,
            // в противном случае 1
            out_data[j].primary = (out_data[j].primary == 0 || cur->primary == 0) ? 0 : 1;
            // поле mode должно иметь максимальное значение из двух представленных
            out_data[j].mode = (cur->mode > out_data[j].mode) ? cur->mode : out_data[j].mode;
        }
    }
}

int JoinDump(StatData *data1, size_t count1, StatData *data2, size_t count2, StatData **out_data, size_t *out_count)
{
    if ((!data1 && count1 > 0) || (!data2 && count2 > 0) || !out_data || !out_count)
    {
        errno = EINVAL;
        return -1;
    }

    size_t total_count = count1 + count2;
    if (total_count == 0)
    {
        *out_data = NULL;
        *out_count = 0;
        return 0;
    }
    else if (total_count > MAX_LENGTH)
    {
        fprintf(stderr, "Число записей в объединенном массиве не должно превышать %d!\n", MAX_LENGTH);
        errno = EINVAL;
        return -1;
    }

    // Выделяем память с расчетом на то, что у каждой записи уникальный id
    StatData *local_out_data = malloc(total_count * sizeof(*local_out_data));
    if (!local_out_data)
    {
        errno = ENOMEM;
        return -1;
    }

    JoinDumpInternal(data1, count1, local_out_data, out_count);
    JoinDumpInternal(data2, count2, local_out_data, out_count);
    *out_data = local_out_data;

    return 0;
}

int compare_data(const void *a, const void *b) {
    StatData* data_a = (StatData*)a;
    StatData* data_b = (StatData*)b;
    if (data_a->cost > data_b->cost) return 1; // a > b
    if (data_a->cost < data_b->cost) return -1; // a < b
    return 0; // a == b
}

int SortDump(StatData *data, size_t count)
{
    if (!data && count > 0)
    {
        errno = EINVAL;
        return -1;
    }

    if (count > 1)
    {
        // Сортировка массива структур StatData в порядке возрастания значения поля cost
        qsort(data, count, sizeof(*data), compare_data);
    }

    return 0;
}

int PrintDump(StatData *data, size_t count, size_t count_print)
{
    if (!data && count > 0)
    {
        errno = EINVAL;
        return -1;
    }

    printf("|    №   |        id        |    count    |    cost    | primary | mode |\n");
    printf("|--------|------------------|-------------|------------|---------|------|\n");
    for (size_t i = 0; i < (count_print < count ? count_print : count); i++)
    {
        char c = (data[i].primary ? 'y' : 'n');
        printf("| %6ld | %16lX | %11d | %10.3e |    %c    | %d%d%d  | \n", 
            i + 1, data[i].id, data[i].count, data[i].cost, c, 
            (data[i].mode >> 2) & 1, (data[i].mode >> 1) & 1, data[i].mode & 1);
    }
    printf("|--------|------------------|-------------|------------|---------|------|\n");

    return 0;
}

int main(int argc, char* argv[])
{
    if (argc != 4)
    {
        printf("Использование:\n  %s <файл_1> <файл_2> <выходной_файл>\n", argv[0]);
        return -1;
    }

    char* in_path_1 = argv[1];
    char* in_path_2 = argv[2];
    char* out_path = argv[3];
    StatData* in_data_1 = NULL;
    StatData* in_data_2 = NULL;
    StatData* out_data = NULL;
    size_t in_count_1 = 0;
    size_t in_count_2 = 0;
    size_t out_count = 0;

    int ret = -1;
    do
    {
        if (LoadDump(in_path_1, &in_data_1, &in_count_1))
        {
            fprintf(stderr, "Ошибка чтения массива записей из файла '%s': %s\n", in_path_1, strerror(errno));
            break;
        }

        if (LoadDump(in_path_2, &in_data_2, &in_count_2))
        {
            fprintf(stderr, "Ошибка чтения массива записей из файла '%s': %s\n", in_path_2, strerror(errno));
            break;
        }

        if (JoinDump(in_data_1, in_count_1, in_data_2, in_count_2, &out_data, &out_count))
        {
            fprintf(stderr, "Ошибка объединения двух массивов: %s\n", strerror(errno));
            break;
        }

        if (SortDump(out_data, out_count))
        {
            fprintf(stderr, "Ошибка сортировки массива: %s\n", strerror(errno));
            break;
        }

        if (StoreDump(out_path, out_data, out_count))
        {
            fprintf(stderr, "Ошибка записи отсортированного массива в файл '%s': %s\n", out_path, strerror(errno));
            break;
        }

        if (PrintDump(out_data, out_count, TEST_PRINT_COUNT))
        {
            fprintf(stderr, "Ошибка печати записей в виде таблицы: %s\n", strerror(errno));
            break;
        }

        printf("Данные обработаны успешно!\n");

        ret = 0;
    } while (0);
    
    free(in_data_1);
    free(in_data_2);
    free(out_data);

    return ret;
}
