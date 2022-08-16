#ifndef BG_CNF_
#define BG_CNF_

#include "bb_error.h"

struct bg_cnf_data {                        // cnf-file data.
    char *s_dir;                            // Source dir.
    char *p_path;                           // P-file.
    char *result_dir;                       // Result dir.
    int threads_n;                          // Number of threads.
    _Bool save_png;                         // 1 - save png, 0 - don't.
};

/* Gets settings from cnf-file.  */
int bg_cnf_get_settings(const char *file_name, struct bg_cnf_data *pcd, struct bb_error *perr);

/* Frees cnf-file data memory.  */
void bg_free_cnf_settings(struct bg_cnf_data *pcd);

/*  Returns error string by error code.  */
const char * bg_cnf_error_message(int err_code);

#endif                                      // BG_CNF_