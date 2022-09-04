#include "bg_cnf.h"
#include "bg_dirent.h"
#include "bg_queue.h"
#include "bg_ts_error.h"
#include "bg_data_list.h"
#include "bg_threads.h"
#include "bb_chart.h"
#include "bb_rates.h"
#include "bg_cleaning.h"
#include "bg_m_tree.h"
#include "bg_graph.h"

#define BG_CNF_PATH "./bg_settings.cnf"
#define BG_GRAPHS_N 15

#define BG_M_FILENAME_LEN 33

#define BG_D_CLEAN_FILENAME "D_clean_"
#define BG_D_CLEAN_FILENAME_LEN 8
// #define BG_D_CLEAN_FILENAME_LEN (sizeof(BG_D_CLEAN_FILENAME) - 1)

#define BG_M_CLEAN_FILENAME "M_clean_"
#define BG_M_CLEAN_FILENAME_LEN 8
// #define BG_M_CLEAN_FILENAME_LEN (sizeof(BG_M_CLEAN_FILENAME) - 1)

#define BG_BALANCES_FILENAME "balances_"
#define BG_BALANCES_FILENAME_LEN 9
// #define BG_BALANCES_FILENAME_LEN (sizeof(BG_BALANCES_FILENAME) - 1)

#ifdef BB_WIN
#define PUTS_MESSAGE(s) puts_msg(s)
#define PRINT_MESSAGE(s) print_msg(s)
#define SAVE_GRAPH(pd, prd, i, p, sp, e) graph_save_graph(pd, prd, i, p, sp, e)
#define SAVE_RATES_GRAPH(prd, p, sp, e) graph_save_rates_graph(prd, p, sp, e)
#else
#define PUTS_MESSAGE(s) puts(s)
#define PRINT_MESSAGE(s) printf(s)
#define SAVE_GRAPH(pd, prd, i, p, sp, e) bg_graph_save_graph(pd, prd, i, p, sp, e)
#define SAVE_RATES_GRAPH(prd, p, sp, e) bg_graph_save_rates_graph(prd, p, sp, e)
#endif

#ifdef BB_WIN
//#include <windows.h>
#define INITGUID
//#define INITKNOWNFOLDERS
#include <guiddef.h>
#include <knownfolders.h>
#include <glib.h>
#include <glib/gstdio.h>
#else
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

struct thread_data {
    struct bg_queue_data *pqd;
    struct bg_ts_error *ptse;
    struct bg_data_list *pdl;
    struct bb_rates_data *prd;
};

struct task_data {
    char *path;
    const char *file_name;
    size_t path_len;
    int y;
    int m;
    int d;
};

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

static int graph_save_graph(const struct bg_data_node *pd, struct bg_graph_rates_data *prd, size_t numb, const char *path, _Bool save_png, 
    struct bb_error *perr)
{
    char *p;
    int err_code;

    err_code = 0;

    if (path)
    {
        p = g_convert(path, -1, "CP1251", "UTF-8", NULL, NULL, NULL);
        if (p)
        {
            err_code = bg_graph_save_graph(pd, prd, numb, p, save_png, perr);
            g_free(p);
        }
        else
            err_code = bg_graph_save_graph(pd, prd, numb, path, save_png, perr);
    }

    return err_code;
}

static void graph_save_rates_graph(struct bg_graph_rates_data *prd, const char *path, _Bool save_png, struct bb_error *perr)
{
    char *p;

    if (path)
    {
        p = g_convert(path, -1, "CP1251", "UTF-8", NULL, NULL, NULL);
        if (p)
        {
            bg_graph_save_rates_graph(prd, p, save_png, perr);
            g_free(p);
        }
        else
            bg_graph_save_rates_graph(prd, path, save_png, perr);
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

            for (p = copy_path; BG_SLASH == *p; ++p)
                continue;
            if (*p)
            {
                do {
                    ++p;
                    if (!*p || (BG_SLASH == *p))
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

static void get_data_list(const struct bg_cnf_data *pcd, struct bg_data_node **ppd, size_t *pnodes_n, int *prid, FILE *log_f, struct bb_error *perr);
static void * get_data(void *data);
static void get_date_and_rid(const char *path, size_t path_len, int *py, int *pm, int *pd, int *prid, const char **pfile_name, struct bb_error *perr);
static void get_file_name(char *p, size_t m, int numb, const char *p2, const char *path, const char *file_name, size_t path_len, 
    struct bg_ts_error *ptse, struct bb_error *perr);
static inline _Bool is_m_file(const char *path, size_t path_len);

int main(int argc, char **argv)
{
    struct bg_data_node *pd;
    struct bg_graph_rates_data *prd;
    struct bg_cnf_data cd = {0};
    struct bb_error err = {0};
    FILE *log_f;
    char *s_path;
    size_t i, n, len, nodes_n;
    int rid;
    char temp[512] = {0};
    const char *file_names[BG_GRAPHS_N] = { "precent_amount_buysell", 
                                            "precent_amount_winloss_BTC", 
                                            "abs_amount_buysell_BTC", 
                                            "abs_amount_winloss_BTC", 
                                            "percent_winloss_among_byuer", 
                                            "percent_winloss_among_seller", 
                                            "percent_buysell_among_winner", 
                                            "percent_buysell_among_losser", 
                                            "total_amount$_buysell", 
                                            "total_amountBTC_buysell", 
                                            "amount_busell$_among_winner", 
                                            "amount_busell$_among_losser", 
                                            "amount_busellBTC_among_winner", 
                                            "amount_busellBTC_among_losser", 
                                            "persent_deltaBTC"};

    log_f = NULL;
    s_path = NULL;
    pd = NULL;
    prd = NULL;

    tzset();

    PUTS_MESSAGE("1.  Получение настроек из конфигурационного файла.");
    bg_cnf_get_settings(BG_CNF_PATH, &cd, &err);

    if (!err.err_code)
        make_directory(cd.result_dir, &err);

    if (!err.err_code)
    {
        len = strlen(cd.result_dir);
        n = len + 64;                           // 64 = 1 '/' + 62 "amount_busellBTC_among_losser.png" + 1 '\0'
        s_path = (char *) malloc(n);
        if (!s_path)
        {
            err.err_code = 1;
            err.err_str = "Ошибка выделения памяти.";
        }
        else
        {
            if (*(cd.result_dir + len - 1) != BG_SLASH)
            {
                snprintf(s_path, n, "%s%clog.txt", cd.result_dir, BG_SLASH);
                ++len;
            }
            else
                snprintf(s_path, n, "%slog.txt", cd.result_dir);

#ifdef BB_WIN
            log_f = g_fopen(s_path, "wb");
#else
            log_f = fopen(s_path, "wb");
#endif

            nodes_n = 0;
            rid = 0;
            get_data_list(&cd, &pd, &nodes_n, &rid, log_f, &err);

            if (!err.err_code && pd)
            {
                PUTS_MESSAGE("4.  Запись csv-файла с данными для графиков.");
                if (log_f)
                    fprintf(log_f, "4.  Запись csv-файла с данными для графиков.\n");

                snprintf(s_path + len, n - len, "graph_data.csv");
                //puts(s_path);
                bg_data_list_write_list(s_path, pd, &err);

                if (!err.err_code)
                {
                    bg_graph_rates_data_init(pd, nodes_n, &prd, rid, &err);
                    if (!err.err_code)
                    {
                        for (i = 0; !err.err_code && (i < (BG_GRAPHS_N - 1)); ++i)
                        //for (i = 0; !err.err_code && (i < 1); ++i)
                        {
                            snprintf(temp, 512, "%zu.%sЗапись графика %s", (i + 5), ((i > 4) ? " " : "  "), *(file_names + i));
                            PUTS_MESSAGE(temp);

                            snprintf(s_path + len, n - len, "%s.png", *(file_names + i));
                            //bg_graph_save_graph(pd, nodes_n, i, s_path, &err);
                            SAVE_GRAPH(pd, prd, i, s_path, cd.save_png, &err);
                        }

                        if (!err.err_code)
                        {
                            snprintf(temp, 512, "%zu.%sЗапись графика %s", (i + 5), ((i > 4) ? " " : "  "), *(file_names + i));
                            PUTS_MESSAGE(temp);

                            snprintf(s_path + len, n - len, "%s.png", *(file_names + i));
                            //bg_graph_save_rates_graph(pd, nodes_n, s_path, &err);
                            SAVE_RATES_GRAPH(prd, s_path, cd.save_png, &err);
                        }

                    }
                    bg_graph_rates_data_deinit(&prd);
                }
            }

            bg_data_list_free_list(&pd);

            free(s_path);
        }
    }

    bg_free_cnf_settings(&cd);

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

static void get_data_list(const struct bg_cnf_data *pcd, struct bg_data_node **ppd, size_t *pnodes_n, int *prid, FILE *log_f, struct bb_error *perr)
{
    BG_THREAD_T *threads;
    struct bg_dirent_data *pdd;
    struct bg_queue_data *pqd;
    struct bg_ts_error *ptse;
    struct bg_data_list *pdl;
    struct task_data *task;
    struct thread_data *ptd;
    struct bb_rates_data *prd;
    const char *path, *file_name;
    void *t;
    char temp[128] = {0};
    size_t path_len, i, j, n;
    int y, m, d, rid;

    pdd = NULL;
    ptse = NULL;
    pdl = NULL;
    pqd = NULL;
    ptd = NULL;
    prd = NULL;
    *ppd = NULL;


    PUTS_MESSAGE("2.  Парсинг P-файла.");
    if (log_f)
        fprintf(log_f, "2.  Парсинг P-файла.\n");

    bb_rates_get_rates(pcd->p_path, &prd, perr);
    if (!perr->err_code)
    {
        PUTS_MESSAGE("3.  Получение данных из файлов.");
        if (log_f)
            fprintf(log_f, "3.  Получение данных из файлов.\n");

        bg_data_list_init(&pdl, perr);
        if (!perr->err_code)
        {
            bg_ts_error_init(&ptse, log_f, perr);
            if (!perr->err_code)
            {
                bg_queue_init(&pqd, perr);
                if (!perr->err_code)
                {
                    bg_dirent_init(pcd->s_dir, &pdd, perr);
                    n = 0;
                    while (!perr->err_code)
                    {
                        path = NULL;
                        path_len = 0;

                        bg_dirent_iter(pdd, &path, &path_len, perr);
                        if (!path || perr->err_code)
                            break;

                        //printf("%s\t%zu\n", path, path_len);

                        if (is_m_file(path, path_len))
                        {
                            //printf("%s\t%zu\n", path, path_len);

                            y = m = d = rid = 0;
                            file_name = NULL;
                            get_date_and_rid(path, path_len, &y, &m, &d, &rid, &file_name, perr);
                            //printf("%s\t%zu\t%d\t%d\t%d\t%d\t%s\n", path, path_len, y, m, d, rid, file_name);

                            if ((rid < 1) || (rid > 11))
                            {
                                perr->err_code = 8;
                                perr->err_str = "rid должен быть в пределах от 1 до 11";
                            }
                            else if (!*prid)
                                *prid = rid;
                            else if (*prid != rid)
                            {
                                perr->err_code = 9;
                                perr->err_str = "rid отличается";
                            }

                            if (perr->err_code)
                                puts(path);
                            else
                            {
                                task = (struct task_data *) malloc(sizeof(struct task_data) + path_len + 1);
                                if (!task)
                                {
                                    perr->err_code = 1;
                                    perr->err_str = "Ошибка выделения памяти";
                                }
                                else
                                {
                                    task->path = (char *) (task + 1);
                                    strncpy(task->path, path, path_len + 1);
                                    *(task->path + path_len) = '\0';

                                    task->path_len = path_len;
                                    task->file_name = task->path + (file_name - path);

                                    task->y = y;
                                    task->m = m;
                                    task->d = d;

                                    bg_queue_add(pqd, (void *) task, perr);
                                    ++n;
                                }
                            }
                        }
                    }
                    bg_dirent_deinit(&pdd, (!perr->err_code ? perr : NULL));

                    if (!perr->err_code && n)
                    {
                        if (n > pcd->threads_n)
                            n = pcd->threads_n;

                        threads = (BG_THREAD_T *) calloc(n, sizeof(BG_THREAD_T));

                        if (!threads)
                        {
                            perr->err_code = 1;
                            perr->err_str = "Ошибка выделения памяти";
                        }
                        else
                        {
                            ptd = (struct thread_data *) calloc(n, sizeof(struct thread_data));
                            if (!threads)
                            {
                                perr->err_code = 1;
                                perr->err_str = "Ошибка выделения памяти";
                            }
                            else
                            {
                                for (i = 0; (i < n) && !perr->err_code; ++i)
                                {
                                    (ptd + i)->pqd = pqd;
                                    (ptd + i)->ptse = ptse;
                                    (ptd + i)->pdl = pdl;
                                    (ptd + i)->prd = prd;
#ifdef BB_WIN
                                    *(threads + i) = g_thread_new(NULL, get_data, ptd + i);
                                    if (!*(threads + i))
#else
                                   if (pthread_create(threads + i, NULL, get_data, ptd + i))
#endif
                                    {
                                        perr->err_code = 4;
                                        perr->err_str = "Ошибка при создании потока";

                                        bg_ts_error_set_error_data(ptse, perr, NULL, NULL);
                                    }
                                }

                                for (j = 0; j < i; ++j)
                                    BG_THREAD_JOIN(*(threads + j));

                                free(ptd);
                            }

                            free(threads);
                        }
                    }
                }

                for (bg_queue_get(pqd, &t, NULL); t; bg_queue_get(pqd, &t, NULL))
                {
                    //task = (struct task_data *) t;
                    //printf("%s\t%zu\n", task->path, task->path_len);

                    free(t);
                }
                bg_queue_deinit(&pqd);

                if (!perr->err_code && bg_ts_error_get_error_code(ptse))
                    bg_ts_error_get_error_data(ptse, perr);
            }
            bg_ts_error_deinit(&ptse);
        }
        if (!perr->err_code)
        {
            bg_data_list_get(pdl, ppd, pnodes_n, perr);

            snprintf(temp, 128, "\tПолучены данные для %zu да%s", *pnodes_n, ((((11 == *pnodes_n) || (*pnodes_n % 10) != 1)) ? "т" : "ты"));
            PUTS_MESSAGE(temp);
        }
        bg_data_list_deinit(&pdl);
    }
    bb_rates_free_rates(&prd);
}

static void * get_data(void *data)
{
    struct thread_data *ptd;
    struct task_data *task;
    struct bb_cleaning_data *pcd;
    struct bb_tree_data *ptree;
    struct bb_m_data *pmd;
    struct bg_data_node node = {0};
    struct bb_chart_data chart = {0};
    struct bb_error err = {0};
    struct tm ttm = {0};
    char *s_path, *p;
    const char *p2;
    void *t;
    double last_day_rate;
    size_t m, n, d_n;
    int year, month, date;

    ptd = (struct thread_data *) data;

    n = (BG_D_CLEAN_FILENAME_LEN > BG_M_CLEAN_FILENAME_LEN) ? BG_D_CLEAN_FILENAME_LEN : BG_M_CLEAN_FILENAME_LEN;
    if (n < BG_BALANCES_FILENAME_LEN)
        n = BG_BALANCES_FILENAME_LEN;
    n += 5;         // 5 = 4 ".txt" or ".csv" + 1 '\0'

    for (bg_queue_get(ptd->pqd, &t, NULL); t && !err.err_code; )
    {
        task = (struct task_data *) t;

        pcd = NULL;
        pmd = NULL;
        last_day_rate = (double) 0;

        s_path = (char *) malloc(task->path_len + n);
        //printf("%zu\n", task->path_len + n);
        //fflush(stdout);
        if (!s_path)
        {
            err.err_code = 1;
            err.err_str = "Ошибка выделения памяти";

            bg_ts_error_set_error_data(ptd->ptse, &err, task->path, "Выделение памяти для создания путей");
        }
        else
        {
            strncpy(s_path, task->path, task->path_len + n);

            m = task->file_name - task->path;
            p = s_path + m;
            m = task->path_len + n - m;

            //puts(p);
            //fflush(stdout);
            //*p++ = BG_SLASH;

            for (p2 = task->path + task->path_len - 1; p2 > task->file_name; --p2)
                if ('.' == *p2)
                    break;
            if (p2 <= task->file_name)
                p2 = task->path + task->path_len;

            get_file_name(p, m, 1, p2, task->path, task->file_name, task->path_len, ptd->ptse, &err);
            if (!err.err_code)
            {
                d_n = 0;            // Dummy.
                bb_cleaning_init(task->path, &pcd, &d_n, &err);
                if (!err.err_code)
                {
                    bb_cleaning_write_data(s_path, pcd, task->y, task->m, task->d, 0, &err);
                    if (!err.err_code)
                    {
                        get_file_name(p, m, 2, p2, task->path, task->file_name, task->path_len, ptd->ptse, &err);
                        if (!err.err_code)
                        {
                            bb_cleaning_write_data(s_path, pcd, task->y, task->m, task->d, 1, &err);
                            if (err.err_code)
                                bg_ts_error_set_error_data(ptd->ptse, &err, task->path, "Запись M_clean-файла");
                        }
                    }
                    else
                        bg_ts_error_set_error_data(ptd->ptse, &err, task->path, "Запись D_clean-файла");
                }
                else
                    bg_ts_error_set_error_data(ptd->ptse, &err, task->path, "Получение D_clean-файла");

                if (!err.err_code)
                {
                    bb_cleaning_data_free(&pcd, &err);
                    if (err.err_code)
                        bg_ts_error_set_error_data(ptd->ptse, &err, task->path, "Освобождение памяти из-под данных clean-файлов");
                }
                else
                    bb_cleaning_data_free(&pcd, NULL);

                if (!err.err_code)
                {
                    ptree = NULL;
                    year = month = date = 0;
                    d_n = 0;

                    get_file_name(p, m, 1, p2, task->path, task->file_name, task->path_len, ptd->ptse, &err);
                    if (!err.err_code)
                    {
                        bb_tree_get_tree(s_path, &ptree, &year, &month, &date, &d_n, &err);
                        if (!err.err_code)
                        {
                            last_day_rate = (double) 0;
                            bb_rates_get_rate(ptd->prd, year, month, date, &last_day_rate, &err);

                            if (!err.err_code)
                            {
                                get_file_name(p, m, 2, p2, task->path, task->file_name, task->path_len, ptd->ptse, &err);
                                if (!err.err_code)
                                {
                                    d_n = 0;

                                    bb_get_m_data(s_path, last_day_rate, &pmd, ptree, ptd->prd, &d_n, &err);
                                    if (err.err_code)
                                        bg_ts_error_set_error_data(ptd->ptse, &err, task->path, "Парсинг M_clean-файла");
                                }
                            }
                            else
                                bg_ts_error_set_error_data(ptd->ptse, &err, task->path, "Получение курса биткоина для даты из D_clean-файла");
                        }
/**/
                        else if (!strcmp(err.err_str, "Пустой файл"))
                            memset(&err, 0, sizeof(struct bb_error));
/**/
                        else
                            bg_ts_error_set_error_data(ptd->ptse, &err, task->path, "Парсинг D_clean-файла");

                        bb_tree_free(&ptree, !err.err_code ? &err : NULL);

                        if (!err.err_code)
                        {
                            bb_tree_free(&ptree, &err);
                            if (err.err_code)
                                bg_ts_error_set_error_data(ptd->ptse, &err, task->path, "Освобождение памяти из-под дерева D_clean-файла");
                        }
                        else
                            bb_tree_free(&ptree, NULL);
                    }
                }

                if (!err.err_code && pmd)
                {
                    bb_get_chart_data(pmd, &chart, &err);

                    if (!err.err_code)
                    {
                        get_file_name(p, m, 0, p2, task->path, task->file_name, task->path_len, ptd->ptse, &err);
                        if (!err.err_code)
                        {
                            bb_write_chart_data(s_path, &chart, last_day_rate, &err);

                            if (err.err_code)
                                bg_ts_error_set_error_data(ptd->ptse, &err, task->path, "Запись файла balances.csv");
                        }
                    }
                    else
                        bg_ts_error_set_error_data(ptd->ptse, &err, task->path, "Получение данных для файла balances.csv");

                    bb_free_chart_data(&chart);

                    if (!err.err_code)
                    {
                        bb_get_graph_data(pmd, &node, last_day_rate, &err);
                        if (err.err_code)
                            bg_ts_error_set_error_data(ptd->ptse, &err, task->path, "Получение целевых показателей");
                    }

                    if (!err.err_code)
                    {
                        bb_free_m_data(&pmd, &err);
                        if (err.err_code)
                            bg_ts_error_set_error_data(ptd->ptse, &err, task->path, "Освобождение памяти из-под дерева M_clean-файла");
                    }
                    else
                        bb_free_m_data(&pmd, NULL);

                    if (!err.err_code)
                    {
                        ttm.tm_year = year + 100;
                        ttm.tm_mon = month - 1;
                        ttm.tm_mday = date;

                        node.date = mktime(&ttm) - timezone;

                        node.rate = last_day_rate;

                        bg_data_list_add(ptd->pdl, &node, &err);
                        if (err.err_code)
                            bg_ts_error_set_error_data(ptd->ptse, &err, task->path, "Добавление целевых показателей в список");
                    }
                }
            }

            free(s_path);
        }

        free(t);
        t = NULL;

        if (!err.err_code)
            bg_queue_get(ptd->pqd, &t, NULL);
    }

    return NULL;
}

static void get_date_and_rid(const char *path, size_t path_len, int *py, int *pm, int *pd, int *prid, const char **pfile_name, struct bb_error *perr)
{
    struct tm ttm = {0};
    const char *p, *p2;
    char y_str[3] = {0};
    char m_str[3] = {0};
    char d_str[3] = {0};
    _Bool done;

    for (done = 0, p = path, p2 = p + path_len - 1; p2 >= p; --p2)
    {
        if (BG_SLASH == *p2)
            break;
        else if (!done && 
                ('_' == *p2) && 
                ('r' == *(p2 + 1)) && 
                ('i' == *(p2 + 2)) && 
                ('d' == *(p2 + 3)) && 
                (sscanf(p2 + 4, "%d", prid) == 1))
            done = 1;
    }

    if (!done)
    {
        perr->err_code = 5;
        perr->err_str = "Не удалось найти rid в имени файла";
    }
    else
    {
        *pfile_name = p2 + 1;
        for (done = 0, p = *pfile_name, p2 = path + path_len - 6; (p < p2) && !done; ++p)
            if ('-' == *p)
            {
                if ((*(p + 1) > 47) && (*(p + 1) < 58) && 
                    (*(p + 2) > 47) && (*(p + 2) < 58) && 
                    (*(p + 3) > 47) && (*(p + 3) < 58) && 
                    (*(p + 4) > 47) && (*(p + 4) < 58) && 
                    (*(p + 5) > 47) && (*(p + 5) < 58) && 
                    (*(p + 6) > 47) && (*(p + 6) < 58))
                {
                    *y_str = *(p + 1);
                    *(y_str + 1) = *(p + 2);
                    *m_str = *(p + 3);
                    *(m_str + 1) = *(p + 4);
                    *d_str = *(p + 5);
                    *(d_str + 1) = *(p + 6);

                    if ((sscanf(y_str, "%02d", py) == 1) &&
                        (sscanf(m_str, "%02d", pm) == 1) &&
                        (sscanf(d_str, "%02d", pd) == 1))
                        done = 1;
                }
            }

        if (!done)
        {
            perr->err_code = 6;
            perr->err_str = "Не удалось найти дату в имени файла";
        }
        else
        {
            ttm.tm_year = *py + 100;    // 2022 - 1900
            ttm.tm_mon = *pm - 1;       // 7 - 1
            ttm.tm_mday = *pd;          // 1

            ttm.tm_mday--;

            mktime(&ttm);

            *py = ttm.tm_year - 100;
            *pm = ttm.tm_mon + 1;
            *pd = ttm.tm_mday;
        }
    }
}

static void get_file_name(char *p, size_t m, int numb, const char *p2, const char *path, const char *file_name, size_t path_len, 
    struct bg_ts_error *ptse, struct bb_error *perr)
{
    char *p3;
    int k;

    k = snprintf(p, m, "%s%s", ((1 == numb) ?   BG_D_CLEAN_FILENAME :
                                (2 == numb) ?   BG_M_CLEAN_FILENAME :
                                                BG_BALANCES_FILENAME), file_name);
    if (k < 0)
    {
        perr->err_code = 7;
        perr->err_str = "Ошибка snprintf";

        bg_ts_error_set_error_data(ptse, perr, path,   ((1 == numb) ?   "Создание пути D_clean-файла" :
                                                        (2 == numb) ?   "Создание пути M_clean-файла" :
                                                                        "Создание пути balances-файла"));
    }
    else
    {
        p3 = p + (size_t) k - (path + path_len - p2);
        strncpy(p3, (!numb ? ".csv" : ".txt"), m - k);
        *(p3 + 4) = '\0';
    }
}

static inline _Bool is_m_file(const char *path, size_t path_len)
{
    const char *p;

    for (p = path + path_len - 1; p >= path; --p)
        if (BG_SLASH == *p)
            break;
    ++p;

    return ((BG_M_FILENAME_LEN == ((p <= path) ? path_len : (path + path_len - p))) && 
            (('r' == *(path + path_len - 9)) || ('R' == *(path + path_len - 9))) && 
            (('i' == *(path + path_len - 8)) || ('I' == *(path + path_len - 8))) && 
            (('d' == *(path + path_len - 7)) || ('D' == *(path + path_len - 7))) && 
            ((('0' == *(path + path_len - 6)) && (*(path + path_len - 5) >= '1') && (*(path + path_len - 5) <= '9')) || 
             (('1' == *(path + path_len - 6)) && (('0' == *(path + path_len - 5)) || ('1' == *(path + path_len - 5))))) && 
            ('.' == *(path + path_len - 4)) && 
            (('t' == *(path + path_len - 3)) || ('T' == *(path + path_len - 3))) && 
            (('x' == *(path + path_len - 2)) || ('X' == *(path + path_len - 2))) && 
            (('t' == *(path + path_len - 1)) || ('T' == *(path + path_len - 1))));
}
