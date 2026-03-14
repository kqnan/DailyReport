cmake_minimum_required(VERSION 3.26)
project(DailyReport VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Core Network Test)

# Find Qt6 installation directory
get_target_property(Qt6CoreLocation Qt6::Core LOCATION)
get_filename_component(Qt6ParentDir "${Qt6CoreLocation}" DIRECTORY)
get_filename_component(Qt6Dir "${Qt6ParentDir}" DIRECTORY)

message(STATUS "Qt6 installation directory: ${Qt6Dir}")

# Find the qxcb platform plugin
find_path(Qt6QXcbPluginPath
    NAMES platforms/libqxcb.so
    PATHS "${Qt6Dir}/plugins" "/usr/lib64/qt6/plugins"
    NO_DEFAULT_PATH
)

message(STATUS "Qt6 QXcb plugin path: ${Qt6QXcbPluginPath}")

add_executable(DailyReport
    src/main.cpp
    src/mainwindow.h
    src/mainwindow.cpp
    src/datamodel.h
    src/datamodel.cpp
    src/sessionmanager.h
    src/sessionmanager.cpp
    src/cloudsessionmanager.h
    src/cloudsessionmanager.cpp
    src/utils.h
    src/utils.cpp
    src/apimanager.h
    src/apimanager.cpp
    src/logindialog.h
    src/logindialog.cpp
)

# Unit tests
add_executable(test_apimanager
    tests/test_apimanager.cpp
    src/datamodel.h
    src/datamodel.cpp
    src/sessionmanager.h
    src/sessionmanager.cpp
    src/cloudsessionmanager.h
    src/cloudsessionmanager.cpp
    src/apimanager.h
    src/apimanager.cpp
    src/utils.h
    src/utils.cpp
)

add_executable(test_login
    tests/test_login.cpp
    src/datamodel.h
    src/datamodel.cpp
    src/sessionmanager.h
    src/sessionmanager.cpp
    src/cloudsessionmanager.h
    src/cloudsessionmanager.cpp
    src/apimanager.h
    src/apimanager.cpp
    src/utils.h
    src/utils.cpp
)

add_executable(test_e2e
    tests/test_e2e.cpp
    src/mainwindow.h
    src/mainwindow.cpp
    src/datamodel.h
    src/datamodel.cpp
    src/sessionmanager.h
    src/sessionmanager.cpp
    src/cloudsessionmanager.h
    src/cloudsessionmanager.cpp
    src/apimanager.h
    src/apimanager.cpp
    src/utils.h
    src/utils.cpp
    src/logindialog.h
    src/logindialog.cpp
)

target_link_libraries(test_apimanager PRIVATE
    Qt6::Widgets
    Qt6::Core
    Qt6::Network
    Qt6::Test
)

target_link_libraries(test_login PRIVATE
    Qt6::Widgets
    Qt6::Core
    Qt6::Network
    Qt6::Test
)

target_link_libraries(test_e2e PRIVATE
    Qt6::Widgets
    Qt6::Core
    Qt6::Network
    Qt6::Test
)

target_link_libraries(DailyReport PRIVATE Qt6::Widgets Qt6::Core Qt6::Network)

# Copy Qt plugins for standalone executable
if(Qt6QXcbPluginPath)
    add_custom_command(TARGET DailyReport POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/plugins/platforms
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${Qt6QXcbPluginPath}/platforms/libqxcb.so ${CMAKE_BINARY_DIR}/plugins/platforms/libqxcb.so
        COMMENT "Copying Qt platform plugins"
    )
endif()
