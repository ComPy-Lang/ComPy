set(SRC
    ../impure/lfortran_intrinsics.c
)
add_library(compy_runtime SHARED ${SRC})
set_target_properties(compy_runtime PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/../$<0:>
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/../$<0:>
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/../$<0:>)
add_library(compy_runtime_static STATIC ${SRC})
set_target_properties(compy_runtime_static PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/../$<0:>
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/../$<0:>
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/../$<0:>)
install(TARGETS compy_runtime compy_runtime_static
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    ARCHIVE DESTINATION share/compy/lib
    LIBRARY DESTINATION share/compy/lib
)
