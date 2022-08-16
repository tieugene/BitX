//#ifdef BB_WIN
//#include <glib.h>
//#include <glib/gstdio.h>
//#else
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#endif

#include "bb_buf_data.h"
#include "bg_svg.h"

#define BG_SVG_DEFAULT_BUF_SIZE 1024        // Default buffer size.
#define BG_SVG_ERRS_N 8                     // Number of errors.

#define BG_SVG_HEX_COLOR_SIZE 8
#define BG_SVG_POINTS_R_STR "3"

struct bg_svg {
    struct bb_buf_data *pbd;                // Buffer.
    unsigned int w;                         // Image width.
    unsigned int h;                         // Image height.
};

/*  Gets hex color by rgb.  */
static void bg_svg_get_hex_color(const unsigned char color[3], char hex_color[BG_SVG_HEX_COLOR_SIZE]);

/*  Sets error data.  */
static void bg_svg_set_error(int err_code, struct bb_error *perr);

int bg_svg_init(struct bg_svg **ppbs, unsigned int w, unsigned int h, struct bb_error *perr)
{
    int err_code;

    err_code = 0;

    if (!ppbs)
        err_code = 3;
    else if (!w || !h)
        err_code = 4;
    else
    {
        *ppbs = (struct bg_svg *) calloc(1, sizeof(struct bg_svg));
        if (!*ppbs)
            err_code = 1;
        else
        {
            if (bb_buf_init(&(*ppbs)->pbd, BG_SVG_DEFAULT_BUF_SIZE) < 0)
            {
                free(*ppbs);
                *ppbs = NULL;

                err_code = 1;
            }
            else
            {
                (*ppbs)->w = w;
                (*ppbs)->h = h;
            }
        }
    }

    if (err_code && perr)
        bg_svg_set_error(err_code, perr);
    return err_code;
}

int bg_svg_draw_rect(struct bg_svg *pbs, int x, int y, int w, int h, const unsigned char color[3], struct bb_error *perr)
{
    char hex_color[BG_SVG_HEX_COLOR_SIZE] = {0};
    char temp[512] = {0};
    int t, err_code;

    err_code = 0;

    if (!pbs)
        err_code = 3;
    else if ((w > 0) && (h > 0))
    {
        bg_svg_get_hex_color(color, hex_color);

        t = snprintf(temp, 512, "    <rect x=\"%d\" y=\"%d\" width=\"%d\" height=\"%d\" fill=\"%s\" />\n", 
                                    x, y, w, h, hex_color);
        if (t < 0)
            err_code = 7;
        else if (bb_buf_put(pbs->pbd, temp, (size_t) t) < 0)
            err_code = 1;
    }

    if (err_code && perr)
        bg_svg_set_error(err_code, perr);
    return err_code;
}

int bg_svg_draw_line(struct bg_svg *pbs, int x1, int y1, int x2, int y2, const unsigned char color[3], struct bb_error *perr)
{
    char hex_color[BG_SVG_HEX_COLOR_SIZE] = {0};
    char temp[256] = {0};
    int t, err_code;
    const char *C1 =        "    <path\n"
                            "        style=\"fill:none;stroke:";
    const size_t C1_LEN = 42;
    const char *C2 =        ";stroke-width:2;stroke-linecap:butt;stroke-linejoin:miter;stroke-opacity:1\"\n"
                            "        d=\"M";
    const size_t C2_LEN = 88;

    err_code = 0;

    if (!pbs)
        err_code = 3;
    else if ((x1 != x2) || (y1 != y2))
    {
        bg_svg_get_hex_color(color, hex_color);

        if ((bb_buf_put(pbs->pbd, C1, C1_LEN) < 0) || 
            (bb_buf_put(pbs->pbd, hex_color, BG_SVG_HEX_COLOR_SIZE - 1) < 0) || 
            (bb_buf_put(pbs->pbd, C2, C2_LEN) < 0))
            err_code = 1;
        else
        {
            if ((x1 == x2) && (y1 > y2))
            {
                t = y1;
                y1 = y2;
                y2 = t;
            }
            else if ((y1 == y2) && (x1 > x2))
            {
                t = x1;
                x1 = x2;
                x2 = t;
            }

            t = snprintf(temp, 256, " %d %d", x1, y1);
            if (t < 0)
                err_code = 7;
            else
            {
                if (bb_buf_put(pbs->pbd, temp, (size_t) t) < 0)
                    err_code = 1;
                else if (x1 == x2)
                {
                    t = snprintf(temp, 256, " V %d", y2);
                    if (t < 0)
                        err_code = 7;
                    else if (bb_buf_put(pbs->pbd, temp, (size_t) t) < 0)
                        err_code = 1;
                }
                else if (y1 == y2)
                {
                    t = snprintf(temp, 256, " H %d", x2);
                    if (t < 0)
                        err_code = 7;
                    else if (bb_buf_put(pbs->pbd, temp, (size_t) t) < 0)
                        err_code = 1;
                }
                else
                {
                    t = snprintf(temp, 256, " L %d %d", x2, y2);
                    if (t < 0)
                        err_code = 7;
                    else if (bb_buf_put(pbs->pbd, temp, (size_t) t) < 0)
                        err_code = 1;
                }

                if (!err_code && 
                    (bb_buf_put(pbs->pbd, "\" />\n", 5) < 0))
                    err_code = 1;
            }
        }
    }

    if (err_code && perr)
        bg_svg_set_error(err_code, perr);
    return err_code;
}

int bg_svg_draw_mpline(struct bg_svg *pbs, const int *coords, size_t points_n, _Bool points, const unsigned char color[3], struct bb_error *perr)
{
    char hex_color[BG_SVG_HEX_COLOR_SIZE] = {0};
    char temp[256] = {0};
    const int *p, *p2;
    int t, err_code;
    const char *C1 =        "    <path\n"
                            "        style=\"fill:none;stroke:";
    const size_t C1_LEN = 42;
    const char *C2 =        ";stroke-width:2;stroke-linecap:butt;stroke-linejoin:miter;stroke-opacity:1\"\n"
                            "        d=\"M";
    const size_t C2_LEN = 88;

    err_code = 0;

    if (!pbs || !coords || !points_n)
        err_code = 3;
    else if ((points_n > 1) || points)
    {
        bg_svg_get_hex_color(color, hex_color);

        if (points_n > 1)
        {
            if ((bb_buf_put(pbs->pbd, C1, C1_LEN) < 0) || 
                (bb_buf_put(pbs->pbd, hex_color, BG_SVG_HEX_COLOR_SIZE - 1) < 0) || 
                (bb_buf_put(pbs->pbd, C2, C2_LEN) < 0))
                err_code = 1;
            else
            {
                t = snprintf(temp, 256, " %d %d", *coords, *(coords + 1));
                if (t < 0)
                    err_code = 7;
                else if (bb_buf_put(pbs->pbd, temp, (size_t) t) < 0)
                    err_code = 1;
                else
                {
                    for (p = coords + 2, p2 = coords + 2 * points_n; (p < p2) && !err_code; p += 2)
                    {
                        t = snprintf(temp, 256, " L %d %d", *p, *(p + 1));
                        if (t < 0)
                            err_code = 7;
                        else if (bb_buf_put(pbs->pbd, temp, (size_t) t) < 0)
                            err_code = 1;
                    }

                    if (!err_code && 
                        (bb_buf_put(pbs->pbd, "\" />\n", 5) < 0))
                        err_code = 1;
                }
            }
        }

        if (!err_code && points)
            for (p = coords, p2 = p + 2 * points_n; (p < p2) && !err_code; p += 2)
            {
                t = snprintf(temp, 256, "    <circle cx=\"%d\" cy=\"%d\" r=\"" 
                                        BG_SVG_POINTS_R_STR 
                                        "\" fill=\"%s\"/>\n", *p, *(p + 1), hex_color);
                if (t < 0)
                    err_code = 7;
                else if (bb_buf_put(pbs->pbd, temp, (size_t) t) < 0)
                    err_code = 1;
            }
    }

    if (err_code && perr)
        bg_svg_set_error(err_code, perr);
    return err_code;
}

int bg_svg_draw_text(struct bg_svg *pbs, int x, int y, const char *text, size_t text_len, const unsigned char color[3], struct bb_error *perr)
{
    char hex_color[BG_SVG_HEX_COLOR_SIZE] = {0};
    char temp[256] = {0};
    int t, err_code;
/*
    const char *T1 =        "    <text\n"
                            "        style=\"font-size:11;line-height:125%;font-family:'DejaVu Sans Mono';"
                            "letter-spacing:0px;word-spacing:0px;fill:";
    const size_t T1_LEN = 127;
    const char *T2 =        ";stroke:none\"\n"
                            "        x=\"";
    const size_t T2_LEN = 25;
*/

    const char *T1 =        "    <text\n"
                            "        font-family=\"DejaVu Sans Mono\" font-size=\"15\" font-weight=\"bold\" fill=\"";
    const size_t T1_LEN = 89;
/*
    const char *T1 =        "    <text\n"
                            "        font-family=\"DejaVu Sans Mono\" font-size=\"16\" fill=\"";
    const size_t T1_LEN = 70;
*/
    const char *T2 =        "\" stroke=\"none\"\n"
                            "        x=\"";
    const size_t T2_LEN = 27;
    const char *T3 =        "</text>\n";
    const size_t T3_LEN = 8;

    err_code = 0;

    if (!pbs || !text)
        err_code = 3;
    else if (*text)
    {
        bg_svg_get_hex_color(color, hex_color);

        //t = snprintf(temp, 256, "%d\" y=\"%d\">", x, (y + 9));
        t = snprintf(temp, 256, "%d\" y=\"%d\">", x, (y + 12));
        if (t < 0)
            err_code = 7;
        else if ((bb_buf_put(pbs->pbd, T1, T1_LEN) < 0) || 
                (bb_buf_put(pbs->pbd, hex_color, BG_SVG_HEX_COLOR_SIZE - 1) < 0) || 
                (bb_buf_put(pbs->pbd, T2, T2_LEN) < 0) || 
                (bb_buf_put(pbs->pbd, temp, (size_t) t) < 0) || 
                (bb_buf_put(pbs->pbd, text, text_len) < 0) || 
                (bb_buf_put(pbs->pbd, T3, T3_LEN) < 0))
            err_code = 1;
    }

    if (err_code && perr)
        bg_svg_set_error(err_code, perr);
    return err_code;
}

int bg_svg_write(const struct bg_svg *pbs, const char *path, struct bb_error *perr)
{
    FILE *f;
    char *data;
    int err_code;

    data = NULL;
    err_code = 0;

    if (!pbs || !path)
        err_code = 3;
    else if (bb_buf_get(pbs->pbd, &data, NULL) < 0)
        err_code = 1;
    else
    {
//#ifdef BB_WIN
//        f = g_fopen(path, "wb");
//#else
        f = fopen(path, "wb");
//#endif
        if (!f)
            err_code = 5;
        else
        {
            //if ((fprintf(f, "<svg version=\"1.1\" width=\"%u\" height=\"%u\" baseProfile=\"full\" xmlns=\"http://www.w3.org/2000/svg\">\n"
            //                "    <rect width=\"100%%\" height=\"100%%\" fill=\"#ffffff\" />\n", pbs->w, pbs->h) < 0) || 
            //    (data && *data && (fprintf(f, "%s", data) < 0)) || 
            //    (fprintf(f, "</svg>\n") < 0))
            //    err_code = 6;
            if ((fprintf(f, "<svg version=\"1.1\" width=\"%u\" height=\"%u\" baseProfile=\"full\" xmlns=\"http://www.w3.org/2000/svg\">\n"
                            "    <rect width=\"%u\" height=\"%u\" fill=\"#ffffff\" />\n", pbs->w, pbs->h, pbs->w, pbs->h) < 0) || 
                (data && *data && (fprintf(f, "%s", data) < 0)) || 
                (fprintf(f, "</svg>\n") < 0))
                err_code = 6;

            fclose(f);
        }
    }

    if (data)
        free(data);

    if (err_code && perr)
        bg_svg_set_error(err_code, perr);
    return err_code;
}

void bg_svg_deinit(struct bg_svg **ppbs)
{
    if (ppbs && *ppbs)
    {
        bb_buf_deinit(&(*ppbs)->pbd);

        free(*ppbs);
        *ppbs = NULL;
    }
}

const char * bg_svg_error_message(int err_code)
{
    const char *BG_SVG_ERRS[BG_SVG_ERRS_N] = {  "OK",                                                       //  0
                                                "Ошибка выделения памяти",                                  //  1
                                                "Неизвестная ошибка",                                       //  2
                                                "Пустой указатель",                                         //  3
                                                "Ширина и высота изображения должны быть не равны нулю",    //  4
                                                "Не удалось открыть файл на запись",                        //  5
                                                "Ошибка записи файла",                                      //  6
                                                "Ошибка snprintf",                                          //  7
    };

    return *(BG_SVG_ERRS + (((err_code < 0) || (err_code >= BG_SVG_ERRS_N)) ? 2 : err_code));
}

static void bg_svg_get_hex_color(const unsigned char color[3], char hex_color[BG_SVG_HEX_COLOR_SIZE])
{
    const unsigned char *p, *p2;
    char *ph;
    int i;
    unsigned char u;

    *hex_color = '#';

    for (p = color, p2 = p + 3, ph = hex_color + 1; p < p2; ++p)
        for (i = 0; i < 2; ++i)
        {
            u = (!i ? (*p >> 4) : *p) & 0xf;
            *ph++ = (char) (u + ((u < (unsigned char) 10) ? 48 : 87));
        }

    *ph = '\0';
}

static void bg_svg_set_error(int err_code, struct bb_error *perr)
{
    perr->err_code = err_code;
    perr->err_str = bg_svg_error_message(err_code);
}
