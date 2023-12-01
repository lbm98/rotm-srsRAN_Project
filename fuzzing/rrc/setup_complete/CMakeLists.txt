add_executable(fuzzing_rrc_setup_complete_fuzz_target target.cc)
target_include_directories(fuzzing_rrc_setup_complete_fuzz_target PRIVATE ${CMAKE_SOURCE_DIR})
target_link_options(fuzzing_rrc_setup_complete_fuzz_target PRIVATE -fsanitize=fuzzer)
target_link_libraries(fuzzing_rrc_setup_complete_fuzz_target rrc_ue_test_helpers srsran_rrc gtest)
set_target_properties(fuzzing_rrc_setup_complete_fuzz_target
        PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin"
        OUTPUT_NAME fuzz_target
)

add_executable(fuzzing_rrc_setup_complete_run_target StandaloneFuzzTargetMain.c target.cc)
target_link_libraries(fuzzing_rrc_setup_complete_run_target rrc_ue_test_helpers srsran_rrc gtest)
set_target_properties(fuzzing_rrc_setup_complete_run_target
        PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin"
        OUTPUT_NAME run_target
)

add_executable(fuzzing_rrc_setup_complete_generate_input generate_input.cc)
set_target_properties(fuzzing_rrc_setup_complete_generate_input
        PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin"
        OUTPUT_NAME generate_input
)

add_executable(fuzzing_rrc_setup_complete_decode decode.cc)
target_link_libraries(fuzzing_rrc_setup_complete_decode srsran_rrc)
set_target_properties(fuzzing_rrc_setup_complete_decode
        PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin"
        OUTPUT_NAME decode
)