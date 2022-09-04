#ifndef BB_IO_
#define BB_IO_

#include <stddef.h>
#include "bb_error.h"

/*  Allocates memory and reads file to that.  */
int bb_read_file(const char *file_name, char **pdata, size_t *plen, struct bb_error *perr);

/*  Writes data to file.  */
int bb_write_file(const char *file_name, const char *data, size_t len, struct bb_error *perr);

/*  Returns error string by error code.  */
const char * bb_io_error_message(int err_code);

#endif                                      // BB_IO_