#ifndef BB_GRAPH_PNG_
#define BB_GRAPH_PNG_

#include <stddef.h>
#include "bb_error.h"

struct bb_graph_png;

/*  Allocates memory and fill it with data.  */
int bb_graph_png_init(struct bb_graph_png **ppgp, int w, int h, struct bb_error *perr);

/*  Draws rectangle.  */
int bb_graph_png_draw_rect(struct bb_graph_png *pgp, int x, int y, int w, int h, const unsigned char color[3], struct bb_error *perr);

/*  Draws line.  */
int bb_graph_png_draw_line(struct bb_graph_png *pgp, int x1, int y1, int x2, int y2, const unsigned char color[3], struct bb_error *perr);

/*  Draws point.  */
void bb_graph_png_draw_point(struct bb_graph_png *pgp, int x, int y, const unsigned char color[3], struct bb_error *perr);

/*  Draws legend.  */
int bb_graph_png_draw_legend(struct bb_graph_png *pgp, int x, int y, struct bb_error *perr);

/*  Writes text.  */
int bb_graph_png_write_text(struct bb_graph_png *pgp, int x, int y, const char *text, size_t text_len, const unsigned char color[3], 
    _Bool rotate, struct bb_error *perr);

/*  Save data to png-file.  */
int bb_graph_png_save(struct bb_graph_png *pgp, const char *file_name, struct bb_error *perr);

/*  Frees memory.  */
void bb_graph_png_free(struct bb_graph_png **ppgp);

/*  Returns error string by error code.  */
const char * bb_graph_png_error_message(int err_code);

#endif                                      // BB_GRAPH_PNG_