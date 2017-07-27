
#include "RigEditor_Gui.h"
#include "Application.h"

const float RigEditor::Gui::TOP_MENUBAR_HEIGHT = 20.f;

RigEditor::Gui::Gui():
    m_is_help_window_open(false)
{}

void RigEditor::Gui::Draw()
{
    if (m_is_help_window_open)
        this->DrawHelpWindow();

    this->DrawTopMenubar();
    this->DrawSoftbodyPanel();
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
    ImGui::BeginMainMenuBar(); // TODO: enforce the TOP_MENUBAR_HEIGHT.

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

bool RigEditor::Gui::DrawAggregateCheckbox(const char* title, bool *value, bool* is_uniform)
{
    // TODO: change text color when non-uniform
    if (ImGui::Checkbox(title, value))
    {
        *is_uniform = true;
        return true;
    }
    return false;
}

void RigEditor::Gui::DrawSoftbodyPanel()
{
    const float WIDTH = 250.f;
    const int FLAGS = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar;

    ImGui::SetNextWindowSize(ImVec2(WIDTH, ImGui::GetIO().DisplaySize.y));
    ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x - WIDTH, TOP_MENUBAR_HEIGHT));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.f);
    ImGui::Begin("Softbody", nullptr, FLAGS);
    ImGui::PopStyleVar();

    // ---------- Nodes ---------- //

    RoR::GStr<64> nodes_title;
    nodes_title << "Nodes [" << m_node_sel.num_selected << "]";
    if (ImGui::CollapsingHeader(nodes_title))
    {
        if (m_node_sel.num_selected > 0)
        {
            if (m_node_sel.num_selected == 1)
            {
                if (ImGui::InputText("ID", m_node_sel.id.GetBuffer(), m_node_sel.id.GetCapacity(), ImGuiInputTextFlags_EnterReturnsTrue))
                {
                    //TODO: commit update
                }
            }

            this->DrawAggregateCheckbox("[m] No mouse grab",      &m_node_sel.options_values.option_m_no_mouse_grab,     &m_node_sel.options_uniform.option_m_no_mouse_grab);
            this->DrawAggregateCheckbox("[c] No ground contact",  &m_node_sel.options_values.option_c_no_ground_contact, &m_node_sel.options_uniform.option_c_no_ground_contact);
            this->DrawAggregateCheckbox("[h] Is hook point",      &m_node_sel.options_values.option_h_hook_point,        &m_node_sel.options_uniform.option_h_hook_point);
            this->DrawAggregateCheckbox("[b] Extra buoyancy",     &m_node_sel.options_values.option_b_extra_buoyancy,    &m_node_sel.options_uniform.option_b_extra_buoyancy);
            this->DrawAggregateCheckbox("[l] Override weight",    &m_node_sel.options_values.option_l_load_weight,       &m_node_sel.options_uniform.option_l_load_weight);
            if (m_node_sel.options_values.option_l_load_weight)
            {
                ImGui::PushItemWidth(100.f);
                if (ImGui::InputFloat("Load weight (Kg)", &m_node_sel.weight_override, 0.f, 0.f, -1, ImGuiInputTextFlags_EnterReturnsTrue))
                {
                    //TODO: commit update
                }
                ImGui::PopItemWidth();
            }
            this->DrawAggregateCheckbox("[f] Gfx: No sparks",     &m_node_sel.options_values.option_f_no_sparks,         &m_node_sel.options_uniform.option_f_no_sparks);
            this->DrawAggregateCheckbox("[x] Gfx: Exhaust point", &m_node_sel.options_values.option_x_exhaust_point,     &m_node_sel.options_uniform.option_x_exhaust_point);
            this->DrawAggregateCheckbox("[y] Gfx: Exhaust dir.",  &m_node_sel.options_values.option_y_exhaust_direction, &m_node_sel.options_uniform.option_y_exhaust_direction);
            this->DrawAggregateCheckbox("[p] Gfx: No particles",  &m_node_sel.options_values.option_p_no_particles,      &m_node_sel.options_uniform.option_p_no_particles);
        }
        else
        {
            ImGui::TextDisabled("None selected");
        }
    }



    // ---------- Beams ---------- //

    RoR::GStr<64> beams_title;
    beams_title << "Beams [0]"; // TODO
    ImGui::CollapsingHeader(beams_title);

    // ---------- Node presets ---------- //

    ImGui::CollapsingHeader("Node presets");

    // ---------- Beam presets ---------- //

    ImGui::CollapsingHeader("Beam presets");

    ImGui::End();
}

