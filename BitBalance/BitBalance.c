#include "bb_cnf.h"
#include "bb_io.h"
#include "bb_tree.h"
#include "bb_m_tree.h"
#include "bb_rates.h"
#include "bb_chart.h"
#include "bb_cleaning.h"
#include "bb_csv_iter.h"
#include "bb_graph.h"

#define BB_CNF_PATH "./settings.cnf"

#ifdef BB_WIN
#define PUTS_MESSAGE(s) puts_msg(s)
#define PRINT_MESSAGE(s) print_msg(s)
#define WRITE_CHART_PNG(p, c, nbs, wl, e) write_chart_png(p, c, nbs, wl, e)
#define SAVE_GRAPH(f, p, e) graph_save_graph(f, p, e)
#else
#define PUTS_MESSAGE(s) puts(s)
#define PRINT_MESSAGE(s) printf(s)
#define WRITE_CHART_PNG(p, c, nbs, wl, e) bb_write_chart_png(p, c, nbs, wl, e)
#define SAVE_GRAPH(f, p, e) bb_graph_save_graph(f, p, e)
#endif

#ifdef BB_WIN
//#include <windows.h>
#define INITGUID
//#define INITKNOWNFOLDERS
#include <guiddef.h>
#include <knownfolders.h>
#include <glib.h>
#include <glib/gstdio.h>
#define BB_SLASH '\\'
#else
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#define BB_SLASH '/'
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

int write_chart_png(const char *file_name, struct bb_chart_data *chart, enum bb_write_modes nbs, enum bb_write_modes wl, 
    struct bb_error *perr)
{
    char *p;
    int err_code;

    err_code = 0;

    if (file_name)
    {
        p = g_convert(file_name, -1, "CP1251", "UTF-8", NULL, NULL, NULL);
        if (p)
        {
            err_code = bb_write_chart_png(p, chart, nbs, wl, perr);
            g_free(p);
        }
        else
            err_code = bb_write_chart_png(file_name, chart, nbs, wl, perr);
    }

    return err_code;
}

void graph_save_graph(struct bb_csv_node *first, const char *file_name, struct bb_error *perr)
{
    char *p;

    if (file_name)
    {
        p = g_convert(file_name, -1, "CP1251", "UTF-8", NULL, NULL, NULL);
        if (p)
        {
            bb_graph_save_graph(first, p, perr);
            g_free(p);
        }
        else
            bb_graph_save_graph(first, file_name, perr);
    }
}

static void make_directory(const char *path, struct bb_error *perr)
{
    if (g_mkdir_with_parents(path, 0) < 0)
    {
        perr->err_code = 4;
        perr->err_str = "Ошибка при создании директории.";
    }
}
#else
static void make_directory(const char *path, struct bb_error *perr)
{
    struct stat st;
    char *copy_path, *p, c;
    size_t n;

    if (path && *path)
    {
        n = strlen(path);
        copy_path = (char *) malloc(n + 1);
        if (!copy_path)
        {
            perr->err_code = 1;
            perr->err_str = "Ошибка выделения памяти.";
        }
        else
        {
            strncpy(copy_path, path, n + 1);
            *(copy_path + n) = '\0';

            for (p = copy_path; BB_SLASH == *p; ++p)
                continue;
            if (*p)
            {
                do {
                    ++p;
                    if (!*p || (BB_SLASH == *p))
                    {
                        memset(&st, 0, sizeof(struct stat));
                        c = *p;
                        *p = '\0';
                        if ((stat(copy_path, &st) == -1) && 
                            mkdir(copy_path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH))
                            //mkdir(copy_path, 0700))
                        {
                            perr->err_code = 4;
                            perr->err_str = "Ошибка при создании директории.";
                        }
                        *p = c;
                    }
                } while (*p && !perr->err_code);
            }
    
            free(copy_path);
        }
    }
}
#endif

int main(int argc, char **argv)
{
    struct bb_cleaning_data *pcd;
    struct bb_tree_data *ptd;
    struct bb_rates_data *prd;
    struct bb_m_data *pmd;
    struct bb_csv_node *first;
    struct bb_chart_data chart = {0};
    struct bb_cnf_data cd = {0};
    struct bb_csv_node node = {0};
    struct bb_error err = {0};
    struct tm ttm = {0};
    char tmp[1024] = {0};
    char *s_path, *p;
    double last_day_rate;
    FILE *log_f;
    size_t m, n, len, m_n, d_n, s_n, wb_n, ws_n, lb_n, ls_n;
    int year, month, date;

    pcd = NULL;
    ptd = NULL;
    prd = NULL;
    pmd = NULL;
    first = NULL;
    log_f = NULL;
    last_day_rate = (double) 0;
    s_path = NULL;
    len = 0;
    n = 0;
    m_n = d_n = s_n = wb_n = ws_n = lb_n = ls_n = 0;

    tzset();

    PUTS_MESSAGE("1.  Получение настроек из конфигурационного файла.");
    //if (log_f)
    //    fprintf(log_f, "1.  Получение настроек из конфигурационного файла.\n");

    bb_cnf_get_settings(BB_CNF_PATH, &cd, &err);

    if (!err.err_code)
        make_directory(cd.result_dir, &err);

    if (!err.err_code)
    {   
        len = strlen(cd.result_dir);
        n = len + 32;                           // 32 = 1 '/' + 30 "winner_sell.txt" + 1 '\0'
        s_path = (char *) malloc(n);
        if (!s_path)
        {
            err.err_code = 1;
            err.err_str = "Ошибка выделения памяти.";
        }
        else
        {
            if (*(cd.result_dir + len - 1) != BB_SLASH)
            {
                snprintf(s_path, n, "%s%clog.txt", cd.result_dir, BB_SLASH);
                ++len;
            }
            else
                snprintf(s_path, n, "%slog.txt", cd.result_dir);

#ifdef BB_WIN
            log_f = g_fopen(s_path, "wb");
#else
            log_f = fopen(s_path, "wb");
#endif
            //printf("m_path\t\t%s\nd_path\t\t%s\np_path\t\t%s\nresult_dir\t%s\n", cd.m_path, cd.d_path, cd.p_path, cd.result_dir);

            PUTS_MESSAGE("2.  Парсинг D-файла.");
            if (log_f)
                fprintf(log_f, "2.  Парсинг D-файла.\n");

            bb_cleaning_init(cd.d_path, &pcd, &d_n, &err);
            if (!err.err_code)
            {
                year = month = date = 0;
                bb_cleaning_get_min_date(pcd, &year, &month, &date);
                snprintf(tmp, 1024, "\tСамая ранняя дата в D-файле: год %d, месяц %d, число %d.", year, month, date);
                PUTS_MESSAGE(tmp);
                if (log_f)
                    fprintf(log_f, "%s\n", tmp);

                m = d_n % 10;
                snprintf(tmp, 1024, "\tВ D-файле %zu адре%s.", d_n, ((1 == m) ? "с" : ((m > 1) && (m < 5)) ? "са" : "сов"));
                PUTS_MESSAGE(tmp);
                if (log_f)
                    fprintf(log_f, "%s\n", tmp);

                snprintf(s_path + len, n - len, "D_clean.txt");
                PUTS_MESSAGE("3.  Запись D_clean-файла.");
                if (log_f)
                    fprintf(log_f, "3.  Запись D_clean-файла.\n");

                bb_cleaning_write_data(s_path, pcd, year, month, date, &err);
            }
            bb_cleaning_data_free(&pcd, !err.err_code ? &err : NULL);

            if (!err.err_code)
            {
                PUTS_MESSAGE("4.  Парсинг M-файла.");
                if (log_f)
                    fprintf(log_f, "4.  Парсинг M-файла.\n");

                bb_cleaning_init(cd.m_path, &pcd, &m_n, &err);
                if (!err.err_code)
                {
                    m = m_n % 10;
                    snprintf(tmp, 1024, "\tВ M-файле %zu адре%s.", m_n, ((1 == m) ? "с" : ((m > 1) && (m < 5)) ? "са" : "сов"));
                    PUTS_MESSAGE(tmp);
                    if (log_f)
                        fprintf(log_f, "%s\n", tmp);

                    snprintf(s_path + len, n - len, "M_clean.txt");

                    PUTS_MESSAGE("5.  Запись M_clean-файла.");
                    if (log_f)
                        fprintf(log_f, "5.  Запись M_clean-файла.\n");

                    bb_cleaning_write_data(s_path, pcd, year, month, date, &err);
                }
                bb_cleaning_data_free(&pcd, !err.err_code ? &err : NULL);
            }

            if (!err.err_code)
            {
                d_n = m_n = 0;

                PUTS_MESSAGE("6.  Парсинг D_clean-файла.");
                if (log_f)
                    fprintf(log_f, "6.  Парсинг D_clean-файла.\n");

                snprintf(s_path + len, n - len, "D_clean.txt");
                bb_tree_get_tree(s_path, &ptd, &year, &month, &date, &d_n, &err);
                if (!err.err_code)
                {
                    snprintf(tmp, 1024, "\tДата в D_clean-файле: год %d, месяц %d, число %d.", year, month, date);
                    PUTS_MESSAGE(tmp);
                    if (log_f)
                        fprintf(log_f, "%s\n", tmp);

                    m = d_n % 10;
                    snprintf(tmp, 1024, "\tВ D_clean-файле %zu адре%s.", d_n, ((1 == m) ? "с" : ((m > 1) && (m < 5)) ? "са" : "сов"));
                    PUTS_MESSAGE(tmp);
                    if (log_f)
                        fprintf(log_f, "%s\n", tmp);

                    PUTS_MESSAGE("7.  Парсинг P-файла.");
                    if (log_f)
                        fprintf(log_f, "7.  Парсинг P-файла.\n");

                    bb_rates_get_rates(cd.p_path, &prd, &err);
                    if (!err.err_code)
                    {
                        PUTS_MESSAGE("8.  Получение курса из P-файла на дату из D_clean-файла.");
                        if (log_f)
                            fprintf(log_f, "8.  Получение курса из P-файла на дату из D_clean-файла.\n");

                        bb_rates_get_rate(prd, year, month, date, &last_day_rate, &err);

                        if (!err.err_code)
                        {
                            snprintf(tmp, 1024, "\tКурс %.3lf.", last_day_rate);

                            PUTS_MESSAGE(tmp);
                            if (log_f)
                                fprintf(log_f, "%s\n", tmp);

                            PUTS_MESSAGE("9.  Парсинг M_clean-файла.");
                            if (log_f)
                                fprintf(log_f, "9.  Парсинг M_clean-файла.\n");

                            snprintf(s_path + len, n - len, "M_clean.txt");
                            bb_get_m_data(s_path, last_day_rate, &pmd, ptd, prd, &m_n, &err);
                        }
                    }

                    bb_rates_free_rates(&prd);
                }

                bb_tree_free(&ptd, !err.err_code ? &err : NULL);
            }
        }

        if (!err.err_code)
        {
            m = m_n % 10;
            snprintf(tmp, 1024, "\tВ M_clean-файле %zu адре%s.", m_n, ((1 == m) ? "с" : ((m > 1) && (m < 5)) ? "са" : "сов"));
            PUTS_MESSAGE(tmp);
            if (log_f)
                fprintf(log_f, "%s\n", tmp);

            PUTS_MESSAGE("10. Запись S-файла.");
            if (log_f)
                fprintf(log_f, "10. Запись S-файла.\n");

            s_n = bb_get_nodes_n(pmd);
            m = s_n % 10;
            snprintf(tmp, 1024, "\tВ S-файле %zu адре%s.", s_n, ((1 == m) ? "с" : ((m > 1) && (m < 5)) ? "са" : "сов"));
            PUTS_MESSAGE(tmp);
            if (log_f)
                fprintf(log_f, "%s\n", tmp);

            snprintf(s_path + len, n - len, "S.txt");
        }

        if (!err.err_code)
        {
            bb_write_m_data(s_path, pmd, BB_WRITE_ALL, BB_WRITE_ALL, &err);

            if (!err.err_code)
            {
                PUTS_MESSAGE("11. Запись Winner-файла.");

                snprintf(s_path + len, n - len, "Winner.txt");

                bb_write_m_data(s_path, pmd, BB_WRITE_ALL, BB_WRITE_WIN, &err);

                if (!err.err_code)
                {
                    PUTS_MESSAGE("12. Запись Loss-файла.");

                    snprintf(s_path + len, n - len, "Loss.txt");

                    bb_write_m_data(s_path, pmd, BB_WRITE_ALL, BB_WRITE_LOS, &err);

                    if (!err.err_code)
                    {
                        PUTS_MESSAGE("13. Запись winner_buy-файла.");

                        snprintf(s_path + len, n - len, "winner_buy.txt");

                        bb_write_m_data(s_path, pmd, BB_WRITE_BUY, BB_WRITE_WIN, &err);

                        if (!err.err_code)
                        {
                            PUTS_MESSAGE("14. Запись winner_sell-файла.");

                            snprintf(s_path + len, n - len, "winner_sell.txt");

                            bb_write_m_data(s_path, pmd, BB_WRITE_SEL, BB_WRITE_WIN, &err);

                            if (!err.err_code)
                            {
                                PUTS_MESSAGE("15. Запись loss_buy-файла.");

                                snprintf(s_path + len, n - len, "loss_buy.txt");

                                bb_write_m_data(s_path, pmd, BB_WRITE_BUY, BB_WRITE_LOS, &err);

                                if (!err.err_code)
                                {
                                    PUTS_MESSAGE("16. Запись loss_sell-файла.");

                                    snprintf(s_path + len, n - len, "loss_sell.txt");

                                    bb_write_m_data(s_path, pmd, BB_WRITE_SEL, BB_WRITE_LOS, &err);

                                    /*if (!err.err_code)
                                    {
                                        PUTS_MESSAGE("13. Запись balances-файла.");

                                        snprintf(s_path + len, n - len, "balances.csv");

                                        bb_write_balances(s_path, pmd, &err);
                                    }*/

                                    if (!err.err_code)
                                    {
                                        PUTS_MESSAGE("17. Запись balances-файла.");

                                        bb_get_chart_data(pmd, &chart, &err);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        bb_free_m_data(&pmd, !err.err_code ? &err : NULL);

        if (!err.err_code)
        {
            snprintf(s_path + len, n - len, "balances.csv");
            bb_write_chart_data(s_path, &chart, last_day_rate, &err);

            if (!err.err_code)
            {
                PUTS_MESSAGE("18. Запись winner_buy-гистограммы.");

                snprintf(s_path + len, n - len, "winner_buy.png");
                WRITE_CHART_PNG(s_path, &chart, BB_WRITE_BUY, BB_WRITE_WIN, &err);

                if (!err.err_code)
                {
                    PUTS_MESSAGE("19. Запись winner_sell-гистограммы.");

                    snprintf(s_path + len, n - len, "winner_sell.png");
                    WRITE_CHART_PNG(s_path, &chart, BB_WRITE_SEL, BB_WRITE_WIN, &err);

                    if (!err.err_code)
                    {
                        PUTS_MESSAGE("20. Запись loss_buy-гистограммы.");

                        snprintf(s_path + len, n - len, "loss_buy.png");
                        WRITE_CHART_PNG(s_path, &chart, BB_WRITE_BUY, BB_WRITE_LOS, &err);

                        if (!err.err_code)
                        {
                            PUTS_MESSAGE("21. Запись loss_sell-гистограммы.");

                            snprintf(s_path + len, n - len, "loss_sell.png");
                            WRITE_CHART_PNG(s_path, &chart, BB_WRITE_SEL, BB_WRITE_LOS, &err);
                        }
                    }
                }
            }
        }

        if (s_path)
            free(s_path);

        if (!err.err_code)
        {
            bb_chart_get_n(&chart, &wb_n, &ws_n, &lb_n, &ls_n);
            n = wb_n + ws_n + lb_n + ls_n;

            snprintf(tmp, 1024, "\tОбщее количество адресов в четырёх файлах %zu.\n\tИз них:", n);
            PUTS_MESSAGE(tmp);
            if (log_f)
                fprintf(log_f, "%s\n", tmp);

            m = wb_n % 10;
            snprintf(tmp, 1024, "\tв winner_buy-файле %zu адре%s (%.1lf%%);", 
                wb_n, ((1 == m) ? "с" : ((m > 1) && (m < 5)) ? "са" : "сов"), (double) wb_n * (double) 100 / (double) n);
            PUTS_MESSAGE(tmp);
            if (log_f)
                fprintf(log_f, "%s\n", tmp);

            m = ws_n % 10;
            snprintf(tmp, 1024, "\tв winner_sell-файле %zu адре%s (%.1lf%%);", 
                ws_n, ((1 == m) ? "с" : ((m > 1) && (m < 5)) ? "са" : "сов"), (double) ws_n * (double) 100 / (double) n);
            PUTS_MESSAGE(tmp);
            if (log_f)
                fprintf(log_f, "%s\n", tmp);

            m = lb_n % 10;
            snprintf(tmp, 1024, "\tв loss_buy-файле %zu адре%s (%.1lf%%);", 
                lb_n, ((1 == m) ? "с" : ((m > 1) && (m < 5)) ? "са" : "сов"), (double) lb_n * (double) 100 / (double) n);
            PUTS_MESSAGE(tmp);
            if (log_f)
                fprintf(log_f, "%s\n", tmp);

            m = ls_n % 10;
            snprintf(tmp, 1024, "\tв loss_sell-файле %zu адре%s (%.1lf%%).", 
                ls_n, ((1 == m) ? "с" : ((m > 1) && (m < 5)) ? "са" : "сов"), (double) ls_n * (double) 100 / (double) n);
            PUTS_MESSAGE(tmp);
            if (log_f)
                fprintf(log_f, "%s\n", tmp);
        }

        bb_free_chart_data(&chart);

        if (!err.err_code)
        {
            PUTS_MESSAGE("22. Запись csv-файла.");

            ttm.tm_year = year + 100;
            ttm.tm_mon = month - 1;
            ttm.tm_mday = date;

            node.date = mktime(&ttm) - timezone;
            //printf("%lld\n", node.date);
            node.win_buy = (double) wb_n * (double) 100 / (double) n;
            node.win_sel = (double) ws_n * (double) 100 / (double) n;
            node.los_buy = (double) lb_n * (double) 100 / (double) n;
            node.los_sel = (double) ls_n * (double) 100 / (double) n;
            node.rate = last_day_rate;

            bb_csv_get_list(cd.csv_path, &first, &node, &err);
            if (!err.err_code)
            {
                PUTS_MESSAGE("23. Запись графика csv-файла.");

                s_path = NULL;

                for (p = cd.csv_path + strlen(cd.csv_path) - 1; (p > cd.csv_path) && (*p != BB_SLASH); --p)
                    continue;
                for ( ; (p > cd.csv_path) && (BB_SLASH == *p); --p)
                    continue;
                if (p < cd.csv_path)
                    p = cd.csv_path;

                len = p - cd.csv_path;
                if (len)
                {
                    m = ++len + 10;                 // 10 = 1 BB_SLASH + 8 "data.png" + 1 '\0'
                    s_path = (char *) calloc(m, 1);
                    if (!s_path)
                    {
                        err.err_code = 1;
                        err.err_str = "Ошибка выделения памяти.";
                    }
                    else
                    {
                        strncpy(s_path, cd.csv_path, m);
                        snprintf(s_path + len, m - len, "%cdata.png", BB_SLASH);
                    }
                }

                if (!err.err_code)
                    SAVE_GRAPH(first, (!len ? "./data.png" : s_path), &err);

                if (s_path)
                    free(s_path);
            }
            bb_csv_free_list(&first);
        }

        bb_free_cnf_settings(&cd);
    }

    if (err.err_code)
    {
        PUTS_MESSAGE(err.err_str);
        if (log_f)
            fprintf(log_f, "%s\n", err.err_str);
    }

    if (log_f)
        fclose(log_f);
/*
    PRINT_MESSAGE("Нажмите Enter, чтобы выйти ");
    while(getchar() != '\n')
        continue;
*/
    return 0;
}