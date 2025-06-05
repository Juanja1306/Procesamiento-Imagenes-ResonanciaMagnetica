cmake_minimum_required(VERSION 3.10)
project(RMProcessorQt)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt5 COMPONENTS Widgets REQUIRED)
find_package(OpenCV REQUIRED)
find_package(ITK REQUIRED)

include(${ITK_USE_FILE})

# Incluir carpetas de OpenCV e ITK
include_directories(
    ${OpenCV_INCLUDE_DIRS}
    ${ITK_INCLUDE_DIRS}
)

add_executable(RMProcessorQt
    main.cpp
    MainWindow.h
    MainWindow.cpp
    VideoDialog.h
    VideoDialog.cpp
    Utils.h
    Utils.cpp
    Filtros.h
    Filtros.cpp
)

target_link_libraries(RMProcessorQt
    Qt5::Widgets
    ${OpenCV_LIBS}
    ${ITK_LIBRARIES}
)
