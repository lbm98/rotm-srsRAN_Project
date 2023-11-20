add_executable(aflpp_target harness.cpp)
target_include_directories(aflpp_target PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(aflpp_target rrc_ue_test_helpers srsran_rrc gtest)
