#ifndef BG_CLEANING_
#define BG_CLEANING_

#include "bb_error.h"

struct bb_cleaning_data;

/*  Allocates memory and creates cleaning data structure.  */
int bb_cleaning_init(const char *file_name, struct bb_cleaning_data **ppcd, size_t *pn, struct bb_error *perr);

/*  Compares date1 and date2. Returns -1, if date1 < date2, 1 if date1 > date2, and 0 if date1 == date2.  */
int bb_cleaning_compare(int y1, int m1, int d1, int y2, int m2, int d2);

/*  Gets min date.  */
void bb_cleaning_get_min_date(const struct bb_cleaning_data *pcd, int *min_date_year, int *min_date_month, int *min_date_date);

/*  Writes data for the date to file.  */
int bb_cleaning_write_data(const char *s_path, const struct bb_cleaning_data *pcd, int year, int month, int date, _Bool le, struct bb_error *perr);

/*  Frees memory.  */
int bb_cleaning_data_free(struct bb_cleaning_data **ppcd, struct bb_error *perr);

/*  Returns error string by error code.  */
const char * bb_cleaning_error_message(int err_code);

#endif                                      // BG_CLEANING_