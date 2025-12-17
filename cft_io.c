
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "cft_types.h"
#include "cft_io.h"

int StoreDump(char *path, const StatData *data, size_t count)
{
    if (!path)
    {
        errno = EINVAL;
        return -1;
    }

    if (!data && count > 0)
    {
        errno = EINVAL;
        return -1;
    }

    if (count > MAX_LENGTH)
    {
        fprintf(stderr, "Число записей в файле '%s' не должно превышать %d!\n", path, MAX_LENGTH);
        errno = EINVAL;
        return -1;
    }

    FILE *fp = fopen(path, "wb");
    if (!fp)
    {
        int err = errno;
        fprintf(stderr, "Ошибка открытия файла '%s': %s\n", path, strerror(errno));
        errno = err;
        return -1;
    }

    unsigned long long num = (unsigned long long)count;
    if ((fwrite(&num, sizeof(num), 1, fp) != 1) ||
        (count > 0 && fwrite(data, sizeof(*data), count, fp) != count))
    {
        int err = errno;
        fclose(fp);
        errno = err;
        return -1;
    }

    fclose(fp);

    return 0;
}

int LoadDump(char *path, StatData **data, size_t *count)
{
    if (!path || !data || !count)
    {
        errno = EINVAL;
        return -1;
    }

    int err = 0;

    FILE *fp = fopen(path, "rb");
    if (!fp)
    {
        err = errno;
        fprintf(stderr, "Ошибка открытия файла '%s': %s\n", path, strerror(errno));
        errno = err;
        return -1;
    }

    do
    {
        unsigned long long num = 0;
        if (fread(&num, sizeof(num), 1, fp) != 1)
        {
            err = errno;
            break;
        }

        if (num == 0)
        {
            *data = NULL;
            *count = 0;
            break;
        }
        else if (num > MAX_LENGTH)
        {
            err = EINVAL;
            fprintf(stderr, "Число записей в файле '%s' не должно превышать %d!\n", path, MAX_LENGTH);
            break;
        }

        StatData *buf = malloc(num * sizeof(*buf));
        if (!buf)
        {
            err = ENOMEM;
            break;
        }

        if (fread(buf, sizeof(*buf), num, fp) != num)
        {
            err = errno;
            free(buf);
            break;
        }

        *data = buf;
        *count = num;
    } while (0);

    fclose(fp);

    if (err)
        errno = err;

    return err ? -1 : 0;
}
