add_executable(fuzzing_asn1_ul_dcch_fuzz_target target.cc)
target_include_directories(fuzzing_asn1_ul_dcch_fuzz_target PRIVATE ${CMAKE_SOURCE_DIR})
target_compile_options(fuzzing_asn1_ul_dcch_fuzz_target PRIVATE -fsanitize=fuzzer,address)
target_link_options(fuzzing_asn1_ul_dcch_fuzz_target PRIVATE -fsanitize=fuzzer,address)
target_link_libraries(fuzzing_asn1_ul_dcch_fuzz_target srsran_rrc)
set_target_properties(fuzzing_asn1_ul_dcch_fuzz_target
        PROPERTIES
        OUTPUT_NAME fuzz_target
)

add_executable(fuzzing_asn1_ul_dcch_run_target StandaloneFuzzTargetMain.c target.cc)
target_link_libraries(fuzzing_asn1_ul_dcch_run_target srsran_rrc)
set_target_properties(fuzzing_asn1_ul_dcch_run_target
        PROPERTIES
        OUTPUT_NAME run_target
)

add_executable(fuzzing_asn1_ul_dcch_generate_input generate_input.cc)
set_target_properties(fuzzing_asn1_ul_dcch_generate_input
        PROPERTIES
        OUTPUT_NAME generate_input
)

add_executable(fuzzing_asn1_ul_dcch_decode decode.cc)
target_link_libraries(fuzzing_asn1_ul_dcch_decode srsran_rrc)
set_target_properties(fuzzing_asn1_ul_dcch_decode
        PROPERTIES
        OUTPUT_NAME decode
)