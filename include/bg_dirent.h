#ifndef BG_DIRENT_
#define BG_DIRENT_

#include <stddef.h>
#include "bb_error.h"

#ifdef BB_WIN
#define BG_SLASH '\\'
#else
#define BG_SLASH '/'
#endif

struct bg_dirent_data;

/*  Allocates memory and creats data structure.  */
int bg_dirent_init(const char *path, struct bg_dirent_data **ppdd, struct bb_error *perr);

/*  Gets another directory path.  */
int bg_dirent_iter(struct bg_dirent_data *pdd, const char **ppath, size_t *ppath_len, struct bb_error *perr);

/*  Frees memory.  */
int bg_dirent_deinit(struct bg_dirent_data **ppdd, struct bb_error *perr);

/*  Returns error message string by error code.  */
const char * bg_dirent_error_message(int err_code);

#endif                                      // BG_DIRENT_