#ifndef BB_CLEANING_
#define BB_CLEANING_

#include <stddef.h>
#include "bb_error.h"

struct bb_cleaning_data;

struct bd_cleaning_date {
    int year;
    int month;
    int date;
    size_t n;
    struct bd_cleaning_date *next;
};

/*  Allocates memory and creates cleaning data structure.  */
int bb_cleaning_init(const char *file_name, struct bb_cleaning_data **ppcd, size_t *pn, struct bd_cleaning_date **pdates, struct bb_error *perr);

/*  Compares date1 and date2. Returns -1, if date1 < date2, 1 if date1 > date2, and 0 if date1 == date2.  */
int bb_cleaning_compare(int y1, int m1, int d1, int y2, int m2, int d2);

/*  Gets min date.  */
void bb_cleaning_get_min_date(const struct bb_cleaning_data *pcd, int *min_date_year, int *min_date_month, int *min_date_date);

/*  Gets number of addresses for each date.  */
int bd_cleaning_get_addresses_n(struct bb_cleaning_data *pcd, struct bd_cleaning_date *dates, struct bb_error *perr);

/*  Writes data for the date to file.  */
int bd_cleaning_write_data(const char *s_path, const struct bb_cleaning_data *pcd, int year, int month, int date, struct bb_error *perr);

/*  Frees memory.  */
int bb_cleaning_data_free(struct bb_cleaning_data **ppcd, struct bb_error *perr);

/*  Frees dates list memory.  */
void bd_cleaning_free_dates(struct bd_cleaning_date **pfirst);

/*  Returns error string by error code.  */
const char * bb_cleaning_error_message(int err_code);

#endif                                      // BB_CLEANING_