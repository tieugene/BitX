#ifndef BG_DATA_
#define BG_DATA_

//#include <stddef.h>
#include <inttypes.h>
#include <time.h>

struct bg_data_node {
/*  date  */
    time_t date;                            // Date.
/*  buy/sell  */
    size_t buy_n;                           // Number of buyers.
    size_t sel_n;                           // Number of sellers.
/*  win/lose  */
    size_t win_n;                           // Number of winners.
    size_t los_n;                           // Number of losers.
/*  win/lose && buy/sell  */
    size_t win_buy_n;                       // Number of winners-buyers.
    size_t win_sel_n;                       // Number of winners-sellers.
    size_t los_buy_n;                       // Number of losers-buyers.
    size_t los_sel_n;                       // Number of losers-sellers.
/*  last day dollars  */
    int64_t dol_buy_n;                      // Dollars * 1000, bought on last day.
    int64_t dol_sel_n;                      // Dollars * 1000, sold on last day.
/*  last day bitcoins  */
    int64_t btc_buy_n;                      // Bitcoins in satoshi (* 100 000 000), bought on last day.
    int64_t btc_sel_n;                      // Bitcoins in satoshi (* 100 000 000), sold on last day.
/*  win/lose && buy/sell && last day dollars/last day bitcoins  */
    int64_t win_btc_buy_n;                  // Bitcoins in satoshi (* 100 000 000), bought on last day by winners.
    int64_t win_btc_sel_n;                  // Bitcoins in satoshi (* 100 000 000), sold on last day by winners.
    int64_t win_dol_buy_n;                  // Dollars * 1000, bought on last day by winners.
    int64_t win_dol_sel_n;                  // Dollars * 1000, sold on last day by winners.
    int64_t los_btc_buy_n;                  // Bitcoins in satoshi (* 100 000 000), bought on last day by losers.
    int64_t los_btc_sel_n;                  // Bitcoins in satoshi (* 100 000 000), sold on last day by losers.
    int64_t los_dol_buy_n;                  // Dollars * 1000, bought on last day by losers.
    int64_t los_dol_sel_n;                  // Dollars * 1000, sold on last day by losers.
/*  rate  */
    double rate;                            // Bitcoin rate in dollars.
/*  next node  */
    struct bg_data_node *next;              // Next node pointer.
};

#endif                                      // BG_DATA_