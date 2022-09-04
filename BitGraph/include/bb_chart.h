#ifndef BB_CHART_
#define BB_CHART_

#include <stdint.h>
#include "bb_error.h"

#define BB_SATOSHI_PER_BITCOIN 100000000
#define BB_D 0.0001

enum bb_write_modes {
    BB_WRITE_NOT = 0, 
    BB_WRITE_BUY = 1, 
    BB_WRITE_SEL = 2, 
    BB_WRITE_WIN = 3, 
    BB_WRITE_LOS = 4, 
    BB_WRITE_ALL = 5, 
};

struct bb_balance_data {                    // Balance data.
    char *address;                          // Address.
    int64_t satoshi;                        // Bitcoins balance in satoshi.
    double dollars;                         // Dollars balance.
    double last_day_dollars;                // Dollars at the exchange rate on the last day.
    double balance;                         // $ balance.
    size_t address_len;                     // Address length.
    int64_t last_day_satoshi;               // Last day bitcoins balance in satoshi.
};

struct bb_chart_data {
    struct bb_balance_data **balances;      // Balances.
    size_t balances_n;                      // Number of balances.
    double min_buy_balance;                 // Min balance for last day buyer (loss).
    size_t min_buy_balance_s_len;           // Length of min balance for last day buyer (loss) string.
    size_t los_buy_max_address_len;         // Max address length for loss buyers.
    size_t los_buy_balances_n;              // Number of balances for loss buyers.
    double max_buy_balance;                 // Max balance for last day buyer (win).
    size_t max_buy_balance_s_len;           // Length of max balance for last day buyer (win) string.
    size_t win_buy_max_address_len;         // Max address length for win buyers.
    size_t win_buy_balances_n;              // Number of balances for win buyers.
    double min_sel_balance;                 // Min balance for last day seller (loss).
    size_t min_sel_balance_s_len;           // Length of min balance for last day seller (loss) string.
    size_t los_sel_max_address_len;         // Max address length for loss sellers.
    size_t los_sel_balances_n;              // Number of balances for loss sellers.
    double max_sel_balance;                 // Max balance for last day seller (win).
    size_t max_sel_balance_s_len;           // Length of max balance for last day seller (win) string.
    size_t win_sel_max_address_len;         // Max address length for win sellers.
    size_t win_sel_balances_n;              // Number of balances for win sellers.
};

/*  Allocates memory and fill it.  */
void bb_get_balance_data(struct bb_chart_data *pcd, const char *address, size_t address_len, int64_t satoshi, 
    double dollars, double last_day_dollars, double balance, int64_t last_day_satoshi, struct bb_balance_data **ppbd);

/*  Compare function for qsort.  */
int bb_balance_data_compare(const void *pbd1, const void *pbd2);

/*  Returns balance string length.  */
size_t bb_get_balance_s_len(double balance);

/*  Save balances and data to file.  */
int bb_write_chart_data(const char *file_name, struct bb_chart_data *chart, double last_day_rate, struct bb_error *perr);

/*  Save data to bar chart png.  */
int bb_write_chart_png(const char *file_name, struct bb_chart_data *chart, enum bb_write_modes nbs, enum bb_write_modes wl, 
    struct bb_error *perr);

/*  Gets nodes number for each category.  */
void bb_chart_get_n(const struct bb_chart_data *chart, size_t *pwb_n, size_t *pws_n, size_t *plb_n, size_t *pls_n);

/*  Frees memory.  */
void bb_free_chart_data(struct bb_chart_data *pcd);

/*  Returns error string by error code.  */
const char * bb_chart_error_message(int err_code);

#endif                                      // BB_CHART_