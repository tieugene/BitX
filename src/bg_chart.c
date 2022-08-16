//#include <stdlib.h>
//#include <stdio.h>
#include <string.h>
#include <math.h>
#include "bb_chart.h"

#ifdef BB_WIN
#include <glib.h>
#include <glib/gstdio.h>
#else
#include <stdio.h>
#include <stdlib.h>
#endif

#define BB_CHART_ERRS_N 7                   // Number of errors.

/* Sets error data. */
static void bg_chart_set_error(int err_code, struct bb_error *perr);

int bb_balance_data_compare(const void *pbd1, const void *pbd2)
{
    return strcmp((*((struct bb_balance_data **) pbd1))->address, (*((struct bb_balance_data **) pbd2))->address);
}

size_t bb_get_balance_s_len(double balance)
{
    size_t balance_s_len;
    int64_t n;

    //balance_s_len = (balance < -0.0) ? 5 : 4;   // 4 (5) = 4 .3 + (+ 1 -)
    balance_s_len = (balance < -BB_D) ? 5 : 4;   // 4 (5) = 4 .3 + (+ 1 -)
    n = (int64_t) balance;
    do {
        ++balance_s_len;
        n /= 10;
    } while (n);

    return balance_s_len;
}

void bb_get_balance_data(struct bb_chart_data *pcd, const char *address, size_t address_len, int64_t satoshi, 
    double dollars, double last_day_dollars, double balance, int64_t last_day_satoshi, struct bb_balance_data **ppbd)
{
    *ppbd = (struct bb_balance_data *) malloc(sizeof(struct bb_balance_data) + address_len + 1);
    if (*ppbd)
    {
        (*ppbd)->address = (char *) (*ppbd + 1);
        strncpy((*ppbd)->address, address, address_len + 1);
        *((*ppbd)->address + address_len) = '\0';

        (*ppbd)->address_len = address_len;

        (*ppbd)->balance = balance;
        (*ppbd)->satoshi = satoshi;
        (*ppbd)->dollars = dollars;
        (*ppbd)->last_day_dollars = last_day_dollars;
        (*ppbd)->last_day_satoshi = last_day_satoshi;

        if ((*ppbd)->last_day_satoshi > (int64_t) 0)
        {
            //if ((*ppbd)->balance > +0.0)               // win
            if ((*ppbd)->balance > +BB_D)               // win
            {
                if (pcd->max_buy_balance < (*ppbd)->balance)
                    pcd->max_buy_balance = (*ppbd)->balance;
                if (pcd->win_buy_max_address_len < (*ppbd)->address_len)
                    pcd->win_buy_max_address_len = (*ppbd)->address_len;
                pcd->win_buy_balances_n++;
            }
            //else if ((*ppbd)->balance < -0.0)          // loss
            else if ((*ppbd)->balance < -BB_D)          // loss
            {
                if (pcd->min_buy_balance > (*ppbd)->balance)
                    pcd->min_buy_balance = (*ppbd)->balance;
                if (pcd->los_buy_max_address_len < (*ppbd)->address_len)
                    pcd->los_buy_max_address_len = (*ppbd)->address_len;
                pcd->los_buy_balances_n++;
            }
        }
        else if ((*ppbd)->last_day_satoshi < (int64_t) 0)
        {
            //if ((*ppbd)->balance > +0.0)               // win
            if ((*ppbd)->balance > +BB_D)               // win
            {
                if (pcd->max_sel_balance < (*ppbd)->balance)
                    pcd->max_sel_balance = (*ppbd)->balance;
                if (pcd->win_sel_max_address_len < (*ppbd)->address_len)
                    pcd->win_sel_max_address_len = (*ppbd)->address_len;
                pcd->win_sel_balances_n++;
            }
            //else if ((*ppbd)->balance < -0.0)          // loss
            else if ((*ppbd)->balance < -BB_D)          // loss
            {
                if (pcd->min_sel_balance > (*ppbd)->balance)
                    pcd->min_sel_balance = (*ppbd)->balance;
                if (pcd->los_sel_max_address_len < (*ppbd)->address_len)
                    pcd->los_sel_max_address_len = (*ppbd)->address_len;
                pcd->los_sel_balances_n++;
            }
        }
    }
}

int bb_write_chart_data(const char *file_name, struct bb_chart_data *chart, double last_day_rate, struct bb_error *perr)
{
    FILE *f;
    struct bb_balance_data **p, **p2;
    int err_code;
    //_Bool first_str;

    err_code = 0;
    //first_str = 1;

    if (!file_name || !chart)
        err_code = 3;
    else
    {
#ifdef BB_WIN
        f = g_fopen(file_name, "wb");
#else
        f = fopen(file_name, "wb");
#endif

        if (!f)
            err_code = 4;
        //else if (fprintf(f, "Address;Bitcoins;Dollars;LastDayDollars;Balance;Buy/Sell/Nothing\n") < 0)
        //else if (fprintf(f, "Address;valuewalletBTC;valuewallet$;LastDayDollars;tradingbalance;Buy/Sell/Nothing\n") < 0)
        else if (fprintf(f, "Address;valuewalletBTC;valuewallet$;LastDayDollars;tradingbalance;Buy/Sell/Nothing;%.3lf\n", last_day_rate) < 0)
            err_code = 5;
        else
        {
            if (chart->balances)
                for (p = chart->balances, p2 = p + chart->balances_n; (p < p2) && !err_code; ++p)
                {
                    /*if (!first_str)
                    {*/
                        if (fprintf(f, "%s;%.3lf;%.3lf;%.3lf;%.3lf%s%.3lf\n", 
                                    (*p)->address, 
                                    (double) (*p)->satoshi / (double) BB_SATOSHI_PER_BITCOIN, 
                                    (*p)->dollars, 
                                    (*p)->last_day_dollars, 
                                    (*p)->balance, 
                                    ((*p)->last_day_satoshi > (int64_t) 0) ? ";+" : ";",
                                    (double) (*p)->last_day_satoshi / (double) BB_SATOSHI_PER_BITCOIN) < 0)
                            err_code = 5;
                    /*}
                    else
                    {
                        if (fprintf(f, "%s;%.3lf;%.3lf;%.3lf;%.3lf%s%.3lf;%.3lf\n", 
                                    (*p)->address, 
                                    (double) (*p)->satoshi / (double) BB_SATOSHI_PER_BITCOIN, 
                                    (*p)->dollars, 
                                    (*p)->last_day_dollars, 
                                    (*p)->balance, 
                                    ((*p)->last_day_satoshi > (int64_t) 0) ? ";+" : ";",
                                    (double) (*p)->last_day_satoshi / (double) BB_SATOSHI_PER_BITCOIN, 
                                    last_day_rate) < 0)
                            err_code = 5;

                        first_str = 0;
                    }*/
                }

            fclose(f);
        }
    }

    if (err_code && perr)
        bg_chart_set_error(err_code, perr);
    return err_code;
}

void bb_free_chart_data(struct bb_chart_data *pcd)
{
    struct bb_balance_data **p, **p2;

    if (pcd)
    {
        if (pcd->balances)
        {
            for (p = pcd->balances, p2 = p + pcd->balances_n; p < p2; ++p)
                free(*p);
            free(pcd->balances);
        }

        memset(pcd, 0, sizeof(struct bb_chart_data));
    }
}

const char * bb_chart_error_message(int err_code)
{
    static const char *BB_CHART_ERRS[BB_CHART_ERRS_N] = {   "ОК",                                                       //  0
                                                            "Ошибка выделения памяти",                                  //  1
                                                            "Неизвестная ошибка",                                       //  2
                                                            "Пустой указатель",                                         //  3
                                                            "Ошибка при открытии файла для записи",                     //  4
                                                            "Ошибка записи в файл",                                     //  5
                                                            "Недопустимый тип баланса",                                 //  6
    };

    if ((err_code < 0) || (err_code >= BB_CHART_ERRS_N))
        err_code = 2;

    return *(BB_CHART_ERRS + err_code);
}

static void bg_chart_set_error(int err_code, struct bb_error *perr)
{
    perr->err_code = err_code;
    perr->err_str = bb_chart_error_message(err_code);
}