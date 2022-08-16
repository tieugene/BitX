#ifndef BB_M_TREE_
#define BB_M_TREE_

#include "bb_tree.h"
#include "bb_rates.h"
#include "bg_data.h"
#include "bb_error.h"

struct bb_m_data;

/*  Allocates memory and creates M-file data structure.  */
int bb_get_m_data(const char *file_name, double last_day_rate, struct bb_m_data **ppmd, struct bb_tree_data *ptd, struct bb_rates_data *prd, 
    size_t *pn, struct bb_error *perr);

/*  Save strings to file.  */
int bb_write_m_data(const char *file_name, const struct bb_m_data *pmd, enum bb_write_modes nbs, enum bb_write_modes wl, struct bb_error *perr);

/*  Save balances and data to file.  */
int bb_write_balances(const char *file_name, const struct bb_m_data *pmd, struct bb_error *perr);

/*  Gets graph data.  */
int bb_get_graph_data(struct bb_m_data *pmd, struct bg_data_node *pd, double last_day_rate, struct bb_error *perr);

/*  Gets chart data.  */
int bb_get_chart_data(struct bb_m_data *pmd, struct bb_chart_data *pcd, struct bb_error *perr);

/*  Returns tree nodes number.  */
size_t bb_get_nodes_n(struct bb_m_data *pmd);

/*  Frees memory.  */
int bb_free_m_data(struct bb_m_data **ppmd, struct bb_error *perr);

/*  Returns error string by error code.  */
const char * bb_m_data_error_message(int err_code);

#endif                                      // BB_M_TREE_