#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include "bb_gl.h"
#include "bb_graph_png.h"
#include "bb_graph.h"

#define BB_D 0.0001

#define BB_GRAPH_BORDER 5
#define BB_GRAPH_PAD_X 2
#define BB_GRAPH_PAD_Y 2

static const unsigned char BB_G_WB_COLOR[3] = {BB_G_WB_R, BB_G_WB_G, BB_G_WB_B};
static const unsigned char BB_G_WS_COLOR[3] = {BB_G_WS_R, BB_G_WS_G, BB_G_WS_B};
static const unsigned char BB_G_LB_COLOR[3] = {BB_G_LB_R, BB_G_LB_G, BB_G_LB_B};
static const unsigned char BB_G_LS_COLOR[3] = {BB_G_LS_R, BB_G_LS_G, BB_G_LS_B};

/*  Gets max values of list.  */
static void bb_graph_get_n(const struct bb_csv_node *first, size_t *pmax_perc_len, size_t *pmax_rate_len, size_t *pdivision, size_t *pn);

/*  Returns value string length.  */
static size_t bb_graph_get_s_len(double value);

/*  Get max scale devision.  */
static size_t bb_graph_get_scale(double d);

void bb_graph_save_graph(const struct bb_csv_node *first, const char *file_name, struct bb_error *perr)
{
    struct bb_graph_png *pgp;
    const struct bb_csv_node *node;
    struct tm ttm;
    //struct bb_error err = {0};
    char tmp[64] = {0};
    unsigned char color_1[3] = {0, 0, 0};
    unsigned char color_2[3] = {172, 172, 172};
    unsigned char color_3[3] = {0x00, 0x9d, 0xcf};
    size_t w, h, sw, gw, gh, max_perc_len, max_rate_len, division, n, len;
    int i, x, y, y1, y2;

    if (first && file_name && perr)
    {
        pgp = NULL;

        bb_graph_get_n(first, &max_perc_len, &max_rate_len, &division, &n);

        sw = max_rate_len * BB_GL_W + (BB_GRAPH_BORDER * 4);
        //gw = BB_GRAPH_PAD_X * 2 + (sw * n);
        gw = sw * n;
        if (gw < BB_G_LEGEND_W)
            gw = BB_G_LEGEND_W;
        w = (BB_GRAPH_BORDER * 2) + (max_perc_len * BB_GL_W) + BB_GRAPH_PAD_X + BB_GRAPH_PNG_LW + gw;

        gh = BB_GL_H + BB_GRAPH_BORDER + BB_GH;
        h = (BB_GRAPH_BORDER * 3) + gh + BB_GRAPH_PNG_LW + BB_GRAPH_PAD_Y + BB_GL_H + BB_G_LEGEND_H;

        x = BB_GRAPH_BORDER + max_perc_len * BB_GL_W + BB_GRAPH_PAD_X;
        y = gh + BB_GRAPH_BORDER;

        if (!bb_graph_png_init(&pgp, w, h, perr) && 
            !bb_graph_png_draw_rect(pgp,    (BB_GRAPH_BORDER + (max_perc_len * BB_GL_W) + BB_GRAPH_PAD_X), 
                                            BB_GRAPH_BORDER, BB_GRAPH_PNG_LW, gh + BB_GRAPH_PNG_LW, color_1, perr) && 
            !bb_graph_png_draw_rect(pgp,    (BB_GRAPH_BORDER + (max_perc_len * BB_GL_W) + BB_GRAPH_PAD_X + BB_GRAPH_PNG_LW), 
                                            (BB_GRAPH_BORDER + gh), gw, BB_GRAPH_PNG_LW, color_1, perr) && 
            !bb_graph_png_draw_legend(pgp,  (BB_GRAPH_BORDER + (max_perc_len * BB_GL_W) + BB_GRAPH_PAD_X + BB_GRAPH_PNG_LW + 
                                            ((gw - BB_G_LEGEND_W) / 2)), 
                                            ((BB_GRAPH_BORDER * 2) + gh + BB_GRAPH_PNG_LW + BB_GRAPH_PAD_Y + BB_GL_H), perr) && 
            !bb_graph_png_write_text(pgp,   (BB_GRAPH_BORDER + ((max_perc_len - 1) * BB_GL_W)), 
                                            (BB_GRAPH_BORDER + gh - (BB_GL_H / 2) + (BB_GRAPH_PNG_LW / 2)), "0", 1, color_1, 0, perr))
        {
            for (i = 0; (i < 5) && !perr->err_code; ++i)
            {
                snprintf(tmp, 64, "%zu%%", division * (5 - i) / 5);
                len = strlen(tmp);
                bb_graph_png_write_text(pgp,    (x - BB_GRAPH_PAD_X - BB_GL_W * len), 
                                                (y - BB_GL_H / 2 + BB_GRAPH_PNG_LW / 2 - BB_GH * (5 - i) / 5), 
                                                tmp, len, color_1, 0, perr);
                if (!perr->err_code)
                    bb_graph_png_draw_rect(pgp, (x + BB_GRAPH_PNG_LW), 
                                                (y - BB_GH * (5 - i) / 5), gw, BB_GRAPH_PNG_LW, color_2, perr);
            }

            if (!perr->err_code)
            {
                x += sw / 2;
                for (node = first; node && !perr->err_code; node = node->next, x += sw)
                {
                    memset(&ttm, 0, sizeof(struct tm));
#ifdef BB_WIN
                    gmtime_s(&ttm, &node->date);
#else
                    gmtime_r(&node->date, &ttm);
#endif
                    //snprintf(tmp, 64, "%02d.%02d.%04d", ttm.tm_mday, ttm.tm_mon + 1, ttm.tm_year + 1900);
                    snprintf(tmp, 64, "%02d.%02d", ttm.tm_mday, ttm.tm_mon + 1);
                    len = strlen(tmp);
                    bb_graph_png_write_text(pgp,    (x - (BB_GL_W * len) / 2), 
                                                    BB_GRAPH_BORDER + gh + BB_GRAPH_PNG_LW + BB_GRAPH_PAD_Y, 
                                                    tmp, len, color_1, 0, perr);

                    snprintf(tmp, 64, "%.3lf", node->rate);
                    len = strlen(tmp);
                    bb_graph_png_write_text(pgp,    (x - (BB_GL_W * len) / 2), 
                                                    BB_GRAPH_BORDER, 
                                                    tmp, len, color_3, 0, perr);

                    if (!perr->err_code)
                    {
                        y1 = (y - BB_GH * node->win_buy / (double) division);
                        if (node->next)
                        {
                            y2 = (y - BB_GH * node->next->win_buy / (double) division);
                            bb_graph_png_draw_line(pgp, x, y1, (x + sw), y2, BB_G_WB_COLOR, perr);
                        }

                        if (!perr->err_code)
                            bb_graph_png_draw_point(pgp, x, y1, BB_G_WB_COLOR, perr);
                    }

                    if (!perr->err_code)
                    {
                        y1 = (y - BB_GH * node->win_sel / (double) division);
                        if (node->next)
                        {
                            y2 = (y - BB_GH * node->next->win_sel / (double) division);
                            bb_graph_png_draw_line(pgp, x, y1, (x + sw), y2, BB_G_WS_COLOR, perr);
                        }

                        if (!perr->err_code)
                            bb_graph_png_draw_point(pgp, x, y1, BB_G_WS_COLOR, perr);
                    }

                    if (!perr->err_code)
                    {
                        y1 = (y - BB_GH * node->los_buy / (double) division);
                        if (node->next)
                        {
                            y2 = (y - BB_GH * node->next->los_buy / (double) division);
                            bb_graph_png_draw_line(pgp, x, y1, (x + sw), y2, BB_G_LB_COLOR, perr);
                        }

                        if (!perr->err_code)
                            bb_graph_png_draw_point(pgp, x, y1, BB_G_LB_COLOR, perr);
                    }

                    if (!perr->err_code)
                    {
                        y1 = (y - BB_GH * node->los_sel / (double) division);
                        if (node->next)
                        {
                            y2 = (y - BB_GH * node->next->los_sel / (double) division);
                            bb_graph_png_draw_line(pgp, x, y1, (x + sw), y2, BB_G_LS_COLOR, perr);
                        }

                        if (!perr->err_code)
                            bb_graph_png_draw_point(pgp, x, y1, BB_G_LS_COLOR, perr);
                    }
                }
            }

            if (!perr->err_code)
                bb_graph_png_save(pgp, file_name, perr);
        }
        bb_graph_png_free(&pgp);
    }
}

static void bb_graph_get_n(const struct bb_csv_node *first, size_t *pmax_perc_len, size_t *pmax_rate_len, size_t *pdivision, size_t *pn)
{
    const struct bb_csv_node *node;
    double n;
    size_t s_len;

    *pmax_perc_len = *pmax_rate_len = *pdivision = *pn = 0;
    n = (double) 0;
    for (node = first; node; node = node->next, (*pn)++)
    {
        if (n < node->win_buy)
            n = node->win_buy;
        if (n < node->win_sel)
            n = node->win_sel;
        if (n < node->los_buy)
            n = node->los_buy;
        if (n < node->los_sel)
            n = node->los_sel;

        s_len = bb_graph_get_s_len(node->rate);
        if (*pmax_rate_len < s_len)
            *pmax_rate_len = s_len;
    }

    *pdivision = bb_graph_get_scale(n);
    *pmax_perc_len = bb_graph_get_s_len((double) *pdivision) + 1;   // %
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
/*
static size_t bb_graph_get_scale(double d)
{
    size_t n, division;

    n = ceil(fabs(d));

    if (n <= 10)
        division = n;
    else
        for (division = 10; division < n; division += 5)
            continue;

    return division;
}
*/
static size_t bb_graph_get_scale(double d)
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
