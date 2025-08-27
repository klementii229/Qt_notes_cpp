cmake_minimum_required(VERSION 3.16)

project(Qt3 VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Charts Sql)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Charts Sql)

set(PROJECT_SOURCES
    main.cpp
    main.h
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    res.qrc
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(Qt3
        ${PROJECT_SOURCES}
    )
else()
    add_executable(Qt3
        ${PROJECT_SOURCES}
    )
endif()

target_precompile_headers(Qt3 PRIVATE pch.h)

target_link_libraries(Qt3 PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Charts
    Qt${QT_VERSION_MAJOR}::Sql
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(Qt3)
endif()

