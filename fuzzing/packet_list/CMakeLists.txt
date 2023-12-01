add_library(packet_list STATIC packet_list.cc)
target_include_directories(packet_list PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(packet_list_test packet_list_test.cc)
target_link_libraries(packet_list_test PRIVATE packet_list)
