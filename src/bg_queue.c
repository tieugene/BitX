#include <stdlib.h>
#include "bg_threads.h"
#include "bg_queue.h"

#define BG_QUEUE_ERRS_N 4

struct bg_queue_node {                  // Queue node.
    void *data;                         // Data pointer.
    struct bg_queue_node *next;         // Next node pointer.
};

struct bg_queue_data {                  // Queue data.
    BG_MUTEX_T mutex;                   // Mutex.
    struct bg_queue_node *first;        // Root (first node of list).
    struct bg_queue_node **pcurrent;    // Current position for inserting.
};

/*  Sets error data.  */
static void bg_queue_set_error(int err_code, struct bb_error *perr);

int bg_queue_init(struct bg_queue_data **ppqd, struct bb_error *perr)
{
    int err_code;

    err_code = 0;

    if (!ppqd)
        err_code = 3;
    else
    {
        *ppqd = (struct bg_queue_data *) calloc(1, sizeof(struct bg_queue_data));
        if (!*ppqd)
            err_code = 1;
        else
        {
            BG_MUTEX_INIT(&(*ppqd)->mutex);
            (*ppqd)->pcurrent = &(*ppqd)->first;
        }
    }

    if (err_code && perr)
        bg_queue_set_error(err_code, perr);
    return err_code;
}

int bg_queue_add(struct bg_queue_data *pqd, void *data, struct bb_error *perr)
{
    struct bg_queue_node *node;
    int err_code;

    err_code = 0;

    if (!pqd || !data)
        err_code = 3;
    else
    {
        node = (struct bg_queue_node *) calloc(1, sizeof(struct bg_queue_node));
        if (!node)
            err_code = 1;
        else
        {
            node->data = data;

            BG_MUTEX_LOCK(&pqd->mutex);

            *pqd->pcurrent = node;
            pqd->pcurrent = &node->next;

            BG_MUTEX_UNLOCK(&pqd->mutex);
        }
    }

    if (err_code && perr)
        bg_queue_set_error(err_code, perr);
    return err_code;
}

int bg_queue_get(struct bg_queue_data *pqd, void **pdata, struct bb_error *perr)
{
    struct bg_queue_node *node;
    int err_code;

    err_code = 0;

    if (!pqd || !pdata)
        err_code = 3;
    else
    {
        BG_MUTEX_LOCK(&pqd->mutex);

        node = pqd->first;
        if (node)
        {
            pqd->first = node->next;
            if (!pqd->first)
                pqd->pcurrent = &pqd->first;
        }

        BG_MUTEX_UNLOCK(&pqd->mutex);

        if (!node)
            *pdata = NULL;
        else
        {
            *pdata = node->data;
            free(node);
        }
    }

    if (err_code && perr)
        bg_queue_set_error(err_code, perr);
    return err_code;
}

void bg_queue_deinit(struct bg_queue_data **ppqd)
{
    struct bg_queue_node *prev;

    if (ppqd && *ppqd)
    {
        while ((*ppqd)->first)
        {
            prev = (*ppqd)->first;
            (*ppqd)->first = prev->next;
            free(prev);
        }

        free(*ppqd);
        *ppqd = NULL;
    }
}

const char * bg_queue_error_message(int err_code)
{
    static const char *BG_QUEUE_ERRS[BG_QUEUE_ERRS_N] = {   "ОК",                       //  0
                                                            "Ошибка выделения памяти",  //  1
                                                            "Неизвестная ошибка",       //  2
                                                            "Пустой указатель",         //  3
    };  

    if ((err_code < 0) || (err_code >= BG_QUEUE_ERRS_N))
        err_code = 2;

    return *(BG_QUEUE_ERRS + err_code);
}

static void bg_queue_set_error(int err_code, struct bb_error *perr)
{
    perr->err_code = err_code;
    perr->err_str = bg_queue_error_message(err_code);
}