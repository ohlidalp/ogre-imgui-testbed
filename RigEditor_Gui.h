#pragma once

#include "ImguiManager.h"
#include "Application.h" // Debugging - copy of RoR

#include "RigEditor_Project.h"

/// @file
/// @author Petr Ohlidal, 07/2017
/// @brief  GUI layout and logic.
///
/// RigEditor's display is partitioned into panels with given roles:
/// +-------------------------------------------------+
/// |                  [TopMenubar]                   |
/// |-------------------------------------------------|
/// |            |                      |  Node/Beam  |
/// |    List    |                      |  Selections |
/// |            |                      |    form     |
/// |            |     { 3D VIEW }      |             |
/// [ElementPanel]                      [SoftbodyPanel]
/// |            |                      |             |
/// |            |                      |  Node/Beam  |
/// |   Detail   | -------------------- |   Presets   |
/// |            |  [Mouse hover info]  |    form     |
/// +-------------------------------------------------+
///
/// * TopMenubar:      Classic open/save/export/import menu; Help menu; Also editor mode selection - determines content of ElementPanel
/// * ElementPanel:    List & edit form for truckfile elements (i.e. wheels, wings, videocameras, props/flexbodies, flares...)
/// * MouseHoverInfo:  Details or hints for current mouse-pointed item in 3D view. May also display hints for GUI items.
/// * SoftbodyPanel:
///   - Selections form: Displays aggregate info of selections from 3D view. Provides bulk editing of the selection. Can assign preset to the selection.
///   - Presets form:    Allows editing individual presets.

namespace RigEditor {

struct Theme ///< All visual styles, including both GUI and the project visualizations
{
    // TODO
};

class Gui
{
public:
    static const float TOP_MENUBAR_HEIGHT; // TODO: Put this in Theme ~ 09/2017

    Gui();

    void                   Draw();
    void                   SetProject(Project* proj)                            { m_project = proj; }

private:

    enum class UpdateTarget
    {
        NONE,
        SOFTBODY_NODE,
        SOFTBODY_BEAM,
    };

    class ScopedUiHelper ///< Helper for drawing UIs which update the project.
    {
    public:
        inline ScopedUiHelper(Project* proj, UpdateTarget target, const char* id)
            : m_target(target), m_project(proj)
        {
            ImGui::PushID(id);
        }
        inline ~ScopedUiHelper()
        {
            ImGui::PopID();
        }

        bool         DrawAggregateCheckbox(const char* title, bool *value, bool& is_uniform);
        bool         DrawAggregateInputFloat(const char* title, float* value, bool& is_uniform);
        bool         DrawAggregateInputInt(const char* title, int* value_ptr, bool& is_uniform);
        bool         DrawAggregateInputText(const char* title, char* buffer, size_t buf_size, bool& is_uniform);
        void         PushUpdates();

    private:
        UpdateTarget m_target;
        Project*     m_project;
    };

    // GUI - top level drawing
    void                   DrawTopMenubar();
    void                   DrawHelpWindow();
    void                   DrawSoftbodyPanel(); // Static position: right side of screen, all the way

    // GUI - partial drawing
    void                   DrawSoftbodyPanelNodesSection();
    void                   DrawSoftbodyPanelBeamsSection();
    void                   DrawSoftbodyPanelNodePresetsSection();
    void                   DrawSoftbodyPanelBeamPresetsSection();

    // GUI - utilities
    bool                   DrawCheckbox(const char* title, bool *value);
    bool                   DrawNodePresetCombo(SoftbodyNode::Preset*& out_preset, const char* title,
                                               SoftbodyNode::Preset* current, bool cur_is_uniform); ///< Returns true if new selection was made

    bool                      m_is_help_window_open;
    SoftbodyNode::Preset*     m_node_preset_edit; ///< Item edited in SoftbodyPanel/NodePresetForm
    SoftbodyBeam::Preset*     m_beam_preset_edit; ///< Item edited in SoftbodyPanel/BeamPresetForm

    Theme           m_theme;
    Project*        m_project;
};

} // namespace RigEditor
