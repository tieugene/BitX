#ifdef BB_WIN
#include <glib.h>
#include <glib/gstdio.h>
#else
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#endif
#include "bg_data_list.h"
#include "bg_threads.h"
#include "bb_chart.h"

#define BG_DATA_LIST_ERRS_N 6

struct bg_data_list {                       // Data list structure.
    BG_MUTEX_T mutex;                       // Mutex.
    struct bg_data_node *first;             // Root (first node of data list).
    size_t n;                               // Number of nodes in data list.
};

/*  Sets error data.  */
static void bg_data_list_set_error(int err_code, struct bb_error *perr);

int bg_data_list_init(struct bg_data_list **ppdl, struct bb_error *perr)
{
    int err_code;

    err_code = 0;

    if (!ppdl)
        err_code = 3;
    else
    {
        *ppdl = (struct bg_data_list *) calloc(1, sizeof(struct bg_data_list));
        if (!*ppdl)
            err_code = 1;
        else
            BG_MUTEX_INIT(&(*ppdl)->mutex);
    }

    if (err_code && perr)
        bg_data_list_set_error(err_code, perr);
    return err_code;
}

int bg_data_list_add(struct bg_data_list *pdl, const struct bg_data_node *pd, struct bb_error *perr)
{
    struct bg_data_node *node, **pcurrent;
    int err_code;

    err_code = 0;

    if (!pdl || !pd)
        err_code = 3;
    else
    {
        node = (struct bg_data_node *) calloc(1, sizeof(struct bg_data_node));
        if (!node)
            err_code = 1;
        else
        {
            memcpy(node, pd, sizeof(struct bg_data_node));

            BG_MUTEX_LOCK(&pdl->mutex);

            for (pcurrent = &pdl->first; *pcurrent; pcurrent = &(*pcurrent)->next)
                if ((*pcurrent)->date >= node->date)
                    break;

            node->next = *pcurrent;
            *pcurrent = node;

            pdl->n++;

            BG_MUTEX_UNLOCK(&pdl->mutex);
        }
    }

    if (err_code && perr)
        bg_data_list_set_error(err_code, perr);
    return err_code;
}

int bg_data_list_get(struct bg_data_list *pdl, struct bg_data_node **pfirst, size_t *pn, struct bb_error *perr)
{
    int err_code;

    err_code = 0;

    if (!pdl || !pfirst)
        err_code = 3;
    else
    {
        BG_MUTEX_LOCK(&pdl->mutex);

        *pfirst = pdl->first;
        pdl->first = NULL;

        if (pn)
            *pn = pdl->n;
        pdl->n = 0;

        BG_MUTEX_UNLOCK(&pdl->mutex);
    }

    if (err_code && perr)
        bg_data_list_set_error(err_code, perr);
    return err_code;
}

void bg_data_list_deinit(struct bg_data_list **ppdl)
{
    if (ppdl && *ppdl)
    {
        bg_data_list_free_list(&(*ppdl)->first);

        BG_MUTEX_DEINIT(&(*ppdl)->mutex);

        free(*ppdl);
        *ppdl = NULL;
    }
}

int bg_data_list_write_list(const char *path, const struct bg_data_node *first, struct bb_error *perr)
{
    struct tm ttm = {0};
    FILE *f;
    const struct bg_data_node *p;
    int err_code;

    err_code = 0;

    if (!path)
        err_code = 3;
    else
    {
#ifdef BB_WIN
        f = g_fopen(path, "wb");
#else
        f = fopen(path, "wb");
#endif

        if (!f)
            err_code = 4;
        else
        {
            //if (fprintf(f, "date;buy;sel;win;los;win_buy;win_sel;los_buy;"
            //                "los_sel;dol_buy;dol_sel;btc_buy;btc_sel;win_btc_buy;win_btc_sel;win_dol_buy;"
            //                "win_dol_sel;los_btc_buy;los_btc_sel;los_dol_buy;los_dol_sel;rate\n") < 0)
            if (fprintf(f, "date;buy;sel;win;los;win_buy;win_sel;los_buy;"
                            "los_sel;$_buy;$_sel;btc_buy;btc_sel;win_btc_buy;win_btc_sel;win_$_buy;"
                            "win_$_sel;los_btc_buy;los_btc_sel;los_$_buy;los_$_sel;rate\n") < 0)
                err_code = 5;
            else
                for (p = first; p; p = p->next)
                {
                    memset(&ttm, 0, sizeof(struct tm));
#ifdef BB_WIN
                    gmtime_s(&ttm, &p->date);
#else
                    gmtime_r(&p->date, &ttm);
#endif

                    if (fprintf(f,  "%02d.%02d.%04d;"   // date
                                    "%zu;"              // buy_n
                                    "%zu;"              // sel_n
                                    "%zu;"              // win_n
                                    "%zu;"              // los_n
                                    "%zu;"              // win_buy_n
                                    "%zu;"              // win_sel_n
                                    "%zu;"              // los_buy_n
                                    "%zu;"              // los_sel_n
                                    "%.3lf;"            // dol_buy_n
                                    "%.3lf;"            // dol_sel_n
                                    "%.3lf;"            // btc_buy_n
                                    "%.3lf;"            // btc_sel_n
                                    "%.3lf;"            // win_btc_buy_n
                                    "%.3lf;"            // win_btc_sel_n
                                    "%.3lf;"            // win_dol_buy_n
                                    "%.3lf;"            // win_dol_sel_n
                                    "%.3lf;"            // los_btc_buy_n
                                    "%.3lf;"            // los_btc_sel_n
                                    "%.3lf;"            // los_dol_buy_n
                                    "%.3lf;"            // los_dol_sel_n
                                    "%.3lf\n",          // rate
                                    ttm.tm_mday, ttm.tm_mon + 1, ttm.tm_year + 1900,    // date
                                    p->buy_n, 
                                    p->sel_n, 
                                    p->win_n, 
                                    p->los_n, 
                                    p->win_buy_n, 
                                    p->win_sel_n, 
                                    p->los_buy_n, 
                                    p->los_sel_n, 
                                    (double) p->dol_buy_n / (double) 1000, 
                                    (double) p->dol_sel_n / (double) 1000, 
                                    (double) p->btc_buy_n / (double) BB_SATOSHI_PER_BITCOIN, 
                                    (double) p->btc_sel_n / (double) BB_SATOSHI_PER_BITCOIN, 
                                    (double) p->win_btc_buy_n / (double) BB_SATOSHI_PER_BITCOIN, 
                                    (double) p->win_btc_sel_n / (double) BB_SATOSHI_PER_BITCOIN, 
                                    (double) p->win_dol_buy_n / (double) 1000, 
                                    (double) p->win_dol_sel_n / (double) 1000, 
                                    (double) p->los_btc_buy_n / (double) BB_SATOSHI_PER_BITCOIN, 
                                    (double) p->los_btc_sel_n / (double) BB_SATOSHI_PER_BITCOIN, 
                                    (double) p->los_dol_buy_n / (double) 1000, 
                                    (double) p->los_dol_sel_n / (double) 1000, 
                                    p->rate) < 0)
                    {
                        err_code = 5;
                        break;
                    }
                }

            fclose(f);
        }
    }

    if (err_code && perr)
        bg_data_list_set_error(err_code, perr);
    return err_code;
}

void bg_data_list_free_list(struct bg_data_node **pfirst)
{
    struct bg_data_node *prev;

    if (pfirst)
        while (*pfirst)
        {
            prev = *pfirst;
            *pfirst = prev->next;
/*
            puts("-----");
            puts("date");
            printf("\tdate %ld\n", prev->date);                         // Date.
            puts("buy/sell");
            printf("\tbuy_n %zu\n", prev->buy_n);                       // Number of buyers.
            printf("\tsel_n %zu\n", prev->sel_n);                       // Number of sellers.
            puts("win/lose");
            printf("\twin_n %zu\n", prev->win_n);                       // Number of winners.
            printf("\tlos_n %zu\n", prev->los_n);                       // Number of losers.
            puts("win/lose && buy/sell");
            printf("\twin_buy_n %zu\n", prev->win_buy_n);               // Number of winners-buyers.
            printf("\twin_sel_n %zu\n", prev->win_sel_n);               // Number of winners-sellers.
            printf("\tlos_buy_n %zu\n", prev->los_buy_n);               // Number of losers-buyers.
            printf("\tlos_sel_n %zu\n", prev->los_sel_n);               // Number of losers-sellers.
            puts("last day dollars");
            printf("\tdol_buy_n %ld\n", prev->dol_buy_n);               // Dollars * 1000, bought on last day.
            printf("\tdol_sel_n %ld\n", prev->dol_sel_n);               // Dollars * 1000, sold on last day.
            puts("last day bitcoins");
            printf("\tbtc_buy_n %ld\n", prev->btc_buy_n);               // Bitcoins in satoshi (* 100 000 000), bought on last day.
            printf("\tbtc_sel_n %ld\n", prev->btc_sel_n);               // Bitcoins in satoshi (* 100 000 000), sold on last day.
            puts("win/lose && buy/sell && last day dollars/last day bitcoins");
            printf("\twin_btc_buy_n %ld\n", prev->win_btc_buy_n);       // Bitcoins in satoshi (* 100 000 000), bought on last day by winners.
            printf("\twin_btc_sel_n %ld\n", prev->win_btc_sel_n);       // Bitcoins in satoshi (* 100 000 000), sold on last day by winners.
            printf("\twin_dol_buy_n %ld\n", prev->win_dol_buy_n);       // Dollars * 1000, bought on last day by winners.
            printf("\twin_dol_sel_n %ld\n", prev->win_dol_sel_n);       // Dollars * 1000, sold on last day by winners.
            printf("\tlos_btc_buy_n %ld\n", prev->los_btc_buy_n);       // Bitcoins in satoshi (* 100 000 000), bought on last day by losers.
            printf("\tlos_btc_sel_n %ld\n", prev->los_btc_sel_n);       // Bitcoins in satoshi (* 100 000 000), sold on last day by losers.
            printf("\tlos_dol_buy_n %ld\n", prev->los_dol_buy_n);       // Dollars * 1000, bought on last day by losers.
            printf("\tlos_dol_sel_n %ld\n", prev->los_dol_sel_n);       // Dollars * 1000, sold on last day by losers.
            puts("rate");
            printf("\trate %.3lf\n", prev->rate);                        // Bitcoin rate in dollars.
*/
            free(prev);
        }
}

const char * bg_data_list_error_message(int err_code)
{
    static const char *BG_DATA_LIST_ERRS[BG_DATA_LIST_ERRS_N] = {   "ОК",                                       //  0
                                                                    "Ошибка выделения памяти",                  //  1
                                                                    "Неизвестная ошибка",                       //  2
                                                                    "Пустой указатель",                         //  3
                                                                    "Ошибка при открытии файла для записи",     //  4
                                                                    "Ошибка записи в файл",                     //  5
    };  

    if ((err_code < 0) || (err_code >= BG_DATA_LIST_ERRS_N))
        err_code = 2;

    return *(BG_DATA_LIST_ERRS + err_code);
}

static void bg_data_list_set_error(int err_code, struct bb_error *perr)
{
    perr->err_code = err_code;
    perr->err_str = bg_data_list_error_message(err_code);
}