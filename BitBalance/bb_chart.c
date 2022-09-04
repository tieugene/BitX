//#include <stdlib.h>
//#include <stdio.h>
#include <string.h>
#include <math.h>
#include "bb_chart.h"
#include "bb_png.h"

#ifdef BB_WIN
#include <glib.h>
#include <glib/gstdio.h>
#else
#include <stdio.h>
#include <stdlib.h>
#endif

#define BB_CHART_ERRS_N 7                   // Number of errors.

#define BB_BORDER 10
#define BB_X_PAD 2
#define BB_Y_PAD 5
#define BB_CHART_H 400
#define BB_LINE_W 2

/* Sets error data. */
static void set_error(int err_code, struct bb_error *perr);

/*  Get max scale devision.  */
static size_t bb_chart_get_scale(double d);

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
        set_error(err_code, perr);
    return err_code;
}

int bb_write_chart_png(const char *file_name, struct bb_chart_data *chart, enum bb_write_modes nbs, enum bb_write_modes wl, 
    struct bb_error *perr)
{
    struct bb_png_data *ppd;
    struct bb_balance_data **p, **p2;
    int division, devision_s_len, balances_n, c_w, c_h, max_balance_s_len, max_address_len, len;
    int width, height, x, y, h, i, err_code, png_err;
    char tmp[64] = {0};
    _Bool d_flag;

    ppd = NULL;
    err_code = 0;

    if (!file_name || !chart)
        err_code = 3;
    else if (((nbs != BB_WRITE_BUY) && (nbs != BB_WRITE_SEL)) || 
            ((wl != BB_WRITE_WIN) && (wl != BB_WRITE_LOS)))
        err_code = 6;
    else
    {
        if (nbs != BB_WRITE_SEL)
        {
            if (wl != BB_WRITE_WIN)
            {
                division = bb_chart_get_scale(chart->min_buy_balance);
                balances_n = chart->los_buy_balances_n;
                max_balance_s_len = chart->min_buy_balance_s_len;
                max_address_len = chart->los_buy_max_address_len;
            }
            else
            {
                division = bb_chart_get_scale(chart->max_buy_balance);
                balances_n = chart->win_buy_balances_n;
                max_balance_s_len = chart->max_buy_balance_s_len;
                max_address_len = chart->win_buy_max_address_len;
            }
        }
        else
        {
            if (wl != BB_WRITE_WIN)
            {
                division = bb_chart_get_scale(chart->min_sel_balance);
                balances_n = chart->los_sel_balances_n;
                max_balance_s_len = chart->min_sel_balance_s_len;
                max_address_len = chart->los_sel_max_address_len;
            }
            else
            {
                division = bb_chart_get_scale(chart->max_sel_balance);
                balances_n = chart->win_sel_balances_n;
                max_balance_s_len = chart->max_sel_balance_s_len;
                max_address_len = chart->win_sel_max_address_len;
            }
        }

        //devision_s_len = bb_get_balance_s_len((wl != BB_WRITE_WIN) ? (double) -division : (double) division);

        d_flag = ((division < 10) && (division != 5));
        if (wl != BB_WRITE_WIN)
            division = -division;
        devision_s_len = bb_get_balance_s_len((double) division) - (!d_flag ? 4 : 2);

        c_w = (BB_X_PAD + BB_LETTER_H) * balances_n + BB_X_PAD + BB_LINE_W;
        width = BB_BORDER * 2 + c_w + devision_s_len * BB_LETTER_W + BB_X_PAD;

        c_h = BB_CHART_H + max_balance_s_len * BB_LETTER_W + BB_Y_PAD + BB_LINE_W;
        height = BB_BORDER * 2 + c_h + max_address_len * BB_LETTER_W + BB_Y_PAD;

        x = BB_BORDER + devision_s_len * BB_LETTER_W + BB_X_PAD;
        y = c_h + BB_BORDER;

        if (!bb_png_init(&ppd, (int) width, (int) height, perr) && 
            !bb_png_draw_rect(ppd, x, BB_BORDER, BB_LINE_W, c_h, 0, perr) && 
            !bb_png_draw_rect(ppd, x, y, c_w, BB_LINE_W, 0, perr) && 
            !bb_png_write_text(ppd, (x - BB_X_PAD - BB_LETTER_W * ((!d_flag) ? 1 : 5)), (y - BB_LETTER_H / 2 + BB_LINE_W), 
                ((!d_flag) ? "0" : "0.000"), ((!d_flag) ? 1 : 5), 0, perr))
        {
            png_err = 0;

            for (i = 0; i < 5; ++i)
            {
                //printf("division %d\n", division);
                if (!d_flag)
                    snprintf(tmp, 64, "%d", division * (5 - i) / 5);
                else
                    snprintf(tmp, 64, "%.3lf", (double) division * (double) (5 - i) / (double) 5);
                len = strlen(tmp);
                png_err = bb_png_write_text(ppd,    (x - BB_X_PAD - BB_LETTER_W * len), 
                                                    (y - BB_LETTER_H / 2 - BB_LINE_W - BB_CHART_H * (5 - i) / 5), tmp, len, 0, perr);
                if (!png_err)
                    png_err = bb_png_draw_rect(ppd, (x + BB_LINE_W), 
                                                    (y - BB_LINE_W - BB_CHART_H * (5 - i) / 5), c_w, BB_LINE_W, 150, perr);
            }

            if (!png_err)
            {
                x += BB_X_PAD + BB_LINE_W;
                for (p = chart->balances, p2 = p + chart->balances_n; !png_err && (p < p2); ++p)
                {
                    if ((((BB_WRITE_WIN == wl) && 
                            //((*p)->balance > +0.0) && 
                            ((*p)->balance > +BB_D) && 
                            (((BB_WRITE_BUY == nbs) && ((*p)->last_day_satoshi > (int64_t) 0)) || 
                                ((BB_WRITE_SEL == nbs) && ((*p)->last_day_satoshi < (int64_t) 0)))) || 
                        ((BB_WRITE_LOS == wl) && 
                            //((*p)->balance < -0.0) && 
                            ((*p)->balance < -BB_D) && 
                            (((BB_WRITE_BUY == nbs) && ((*p)->last_day_satoshi > (int64_t) 0)) || 
                                ((BB_WRITE_SEL == nbs) && ((*p)->last_day_satoshi < (int64_t) 0))))))
                    {
                        h = (int) (fabs((*p)->balance * (double) BB_CHART_H / (double) division));
                        if (!h)
                            ++h;
                        //printf("h %d\n", h);
                        png_err = bb_png_draw_rect(ppd, x, (y - h), BB_LETTER_H, h, (wl != BB_WRITE_LOS) ? 223 : 224, perr);
                        if (!png_err)
                        {
                            png_err = bb_png_write_text(ppd, x, (y + BB_Y_PAD + (*p)->address_len * BB_LETTER_W), 
                                (*p)->address, (*p)->address_len, 1, perr);
                            if (!png_err)
                            {
                                snprintf(tmp, 64, "%.3lf", (*p)->balance);
                                png_err = bb_png_write_text(ppd, x, (y - h - BB_Y_PAD), 
                                    tmp, strlen(tmp), 1, perr);
                                x += BB_X_PAD + BB_LETTER_H;
                            }
                        }
                    }
                }
            }

            if (!png_err)
                bb_png_save(ppd, file_name, perr);
        }
        bb_png_free(&ppd);
    }

    if (err_code && perr)
        set_error(err_code, perr);
    return err_code;
}

void bb_chart_get_n(const struct bb_chart_data *chart, size_t *pwb_n, size_t *pws_n, size_t *plb_n, size_t *pls_n)
{
    struct bb_balance_data **p, **p2;
    size_t wb_n, ws_n, lb_n, ls_n;

    if (chart)
    {
        wb_n = ws_n = lb_n = ls_n = 0;
        for (p = chart->balances, p2 = p + chart->balances_n; p < p2; ++p)
        {
            if ((*p)->balance > +BB_D)
            {
                if ((*p)->last_day_satoshi > (int64_t) 0)       // buy
                    ++wb_n;
                else if ((*p)->last_day_satoshi < (int64_t) 0)  // sell
                    ++ws_n;
            }
            else if ((*p)->balance < -BB_D)
            {
                if ((*p)->last_day_satoshi > (int64_t) 0)       // buy
                    ++lb_n;
                else if ((*p)->last_day_satoshi < (int64_t) 0)  // sell
                    ++ls_n;
            }
        }

        if (pwb_n)
            *pwb_n = wb_n;
        if (pws_n)
            *pws_n = ws_n;
        if (plb_n)
            *plb_n = lb_n;
        if (pls_n)
            *pls_n = ls_n;
    }
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

static size_t bb_chart_get_scale(double d)
{
    size_t t, k, m, n, division;
    _Bool flag;

    n = ceil(fabs(d));

    if (n <= 10)
        division = n;
    else
        for (division = 10, m = 5, k = 50, flag = 0; division < n; division += m)
            if (division >= k)
            {
                t = !flag ? 2 : 5;
                flag = !flag;
                k *= t;
                m *= t;
            }

    return division;
}

static void set_error(int err_code, struct bb_error *perr)
{
    perr->err_code = err_code;
    perr->err_str = bb_chart_error_message(err_code);
}