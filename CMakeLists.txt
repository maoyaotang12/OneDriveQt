cmake_minimum_required(VERSION 3.16)
project(OneDriveQt)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets)

add_executable(OneDriveQt
    main.cpp
    mainwindow.h mainwindow.cpp
    settingsdialog.h settingsdialog.cpp
    logindialog.h logindialog.cpp
    icons.qrc
)

target_link_libraries(OneDriveQt Qt6::Core Qt6::Widgets)
