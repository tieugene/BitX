#ifndef BG_TS_ERROR_
#define BG_TS_ERROR_

#include <stdio.h>
#include "bb_error.h"

struct bg_ts_error;

/*  Allocates memory and initializes data structure.  */
int bg_ts_error_init(struct bg_ts_error **pptse, FILE *log_f, struct bb_error *perr);

/*  Sets error data.  */
void bg_ts_error_set_error_data(struct bg_ts_error *ptse, struct bb_error *perr_data, const char *path, const char *msg);

/*  Gets error data.  */
void bg_ts_error_get_error_data(struct bg_ts_error *ptse, struct bb_error *perr_data);

/*  Gets error code.  */
int bg_ts_error_get_error_code(struct bg_ts_error *ptse);

/*  Frees memory.  */
void bg_ts_error_deinit(struct bg_ts_error **pptse);

/*  Returns error message string by error code.  */
const char * bg_ts_error_error_message(int err_code);

#endif                                      // BG_TS_ERROR_