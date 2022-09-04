#ifndef BG_SVG_
#define BG_SVG_

#include "bb_error.h"

struct bg_svg;

/*  Allocates memory and initializes data structure.  */
int bg_svg_init(struct bg_svg **ppbs, unsigned int w, unsigned int h, struct bb_error *perr);

/*  Draws rectangle.  */
int bg_svg_draw_rect(struct bg_svg *pbs, int x, int y, int w, int h, const unsigned char color[3], struct bb_error *perr);

/*  Draws line.  */
int bg_svg_draw_line(struct bg_svg *pbs, int x1, int y1, int x2, int y2, const unsigned char color[3], struct bb_error *perr);

/*  Draws multiline with points.  */
int bg_svg_draw_mpline(struct bg_svg *pbs, const int *coords, size_t points_n, _Bool points, const unsigned char color[3], struct bb_error *perr);

/*  Draws text.  */
int bg_svg_draw_text(struct bg_svg *pbs, int x, int y, const char *text, size_t text_len, const unsigned char color[3], struct bb_error *perr);

/*  Writes svg to file.  */
int bg_svg_write(const struct bg_svg *pbs, const char *path, struct bb_error *perr);

/*  Frees memory.  */
void bg_svg_deinit(struct bg_svg **ppbs);

/*  Returns error message string by error code.  */
const char * bg_svg_error_message(int err_code);

#endif                                      // BG_SVG_
