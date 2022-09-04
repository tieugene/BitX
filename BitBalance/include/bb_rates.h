#ifndef BB_RATES_
#define BB_RATES_

#include "bb_error.h"

#define BB_DAYS_PER_MONTH 31
#define BB_MONTHS_PER_YEAR 12

struct bb_rates_data;

/*  Allocates memory and fills that with rates data.  */
int bb_rates_get_rates(const char *file_name, struct bb_rates_data **pprd, struct bb_error *perr);

/*  Gets rate for date.  */
int bb_rates_get_rate(struct bb_rates_data *prd, int year, int month, int date, double *prate, struct bb_error *perr);

/*  Gets last rate.  */
//int bb_rates_get_last_rate(struct bb_rates_data *prd, double *prate, struct bb_error *perr);

/*  Frees rates data memory.  */
void bb_rates_free_rates(struct bb_rates_data **pprd);

/*  Returns error string by error code.  */
const char * bb_rates_error_message(int err_code);

#endif                                      // BB_RATES_