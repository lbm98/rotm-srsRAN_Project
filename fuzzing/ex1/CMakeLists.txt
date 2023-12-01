
add_executable(fuzzing_target target.cc pcap_mutator.cc)
target_include_directories(fuzzing_target PRIVATE ${CMAKE_SOURCE_DIR})
target_link_options(fuzzing_target PRIVATE -fsanitize=fuzzer)
target_link_libraries(fuzzing_target PRIVATE rrc_ue_test_helpers srsran_rrc gtest packet_list)
set_target_properties(fuzzing_target PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(generate_input generate_input.cc)
target_link_libraries(generate_input PRIVATE packet_list)
set_target_properties(generate_input PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(StandaloneFuzzTargetMain StandaloneFuzzTargetMain.c target.cc)
target_include_directories(StandaloneFuzzTargetMain PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(StandaloneFuzzTargetMain PRIVATE rrc_ue_test_helpers srsran_rrc gtest packet_list)
set_target_properties(StandaloneFuzzTargetMain PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(fuzzing_decode decode.cpp)
target_link_libraries(fuzzing_decode rrc_nr_asn1 packet_list)
set_target_properties(fuzzing_decode
        PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        OUTPUT_NAME decode
)
