#ifdef BB_WIN
#define INITGUID
#include <guiddef.h>
#include <knownfolders.h>
#include <glib.h>
#include <glib/gstdio.h>
#else
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#endif

#include <time.h>
#include "bd_cnf.h"
#include "bd_cleaning.h"

#define BD_CNF_PATH "./bd_settings.cnf"

#define BG_D_FILENAME "d.txt"
#define BG_D_FILENAME_LEN 5
// #define BG_D_FILENAME_LEN (sizeof(BG_D_FILENAME) - 1)

#define BG_LOG_FILENAME "log.txt"
#define BG_LOG_FILENAME_LEN 7
// #define BG_LOG_FILENAME_LEN (sizeof(BG_LOG_FILENAME) - 1)

#define BG_MAX_FILE_NAME_LEN ((BG_D_FILENAME_LEN > BG_LOG_FILENAME_LEN) ? BG_D_FILENAME_LEN : BG_LOG_FILENAME_LEN)

#ifdef BB_WIN
#define PUTS_MESSAGE(s) puts_msg(s)
#define PRINT_MESSAGE(s) print_msg(s)
#define BD_SLASH '\\'
#else
#define PUTS_MESSAGE(s) puts(s)
#define PRINT_MESSAGE(s) printf(s)
#define BD_SLASH '/'
#endif

#ifdef BB_WIN
static void puts_msg(const char *s)
{
    char *p;

    if (s)
    {
        p = g_convert(s, -1, "866", "UTF-8", NULL, NULL, NULL);
        if (p)
        {
            puts(p);
            g_free(p);
        }
        else
            puts(s);
    }
}

static void print_msg(const char *s)
{
    char *p;

    if (s)
    {
        p = g_convert(s, -1, "866", "UTF-8", NULL, NULL, NULL);
        if (p)
        {
            printf("%s", p);
            g_free(p);
        }
        else
            printf("%s", s);
    }
}
#endif

static void get_path(const char *m_path, char **ps_path, char **pp, struct bb_error *perr);
static void get_date(const struct bd_cleaning_date *dates, int *pyear, int *pmonth, int *pdate, _Bool *pq);

int main(int argc, char **argv)
{
    struct bb_cleaning_data *pcd;
    struct bd_cleaning_date *dates;
    struct bb_error err = {0};
    char tmp[1024] = {0};
    FILE *log_f;
    char *s_path, *m_path, *p;
    size_t m, m_n;
    int year, month, date;
    _Bool q;

    pcd = NULL;
    dates = NULL;
    log_f = NULL;
    s_path = m_path = p = NULL;
    q = 0;

    tzset();

    PUTS_MESSAGE("1.  Получение настроек из конфигурационного файла.");
    bd_cnf_get_settings(BD_CNF_PATH, &m_path, &err);

    if (!err.err_code)
    {
        //puts(m_path);
        get_path(m_path, &s_path, &p, &err);
        if (!err.err_code)        
        {
            strncpy(p, BG_LOG_FILENAME, BG_MAX_FILE_NAME_LEN + 1);
            *(p + BG_LOG_FILENAME_LEN) = '\0';

#ifdef BB_WIN
            log_f = g_fopen(s_path, "wb");
#else
            log_f = fopen(s_path, "wb");
#endif

            PUTS_MESSAGE("2.  Парсинг M-файла.");
            if (log_f)
                fprintf(log_f, "2.  Парсинг M-файла.\n");

            m_n = 0;            // Dummy.
            bb_cleaning_init(m_path, &pcd, &m_n, &dates, &err);
            if (!err.err_code)
            {
                m = m_n % 10;
                snprintf(tmp, 1024, "\tВ M-файле %zu адре%s.", m_n, ((1 == m) ? "с" : ((m > 1) && (m < 5)) ? "са" : "сов"));
                PUTS_MESSAGE(tmp);
                if (log_f)
                    fprintf(log_f, "%s\n", tmp);

                if (!dates)
                {
                    PUTS_MESSAGE("\tM-файл не содержит транзакций.");
                    if (log_f)
                        fprintf(log_f, "\tM-файл не содержит транзакций.\n");
                }
                else
                {

                    PUTS_MESSAGE("3.  Получение количества адресов для каждой даты.");
                    if (log_f)
                        fprintf(log_f, "3.  Получение количества адресов для каждой даты.\n");

                    bd_cleaning_get_addresses_n(pcd, dates, &err);

                    if (!err.err_code)
                    {
                        year = month = date = 0;
                        get_date(dates, &year, &month, &date, &q);

                        if (!q)
                        {
                            snprintf(tmp, 1024, "\tВыбрана дата: год %d, месяц %d, число %d.", year, month, date);
                            PUTS_MESSAGE(tmp);
                            if (log_f)
                                fprintf(log_f, "%s\n", tmp);

                            strncpy(p, BG_D_FILENAME, BG_MAX_FILE_NAME_LEN + 1);
                            *(p + BG_D_FILENAME_LEN) = '\0';
                            //puts(s_path);

                            PUTS_MESSAGE("4.  Запись D-файла.");
                            if (log_f)
                                fprintf(log_f, "4.  Запись D-файла.\n");

                            bd_cleaning_write_data(s_path, pcd, year, month, date, &err);
                        }
                    }
                }
            }
            bb_cleaning_data_free(&pcd, (!err.err_code ? &err : NULL));

            free(s_path);
        }
    }

    bd_cnf_free_settings(&m_path);

    if (err.err_code)
    {
        PUTS_MESSAGE(err.err_str);
        if (log_f)
            fprintf(log_f, "%s\n", err.err_str);
    }

    if (log_f)
        fclose(log_f);

    if (!q)
    {
        PRINT_MESSAGE("Нажмите Enter, чтобы выйти ");
        while(getchar() != '\n')
            continue;
    }

    return 0;
}

static void get_path(const char *m_path, char **ps_path, char **pp, struct bb_error *perr)
{
    const char *p;
    size_t n;

    for (p = m_path + strlen(m_path) - 1; p >= m_path; --p)
        if (BD_SLASH == *p)
            break;
    while ((p >= m_path) && (BD_SLASH == *p))
        --p;
    ++p;
    if (BD_SLASH == *p)
        ++p;
    n = p - m_path;

    *ps_path = (char *) calloc(n + BG_MAX_FILE_NAME_LEN + 1, 1);
    if (!*ps_path)
    {
        perr->err_code = 1;
        perr->err_str = "Ошибка выделения памяти.";
    }
    else
    {
        if (n)
            strncpy(*ps_path, m_path, n + BG_MAX_FILE_NAME_LEN + 1);
        *pp = *ps_path + n;
    }
}

static void get_date(const struct bd_cleaning_date *dates, int *pyear, int *pmonth, int *pdate, _Bool *pq)
{
    const struct bd_cleaning_date *p;
    int n, x;
    char tmp[64] = {0};
    char c;
    _Bool res;

    *pq = 0;

    //PUTS_MESSAGE("\tM-файл содержит даты:\n\t№\tDate");
    PUTS_MESSAGE("\tM-файл содержит даты:\n\t№\tDate\t\tAddresses");
    for (p = dates, n = 0; p; p = p->next)
    {
        //snprintf(tmp, 64, "\t%3d\t%02d-%02d-%02d", ++n, p->year, p->month, p->date);
        //PUTS_MESSAGE(tmp);

        //printf("\t%d\t%02d-%02d-%02d\n", ++n, p->year, p->month, p->date);
        printf("\t%d\t%02d-%02d-%02d\t%zu\n", ++n, p->year, p->month, p->date, p->n);
    }

    for (res = 0; !res; )
    {
        PRINT_MESSAGE("Введите номер нужной даты из списка или q для выхода: ");
        fflush(stdout);

        x = 0;
        if (scanf("%d", &x) == 1)
        {
            if (x < 1)
                PUTS_MESSAGE("\tОшибка: номер не может быть меньше 1");
            else if (x > n)
            {
                PRINT_MESSAGE("\tОшибка: номер не может быть больше ");
                snprintf(tmp, 64, "%d", n);
                PUTS_MESSAGE(tmp);
            }
            else
            {
                --x;
                for (p = dates, n = 0; n < x; p = p->next, ++n)
                    continue;
                if (!p)
                {
                    *pq = 1;
                    break;
                }
                else
                {
                    *pyear = p->year;
                    *pmonth = p->month;
                    *pdate = p->date;
                }

                res = 1;
            }
        }
        else if ((scanf("%c", &c) == 1) && ('q' == c))
        {
            *pq = 1;
            break;
        }
        else
            PUTS_MESSAGE("\tОшибка: некорректный ввод");
    }

    if (!*pq)
        while(getchar() != '\n')
            continue;
/*
    if (!*pq)
        while (scanf("%64s", tmp) != EOF)
            continue;
*/
}
