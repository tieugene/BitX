#ifndef BG_GL_
#define BG_GL_

#define BB_GRAPH_BORDER 5
#define BB_GRAPH_PAD_X 2
#define BB_GRAPH_PAD_Y 2

#define BB_GL_W 9
#define BB_GL_H 12
#define BB_GL_WH (BB_GL_W * BB_GL_H)
#define BB_GL_N 15

#define BB_GP_W 6
#define BB_GP_H 6
#define BB_GP_WH (BB_GP_W * BB_GP_H)

#define BG_GT_W 28
#define BG_GT_H 16
#define BG_GT_WH (BG_GT_W * BG_GT_H)

#define BG_GT_BUY 0
#define BG_GT_SEL 1
#define BG_GT_WIN 2
#define BG_GT_LOS 3
#define BG_GT_BTC 4
#define BG_GT_N 5

#define BB_GT_RW 24
#define BB_GT_RH 16

#define BB_GT_P1 4
#define BB_GT_P2 8

#define BG_YT0_W 9
#define BG_YT0_H 14
#define BG_YT1_W 7
#define BG_YT1_H 14
#define BG_YT2_W 24
#define BG_YT2_H 14
#define BG_YT3_W 99
#define BG_YT3_H 16
#define BG_YT4_W 99
#define BG_YT4_H 16

#define BG_G_R_W 85
#define BG_G_R_H 16

#define BG_G_R_LEGEND_W (BB_GT_P2 + BB_GT_RW + BB_GT_P1 + BG_G_R_W)

#define BG_G_LEGEND_W ((BB_GT_RW + BB_GT_P1 + BG_GT_W) * 2 + BB_GT_P2 + BG_G_R_LEGEND_W)
//#define BG_G_LEGEND_W ((BB_GT_RW + BB_GT_P1 + BG_GT_W) * 3 + BB_GT_P2 * 2)
//#define BG_G_LEGEND_W ((BB_GT_RW + BB_GT_P1 + BG_GT_W + BB_GT_P2) * 3 + BG_GTTL_MAX_W
#define BG_G_LEGEND_H 16

#define BB_GRAPH_PNG_LW 2

#define BB_GH 400

//#define BG_GL_W 8
//#define BG_GL_H 16
#define BG_GL_W 9
#define BG_GL_H 18
#define BG_GL_WH (BG_GL_W * BG_GL_H)
#define BG_GL_N 160

/*  Colors.  */
#define BG_G_BW_R 0x00
#define BG_G_BW_G 0xff
#define BG_G_BW_B 0x00

#define BG_G_SL_R 0xff
#define BG_G_SL_G 0x00
#define BG_G_SL_B 0x00

#define BG_G_BC_R 0x00
#define BG_G_BC_G 0x56
#define BG_G_BC_B 0x9d

#endif                                      // BG_GL_