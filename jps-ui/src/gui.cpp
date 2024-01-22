// Copyright © 2024 Forschungszentrum Jülich GmbH
// SPDX-License-Identifier: LGPL-3.0-or-later
#include "gui.hpp"
#include "mesh.hpp"
#include "rendering_mesh.hpp"
#include "wkt.hpp"

#include <imgui_internal.h>

#include <ImGuiFileDialog.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <filesystem>
#include <memory>

std::string to_shortcut(const ImGuiKeyChord key)
{
    std::array<char, 32> shortcut{};
    ImGui::GetKeyChordName(key, shortcut.data(), shortcut.size());
    return {shortcut.data()};
}

void Gui::Draw()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    should_recenter = false;

    if(ImGui::BeginMainMenuBar()) {
        if(ImGui::BeginMenu("Menu")) {
            static const auto open_shortcut = to_shortcut(ImGuiMod_Shortcut | ImGuiKey_O);
            if(ImGui::MenuItem("Open", open_shortcut.c_str())) {
                ImGuiFileDialog::Instance()->OpenDialog(
                    "ChooseFileDlgKey",
                    "Choose File",
                    ".wkt",
                    ".",
                    1,
                    nullptr,
                    ImGuiFileDialogFlags_Modal);
            }
            if(ImGui::MenuItem("Center View", "C")) {
                should_recenter = true;
            }
            ImGui::Separator();
            static const auto exit_shortcut = to_shortcut(ImGuiMod_Shortcut | ImGuiKey_Q);
            if(ImGui::MenuItem("Exit", exit_shortcut.c_str())) {
                should_exit = true;
            }
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }

    if(ImGui::IsKeyChordPressed(ImGuiMod_Shortcut | ImGuiKey_O)) {
        ImGuiFileDialog::Instance()->OpenDialog(
            "ChooseFileDlgKey", "Choose File", ".wkt", ".", 1, nullptr, ImGuiFileDialogFlags_Modal);
    }

    if(ImGui::IsKeyChordPressed(ImGuiMod_Shortcut | ImGuiKey_Q)) {
        should_exit = true;
    }


    if(ImGui::IsKeyChordPressed(ImGuiKey_C)) {
        should_recenter = true;
    }

    if(ImGuiFileDialog::Instance()->Display("ChooseFileDlgKey")) {
        if(ImGuiFileDialog::Instance()->IsOk()) {
            std::filesystem::path filePathName = ImGuiFileDialog::Instance()->GetFilePathName();
            std::filesystem::path filePath = ImGuiFileDialog::Instance()->GetCurrentPath();

            const auto wkt = read_wkt(filePath / filePathName);
            if(wkt) {
                geo = std::make_unique<DrawableGEOS>(wkt);
                const Mesh m(geo->tri());
                mesh = std::make_unique<RenderingMesh>(m);
                should_recenter = true;
            }
        }
        ImGuiFileDialog::Instance()->Close();
    }

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}
