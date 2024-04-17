cmake_minimum_required(VERSION 3.0) 
set(CMAKE_C_STANDARD 11)
add_compile_options(-lm)

project(picture_lib)

set(SOURCE jpg_codec.c huffman.c bmp_codec.c)#
set(HEADER jpg_common.h jpg_codec.h huffman.h bmp_codec.h DCT_coefficients.h)#

add_library(picture_lib STATIC ${HEADER} ${SOURCE})

target_link_libraries(picture_lib m)