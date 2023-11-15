add_executable(fuzz_decode decode.cpp)
target_link_libraries(fuzz_decode rrc_nr_asn1)

set_target_properties(fuzz_decode
        PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        OUTPUT_NAME decode
)