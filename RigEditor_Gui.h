#pragma once

#include "ImguiManager.h"
#include "Application.h" // Debugging - copy of RoR

namespace RigEditor {

class Gui
{
public:
    Gui();

    void                 Draw();

private:
    void                 DrawTopMenubar();
    void                 DrawHelpWindow();

    bool            m_is_help_window_open;
};

} // namespace RigEditor
