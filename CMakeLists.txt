cmake_minimum_required(VERSION 3.10)
project(SnakeGame VERSION 2.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ========== 源文件列表（工程化目录结构） ==========
set(SOURCES
    main.cpp
    core/GameLoop.cpp
    core/StateMachine.cpp
    systems/InputSystem.cpp
    systems/RenderSystem.cpp
    systems/TimeSystem.cpp
    gameplay/Snake.cpp
    gameplay/Food.cpp
    persistence/SaveSystem.cpp
)

set(HEADERS
    core/GameLoop.h
    core/StateMachine.h
    systems/InputSystem.h
    systems/RenderSystem.h
    systems/TimeSystem.h
    gameplay/Snake.h
    gameplay/Food.h
    persistence/SaveSystem.h
    common/Commom.h
)

add_executable(snake ${SOURCES} ${HEADERS})

# 平台特定设置
if(WIN32)
    set_target_properties(snake PROPERTIES
        WIN32_EXECUTABLE OFF
    )
endif()

# GCC 编译选项
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(snake PRIVATE -Wall -Wextra)
elseif(MSVC)
    target_compile_options(snake PRIVATE /W4)
endif()
