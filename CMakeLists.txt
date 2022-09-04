cmake_minimum_required(VERSION 3.5)
project(BitX LANGUAGES C)
# include_directories(include)
# BitBalance
set(BB_SRC
	BitBalance/BitBalance.c
	BitBalance/bb_chart.c
	BitBalance/bb_cleaning.c
	BitBalance/bb_cnf.c
	BitBalance/bb_csv_iter.c
	BitBalance/bb_graph.c
	BitBalance/bb_graph_png.c
	BitBalance/bb_io.c
	BitBalance/bb_m_tree.c
	BitBalance/bb_png.c
	BitBalance/bb_rates.c
	BitBalance/bb_scan_double.c
	BitBalance/bb_tree.c
	BitBalance/configini.c
	BitBalance/libattopng.c
)
add_executable(BitBalance ${BB_SRC})
target_include_directories(BitBalance PUBLIC BitBalance/include)
target_link_libraries(BitBalance m)
install(TARGETS BitBalance RUNTIME DESTINATION bin)
# BitDay
set(BD_SRC
	BitDay/BitDay.c
	BitDay/bb_io.c
	BitDay/bb_rates.c
	BitDay/bb_scan_double.c
	BitDay/bb_tree.c
	BitDay/bd_cleaning.c
	BitDay/bd_cnf.c
	BitDay/configini.c
)
add_executable(BitDay ${BD_SRC})
target_include_directories(BitDay PUBLIC BitDay/include)
target_link_libraries(BitDay m)
install(TARGETS BitDay RUNTIME DESTINATION bin)
# BitGraph
set(BG_SRC
	BitGraph/BitGraph.c
	BitGraph/bb_buf_data.c
	BitGraph/bb_io.c
	BitGraph/bb_rates.c
	BitGraph/bb_scan_double.c
	BitGraph/bb_tree.c
	BitGraph/bg_chart.c
	BitGraph/bg_cleaning.c
	BitGraph/bg_cnf.c
	BitGraph/bg_data_list.c
	BitGraph/bg_dirent.c
	BitGraph/bg_graph.c
	BitGraph/bg_m_tree.c
	BitGraph/bg_png.c
	BitGraph/bg_queue.c
	BitGraph/bg_svg.c
	BitGraph/bg_ts_error.c
	BitGraph/configini.c
	BitGraph/libattopng.c
)
add_executable(BitGraph ${BG_SRC})
target_include_directories(BitGraph PUBLIC BitGraph/include)
target_link_libraries(BitGraph m)
install(TARGETS BitGraph RUNTIME DESTINATION bin)
