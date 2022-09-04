#include <stdlib.h>
#include <string.h>
#include "bb_buf_data.h"

#define BD_DEFAULT_BUF_SIZE 1024                        // Default buffer size.

struct bb_buf_data {                                    // Buffer data structure.
    char *buf;                                          // Buffer.
    char *p;                                            // Current position pointer.
    size_t len;                                         // Buffer length (not including '\0').
    size_t step;                                        // Size step by which buffer increases when out of memory.
};

int bb_buf_init(struct bb_buf_data **ppbd, size_t size)
{
    if (ppbd)
    {
        *ppbd = (struct bb_buf_data *) calloc(1, sizeof(struct bb_buf_data));
        if (*ppbd)
        {
            if (bb_buf_reset(*ppbd) < 0)
            {
                free(*ppbd);
                *ppbd = NULL;
            }
            else
                (*ppbd)->step = size ? size : BD_DEFAULT_BUF_SIZE;
        }
    }

    return (!ppbd || !*ppbd) ? -1 : 0;
}

int bb_buf_reset(struct bb_buf_data *pbd)
{
    if (pbd)
    {
        if (pbd->buf)
            free((pbd->buf));
        pbd->buf = (char *) calloc(1, 1);

        pbd->p = pbd->buf;

        pbd->len = 0;
    }

    return (!pbd || !pbd->buf) ? -1 : 0;
}

int bb_buf_put(struct bb_buf_data *pbd, const char *s, size_t s_len)
{
    size_t m, n;

    if (pbd && s)
    {
        if (!s_len)
            s_len = strlen(s);

        if (s_len)
        {
            n = pbd->p - pbd->buf;
            if (s_len > (pbd->len - n))
            {
                m = n + s_len;
                while (pbd->len < m)
                    pbd->len += pbd->step;

                pbd->buf = realloc(pbd->buf, pbd->len + 1);
                if (pbd->buf)
                    pbd->p = pbd->buf + n;
            }

            if (pbd->buf)
            {
                memcpy(pbd->p, s, s_len);
                pbd->p += s_len;
                *pbd->p = '\0';
            }
        }
    }

    return (!pbd || !pbd->buf) ? -1 : 0;
}

int bb_buf_get(struct bb_buf_data *pbd, char **pbuf, size_t *pbuf_len)
{
    int err_code;

    err_code = 0;

    if (pbd && pbuf)
    {
        *pbuf = pbd->buf;
        if (pbuf_len)
            *pbuf_len = pbd->p - pbd->buf;

        pbd->buf = pbd->p = NULL;
        pbd->len = 0;

        err_code = bb_buf_reset(pbd);
        if (err_code)
        {
            if (*pbuf)
            {
                free(*pbuf);
                *pbuf = NULL;
            }

            if (pbuf_len)
                *pbuf_len = 0;
        }
    }
    else
        err_code = -1;

    return err_code;
}

void bb_buf_deinit(struct bb_buf_data **ppbd)
{
    if (ppbd && *ppbd)
    {
        if ((*ppbd)->buf)
            free((*ppbd)->buf);

        free(*ppbd);
        *ppbd = NULL;
    }
}