cmake_minimum_required(VERSION 3.10)
project(GreedySnake VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 源文件列表
set(SOURCES
    GreedySnake.cpp
    Game.cpp
    Snake.cpp
    Food.cpp
    Renderer.cpp
    Input.cpp
    SaveSystem.cpp
)

set(HEADERS
    Game.h
    Snake.h
    Food.h
    Renderer.h
    Input.h
    SaveSystem.h
    common/Commom.h
)

add_executable(snake ${SOURCES} ${HEADERS})

# 平台特定设置
if(WIN32)
    # Windows 控制台应用
    set_target_properties(snake PROPERTIES
        WIN32_EXECUTABLE OFF
    )
endif()

# 可选：GCC 特定编译选项
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(snake PRIVATE -Wall -Wextra)
elseif(MSVC)
    target_compile_options(snake PRIVATE /W4)
endif()
