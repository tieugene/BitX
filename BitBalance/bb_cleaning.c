#ifdef BB_WIN
#include <glib.h>
#include <glib/gstdio.h>
#else
#include <stdio.h>
#include <stdlib.h>
#endif

#include <inttypes.h>
#include <limits.h>
#include <string.h>
#include "bb_rates.h"
#include "bb_io.h"
#include "bb_tree.h"
#include "bb_scan_double.h"
#include "bb_cleaning.h"

#define BB_TRTM_N 5

#define BB_CLEANING_ERRS_N 22              // Number of errors.

struct bb_cleaning_transaction {            // Transactions list node data.
    double value;                           // Value.
    int year;                               // Year.
    int8_t month;                           // Month.
    int8_t date;                            // Date.
    int8_t hours;                           // Hours.
    int8_t mins;                            // Mins.
    struct bb_cleaning_transaction *next;   // Next tree node pointer.
};

struct bb_cleaning_tree {                   // Clening tree node data.
    char *address;                          // Address.
    struct bb_cleaning_transaction *trs;    // Transactions.
    struct bb_cleaning_transaction **pt;    // Current pointer.
    struct bb_cleaning_tree *left;          // Left child.
    struct bb_cleaning_tree *right;         // Right child.
};

struct bb_cleaning_queue {                  // Cleaning queue data.
    struct bb_cleaning_tree *node;          // Tree node pointer.
    struct bb_cleaning_queue *next;         // Next list item pointer.
};

struct bb_cleaning_data {                   // Cleaning data.
    struct bb_cleaning_tree *first;         // Tree root.
    struct bb_cleaning_queue *queue;        // Queue for file write.
    size_t nodes_n;                         // Number of nodes.
    int min_date_year;                      // Min date year.
    int8_t min_date_month;                  // Min date month.
    int8_t min_date_date;                   // Min date date.
};

/*  Allocates memory and adds data to structure.  */
static int bb_cleaning_data_add(char *s, size_t s_len, size_t address_len, struct bb_cleaning_data *pcd, struct bb_cleaning_queue ***ppcurrent);

/*  Finds node with the address in tree, or creates new one, if it's not exist.  */
static int bb_cleaning_data_get_node(const char *address, size_t address_len, struct bb_cleaning_tree **pfirst, struct bb_cleaning_tree **pnode, 
    struct bb_cleaning_queue ***ppcurrent, size_t *pn);

/*  Parses string and gets transactions data.  */
static int bb_get_transaction(char *s, size_t s_len, size_t address_len, struct bb_cleaning_transaction ***ppt, int *pyear, int8_t *pmonth, int8_t *pdate);

/*  Get year, month, date, hours, mins and bitcoins from string. Allocates memory and fill it.  */
static int bb_get_transaction_from_string(char *s, struct bb_cleaning_transaction ***ppt, int *pyear, int8_t *pmonth, int8_t *pdate);

/*  Frees cleaning queue data memory.  */
static void bb_cleaning_queue_free(struct bb_cleaning_queue **pqueue);

/*  Frees cleaning tree data memory.  */
static int bb_cleaning_tree_free(struct bb_cleaning_tree **pfirst);

/*  Frees transactions list memory.  */
inline static void bb_cleaning_transactions_free(struct bb_cleaning_transaction **pptrs);

/*  Allocates memory and fill it.  */
static int bb_cleaning_queue_get_item(struct bb_cleaning_tree *node, struct bb_cleaning_queue ***ppcurrent);

/*  Sets error data.  */
static void set_error(int err_code, struct bb_error *perr);

int bb_cleaning_init(const char *file_name, struct bb_cleaning_data **ppcd, size_t *pn, struct bb_error *perr)
{
    struct bb_cleaning_queue **pcurrent;
    char *data, *p, *p2, c;
    size_t len, address_len;
    int m, n, err_code;

    data = NULL;
    len = 0;
    err_code = 0;

    if (!file_name || !ppcd || !pn)
        err_code = 3;
    else
    {
        *ppcd = (struct bb_cleaning_data *) calloc(1, sizeof(struct bb_cleaning_data));
        if (!*ppcd)
            err_code = 1;
        else if (!bb_read_file(file_name, &data, &len, perr))
        {
            (*ppcd)->min_date_year = INT_MAX;
            (*ppcd)->min_date_month = 255;
            (*ppcd)->min_date_date = 255;

            pcurrent = &(*ppcd)->queue;

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
                            c = *p;
                            *p = '\0';
                            //puts(p2);
                            address_len = bb_tree_get_address_len(p2);
                            err_code = bb_cleaning_data_add(p2, (p - p2), address_len, *ppcd, &pcurrent);
                            *p = c;

                            if (*p)
                                ++p;
                        }
                    }
                }
            }
        }
    }

    if (data)
        free(data);

    if (err_code)
        bb_cleaning_data_free(ppcd, perr);
    else
        *pn = (*ppcd)->nodes_n;

    if (err_code && perr)
        set_error(err_code, perr);
    return err_code;
}

static int bb_cleaning_data_add(char *s, size_t s_len, size_t address_len, struct bb_cleaning_data *pcd, struct bb_cleaning_queue ***ppcurrent)
{
    struct bb_cleaning_tree *node;
    char c;
    int err_code;

    node = NULL;
    err_code = 0;

    c = *(s + address_len);
    *(s + address_len) = '\0';
    err_code = bb_cleaning_data_get_node(s, address_len, &pcd->first, &node, ppcurrent, &pcd->nodes_n);
    *(s + address_len) = c;

    if (!err_code)
        bb_get_transaction(s, s_len, address_len, &node->pt, &pcd->min_date_year, &pcd->min_date_month, &pcd->min_date_date);

    return err_code;
}

static int bb_cleaning_data_get_node(const char *address, size_t address_len, struct bb_cleaning_tree **pfirst, struct bb_cleaning_tree **pnode, 
    struct bb_cleaning_queue ***ppcurrent, size_t *pn)
{
    struct bb_cleaning_tree **pcurrent;
    int n, err_code;

    err_code = 0;

    pcurrent = pfirst;
    while (*pcurrent)
    {
        n = strcmp(address, (*pcurrent)->address);
        if (!n)
            break;
        else if (n < 0)
            pcurrent = &(*pcurrent)->left;
        else
            pcurrent = &(*pcurrent)->right;
    }

    if (!*pcurrent)
    {
        *pcurrent = (struct bb_cleaning_tree *) malloc(sizeof(struct bb_cleaning_tree) + address_len + 1);
        if (!*pcurrent)
            err_code = 1;
        else
        {
            err_code = bb_cleaning_queue_get_item(*pcurrent, ppcurrent);
            if (!err_code)
            {
                // memset(*pcurrent, 0, sizeof(struct bb_cleaning_tree) + address_len + 1);
                (*pcurrent)->address = (char *) (*pcurrent + 1);
                strncpy((*pcurrent)->address, address, address_len + 1);
                *((*pcurrent)->address + address_len) = '\0';

                (*pcurrent)->trs = NULL;
                (*pcurrent)->pt = &(*pcurrent)->trs;
                (*pcurrent)->left = (*pcurrent)->right = NULL;

                *pnode = *pcurrent;
                (*pn)++;
            }
        }
    }

    return err_code;
}

static int bb_get_transaction(char *s, size_t s_len, size_t address_len, struct bb_cleaning_transaction ***ppt, int *pyear, int8_t *pmonth, int8_t *pdate)
{
    char *ns, *p, *p2, c;
    int m, n, err_code;

    err_code = 0;

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
                        c = *p;
                        *p = '\0';
                        //puts(p2);
                        err_code = bb_get_transaction_from_string(p2, ppt, pyear, pmonth, pdate);
                        *p = c;

                        if (*p)
                            ++p;
                    }
                }
            }
        }
    }

    return err_code;
}

static int bb_get_transaction_from_string(char *s, struct bb_cleaning_transaction ***ppt, int *pyear, int8_t *pmonth, int8_t *pdate)
{
    char *p;
    char *pp[BB_TRTM_N] = {0};
    double bitcoins;
    int year, month, date, hours, mins, i, err_code;

    err_code = 0;
    for (p = s, i = 0; *p && (i < BB_TRTM_N); ++p)
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

    year = month = date = hours = mins = 0;
    bitcoins = (double) 0;
    if (i < BB_TRTM_N)
        err_code = 7;
    else if (!bb_scan_double(s, &bitcoins))
        err_code = 8;
    else if (sscanf(*pp, "%d", &year) != 1)
        err_code = 9;
    else if (year < 0)
        err_code = 10;
    else if (sscanf(*(pp + 1), "%d", &month) != 1)
        err_code = 11;
    else if ((month <= 0) || (month > BB_MONTHS_PER_YEAR))
        err_code = 12;
    else if (sscanf(*(pp + 2), "%d", &date) != 1)
        err_code = 13;
    else if ((date <= 0) || (date > BB_DAYS_PER_MONTH))
        err_code = 14;
    else if (sscanf(*(pp + 3), "%d", &hours) != 1)
        err_code = 15;
    else if ((hours < 0) || (hours > 23))
        err_code = 16;
    else if (sscanf(*(pp + 4), "%d", &mins) != 1)
        err_code = 17;
    else if ((mins < 0) || (mins > 59))
        err_code = 18;
    else
    {
        **ppt = (struct bb_cleaning_transaction *) calloc(1, sizeof(struct bb_cleaning_transaction));
        if (!**ppt)
            err_code = 1;
        else
        {
            (**ppt)->value = bitcoins;
            (**ppt)->year = year;
            (**ppt)->month = (int8_t) month;
            (**ppt)->date = (int8_t) date;
            (**ppt)->hours = (int8_t) hours;
            (**ppt)->mins = (int8_t) mins;

            (**ppt)->next = NULL;
            *ppt = &(**ppt)->next;

            if (bb_cleaning_compare(year, month, date, *pyear, *pmonth, *pdate) < 0)
            {
                *pyear = year;
                *pmonth = month;
                *pdate = date;
            }
        }
    }

    return err_code;
}

int bb_cleaning_compare(int y1, int m1, int d1, int y2, int m2, int d2)
{
    //int ret;
/*
    if (y1 < y2)
        ret = -1;
    else if (y1 > y2)
        ret = 1;
    else if (m1 < m2)   // (y1 == y2)
        ret = -1;
    else if (m1 > m2)
        ret = 1;
    else if (d1 < d2)   // (y1 == y2) && (m1 == m2)
        ret = -1;
    else if (d1 > d2)
        ret = 1;
    else
        ret = 0;        // (y1 == y2) && (m1 == m2) && (d1 == d2)
*/
/*
    if ((y1 == y2) && (m1 == m2) && (d1 == d2))
        ret = 0;
    else if ((y1 < y2) || ((y1 == y2) && (m1 < m2)) || ((y1 == y2) && (m1 == m2) && (d1 < d2)))
        ret = -1;
    else
        ret = 1;
*/
    //return ret;

    return ((y1 < y2) ? -1 : (y1 > y2) ? 1 : 
            (m1 < m2) ? -1 : (m1 > m2) ? 1 :        // (y1 == y2)
            (d1 < d2) ? -1 : (d1 > d2) ? 1 : 0);    // (y1 == y2) && (m1 == m2)
}

void bb_cleaning_get_min_date(const struct bb_cleaning_data *pcd, int *pmin_date_year, int *pmin_date_month, int *pmin_date_date)
{
    if (pcd && pmin_date_year && pmin_date_month && pmin_date_date)
    {
        *pmin_date_year = pcd->min_date_year;
        *pmin_date_month = (int) pcd->min_date_month & 0xff;
        *pmin_date_date = (int) pcd->min_date_date & 0xff;
    }
}

int bb_cleaning_write_data(const char *s_path, const struct bb_cleaning_data *pcd, int year, int month, int date, struct bb_error *perr)
{
    FILE *f;
    struct bb_cleaning_queue *pcq;
    const struct bb_cleaning_tree *node;
    struct bb_cleaning_transaction *trs;
    char *p;
    int err_code;
    _Bool parenthesis, a, b;
    char tmp[128] = {0};

    err_code = 0;
    parenthesis = b = 0;

#ifdef BB_WIN
    f = g_fopen(s_path, "wb");
#else
    f = fopen(s_path, "wb");
#endif

    if (!f)
        err_code = 19;
    else
    {
        for (pcq = pcd->queue; pcq && !err_code; pcq = pcq->next)
        {
            a = 0;
            node = pcq->node;
            for (trs = node->trs; trs && !err_code; trs = trs->next)
                if (bb_cleaning_compare(trs->year, trs->month, trs->date, year, month, date) <= 0)
                {
                    if (!parenthesis)
                    {
                        if (fprintf(f, "{") < 0)
                            err_code = 20;
                        else
                            parenthesis = 1;
                    }

                    if (!a && !err_code)
                    {
                        if (!b)
                        {
                            if (fprintf(f, "{%s", node->address) < 0)
                                err_code = 20;
                            b = 1;
                        }
                        else
                        {
                            if (fprintf(f, ",{%s", node->address) < 0)
                                err_code = 20;
                        }
                        a = 1;
                    }

                    if (!err_code && fprintf(f, ",{") < 0)
                        err_code = 20;
                    else if (snprintf(tmp, 128, "%.3lf", trs->value) < 0)
                        err_code = 21;
                    else
                    {
                        for (p = tmp; *p; ++p)
                            if (',' == *p)
                            {
                                *p = '.';
                                break;
                            }
                        if (!err_code && fprintf(f, "%s,%02d,%02d,%02d,%02d,%02d", 
                                                    tmp, 
                                                    trs->year, 
                                                    (int) trs->month & 0xff, 
                                                    (int) trs->date & 0xff, 
                                                    (int) trs->hours & 0xff, 
                                                    (int) trs->mins & 0xff) < 0)
                            err_code = 20;
                        else if (!err_code && fprintf(f, "}") < 0)
                            err_code = 20;
                    }
                }

            if (!err_code && a && (fprintf(f, "}") < 0))
                err_code = 20;
        }

        if (!err_code && parenthesis && (fprintf(f, "}\n") < 0))
            err_code = 20;

        fclose(f);
    }

    if (err_code && perr)
        set_error(err_code, perr);
    return err_code;
}

int bb_cleaning_data_free(struct bb_cleaning_data **ppcd, struct bb_error *perr)
{
    int err_code;

    err_code = 0;

    if (ppcd && *ppcd)
    {
        err_code = bb_cleaning_tree_free(&(*ppcd)->first);

        bb_cleaning_queue_free(&(*ppcd)->queue);

        free(*ppcd);
        *ppcd = NULL;
    }

    if (err_code && perr)
        set_error(err_code, perr);
    return err_code;
}

static int bb_cleaning_tree_free(struct bb_cleaning_tree **pfirst)
{
    struct bb_cleaning_queue *q_first, *q_node, **q_pcurrent;
    int err_code;

    q_first = NULL;
    err_code = 0;

    if (*pfirst)
    {
        q_pcurrent = &q_first;
        err_code = bb_cleaning_queue_get_item(*pfirst, &q_pcurrent);
        while (q_first && !err_code)
        {
            if (q_first->node->left)
                err_code = bb_cleaning_queue_get_item(q_first->node->left, &q_pcurrent);
            if (!err_code && q_first->node->right)
                err_code = bb_cleaning_queue_get_item(q_first->node->right, &q_pcurrent);

            bb_cleaning_transactions_free(&q_first->node->trs);

            free(q_first->node);

            q_node = q_first;
            q_first = q_node->next;
            free(q_node);
        }
    }

    if (err_code)
        bb_cleaning_queue_free(&q_first);

    return err_code;
}

inline static void bb_cleaning_transactions_free(struct bb_cleaning_transaction **pptrs)
{
    struct bb_cleaning_transaction *prev;

    while (*pptrs)
    {
        prev = *pptrs;
        *pptrs = prev->next;
        free(prev);
    }
}

static void bb_cleaning_queue_free(struct bb_cleaning_queue **pqueue)
{
    struct bb_cleaning_queue *prev;

    while (*pqueue)
    {
        prev = *pqueue;
        *pqueue = prev->next;
        free(prev);
    }
}

static int bb_cleaning_queue_get_item(struct bb_cleaning_tree *node, struct bb_cleaning_queue ***ppcurrent)
{
    int err_code;

    err_code = 0;

    **ppcurrent = (struct bb_cleaning_queue *) calloc(1, sizeof(struct bb_cleaning_queue));
    if (!**ppcurrent)
        err_code = 1;
    else
    {
        (**ppcurrent)->node = node;
        *ppcurrent = &(**ppcurrent)->next;
    }

    return err_code;
}

const char * bb_cleaning_error_message(int err_code)
{
    static const char *BB_CLEANING_ERRS[BB_CLEANING_ERRS_N] = { "ОК",                                                           //  0
                                                                "Ошибка выделения памяти",                                      //  1
                                                                "Неизвестная ошибка",                                           //  2
                                                                "Пустой указатель",                                             //  3
                                                                "Ошибка в данных D-файла",                                      //  4
                                                                "Строка не содержит транзакций",                                //  5
                                                                "Ошибка в строке транзакций",                                   //  6
                                                                "В строке транзакции меньше элементов, чем необходимо",         //  7
                                                                "Не удалось получить сумму из строки транзакции",               //  8
                                                                "Не удалось получить год из строки транзакции",                 //  9
                                                                "Некорректное значение года в строке транзакции",               // 10
                                                                "Не удалось получить месяц из строки транзакции",               // 11
                                                                "Некорректное значение месяца в строке транзакции",             // 12
                                                                "Не удалось получить число (дату) из строки транзакции",        // 13
                                                                "Некорректное значение числа (даты) в строке транзакции",       // 14
                                                                "Не удалось получить время (часы) из строки транзакции",        // 15
                                                                "Некорректное значение времени (часов) в строке транзакции",    // 16
                                                                "Не удалось получить время (минуты) из строки транзакции",      // 17
                                                                "Некорректное значение времени (минут) в строке транзакции",    // 18
                                                                "Ошибка при открытии файла для записи",                         // 19
                                                                "Ошибка записи в файл",                                         // 20
                                                                "Ошибка snprintf",                                              // 21
    };

    if ((err_code < 0) || (err_code >= BB_CLEANING_ERRS_N))
        err_code = 2;

    return *(BB_CLEANING_ERRS + err_code);
}

static void set_error(int err_code, struct bb_error *perr)
{
    perr->err_code = err_code;
    perr->err_str = bb_cleaning_error_message(err_code);
}