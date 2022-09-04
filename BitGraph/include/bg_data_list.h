#ifndef BG_DATA_LIST_
#define BG_DATA_LIST_

#include "bb_error.h"
#include "bg_data.h"

struct bg_data_list;

/*  Allocates memory and initializes data structure.  */
int bg_data_list_init(struct bg_data_list **ppdl, struct bb_error *perr);

/*  Adds data to data list.  */
int bg_data_list_add(struct bg_data_list *pdl, const struct bg_data_node *pd, struct bb_error *perr);

/*  Gets data list.  */
int bg_data_list_get(struct bg_data_list *pdl, struct bg_data_node **pfirst, size_t *pn, struct bb_error *perr);

/*  Frees memory.  */
void bg_data_list_deinit(struct bg_data_list **ppdl);

/*  Writes list data to file.  */
int bg_data_list_write_list(const char *path, const struct bg_data_node *first, struct bb_error *perr);

/*  Frees list memory.  */
void bg_data_list_free_list(struct bg_data_node **pfirst);

/*  Returns error message string by error code.  */
const char * bg_data_list_error_message(int err_code);

#endif                                      // BG_DATA_LIST_
