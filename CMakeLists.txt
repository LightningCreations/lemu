cmake_minimum_required(VERSION 3.1.0)
project(lemu CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

if(CMAKE_VERSION VERSION_LESS "3.7.0")
	set(CMAKE_INCLUDE_CURRENT_DIR ON)
endif(CMAKE_VERSION VERSION_LESS "3.7.0")

find_package(Qt5 COMPONENTS REQUIRED Core Gui Widgets)

add_executable(lemu
	src/mainwindow.ui
	src/mainwindow.cpp
	src/main.cpp
	src/version.h
)

target_link_libraries(lemu
	Qt5::Core
	Qt5::Gui
	Qt5::Widgets
)

if(WIN32)
	add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:Qt5::Core> $<TARGET_FILE_DIR:${PROJECT_NAME}>
		COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:Qt5::Gui> $<TARGET_FILE_DIR:${PROJECT_NAME}>
		COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:Qt5::Widgets> $<TARGET_FILE_DIR:${PROJECT_NAME}> )
endif(WIN32)
