cmake_minimum_required(VERSION 3.16)
project(TransitIQ VERSION 2.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Find Qt6 components
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)

include_directories(include)

# Core application sources
set(TRANSITIQ_SOURCES
    src/Common.cpp
    src/Graph.cpp
    src/BusManager.cpp
    src/DataManager.cpp
    src/UI.cpp
    src/App.cpp
    src/QtMapWidget.cpp
    src/QtMainWindow.cpp
    src/main.cpp
)

# Header files with Q_OBJECT
set(TRANSITIQ_HEADERS
    include/Common.h
    include/Stop.h
    include/Road.h
    include/Bus.h
    include/Graph.h
    include/BusManager.h
    include/DataManager.h
    include/UI.h
    include/App.h
    include/QtMapWidget.h
    include/QtMainWindow.h
)

add_executable(TransitIQ ${TRANSITIQ_SOURCES} ${TRANSITIQ_HEADERS})
target_link_libraries(TransitIQ PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets)

# Set Windows subsystem for clean GUI execution
if(WIN32)
    set_target_properties(TransitIQ PROPERTIES WIN32_EXECUTABLE TRUE)
endif()

# Automated Algorithm Test Suite target
set(TEST_SOURCES
    src/Common.cpp
    src/Graph.cpp
    src/BusManager.cpp
    src/DataManager.cpp
    src/UI.cpp
    tests/test_algorithms.cpp
)

add_executable(test_algorithms ${TEST_SOURCES})
