
#include "RigEditor_Gui.h"

RigEditor::Gui::Gui():
    m_is_help_window_open(false)
{}

void RigEditor::Gui::Draw()
{
    if (m_is_help_window_open)
        this->DrawHelpWindow();

    this->DrawTopMenubar();
}

void RigEditor::Gui::DrawHelpWindow()
{
    ImGui::SetNextWindowSize(ImVec2(600, 400), ImGuiSetCond_Once);
    ImGui::SetNextWindowPosCenter(ImGuiSetCond_Once);
    ImGui::Begin("RigEditor Help", &m_is_help_window_open, ImGuiWindowFlags_NoCollapse);

    ImGui::TextWrapped("RigEditor lets you create and modify softbody actors of Rigs of Rods\n"
        "These can be anything from soda can to the most advanced vehicles, be it land, air or nautical ones.\n"
        "It has Blender-like controls and behavior.");

    ImGui::Text("*** UNDER CONSTRUCTION ***\nThis tool is very early in development - expect bugs and quirks");

    if (ImGui::CollapsingHeader("Features"))
    {
        ImGui::BulletText("Import/export of truckfiles (.truck .car etc...)");
        ImGui::BulletText("Saving/loading of RigEditor project files (.json) - experimental");
        ImGui::BulletText("Blender-like camera controls: perspective/ortho switching, axis-aligned views");
        ImGui::BulletText("Moving nodes with G key (Blender-like)");
        ImGui::BulletText("Adding/deleting nodes with mouse");
        ImGui::BulletText("Adding/deleting beams with mouse");

        ImGui::Text("Supports following elements:");
        ImGui::BulletText("Nodes: nodes, nodes2");
        ImGui::BulletText("Beams: beams, shocks[2], hydros, commands[2]");
        ImGui::BulletText("Engines: engine, engoption (NOTE: import only)");
        ImGui::BulletText("Physics: globals, minimass, rollon, enable_advanced_deformation");
        ImGui::BulletText("Visuals: set_skeleton_settings, guisettings, help");
        ImGui::BulletText("Global: fileinfo, author, [forward/import]commands, rescuer");
    }

    if (ImGui::CollapsingHeader("Controls"))
    {
        ImGui::Text("Camera:");
        ImGui::BulletText("RMB        = Hold and move mouse to rotate camera");
        ImGui::BulletText("MouseWheel = Zoom camera");
        ImGui::BulletText("Numpad 5   = Toggle perspective/ortho view");
        ImGui::BulletText("Numpad 1   = Front view");
        ImGui::BulletText("Numpad 3   = Left view");
        ImGui::BulletText("Numpad 7   = Top view");

        ImGui::Text("Editing:");
        ImGui::BulletText("LMB        = Select/deselect node");
        ImGui::BulletText("CTRL + LMB = Select multiple nodes");
        ImGui::BulletText("G          = Grab node[s] (LMB to release)");
        ImGui::BulletText("N + LMB    = Add new node");
        ImGui::BulletText("DELETE     = Show 'delete menu'\n>"
            " > 'Nodes & beams' => del. selected nodes and all attached beams.\n"
            " > 'Beams only' => del. only beams between selected nodes.");
    }

    ImGui::End();
}

void RigEditor::Gui::DrawTopMenubar()
{
    ImGui::BeginMainMenuBar();

    if (ImGui::BeginMenu("File"))
    {
        if (ImGui::MenuItem("New project"))        {  }
        if (ImGui::MenuItem("Load project"))       {  }
        if (ImGui::MenuItem("Save project as...")) {  }
        if (ImGui::MenuItem("Close project"))      {  }
        ImGui::Separator();                           
        if (ImGui::MenuItem("Import truckfile"))   {  }
        if (ImGui::MenuItem("Export truckfile"))   {  }
        ImGui::Separator();                           
        if (ImGui::MenuItem("Exit"))               {  }

        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Project"))
    {
        if (ImGui::MenuItem("General properties"))        { }
        if (ImGui::MenuItem("Land vehicle properties"))   { }
        ImGui::Separator();
        ImGui::MenuItem("Left-side panel:", nullptr, false, false); // Just a caption - force disabled
        if (ImGui::MenuItem("None"))                      {  }
        if (ImGui::MenuItem("Wheels"))                    {  }

        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Help"))
    {
        ImGui::EndMenu();
    }

    ImGui::EndMainMenuBar();
}

