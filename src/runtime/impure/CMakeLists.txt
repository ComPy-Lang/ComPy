cmake_minimum_required(VERSION 3.5 FATAL_ERROR)

project(compy_runtime_impure C Fortran)

add_library(compy_runtime_impure
    STATIC
    compy_intrinsics.c
    compy_intrinsic_sin_c.c
)
