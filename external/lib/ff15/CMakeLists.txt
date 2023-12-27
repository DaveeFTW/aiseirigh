cmake_minimum_required(VERSION 3.9.4)

include(CheckIPOSupported)
check_ipo_supported(RESULT supported OUTPUT error)

project(fatfs C)

add_library(fatfs STATIC source/pspio.c source/ff.c source/ffsystem.c source/ffunicode.c source/ff.h)
target_link_libraries(fatfs led lflash emcsm mspro uart model sysreg gcc)

target_include_directories(fatfs
    PUBLIC
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/source>
)

set_property(TARGET fatfs PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)

install(TARGETS fatfs
        EXPORT fatfsTargets
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        RUNTIME DESTINATION bin
        INCLUDES DESTINATION include
)

install(FILES source/ff.h DESTINATION include)
