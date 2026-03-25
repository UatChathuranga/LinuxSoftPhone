cmake_minimum_required(VERSION 3.10)
project(FacetoneSoftPhone VERSION 1.0)

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

add_executable(facetonesoftphone
    src/main.cpp
    src/Agent.cpp
    src/MainWindow.cpp
    src/CommandServer.cpp
    src/AppConfig.cpp
    src/SettingsWindow.cpp
)

target_include_directories(facetonesoftphone PRIVATE ${LINPHONE_INCLUDE_DIRS})
target_link_directories(facetonesoftphone PRIVATE ${LINPHONE_LIBRARY_DIRS})
target_link_libraries(facetonesoftphone PRIVATE ${LINPHONE_LIBRARIES} linphone Qt6::Core Qt6::Gui Qt6::Widgets Qt6::WebSockets)

# --- Installation ---
include(GNUInstallDirs)

install(TARGETS facetonesoftphone
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(FILES ringtone.wav
    DESTINATION ${CMAKE_INSTALL_DATADIR}/facetonesoftphone
)

install(FILES LinuxSoftPhone.desktop
    DESTINATION ${CMAKE_INSTALL_DATADIR}/applications
    RENAME facetonesoftphone.desktop
)

install(FILES linuxsoftphone.png
    DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/256x256/apps
    RENAME facetonesoftphone.png
)
