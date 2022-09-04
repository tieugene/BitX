#ifndef BG_GRAPH_
#define BG_GRAPH_

#include "bg_data.h"
#include "bb_error.h"

struct bg_graph_rates_data;

/*  Allocates memory and initializes rates data structure.  */
int bg_graph_rates_data_init(const struct bg_data_node *pd, size_t n, struct bg_graph_rates_data **pprd, int rid, struct bb_error *perr);

/*  Frees rates data memory.  */
void bg_graph_rates_data_deinit(struct bg_graph_rates_data **pprd);

/*  Creats and saves graph to file.  */
int bg_graph_save_graph(const struct bg_data_node *pd, struct bg_graph_rates_data *prd, size_t numb, const char *path, _Bool save_png, struct bb_error *perr);

/*  Creats and saves rates graph to file.  */
void bg_graph_save_rates_graph(struct bg_graph_rates_data *prd, const char *path, _Bool save_png, struct bb_error *perr);

/*  Returns error message string by error code.  */
const char * bg_graph_error_message(int err_code);

#endif                                      // BG_GRAPH_
