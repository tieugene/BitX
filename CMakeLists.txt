cmake_minimum_required(VERSION 3.5)
project(BitX LANGUAGES C)
include_directories(include)
# BitDay
set(BD_SRC
	src/BitDay.c
	src/bb_io.c
	src/bb_rates.c
	src/bb_scan_double.c
	src/bb_tree.c
	src/bd_cleaning.c
	src/bd_cnf.c
	src/configini.c
)
add_executable(BitDay ${BD_SRC})
target_link_libraries(BitDay m)
install(TARGETS BitDay RUNTIME DESTINATION bin)
# BitGraph
set(BG_SRC
	src/BitGraph.c
	src/bb_buf_data.c
	src/bb_io.c
	src/bb_rates.c
	src/bb_scan_double.c
	src/bb_tree.c
	src/bg_chart.c
	src/bg_cleaning.c
	src/bg_cnf.c
	src/bg_data_list.c
	src/bg_dirent.c
	src/bg_graph.c
	src/bg_m_tree.c
	src/bg_png.c
	src/bg_queue.c
	src/bg_svg.c
	src/bg_ts_error.c
	src/configini.c
	src/libattopng.c
)
add_executable(BitGraph ${BG_SRC})
target_link_libraries(BitGraph m)
install(TARGETS BitGraph RUNTIME DESTINATION bin)
