#ifdef BB_WIN
#include <glib.h>
#include <glib/gstdio.h>
#else
#include <stdlib.h>
#include <string.h>
#endif

#include "bg_threads.h"
#include "bg_ts_error.h"

#define BG_TS_ERROR_ERRS_N 4
#define BG_MSG_BUF_SIZE 1024

struct bg_ts_error {
    BG_MUTEX_T mutex;
    struct bb_error err;
    FILE *log_f;
};

/*  Sets error data.  */
static void bg_ts_error_set_error(int err_code, struct bb_error *perr);

int bg_ts_error_init(struct bg_ts_error **pptse, FILE *log_f, struct bb_error *perr)
{
    int err_code;

    err_code = 0;

    if (!pptse)
        err_code = 3;
    else
    {
        *pptse = (struct bg_ts_error *) calloc(1, sizeof(struct bg_ts_error));
        if (!*pptse)
            err_code = 1;
        else
        {
            BG_MUTEX_INIT(&(*pptse)->mutex);
            (*pptse)->log_f = log_f;
        }
    }

    if (err_code && perr)
        bg_ts_error_set_error(err_code, perr);
    return err_code;
}

void bg_ts_error_set_error_data(struct bg_ts_error *ptse, struct bb_error *perr_data, const char *path, const char *msg)
{
    char temp[BG_MSG_BUF_SIZE] = {0};
#ifdef BB_WIN
    char *p;

    p = NULL;
#endif

    if (ptse && perr_data)
    {
        if (path)
        {
            if (!msg)
            {
                snprintf(temp, BG_MSG_BUF_SIZE, "%s: %s", 
                    path, 
                    (!perr_data->err_str ? "Неизвестная ошибка" : perr_data->err_str));
            }
            else
            {
                snprintf(temp, BG_MSG_BUF_SIZE, "%s %s: %s", 
                    path, msg, 
                    (!perr_data->err_str ? "Неизвестная ошибка" : perr_data->err_str));
            }
#ifdef BB_WIN
            p = g_convert(temp, -1, "866", "UTF-8", NULL, NULL, NULL);
#endif
        }

        BG_MUTEX_LOCK(&ptse->mutex);

        if (path)
        {
#ifdef BB_WIN
            puts(!p ? temp : p);
#else
            puts(temp);
#endif
            if (ptse->log_f)
                fprintf(ptse->log_f, "%s\n", temp);
        }

        if (!ptse->err.err_code)
            memcpy(&ptse->err, perr_data, sizeof(struct bb_error));
        BG_MUTEX_UNLOCK(&ptse->mutex);

#ifdef BB_WIN
        if (p)
            g_free(p);
#endif
    }
}

void bg_ts_error_get_error_data(struct bg_ts_error *ptse, struct bb_error *perr_data)
{
    if (ptse && perr_data)
    {
        BG_MUTEX_LOCK(&ptse->mutex);
        memcpy(perr_data, &ptse->err, sizeof(struct bb_error));
        BG_MUTEX_UNLOCK(&ptse->mutex);
    }
}

int bg_ts_error_get_error_code(struct bg_ts_error *ptse)
{
    int err_code;

    if (!ptse)
        err_code = 0;
    else
    {
        BG_MUTEX_LOCK(&ptse->mutex);
        err_code = ptse->err.err_code;
        BG_MUTEX_UNLOCK(&ptse->mutex);
    }

    return err_code;
}

void bg_ts_error_deinit(struct bg_ts_error **pptse)
{
    if (pptse && *pptse)
    {
        BG_MUTEX_DEINIT(&(*pptse)->mutex);

        free(*pptse);
        *pptse = NULL;
    }
}

const char * bg_ts_error_error_message(int err_code)
{
    static const char *BG_TS_ERROR_ERRS[BG_TS_ERROR_ERRS_N] = { "ОК",                       //  0
                                                                "Ошибка выделения памяти",  //  1
                                                                "Неизвестная ошибка",       //  2
                                                                "Пустой указатель",         //  3
    };  

    if ((err_code < 0) || (err_code >= BG_TS_ERROR_ERRS_N))
        err_code = 2;

    return *(BG_TS_ERROR_ERRS + err_code);
}

static void bg_ts_error_set_error(int err_code, struct bb_error *perr)
{
    perr->err_code = err_code;
    perr->err_str = bg_ts_error_error_message(err_code);
}
