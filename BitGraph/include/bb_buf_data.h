#ifndef BD_BUF_DATA_
#define BD_BUF_DATA_

struct bb_buf_data;

/*  Allocates memory and initializes data structure.  */
int bb_buf_init(struct bb_buf_data **ppbd, size_t size);

/*  Resets buffer data structure.  */
int bb_buf_reset(struct bb_buf_data *pbd);

/*  Puts the string into buffer.  */
int bb_buf_put(struct bb_buf_data *pbd, const char *s, size_t s_len);

/*  Gets buffer and its length.  */
int bb_buf_get(struct bb_buf_data *pbd, char **pbuf, size_t *pbuf_len);

/*  Frees memory.  */
void bb_buf_deinit(struct bb_buf_data **ppbd);

#endif                                      // BD_BUF_DATA_