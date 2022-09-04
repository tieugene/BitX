#ifndef BB_CNF_
#define BB_CNF_

#include "bb_error.h"

struct bb_cnf_data {                        // cnf-file data.
    char *m_path;                           // M-file.
    char *d_path;                           // D-file.
    char *p_path;                           // P-file.
    char *result_dir;                       // Result dir.
    char *csv_path;                         // CSV-file.
};

/* Gets settings from cnf-file.  */
int bb_cnf_get_settings(const char *file_name, struct bb_cnf_data *pcd, struct bb_error *perr);

/* Frees cnf-file data memory.  */
void bb_free_cnf_settings(struct bb_cnf_data *pcd);

/*  Returns error string by error code.  */
const char * bb_cnf_error_message(int err_code);

#endif                                      // BB_CNF_