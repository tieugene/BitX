#include <stdio.h>
#include <stdlib.h>
#include "bb_rates.h"
#include "bb_io.h"
#include "bb_scan_double.h"
#include <math.h>

#define BB_RATES_ERRS_N 16                  // Number of errors.

#define BB_DATE_N 3

struct bb_rates_day {                       // Rates data for day.
    double rate;                            // Rate.
    _Bool known;                            // 1 if rate is known, 0 if unknown.
};

struct bb_rates_data {                      // Rates data for year.
    int year;                               // Year.
    struct bb_rates_day rates[BB_MONTHS_PER_YEAR * BB_DAYS_PER_MONTH];  // Rates for days.
    struct bb_rates_data *next;             // Next node pointer.
};

/*  Get year, month, date and rate from string.  */
static int bb_get_rate_from_string(char *s, int *pyear, int *pmonth, int *pdate, double *prate);

/*  Add rate data to rates.  */
static int bb_add_rate(int year, int month, int date, double rate, struct bb_rates_data **pprd);

/* Sets error data. */
static void set_error(int err_code, struct bb_error *perr);

int bb_rates_get_rates(const char *file_name, struct bb_rates_data **pprd, struct bb_error *perr)
{
    char *data, *p, *p2, c;
    double rate;
    size_t len;
    int year, month, date, m, n, err_code;

    data = NULL;
    len = 0;
    err_code = 0;

    if (!file_name || !pprd)
        err_code = 3;
    else if (!bb_read_file(file_name, &data, &len, perr) && data)
    {
        n = 0;
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
                        year = month = date = 0;
                        rate = (double) 0;
                        err_code = bb_get_rate_from_string(p2, &year, &month, &date, &rate);
                        if (!err_code)
                            err_code = bb_add_rate(year, month, date, rate, pprd);
                        *p = c;

                        if (*p)
                            ++p;
                    }
                }
            }
        }

        free(data);
    }

    if (err_code && perr)
        set_error(err_code, perr);
    return err_code;
}

static int bb_get_rate_from_string(char *s, int *pyear, int *pmonth, int *pdate, double *prate)
{
    char *p;
    char *pp[BB_DATE_N] = {0};
    int i, err_code;

    err_code = 0;
    for (p = s, i = 0; *p && (i < BB_DATE_N); ++p)
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

    if (i < BB_DATE_N)
        err_code = 5;
    else if (!bb_scan_double(s, prate))
        err_code = 6;
    else if (sscanf(*pp, "%d", pyear) != 1)
        err_code = 7;
    else if (*pyear < 0)
        err_code = 8;
    else if (sscanf(*(pp + 1), "%d", pmonth) != 1)
        err_code = 9;
    else if ((*pmonth <= 0) || (*pmonth > BB_MONTHS_PER_YEAR))
        err_code = 10;
    else if (sscanf(*(pp + 2), "%d", pdate) != 1)
        err_code = 11;
    else if ((*pdate <= 0) || (*pdate > BB_DAYS_PER_MONTH))
        err_code = 12;

    return err_code;
}

static int bb_add_rate(int year, int month, int date, double rate, struct bb_rates_data **pprd)
{
    struct bb_rates_data **pcurrent, *node;
    struct bb_rates_day *r;
    int err_code;

    err_code = 0;
    for (pcurrent = pprd; *pcurrent; pcurrent = &(*pcurrent)->next)
        if ((*pcurrent)->year >= year)
            break;

    if (!*pcurrent || ((*pcurrent)->year > year))
    {
        node = (struct bb_rates_data *) calloc(1, sizeof(struct bb_rates_data));
        if (!node)
            err_code = 1;
        else
        {
            node->year = year;
            node->next = *pcurrent;
            *pcurrent = node;
        }
    }

    if (!err_code)
    {
        r = (*pcurrent)->rates + (month - 1) * BB_DAYS_PER_MONTH + (date - 1);
        r->rate = rate;
        r->known = 1;
    }

    return err_code;
}

int bb_rates_get_rate(struct bb_rates_data *prd, int year, int month, int date, double *prate, struct bb_error *perr)
{
    struct bb_rates_data *node;
    struct bb_rates_day *r;
    int err_code;

    err_code = 0;

    if (!prd || !prate)
        err_code = 3;
    else
    {
        for (node = prd; node; node = node->next)
            if (node->year >= year)
                break;

        if (!node || (node->year > year))
            err_code = 13;
        else
        {
            r = node->rates + (month - 1) * BB_DAYS_PER_MONTH + (date - 1);
            if (!r->known)
                err_code = 14;
            else
                *prate = r->rate;
        }
    }

    if (err_code && perr)
        set_error(err_code, perr);
    return err_code;
}
/*
int bb_rates_get_last_rate(struct bb_rates_data *prd, double *prate, struct bb_error *perr)
{
    struct bb_rates_data *node;
    struct bb_rates_day *r;
    int err_code;

    err_code = 0;

    if (!prate)
        err_code = 3;
    else if (!prd)
        err_code = 15;
    else
    {
        for (node = prd; node->next; node = node->next)
            continue;

        for (r = node->rates + BB_MONTHS_PER_YEAR * BB_DAYS_PER_MONTH - 1; r >= node->rates; --r)
            if (r->known)
            {
                *prate = r->rate;
                break;
            }
    }

    if (err_code && perr)
        set_error(err_code, perr);
    return err_code;
}
*/
void bb_rates_free_rates(struct bb_rates_data **pprd)
{
    struct bb_rates_data *node;

    if (pprd)
        while (*pprd)
        {
            node = *pprd;
            *pprd = node->next;
            /*for (int i = 0; i < BB_MONTHS_PER_YEAR; ++i)
                for (int j = 0; j < BB_DAYS_PER_MONTH; ++j)
                {
                    struct bb_rates_day *rate = node->rates + i * BB_DAYS_PER_MONTH + j;
                    if (rate->known)
                        printf("%.3lf,%d,%d,%d\n", rate->rate, node->year, (i + 1), (j + 1));
                }*/
            free(node);
        }
}

const char * bb_rates_error_message(int err_code)
{
    static const char *BB_RATES_ERRS[BB_RATES_ERRS_N] = {   "ОК",                                                   //  0
                                                            "Ошибка выделения памяти",                              //  1
                                                            "Неизвестная ошибка",                                   //  2
                                                            "Пустой указатель",                                     //  3
                                                            "Ошибка в данных P-файла",                              //  4
                                                            "В строке курса меньше элементов, чем необходимо",      //  5
                                                            "Не удалось получить курс из строки курса",             //  6
                                                            "Не удалось получить год из строки курса",              //  7
                                                            "Некорректное значение года в строке курса",            //  8
                                                            "Не удалось получить месяц из строки курса",            //  9
                                                            "Некорректное значение месяца в строке курса",          // 10
                                                            "Не удалось получить число (дату) из строки курса",     // 11
                                                            "Некорректное значение числа (даты) в строке курса",    // 12
                                                            "Ошибка: нет курса для заданного года",                 // 13
                                                            "Ошибка: нет курса для заданной даты",                  // 14
                                                            "Ошибка: нет курса ни для одной даты",                  // 15
    };

    if ((err_code < 0) || (err_code >= BB_RATES_ERRS_N))
        err_code = 2;

    return *(BB_RATES_ERRS + err_code);
}

static void set_error(int err_code, struct bb_error *perr)
{
    perr->err_code = err_code;
    perr->err_str = bb_rates_error_message(err_code);
}