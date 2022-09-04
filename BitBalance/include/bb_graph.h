#ifndef BB_GRAPH_
#define BB_GRAPH_

#include "bb_csv_data.h"
#include "bb_error.h"

/*  Creates graph and saves it to png-file.  */
void bb_graph_save_graph(const struct bb_csv_node *first, const char *file_name, struct bb_error *perr);

#endif                                      // BB_GRAPH_
