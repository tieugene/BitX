#ifndef BG_QUEUE_
#define BG_QUEUE_

#include "bb_error.h"

struct bg_queue_data;

/*  Allocates memory and initializes data structure.  */
int bg_queue_init(struct bg_queue_data **ppqd, struct bb_error *perr);

/*  Adds data to queue.  */
int bg_queue_add(struct bg_queue_data *pqd, void *data, struct bb_error *perr);

/*  Gets data from queue.  */
int bg_queue_get(struct bg_queue_data *pqd, void **pdata, struct bb_error *perr);

/*  Frees memory.  */
void bg_queue_deinit(struct bg_queue_data **ppqd);

/*  Returns error message string by error code.  */
const char * bg_queue_error_message(int err_code);

#endif                                      // BG_QUEUE_