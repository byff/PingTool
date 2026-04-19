cmake_minimum_required(VERSION 3.16)
project(PingTool VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(ENABLE_LTO "Enable Link Time Optimization" OFF)
option(ENABLE_STRIP "Strip symbols from binary" OFF)

if(ENABLE_LTO)
    include(CheckIPOSupported)
    check_ipo_supported(RESULT LTO_SUPPORTED OUTPUT LTO_ERROR)
    if(LTO_SUPPORTED)
        set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
    endif()
endif()

set(COMMON_LIBS
    iphlpapi.lib
    ws2_32.lib
    winmm.lib
    comdlg32.lib
    shlwapi.lib
    comctl32.lib
)

add_executable(${PROJECT_NAME}
    WIN32
    src/main.cpp
    src/GUI.cpp
    src/HostStat.cpp
    src/IcmpBackend.cpp
    src/Scheduler.cpp
    src/CIDR.cpp
    src/Config.cpp
    src/ExcelExport.cpp
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    ${COMMON_LIBS}
)

target_include_directories(${PROJECT_NAME} PRIVATE
    include
    thirdparty/nlohmann_json
)

if(WIN32)
    add_compile_definitions(_WIN32_WINNT=0x0A00 WIN32_LEAN_AND_MEAN NOMINMAX)
    add_compile_definitions(UNICODE _UNICODE)
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS OFF)
endif()

if(MSVC)
    add_compile_options(/utf-8 /W3 /WX-)
    target_compile_options(${PROJECT_NAME} PRIVATE /permissive-)
endif()

if(ENABLE_STRIP)
    target_link_options(${PROJECT_NAME} PRIVATE /OPT:REF /OPT:ICF /LTCG)
endif()

set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    OUTPUT_NAME "PingTool"
)

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E echo "Build completed: $<TARGET_FILE:${PROJECT_NAME}>"
    )
endif()