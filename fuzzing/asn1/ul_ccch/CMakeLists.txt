add_executable(fuzzing_asn1_ul_ccch_fuzz_target target.cc)
target_include_directories(fuzzing_asn1_ul_ccch_fuzz_target PRIVATE ${CMAKE_SOURCE_DIR})
target_compile_options(fuzzing_asn1_ul_ccch_fuzz_target PRIVATE -fsanitize=fuzzer,address)
target_link_options(fuzzing_asn1_ul_ccch_fuzz_target PRIVATE -fsanitize=fuzzer,address)
target_link_libraries(fuzzing_asn1_ul_ccch_fuzz_target srsran_rrc)
set_target_properties(fuzzing_asn1_ul_ccch_fuzz_target
        PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin"
        OUTPUT_NAME fuzz_target
)

add_executable(fuzzing_asn1_ul_ccch_run_target StandaloneFuzzTargetMain.c target.cc)
target_link_libraries(fuzzing_asn1_ul_ccch_run_target srsran_rrc)
set_target_properties(fuzzing_asn1_ul_ccch_run_target
        PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin"
        OUTPUT_NAME run_target
)

add_executable(fuzzing_asn1_ul_ccch_generate_input generate_input.cc)
set_target_properties(fuzzing_asn1_ul_ccch_generate_input
        PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin"
        OUTPUT_NAME generate_input
)

add_executable(fuzzing_asn1_ul_ccch_decode decode.cc)
target_link_libraries(fuzzing_asn1_ul_ccch_decode srsran_rrc)
set_target_properties(fuzzing_asn1_ul_ccch_decode
        PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin"
        OUTPUT_NAME decode
)