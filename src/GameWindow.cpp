#include <cstdio>
#include <imgui.h>
#include <ImGuiFileDialog.h>

void ShowGameFileMenu() {
    if(ImGui::BeginMenu("File")) {
        if(ImGui::MenuItem("Open NES ROM image...")) {
            igfd::ImGuiFileDialog::Instance()->OpenDialog("OpenNESROMDlg", "Open NES ROM", ".nes", ".");
        }
        ImGui::EndMenu();
    }
}

void ShowGameWindow() {
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar;
    if(!ImGui::Begin("Game", NULL, window_flags)) {
        ImGui::End();
        return;
    }
    if(ImGui::BeginMenuBar()) {
        ShowGameFileMenu();
        ImGui::EndMenuBar();
    }
    ImGui::End();

    // Dialogs
    if(igfd::ImGuiFileDialog::Instance()->FileDialog("OpenNESROMDlg")) {
        if(igfd::ImGuiFileDialog::Instance()->IsOk) {
            std::string filePathName = igfd::ImGuiFileDialog::Instance()->GetFilePathName();
            std::string filePath = igfd::ImGuiFileDialog::Instance()->GetCurrentPath();
            printf("File name: %s\n", filePathName.c_str());
        }

        igfd::ImGuiFileDialog::Instance()->CloseDialog("OpenNESROMDlg");
    }
}
