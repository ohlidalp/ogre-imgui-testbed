
#include "RigEditor_Gui.h"
#include "Application.h"

// ------------------------------------ Basics ------------------------------------ //

const float RigEditor::Gui::TOP_MENUBAR_HEIGHT = 20.f;

static const char* COMBO_ENTRY_NO_PRESET = "~ No preset ~";

RigEditor::Gui::Gui():
    m_is_help_window_open(false),
    m_node_preset_edit(nullptr),
    m_beam_preset_edit(nullptr)
{}

void RigEditor::Gui::Draw()
{
    if (m_is_help_window_open)
        this->DrawHelpWindow();

    this->DrawTopMenubar();
    this->DrawSoftbodyPanel();
}

// ------------------------------------ Utilities ------------------------------------ //

bool RigEditor::Gui::DrawAggregateCheckbox(const char* title, bool *value, bool* is_uniform /*=nullptr*/)
{
    // TODO: change text color when non-uniform
    if (ImGui::Checkbox(title, value))
    {
        if (is_uniform != nullptr)
            *is_uniform = true;
        return true;
    }
    return false;
}

template<typename T>
static bool StdVectorComboItemGetter(std::vector<T>& vec, int index, const char** out_text) // Utility for node/beam presets
{
    if (index >= 0 && index < static_cast<int>(vec.size()))
    {
        (*out_text) = vec[index]->name.GetBuffer();
        return true;
    }
    (*out_text) = nullptr;
    return false;
}

static bool NodePresetComboItemGetter(void* data, int index, const char** out_text)
{
    if (index == 0)
    {
        *out_text = COMBO_ENTRY_NO_PRESET;
        return true;
    }
    auto* softbody = reinterpret_cast<RigEditor::Project::Softbody*>(data);
    return StdVectorComboItemGetter<RigEditor::SoftbodyNode::Preset*>(softbody->node_presets, index - 1, out_text);
}

bool RigEditor::Gui::DrawNodePresetCombo(RigEditor::SoftbodyNode::Preset*& out_preset, const char* title, SoftbodyNode::Preset* current_preset, bool current_is_uniform)
{
    // Important: we add a dummy item "~ No preset ~" with index 0. Thus, all other indices are shifted by 1.

    int entry_index = (current_is_uniform && (current_preset != nullptr)) ? (m_project->softbody.GetNodePresetArrayIndex(current_preset) + 1) : -1;
    int num_entries = static_cast<int>(m_project->softbody.node_presets.size()) + 1; // Add 1 to make space for dummy "~ No preset~"
    if (ImGui::Combo(title, &entry_index, NodePresetComboItemGetter, &m_project->softbody, num_entries))
    {
        if (entry_index == 0) // Index 0 is the dummy "~ No preset ~" item
        {
            if ((current_is_uniform && (nullptr != current_preset)) || (!current_is_uniform))
            {
                out_preset = nullptr;
                return true;
            }
            return false;
        }
        SoftbodyNode::Preset* pick = m_project->softbody.node_presets[entry_index - 1];
        if ((current_is_uniform && (pick != current_preset)) || (!current_is_uniform))
        {
            out_preset = pick;
            return true;
        }
    }
    return false;
}

static bool BeamPresetComboItemGetter(void* data, int index, const char** out_text)
{
    if (index == 0)
    {
        out_text = &COMBO_ENTRY_NO_PRESET;
        return true;
    }
    auto* softbody = reinterpret_cast<RigEditor::Project::Softbody*>(data);
    return StdVectorComboItemGetter<RigEditor::SoftbodyBeam::Preset*>(softbody->beam_presets, index, out_text);
}

// ------------------------------------ Windows ------------------------------------ //

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

void RigEditor::Gui::DrawSoftbodyPanelNodesSection()
{
    RoR::GStr<64> nodes_title;
    nodes_title << "Nodes [" << m_node_sel.num_selected << "]###idNodes";
    if (!ImGui::CollapsingHeader(nodes_title))
        return; // Section is collapsed -> nothing to do

    if (m_node_sel.num_selected == 0)
    {
        ImGui::TextDisabled("None selected");
        return;
    }

    ImGui::PushID("idNodes_");

    if (m_node_sel.num_selected == 1)
    {
        if (ImGui::InputText("Name", m_node_sel.name.GetBuffer(), m_node_sel.name.GetCapacity(), ImGuiInputTextFlags_EnterReturnsTrue))
        {
            //TODO: commit update to project
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
            //TODO: commit update to project
        }
        ImGui::PopItemWidth();
    }
    this->DrawAggregateCheckbox("[f] Gfx: No sparks",     &m_node_sel.options_values.option_f_no_sparks,         &m_node_sel.options_uniform.option_f_no_sparks);
    this->DrawAggregateCheckbox("[x] Gfx: Exhaust point", &m_node_sel.options_values.option_x_exhaust_point,     &m_node_sel.options_uniform.option_x_exhaust_point);
    this->DrawAggregateCheckbox("[y] Gfx: Exhaust dir.",  &m_node_sel.options_values.option_y_exhaust_direction, &m_node_sel.options_uniform.option_y_exhaust_direction);
    this->DrawAggregateCheckbox("[p] Gfx: No particles",  &m_node_sel.options_values.option_p_no_particles,      &m_node_sel.options_uniform.option_p_no_particles);

    if (this->DrawNodePresetCombo(m_node_sel.node_preset, "Preset", m_node_sel.node_preset, m_node_sel.node_preset_is_uniform))
    {
        m_node_sel.node_preset_is_uniform = true;        //TODO: commit update to project
    }

    ImGui::PopID();
}

void RigEditor::Gui::DrawSoftbodyPanelBeamsSection()
{
    RoR::GStr<64> title;
    title << "Beams [" << m_beam_sel.num_selected << "]###idBeams";
    if (!ImGui::CollapsingHeader(title.GetBuffer()))
        return; // Section is collapsed -> nothing to do.

    if (m_beam_sel.num_selected == 0)
    {
        ImGui::TextDisabled("None selected");
        return;
    }

    if (m_beam_sel.num_selected == 1)
    {
        if (ImGui::InputText("Name", m_beam_sel.name.GetBuffer(), m_beam_sel.name.GetCapacity(), ImGuiInputTextFlags_EnterReturnsTrue))
        {
            //TODO: commit update to project
        }
    }

    // +++ TODO +++
}

void RigEditor::Gui::DrawSoftbodyPanelNodePresetsSection()
{
    if (!ImGui::CollapsingHeader("Node presets"))
        return; // Section collapsed -> nothing to draw.

    ImGui::PushID("idNPresets_");

    this->DrawNodePresetCombo(m_node_preset_edit, "Preset", m_node_preset_edit, true);

    if (m_node_preset_edit == nullptr)
    {
        ImGui::TextDisabled("None selected");
        return;
    }

    ImGui::InputText("Name", m_node_preset_edit->name.GetBuffer(), m_node_preset_edit->name.GetCapacity());

    ImGui::InputFloat("Friction", &m_node_preset_edit->friction);
    ImGui::InputFloat("Volume",   &m_node_preset_edit->volume  );
    ImGui::InputFloat("Surface",  &m_node_preset_edit->surface );

    bool dummy_uniformity = true;
    this->DrawAggregateCheckbox("[m] No mouse grab",      &m_node_preset_edit->options.option_m_no_mouse_grab    ); // TODO: Use widget IDs instead of "" trick
    this->DrawAggregateCheckbox("[c] No ground contact",  &m_node_preset_edit->options.option_c_no_ground_contact);
    this->DrawAggregateCheckbox("[h] Is hook point",      &m_node_preset_edit->options.option_h_hook_point       );
    this->DrawAggregateCheckbox("[b] Extra buoyancy",     &m_node_preset_edit->options.option_b_extra_buoyancy   );
    this->DrawAggregateCheckbox("[l] Override weight",    &m_node_preset_edit->options.option_l_load_weight      );
    if (m_node_preset_edit->options.option_l_load_weight)
    {
        ImGui::PushItemWidth(100.f);
        ImGui::InputFloat("Load weight (Kg)", &m_node_preset_edit->load_weight);
        ImGui::PopItemWidth();
    }
    this->DrawAggregateCheckbox("[f] Gfx: No sparks",     &m_node_preset_edit->options.option_f_no_sparks        );
    this->DrawAggregateCheckbox("[x] Gfx: Exhaust point", &m_node_preset_edit->options.option_x_exhaust_point    );
    this->DrawAggregateCheckbox("[y] Gfx: Exhaust dir.",  &m_node_preset_edit->options.option_y_exhaust_direction);
    this->DrawAggregateCheckbox("[p] Gfx: No particles",  &m_node_preset_edit->options.option_p_no_particles     );

    ImGui::PopID();
}

void RigEditor::Gui::DrawSoftbodyPanelBeamPresetsSection()
{
    if (!ImGui::CollapsingHeader("Beam presets"))
        return; // Section collapsed -> nothing to draw.

    // TODO SoftbodyBeam::Preset* pick = this->DrawBeamPresetCombo("Preset", m_beam_preset_edit, true);
   // if (pick != nullptr)
   //     m_beam_preset_edit = pick;

    if (m_beam_preset_edit == nullptr)
    {
        ImGui::TextDisabled("None selected");
        return;
    }
}

void RigEditor::Gui::DrawSoftbodyPanel()
{
    const float WIDTH = 250.f;
    const int FLAGS = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse
                      | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysVerticalScrollbar;

    ImGui::SetNextWindowSize(ImVec2(WIDTH, ImGui::GetIO().DisplaySize.y));
    ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x - WIDTH, TOP_MENUBAR_HEIGHT));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.f);
    ImGui::Begin("Softbody", nullptr, FLAGS);
    ImGui::PopStyleVar();

    this->DrawSoftbodyPanelNodesSection();
    this->DrawSoftbodyPanelBeamsSection();
    this->DrawSoftbodyPanelNodePresetsSection();
    this->DrawSoftbodyPanelBeamPresetsSection();

    ImGui::End();
}

