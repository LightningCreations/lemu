cmake_minimum_required(VERSION 3.13)
project(LEMU C CXX)

add_subdirectory(imgui EXCLUDE_FROM_ALL)

add_executable(lemu src/main.cpp src/GameWindow.hpp src/GameWindow.cpp ImGuiFileDialog/ImGuiFileDialog/ImGuiFileDialog.cpp)
target_link_libraries(lemu PUBLIC imgui imgui-sdl imgui-opengl3)
target_include_directories(lemu PRIVATE ImGuiFileDialog/ImGuiFileDialog)
