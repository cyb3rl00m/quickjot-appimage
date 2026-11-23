cmake_minimum_required(VERSION 3.16)

project(QuickJot
    VERSION 1.0.0
    DESCRIPTION "Lightweight offline notes application"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)

add_executable(quickjot
    src/main.cpp
    src/MainWindow.cpp
    src/MainWindow.h
    src/NoteModel.cpp
    src/NoteModel.h
    src/NoteStorage.cpp
    src/NoteStorage.h
    resources/resources.qrc
)

target_include_directories(quickjot PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(quickjot PRIVATE
    Qt6::Widgets
    Qt6::Core
    Qt6::Gui
)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(quickjot PRIVATE -Wall -Wextra -Wpedantic)
endif()

install(TARGETS quickjot RUNTIME DESTINATION bin)

install(FILES packaging/quickjot.desktop
        DESTINATION share/applications
        OPTIONAL)

install(FILES packaging/quickjot.svg
        DESTINATION share/icons/hicolor/scalable/apps
        OPTIONAL)
