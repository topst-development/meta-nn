cmake_minimum_required(VERSION 2.8)
project(tcchiptestapp)

# Set compiler and flags
set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -D__NO_INLINE__ -std=gnu99")
set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pg ")
set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-unused -Wno-attributes -fno-delete-null-pointer-checks ")
set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC -no-pie ")
set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ldl")
set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -static")

# Add include directories
include_directories(${LINUX_KERNEL_DIR}/drivers/misc/telechips/npu)
include_directories(enlight)
include_directories(npu_api)

# Add source files
add_executable(tcchiptestapp
    enlight/enlight_network.c
    npu_api/npu_api_ex.c
    npu_api/npu_api_mem.c
    npu_api/npu_api.c
    test_npu_reg.c
    test_npu_mlx.c
    test_npu_ecc.c
    test_npu_unit.c
    test_npu_network.c
    test_npu_reset.c
    test_npu_wdt.c
    chip_test.c
    test_main.c
)

# Link libraries
target_link_libraries(tcchiptestapp dl)

# Install
install(TARGETS tcchiptestapp DESTINATION bin)
