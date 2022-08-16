#ifndef BD_CNF_
#define BD_CNF_

#include "bb_error.h"

/*  Gets settings from file.  */
int bd_cnf_get_settings(const char *file_name, char **pm_path, struct bb_error *perr);

/*  Frees memory.  */
void bd_cnf_free_settings(char **pm_path);

/*  Returns error message string by error code.  */
const char * bd_cnf_error_message(int err_code);

#endif                                      // BD_CNF_
