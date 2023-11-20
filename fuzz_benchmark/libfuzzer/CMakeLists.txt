add_executable(libfuzzer_target target.cpp)
target_include_directories(libfuzzer_target PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(libfuzzer_target rrc_ue_test_helpers srsran_rrc gtest)

#target_compile_options(
#        libfuzzer_target
#        PRIVATE
#        -fsanitize=fuzzer
#)
#
#set_target_properties(libfuzzer_target
#        PROPERTIES
#        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
#        OUTPUT_NAME libfuzzer_target
#)