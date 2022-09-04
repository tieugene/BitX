#ifdef BB_WIN
#include <glib.h>
#include <glib/gstdio.h>
#else
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#endif

#include "bb_scan_double.h"
#include "bb_csv_iter.h"

#define BB_SECS_PER_DAY 86400

#define BB_CSV_ITER_ERRS_N 21               // Number of errors.
#define BB_CSV_ITER_DEFAULT_BUF_SIZE 512    // Default buffer size.
#define BB_CSV_ITER_SYMB ';'                // Separator.

#define BB_CSV_ITER_COLUMNS_DT 0            // Date.
#define BB_CSV_ITER_COLUMNS_WB 1            // Winner Buy.
#define BB_CSV_ITER_COLUMNS_WS 2            // Winner Sell.
#define BB_CSV_ITER_COLUMNS_LB 3            // Loser Buy.
#define BB_CSV_ITER_COLUMNS_LS 4            // Loser Sell.
#define BB_CSV_ITER_COLUMNS_RT 5            // Rate.
#define BB_CSV_ITER_COLUMNS_N 6             // Number of columns.

/*  Gets data and set it.  */
static int bb_csv_get_columns_data_from_strings(struct bb_csv_node *pnode, char *pp[BB_CSV_ITER_COLUMNS_N]);

/*  Adds node to list.  */
static int bb_csv_add_node(struct bb_csv_node **pfirst, struct bb_csv_node **pcurrent, const struct bb_csv_node *pnode);

/*  Allocates memory and copies node.  */
static int bb_csv_copy_node(struct bb_csv_node **pcurrent, const struct bb_csv_node *src);

/*  Writes data to file.  */
static int bb_csv_write_node(const char *file_name, const struct bb_csv_node *pcn, _Bool to_write_headers);

/*  Sets error data.  */
static void bb_csv_set_error(int err_code, struct bb_error *perr);

int bb_csv_get_list(const char *file_name, struct bb_csv_node **pfirst, const struct bb_csv_node *pcn, struct bb_error *perr)
{
    struct bb_csv_node node, *current, *pnode;
    FILE *f;
    char *buf, *p, *p2, *pp[BB_CSV_ITER_COLUMNS_N];
    size_t buf_size;
    long int offset, end, n;
    int i, ch, err_code;

    //tzset();
    err_code = 0;

    if (!file_name || !pfirst || !pcn)
        err_code = 3;
    else
    {
        *pfirst = current = NULL;

#ifdef BB_WIN
        f = g_fopen(file_name, "rb");
#else
        f = fopen(file_name, "rb");
#endif

        if (f)
        {
            offset = 0L;
            buf_size = BB_CSV_ITER_DEFAULT_BUF_SIZE;
            buf = (char *) calloc(buf_size + 1, 1);         // 1 байт на терминальный ноль.
            if (!buf)
                err_code = 1;
            else
            {
                ch = fgetc(f);                              // Если есть сигнатура UTF-8, "перематываем" её.
                if (239 == ch)
                {
                    offset++;
                    ch = fgetc(f);
                    if (187 == ch)
                    {
                        offset++;
                        ch = fgetc(f);
                        if (191 == ch)
                        {
                            offset++;
                            ch = fgetc(f);
                        }
                    }
                }

                for ( ; (ch != EOF) && ('\r' != (char) ch) && ('\n' != (char) ch); ch = fgetc(f), ++offset)     // Skip first line.
                    continue;

                while (ch != EOF)
                {
                    for ( ; (ch != EOF) && (('\r' == (char) ch) || ('\n' == (char) ch)); ch = fgetc(f), ++offset)
                        continue;
                    for (end = offset; (ch != EOF) && ('\r' != (char) ch) && ('\n' != (char) ch); ch = fgetc(f), ++end)
                        continue;

                    n = end - offset;
                    if (n)
                    {
                        if (n > buf_size)
                        {
                            free(buf);
                            buf_size = n;
                            buf = (char *) calloc(buf_size + 1, 1);
                            if (!buf)
                            {
                                err_code = 1;
                                break;
                            }
                        }

                        if (fseek(f, offset, SEEK_SET))
                        {
                            err_code = 5;
                            break;
                        }

                        if (fread(buf, n, 1, f) != 1)
                        {
                            err_code = 6;
                            break;
                        }
                        *(buf + n) = '\0';

                        memset(pp, 0, sizeof(char *) * BB_CSV_ITER_COLUMNS_N);
                        for (p = buf, i = 0; (i < BB_CSV_ITER_COLUMNS_N) && (*p != '\0'); )
                        {
                            for (p2 = p; (*p2 != '\0') && (*p2 != BB_CSV_ITER_SYMB); ++p2)
                                continue;
                            if (*p2)
                                *p2++ = '\0';

                            *(pp + i++) = p;
                            p = p2;
                        }

                        err_code = bb_csv_get_columns_data_from_strings(&node, pp);
                        if (err_code)
                            break;

                        err_code = bb_csv_add_node(pfirst, &current, &node);
                        if (err_code)
                            break;

                        ch = fgetc(f);
                    }

                    offset = end;
                }

                if (buf)
                    free(buf);
            }

            fclose(f);
        }

        if (!err_code)
        {
            memcpy(&node, pcn, sizeof(struct bb_csv_node));
            node.date = (node.date / (time_t) BB_SECS_PER_DAY) * (time_t) BB_SECS_PER_DAY;

            if (!current)
                err_code = bb_csv_write_node(file_name, &node, 1);
            else if (current->date < node.date)
                err_code = bb_csv_write_node(file_name, &node, 0);
            else if (current->date > node.date)
            {
                for (pnode = *pfirst; pnode; pnode = pnode->next)
                    if (pnode->date >= node.date)
                        break;

                if (pnode && (pnode->date > node.date))
                    err_code = bb_csv_write_node(file_name, &node, 0);
            }

            if (!err_code)
                err_code = bb_csv_add_node(pfirst, &current, &node);
        }
    }

    if (err_code)
        bb_csv_free_list(pfirst);

    if (err_code && perr)
        bb_csv_set_error(err_code, perr);
    return err_code;
}

void bb_csv_free_list(struct bb_csv_node **pfirst)
{
    struct bb_csv_node *prev;

    if (pfirst)
        while (*pfirst)
        {
            prev = *pfirst;
            *pfirst = prev->next;
/*
            struct tm *atm = gmtime(&prev->date);
            printf("%02d.%02d.%04d;%.1lf;%.1lf;%.1lf;%.1lf;%.3lf\n", 
                atm->tm_mday, atm->tm_mon + 1, atm->tm_year + 1900, 
                prev->win_buy, prev->win_sel, prev->los_buy, prev->los_sel, prev->rate);
*/
            free(prev);
        }
}

const char * bb_csv_error_message(int err_code)
{
    const char *BB_CSV_ITER_ERRS[BB_CSV_ITER_ERRS_N] = {"OK",                                                   //  0
                                                        "Ошибка выделения памяти",                              //  1
                                                        "Неизвестная ошибка",                                   //  2
                                                        "Пустой указатель",                                     //  3
                                                        "Не удалось открыть файл на чтение",                    //  4
                                                        "Ошибка в данных файла",                                //  5
                                                        "Ошибка чтения файла",                                  //  6
                                                        "В строке отсутствует дата",                            //  7
                                                        "Не удалось получить даты из строки",                   //  8
                                                        "В строке отсутствует значение winner buy",             //  9
                                                        "Не удалось получить значения winner buy из строки",    // 10
                                                        "В строке отсутствует значение winner sell",            // 11
                                                        "Не удалось получить значения winner sell из строки",   // 12
                                                        "В строке отсутствует значение lose buy",               // 13
                                                        "Не удалось получить значения lose buy из строки",      // 14
                                                        "В строке отсутствует значение lose sell",              // 15
                                                        "Не удалось получить значения lose sell из строки",     // 16
                                                        "В строке отсутствует значение rate",                   // 17
                                                        "Не удалось получить значения rate из строки",          // 18
                                                        "Не удалось открыть файл на запись",                    // 19
                                                        "Ошибка записи файла",                                  // 20
    };

    return *(BB_CSV_ITER_ERRS + (((err_code < 0) || (err_code >= BB_CSV_ITER_ERRS_N)) ? 2 : err_code));
}

static int bb_csv_get_columns_data_from_strings(struct bb_csv_node *pnode, char *pp[BB_CSV_ITER_COLUMNS_N])
{
    struct tm ttm = {0};
    int err_code;

    err_code = 0;

    if (!*(pp + BB_CSV_ITER_COLUMNS_DT))
        err_code = 7;
    else if (sscanf(*(pp + BB_CSV_ITER_COLUMNS_DT), "%02d.%02d.%04d", &ttm.tm_mday, &ttm.tm_mon, &ttm.tm_year) != 3)
        err_code = 8;
    else
    {
        ttm.tm_year -= 1900;
        ttm.tm_mon -= 1;

        pnode->date = mktime(&ttm) - timezone;

        if (!*(pp + BB_CSV_ITER_COLUMNS_WB))
            err_code = 9;
        else if (!bb_scan_double(*(pp + BB_CSV_ITER_COLUMNS_WB), &pnode->win_buy))
            err_code = 10;
        else if (!*(pp + BB_CSV_ITER_COLUMNS_WS))
            err_code = 11;
        else if (!bb_scan_double(*(pp + BB_CSV_ITER_COLUMNS_WS), &pnode->win_sel))
            err_code = 12;
        else if (!*(pp + BB_CSV_ITER_COLUMNS_LB))
            err_code = 13;
        else if (!bb_scan_double(*(pp + BB_CSV_ITER_COLUMNS_LB), &pnode->los_buy))
            err_code = 14;
        else if (!*(pp + BB_CSV_ITER_COLUMNS_LS))
            err_code = 15;
        else if (!bb_scan_double(*(pp + BB_CSV_ITER_COLUMNS_LS), &pnode->los_sel))
            err_code = 16;
        else if (!*(pp + BB_CSV_ITER_COLUMNS_RT))
            err_code = 17;
        else if (!bb_scan_double(*(pp + BB_CSV_ITER_COLUMNS_RT), &pnode->rate))
            err_code = 18;
    }

    return err_code;
}

static int bb_csv_add_node(struct bb_csv_node **pfirst, struct bb_csv_node **pcurrent, const struct bb_csv_node *pnode)
{
    struct bb_csv_node **ppcn;
    int err_code;

    err_code = 0;

    if (!*pfirst)
    {
        err_code = bb_csv_copy_node(pfirst, pnode);
        if (!err_code)
            *pcurrent = *pfirst;
    }
    else if ((*pcurrent)->date < pnode->date)
    {
        err_code = bb_csv_copy_node(&(*pcurrent)->next, pnode);
        if (!err_code)
            *pcurrent = (*pcurrent)->next;
    }
    else
    {
        for (ppcn = pfirst; *ppcn; ppcn = &(*ppcn)->next)
            if ((*ppcn)->date >= pnode->date)
                break;

        if (*ppcn && ((*ppcn)->date > pnode->date))
            err_code = bb_csv_copy_node(ppcn, pnode);
    }

    return err_code;
}

static int bb_csv_copy_node(struct bb_csv_node **pcurrent, const struct bb_csv_node *src)
{
    struct bb_csv_node *node;

    node = (struct bb_csv_node *) malloc(sizeof(struct bb_csv_node));
    if (node)
    {
        memcpy(node, src, sizeof(struct bb_csv_node));
        node->next = *pcurrent;
        *pcurrent = node;
    }

    return (!node) ? 1 : 0;
}

static int bb_csv_write_node(const char *file_name, const struct bb_csv_node *pcn, _Bool to_write_headers)
{
    FILE *f;
    struct tm ttm = {0};
    int err_code;

    err_code = 0;

#ifdef BB_WIN
    f = g_fopen(file_name, "ab");
#else
    f = fopen(file_name, "ab");
#endif

    if (!f)
        err_code = 19;
    else
    {
        if (to_write_headers && (fprintf(f, "date;winner_buy;winner_sell;loser_buy;loser_sell;rate\n") < 0))
            err_code = 20;

        if (!err_code)
        {
#ifdef BB_WIN
            gmtime_s(&ttm, &pcn->date);
#else
            gmtime_r(&pcn->date, &ttm);
#endif
            if (fprintf(f, "%02d.%02d.%04d;%.1lf;%.1lf;%.1lf;%.1lf;%.3lf\n", 
                ttm.tm_mday, ttm.tm_mon + 1, ttm.tm_year + 1900, 
                pcn->win_buy, pcn->win_sel, pcn->los_buy, pcn->los_sel, pcn->rate) < 0)
                err_code = 20;
        }

        fclose(f);
    }

    return err_code;
}

static void bb_csv_set_error(int err_code, struct bb_error *perr)
{
    perr->err_code = err_code;
    perr->err_str = bb_csv_error_message(err_code);
}
