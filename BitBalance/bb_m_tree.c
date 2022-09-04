#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <float.h>
#ifdef BB_WIN
#include <glib.h>
#include <glib/gstdio.h>
#endif
//#include "bb_tree.h"
//#include "bb_rates.h"
#include "bb_scan_double.h"
#include "bb_io.h"
//#include "bb_chart.h"
#include "bb_m_tree.h"

#define BB_M_TREE_ERRS_N 20                 // Number of errors.

#define BB_TRNS_N 3

struct bb_str_node {                        // String list node data.
    char *s;                                // String.
    struct bb_str_node *next;               // Next node pinter.
};

struct bb_m_tree {                          // M-file tree.
    char *address;                          // Address.
    int64_t satoshi;                        // Bitcoins balance in satoshi.
    int64_t last_day_satoshi;               // Last day bitcoins balance in satoshi.
    double dollars;                         // Dollars balance.
    double last_day_dollars;                // Dollars at the exchange rate on the last day.
    double balance;                         // Balance (dollars + last_day_dollars).
    size_t address_len;                     // Address length.
    struct bb_str_node *s_list;             // String list.
    struct bb_m_tree *left;                 // Left pointer.
    struct bb_m_tree *right;                // Right pointer.
};

struct bb_str_data {                        // Strings data.
    const char *s;                          // String.
    const struct bb_m_tree *node;           // Pointer to node containing string.
    struct bb_str_data *next;               // Next node pointer.
};

struct bb_m_data {                          // M-file data.
    struct bb_m_tree *first;                // M-Tree root.
    struct bb_str_data *psd;                // Strings data.
    size_t n;                               // Number of nodes in tree (and addresses in S-file.).
};

struct bb_m_tree_queue {                    // M-Tree queue data.
    struct bb_m_tree *node;                 // M-Tree node.
    struct bb_m_tree_queue *next;           // Pointer to next item of queue.
};

/*  Parses string and adds it to M-data.  */
static int bb_add_m_data(struct bb_m_data *pmd, char *s, size_t s_len, size_t address_len, struct bb_str_data ***pppsd, 
    struct bb_rates_data *prd, int64_t last_day_satoshi);

/*  Finds node with the address in tree, or creates new one, if it's not exist.  */
static int bb_get_m_tree_node(struct bb_m_tree **pfirst, size_t *pn, char *address, size_t address_len, struct bb_m_tree **pnode, 
    int64_t last_day_satoshi);

/*  Allocates memory and adds string to list.  */
static int bb_m_tree_add_str(struct bb_str_node **pfirst, const char *s, size_t s_len, char **ps);

/*  Allocates memory and adds node to list.  */
static int bb_str_data_add(const char *s, const struct bb_m_tree *node, struct bb_str_data ***pppsd);

/*  Get year, month, date and bitcoins from string.  */
static int bb_get_transaction_from_string(char *s, int *pyear, int *pmonth, int *pdate, double *pbitcoins);

/*  Frees tree memory.  */
static int bb_m_tree_free(struct bb_m_tree **pfirst);

/*  Calculates balances.  */
static int bb_m_tree_get_balances(struct bb_m_tree *first, double last_day_rate);

/*  Recursive writes balances.  */
static int bb_write_balance(FILE *f, struct bb_m_tree *node);

/*  Allocates memory and adds item to queue.  */
static int bb_m_tree_get_queue_item(struct bb_m_tree_queue **pq_first, struct bb_m_tree *node);

/* Sets error data. */
static void set_error(int err_code, struct bb_error *perr);

int bb_get_m_data(const char *file_name, double last_day_rate, struct bb_m_data **ppmd, struct bb_tree_data *ptd, struct bb_rates_data *prd, 
    size_t *pn, struct bb_error *perr)
{
    struct bb_str_data **ppsd;
    char *data, *p, *p2, c;
    int64_t last_day_satoshi;
    size_t len, address_len;
    int m, n, err_code;
    _Bool res;

    data = NULL;
    len = 0;
    err_code = 0;

    if (!file_name || !ppmd || !pn)
        err_code = 3;
    else
    {
        *pn = 0;
        *ppmd = (struct bb_m_data *) calloc(1, sizeof(struct bb_m_data));
        if (!*ppmd)
            err_code = 1;
        else if (!bb_read_file(file_name, &data, &len, perr))
        {
            ppsd = &(*ppmd)->psd;
            (*ppmd)->n = 0;

            n = 0;
            for (p = data; *p && !err_code; )
            {
                while ((' ' == *p) || ('\n' == *p) || ('\r' == *p) || (',' == *p))
                    ++p;

                if (*p)
                {
                    if ('{' == *p)
                    {
                        ++n;
                        ++p;
                    }
                    else if ('}' == *p)
                    {
                        if (--n < 0)
                            err_code = 4;
                        else
                            ++p;
                    }
                    else
                    {
                        if (!n)
                        {
                            p2 = p;
                            p = data + len;
                        }
                        else
                        {
                            m = n;
                            p2 = p++;

                            while (*p)
                            {
                                if ('{' == *p)
                                    ++n;
                                else if ('}' == *p)
                                {
                                    if (--n < m)
                                        break;
                                }
                                ++p;
                            }

                            if (n >= m)
                                err_code = 4;
                        }

                        if (!err_code)
                        {
                            c = *p;
                            *p = '\0';
                            //puts(p2);
                            address_len = bb_tree_get_address_len(p2);
                            *p = c;

                            c = *(p2 + address_len);
                            *(p2 + address_len) = '\0';
                            last_day_satoshi = (int64_t) 0;
                            //puts(p2);
                            res = bb_tree_contains(p2, &last_day_satoshi, ptd);
                            *(p2 + address_len) = c;

                            if (res)
                                err_code = bb_add_m_data(*ppmd, p2, (p - p2), address_len, &ppsd, prd, last_day_satoshi);

                            if (*p)
                                ++p;

                            (*pn)++;
                        }
                    }
                }
            }
        }
    }

    if (data)
        free(data);

    if (!err_code)
        err_code = bb_m_tree_get_balances((*ppmd)->first, last_day_rate);

    if (err_code)
        bb_free_m_data(ppmd, NULL);

    if (err_code && perr)
        set_error(err_code, perr);
    return err_code;
}

static int bb_add_m_data(struct bb_m_data *pmd, char *s, size_t s_len, size_t address_len, struct bb_str_data ***pppsd, 
    struct bb_rates_data *prd, int64_t last_day_satoshi)
{
    struct bb_m_tree *node;
    char *s2, *ns, *p, *p2, c, c2;
    double bitcoins, rate;
    int year, month, date, m, n, err_code;

    node = NULL;
    s2 = NULL;
    err_code = 0;

    c = *(s + address_len);
    *(s + address_len) = '\0';
    err_code = bb_get_m_tree_node(&pmd->first, &pmd->n, s, address_len, &node, last_day_satoshi);
    *(s + address_len) = c;
    if (!err_code)
    {
        err_code = bb_m_tree_add_str(&node->s_list, s, s_len, &s2);
        if (!err_code)
        {
            err_code = bb_str_data_add(s2, node, pppsd);
            if (!err_code)
            {
                c = *(s + s_len);
                *(s + s_len) = '\0';

                ns = s + address_len;
                while ((',' == *ns) || (' ' == *ns) || ('\n' == *ns) || ('\r' == *ns))
                    ++ns;

                if (!*ns)
                    err_code = 5;
                else
                {
                    n = 0;
                    for (p = ns; *p && !err_code; )
                    {
                        while ((' ' == *p) || ('\n' == *p) || ('\r' == *p) || (',' == *p))
                            ++p;

                        if (*p)
                        {
                            if ('{' == *p)
                            {
                                ++n;
                                ++p;
                            }
                            else if ('}' == *p)
                            {
                                if (--n < 0)
                                    err_code = 6;
                                else
                                    ++p;
                            }
                            else
                            {
                                if (!n)
                                {
                                    p2 = p;
                                    p = s + s_len;
                                }
                                else
                                {
                                    m = n;
                                    p2 = p++;

                                    while (*p)
                                    {
                                        if ('{' == *p)
                                            ++n;
                                        else if ('}' == *p)
                                        {
                                            if (--n < m)
                                                break;
                                        }
                                        ++p;
                                    }

                                    if (n >= m)
                                        err_code = 6;
                                }

                                if (!err_code)
                                {
                                    c2 = *p;
                                    *p = '\0';
                                    //puts(p2);
                                    year = month = date = 0;
                                    bitcoins = rate = (double) 0;
                                    err_code = bb_get_transaction_from_string(p2, &year, &month, &date, &bitcoins);
                                    if (!err_code)
                                    {
                                        if (bb_rates_get_rate(prd, year, month, date, &rate, NULL))
                                        {
                                            //printf("Год %d, месяц %d, число %d.\n", year, month, date);
                                            err_code = 15;
                                        }
                                        else
                                        {
                                            node->satoshi += llround(bitcoins * (double) BB_SATOSHI_PER_BITCOIN);
                                            node->dollars -= bitcoins * rate;
                                            //node->last_day_dollars += bitcoins * last_day_rate;
                                        }
                                    }
                                    *p = c2;

                                    if (*p)
                                        ++p;
                                }
                            }
                        }
                    }
                }

                *(s + s_len) = c;
            }
        }
    }

    return err_code;
}

static int bb_get_m_tree_node(struct bb_m_tree **pfirst, size_t *pn, char *address, size_t address_len, struct bb_m_tree **pnode, 
    int64_t last_day_satoshi)
{
    struct bb_m_tree **pcurrent;
    int n, err_code;

    err_code = 0;

    for (pcurrent = pfirst; *pcurrent; )
    {
        n = strcmp(address, (*pcurrent)->address);
        if (!n)
            break;
        else
            pcurrent = (n < 0) ? &(*pcurrent)->left : &(*pcurrent)->right;
    }

    if (!*pcurrent)
    {
        *pcurrent = (struct bb_m_tree *) malloc(sizeof(struct bb_m_tree) + address_len + 1);
        if (!*pcurrent)
            err_code = 1;
        else
        {
            //memset(*pcurrent, 0, sizeof(struct bb_m_tree) + address_len + 1);

            (*pcurrent)->address = (char *) (*pcurrent + 1);
            strncpy((*pcurrent)->address, address, address_len + 1);
            *((*pcurrent)->address + address_len) = '\0';

            (*pcurrent)->address_len = address_len;

            (*pcurrent)->last_day_satoshi = last_day_satoshi;

            (*pcurrent)->left = (*pcurrent)->right = NULL;

            (*pcurrent)->satoshi = (int64_t) 0;
            (*pcurrent)->dollars = (double) 0;
            (*pcurrent)->last_day_dollars = (double) 0;
            (*pcurrent)->balance = (double) 0;
            (*pcurrent)->s_list = NULL;

            (*pn)++;
        }
    }

    *pnode = *pcurrent;

    return err_code;
}

static int bb_m_tree_add_str(struct bb_str_node **pfirst, const char *s, size_t s_len, char **ps)
{
    struct bb_str_node **pcurrent;

    for (pcurrent = pfirst; *pcurrent; pcurrent = &(*pcurrent)->next)
        continue;

    *pcurrent = (struct bb_str_node *) malloc(sizeof(struct bb_str_node) + s_len + 1);
    if (*pcurrent)
    {
        //memset(*pcurrent, 0, sizeof(struct bb_str_node) + s_len + 1);

        (*pcurrent)->s = (char *) (*pcurrent + 1);
        strncpy((*pcurrent)->s, s, s_len + 1);
        *((*pcurrent)->s + s_len) = '\0';

        *ps = (*pcurrent)->s;

        (*pcurrent)->next = NULL;
    }

    return (!*pcurrent) ? 1 : 0;
}

static int bb_str_data_add(const char *s, const struct bb_m_tree *node, struct bb_str_data ***pppsd)
{
    int err_code;

    err_code = 0;
    **pppsd = (struct bb_str_data *) calloc(1, sizeof(struct bb_str_data));
    if (!**pppsd)
        err_code = 1;
    else
    {
        (**pppsd)->s = s;
        (**pppsd)->node = node;
        *pppsd = &(**pppsd)->next;
    }

    return err_code;
}

static int bb_get_transaction_from_string(char *s, int *pyear, int *pmonth, int *pdate, double *pbitcoins)
{
    char *p;
    char *pp[BB_TRNS_N] = {0};
    int i, err_code;

    err_code = 0;
    for (p = s, i = 0; *p && (i < BB_TRNS_N); ++p)
        if (',' == *p)
        {
            *p++ = '\0';
            while ((' ' == *p) && ('\n' == *p) && ('\r' == *p))
                ++p;
            if (!*p)
                break;
            else
                *(pp + i++) = p;
        }

    if (i < BB_TRNS_N)
        err_code = 7;
    else if (!bb_scan_double(s, pbitcoins))
        err_code = 8;
    else if (sscanf(*pp, "%d", pyear) != 1)
        err_code = 9;
    else if (*pyear < 0)
        err_code = 10;
    else if (sscanf(*(pp + 1), "%d", pmonth) != 1)
        err_code = 11;
    else if ((*pmonth <= 0) || (*pmonth > BB_MONTHS_PER_YEAR))
        err_code = 12;
    else if (sscanf(*(pp + 2), "%d", pdate) != 1)
        err_code = 13;
    else if ((*pdate <= 0) || (*pdate > BB_DAYS_PER_MONTH))
        err_code = 14;

    return err_code;
}

static int bb_m_tree_get_balances(struct bb_m_tree *first, double last_day_rate)
{
    struct bb_m_tree_queue *q_first, *q_item;
    int err_code;

    q_first = NULL;
    err_code = 0;

    if (first)
    {
        err_code = bb_m_tree_get_queue_item(&q_first, first);
        while (q_first && !err_code)
        {
            q_item = q_first;
            q_first = q_first->next;
            if (q_item->node->left)
                err_code = bb_m_tree_get_queue_item(&q_first, q_item->node->left);
            if (q_item->node->right && !err_code)
                err_code = bb_m_tree_get_queue_item(&q_first, q_item->node->right);

            q_item->node->last_day_dollars = q_item->node->satoshi / (double) BB_SATOSHI_PER_BITCOIN * last_day_rate;
            q_item->node->balance = q_item->node->dollars + q_item->node->last_day_dollars;

            free(q_item);
        }
    }

    if (err_code)
        while (q_first)
        {
            q_item = q_first;
            q_first = q_first->next;
            free(q_item);
        }

    return err_code;
}

int bb_write_m_data(const char *file_name, const struct bb_m_data *pmd, enum bb_write_modes nbs, enum bb_write_modes wl, struct bb_error *perr)
{
    FILE *f;
    const struct bb_str_data *psd;
    int err_code;
    _Bool fs;

    fs = 0;
    err_code = 0;

    if (!file_name || !pmd)
        err_code = 3;
    else
    {
#ifdef BB_WIN
        f = g_fopen(file_name, "wb");
#else
        f = fopen(file_name, "wb");
#endif

        if (!f)
            err_code = 16;
        else
        {
            if (fprintf(f, "%c", '{') < 0)
                err_code = 17;

            for (psd = pmd->psd; psd && !err_code; psd = psd->next)
            {
                if (((BB_WRITE_ALL == nbs) || 
                    ((BB_WRITE_NOT == nbs) && ((int64_t) 0 == psd->node->last_day_satoshi)) || 
                    ((BB_WRITE_BUY == nbs) && (psd->node->last_day_satoshi > (int64_t) 0)) || 
                    ((BB_WRITE_SEL == nbs) && (psd->node->last_day_satoshi < (int64_t) 0))) 
                    && 
                    ((BB_WRITE_ALL == wl) || 
                    //((BB_WRITE_NOT == wl) && (psd->node->balance >= -0.0) && (psd->node->balance <= +0.0)) || 
                    //((BB_WRITE_WIN == wl) && (psd->node->balance > +0.0)) || 
                    //((BB_WRITE_LOS == wl) && (psd->node->balance < -0.0))))
                    ((BB_WRITE_NOT == wl) && (psd->node->balance >= -BB_D) && (psd->node->balance <= +BB_D)) || 
                    ((BB_WRITE_WIN == wl) && (psd->node->balance > +BB_D)) || 
                    ((BB_WRITE_LOS == wl) && (psd->node->balance < -BB_D))))
                {
                    if (!fs)
                    {
                        if (fprintf(f, "{%s}", psd->s) < 0)
                            err_code = 17;
                        else
                            fs = 1;
                    }
                    else
                    {
                        if (fprintf(f, ",{%s}", psd->s) < 0)
                            err_code = 17;
                    }
                }
            }

            if (!err_code && (fprintf(f, "%c", '}') < 0))
                err_code = 17;

            fclose(f);
        }
    }

    if (err_code && perr)
        set_error(err_code, perr);
    return err_code;
}

int bb_write_balances(const char *file_name, const struct bb_m_data *pmd, struct bb_error *perr)
{
    FILE *f;
    int err_code;

    err_code = 0;

    if (!file_name || !pmd)
        err_code = 3;
    else
    {
#ifdef BB_WIN
        f = g_fopen(file_name, "wb");
#else
        f = fopen(file_name, "wb");
#endif

        if (!f)
            err_code = 16;
        //else if (fprintf(f, "Address;Bitcoins;Dollars;LastDayDollars;Balance;Buy/Sell/Nothing\n") < 0)
        else
        {
            if (fprintf(f, "Address;valuewalletBTC;valuewallet$;LastDayDollars;tradingbalance;Buy/Sell/Nothing\n") < 0)
                err_code = 17;
            else
                err_code = bb_write_balance(f, pmd->first);

            fclose(f);
        }
    }

    if (err_code && perr)
        set_error(err_code, perr);
    return err_code;
}

static int bb_write_balance(FILE *f, struct bb_m_tree *node)
{
    int err_code;

    err_code = 0;

    if (node->left)
        err_code = bb_write_balance(f, node->left);

    //if (!err_code && (fprintf(f, "%s;%.3lf;%.3lf;%.3lf;%.3lf;%s\n", 
    if (!err_code && (fprintf(f, "%s;%.3lf;%.3lf;%.3lf;%.3lf;%.3lf\n", 
                                node->address, 
                                (double) node->satoshi / (double) BB_SATOSHI_PER_BITCOIN, 
                                node->dollars, 
                                node->last_day_dollars, 
                                node->balance, 
                                //((int64_t) 0 == node->last_day_satoshi) ? "not" : (node->last_day_satoshi > (int64_t) 0) ? "buy" : "sel") < 0))
                                (double) node->last_day_satoshi) / (double) BB_SATOSHI_PER_BITCOIN))
        err_code = 17;

    if (!err_code && node->right)
        err_code = bb_write_balance(f, node->right);

    return err_code;
}

int bb_get_chart_data(struct bb_m_data *pmd, struct bb_chart_data *pcd, struct bb_error *perr)
{
    struct bb_m_tree_queue *q_first, *q_item;
    struct bb_balance_data **p, **p2;
    int err_code;

    q_first = NULL;
    err_code = 0;

    if (!pmd || !pcd)
        err_code = 3;
    else if (pmd->n && pmd->first)
    {
        pcd->balances = (struct bb_balance_data **) calloc(pmd->n, sizeof(struct bb_balance_data *));
        if (!pcd->balances)
            err_code = 1;
        else
        {
            pcd->balances_n = pmd->n;

            pcd->max_buy_balance = pcd->max_sel_balance = -DBL_MAX;
            pcd->min_buy_balance = pcd->min_sel_balance = DBL_MAX;
            pcd->los_buy_balances_n = pcd->win_buy_balances_n = pcd->los_sel_balances_n = pcd->win_sel_balances_n = 0;

            err_code = bb_m_tree_get_queue_item(&q_first, pmd->first);
            for (p = pcd->balances, p2 = p + pcd->balances_n; q_first && !err_code && (p < p2); ++p)
            {
                q_item = q_first;
                q_first = q_first->next;
                if (q_item->node->left)
                    err_code = bb_m_tree_get_queue_item(&q_first, q_item->node->left);
                if (q_item->node->right && !err_code)
                    err_code = bb_m_tree_get_queue_item(&q_first, q_item->node->right);

                bb_get_balance_data(pcd, q_item->node->address, q_item->node->address_len, q_item->node->satoshi, 
                    q_item->node->dollars, q_item->node->last_day_dollars, q_item->node->balance, q_item->node->last_day_satoshi, p);
                if (!*p)
                    err_code = 1;

                free(q_item);
            }

            if (!err_code)
            {
                if (p < p2)
                    err_code = 18;
                else if (q_first)
                    err_code = 19;
            }

            if (!err_code)
            {
                pcd->min_buy_balance_s_len = bb_get_balance_s_len(pcd->min_buy_balance);
                pcd->max_buy_balance_s_len = bb_get_balance_s_len(pcd->max_buy_balance);
                pcd->min_sel_balance_s_len = bb_get_balance_s_len(pcd->min_sel_balance);
                pcd->max_sel_balance_s_len = bb_get_balance_s_len(pcd->max_sel_balance);
            }
        }
    }

    if (err_code)
        while (q_first)
        {
            q_item = q_first;
            q_first = q_first->next;
            free(q_item);
        }
    else
        qsort((void *) pcd->balances, pmd->n, sizeof(struct bb_balance_data *), bb_balance_data_compare);

    if (err_code && perr)
        set_error(err_code, perr);
    return err_code;
}

size_t bb_get_nodes_n(struct bb_m_data *pmd)
{
    return (pmd ? pmd->n : 0);
}

int bb_free_m_data(struct bb_m_data **ppmd, struct bb_error *perr)
{
    struct bb_str_data **ppsd, *node;
    int err_code;

    err_code = 0;

    if (!ppmd)
        err_code = 3;
    else if (*ppmd)
    {
        for (ppsd = &(*ppmd)->psd; *ppsd; ) // Free strings data.
        {
            node = *ppsd;
            *ppsd = (*ppsd)->next;
            free(node);
        }

        err_code = bb_m_tree_free(&(*ppmd)->first);

        free(*ppmd);
        *ppmd = NULL;
    }

    if (err_code && perr)
        set_error(err_code, perr);
    return err_code;
}

static int bb_m_tree_free(struct bb_m_tree **pfirst)
{
    struct bb_m_tree_queue *q_first, *q_item;
    struct bb_str_node *s_node, *s_prev;
    int err_code;

    q_first = NULL;
    err_code = 0;

    if (!pfirst)
        err_code = 3;
    else if (*pfirst)
    {
        err_code = bb_m_tree_get_queue_item(&q_first, *pfirst);
        if (!err_code)
        {
            *pfirst = NULL;
            while (q_first && !err_code)
            {
                q_item = q_first;
                q_first = q_first->next;
                if (q_item->node->left)
                    err_code = bb_m_tree_get_queue_item(&q_first, q_item->node->left);
                if (q_item->node->right && !err_code)
                    err_code = bb_m_tree_get_queue_item(&q_first, q_item->node->right);

                for (s_node = q_item->node->s_list; s_node; )
                {
                    s_prev = s_node;
                    s_node = s_node->next;
                    free(s_prev);
                }

                free(q_item->node);
                free(q_item);
            }
        }
    }

    return err_code;
}

static int bb_m_tree_get_queue_item(struct bb_m_tree_queue **pq_first, struct bb_m_tree *node)
{
    struct bb_m_tree_queue *q_item;
    int err_code;

    err_code = 0;

    //q_item = (struct bb_m_tree_queue *) malloc(sizeof(struct bb_m_tree_queue));
    q_item = (struct bb_m_tree_queue *) calloc(sizeof(struct bb_m_tree_queue), 1);
    if (!q_item)
        err_code = 1;
    else
    {
        q_item->node = node;
        q_item->next = *pq_first;
        *pq_first = q_item;
    }

    return err_code;
}

const char * bb_m_data_error_message(int err_code)
{
    static const char *BB_M_TREE_ERRS[BB_M_TREE_ERRS_N] = { "ОК",                                                       //  0
                                                            "Ошибка выделения памяти",                                  //  1
                                                            "Неизвестная ошибка",                                       //  2
                                                            "Пустой указатель",                                         //  3
                                                            "Ошибка в данных M-файла",                                  //  4
                                                            "Строка не содержит транзакций",                            //  5
                                                            "Ошибка в строке транзакций",                               //  6
                                                            "В строке транзакции меньше элементов, чем необходимо",     //  7
                                                            "Не удалось получить сумму из строки транзакции",           //  8
                                                            "Не удалось получить год из строки транзакции",             //  9
                                                            "Некорректное значение года в строке транзакции",           // 10
                                                            "Не удалось получить месяц из строки транзакции",           // 11
                                                            "Некорректное значение месяца в строке транзакции",         // 12
                                                            "Не удалось получить число (дату) из строки транзакции",    // 13
                                                            "Некорректное значение числа (даты) в строке транзакции",   // 14
                                                            "Нет курса для заданной даты",                              // 15
                                                            "Ошибка при открытии файла для записи",                     // 16
                                                            "Ошибка записи в файл",                                     // 17
                                                            "В дереве меньше узлов, чем ожидалось",                     // 18
                                                            "В дереве больше узлов, чем ожидалось",                     // 19
    };

    if ((err_code < 0) || (err_code >= BB_M_TREE_ERRS_N))
        err_code = 2;

    return *(BB_M_TREE_ERRS + err_code);
}

static void set_error(int err_code, struct bb_error *perr)
{
    perr->err_code = err_code;
    perr->err_str = bb_m_data_error_message(err_code);
}
