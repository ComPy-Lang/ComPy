set(SRC
    parser/tokenizer.cpp
    parser/parser.cpp
    parser/parser.tab.cc
    semantics/ast_to_asr.cpp

    # python_evaluator.cpp

    # pickle.cpp
    # python_serialization.cpp

    utils.cpp

    ../bin/tpl/whereami/whereami.cpp
)

add_library(compy_lib ${SRC})
target_link_libraries(compy_lib asr compy_runtime_static ZLIB::ZLIB)
target_include_directories(compy_lib BEFORE PUBLIC ${compy_SOURCE_DIR}/src)
target_include_directories(compy_lib BEFORE PUBLIC ${compy_BINARY_DIR}/src)
if (WITH_BFD)
    target_link_libraries(compy_lib p::bfd)
endif()
if (WITH_LINK)
    target_link_libraries(compy_lib p::link)
endif()
if (WITH_EXECINFO)
    target_link_libraries(compy_lib p::execinfo)
endif()
if (WITH_LLVM)
    target_link_libraries(compy_lib p::llvm)
endif()
#install(TARGETS compy_lib
#    RUNTIME DESTINATION bin
#    ARCHIVE DESTINATION lib
#    LIBRARY DESTINATION lib
#)

# add_subdirectory(tests)
