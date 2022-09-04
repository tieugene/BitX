#ifndef BB_CSV_ITER_
#define BB_CSV_ITER_

#include "bb_csv_data.h"
#include "bb_error.h"

/*  Allocates memory and creates csv file data list.  */
int bb_csv_get_list(const char *file_name, struct bb_csv_node **pfirst, const struct bb_csv_node *pcn, struct bb_error *perr);

/*  Frees memory.  */
void bb_csv_free_list(struct bb_csv_node **pfirst);

#endif                                      // BB_CSV_ITER_