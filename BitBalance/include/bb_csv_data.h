#ifndef BB_CSV_DATA_
#define BB_CSV_DATA_

#include <time.h>

struct bb_csv_node {                        // CSV-file data node.
    time_t date;                            // Date.
    double win_buy;                         // Winner Buy.
    double win_sel;                         // Winner Sell.
    double los_buy;                         // Loser Buy.
    double los_sel;                         // Loser Sell.
    double rate;                            // Rate.
    struct bb_csv_node *next;               // Next node pointer.
};

#endif                                      // BB_CSV_DATA_ 
