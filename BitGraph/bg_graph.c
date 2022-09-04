#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "bb_chart.h"
#include "bg_gl.h"
#include "bg_png.h"
#include "bg_svg.h"
#include "bg_graph.h"

#define BB_GRAPH_ERRS_N 5                   // Number of errors.

#define BB_D 0.0001

#define BG_RATE_D_SIZE 64

static const unsigned char BG_G_BW_COLOR[3] = {BG_G_BW_R, BG_G_BW_G, BG_G_BW_B};
static const unsigned char BG_G_SL_COLOR[3] = {BG_G_SL_R, BG_G_SL_G, BG_G_SL_B};
static const unsigned char BG_G_BC_COLOR[3] = {BG_G_BC_R, BG_G_BC_G, BG_G_BC_B};

#define BG_G_DESC_N 15
static const char *BG_G_DESC_EN[BG_G_DESC_N] = {"Percentage number of buyers and sellers of BTC from their sum range",                  //  1
                                                "The number of winners and the number of losers as a percentage of their sum range",    //  2
                                                "Number of buyers and number of sellers, absolute value range",                         //  3
                                                "Number of Winners and Number of Losers, absolute value range",                         //  4
                                                "The percentage of winners and losers (relative to their sum) among BTC buyers range",  //  5
                                                "The percentage of winners and losers (relative to their sum) among BTC sellers range", //  6
                                                "Percentage of buyers and sellers extracted from winners range",                        //  7
                                                "Percentage of buyers and sellers extracted from losers range",                         //  8
                                                "Total dollars bought and sold range",                                                  //  9
                                                "Total number of bought and sold bitcoins range",                                       // 10
                                                "The number of dollars bought and sold by winners range",                               // 11
                                                "The number of dollars bought and sold by losers range",                                // 12
                                                "The number of bought and sold bitcoins by winners range",                              // 13
                                                "The number of bought and sold bitcoins by losers range",                               // 14
                                                "BTC price change as a percentage relative to the previous day"};                       // 15

                                            //   1   2   3   4   5   6   7   8   9  10  11  12  13  14  15
static size_t BG_G_DESC_EN_SLEN[BG_G_DESC_N] = {67, 81, 60, 60, 83, 84, 61, 60, 35, 46, 54, 53, 55, 54, 61};

static const char *BG_G_DESC_RU[BG_G_DESC_N] = {"Количество покупателей и продавцов BTC в процентах от их суммы в диапазоне",           //  1
                                                "Количество виннеров и количество лузеров в процентах от их суммы в диапазоне",         //  2
                                                "Количество покупателей и количество продавцов, абсолютное значение в диапазоне",       //  3
                                                "Количество виннеров и количество лузеров, абсолютное значение в диапазоне",            //  4
                                                "Процент виннеров и лузеров (относительно их суммы) среди покупателей BTC в диапазоне", //  5
                                                "Процент виннеров и лузеров (относительно их суммы) среди продавцов BTC в диапазоне",   //  6
                                                "Процент покупателей и продавцов извлеченных из удачников с балансом в диапазоне",      //  7
                                                "Процент покупателей и продавцов извлеченных из лузеров с балансом в диапазоне",        //  8
                                                "Общее количество купленных и проданных долларов в диапазоне",                          //  9
                                                "Общее количество купленных и проданных биткоинов в диапазоне",                         // 10
                                                "Количество купленных и проданных долларов виннерами в диапазоне",                      // 11
                                                "Количество купленных и проданных долларов лузерами в диапазоне",                       // 12
                                                "Количество купленных и проданных биткоинов виннерами в диапазоне",                     // 13
                                                "Количество купленных и проданных биткоинов лузерами в диапазоне",                      // 14
                                                "Изменение цены БТЦ в процентах относительно предыдущего дня"};                         // 15

                                            //   1   2   3   4   5   6   7   8   9  10  11  12  13  14  15
static size_t BG_G_DESC_RU_SLEN[BG_G_DESC_N] = {74, 76, 78, 73, 84, 82, 79, 77, 59, 60, 63, 62, 64, 63, 59};

                                            //    1    2    3    4    5    6    7    8    9   10   11   12   13   14   15
static size_t BG_G_DESC_RU_CLEN[BG_G_DESC_N] = {134, 141, 147, 137, 152, 148, 148, 144, 111, 113, 119, 117, 121, 119, 111};

#define BG_G_RANGE_S_N 11
static const char *BG_G_RANGE_S[BG_G_RANGE_S_N] = { "0-0,001 BTC",      //  1
                                                    "0,001-0,01 BTC",   //  2
                                                    "0,01-0,1 BTC",     //  3
                                                    "0,1-1 BTC",        //  4
                                                    "1-10 BTC",         //  5
                                                    "10-100 BTC",       //  6
                                                    "100-1k BTC",       //  7
                                                    "1k-10k BTC",       //  8
                                                    "10k-100k BTC",     //  9
                                                    "100k-1m BTC",      // 10
                                                    "1m-21m BTC"};      // 11

                                                //   1   2   3   4   5   6   7   8   9  10  11
static size_t BG_G_RANGE_S_SLEN[BG_G_RANGE_S_N] = { 11, 14, 12,  9,  8, 10, 10, 10, 12, 11, 10};

struct bg_graph_rates_data {
    char min_val_str[64];                   // Min val string.
    char max_val_str[64];                   // Max val string.
    double *rates;                          // Rates.
    double *values;                         // Rates values.
    time_t *dates;                          // Dates.
    double min_val;                         // Min value.
    double max_val;                         // Max value.
    double division;                        // Division.
    size_t points_n;                        // Number of points.
    size_t max_rate_len;                    // Max rate string length.
    size_t min_val_str_len;                 // Min val string length.
    size_t max_val_str_len;                 // Max val string length.
    size_t max_perc_len;                    // Max percentage string length.
    int rid;                                // Range id.
};

struct bg_graph_data {
    const char *path;                       // Path to save file.
    double *g_points;                       // Data for green line.
    double *r_points;                       // Data for red line.
    size_t title_n;                         // Number of title.
    int is_abs;                             // 1 - abs_buy_sel, 2 - abs_win_los.
    _Bool is_perc;                          // 1 - percentage, 0 - not.
    _Bool wl;                               // 1 - win/lose, 0 - buy/sell.
    _Bool is_btc;                           // 1 - bitcoins, 0 - dollars.
};

/*  Draws and saves graph to png.  */
static void bg_graph_get_graph(const struct bg_graph_data *pgd, struct bg_graph_rates_data *prd, const char *path, struct bb_error *perr);

/*  Draws and saves graph to svg.  */
static void bg_graph_get_svg_graph(const struct bg_graph_data *pgd, struct bg_graph_rates_data *prd, const char *path, struct bb_error *perr);

/*  Draws and saves rates graph to svg.  */
static void bg_graph_save_rates_graph_svg(struct bg_graph_rates_data *prd, const char *path, struct bb_error *perr);

/*  Draws legend for svg.  */
static void bg_graph_svg_draw_legend(struct bg_svg *pbs, int x, int y, _Bool wl, struct bb_error *perr);

/*  Draws rates legend for svg.  */
static void bg_graph_svg_draw_r_legend(struct bg_svg *pbs, int x, int y, struct bb_error *perr);

/*  Gets max values of list.  */
static void bg_graph_get_n(const struct bg_graph_data *pgd, const struct bg_graph_rates_data *prd, 
    size_t *pmax_perc_len, size_t *pmax_rate_len, size_t *pdivision, int *pkm);

/*  Returns value string length.  */
static size_t bb_graph_get_s_len(double value);

/*  Gets max scale devision.  */
static size_t bb_graph_get_scale(double d);

/*  Gets parameters for rates graph.  */
static void bg_graph_get_rates_parameters(struct bg_graph_rates_data *prd);

/*  Gets scale values for rates graph.  */
static void bg_graph_get_rate_values(double *pmin_val, char min_val_str[BG_RATE_D_SIZE], double *pmax_val, char max_val_str[BG_RATE_D_SIZE]);

/* Sets error data. */
static void bg_graph_set_error(int err_code, struct bb_error *perr);

int bg_graph_rates_data_init(const struct bg_data_node *pd, size_t n, struct bg_graph_rates_data **pprd, int rid, struct bb_error *perr)
{
    const struct bg_data_node *node;
    double *prt;
    time_t *pt;
    size_t i;
    int err_code;

    err_code = 0;

    if (!pd || !n || !pprd)
        err_code = 3;
    else
    {
        *pprd = (struct bg_graph_rates_data *) malloc(sizeof(struct bg_graph_rates_data) + 2 * n * sizeof(double) + n * sizeof(time_t));
        if (!*pprd)
            err_code = 1;
        else
        {
            memset(*pprd, 0, sizeof(struct bg_graph_rates_data));

            (*pprd)->rates = (double *) (*pprd + 1);
            (*pprd)->values = (*pprd)->rates + n;
            memset((*pprd)->rates, 0, 2 * n * sizeof(double));

            (*pprd)->dates = (time_t *) ((*pprd)->values + n);
            memset((*pprd)->dates, 0, n * sizeof(time_t));

            (*pprd)->points_n = n;
            (*pprd)->rid = rid - 1;     // - 1 !!!

            for (node = pd, pt = (*pprd)->dates, prt = (*pprd)->rates, i = 0; node && (i < n); node = node->next, ++i)
            {
                *pt++ = node->date;
                *prt++ = node->rate;
            }

            bg_graph_get_rates_parameters(*pprd);
        }
    }

    if (err_code && perr)
        bg_graph_set_error(err_code, perr);
    return err_code;
}

void bg_graph_rates_data_deinit(struct bg_graph_rates_data **pprd)
{
    if (pprd && *pprd)
    {
        free(*pprd);
        *pprd = NULL;
    }
}

int bg_graph_save_graph(const struct bg_data_node *pd, struct bg_graph_rates_data *prd, size_t numb, const char *path, _Bool save_png, 
    struct bb_error *perr)
{
    struct bg_graph_data *pgd;
    const struct bg_data_node *node;
    double *pg, *pr;
    size_t i, m;
    int err_code;

    err_code = 0;

    if (!pd || !prd || !path)
        err_code = 3;
    else
    {
        pgd = (struct bg_graph_data *) malloc(sizeof(struct bg_graph_data) + 2 * prd->points_n * sizeof(double));
        if (!pgd)
            err_code = 1;
        else
        {
            pgd->path = path;

            pgd->g_points = (double *) (pgd + 1);
            pgd->r_points = pgd->g_points + prd->points_n;
            memset(pgd->g_points, 0, 2 * prd->points_n * sizeof(double));

            pgd->title_n = numb;

            node = pd;
            pg = pgd->g_points;
            pr = pgd->r_points;

            switch (numb)
            {
                case 0:
                    for (i = 0; node && (i < prd->points_n); node = node->next, ++i, ++pg, ++pr)
                    {
                        m = node->buy_n + node->sel_n;
                        *pg = (double) (node->buy_n * 100) / (double) m;
                        *pr = (double) (node->sel_n * 100) / (double) m;
                    }
                    pgd->is_perc = 1;
                    pgd->wl = 0;
                    pgd->is_btc = 0;
                    pgd->is_abs = 0;
                    break;

                case 1:
                    for (i = 0; node && (i < prd->points_n); node = node->next, ++i, ++pg, ++pr)
                    {
                        m = node->win_n + node->los_n;
                        *pg = (double) (node->win_n * 100) / (double) m;
                        *pr = (double) (node->los_n * 100) / (double) m;
                    }
                    pgd->is_perc = 1;
                    pgd->wl = 1;
                    pgd->is_btc = 0;
                    pgd->is_abs = 0;
                    break;

                case 2:
                    for (i = 0; node && (i < prd->points_n); node = node->next, ++i, ++pg, ++pr)
                    {
                        *pg = (double) node->buy_n;
                        *pr = (double) node->sel_n;
                    }
                    pgd->is_perc = 0;
                    pgd->wl = 0;
                    pgd->is_btc = 0;
                    pgd->is_abs = 1;
                    break;

                case 3:
                    for (i = 0; node && (i < prd->points_n); node = node->next, ++i, ++pg, ++pr)
                    {
                        *pg = (double) node->win_n;
                        *pr = (double) node->los_n;
                    }
                    pgd->is_perc = 0;
                    pgd->wl = 1;
                    pgd->is_btc = 0;
                    pgd->is_abs = 2;
                    break;

                case 4:
                    for (i = 0; node && (i < prd->points_n); node = node->next, ++i, ++pg, ++pr)
                    {
                        m = node->win_buy_n + node->los_buy_n;
                        *pg = (double) (node->win_buy_n * 100) / (double) m;
                        *pr = (double) (node->los_buy_n * 100) / (double) m;
                    }
                    pgd->is_perc = 1;
                    pgd->wl = 1;
                    pgd->is_btc = 0;
                    pgd->is_abs = 0;
                    break;

                case 5:
                    for (i = 0; node && (i < prd->points_n); node = node->next, ++i, ++pg, ++pr)
                    {
                        m = node->win_sel_n + node->los_sel_n;
                        *pg = (double) (node->win_sel_n * 100) / (double) m;
                        *pr = (double) (node->los_sel_n * 100) / (double) m;
                    }
                    pgd->is_perc = 1;
                    pgd->wl = 1;
                    pgd->is_btc = 0;
                    pgd->is_abs = 0;
                    break;

                case 6:
                    for (i = 0; node && (i < prd->points_n); node = node->next, ++i, ++pg, ++pr)
                    {
                        m = node->win_buy_n + node->win_sel_n;
                        *pg = (double) (node->win_buy_n * 100) / (double) m;
                        *pr = (double) (node->win_sel_n * 100) / (double) m;
                    }
                    pgd->is_perc = 1;
                    pgd->wl = 0;
                    pgd->is_btc = 0;
                    pgd->is_abs = 0;
                    break;

                case 7:
                    for (i = 0; node && (i < prd->points_n); node = node->next, ++i, ++pg, ++pr)
                    {
                        m = node->los_buy_n + node->los_sel_n;
                        *pg = (double) (node->los_buy_n * 100) / (double) m;
                        *pr = (double) (node->los_sel_n * 100) / (double) m;
                    }
                    pgd->is_perc = 1;
                    pgd->wl = 0;
                    pgd->is_btc = 0;
                    pgd->is_abs = 0;
                    break;

                case 8:
                    for (i = 0; node && (i < prd->points_n); node = node->next, ++i, ++pg, ++pr)
                    {
                        *pg = (double) node->dol_buy_n / (double) 1000;
                        *pr = (double) node->dol_sel_n / (double) 1000;
                    }
                    pgd->is_perc = 0;
                    pgd->wl = 0;
                    pgd->is_btc = 0;
                    pgd->is_abs = 0;
                    break;

                case 9:
                    for (i = 0; node && (i < prd->points_n); node = node->next, ++i, ++pg, ++pr)
                    {
                        *pg = (double) node->btc_buy_n / (double) BB_SATOSHI_PER_BITCOIN;
                        *pr = (double) node->btc_sel_n / (double) BB_SATOSHI_PER_BITCOIN;
                    }
                    pgd->is_perc = 0;
                    pgd->wl = 0;
                    pgd->is_btc = 1;
                    pgd->is_abs = 0;
                    break;

                case 10:
                    for (i = 0; node && (i < prd->points_n); node = node->next, ++i, ++pg, ++pr)
                    {
                        *pg = (double) node->win_dol_buy_n / (double) 1000;
                        *pr = (double) node->win_dol_sel_n / (double) 1000;
                    }
                    pgd->is_perc = 0;
                    pgd->wl = 0;
                    pgd->is_btc = 0;
                    pgd->is_abs = 0;
                    break;

                case 11:
                    for (i = 0; node && (i < prd->points_n); node = node->next, ++i, ++pg, ++pr)
                    {
                        *pg = (double) node->los_dol_buy_n / (double) 1000;
                        *pr = (double) node->los_dol_sel_n / (double) 1000;
                    }
                    pgd->is_perc = 0;
                    pgd->wl = 0;
                    pgd->is_btc = 0;
                    pgd->is_abs = 0;
                    break;

                case 12:
                    for (i = 0; node && (i < prd->points_n); node = node->next, ++i, ++pg, ++pr)
                    {
                        *pg = (double) node->win_btc_buy_n / (double) BB_SATOSHI_PER_BITCOIN;
                        *pr = (double) node->win_btc_sel_n / (double) BB_SATOSHI_PER_BITCOIN;
                    }
                    pgd->is_perc = 0;
                    pgd->wl = 0;
                    pgd->is_btc = 1;
                    pgd->is_abs = 0;
                    break;

                case 13:
                    for (i = 0; node && (i < prd->points_n); node = node->next, ++i, ++pg, ++pr)
                    {
                        *pg = (double) node->los_btc_buy_n / (double) BB_SATOSHI_PER_BITCOIN;
                        *pr = (double) node->los_btc_sel_n / (double) BB_SATOSHI_PER_BITCOIN;
                    }
                    pgd->is_perc = 0;
                    pgd->wl = 0;
                    pgd->is_btc = 1;
                    pgd->is_abs = 0;
                    break;

                default:
                    err_code = 4;
            }

            if (!err_code && save_png)
                bg_graph_get_graph(pgd, prd, path, perr);

            if (!perr || !perr->err_code)
                bg_graph_get_svg_graph(pgd, prd, path, perr);

            free(pgd);
        }
    }

    if (err_code && perr)
        bg_graph_set_error(err_code, perr);
    return err_code;
}

static void bg_graph_get_graph(const struct bg_graph_data *pgd, struct bg_graph_rates_data *prd, const char *path, struct bb_error *perr)
{
    struct bb_graph_png *pgp;
    struct tm ttm;
    char tmp[64] = {0};
    unsigned char color_1[3] = {0, 0, 0};
    unsigned char color_2[3] = {172, 172, 172};
    unsigned char color_3[3] = {0x00, 0x9d, 0xcf};
    unsigned char color_4[3] = {100, 100, 100};
    size_t i, w, h, sw, gw, gh, max_perc_len, max_rate_len, division, len, yw, yh, drw, dew;
    int x, x1, y, y1, y2, y_r, km;
    _Bool d_flag;

    if (pgd && path && perr)
    {
        pgp = NULL;
        km = 0;

        bg_graph_get_n(pgd, prd, &max_perc_len, &max_rate_len, &division, &km);
        d_flag = ((division < 10) && (division != 5));
        if (d_flag)
            max_perc_len += 4;  // .3

        if (pgd->is_perc)
        {
            yw = BG_YT0_W;
            yh = BG_YT0_H;
        }
        else if (pgd->is_abs)
        {
            if (1 == pgd->is_abs)
            {
                yw = BG_YT3_W;
                yh = BG_YT3_H;
            }
            else
            {
                yw = BG_YT4_W;
                yh = BG_YT4_H;
            }
        }
        else if (!pgd->is_btc)
        {
            yw = BG_YT1_W;
            yh = BG_YT1_H;
        }
        else
        {
            yw = BG_YT2_W;
            yh = BG_YT2_H;
        }

        if (yw < (max_perc_len * BB_GL_W))
            yw = (max_perc_len * BB_GL_W);

        sw = max_rate_len * BB_GL_W + (BB_GRAPH_BORDER * 4);
        //gw = BB_GRAPH_PAD_X * 2 + (sw * prd->points_n);
        gw = sw * prd->points_n;
        //x = (BG_G_LEGEND_W + BB_GT_P2 + BG_GTTL_MAX_W) - ((max_perc_len + prd->max_perc_len) * BB_GL_W + BB_GRAPH_PAD_X * 2 + BB_GRAPH_PNG_LW * 2 + gw);
        //x = (BG_G_LEGEND_W + BB_GT_P2 + BG_GTTL_MAX_W) - (yw + (prd->max_perc_len * BB_GL_W) + (BB_GRAPH_PAD_X * 2) + (BB_GRAPH_PNG_LW * 2) + gw);
        x = BG_G_LEGEND_W - (yw + (prd->max_perc_len * BB_GL_W) + (BB_GRAPH_PAD_X * 2) + (BB_GRAPH_PNG_LW * 2) + gw);
        if (x > 0)
            gw += (size_t) x;
        //if (gw < BG_G_LEGEND_W)
        //    gw = BG_G_LEGEND_W;
        //w = (BB_GRAPH_BORDER * 2) + (max_perc_len * BB_GL_W) + BB_GRAPH_PAD_X + BB_GRAPH_PNG_LW + gw;
        //w = (BB_GRAPH_BORDER * 2) + ((max_perc_len + prd->max_perc_len) * BB_GL_W) + (BB_GRAPH_PAD_X * 2) + (BB_GRAPH_PNG_LW * 2) + gw;
        w = (BB_GRAPH_BORDER * 2) + yw + (prd->max_perc_len * BB_GL_W) + (BB_GRAPH_PAD_X * 2) + (BB_GRAPH_PNG_LW * 2) + gw;
        //if (w < (BG_G_LEGEND_W + BB_GT_P2 + BG_GTTL_MAX_W + (BB_GRAPH_BORDER * 2)))
        //    w = BG_G_LEGEND_W + BB_GT_P2 + BG_GTTL_MAX_W + (BB_GRAPH_BORDER * 2);
        dew = ((*(BG_G_DESC_EN_SLEN + pgd->title_n) + 1 + *(BG_G_RANGE_S_SLEN + prd->rid)) * BG_GL_W);
        if (w < (dew + (BB_GRAPH_BORDER * 2)))
            w = (dew + (BB_GRAPH_BORDER * 2));
        drw = ((*(BG_G_DESC_RU_SLEN + pgd->title_n) + 1 + *(BG_G_RANGE_S_SLEN + prd->rid)) * BG_GL_W);
        if (w < (drw + (BB_GRAPH_BORDER * 2)))
            w = (drw + (BB_GRAPH_BORDER * 2));

        //gh = BB_GL_H + BB_GRAPH_BORDER + BB_GH;
        gh = yh + BB_GL_H + BB_GRAPH_BORDER + BB_GH;
        h = (BB_GRAPH_BORDER * 4) + gh + BB_GRAPH_PNG_LW + BB_GRAPH_PAD_Y * 2 + BB_GL_H + BG_G_LEGEND_H + BG_GL_H * 2;
        //h = (BB_GRAPH_BORDER * 3) + gh + BB_GRAPH_PNG_LW + BB_GRAPH_PAD_Y + BB_GL_H + BG_G_LEGEND_H;
        //h = (BB_GRAPH_BORDER * 4) + gh + BB_GRAPH_PNG_LW * 2 + BB_GRAPH_PAD_Y + (BB_GL_H * 2) + BG_G_LEGEND_H;

        x1 = BB_GRAPH_BORDER + yw + BB_GRAPH_PAD_X;
        y = gh + BB_GRAPH_BORDER;
        //y_r = BB_GRAPH_BORDER * 2 + BB_GL_H + (prd->max_val / prd->division) * BB_GH;
        y_r = BB_GRAPH_BORDER * 2 + BB_GL_H + yh + (prd->max_val / prd->division) * BB_GH;

        if (!bb_graph_png_init(&pgp, w, h, perr) && 
            !bb_graph_png_draw_rect(pgp,    (BB_GRAPH_BORDER + yw + BB_GRAPH_PAD_X), 
                                            BB_GRAPH_BORDER, BB_GRAPH_PNG_LW, gh + BB_GRAPH_PNG_LW, color_1, perr) && 
            !bb_graph_png_draw_rect(pgp,    (BB_GRAPH_BORDER + yw + BB_GRAPH_PAD_X + BB_GRAPH_PNG_LW + gw), 
                                            BB_GRAPH_BORDER, BB_GRAPH_PNG_LW, gh + BB_GRAPH_PNG_LW, color_1, perr) && 
            !bb_graph_png_draw_rect(pgp,    (BB_GRAPH_BORDER + yw + BB_GRAPH_PAD_X + BB_GRAPH_PNG_LW), 
                                            (BB_GRAPH_BORDER + gh), gw, BB_GRAPH_PNG_LW, color_1, perr) && 
            //!bb_graph_png_draw_rect(pgp,    (BB_GRAPH_BORDER + (max_perc_len * BB_GL_W) + BB_GRAPH_PAD_X + BB_GRAPH_PNG_LW), 
            //                                y_r, gw, BB_GRAPH_PNG_LW, color_4, perr) && 
            !bg_graph_png_draw_legend(pgp,  //(BB_GRAPH_BORDER + (max_perc_len * BB_GL_W) + BB_GRAPH_PAD_X + BB_GRAPH_PNG_LW + 
                                            //((gw - BG_G_LEGEND_W) / 2)), 
                                            BB_GRAPH_BORDER, 
                                            ((BB_GRAPH_BORDER * 2) + gh + BB_GRAPH_PNG_LW + BB_GRAPH_PAD_Y + BB_GL_H), pgd->wl, perr) && 
            !bb_graph_png_write_text(pgp,   (BB_GRAPH_BORDER + yw - (!d_flag ? 1 : 5) * BB_GL_W), 
                                            (BB_GRAPH_BORDER + gh - (BB_GL_H / 2) + (BB_GRAPH_PNG_LW / 2)), 
                                            (!d_flag ? "0" : "0.000"), (!d_flag ? 1 : 5), color_1, 0, perr) && 
            !bb_graph_png_write_text(pgp,   (BB_GRAPH_BORDER + yw + (BB_GRAPH_PAD_X * 2) + (BB_GRAPH_PNG_LW * 2) + gw), 
                                            (BB_GRAPH_BORDER + gh - (BB_GL_H / 2) + (BB_GRAPH_PNG_LW / 2)), 
                                            prd->min_val_str, prd->min_val_str_len, color_1, 0, perr) && 
            !bb_graph_png_write_text(pgp,   (BB_GRAPH_BORDER + yw + (BB_GRAPH_PAD_X * 2) + (BB_GRAPH_PNG_LW * 2) + gw), 
                                            (BB_GRAPH_BORDER * 2 + BB_GL_H - (BB_GL_H / 2) + (BB_GRAPH_PNG_LW / 2) + yh), 
                                            prd->max_val_str, prd->max_val_str_len, color_1, 0, perr) && 
            !bg_graph_png_write_text(pgp,   (w - dew) / 2, 
                                            ((BB_GRAPH_BORDER * 3) + gh + BB_GRAPH_PNG_LW + BB_GRAPH_PAD_Y + BB_GL_H + BG_G_LEGEND_H), 
                                            *(BG_G_DESC_EN + pgd->title_n), *(BG_G_DESC_EN_SLEN + pgd->title_n), color_1, perr) && 
            !bg_graph_png_write_text(pgp,   (w - dew) / 2 + (*(BG_G_DESC_EN_SLEN + pgd->title_n) + 1) * BG_GL_W, 
                                            ((BB_GRAPH_BORDER * 3) + gh + BB_GRAPH_PNG_LW + BB_GRAPH_PAD_Y + BB_GL_H + BG_G_LEGEND_H), 
                                            *(BG_G_RANGE_S + prd->rid), *(BG_G_RANGE_S_SLEN + prd->rid), color_1, perr) && 
            !bg_graph_png_write_text(pgp,   (w - drw) / 2, 
                                            ((BB_GRAPH_BORDER * 3) + gh + BB_GRAPH_PNG_LW + (BB_GRAPH_PAD_Y * 2) + BB_GL_H + BG_G_LEGEND_H + BG_GL_H), 
                                            *(BG_G_DESC_RU + pgd->title_n), *(BG_G_DESC_RU_CLEN + pgd->title_n), color_1, perr) && 
            !bg_graph_png_write_text(pgp,   (w - drw) / 2 + (*(BG_G_DESC_RU_SLEN + pgd->title_n) + 1) * BG_GL_W, 
                                            ((BB_GRAPH_BORDER * 3) + gh + BB_GRAPH_PNG_LW + (BB_GRAPH_PAD_Y * 2) + BB_GL_H + BG_G_LEGEND_H + BG_GL_H), 
                                            *(BG_G_RANGE_S + prd->rid), *(BG_G_RANGE_S_SLEN + prd->rid), color_1, perr))
        {
            bb_graph_png_draw_yt(pgp, BB_GRAPH_BORDER + yw, BB_GRAPH_BORDER, pgd->is_perc, pgd->is_btc, pgd->is_abs, color_1);

            for (i = 0; (i < 5) && !perr->err_code; ++i)
            {
                if (pgd->is_perc)
                    snprintf(tmp, 64, "%zu%%", division * (5 - i) / 5);
                else if (!d_flag)
                {
                    if (!km)
                        snprintf(tmp, 64, "%zu", division * (5 - i) / 5);
                    else if (1 == km)
                        snprintf(tmp, 64, "%zuk", (division * (5 - i) / 5) / 1000);
                    else
                        snprintf(tmp, 64, "%zum", (division * (5 - i) / 5) / 1000000);
                }
                else
                    snprintf(tmp, 64, "%.3lf", (double) division * (double) (5 - i) / (double) 5);

                len = strlen(tmp);
                bb_graph_png_write_text(pgp,    (x1 - BB_GRAPH_PAD_X - BB_GL_W * len), 
                                                (y - BB_GL_H / 2 + BB_GRAPH_PNG_LW / 2 - BB_GH * (5 - i) / 5), 
                                                tmp, len, color_1, 0, perr);
                if (!perr->err_code)
                    bb_graph_png_draw_rect(pgp, (x1 + BB_GRAPH_PNG_LW), 
                                                (y - BB_GH * (5 - i) / 5), gw, BB_GRAPH_PNG_LW, color_2, perr);
            }

            if (!perr->err_code)
                bb_graph_png_draw_rect(pgp,    (BB_GRAPH_BORDER + yw + BB_GRAPH_PAD_X + BB_GRAPH_PNG_LW), 
                                            y_r, gw, BB_GRAPH_PNG_LW, color_4, perr);

            if (!perr->err_code && 
                ((BB_GRAPH_BORDER + gh - (BB_GL_H / 2) + (BB_GRAPH_PNG_LW / 2)) > (y_r + BB_GL_H / 2 + BB_GRAPH_PNG_LW / 2 + BB_GRAPH_PAD_Y)) && 
                ((BB_GRAPH_BORDER * 2 + BB_GL_H + (BB_GL_H / 2) + (BB_GRAPH_PNG_LW / 2) + yh) < (y_r - BB_GL_H / 2 + BB_GRAPH_PNG_LW / 2 - BB_GRAPH_PAD_Y)))
                bb_graph_png_write_text(pgp,    (BB_GRAPH_BORDER + yw + (BB_GRAPH_PAD_X * 2) + (BB_GRAPH_PNG_LW * 2) + gw), 
                                                (y_r - (BB_GL_H / 2) + (BB_GRAPH_PNG_LW / 2)), 
                                                "0", 1, color_1, 0, perr);

            if (!perr->err_code)
            {
                x1 += sw / 2;
                for (x = x1, i = 0; (i < prd->points_n) && !perr->err_code; ++i, x += sw)
                {
                    memset(&ttm, 0, sizeof(struct tm));
#ifdef BB_WIN
                    gmtime_s(&ttm, prd->dates + i);
#else
                    gmtime_r(prd->dates + i, &ttm);
#endif
                    //snprintf(tmp, 64, "%02d.%02d.%04d", ttm.tm_mday, ttm.tm_mon + 1, ttm.tm_year + 1900);
                    snprintf(tmp, 64, "%02d.%02d", ttm.tm_mday, ttm.tm_mon + 1);
                    len = strlen(tmp);
                    bb_graph_png_write_text(pgp,    (x - (BB_GL_W * len) / 2), 
                                                    BB_GRAPH_BORDER + gh + BB_GRAPH_PNG_LW + BB_GRAPH_PAD_Y, 
                                                    tmp, len, color_1, 0, perr);

                    snprintf(tmp, 64, "%.3lf", *(prd->rates + i));
                    len = strlen(tmp);
                    bb_graph_png_write_text(pgp,    (x - (BB_GL_W * len) / 2), 
                                                    BB_GRAPH_BORDER + yh / 2, 
                                                    tmp, len, color_3, 0, perr);

                }

                for (x = x1, i = 0; (i < prd->points_n) && !perr->err_code; ++i, x += sw)
                {
                    y1 = (y_r - BB_GH * *(prd->values + i) / prd->division);
                    if ((i + 1) < prd->points_n)
                    {
                        y2 = (y_r - BB_GH * *(prd->values + i + 1) / prd->division);
                        bb_graph_png_draw_line(pgp, x, y1, (x + sw), y2, BG_G_BC_COLOR, perr);
                    }

                    if (!perr->err_code)
                        bb_graph_png_draw_point(pgp, x, y1, BG_G_BC_COLOR, perr);
                }

                for (x = x1, i = 0; (i < prd->points_n) && !perr->err_code; ++i, x += sw)
                {
                    y1 = (y - BB_GH * *(pgd->g_points + i) / (double) division);
                    if ((i + 1) < prd->points_n)
                    {
                        y2 = (y - BB_GH * *(pgd->g_points + i + 1) / (double) division);
                        bb_graph_png_draw_line(pgp, x, y1, (x + sw), y2, BG_G_BW_COLOR, perr);
                    }

                    if (!perr->err_code)
                        bb_graph_png_draw_point(pgp, x, y1, BG_G_BW_COLOR, perr);
                }

                for (x = x1, i = 0; (i < prd->points_n) && !perr->err_code; ++i, x += sw)
                {
                    y1 = (y - BB_GH * *(pgd->r_points + i) / (double) division);
                    if ((i + 1) < prd->points_n)
                    {
                        y2 = (y - BB_GH * *(pgd->r_points + i + 1) / (double) division);
                        bb_graph_png_draw_line(pgp, x, y1, (x + sw), y2, BG_G_SL_COLOR, perr);
                    }

                    if (!perr->err_code)
                        bb_graph_png_draw_point(pgp, x, y1, BG_G_SL_COLOR, perr);
                }
            }

            //if (!perr->err_code)
            //    bb_graph_png_draw_title(pgp, pgd->title_n, ((BB_GRAPH_BORDER * 2) + gh + BB_GRAPH_PNG_LW + BB_GRAPH_PAD_Y + BB_GL_H), color_1, perr);

            if (!perr->err_code)
                bb_graph_png_save(pgp, path, perr);
        }
        bb_graph_png_free(&pgp);
    }
}

static void bg_graph_get_svg_graph(const struct bg_graph_data *pgd, struct bg_graph_rates_data *prd, const char *path, struct bb_error *perr)
{
    struct bg_svg *pbs;
    struct tm ttm;
    char tmp[64] = {0};
    int ords[8] = {0};
    int *points, *p;
    char *svg_path;
    const char *yt;
    unsigned char color_1[3] = {0, 0, 0};
    unsigned char color_2[3] = {172, 172, 172};
    unsigned char color_3[3] = {0x00, 0x9d, 0xcf};
    unsigned char color_4[3] = {100, 100, 100};
    size_t i, w, h, sw, gw, gh, max_perc_len, max_rate_len, division, len, yw, yh, yt_len, drw, dew;
    int x, x1, y, y_r, km;
    _Bool d_flag;

    if (pgd && path && perr)
    {
        pbs = NULL;
        points = NULL;
        svg_path = NULL;
        km = 0;

        bg_graph_get_n(pgd, prd, &max_perc_len, &max_rate_len, &division, &km);
        d_flag = ((division < 10) && (division != 5));
        if (d_flag)
            max_perc_len += 4;  // .3

        if (pgd->is_perc)
        {
            yw = BG_YT0_W;
            yh = BG_YT0_H;
            yt = "%";
            yt_len = 1;
        }
        else if (pgd->is_abs)
        {
            if (1 == pgd->is_abs)
            {
                yw = BG_YT3_W;
                yh = BG_YT3_H;
                yt = "abs_buy_sel";
                yt_len = 11;
            }
            else
            {
                yw = BG_YT4_W;
                yh = BG_YT4_H;
                yt = "abs_win_los";
                yt_len = 11;
            }
        }
        else if (!pgd->is_btc)
        {
            yw = BG_YT1_W;
            yh = BG_YT1_H;
            yt = "$";
            yt_len = 1;
        }
        else
        {
            yw = BG_YT2_W;
            yh = BG_YT2_H;
            yt = "BTC";
            yt_len = 3;
        }

        if (yw < (max_perc_len * BB_GL_W))
            yw = (max_perc_len * BB_GL_W);

        sw = max_rate_len * BB_GL_W + (BB_GRAPH_BORDER * 4);
        gw = sw * prd->points_n;
        //x = (BG_G_LEGEND_W + BB_GT_P2 + BG_GTTL_MAX_W) - ((max_perc_len + prd->max_perc_len) * BB_GL_W + BB_GRAPH_PAD_X * 2 + BB_GRAPH_PNG_LW * 2 + gw);
        //x = (BG_G_LEGEND_W + BB_GT_P2 + BG_GTTL_MAX_W) - (yw + (prd->max_perc_len * BB_GL_W) + (BB_GRAPH_PAD_X * 2) + (BB_GRAPH_PNG_LW * 2) + gw);
        x = BG_G_LEGEND_W - (yw + (prd->max_perc_len * BB_GL_W) + (BB_GRAPH_PAD_X * 2) + (BB_GRAPH_PNG_LW * 2) + gw);
        if (x > 0)
            gw += (size_t) x;
        //w = (BB_GRAPH_BORDER * 2) + ((max_perc_len + prd->max_perc_len) * BB_GL_W) + (BB_GRAPH_PAD_X * 2) + (BB_GRAPH_PNG_LW * 2) + gw;
        w = (BB_GRAPH_BORDER * 2) + yw + (prd->max_perc_len * BB_GL_W) + (BB_GRAPH_PAD_X * 2) + (BB_GRAPH_PNG_LW * 2) + gw;
        dew = ((*(BG_G_DESC_EN_SLEN + pgd->title_n) + 1 + *(BG_G_RANGE_S_SLEN + prd->rid)) * BG_GL_W);
        if (w < (dew + (BB_GRAPH_BORDER * 2)))
            w = (dew + (BB_GRAPH_BORDER * 2));
        drw = ((*(BG_G_DESC_RU_SLEN + pgd->title_n) + 1 + *(BG_G_RANGE_S_SLEN + prd->rid)) * BG_GL_W);
        if (w < (drw + (BB_GRAPH_BORDER * 2)))
            w = (drw + (BB_GRAPH_BORDER * 2));

        //gh = BB_GL_H + BB_GRAPH_BORDER + BB_GH;
        gh = yh + BB_GL_H + BB_GRAPH_BORDER + BB_GH;
        //h = (BB_GRAPH_BORDER * 3) + gh + BB_GRAPH_PNG_LW + BB_GRAPH_PAD_Y + BB_GL_H + BG_G_LEGEND_H;
        h = (BB_GRAPH_BORDER * 4) + gh + BB_GRAPH_PNG_LW + BB_GRAPH_PAD_Y * 2 + BB_GL_H + BG_G_LEGEND_H + BG_GL_H * 2;

        x1 = BB_GRAPH_BORDER + yw + BB_GRAPH_PAD_X;
        y = gh + BB_GRAPH_BORDER;
        //y_r = BB_GRAPH_BORDER * 2 + BB_GL_H + (prd->max_val / prd->division) * BB_GH;
        y_r = BB_GRAPH_BORDER * 2 + BB_GL_H + yh + (prd->max_val / prd->division) * BB_GH;

        *ords = *(ords + 2) = BB_GRAPH_BORDER + yw + BB_GRAPH_PAD_X;
        *(ords + 1) = *(ords + 7) = BB_GRAPH_BORDER;
        *(ords + 3) = *(ords + 5) = BB_GRAPH_BORDER + gh + BB_GRAPH_PNG_LW / 2;
        *(ords + 4) = *(ords + 6) = *ords + BB_GRAPH_PNG_LW + gw;

        if (!bg_svg_init(&pbs, w, h, perr) && 
            !bg_svg_draw_mpline(pbs, ords, 4, 0, color_1, perr) && 
            !bg_svg_draw_text(pbs,      (BB_GRAPH_BORDER + yw - (!d_flag ? 1 : 5) * BB_GL_W), 
                                        (BB_GRAPH_BORDER + gh - (BB_GL_H / 2) + (BB_GRAPH_PNG_LW / 2)), 
                                        (!d_flag ? "0" : "0.000"), (!d_flag ? 1 : 5), color_1, perr) && 
            !bg_svg_draw_text(pbs,      (BB_GRAPH_BORDER + yw + (BB_GRAPH_PAD_X * 2) + (BB_GRAPH_PNG_LW * 2) + gw), 
                                        (BB_GRAPH_BORDER + gh - (BB_GL_H / 2) + (BB_GRAPH_PNG_LW / 2)), 
                                        prd->min_val_str, prd->min_val_str_len, color_1, perr) && 
            !bg_svg_draw_text(pbs,      (BB_GRAPH_BORDER + yw + (BB_GRAPH_PAD_X * 2) + (BB_GRAPH_PNG_LW * 2) + gw), 
                                        (BB_GRAPH_BORDER * 2 + BB_GL_H - (BB_GL_H / 2) + (BB_GRAPH_PNG_LW / 2) + yh), 
                                        prd->max_val_str, prd->max_val_str_len, color_1, perr) && 
            !bg_svg_draw_text(pbs,      BB_GRAPH_BORDER + yw - yt_len * BB_GL_W, BB_GRAPH_BORDER, yt, yt_len, color_1, perr) && 
            !bg_svg_draw_text(pbs,      (w - dew) / 2, 
                                        ((BB_GRAPH_BORDER * 3) + gh + BB_GRAPH_PNG_LW + BB_GRAPH_PAD_Y + BB_GL_H + BG_G_LEGEND_H) + 2, 
                                        *(BG_G_DESC_EN + pgd->title_n), *(BG_G_DESC_EN_SLEN + pgd->title_n), color_1, perr) && 
            !bg_svg_draw_text(pbs,      (w - dew) / 2 + (*(BG_G_DESC_EN_SLEN + pgd->title_n) + 1) * BG_GL_W, 
                                        ((BB_GRAPH_BORDER * 3) + gh + BB_GRAPH_PNG_LW + BB_GRAPH_PAD_Y + BB_GL_H + BG_G_LEGEND_H) + 2, 
                                        *(BG_G_RANGE_S + prd->rid), *(BG_G_RANGE_S_SLEN + prd->rid), color_1, perr) && 
            !bg_svg_draw_text(pbs,      (w - drw) / 2, 
                                        ((BB_GRAPH_BORDER * 3) + gh + BB_GRAPH_PNG_LW + (BB_GRAPH_PAD_Y * 2) + BB_GL_H + BG_G_LEGEND_H + BG_GL_H) + 2, 
                                        *(BG_G_DESC_RU + pgd->title_n), *(BG_G_DESC_RU_CLEN + pgd->title_n), color_1, perr) && 
            !bg_svg_draw_text(pbs,      (w - drw) / 2 + (*(BG_G_DESC_RU_SLEN + pgd->title_n) + 1) * BG_GL_W, 
                                        ((BB_GRAPH_BORDER * 3) + gh + BB_GRAPH_PNG_LW + (BB_GRAPH_PAD_Y * 2) + BB_GL_H + BG_G_LEGEND_H + BG_GL_H) + 2, 
                                        *(BG_G_RANGE_S + prd->rid), *(BG_G_RANGE_S_SLEN + prd->rid), color_1, perr))
        {
            bg_graph_svg_draw_legend(pbs,   BB_GRAPH_BORDER, 
                                            ((BB_GRAPH_BORDER * 2) + gh + BB_GRAPH_PNG_LW + BB_GRAPH_PAD_Y + BB_GL_H), pgd->wl, perr);

            for (i = 0; (i < 5) && !perr->err_code; ++i)
            {
                if (pgd->is_perc)
                    snprintf(tmp, 64, "%zu%%", division * (5 - i) / 5);
                else if (!d_flag)
                {
                    if (!km)
                        snprintf(tmp, 64, "%zu", division * (5 - i) / 5);
                    else if (1 == km)
                        snprintf(tmp, 64, "%zuk", (division * (5 - i) / 5) / 1000);
                    else
                        snprintf(tmp, 64, "%zum", (division * (5 - i) / 5) / 1000000);
                }
                else
                    snprintf(tmp, 64, "%.3lf", (double) division * (double) (5 - i) / (double) 5);

                len = strlen(tmp);
                bg_svg_draw_text(pbs,   (x1 - BB_GRAPH_PAD_X - BB_GL_W * len), 
                                        (y - BB_GL_H / 2 + BB_GRAPH_PNG_LW / 2 - BB_GH * (5 - i) / 5), 
                                        tmp, len, color_1, perr);
                if (!perr->err_code)
                    //bg_svg_draw_line(pbs,   (x1 + BB_GRAPH_PNG_LW / 2), (y - BB_GH * (5 - i) / 5), 
                    //                        (x1 + BB_GRAPH_PNG_LW / 2 + gw), (y - BB_GH * (5 - i) / 5), color_2, perr);
                    bg_svg_draw_line(pbs,   (x1 + BB_GRAPH_PNG_LW / 2), (y - BB_GH * (5 - i) / 5 + (BB_GRAPH_PNG_LW / 2)), 
                                            (x1 + BB_GRAPH_PNG_LW / 2 + gw), (y - BB_GH * (5 - i) / 5 + (BB_GRAPH_PNG_LW / 2)), 
                                            color_2, perr);
            }

            if (!perr->err_code)
                //bg_svg_draw_line(pbs,   (BB_GRAPH_BORDER + (max_perc_len * BB_GL_W) + BB_GRAPH_PAD_X + BB_GRAPH_PNG_LW / 2), y_r, 
                //                        (BB_GRAPH_BORDER + (max_perc_len * BB_GL_W) + BB_GRAPH_PAD_X + BB_GRAPH_PNG_LW / 2 + gw), y_r, 
                //                        color_4, perr);
                bg_svg_draw_line(pbs,   (BB_GRAPH_BORDER + yw + BB_GRAPH_PAD_X + BB_GRAPH_PNG_LW / 2), 
                                        y_r + (BB_GRAPH_PNG_LW / 2), 
                                        (BB_GRAPH_BORDER + yw + BB_GRAPH_PAD_X + BB_GRAPH_PNG_LW / 2 + gw), 
                                        y_r + (BB_GRAPH_PNG_LW / 2), 
                                        color_4, perr);

            if (!perr->err_code && 
                ((BB_GRAPH_BORDER + gh - (BB_GL_H / 2) + (BB_GRAPH_PNG_LW / 2)) > (y_r + BB_GL_H / 2 + BB_GRAPH_PNG_LW / 2 + BB_GRAPH_PAD_Y)) && 
                ((BB_GRAPH_BORDER * 2 + BB_GL_H + (BB_GL_H / 2) + (BB_GRAPH_PNG_LW / 2) + yh) < (y_r - BB_GL_H / 2 + BB_GRAPH_PNG_LW / 2 - BB_GRAPH_PAD_Y)))
                bg_svg_draw_text(pbs,   (BB_GRAPH_BORDER + yw + (BB_GRAPH_PAD_X * 2) + (BB_GRAPH_PNG_LW * 2) + gw), 
                                        (y_r - (BB_GL_H / 2) + (BB_GRAPH_PNG_LW / 2)), 
                                        "0", 1, color_1, perr);

            if (!perr->err_code)
            {
                points = (int *) calloc(prd->points_n * 2, sizeof(int));
                if (!points)
                    bg_graph_set_error(1, perr);
                else
                {
                    x1 += sw / 2;
                    for (x = x1, i = 0; (i < prd->points_n) && !perr->err_code; ++i, x += sw)
                    {
                        memset(&ttm, 0, sizeof(struct tm));
#ifdef BB_WIN
                        gmtime_s(&ttm, prd->dates + i);
#else
                        gmtime_r(prd->dates + i, &ttm);
#endif
                        snprintf(tmp, 64, "%02d.%02d", ttm.tm_mday, ttm.tm_mon + 1);
                        len = strlen(tmp);
                        bg_svg_draw_text(pbs,   (x - (BB_GL_W * len) / 2), 
                                                BB_GRAPH_BORDER + gh + BB_GRAPH_PNG_LW + BB_GRAPH_PAD_Y, 
                                                tmp, len, color_1, perr);

                        snprintf(tmp, 64, "%.3lf", *(prd->rates + i));
                        len = strlen(tmp);
                        bg_svg_draw_text(pbs,   (x - (BB_GL_W * len) / 2), 
                                                BB_GRAPH_BORDER + yh / 2, 
                                                tmp, len, color_3, perr);

                    }

                    if (!perr->err_code)
                    {
                        for (p = points, x = x1, i = 0; i < prd->points_n; ++i, x += sw)
                        {
                            *p++ = x;
                            *p++ = y_r - BB_GH * *(prd->values + i) / prd->division;
                        }
                        bg_svg_draw_mpline(pbs, points, prd->points_n, 1, BG_G_BC_COLOR, perr);
                    }

                    if (!perr->err_code)
                    {
                        for (p = points, x = x1, i = 0; i < prd->points_n; ++i, x += sw)
                        {
                            *p++ = x;
                            *p++ = (y - BB_GH * *(pgd->g_points + i) / (double) division);
                        }
                        bg_svg_draw_mpline(pbs, points, prd->points_n, 1, BG_G_BW_COLOR, perr);
                    }

                    if (!perr->err_code)
                    {
                        for (p = points, x = x1, i = 0; i < prd->points_n; ++i, x += sw)
                        {
                            *p++ = x;
                            *p++ = (y - BB_GH * *(pgd->r_points + i) / (double) division);
                        }
                        bg_svg_draw_mpline(pbs, points, prd->points_n, 1, BG_G_SL_COLOR, perr);
                    }
                }
            }

            if (points)
                free(points);

            if (!perr->err_code)
            {
                len = strlen(path);
                svg_path = (char *) malloc(len + 1);
                if (!svg_path)
                    bg_graph_set_error(1, perr);
                else
                {
                    strncpy(svg_path, path, len + 1);
                    *(svg_path + len - 3) = 's';
                    *(svg_path + len - 2) = 'v';
                    *(svg_path + len - 1) = 'g';
                    *(svg_path + len) = '\0';

                    bg_svg_write(pbs, svg_path, perr);

                    free(svg_path);
                }
            }
        }
        bg_svg_deinit(&pbs);
    }
}

static void bg_graph_svg_draw_legend(struct bg_svg *pbs, int x, int y, _Bool wl, struct bb_error *perr)
{
    unsigned char color[3] = {0, 0, 0};

    bg_svg_draw_rect(pbs, x, y, BB_GT_RW, BB_GT_RH, BG_G_BW_COLOR, perr);
    if (!perr->err_code)
    {
        x += BB_GT_RW + BB_GT_P1;
        bg_svg_draw_text(pbs, x, y, (!wl ? "buy" : "win"), 3, color, perr);

        x += BG_GT_W + BB_GT_P2;

        bg_svg_draw_rect(pbs, x, y, BB_GT_RW, BB_GT_RH, BG_G_SL_COLOR, perr);
        if (!perr->err_code)
        {
            x += BB_GT_RW + BB_GT_P1;
            bg_svg_draw_text(pbs, x, y, (!wl ? "sel" : "los"), 3, color, perr);

            x += BG_GT_W;
            bg_graph_svg_draw_r_legend(pbs, x, y, perr);
/*
            x += BG_GT_W + BB_GT_P2;

            bg_svg_draw_rect(pbs, x, y, BB_GT_RW, BB_GT_RH, BG_G_BC_COLOR, perr);
            if (!perr->err_code)
            {
                x += BB_GT_RW + BB_GT_P1;
                bg_svg_draw_text(pbs, x, y, "btc", 3, color, perr);
            }
*/
        }
    }
}

static void bg_graph_svg_draw_r_legend(struct bg_svg *pbs, int x, int y, struct bb_error *perr)
{
    unsigned char color[3] = {0, 0, 0};

    x += BB_GT_P2;

    bg_svg_draw_rect(pbs, x, y, BB_GT_RW, BB_GT_RH, BG_G_BC_COLOR, perr);
    if (!perr->err_code)
    {
        x += BB_GT_RW + BB_GT_P1;
        bg_svg_draw_text(pbs, x, y, "delta%BTC", 9, color, perr);
    }
}

static void bg_graph_get_n(const struct bg_graph_data *pgd, const struct bg_graph_rates_data *prd, 
    size_t *pmax_perc_len, size_t *pmax_rate_len, size_t *pdivision, int *pkm)
{
    double *pd1, *pd2;
    double n;
    size_t s_len;
    int i;
    _Bool k, m;

    *pmax_perc_len = *pmax_rate_len = *pdivision = 0;
    n = (double) 0;
    k = m = 1;

    for (pd1 = pgd->g_points, pd2 = pd1 + prd->points_n; pd1 < pd2; ++pd1)
        if (n < *pd1)
            n = *pd1;

    for (pd1 = pgd->r_points, pd2 = pd1 + prd->points_n; pd1 < pd2; ++pd1)
        if (n < *pd1)
            n = *pd1;

    for (pd1 = prd->rates, pd2 = pd1 + prd->points_n; pd1 < pd2; ++pd1)
    {
        s_len = bb_graph_get_s_len(*pd1);
        if (*pmax_rate_len < s_len)
            *pmax_rate_len = s_len;
    }

    *pdivision = bb_graph_get_scale(n);
/*
    *pmax_perc_len = bb_graph_get_s_len((double) *pdivision);
    if (pgd->is_perc)
        (*pmax_perc_len)++;    
*/
    if (pgd->is_perc)
    {
        *pmax_perc_len = bb_graph_get_s_len((double) *pdivision);
        (*pmax_perc_len)++;                                         // %
    }
    else
    {
        for (i = 0; i < 5; ++i)
            if ((*pdivision * (5 - i) / 5) % 1000000)
            {
                m = 0;
                break;
            }

        if (!m)
            for (i = 0; i < 5; ++i)
                if ((*pdivision * (5 - i) / 5) % 1000)
                {
                    k = 0;
                    break;
                }

        if (m)
        {
            *pmax_perc_len = bb_graph_get_s_len((double) *pdivision) - 5;
            *pkm = 2;
        }
        else if (k)
        {
            *pmax_perc_len = bb_graph_get_s_len((double) *pdivision) - 2;
            *pkm = 1;
        }
        else
        {
            *pmax_perc_len = bb_graph_get_s_len((double) *pdivision);
            *pkm = 0;
        }
    }    

    *pmax_rate_len += 4;                                            // .3
}

static size_t bb_graph_get_s_len(double value)
{
    size_t s_len;
    int64_t n;

    s_len = (value < -BB_D) ? 1 : 0;
    n = (int64_t) llround(value);
    do {
        ++s_len;
        n /= 10;
    } while (n);

    return s_len;
}

static size_t bb_graph_get_scale(double d)
{
    size_t t, k, m, n, division;
    _Bool flag;

    n = ceil(fabs(d));

    if (!n)
        division = 1;
    else if (n <= 10)
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

void bg_graph_save_rates_graph(struct bg_graph_rates_data *prd, const char *path, _Bool save_png, struct bb_error *perr)
{
    struct bb_graph_png *pgp;
    struct tm ttm;
    double *pv, *pv2;
    size_t i, w, h, sw, gw, gh, len, drw, dew;
    int x, y, y1, y2;
    char tmp[64] = {0};
    unsigned char color_1[3] = {0, 0, 0};
    unsigned char color_2[3] = {172, 172, 172};
    unsigned char color_3[3] = {0x00, 0x9d, 0xcf};

    pgp = NULL;

    if (prd && path && perr)
    {
        if (save_png)
        {
            sw = prd->max_rate_len * BB_GL_W + (BB_GRAPH_BORDER * 4);
            gw = sw * prd->points_n;
            //if (gw < BG_G_LEGEND_W)
            //    gw = BG_G_LEGEND_W;
            w = (BB_GRAPH_BORDER * 2) + (prd->max_perc_len * BB_GL_W) + BB_GRAPH_PAD_X + BB_GRAPH_PNG_LW + gw;
            if (w < BG_G_R_LEGEND_W)
                w = BG_G_R_LEGEND_W;
            dew = *(BG_G_DESC_EN_SLEN + (BG_G_DESC_N - 1)) * BG_GL_W;
            if (w < (dew + (BB_GRAPH_BORDER * 2)))
                w = (dew + (BB_GRAPH_BORDER * 2));
            drw = *(BG_G_DESC_RU_SLEN + (BG_G_DESC_N - 1)) * BG_GL_W;
            if (w < (drw + (BB_GRAPH_BORDER * 2)))
                w = (drw + (BB_GRAPH_BORDER * 2));

            gh = BG_YT0_H + BB_GL_H + BB_GRAPH_BORDER + BB_GH;
            //h = (BB_GRAPH_BORDER * 3) + gh + BB_GRAPH_PNG_LW + BB_GRAPH_PAD_Y + BB_GL_H + BG_G_LEGEND_H;
            //h = (BB_GRAPH_BORDER * 2) + gh + BB_GRAPH_PNG_LW + BB_GRAPH_PAD_Y + BB_GL_H;
            //h = (BB_GRAPH_BORDER * 3) + gh + BB_GRAPH_PNG_LW * 2 + BB_GRAPH_PAD_Y + (BB_GL_H * 2);
            h = (BB_GRAPH_BORDER * 4) + gh + BB_GRAPH_PNG_LW * 2 + BB_GRAPH_PAD_Y * 2 + BB_GL_H + BG_G_LEGEND_H + (BG_GL_H * 2);

            x = BB_GRAPH_BORDER + prd->max_perc_len * BB_GL_W + BB_GRAPH_PAD_X;
            //y = gh + BB_GRAPH_BORDER;
            y = BB_GRAPH_BORDER * 2 + BB_GL_H + BG_YT0_H + (prd->max_val / prd->division) * BB_GH;

            if (!bb_graph_png_init(&pgp, w, h, perr) && 
                !bb_graph_png_draw_rect(pgp,    (BB_GRAPH_BORDER + (prd->max_perc_len * BB_GL_W) + BB_GRAPH_PAD_X), 
                                                BB_GRAPH_BORDER, BB_GRAPH_PNG_LW, gh + BB_GRAPH_PNG_LW, color_1, perr) && 
                !bb_graph_png_draw_rect(pgp,    (BB_GRAPH_BORDER + (prd->max_perc_len * BB_GL_W) + BB_GRAPH_PAD_X + BB_GRAPH_PNG_LW), 
                                                (BB_GRAPH_BORDER * 2 + BB_GL_H) + BG_YT0_H, gw, BB_GRAPH_PNG_LW, color_2, perr) && 
                !bb_graph_png_draw_rect(pgp,    (BB_GRAPH_BORDER + (prd->max_perc_len * BB_GL_W) + BB_GRAPH_PAD_X + BB_GRAPH_PNG_LW), 
                                                (BB_GRAPH_BORDER + gh), gw, BB_GRAPH_PNG_LW, color_2, perr) && 
                !bb_graph_png_write_text(pgp,   (BB_GRAPH_BORDER + ((prd->max_perc_len - prd->min_val_str_len) * BB_GL_W)), 
                                                (BB_GRAPH_BORDER + gh - (BB_GL_H / 2) + (BB_GRAPH_PNG_LW / 2)), 
                                                prd->min_val_str, prd->min_val_str_len, color_1, 0, perr) && 
                !bb_graph_png_write_text(pgp,   (BB_GRAPH_BORDER + ((prd->max_perc_len - prd->max_val_str_len) * BB_GL_W)), 
                                                (BB_GRAPH_BORDER * 2 + BB_GL_H - (BB_GL_H / 2) + (BB_GRAPH_PNG_LW / 2) + BG_YT0_H), 
                                                prd->max_val_str, prd->max_val_str_len, color_1, 0, perr) && 
                !bg_graph_png_write_text(pgp,   (w - dew) / 2, 
                                                (BB_GRAPH_BORDER * 3 + gh + BB_GRAPH_PNG_LW * 2 + BB_GRAPH_PAD_Y + BB_GL_H + BG_G_LEGEND_H), 
                                                *(BG_G_DESC_EN + (BG_G_DESC_N - 1)), *(BG_G_DESC_EN_SLEN + (BG_G_DESC_N - 1)), color_1, perr) && 
                !bg_graph_png_write_text(pgp,   (w - drw) / 2, 
                                                (BB_GRAPH_BORDER * 3 + gh + BB_GRAPH_PNG_LW * 2 + BB_GRAPH_PAD_Y * 2 + BB_GL_H + BG_G_LEGEND_H + BG_GL_H), 
                                                *(BG_G_DESC_RU + (BG_G_DESC_N - 1)), *(BG_G_DESC_RU_CLEN + (BG_G_DESC_N - 1)), color_1, perr))
            {
                bb_graph_png_draw_yt(pgp, BB_GRAPH_BORDER + prd->max_perc_len * BB_GL_W, BB_GRAPH_BORDER, 1, 0, 0, color_1);

                x += sw / 2;
                for (i = 0, pv = prd->values, pv2 = pv + prd->points_n; (pv < pv2) && !perr->err_code; ++i, ++pv, x += sw)
                {
                    memset(&ttm, 0, sizeof(struct tm));
#ifdef BB_WIN
                    gmtime_s(&ttm, prd->dates + i);
#else
                    gmtime_r(prd->dates + i, &ttm);
#endif
                    //snprintf(tmp, 64, "%02d.%02d.%04d", ttm.tm_mday, ttm.tm_mon + 1, ttm.tm_year + 1900);
                    snprintf(tmp, 64, "%02d.%02d", ttm.tm_mday, ttm.tm_mon + 1);
                    len = strlen(tmp);
                    bb_graph_png_write_text(pgp,    (x - (BB_GL_W * len) / 2), 
                                                    BB_GRAPH_BORDER + gh + BB_GRAPH_PNG_LW + BB_GRAPH_PAD_Y, 
                                                    tmp, len, color_1, 0, perr);

                    if (!perr->err_code)
                    {
                        snprintf(tmp, 64, "%.3lf", *(prd->rates + i));
                        len = strlen(tmp);
                        bb_graph_png_write_text(pgp,    (x - (BB_GL_W * len) / 2), 
                                                        BB_GRAPH_BORDER + BG_YT0_H / 2, 
                                                        tmp, len, color_3, 0, perr);

                        if (!perr->err_code)
                        {
                            y1 = (y - BB_GH * *pv / prd->division);
                            if ((pv + 1) < pv2)
                            {
                                y2 = (y - BB_GH * *(pv + 1) / prd->division);
                                bb_graph_png_draw_line(pgp, x, y1, (x + sw), y2, BG_G_BC_COLOR, perr);
                            }

                            if (!perr->err_code)
                                bb_graph_png_draw_point(pgp, x, y1, BG_G_BC_COLOR, perr);
                        }
                    }
                }

                if (!perr->err_code)
                    bg_graph_png_draw_r_legend(pgp, BB_GRAPH_BORDER, 
                                                    (BB_GRAPH_BORDER * 2 + gh + BB_GRAPH_PNG_LW + BB_GRAPH_PAD_Y + BB_GL_H), perr);

                if (!perr->err_code)
                    bb_graph_png_save(pgp, path, perr);
            }
            bb_graph_png_free(&pgp);
        }

        if (!perr->err_code)
            bg_graph_save_rates_graph_svg(prd, path, perr);
    }
}

static void bg_graph_save_rates_graph_svg(struct bg_graph_rates_data *prd, const char *path, struct bb_error *perr)
{
    struct bg_svg *pbs;
    struct tm ttm;
    double *pv, *pv2;
    int *points, *p;
    char *svg_path;
    size_t i, w, h, sw, gw, gh, len, drw, dew;
    int x, y;
    char tmp[64] = {0};
    unsigned char color_1[3] = {0, 0, 0};
    unsigned char color_2[3] = {172, 172, 172};
    unsigned char color_3[3] = {0x00, 0x9d, 0xcf};

    pbs = NULL;

    if (prd && path && perr)
    {
        sw = prd->max_rate_len * BB_GL_W + (BB_GRAPH_BORDER * 4);
        gw = sw * prd->points_n;
        w = (BB_GRAPH_BORDER * 2) + (prd->max_perc_len * BB_GL_W) + BB_GRAPH_PAD_X + BB_GRAPH_PNG_LW + gw;
        if (w < BG_G_R_LEGEND_W)
            w = BG_G_R_LEGEND_W;
        dew = *(BG_G_DESC_EN_SLEN + (BG_G_DESC_N - 1)) * BG_GL_W;
        if (w < (dew + (BB_GRAPH_BORDER * 2)))
            w = (dew + (BB_GRAPH_BORDER * 2));
        drw = *(BG_G_DESC_RU_SLEN + (BG_G_DESC_N - 1)) * BG_GL_W;
        if (w < (drw + (BB_GRAPH_BORDER * 2)))
            w = (drw + (BB_GRAPH_BORDER * 2));

        gh = BG_YT0_H + BB_GL_H + BB_GRAPH_BORDER + BB_GH;
        //h = (BB_GRAPH_BORDER * 3) + gh + BB_GRAPH_PNG_LW * 2 + BB_GRAPH_PAD_Y + (BB_GL_H * 2);
        h = (BB_GRAPH_BORDER * 4) + gh + BB_GRAPH_PNG_LW * 2 + BB_GRAPH_PAD_Y * 2 + BB_GL_H + BG_G_LEGEND_H + (BG_GL_H * 2);

        x = BB_GRAPH_BORDER + prd->max_perc_len * BB_GL_W + BB_GRAPH_PAD_X;
        y = BB_GRAPH_BORDER * 2 + BB_GL_H + BG_YT0_H + (prd->max_val / prd->division) * BB_GH;

        if (!bg_svg_init(&pbs, w, h, perr) && 
            !bg_svg_draw_line(pbs,  (BB_GRAPH_BORDER + (prd->max_perc_len * BB_GL_W) + BB_GRAPH_PAD_X), BB_GRAPH_BORDER, 
                                    (BB_GRAPH_BORDER + (prd->max_perc_len * BB_GL_W) + BB_GRAPH_PAD_X), (BB_GRAPH_BORDER + gh + BB_GRAPH_PNG_LW / 2), 
                                    color_1, perr) && 
            !bg_svg_draw_line(pbs,  (BB_GRAPH_BORDER + (prd->max_perc_len * BB_GL_W) + BB_GRAPH_PAD_X + BB_GRAPH_PNG_LW / 2), 
                                    (BB_GRAPH_BORDER * 2 + BB_GL_H + BG_YT0_H), 
                                    (BB_GRAPH_BORDER + (prd->max_perc_len * BB_GL_W) + BB_GRAPH_PAD_X + BB_GRAPH_PNG_LW + gw), 
                                    (BB_GRAPH_BORDER * 2 + BB_GL_H + BG_YT0_H), color_2, perr) && 
            !bg_svg_draw_line(pbs,  (BB_GRAPH_BORDER + (prd->max_perc_len * BB_GL_W) + BB_GRAPH_PAD_X + BB_GRAPH_PNG_LW / 2), 
                                    (BB_GRAPH_BORDER + gh), 
                                    (BB_GRAPH_BORDER + (prd->max_perc_len * BB_GL_W) + BB_GRAPH_PAD_X + BB_GRAPH_PNG_LW + gw), 
                                    (BB_GRAPH_BORDER + gh), color_2, perr) && 
            !bg_svg_draw_text(pbs,  (BB_GRAPH_BORDER + ((prd->max_perc_len - prd->min_val_str_len) * BB_GL_W)), 
                                    (BB_GRAPH_BORDER + gh - (BB_GL_H / 2) + (BB_GRAPH_PNG_LW / 2)), 
                                    prd->min_val_str, prd->min_val_str_len, color_1, perr) && 
            !bg_svg_draw_text(pbs,  (BB_GRAPH_BORDER + ((prd->max_perc_len - prd->max_val_str_len) * BB_GL_W)), 
                                    (BB_GRAPH_BORDER * 2 + BB_GL_H - (BB_GL_H / 2) + (BB_GRAPH_PNG_LW / 2) + BG_YT0_H), 
                                    prd->max_val_str, prd->max_val_str_len, color_1, perr) && 
            !bg_svg_draw_text(pbs,  BB_GRAPH_BORDER + (prd->max_perc_len - 1) * BB_GL_W, BB_GRAPH_BORDER, "%", 1, color_1, perr)&& 
            !bg_svg_draw_text(pbs,  (w - dew) / 2, 
                                    (BB_GRAPH_BORDER * 3 + gh + BB_GRAPH_PNG_LW * 2 + BB_GRAPH_PAD_Y + BB_GL_H + BG_G_LEGEND_H) + 2, 
                                    *(BG_G_DESC_EN + (BG_G_DESC_N - 1)), *(BG_G_DESC_EN_SLEN + (BG_G_DESC_N - 1)), color_1, perr) && 
            !bg_svg_draw_text(pbs,  (w - drw) / 2, 
                                    (BB_GRAPH_BORDER * 3 + gh + BB_GRAPH_PNG_LW * 2 + BB_GRAPH_PAD_Y * 2 + BB_GL_H + BG_G_LEGEND_H + BG_GL_H) + 2, 
                                    *(BG_G_DESC_RU + (BG_G_DESC_N - 1)), *(BG_G_DESC_RU_CLEN + (BG_G_DESC_N - 1)), color_1, perr))
        {
            points = (int *) calloc(prd->points_n * 2, sizeof(int));
            if (!points)
                bg_graph_set_error(1, perr);
            else
            {
                bg_graph_svg_draw_r_legend(pbs, BB_GRAPH_BORDER, 
                                                (BB_GRAPH_BORDER * 2 + gh + BB_GRAPH_PNG_LW + BB_GRAPH_PAD_Y + BB_GL_H), perr);

                x += sw / 2;
                for (p = points, i = 0, pv = prd->values, pv2 = pv + prd->points_n; (pv < pv2) && !perr->err_code; ++i, ++pv, x += sw)
                {
                    memset(&ttm, 0, sizeof(struct tm));
#ifdef BB_WIN
                    gmtime_s(&ttm, prd->dates + i);
#else
                    gmtime_r(prd->dates + i, &ttm);
#endif
                    snprintf(tmp, 64, "%02d.%02d", ttm.tm_mday, ttm.tm_mon + 1);
                    len = strlen(tmp);
                    bg_svg_draw_text(pbs,   (x - (BB_GL_W * len) / 2), 
                                            BB_GRAPH_BORDER + gh + BB_GRAPH_PNG_LW + BB_GRAPH_PAD_Y, 
                                            tmp, len, color_1, perr);

                    if (!perr->err_code)
                    {
                        snprintf(tmp, 64, "%.3lf", *(prd->rates + i));
                        len = strlen(tmp);
                        bg_svg_draw_text(pbs,   (x - (BB_GL_W * len) / 2), 
                                                BB_GRAPH_BORDER + BG_YT0_H / 2, 
                                                tmp, len, color_3, perr);

                        if (!perr->err_code)
                        {
                            *p++ = x;
                            *p++ = (y - BB_GH * *pv / prd->division);
                        }
                    }
                }

                if (!perr->err_code)
                    bg_svg_draw_mpline(pbs, points, prd->points_n, 1, BG_G_BC_COLOR, perr);

                if (points)
                    free(points);

                if (!perr->err_code)
                {
                    len = strlen(path);
                    svg_path = (char *) malloc(len + 1);
                    if (!svg_path)
                        bg_graph_set_error(1, perr);
                    else
                    {
                        strncpy(svg_path, path, len + 1);
                        *(svg_path + len - 3) = 's';
                        *(svg_path + len - 2) = 'v';
                        *(svg_path + len - 1) = 'g';
                        *(svg_path + len) = '\0';

                        bg_svg_write(pbs, svg_path, perr);

                        free(svg_path);
                    }
                }
            }
        }
        bg_svg_deinit(&pbs);
    }
}

static void bg_graph_get_rates_parameters(struct bg_graph_rates_data *prd)
{
    double *pv, *pv2, *prt, prev;
    size_t len;

    prd->min_val = prd->max_val = *prd->values = (double) 0;
    prev = *prd->rates;
    prd->max_rate_len = bb_graph_get_s_len(prev);
    for (prt = prd->rates + 1, pv = prd->values + 1, pv2 = prd->values + prd->points_n; pv < pv2; ++prt, ++pv)
    {
        *pv = (*prt - prev) * (double) 100 / prev;

        if (prd->min_val > *pv)
            prd->min_val = *pv;
        if (prd->max_val < *pv)
            prd->max_val = *pv;

        len = bb_graph_get_s_len(*prt);
        if (prd->max_rate_len < len)
            prd->max_rate_len = len;

        prev = *prt;
    }

    prd->max_rate_len += 4;

    bg_graph_get_rate_values(&prd->min_val, prd->min_val_str, &prd->max_val, prd->max_val_str);

    prd->division = prd->max_val - prd->min_val;
    //printf("division %.3lf\n", prd->division);

    prd->min_val_str_len = strlen(prd->min_val_str);
    prd->max_val_str_len = strlen(prd->max_val_str);
    prd->max_perc_len = (prd->min_val_str_len > prd->max_val_str_len) ? prd->min_val_str_len : prd->max_val_str_len;
}

static void bg_graph_get_rate_values(double *pmin_val, char min_val_str[BG_RATE_D_SIZE], double *pmax_val, char max_val_str[BG_RATE_D_SIZE])
{
    double abs_min_val, abs_max_val;

    if (fabs(*pmax_val - *pmin_val) < BB_D)
    {
        *pmin_val -= 0.001;
        *pmax_val += 0.001;
    }

    abs_min_val = fabs(*pmin_val);
    abs_max_val = fabs(*pmax_val);

    if (abs_min_val < 0.001)
    {
        *pmin_val = (*pmin_val < -0.0) ? -0.001 : 0.001;
        abs_min_val = 0.001;
    }

    if (abs_max_val < 0.001)
    {
        *pmax_val = (*pmax_val < -0.0) ? -0.001 : 0.001;
        abs_max_val = 0.001;
    }

    if ((abs_min_val < 0.01) || (abs_max_val < 0.01))
    {
        if (abs_min_val < 0.01)
        {
            abs_min_val = ceil(abs_min_val * (double) 1000) / (double) 1000;
            *pmin_val = (*pmin_val < -0.0) ? -abs_min_val : abs_min_val;
        }
        if (abs_max_val < 0.01)
        {
            abs_max_val = ceil(abs_max_val * (double) 1000) / (double) 1000;
            *pmax_val = (*pmax_val < -0.0) ? -abs_max_val : abs_max_val;
        }

        snprintf(min_val_str, BG_RATE_D_SIZE, "%.3lf%%", *pmin_val);
        snprintf(max_val_str, BG_RATE_D_SIZE, "%.3lf%%", *pmax_val);
    }
    else if ((abs_min_val < 0.1) || (abs_max_val < 0.1))
    {
        if (abs_min_val < 0.1)
        {
            abs_min_val = ceil(abs_min_val * (double) 100) / (double) 100;
            *pmin_val = (*pmin_val < -0.0) ? -abs_min_val : abs_min_val;
        }
        if (abs_max_val < 0.1)
        {
            abs_max_val = ceil(abs_max_val * (double) 100) / (double) 100;
            *pmax_val = (*pmax_val < -0.0) ? -abs_max_val : abs_max_val;
        }

        snprintf(min_val_str, BG_RATE_D_SIZE, "%.2lf%%", *pmin_val);
        snprintf(max_val_str, BG_RATE_D_SIZE, "%.2lf%%", *pmax_val);
    }
    else if ((abs_min_val < 1.0) || (abs_max_val < 1.0))
    {
        if (abs_min_val < 1.0)
        {
            abs_min_val = ceil(abs_min_val * (double) 10) / (double) 10;
            *pmin_val = (*pmin_val < -0.0) ? -abs_min_val : abs_min_val;
        }
        if (abs_max_val < 1.0)
        {
            abs_max_val = ceil(abs_max_val * (double) 10) / (double) 10;
            *pmax_val = (*pmax_val < -0.0) ? -abs_max_val : abs_max_val;
        }

        snprintf(min_val_str, BG_RATE_D_SIZE, "%.1lf%%", *pmin_val);
        snprintf(max_val_str, BG_RATE_D_SIZE, "%.1lf%%", *pmax_val);
    }
    else
    {
        abs_min_val = ceil(abs_min_val);
        *pmin_val = (*pmin_val < -0.0) ? -abs_min_val : abs_min_val;

        abs_max_val = ceil(abs_max_val);
        *pmax_val = (*pmax_val < -0.0) ? -abs_max_val : abs_max_val;

        snprintf(min_val_str, BG_RATE_D_SIZE, "%ld%%", lround(*pmin_val));
        snprintf(max_val_str, BG_RATE_D_SIZE, "%ld%%", lround(*pmax_val));
    }
}

const char * bg_graph_error_message(int err_code)
{
    static const char *BB_GRAPH_ERRS[BB_GRAPH_ERRS_N] = {   "ОК",                           //  0
                                                            "Ошибка выделения памяти",      //  1
                                                            "Неизвестная ошибка",           //  2
                                                            "Пустой указатель",             //  3
                                                            "Некорректный номер графика",   //  4
    };

    if ((err_code < 0) || (err_code >= BB_GRAPH_ERRS_N))
        err_code = 2;

    return *(BB_GRAPH_ERRS + err_code);
}

static void bg_graph_set_error(int err_code, struct bb_error *perr)
{
    perr->err_code = err_code;
    perr->err_str = bg_graph_error_message(err_code);
}
