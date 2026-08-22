cmake_minimum_required(VERSION 3.28)

project(Vortx VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

file(GLOB_RECURSE VORTX_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp"

    "${CMAKE_CURRENT_SOURCE_DIR}/src/core/io/binary.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/core/io/png.cpp"

    "${CMAKE_CURRENT_SOURCE_DIR}/src/Platform/window.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/Platform/input.cpp"

    "${CMAKE_CURRENT_SOURCE_DIR}/src/Renderer/renderer.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/Renderer/transfer.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/Renderer/vma.cpp"
    
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h"
)

add_executable(Vortx ${VORTX_SOURCES})

set(MEMORY_PLATFORM_SRC)

target_compile_definitions(Vortx
    PRIVATE
        $<$<CONFIG:Debug>:_DEBUG>
)

if(WIN32)
    target_compile_definitions(Vortx 
    PRIVATE 
        PLATFORM_WINDOWS
        WINDOW_WIN32
    )

elseif(UNIX AND NOT APPLE)
    target_compile_definitions(Vortx 
    PRIVATE 
        PLATFORM_LINUX
    )

    option(VORTX_USE_WAYLAND "Use Wayland Windows" OFF)

    if(VORTX_USE_WAYLAND)
        target_compile_definitions(Vortx
        PRIVATE
            WINDOW_WAYLAND
        )

        find_package(PkgConfig REQUIRED)
        pkg_check_modules(WAYLAND REQUIRED wayland-client)
        target_include_directories(Vortx PRIVATE ${WAYLAND_INCLUDE_DIRS})
        target_link_libraries(Vortx PRIVATE ${WAYLAND_LIBRARIES})

    else()
        target_compile_definitions(Vortx
        PRIVATE
            WINDOW_XCB
        )

        find_package(PkgConfig REQUIRED)
        pkg_check_modules(XCB REQUIRED xcb)
        pkg_check_modules(UDEV REQUIRED libudev)

        target_include_directories(Vortx
            PRIVATE 
                ${XCB_INCLUDE_DIRS}
                ${UDEV_INCLUDE_DIRS}
        )

        target_link_libraries(Vortx
            PRIVATE 
                ${XCB_LIBRARIES}
                ${UDEV_LIBRARIES}
        )
        
    endif()

endif()

target_include_directories(Vortx 
PRIVATE 
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
    "${CMAKE_CURRENT_SOURCE_DIR}/third_party/VulkanMemoryAllocator/include"
)

find_package(Vulkan REQUIRED)
target_link_libraries(Vortx PRIVATE Vulkan::Vulkan)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(Vortx 
    PRIVATE 
        -Wall 
        -Wextra 
        -Wpedantic
    )
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(Vortx 
    PRIVATE
        $<$<CONFIG:Debug>:-O0>
        $<$<CONFIG:Debug>:-g3>

        $<$<CONFIG:RelWithDebInfo>:-O2>
        $<$<CONFIG:RelWithDebInfo>:-g>

        $<$<CONFIG:Release>:-O3>
    )
endif()
