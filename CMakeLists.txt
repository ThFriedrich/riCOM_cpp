cmake_minimum_required(VERSION 3.17.0)

project(RICOM LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 17)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

MESSAGE(STATUS "Using ${CMAKE_CXX_COMPILER_ID} Compiler!")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

if (UNIX)
    set(SDL2_INCLUDE_DIR /usr/include/SDL2)
    set(CMAKE_CXX_FLAGS "-Wall -Wextra -pthread")
    set(CMAKE_CXX_FLAGS_DEBUG "-g")
    set(CMAKE_CXX_FLAGS_RELEASE "-Ofast")
endif(UNIX)

if (WIN32)
    set(SDL2_INCLUDE_DIR C:/SDL2_VC/include/)
    set(SDL2_LIB_DIR C:/SDL2_VC/lib/x64)
    file(COPY ${SDL2_LIB_DIR}/SDL2.dll DESTINATION ${PROJECT_BINARY_DIR})
    link_directories(${SDL2_LIB_DIR})

    if (${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU")
        set(CMAKE_CXX_FLAGS "-Wall -Wextra -pthread")
        set(CMAKE_CXX_FLAGS_DEBUG "-g")
        set(CMAKE_CXX_FLAGS_RELEASE "-Ofast")
    endif()

    if (${CMAKE_CXX_COMPILER_ID} STREQUAL "MSVC")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++17")
    endif()

    if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "IntelLLVM")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++17")
        set(MMD_LIB C:/Program\ Files\ \(x86\)/Intel/oneAPI/compiler/latest/windows/redist/intel64_win/compiler)
        file(COPY ${MMD_LIB}/libmmd.dll DESTINATION ${PROJECT_BINARY_DIR})
        file(COPY ${MMD_LIB}/svml_dispmd.dll DESTINATION ${PROJECT_BINARY_DIR})
    endif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "IntelLLVM")

endif (WIN32)

add_executable(RICOM src/main.cpp)

target_sources( RICOM PRIVATE
    imgui/imgui_demo.cpp
    imgui/imgui_draw.cpp
    imgui/imgui_tables.cpp
    imgui/imgui_widgets.cpp
    imgui/imgui.cpp
    imgui/backends/imgui_impl_opengl3.cpp
    imgui/backends/imgui_impl_sdl.cpp
    src/progress_bar.cpp
    src/TimpixInterface.cpp
    src/Ricom.cpp 
    src/main.cpp
    )

target_include_directories( RICOM PUBLIC
    imgui
    imgui/backends
    ${SDL2_INCLUDE_DIR}
    include
    )

find_package(OpenGL REQUIRED)
target_link_libraries(RICOM PUBLIC ${OPENGL_LIBRARIES})
target_link_libraries(RICOM PUBLIC SDL2main SDL2 ${CMAKE_DL_LIBS})
