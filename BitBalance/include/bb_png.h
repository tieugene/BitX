#ifndef BB_PNG_
#define BB_PNG_

#include "bb_error.h"

#define BB_LETTER_W 9
#define BB_LETTER_H 15

struct bb_png_data;

/*  Allocates memory and fill it with data.  */
int bb_png_init(struct bb_png_data **pppd, int w, int h, struct bb_error *perr);

/*  Draws rectangle.  */
int bb_png_draw_rect(struct bb_png_data *ppd, int x, int y, int w, int h, int color_n, struct bb_error *perr);

/*  Writes text.  */
int bb_png_write_text(struct bb_png_data *ppd, int x, int y, const char *text, size_t text_len, _Bool rotate, struct bb_error *perr);

/*  Save data to png-file.  */
int bb_png_save(struct bb_png_data *ppd, const char *file_name, struct bb_error *perr);

/*  Frees memory.  */
void bb_png_free(struct bb_png_data **pppd);

/*  Returns error string by error code.  */
const char * bb_png_error_message(int err_code);

#endif                                      // BB_PNG_