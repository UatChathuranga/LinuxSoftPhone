cmake_minimum_required(VERSION 3.10)
project(LinuxSoftPhone VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Use PkgConfig to find liblinphone
find_package(PkgConfig REQUIRED)
pkg_check_modules(LINPHONE REQUIRED linphone)

# Find Qt6
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets WebSockets)

add_executable(LinuxSoftPhone
    src/main.cpp
    src/Agent.cpp
    src/MainWindow.cpp
    src/CommandServer.cpp
    src/AppConfig.cpp
)

target_include_directories(LinuxSoftPhone PRIVATE ${LINPHONE_INCLUDE_DIRS})
target_link_directories(LinuxSoftPhone PRIVATE ${LINPHONE_LIBRARY_DIRS})
target_link_libraries(LinuxSoftPhone PRIVATE ${LINPHONE_LIBRARIES} linphone Qt6::Core Qt6::Gui Qt6::Widgets Qt6::WebSockets)
