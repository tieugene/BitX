#ifndef BB_TREE_
#define BB_TREE_

#include "bb_chart.h"
#include "bb_error.h"

struct bb_tree_data;

/*  Allocates memory and creates tree.  */
int bb_tree_get_tree(const char *file_name, struct bb_tree_data **pfirst, int *pyear, int *pmonth, int *pdate, size_t *pn, 
    struct bb_error *perr);

/*  Returns length of address in null-terminated string.  */
size_t bb_tree_get_address_len(const char *s);

/*  Returns 1, if tree contains that string, or 0, if not.  */
_Bool bb_tree_contains(const char *s, int64_t *psatoshi, const struct bb_tree_data *first);

/*  Frees memory.  */
int bb_tree_free(struct bb_tree_data **pfirst, struct bb_error *perr);

/*  Returns error string by error code.  */
const char * bb_tree_error_message(int err_code);

#endif                                      // BB_TREE_