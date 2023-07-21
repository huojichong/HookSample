
cmake_minimum_required(VERSION 3.18.1)

project("hooksample")

find_package(bytehook REQUIRED CONFIG)

add_library( # Sets the name of the library.
        hooksample
        SHARED
        native-lib.cpp)

target_compile_features(hooksample PUBLIC c_std_11)
target_compile_options(hooksample PUBLIC -Werror) # -Weverything
find_library( # Sets the name of the path variable.
        log-lib
        log)

target_link_libraries( # Specifies the target library.
        hooksample
        bytehook::bytehook
        ${log-lib})


# libhacker.so
add_library(hacker SHARED hacker.c)
target_compile_features(hacker PUBLIC c_std_11)
target_compile_options(hacker PUBLIC -Werror)
target_link_libraries(hacker bytehook::bytehook log)
