cmake_minimum_required(VERSION 3.24)
project(re_overlay)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_GENERATOR_PLATFORM x64)
add_library(re_overlay SHARED)
target_sources(re_overlay PRIVATE
        src/Speedrun.cpp
        src/Speedrun.h
        src/utilities/Mod.h
        src/enums.h
        src/utilities/Config.h
        src/utilities/Config.cpp
        src/utilities/String.cpp
        src/utilities/String.h
        src/main.cpp
        src/utilities/Mod.h
        src/imgui/font_robotomedium.hpp
        src/imgui/imgui_impl_dx11.cpp
        src/imgui/imgui_impl_dx12.cpp
        src/imgui/imgui_impl_dx11.h
        src/imgui/imgui_impl_dx12.h
        src/imgui/imgui_impl_win32.cpp
        src/imgui/imgui_impl_win32.h
        src/imgui/imgui.h
        src/imgui/imgui.cpp
        src/imgui/imgui_internal.h
        src/imgui/imgui_widgets.cpp
        src/imgui/imgui_tables.cpp
        src/imgui/imgui_draw.cpp
        src/rendering/d3d11.cpp
        src/rendering/d3d11.hpp
        src/rendering/d3d12.cpp
        src/rendering/d3d12.hpp
        src/rendering/shared.hpp
        )
add_library(reframework INTERFACE)

target_include_directories(reframework INTERFACE "REFramework/include")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /MP")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /MT /O2")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MT /O2")
string(REGEX REPLACE "/MD" "/MT" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
string(REGEX REPLACE "/MD" "/MT" CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
string(REGEX REPLACE "/MD" "/MT" CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")
string(REGEX REPLACE "/MD" "/MT" CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE}")

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded")
target_link_libraries(re_overlay PUBLIC reframework d3d12 d3d11)

install(
        TARGETS
        re_overlay
        DESTINATION
        bin
        COMPONENT
        re_overlay
)

set(BENCHMARK_ENABLE_TESTING NO)
include(FetchContent)
FetchContent_Declare(
        googlebenchmark
        GIT_REPOSITORY https://github.com/google/benchmark.git
        GIT_TAG origin/main
)

FetchContent_MakeAvailable(googlebenchmark)
add_executable (overlay_benchmarks bench/benches.cpp bench/benches.h)
target_link_libraries(overlay_benchmarks benchmark::benchmark)

if ("${CMAKE_SYSTEM_NAME}" MATCHES "Windows")
    target_link_libraries(overlay_benchmarks Shlwapi)
endif()