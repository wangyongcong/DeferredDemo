cmake_minimum_required(VERSION 3.16)

project(D3D12Deferred LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_definitions(-DUNICODE -D_UNICODE)

set(SUPRESS_WARNINGS
    # use STL in dll
    /wd4251 
)

add_compile_options("/utf-8" ${SUPRESS_WARNINGS})

# 将 CMake 相关的 project 放到 solution 目录里面
set_property(GLOBAL 
    PROPERTY USE_FOLDERS ON 
)

# 设置输出目录
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/Libraries/$<CONFIGURATION>) # LIB 输出目录
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/Binaries/$<CONFIGURATION>) # DLL 输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/Binaries/$<CONFIGURATION>) # EXE 输出目录


add_subdirectory(Source/D3DGame)
add_subdirectory(Source/Deferred)

# 设置 startup project
set_directory_properties(PROPERTIES
    VS_STARTUP_PROJECT Deferred
)