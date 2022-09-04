#ifndef BB_GL_
#define BB_GL_

#define BB_GL_W 9
#define BB_GL_H 12
#define BB_GL_WH (BB_GL_W * BB_GL_H)
#define BB_GL_N 13

#define BB_GP_W 6
#define BB_GP_H 6
#define BB_GP_WH (BB_GP_W * BB_GP_H)

#define BB_GT_W 64
#define BB_GT_H 16
#define BB_GT_WH (BB_GT_W * BB_GT_H)

#define BB_GT_WB 0
#define BB_GT_WS 1
#define BB_GT_LB 2
#define BB_GT_LS 3
#define BB_GT_N 4

#define BB_GT_RW 24
#define BB_GT_RH 16

#define BB_GT_P1 4
#define BB_GT_P2 8

#define BB_G_LEGEND_W ((BB_GT_RW + BB_GT_P1 + BB_GT_W) * BB_GT_N + BB_GT_P2 * (BB_GT_N - 1))
#define BB_G_LEGEND_H 16

#define BB_GRAPH_PNG_LW 2

#define BB_GH 400

/*  Colors.  */
#define BB_G_WB_R 0x00
#define BB_G_WB_G 0xff
#define BB_G_WB_B 0x00

#define BB_G_WS_R 0xff
#define BB_G_WS_G 0x00
#define BB_G_WS_B 0x00

#define BB_G_LB_R 0x00
#define BB_G_LB_G 0x56
#define BB_G_LB_B 0x9d

#define BB_G_LS_R 0x6f
#define BB_G_LS_G 0x14
#define BB_G_LS_B 0xfb

#endif                                      // BB_GL_
