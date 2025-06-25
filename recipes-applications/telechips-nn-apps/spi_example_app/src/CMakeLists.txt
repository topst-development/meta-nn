cmake_minimum_required(VERSION 2.8)
project(spi_example_app)


# Set compiler and flags
set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -D__NO_INLINE__ -std=gnu99")
set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pg ")
set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-unused -Wno-attributes -fno-delete-null-pointer-checks ")
set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC -no-pie ")
set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ldl")
set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -static")

add_executable(spi_example_app spi_example_app.c)


install(TARGETS spi_example_app DESTINATION bin)
