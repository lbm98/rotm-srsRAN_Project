add_executable(fuzz_decode decode.cpp)
target_link_libraries(fuzz_decode rrc_nr_asn1)
set_target_properties(fuzz_decode
        PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        OUTPUT_NAME decode
)

add_executable(fuzz_generate_input_corpus generate_input_corpus.cpp)
set_target_properties(fuzz_generate_input_corpus
        PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        OUTPUT_NAME generate_input_corpus
)

add_executable(fuzz_setup_request_non_persistent fuzz_setup_request_non_persistent.cpp)
target_include_directories(fuzz_setup_request_non_persistent PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(fuzz_setup_request_non_persistent rrc_ue_test_helpers srsran_rrc gtest)

add_executable(fuzz_setup_request_persistent fuzz_setup_request_persistent.cpp)
target_include_directories(fuzz_setup_request_persistent PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(fuzz_setup_request_persistent rrc_ue_test_helpers srsran_rrc gtest)