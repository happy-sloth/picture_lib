cmake_minimum_required(VERSION 2.8) 

project(picture_lib)

set(SOURCE jpg_codec.c huffman.c)#
set(HEADER jpg_common.h jpg_codec.h huffman.h)#

add_library(picture_lib STATIC ${HEADER} ${SOURCE})