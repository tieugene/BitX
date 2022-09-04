#ifdef BB_WIN
#include <glib.h>
#include <glib/gstdio.h>
#else
#include <dirent.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#endif

#include "bg_dirent.h"

#define BG_DIRENT_ERRS_N 8                  // Number of errors.
#define BG_BUF_LEN 256

struct bg_dirent_data {
#ifdef BB_WIN
    GDir *dir;                              // Directory descriptor.
#else
    DIR *dir;                               // Directory descriptor.
#endif
    char *path;                             // Path memory pointer.
    char *p;                                // File name position in path memory pointer.
    size_t max_filename_size;               // File name length in bytes (including '\0') for which memory is allocated.
    size_t rest_path_size;                  // File path length not including file name (dir path length and slash), not including '\0'.
};

/* Sets error data. */
static void bg_dirent_set_error(int err_code, struct bb_error *perr);

int bg_dirent_init(const char *path, struct bg_dirent_data **ppdd, struct bb_error *perr)
{
    size_t len;
    int n, err_code;

    err_code = 0;
    if (ppdd)
        *ppdd = NULL;

    if (!path || !ppdd)
        err_code = 3;
    else if (!*path)
        err_code = 4;
    else
    {
        *ppdd = (struct bg_dirent_data *) calloc(1, sizeof(struct bg_dirent_data));
        if (!*ppdd)
            err_code = 1;
        else
        {
            len = strlen(path);
            if (BG_SLASH == *(path + len - 1))
                --len;
            (*ppdd)->rest_path_size = len + 1;                              // 1 BG_SLASH.
            (*ppdd)->max_filename_size = BG_BUF_LEN + 1;                    // 1 '\0'.

            (*ppdd)->path = (char *) calloc(((*ppdd)->rest_path_size + (*ppdd)->max_filename_size), 1);
            if (!(*ppdd)->path)
                err_code = 1;
            else
            {
                if (BG_SLASH == *(path + len))
                    n = snprintf((*ppdd)->path, (*ppdd)->rest_path_size + (*ppdd)->max_filename_size, "%s", path);
                else
                    n = snprintf((*ppdd)->path, (*ppdd)->rest_path_size + (*ppdd)->max_filename_size, "%s%c", path, BG_SLASH);
                if ((n < 0) || (n != (*ppdd)->rest_path_size))
                    err_code = 5;
                else
                {
                    (*ppdd)->p = (*ppdd)->path + n;
#ifdef BB_WIN
    (*ppdd)->dir = g_dir_open(path, 0, NULL);                       // Open directory.
#else
    (*ppdd)->dir = opendir(path);                                   // Open directory.
#endif
                    if (!(*ppdd)->dir)
                        err_code = 6;
                }
            }
        }
    }

    if (err_code)
        bg_dirent_deinit(ppdd, NULL);

    if (err_code && perr)
        bg_dirent_set_error(err_code, perr);
    return err_code;
}

int bg_dirent_iter(struct bg_dirent_data *pdd, const char **ppath, size_t *ppath_len, struct bb_error *perr)
{
#ifdef BB_WIN
    const char *name;
#else
    //struct dirent d = {0};
    struct dirent *pd;
#endif
    const char *t;
    char *p, *p2;
    int n, err_code;

    err_code = 0;

    if (!pdd || !ppath)
        err_code = 3;
    else
    {
        if (ppath_len)
            *ppath_len = 0;
#ifdef BB_WIN
        for (*ppath = NULL, name = g_dir_read_name(pdd->dir); name; name = g_dir_read_name(pdd->dir))
            if (*name && 
                ((*name != '.') || (*(name + 1) && 
                ((*(name + 1) != '.') || *(name + 2)))))
            {
                for (t = name, p = pdd->p, p2 = p + pdd->max_filename_size - 1; (p < p2) && *t; )
                    *p++ = *t++;
                if (*t)
                {
                    n = strlen(name);
                    while (n >= pdd->max_filename_size)
                        pdd->max_filename_size += BG_BUF_LEN;
                    pdd->path = (char *) realloc(pdd->path, (pdd->max_filename_size + pdd->rest_path_size));
                    if (!pdd->path)
                    {
                        err_code = 1;
                        break;
                    }
                    else
                    {
                        pdd->p = pdd->path + pdd->rest_path_size;
                        for (p = pdd->p + (t - name), p2 = pdd->p + pdd->max_filename_size - 1; (p < p2) && *t; )
                            *p++ = *t++;
                    }
                }
                *p = '\0';

                //if (g_file_test(pdd->path, G_FILE_TEST_IS_DIR))
                if (!g_file_test(pdd->path, G_FILE_TEST_IS_DIR))
                {
                    *ppath = pdd->path;
                    if (ppath_len)
                        *ppath_len = p - *ppath;
                    break;
                }
            }
#else
        //for (*ppath = NULL, readdir_r(pdd->dir, &d, &pd); pd; readdir_r(pdd->dir, &d, &pd))
        for (*ppath = NULL, pd = readdir(pdd->dir); pd; pd = readdir(pdd->dir))
/*
            if ((DT_DIR == pd->d_type) && 
                *pd->d_name && 
                ((*pd->d_name != '.') || (*(pd->d_name + 1) && 
                ((*(pd->d_name + 1) != '.') || *(pd->d_name + 2)))))
*/
            if ((DT_DIR != pd->d_type) && *pd->d_name)
            {
                for (t = pd->d_name, p = pdd->p, p2 = p + pdd->max_filename_size - 1; (p < p2) && *t; )         // Copy dir name.
                    *p++ = *t++;
                if (*t)                                                             // Dir name is too long - reallocate memory.
                {
                    n = strlen(pd->d_name);
                    while (n >= pdd->max_filename_size)
                        pdd->max_filename_size += BG_BUF_LEN;
                    pdd->path = (char *) realloc(pdd->path, (pdd->max_filename_size + pdd->rest_path_size));
                    if (!pdd->path)
                    {
                        err_code = 1;
                        break;
                    }
                    else
                    {
                        pdd->p = pdd->path + pdd->rest_path_size;
                        for (p = pdd->p + (t - pd->d_name), p2 = pdd->p + pdd->max_filename_size - 1; (p < p2) && *t; )
                            *p++ = *t++;
                    }
                }
                *p = '\0';

                *ppath = pdd->path;
                if (ppath_len)
                    *ppath_len = p - *ppath;
                break;
            }
#endif
    }

    if (err_code && perr)
        bg_dirent_set_error(err_code, perr);
    return err_code;
}

int bg_dirent_deinit(struct bg_dirent_data **ppdd, struct bb_error *perr)
{
#ifndef BB_WIN          // !ifndef
    int err_code;

    err_code = 0;
#endif

    if (ppdd && *ppdd)
    {
#ifdef BB_WIN
        if ((*ppdd)->dir)
            g_dir_close((*ppdd)->dir);
#else
        if ((*ppdd)->dir && (closedir((*ppdd)->dir) < 0))
            err_code = 7;
#endif

        if ((*ppdd)->path)
            free((*ppdd)->path);

        free(*ppdd);
        *ppdd = NULL;
    }

#ifdef BB_WIN
    return 0;
#else
    if (err_code && perr)
        bg_dirent_set_error(err_code, perr);
    return err_code;
#endif
}

const char * bg_dirent_error_message(int err_code)
{
    static const char *BG_DIRENT_ERRS[BG_DIRENT_ERRS_N] = { "ОК",                                                           //  0
                                                            "Ошибка выделения памяти",                                      //  1
                                                            "Неизвестная ошибка",                                           //  2
                                                            "Пустой указатель",                                             //  3
                                                            "Путь к директории не может быть пустым",                       //  4
                                                            "Ошбика snprintf",                                              //  5
                                                            "Ошбика при открытии директории",                               //  6
                                                            "Ошбика при закрытии директории",                               //  7
    };  

    if ((err_code < 0) || (err_code >= BG_DIRENT_ERRS_N))
        err_code = 2;

    return *(BG_DIRENT_ERRS + err_code);
}

static void bg_dirent_set_error(int err_code, struct bb_error *perr)
{
    perr->err_code = err_code;
    perr->err_str = bg_dirent_error_message(err_code);
}
