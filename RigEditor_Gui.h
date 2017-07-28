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
/// |                   [TopMenubar]                  |
/// |-------------------------------------------------|
/// |            |                      |  Node/Beam  |
/// |    List    |                      |  Selections |
/// |            |                      |    form     |
/// |            |      { 3D VIEW }     |             |
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

};

class Gui
{
public:
    static const float TOP_MENUBAR_HEIGHT;

    Gui();

    void                   Draw();
    void                   SetProject(Project* proj)                            { m_project = proj; }
    void                   UpdateNodeSelection(SoftbodyNode::Selection& sel)    { m_node_sel = sel; }

private:
    // GUI - top level drawing
    void                   DrawTopMenubar();
    void                   DrawHelpWindow();
    void                   DrawSoftbodyPanel(); // Static position: right side of screen, all the way

    // GUI - partial drawing
    void                   DrawSoftbodyPanelNodesSection();

    // GUI - utilities
    bool                   DrawAggregateCheckbox(const char* title, bool *value, bool* is_uniform);
    SoftbodyNode::Preset*  DrawNodePresetCombo(const char* title, SoftbodyNode::Preset* current, bool current_is_uniform); ///< Returns new selection (only if differs from previous)

    bool                      m_is_help_window_open;
    SoftbodyNode::Selection   m_node_sel;
    SoftbodyNode::Preset*     m_selected_node_preset; ///< Item edited in SoftbodyPanel/

    Theme           m_theme;
    Project*        m_project;
};

} // namespace RigEditor
