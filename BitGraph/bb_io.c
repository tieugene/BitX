#include "bb_io.h"

#define BB_IO_ERRS_N 9                      // Number of errors.

#ifdef BB_WIN
#include <glib.h>
#include <glib/gstdio.h>
#else
#include <stdio.h>
#include <stdlib.h>
#endif

/* Sets error data. */
static void set_error(int err_code, struct bb_error *perr);

int bb_read_file(const char *file_name, char **pdata, size_t *plen, struct bb_error *perr)
{
    FILE *f;
    long int len;
    int err_code;

    err_code = 0;

    if (!file_name || !pdata)
        err_code = 3;
    else
    {
        *pdata = NULL;
        if (plen)
            *plen = 0;

#ifdef BB_WIN
        f = g_fopen(file_name, "rb");
#else
        f = fopen(file_name, "rb");
#endif

        if (!f)
            err_code = 4;
        else
        {
            if (fseek(f, 0L, SEEK_END))
                err_code = 5;
            else
            {
                len = ftell(f);
                if (len <= 0)
                    err_code = 6;
                else
                {
                    if (fseek(f, 0L, SEEK_SET))
                        err_code = 5;
                    else
                    {
                        //*pdata = (char *) malloc((size_t) (len + 1));
                        *pdata = (char *) calloc((size_t) (len + 1), 1);
                        if (!*pdata)
                            err_code = 1;
                        else if (fread(*pdata, (size_t) len, 1, f) != 1)
                        {
                            free(*pdata);
                            *pdata = NULL;
                            err_code = 7;
                        }
                        else
                        {
                            *(*pdata + (size_t) len) = '\0';
                            if (plen)
                                *plen = (size_t) len;
                        }
                    }
                }
            }
            fclose(f);
        }
    }

    if (err_code && perr)
        set_error(err_code, perr);
    return err_code;
}

int bb_write_file(const char *file_name, const char *data, size_t len, struct bb_error *perr)
{
    FILE *f;
    int err_code;

    err_code = 0;

#ifdef BB_WIN
    f = g_fopen(file_name, "wb");
#else
    f = fopen(file_name, "wb");
#endif

    if (!f)
        err_code = 4;
    else
    {
        if (fwrite(data, len, 1, f) != 1)
            err_code = 8;

        fclose(f);
    }

    if (err_code && perr)
        set_error(err_code, perr);
    return err_code;
}

const char * bb_io_error_message(int err_code)
{
    static const char *BB_IO_ERRS[BB_IO_ERRS_N] = { "ОК",                                                   //  0
                                                    "Ошибка выделения памяти",                              //  1
                                                    "Неизвестная ошибка",                                   //  2
                                                    "Пустой указатель",                                     //  3
                                                    "Не удалось открыть файл",                              //  4
                                                    "Ошибка данных файла",                                  //  5
                                                    "Пустой файл",                                          //  6
                                                    "Ошибка чтения файла",                                  //  7
                                                    "Ошибка записи файла",                                  //  8
    };

    if ((err_code < 0) || (err_code >= BB_IO_ERRS_N))
        err_code = 2;

    return *(BB_IO_ERRS + err_code);
}

static void set_error(int err_code, struct bb_error *perr)
{
    perr->err_code = err_code;
    perr->err_str = bb_io_error_message(err_code);
}