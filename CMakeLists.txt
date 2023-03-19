cmake_minimum_required(VERSION 3.24)
project(diffscope_pseudo_editor C)

set(CMAKE_C_STANDARD 11)

add_library(diffscope_pseudo_editor SHARED main.c)
