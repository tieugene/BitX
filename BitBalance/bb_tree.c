#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <inttypes.h>
#include "bb_rates.h"
#include "bb_tree.h"
#include "bb_io.h"
#include "bb_scan_double.h"

#define BB_D_FILE_DATE_CHECK_ON

#ifndef BB_D_FILE_DATE_CHECK_ON
#warning D-file date check is off!
#endif

#define BB_TREE_ERRS_N 17                   // Number of errors.

#define BB_TIME_N 3

struct bb_tree_data {                       // Tree node data.
    char *s;                                // A string.
    int64_t satoshi;                        // Bitcoins in satoshi.
    struct bb_tree_data *left;              // Left node pointer.
    struct bb_tree_data *right;             // Right node pointer.
};

struct bb_tree_queue {                      // Tree queue data.
    struct bb_tree_data *node;              // Tree node.
    struct bb_tree_queue *next;             // Pointer to next item of queue.
};

/*  Parses string and gets transactions data.  */
static int bb_get_transaction(char *s, size_t s_len, size_t address_len, int64_t *psatoshi, int *pyear, int *pmonth, int *pdate);

/*  Get year, month, date and bitcoins from string.  */
static int bb_get_transaction_from_string(char *s, int *pyear, int *pmonth, int *pdate, double *pbitcoins);

/*  Adds node to tree.  */
static int bb_tree_add(const char *s, int64_t satoshi, size_t s_len, struct bb_tree_data **pfirst, size_t *pn);

/*  Allocates memory and adds item to queue  */
static int bb_tree_get_queue_item(struct bb_tree_queue **pq_first, struct bb_tree_data *node);

/* Sets error data. */
static void set_error(int err_code, struct bb_error *perr);

int bb_tree_get_tree(const char *file_name, struct bb_tree_data **pfirst, int *pyear, int *pmonth, int *pdate, size_t *pn, 
    struct bb_error *perr)
{
    char *data, *p, *p2, c;
    int64_t satoshi;
    size_t len, address_len;
    int m, n, year, month, date, err_code;
    _Bool date_flag;

    data = NULL;
    len = 0;
    date_flag = 0;
    err_code = 0;

    if (!file_name || !pfirst || !pyear || !pmonth || !pdate || !pn)
        err_code = 3;
    else if (!bb_read_file(file_name, &data, &len, perr))
    {
        n = 0;
        *pn = 0;
        for (p = data; *p && !err_code; )
        {
            while ((' ' == *p) || ('\n' == *p) || ('\r' == *p) || (',' == *p))
                ++p;

            if (*p)
            {
                if ('{' == *p)
                {
                    ++n;
                    ++p;
                }
                else if ('}' == *p)
                {
                    if (--n < 0)
                        err_code = 4;
                    else
                        ++p;
                }
                else
                {
                    if (!n)
                    {
                        p2 = p;
                        p = data + len;
                    }
                    else
                    {
                        m = n;
                        p2 = p++;

                        while (*p)
                        {
                            if ('{' == *p)
                                ++n;
                            else if ('}' == *p)
                            {
                                if (--n < m)
                                    break;
                            }
                            ++p;
                        }

                        if (n >= m)
                            err_code = 4;
                    }

                    if (!err_code)
                    {
                        satoshi = (int64_t) 0;

                        c = *p;
                        *p = '\0';
                        //puts(p2);
                        address_len = bb_tree_get_address_len(p2);
                        if (!err_code)
                        {
                            if (!date_flag)
                            {
                                err_code = bb_get_transaction(p2, (p - p2), address_len, &satoshi, pyear, pmonth, pdate);
                                //printf("year %d, month %d, date %d\n", *pyear, *pmonth, *pdate);
                                date_flag = 1;
                            }
                            else
                            {
                                err_code = bb_get_transaction(p2, (p - p2), address_len, &satoshi, &year, &month, &date);
#ifdef BB_D_FILE_DATE_CHECK_ON
                                if (!err_code && ((*pyear != year) || (*pmonth != month) || (*pdate != date)))
                                    err_code = 15;
#endif
                            }
                        }
                        *p = c;

                        if (!err_code)
                        {
                            //c = *(p2 + address_len);
                            *(p2 + address_len) = '\0';
                            //puts(p2);
                            err_code = bb_tree_add(p2, satoshi, address_len, pfirst, pn);
                            //*(p2 + address_len) = c;
                        }

                        if (*p)
                            ++p;
                    }
                }
            }
        }
    }

    if (data)
        free(data);

    if (err_code)
        bb_tree_free(pfirst, NULL);

    if (err_code && perr)
        set_error(err_code, perr);
    return err_code;
}

size_t bb_tree_get_address_len(const char *s)
{
    const char *p;
    size_t len;

    len = 0;
    if (s && *s)
    {
        for (p = s; *p; ++p)
            if ((',' == *p) || (' ' == *p) || ('\n' == *p) || ('\r' == *p))
                break;
        len = p - s;
    }

    return len;
}
/*
static int bb_get_transaction(char *s, double *pbitcoins, int *pyear, int *pmonth, int *pdate)
{
    char *p;
    char *pp[BB_TIME_N] = {0};
    int i, err_code;

    err_code = 0;

    if (pbitcoins || pyear || pmonth || pdate)
    {
        while ((',' == *s) || ('{' == *s) || (' ' == *s) || ('\n' == *s) || ('\r' == *s))
            ++s;

        for (p = s, i = 0; *p && (i < BB_TIME_N); ++p)
            if (',' == *p)
            {
                *p++ = '\0';
                while ((' ' == *p) && ('\n' == *p) && ('\r' == *p))
                    ++p;
                if (!*p)
                    break;
                else
                    *(pp + i++) = p;
            }

        if (i < BB_TIME_N)
            err_code = 5;
        else if (pbitcoins && !bb_scan_double(s, pbitcoins))
            err_code = 6;
        else if (pyear)
        {
            if (sscanf(*pp, "%d", pyear) != 1)
                err_code = 7;
            else if (*pyear < 0)
                err_code = 8;
        }
        if (!err_code && pmonth)
        {
            if (sscanf(*(pp + 1), "%d", pmonth) != 1)
                err_code = 9;
            else if ((*pmonth <= 0) || (*pmonth > BB_MONTHS_PER_YEAR))
                err_code = 10;
        }
        if (!err_code && pdate)
        {
            if (sscanf(*(pp + 2), "%d", pdate) != 1)
                err_code = 11;
            else if ((*pdate <= 0) || (*pdate > BB_DAYS_PER_MONTH))
                err_code = 12;
        }
    }

    return err_code;
}
*/
static int bb_get_transaction(char *s, size_t s_len, size_t address_len, int64_t *psatoshi, int *pyear, int *pmonth, int *pdate)
{
    char *ns, *p, *p2/*, c*/, c2;
    double bitcoins;
    int year, month, date, m, n, err_code;
    _Bool date_flag;

    *psatoshi = (int64_t) 0;
    err_code = 0;
    date_flag = 0;

    //c = *(s + s_len);
    //*(s + s_len) = '\0';

    ns = s + address_len;
    while ((',' == *ns) || (' ' == *ns) || ('\n' == *ns) || ('\r' == *ns))
        ++ns;

    if (!*ns)
        err_code = 5;
    else
    {
        n = 0;
        for (p = ns; *p && !err_code; )
        {
            while ((' ' == *p) || ('\n' == *p) || ('\r' == *p) || (',' == *p))
                ++p;

            if (*p)
            {
                if ('{' == *p)
                {
                    ++n;
                    ++p;
                }
                else if ('}' == *p)
                {
                    if (--n < 0)
                        err_code = 6;
                    else
                        ++p;
                }
                else
                {
                    if (!n)
                    {
                        p2 = p;
                        p = s + s_len;
                    }
                    else
                    {
                        m = n;
                        p2 = p++;

                        while (*p)
                        {
                            if ('{' == *p)
                                ++n;
                            else if ('}' == *p)
                            {
                                if (--n < m)
                                    break;
                            }
                            ++p;
                        }

                        if (n >= m)
                            err_code = 6;
                    }

                    if (!err_code)
                    {
                        c2 = *p;
                        *p = '\0';
                        //puts(p2);
                        year = month = date = 0;
                        bitcoins = (double) 0;
                        if (!err_code)
                        {
                            if (!date_flag)
                            {
                                err_code = bb_get_transaction_from_string(p2, pyear, pmonth, pdate, &bitcoins);
                                date_flag = 1;
                            }
                            else
                            {
                                err_code = bb_get_transaction_from_string(p2, &year, &month, &date, &bitcoins);
#ifdef BB_D_FILE_DATE_CHECK_ON
                                if (!err_code && ((year != *pyear) || (month != *pmonth) || (date != *pdate)))
                                    err_code = 16;
#endif
                            }
                            *psatoshi += llround(bitcoins * (double) BB_SATOSHI_PER_BITCOIN);
                        }

                        *p = c2;

                        if (*p)
                            ++p;
                    }
                }
            }
        }
    }

    //*(s + s_len) = c;

    return err_code;
}

static int bb_get_transaction_from_string(char *s, int *pyear, int *pmonth, int *pdate, double *pbitcoins)
{
    char *p;
    char *pp[BB_TIME_N] = {0};
    int i, err_code;

    err_code = 0;
    for (p = s, i = 0; *p && (i < BB_TIME_N); ++p)
        if (',' == *p)
        {
            *p++ = '\0';
            while ((' ' == *p) && ('\n' == *p) && ('\r' == *p))
                ++p;
            if (!*p)
                break;
            else
                *(pp + i++) = p;
        }

    if (i < BB_TIME_N)
        err_code = 7;
    else if (!bb_scan_double(s, pbitcoins))
        err_code = 8;
    else if (sscanf(*pp, "%d", pyear) != 1)
        err_code = 9;
    else if (*pyear < 0)
        err_code = 10;
    else if (sscanf(*(pp + 1), "%d", pmonth) != 1)
        err_code = 11;
    else if ((*pmonth <= 0) || (*pmonth > BB_MONTHS_PER_YEAR))
        err_code = 12;
    else if (sscanf(*(pp + 2), "%d", pdate) != 1)
        err_code = 13;
    else if ((*pdate <= 0) || (*pdate > BB_DAYS_PER_MONTH))
        err_code = 14;

    return err_code;
}

static int bb_tree_add(const char *s, int64_t satoshi, size_t s_len, struct bb_tree_data **pfirst, size_t *pn)
{
    struct bb_tree_data **pcurrent;
    int n, err_code;

    err_code = 0;

    if (!s || !s_len || !pfirst)
        err_code = 3;
    else
    {
        for (pcurrent = pfirst; *pcurrent; )
        {
            n = strcmp(s, (*pcurrent)->s);
            if (!n)
                break;
            else
                pcurrent = (n < 0) ? &(*pcurrent)->left : &(*pcurrent)->right;
        }

        if (!*pcurrent)
        {
            *pcurrent = (struct bb_tree_data *) malloc(sizeof(struct bb_tree_data) + s_len + 1);
            if (!*pcurrent)
                err_code = 1;
            else
            {
                (*pcurrent)->s = (char *) (*pcurrent + 1);
                strncpy((*pcurrent)->s, s, s_len);
                *((*pcurrent)->s + s_len) = '\0';

                (*pcurrent)->satoshi = (int64_t) 0;

                (*pcurrent)->left = (*pcurrent)->right = NULL;

                (*pn)++;
            }
        }

        if (!err_code)
            (*pcurrent)->satoshi += satoshi;
    }

    return err_code;
}

_Bool bb_tree_contains(const char *s, int64_t *psatoshi, const struct bb_tree_data *first)
{
    const struct bb_tree_data *node;
    int n;
    _Bool res;

    res = 0;
    for (node = first; node; )
    {
        n = strcmp(s, node->s);
        if (!n)
        {
            res = 1;
            *psatoshi = node->satoshi;
            break;
        }
        else
            node = (n < 0) ? node->left : node->right;
    }

    return res;
}

int bb_tree_free(struct bb_tree_data **pfirst, struct bb_error *perr)
{
    struct bb_tree_queue *q_first, *q_item;
    int err_code;

    q_first = NULL;
    err_code = 0;

    if (!pfirst)
        err_code = 3;
    else if (*pfirst)
    {
        err_code = bb_tree_get_queue_item(&q_first, *pfirst);
        if (!err_code)
        {
            *pfirst = NULL;
            while (q_first && !err_code)
            {
                q_item = q_first;
                q_first = q_first->next;
                if (q_item->node->left)
                    err_code = bb_tree_get_queue_item(&q_first, q_item->node->left);
                if (q_item->node->right && !err_code)
                    err_code = bb_tree_get_queue_item(&q_first, q_item->node->right);
                //puts(q_item->node->s);
                //printf("%s\t%0.3lf\n", q_item->node->s, (double) q_item->node->satoshi / (double) BB_SATOSHI_PER_BITCOIN);
                free(q_item->node);
                free(q_item);
            }
        }
    }

    if (err_code && perr)
        set_error(err_code, perr);
    return err_code;
}

static int bb_tree_get_queue_item(struct bb_tree_queue **pq_first, struct bb_tree_data *node)
{
    struct bb_tree_queue *q_item;
    int err_code;

    err_code = 0;

    q_item = (struct bb_tree_queue *) malloc(sizeof(struct bb_tree_queue));
    if (!q_item)
        err_code = 1;
    else
    {
        q_item->node = node;
        q_item->next = *pq_first;
        *pq_first = q_item;
    }

    return err_code;
}

const char * bb_tree_error_message(int err_code)
{
    static const char *BB_TREE_ERRS[BB_TREE_ERRS_N] = { "ОК",                                                       //  0
                                                        "Ошибка выделения памяти",                                  //  1
                                                        "Неизвестная ошибка",                                       //  2
                                                        "Пустой указатель",                                         //  3
                                                        "Ошибка в данных D-файла",                                  //  4
                                                        //"В строке транзакции меньше элементов, чем необходимо",     //  5
                                                        //"Не удалось получить сумму из строки транзакции",           //  6
                                                        "Строка не содержит транзакций",                            //  5
                                                        "Ошибка в строке транзакций",                               //  6
                                                        "В строке транзакции меньше элементов, чем необходимо",     //  7
                                                        "Не удалось получить сумму из строки транзакции",           //  8
                                                        "Не удалось получить год из строки транзакции",             //  9
                                                        "Некорректное значение года в строке транзакции",           // 10
                                                        "Не удалось получить месяц из строки транзакции",           // 11
                                                        "Некорректное значение месяца в строке транзакции",         // 12
                                                        "Не удалось получить число (дату) из строки транзакции",    // 13
                                                        "Некорректное значение числа (даты) в строке транзакции",   // 14
                                                        "Даты в D-файле отличаются",                                // 15
                                                        "Даты в строке транзакций отличаются",                      // 16
    };

    if ((err_code < 0) || (err_code >= BB_TREE_ERRS_N))
        err_code = 2;

    return *(BB_TREE_ERRS + err_code);
}

static void set_error(int err_code, struct bb_error *perr)
{
    perr->err_code = err_code;
    perr->err_str = bb_tree_error_message(err_code);
}