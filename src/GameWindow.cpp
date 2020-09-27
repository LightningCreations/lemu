#include <cstdio>
#include <imgui.h>
#include <ImGuiFileDialog.h>

// Code copied from ImGui examples
#if defined(IMGUI_IMPL_OPENGL_LOADER_GL3W)
#include <GL/gl3w.h>
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLEW)
#include <GL/glew.h>
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD)
#include <glad/glad.h>
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD2)
#include <glad/gl.h>
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLBINDING2)
#define GLFW_INCLUDE_NONE
#include <glbinding/Binding.h>
#include <glbinding/gl/gl.h>
using namespace gl;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLBINDING3)
#define GLFW_INCLUDE_NONE
#include <glbinding/glbinding.h>
#include <glbinding/gl/gl.h>
using namespace gl;
#else
#include IMGUI_IMPL_OPENGL_LOADER_CUSTOM
#endif

#include "NESEmu.hpp"

static unsigned int game_texture;
static unsigned char *game_texture_data;

void InitGameWindow() {
    game_texture_data = (unsigned char*) malloc(512*480*4);
    for(uint32_t i = 0; i < 512*480; i++) {
        game_texture_data[(i<<2)+0] = 0;
        game_texture_data[(i<<2)+1] = 0;
        game_texture_data[(i<<2)+2] = 0;
        game_texture_data[(i<<2)+3] = 255;
    }

    glGenTextures(1, &game_texture);
    glBindTexture(GL_TEXTURE_2D, game_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 512, 480, 0, GL_RGBA, GL_UNSIGNED_BYTE, game_texture_data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
}

void ShowGameFileMenu() {
    if(ImGui::BeginMenu("File")) {
        if(ImGui::MenuItem("Open NES ROM image...")) {
            igfd::ImGuiFileDialog::Instance()->OpenDialog("OpenNESROMDlg", "Open NES ROM", ".nes", ".");
        }
        ImGui::EndMenu();
    }
}

void ShowGameWindow() {
    // Run the actual game before we deal with ImGui
    NESEmuDoFrame();

    glBindTexture(GL_TEXTURE_2D, game_texture);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 512, 480, GL_RGBA, GL_UNSIGNED_BYTE, game_texture_data);

    ImGui::SetNextWindowSize(ImVec2(512, 544), ImGuiCond_FirstUseEver);

    ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar;
    if(!ImGui::Begin("Game", NULL, window_flags)) {
        ImGui::End();
        return;
    }
    if(ImGui::BeginMenuBar()) {
        ShowGameFileMenu();
        ImGui::EndMenuBar();
    }
    ImGui::Image((ImTextureID) game_texture, ImVec2(512, 480));
    ImGui::End();

    // Dialogs
    if(igfd::ImGuiFileDialog::Instance()->FileDialog("OpenNESROMDlg")) {
        if(igfd::ImGuiFileDialog::Instance()->IsOk) {
            std::string filePathName = igfd::ImGuiFileDialog::Instance()->GetFilePathName();
            std::string filePath = igfd::ImGuiFileDialog::Instance()->GetCurrentPath();
            NESEmuLoadROM(filePathName);
        }

        igfd::ImGuiFileDialog::Instance()->CloseDialog("OpenNESROMDlg");
    }
}
