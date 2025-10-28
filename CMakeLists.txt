cmake_minimum_required(VERSION 3.16)
project(FiniteAlgebra VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Qt6
find_package(Qt6 REQUIRED COMPONENTS Core Widgets)

# Enable automatic MOC, UIC, and RCC
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Qt Application
set(QT_SOURCES
    mainwindow.cpp
    main_qt.cpp
    algebra.cpp
    hassediagramwidget.cpp
)

set(QT_HEADERS
    mainwindow.h
    algebra.h
    hassediagramwidget.h
)

add_executable(QtAlgebraCalculator ${QT_SOURCES} ${QT_HEADERS})
target_link_libraries(QtAlgebraCalculator PRIVATE Qt6::Core Qt6::Widgets)

set_target_properties(QtAlgebraCalculator PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
)

# Console 
add_executable(algebra main.cpp algebra.cpp)
