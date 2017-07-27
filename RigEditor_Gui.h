#pragma once

#include "ImguiManager.h"
#include "Application.h" // Debugging - copy of RoR

#include "RigEditor_Project.h"

namespace RigEditor {

struct Theme ///< All visual styles, including both GUI and the project visualizations
{

};

class Gui
{
public:
    static const float TOP_MENUBAR_HEIGHT;

    Gui();

    void                 Draw();
    void                 UpdateNodeSelection(SoftbodyNode::Selection& sel) { m_node_sel = sel; }

private:
    void                 DrawTopMenubar();
    void                 DrawHelpWindow();
    void                 DrawSoftbodyPanel(); // Static position: right side of screen, all the way
    void                 DrawSoftbodyPanelNodesSection();
    bool                 DrawAggregateCheckbox(const char* title, bool *value, bool* is_uniform);

    bool                      m_is_help_window_open;
    SoftbodyNode::Selection   m_node_sel;

    Theme           m_theme;
    Project*        m_project;
};

} // namespace RigEditor
