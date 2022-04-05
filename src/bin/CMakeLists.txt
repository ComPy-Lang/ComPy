add_executable(compy compy.cpp)
target_include_directories(compy PRIVATE "tpl")
target_link_libraries(compy compy_lib)
if (COMPY_STATIC_BIN)
    if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
        # Link statically on Linux with gcc or clang
        if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR
                CMAKE_CXX_COMPILER_ID MATCHES Clang)
            target_link_options(compy PRIVATE -static)
        endif()
    endif()
endif()

if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_link_options(compy PRIVATE "LINKER:--export-dynamic")
endif()

if (WITH_STACKTRACE AND APPLE AND CMAKE_CXX_COMPILER_ID MATCHES Clang)
    # On macOS we have to call dsymutil to create the dSYM bundle so that the
    # stacktrace can find debugging information corresponding to the compy
    # binary
    add_custom_command(
        TARGET compy
        POST_BUILD
        COMMAND dsymutil compy
    )
    if (WITH_DWARFDUMP)
        add_custom_command(
            TARGET compy
            POST_BUILD
            COMMAND llvm-dwarfdump --debug-line compy.dSYM > compy.dSYM/raw.txt
        )
        add_custom_command(
            TARGET compy
            POST_BUILD
            COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/dwarf_convert.py compy.dSYM/raw.txt compy.dSYM/lines.txt compy.dSYM/lines.dat
        )
    endif()
endif()


# Ensure "Release" is not appended to the path on Windows:
# https://stackoverflow.com/a/56201564/479532
set_target_properties(compy PROPERTIES RUNTIME_OUTPUT_DIRECTORY $<0:>)

set_target_properties(compy PROPERTIES
    INSTALL_RPATH_USE_LINK_PATH TRUE
)

install(TARGETS compy
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    ARCHIVE DESTINATION share/compy/lib
    LIBRARY DESTINATION share/compy/lib
)

set_target_properties(compy PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/$<0:>
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/$<0:>
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/$<0:>)
