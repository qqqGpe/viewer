cmake_minimum_required(VERSION 3.5)
project(LogViewer VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Find Qt5 packages
find_package(Qt5 REQUIRED COMPONENTS Core Widgets Charts)

# Source files
set(SOURCES
    src/main.cpp
    src/MainWindow.cpp
    src/LeftPane.cpp
    src/CanvasTabWidget.cpp
    src/ChartWidget.cpp
    src/DataParser.cpp
    src/DataModel.cpp
)

# Header files
set(HEADERS
    src/MainWindow.h
    src/LeftPane.h
    src/CanvasTabWidget.h
    src/ChartWidget.h
    src/DataParser.h
    src/DataModel.h
    src/models/CategoryItem.h
    src/models/SeriesData.h
)

# Create executable
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# Link Qt libraries
target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt5::Core
    Qt5::Widgets
    Qt5::Charts
)

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)
