cmake_minimum_required(VERSION 3.0) 

set(CMAKE_C_STANDARD 11)
add_compile_options(-lm)

project(test_jpg_codec)

include_directories(src)
add_subdirectory(src)	
			
add_executable(main main.c)	

target_link_libraries(main m)
target_link_libraries(main picture_lib)	
