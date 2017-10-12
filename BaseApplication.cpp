
#include "stdafx.h" // Precompiled

#include "ImguiManager.h"
#include "Application.h" // Debugging - copy of RoR

#include <memory>    // std::unique_ptr
#include <algorithm> // std::min()


// -------------------------------------- loading spinner prototype -------------------------------------------

void DrawImGuiSpinner(float& counter, const ImVec2 size = ImVec2(16.f, 16.f), const float spacing = 2.f, const float step_sec = 0.15f)
{
    // Hardcoded to 4 segments, counter is reset after full round (4 steps)
    // --------------------------------------------------------------------

    const ImU32 COLORS[] = { ImColor(255,255,255,255), ImColor(210,210,210,255), ImColor(120,120,120,255), ImColor(60,60,60,255) };

    // Update counter, determine coloring
    counter += ImGui::GetIO().DeltaTime;
    int color_start = 0; // Index to GUI_SPINNER_COLORS array for the top middle segment (segment 0)
    while (counter > (step_sec*4.f))
    {
        counter -= (step_sec*4.f);
    }

    if (counter > (step_sec*3.f))
    {
        color_start = 3;
    }
    else if (counter > (step_sec*2.f))
    {
        color_start = 2;
    }
    else if (counter > (step_sec))
    {
        color_start = 1;
    }

    // Draw segments
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    const ImVec2 pos = ImGui::GetCursorScreenPos();
    const float left = pos.x;
    const float top = pos.y;
    const float right = pos.x + size.x;
    const float bottom = pos.y + size.y;
    const float mid_x = pos.x + (size.x / 2.f);
    const float mid_y = pos.y + (size.y / 2.f);

    // NOTE: Enter vertices in clockwise order, otherwise anti-aliasing doesn't work and polygon is rasterized larger! -- Observed under OpenGL2 / OGRE 1.9

    // Top triangle, vertices: mid, left, right
    draw_list->AddTriangleFilled(ImVec2(mid_x, mid_y-spacing),   ImVec2(left + spacing, top),     ImVec2(right - spacing, top),     COLORS[color_start]);
    // Right triangle, vertices: mid, top, bottom
    draw_list->AddTriangleFilled(ImVec2(mid_x+spacing, mid_y),   ImVec2(right, top + spacing),    ImVec2(right, bottom - spacing),  COLORS[(color_start+3)%4]);
    // Bottom triangle, vertices: mid, right, left
    draw_list->AddTriangleFilled(ImVec2(mid_x, mid_y+spacing),   ImVec2(right - spacing, bottom), ImVec2(left + spacing, bottom),   COLORS[(color_start+2)%4]);
    // Left triangle, vertices: mid, bottom, top
    draw_list->AddTriangleFilled(ImVec2(mid_x-spacing, mid_y),   ImVec2(left, bottom - spacing),  ImVec2(left, top + spacing),      COLORS[(color_start+1)%4]);
}

// -------------------------------------- multiplayer connecting status dialog prototype -------------------------------------------

void DrawMpConnecting()
{
    static float spin_counter=0.f;
    const char* infotext = "Joining [stupidly-long-named-sweet-game.rigsofrods.org:12345]";

    const ImVec2 spin_size(20.f, 20.f);
    const float spin_column_w(50.f);
    const int win_flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoInputs
        | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar;

    ImGui::SetNextWindowPosCenter();
    ImGui::Begin("Connecting to MP server...", nullptr, win_flags);
    ImGui::Columns(2);
    ImGui::SetColumnOffset(1, spin_column_w);

    ImGui::SetCursorPos(ImGui::GetCursorPos() + ImVec2(5.f, 7.f)); // NOTE: Hand aligned; I failed calculate the positioning here ~ only_a_ptr, 10/2017
    DrawImGuiSpinner(spin_counter, spin_size);

    ImGui::NextColumn();
    ImGui::Text(infotext);
    ImGui::TextDisabled("Running demo...");
    ImGui::End();
}

// -------------------------------------- game settings UI prototype -------------------------------------

namespace RoR{

class GameSettings
{
public:
    enum SettingsTab { GENERAL, CONTROL, VIDEO, DIAG };

    GameSettings(): m_is_visible(false), m_tab(SettingsTab::GENERAL) {}

    void Draw()
    {
        bool is_visible = true;
        const int flags = ImGuiWindowFlags_NoCollapse;
        ImGui::SetNextWindowSize(ImVec2(400.f, 300.f), ImGuiSetCond_FirstUseEver);
        ImGui::SetNextWindowPosCenter(ImGuiSetCond_Appearing);
        ImGui::Begin("Game settings", &is_visible, flags);
        if (! is_visible)
        {
            this->SetVisible(false);
            return;
        }

        // 'Tabs' buttons
        if (ImGui::Button("General"))    { m_tab = SettingsTab::GENERAL; }
        ImGui::SameLine();
        if (ImGui::Button("Controls"))   { m_tab = SettingsTab::CONTROL; }
        ImGui::SameLine();
        if (ImGui::Button("Video"))      { m_tab = SettingsTab::VIDEO;   }
        ImGui::SameLine();
        if (ImGui::Button("Diagnostic")) { m_tab = SettingsTab::DIAG;    }

        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 4.f);
        ImGui::Separator();

        if (m_tab == SettingsTab::GENERAL)
        {
            ImGui::TextDisabled("Application settings");

            int lang_selection = 0;
            ImGui::Combo("Language", &lang_selection, "English\0\0"); // Dummy; TODO: List available languages

            int sshot_select = (std::strcmp(App::app_screenshot_format.GetActive(),"jpg") == 0) ? 1 : 0; // Hardcoded; TODO: list available formats.
            if (ImGui::Combo("Screenshot format", &sshot_select, "png\0jpg\0\0"))
            {
                App::app_screenshot_format.SetActive((sshot_select == 1) ? "jpg" : "png");
            }

            this->DrawGCheckbox(App::app_multithread, "Multithreading");

            ImGui::Separator();
            ImGui::TextDisabled("Simulation settings");

            this->DrawGCheckbox(App::sim_position_storage, "Use position storage");
            this->DrawGCheckbox(App::sim_replay_enabled, "Replay mode");

            if (App::sim_replay_enabled.GetActive())
            {
                int replay_len = App::sim_replay_length.GetActive();
                if (ImGui::InputInt("Replay length", &replay_len))
                {
                    App::sim_replay_length.SetActive(replay_len);
                }

                int replay_step = App::sim_replay_stepping.GetActive();
                if (ImGui::InputInt("Replay stepping", &replay_step))
                {
                    App::sim_replay_stepping.SetActive(replay_len);
                }
            }

            this->DrawGCombo(App::sim_gearbox_mode, "Gearbox mode",
                "Automatic shift\0"
                "Manual shift - Auto clutch\0"
                "Fully Manual: sequential shift\0"
                "Fully manual: stick shift\0"
                "Fully Manual: stick shift with ranges\00");

        }
        else if (m_tab == SettingsTab::VIDEO)
        {
            ImGui::TextDisabled("Video settings");

            this->DrawGCombo(App::gfx_flares_mode, "Lights",
                "None (fastest)\0"
                "No light sources\0"
                "Only current vehicle, main lights\0"
                "All vehicles, main lights\0"
                "All vehicles, all lights\0\0");

            this->DrawGCombo(App::gfx_shadow_type, "Shadow type",
                "Disabled\0"
                "Texture\0"
                "PSSM\0\0");

            this->DrawGCombo(App::gfx_extcam_mode, "Exterior camera mode",
                "None\0"
                "Static\0"
                "Pitching\0\0");

            this->DrawGCombo(App::gfx_sky_mode, "Sky gfx",
                "Sandstorm (fastest)\0"
                "Caelum (best looking, slower)\0"
                "SkyX (best looking, slower)\0\0");

            this->DrawGCombo(App::gfx_texture_filter , "Texture filtering",
                "None\0"
                "Bilinear\0"
                "Trilinear\0"
                "Anisotropic\0\0");

            this->DrawGCombo(App::gfx_vegetation_mode, "Vegetation density",
                "None\0"
                "20%\0"
                "50%\0"
                "Full\0\0");

            this->DrawGCombo(App::gfx_water_mode, "Water gfx",
                "None\0"
                "Basic (fastest)\0"
                "Reflection\0"
                "Reflection + refraction (speed optimized)\0"
                "Reflection + refraction (quality optimized)\0"
                "HydraX\0\0");

            this->DrawGCheckbox(App::gfx_enable_sunburn,   "Sunburn effect");
            this->DrawGCheckbox(App::gfx_water_waves,      "Waves on water");
            this->DrawGCheckbox(App::gfx_minimap_disabled, "Minimap disabled");
            this->DrawGCheckbox(App::gfx_enable_glow,      "Glow (bloom) effect");
            this->DrawGCheckbox(App::gfx_enable_hdr,       "HDR (high dynamic range)");
            this->DrawGCheckbox(App::gfx_enable_heathaze,  "HeatHaze effect");
            this->DrawGCheckbox(App::gfx_enable_videocams, "Render VideoCameras");
            this->DrawGCheckbox(App::gfx_envmap_enabled,   "Realtime reflections");
            this->DrawGIntCheck(App::gfx_particles_mode,   "Enable particle gfx");
            this->DrawGIntCheck(App::gfx_skidmarks_mode,   "Enable skidmarks");

            ImGui::PushItemWidth(100.f); // Width includes [+/-] buttons
            this->DrawGIntBox(App::gfx_envmap_rate,    "Realtime refl. update rate");
            this->DrawGIntBox(App::gfx_fps_limit,      "FPS limit");
            ImGui::PopItemWidth();

            ImGui::PushItemWidth(125.f);
            this->DrawGFloatBox(App::gfx_sight_range,  "Sight range (meters)");
            this->DrawGFloatBox(App::gfx_fov_external, "Exterior FOV (field of view)");
            this->DrawGFloatBox(App::gfx_fov_internal, "Interior FOV (field of view)");
            ImGui::PopItemWidth();

        }
        else if (m_tab == SettingsTab::DIAG)
        {
            ImGui::TextDisabled("Diagnostic options");

            this->DrawGCheckbox(App::diag_rig_log_node_import, "Log node import (spawn)");
            this->DrawGCheckbox(App::diag_rig_log_node_stats,  "Log node stats (spawn)");
            this->DrawGCheckbox(App::diag_rig_log_messages,    "Log messages (spawn)");
            this->DrawGCheckbox(App::diag_collisions,          "Debug collisions");
            this->DrawGCheckbox(App::diag_truck_mass,          "Debug actor mass");
            this->DrawGCheckbox(App::diag_envmap,              "Debug realtime reflections");
            this->DrawGCheckbox(App::diag_videocameras,        "Debug videocameras");
//extern GVarStr<100>            App::diag_preset_terrain;
//extern GVarStr<100>            App::diag_preset_vehicle;
//extern GVarStr<100>            App::diag_preset_veh_config;
            this->DrawGCheckbox(App::diag_preset_veh_enter,    "Enter preselected vehicle");
            this->DrawGCheckbox(App::diag_log_console_echo,    "Echo log to console");
            this->DrawGCheckbox(App::diag_log_beam_break,      "Log beam breaking");
            this->DrawGCheckbox(App::diag_log_beam_deform,     "Log beam deforming");
            this->DrawGCheckbox(App::diag_log_beam_trigger,    "Log beam triggers");
            this->DrawGCheckbox(App::diag_dof_effect,          "Debug DOF (depth of field)");
//extern GVarStr<300>            App::diag_extra_resource_dir;
        }
        else if (m_tab == SettingsTab::CONTROL)
        {
            ImGui::TextDisabled("Controller options");

            this->DrawGCheckbox(App::io_ffb_enabled, "Enable ForceFeedback");
            if (App::io_ffb_enabled.GetActive())
            {
                ImGui::PushItemWidth(125.f);
                this->DrawGFloatBox(App::io_ffb_camera_gain, "FFB camera gain");
                this->DrawGFloatBox(App::io_ffb_center_gain, "FFB center gain");
                this->DrawGFloatBox(App::io_ffb_master_gain, "FFB master gain");
                this->DrawGFloatBox(App::io_ffb_stress_gain, "FFB stress gain");
                ImGui::PopItemWidth();
            }

            this->DrawGCombo(App::io_input_grab_mode, "Input grab mode",
                "None\0"
                "All\0"
                "Dynamic\0\0");

            this->DrawGCheckbox(App::io_arcade_controls, "Use arcade controls");
            this->DrawGIntCheck(App::io_outgauge_mode, "Enable OutGauge protocol");
            if (App::io_outgauge_mode.GetActive())
            {
//<50>              App::io_outgauge_ip
                ImGui::PushItemWidth(125.f);
                this->DrawGIntBox(App::io_outgauge_port,    "OutGauge port");
                this->DrawGIntBox(App::io_outgauge_id,      "OutGauge ID");
                this->DrawGFloatBox(App::io_outgauge_delay, "OutGauge delay");
                ImGui::PopItemWidth();

            }

        }

        ImGui::End();
    }

    inline bool IsVisible() const { return m_is_visible; }
    inline void SetVisible(bool v)
    {
        if (!v)
        {
            m_tab = SettingsTab::GENERAL;
        }
        m_is_visible = v;
    }

    inline void DrawGCheckbox(GVarPod<bool>& gvar, const char* label)
    {
        bool val = gvar.GetActive();
        if (ImGui::Checkbox(label, &val))
        {
            gvar.SetActive(val);
        }
    }

    inline void DrawGIntCheck(GVarPod<int>& gvar, const char* label)
    {
        bool val = (gvar.GetActive() != 0);
        if (ImGui::Checkbox(label, &val))
        {
            gvar.SetActive(val ? 1 : 0);
        }
    }

    template <typename GEnum_T>
    inline void DrawGCombo(GVarEnum<GEnum_T>& gvar, const char* label, const char* values)
    {
        int selection = static_cast<int>(gvar.GetActive());
        if (ImGui::Combo(label, &selection, values))
        {
            gvar.SetActive(static_cast<GEnum_T>(selection));
        }
    }

    inline void DrawGIntBox(GVarPod<int>& gvar, const char* label)
    {
        int val = gvar.GetActive();
        if (ImGui::InputInt(label, &val, 1, 100, ImGuiInputTextFlags_EnterReturnsTrue))
        {
            gvar.SetActive(val);
        }
    }

    inline void DrawGFloatBox(GVarPod<float>& gvar, const char* label)
    {
        float fval = gvar.GetActive();
        if (ImGui::InputFloat(label, &fval, 0.f, 0.f, -1, ImGuiInputTextFlags_EnterReturnsTrue))
        {
            gvar.SetActive(fval);
        }
    }

private:
    bool m_is_visible;
    SettingsTab m_tab;
};

} // namespace RoR

// -------------------------------------- console prototype -------------------------------------------

ImVec4 col_white(1.f, 1.f, 1.f, 1.f);
ImVec4 col_green(0.2f, 0.8f, 0.3f, 1.f);
ImVec4 col_blue(0.1f, 0.4f, 0.8f, 1.f);

class Console
{
public:

    struct ConsoleCmd
    {
        ConsoleCmd(const char* name, const char* syntax, const char* descr): 
            cmd_name(name), cmd_syntax(syntax), cmd_descr(descr) {}

        const char* cmd_name;
        const char* cmd_syntax;
        const char* cmd_descr;
    };

    static const size_t MAX_LINES = 200;
    static const size_t INPUT_BUFFER_LEN = 500;

    static const char* CMD_GET;// = "get";
    static const char* CMD_SET;// = "set";
    static const char* CMD_GRAVITY;// = "gravity";
    static const char* CMD_HELP;// = "help";
    static const char* CMD_VERSION;// = "version";
    static const char* CMD_GOTO;// = "goto";
    static const char* CMD_QUIT;// = "quit";
    static const char* CMD_SCRIPT;// = "script";

    std::string              m_lines[MAX_LINES];
    size_t                   m_lines_caret;
    bool                     m_lines_full;
    std::vector<std::string> m_history;
    char                     m_input_buffer[INPUT_BUFFER_LEN];

    void Draw()
    {
        if (!ImGui::Begin("RoR-Console-Draft"))
        {
            return;
        }

        const float CONSOLE_PADDING = 6.f;
        
        // The actual console body
        const float body_height = -(ImGui::GetItemsLineHeightWithSpacing() * 3.f);
        const int body_flags = ImGuiWindowFlags_HorizontalScrollbar;
        ImGui::BeginChild("ScrollingRegion", ImVec2(0,body_height), false, body_flags);

        // Spacing: minimize spacing to make separate text elements appear as fluent colored text.
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(1.f, 1.f));

        ImVec2 pos = ImGui::GetCursorPos();
        pos.x += CONSOLE_PADDING;
        pos.y += CONSOLE_PADDING;
        ImGui::SetCursorPos(pos);

        // Console body experiments
        ImGui::Text("XXXXX|=====| 5+5");
        
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + CONSOLE_PADDING);
        ImGui::Text("=====|=====| 5+5");

        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + CONSOLE_PADDING);
        ImGui::TextColored(col_blue, "=====|");
        ImGui::SameLine();
        ImGui::TextColored(col_green, "=====|");
        ImGui::SameLine();
        ImGui::Text(" 5x5");
        



        // End body
        ImGui::PopStyleVar();
        ImGui::EndChild();

        // The botom tooltip area
        ImGui::TextColored(col_blue, "  < tooltip here >");

        // The bottom-most input box
        ImGui::InputText("Command:", m_input_buffer, INPUT_BUFFER_LEN);

        // Finalize the window
        ImGui::End();
    }

};

     const char* Console::CMD_GET = "get";
     const char* Console::CMD_SET = "set";
     const char* Console::CMD_GRAVITY = "gravity";
     const char* Console::CMD_HELP = "help";
     const char* Console::CMD_VERSION = "version";
     const char* Console::CMD_GOTO = "goto";
     const char* Console::CMD_QUIT = "quit";
     const char* Console::CMD_SCRIPT = "script";

//// -------------------------------------- Skeletonview test ------------------------------------------

void DrawSkeletonView()
{
    // Var
    ImVec2 screen_size = ImGui::GetIO().DisplaySize;

    // Dummy fullscreen window to draw to
    int window_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar| ImGuiWindowFlags_NoInputs 
                     | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoBringToFrontOnFocus;
    ImGui::Begin("RoR-SoftBodyView", NULL, screen_size, 0, window_flags);
    ImDrawList* drawlist = ImGui::GetWindowDrawList();
    ImGui::End();
    
    // ** COLOR = ABGR **
    float pad1 = 100.f;

    // Some test lines
    ImVec2 mouse_pos = ImGui::GetIO().MousePos;

    ImVec2 hpos_tl = ImVec2( ( (mouse_pos.x + pad1)/2.f),               ((mouse_pos.y + pad1)/2.f));            
    ImVec2 hpos_tr = ImVec2( ( (mouse_pos.x + screen_size.x-pad1)/2.f), ((mouse_pos.y + pad1)/2.f));              
    ImVec2 hpos_br = ImVec2( ( (mouse_pos.x + screen_size.x-pad1)/2.f), ((mouse_pos.y + screen_size.y-pad1)/2.f)); 
    ImVec2 hpos_bl = ImVec2( ( (mouse_pos.x + pad1)/2.f),               ((mouse_pos.y + screen_size.y-pad1)/2.f)); 

    //drawlist->AddLine(mouse_pos, mouse_pos, color, float_thickness
    drawlist->AddLine(ImVec2(pad1,               pad1),               hpos_tl,  0xFFcc5588, 1.f);
    drawlist->AddLine(ImVec2(screen_size.x-pad1, pad1),               hpos_tr,  0xFFcc5599, 2.f);
    drawlist->AddLine(ImVec2(screen_size.x-pad1, screen_size.y-pad1), hpos_br,  0xFF2299dd, 3.f);
    drawlist->AddLine(ImVec2(pad1,               screen_size.y-pad1), hpos_bl,  0xFFcc55bb, 4.f);

    // Some test circles

    drawlist->AddCircleFilled(ImVec2(pad1,               pad1),               5.f, 0xFFcc5588); 
    drawlist->AddCircle      (ImVec2(pad1,               pad1),               8.f, 0xFFddbb33, 16, 2.f);
    drawlist->AddCircleFilled(ImVec2(screen_size.x-pad1, pad1),               10.f, 0xFFcc5599);
    drawlist->AddCircleFilled(ImVec2(screen_size.x-pad1, screen_size.y-pad1), 15.f, 0xFFcc55aa);
    drawlist->AddCircleFilled(ImVec2(pad1,               screen_size.y-pad1), 20.f, 0xFFcc55bb);



    // Triangle
    // ... left side
    drawlist->AddTriangle(hpos_tl, hpos_bl, mouse_pos, 0xFF22cc77);
    // .. right side
    drawlist->AddTriangleFilled(hpos_tr, hpos_br, mouse_pos, 0xFF55cc77);
    drawlist->AddTriangle      (hpos_tr, hpos_br, mouse_pos, 0xFF2299dd, 3.f);
}

//// -------------------------------------- APP ------------------------------------------

class DemoApp: public Ogre::FrameListener, public OIS::KeyListener, public OIS::MouseListener,  public Ogre::WindowEventListener
{
public:
    DemoApp():
        m_is_mpconnect_visible(false),
        m_is_spinner_visible(false),
        m_is_test_window_visible(false),
        m_is_style_editor_visible(false),
        m_is_console_visible(false),
        m_is_skeleton_visible(false)
        {}

    void RoR_SetGuiStyle()
    {
// Exported colors

ImGuiStyle& style = ImGui::GetStyle();
style.Colors[ImGuiCol_Text]                  = ImVec4(0.90f, 0.90f, 0.90f, 1.00f);
style.Colors[ImGuiCol_TextDisabled]          = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
style.Colors[ImGuiCol_WindowBg]              = ImVec4(0.06f, 0.06f, 0.06f, 1.00f);
style.Colors[ImGuiCol_ChildWindowBg]         = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
style.Colors[ImGuiCol_PopupBg]               = ImVec4(0.05f, 0.05f, 0.10f, 1.00f);
style.Colors[ImGuiCol_Border]                = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
style.Colors[ImGuiCol_BorderShadow]          = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
style.Colors[ImGuiCol_FrameBg]               = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
style.Colors[ImGuiCol_FrameBgHovered]        = ImVec4(0.78f, 0.39f, 0.00f, 0.99f);
style.Colors[ImGuiCol_FrameBgActive]         = ImVec4(0.90f, 0.65f, 0.65f, 0.98f);
style.Colors[ImGuiCol_TitleBg]               = ImVec4(0.57f, 0.31f, 0.00f, 1.00f);
style.Colors[ImGuiCol_TitleBgCollapsed]      = ImVec4(0.40f, 0.40f, 0.80f, 1.00f);
style.Colors[ImGuiCol_TitleBgActive]         = ImVec4(0.74f, 0.44f, 0.00f, 1.00f);
style.Colors[ImGuiCol_MenuBarBg]             = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
style.Colors[ImGuiCol_ScrollbarBg]           = ImVec4(0.16f, 0.16f, 0.16f, 0.99f);
style.Colors[ImGuiCol_ScrollbarGrab]         = ImVec4(0.30f, 0.30f, 0.29f, 1.00f);
style.Colors[ImGuiCol_ScrollbarGrabHovered]  = ImVec4(0.78f, 0.39f, 0.00f, 0.99f);
style.Colors[ImGuiCol_ScrollbarGrabActive]   = ImVec4(1.00f, 0.50f, 0.00f, 0.99f);
style.Colors[ImGuiCol_ComboBg]               = ImVec4(0.20f, 0.20f, 0.20f, 0.99f);
style.Colors[ImGuiCol_CheckMark]             = ImVec4(0.90f, 0.90f, 0.90f, 1.00f);
style.Colors[ImGuiCol_SliderGrab]            = ImVec4(0.39f, 0.39f, 0.39f, 1.00f);
style.Colors[ImGuiCol_SliderGrabActive]      = ImVec4(1.00f, 0.48f, 0.00f, 1.00f);
style.Colors[ImGuiCol_Button]                = ImVec4(0.26f, 0.26f, 0.25f, 1.00f);
style.Colors[ImGuiCol_ButtonHovered]         = ImVec4(0.78f, 0.39f, 0.00f, 0.98f);
style.Colors[ImGuiCol_ButtonActive]          = ImVec4(1.00f, 0.48f, 0.00f, 1.00f);
style.Colors[ImGuiCol_Header]                = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
style.Colors[ImGuiCol_HeaderHovered]         = ImVec4(0.57f, 0.30f, 0.00f, 1.00f);
style.Colors[ImGuiCol_HeaderActive]          = ImVec4(0.78f, 0.39f, 0.00f, 1.00f);
style.Colors[ImGuiCol_Column]                = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
style.Colors[ImGuiCol_ColumnHovered]         = ImVec4(0.70f, 0.60f, 0.60f, 1.00f);
style.Colors[ImGuiCol_ColumnActive]          = ImVec4(0.90f, 0.70f, 0.70f, 1.00f);
style.Colors[ImGuiCol_ResizeGrip]            = ImVec4(0.22f, 0.22f, 0.21f, 1.00f);
style.Colors[ImGuiCol_ResizeGripHovered]     = ImVec4(0.78f, 0.39f, 0.00f, 1.00f);
style.Colors[ImGuiCol_ResizeGripActive]      = ImVec4(1.00f, 0.48f, 0.00f, 1.00f);
style.Colors[ImGuiCol_CloseButton]           = ImVec4(0.55f, 0.27f, 0.09f, 1.00f);
style.Colors[ImGuiCol_CloseButtonHovered]    = ImVec4(0.86f, 0.43f, 0.00f, 1.00f);
style.Colors[ImGuiCol_CloseButtonActive]     = ImVec4(1.00f, 0.48f, 0.00f, 1.00f);
style.Colors[ImGuiCol_PlotLines]             = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
style.Colors[ImGuiCol_PlotLinesHovered]      = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
style.Colors[ImGuiCol_PlotHistogram]         = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
style.Colors[ImGuiCol_PlotHistogramHovered]  = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
style.Colors[ImGuiCol_TextSelectedBg]        = ImVec4(0.00f, 0.00f, 1.00f, 1.00f);
style.Colors[ImGuiCol_ModalWindowDarkening]  = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);


        // Other styles
        style.WindowPadding      = ImVec2(10.f, 10.f);
        style.FrameRounding      = 2.f;
        style.WindowRounding     = 4.f;
        style.WindowTitleAlign   = ImVec2(0.5f, 0.5f);
        style.ItemSpacing        = ImVec2(5.f, 5.f);
        style.GrabRounding       = 3.f;
        style.ChildWindowRounding = 4.f;


    }

    void RoR_DrawConsole()
    {
        static Console console;
        console.Draw();
    }

    void DrawGui()
    {
        if (ImGui::BeginMainMenuBar())
        {
            ImGui::Checkbox("Test", &m_is_test_window_visible);
            ImGui::SameLine();
            ImGui::Checkbox("Styles", &m_is_style_editor_visible);
            ImGui::SameLine();
            ImGui::Checkbox("Console", &m_is_console_visible);
            ImGui::SameLine();
            ImGui::Checkbox("Skeleton", &m_is_skeleton_visible);
            ImGui::SameLine();
            ImGui::Checkbox("Spinner", &m_is_spinner_visible);
            ImGui::SameLine();
            ImGui::Checkbox("MP connect", &m_is_mpconnect_visible);
            ImGui::SameLine();
            bool settings_vis = m_settings_ui.IsVisible();
            ImGui::Checkbox("Settings", &settings_vis);
            m_settings_ui.SetVisible(settings_vis);

            ImGui::EndMainMenuBar();
        }

        if (m_is_mpconnect_visible)
        {
            DrawMpConnecting();
        }

        if (m_is_spinner_visible)
        {
            ImGui::Begin("Spinner test");
            static float spinner_counter = 0.f;
            ImGui::SetCursorPos(ImGui::GetCursorPos() + ImVec2(50.f, 50.f));
            DrawImGuiSpinner(spinner_counter);

            static float counter2 = 0.f;
            ImGui::SetCursorPos(ImGui::GetCursorPos() + ImVec2(31.f, 1.f));
            DrawImGuiSpinner(counter2, ImVec2(25.f, 25.f), 3.f);
            ImGui::End();
        }

        if (m_is_test_window_visible)
        {
            ImGui::ShowTestWindow(&m_is_test_window_visible);
        }

        if (m_is_style_editor_visible)
        {
            ImGui::ShowStyleEditor();
        }

        if (m_is_console_visible)
        {
            this->RoR_DrawConsole();
        }

        if (m_is_skeleton_visible)
        {
            DrawSkeletonView();
        }

        if (m_settings_ui.IsVisible())
        {
            m_settings_ui.Draw();
        }

    }


    void Go()
    {
#ifdef _DEBUG
        static const std::string mPluginsCfg = "plugins_d.cfg";
#else
        static const std::string mPluginsCfg = "plugins.cfg";
#endif
        mRoot = new Ogre::Root(mPluginsCfg);

        // Show the configuration dialog and initialise the system.
        // NOTE: If you have valid file 'ogre.cfg', you can use `root.restoreConfig()` instead.
        if(!mRoot->showConfigDialog())
        {
            // User abort
            delete mRoot;
            return;
        }

        const bool create_window = true; // Let's be descriptive :)
        const char* window_name = "OGRE/ImGui demo app";
        mWindow = mRoot->initialise(create_window, window_name);

        mSceneMgr = mRoot->createSceneManager(Ogre::ST_GENERIC);
        mCamera = mSceneMgr->createCamera("PlayerCam");

        // Create one viewport, entire window
        Ogre::Viewport* vp = mWindow->addViewport(mCamera);
        vp->setBackgroundColour(Ogre::ColourValue(0,0,0));

        // Alter the camera aspect ratio to match the viewport
        mCamera->setAspectRatio(Ogre::Real(vp->getActualWidth()) / Ogre::Real(vp->getActualHeight()));

        Ogre::TextureManager::getSingleton().setDefaultNumMipmaps(5);
        Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups();

        mSceneMgr->setAmbientLight(Ogre::ColourValue(.7f, .7f, .7f));
        mCamera->getViewport()->setBackgroundColour(Ogre::ColourValue(0.f, 0.2f, 0.1f)); // Dark green

        // OIS setup
        OIS::ParamList pl;

        size_t windowHnd = 0;
        std::ostringstream windowHndStr;
        mWindow->getCustomAttribute("WINDOW", &windowHnd);
        windowHndStr << windowHnd;
        pl.insert(std::make_pair(std::string("WINDOW"), windowHndStr.str()));
#if OGRE_PLATFORM == OGRE_PLATFORM_LINUX
        pl.insert(OIS::ParamList::value_type("x11_mouse_hide", "false"));
        pl.insert(OIS::ParamList::value_type("XAutoRepeatOn", "false"));
        pl.insert(OIS::ParamList::value_type("x11_mouse_grab", "false"));
#else
        pl.insert(OIS::ParamList::value_type("w32_mouse", "DISCL_FOREGROUND"));
        pl.insert(OIS::ParamList::value_type("w32_mouse", "DISCL_NONEXCLUSIVE"));
#endif
        mInputManager = OIS::InputManager::createInputSystem(pl);

        mKeyboard = static_cast<OIS::Keyboard*>(mInputManager->createInputObject(OIS::OISKeyboard, true));
        mMouse = static_cast<OIS::Mouse*>(mInputManager->createInputObject(OIS::OISMouse, true));

        // Set initial mouse clipping size
        this->windowResized(mWindow);

        // Register as a Window listener
        Ogre::WindowEventUtilities::addWindowEventListener(mWindow, this);

        // === Create IMGUI ====
        m_imgui.Init(mSceneMgr);

        this->RoR_SetGuiStyle();

        mRoot->addFrameListener(this);

        mMouse->setEventCallback(this);
        mKeyboard->setEventCallback(this);

        std::cout << "=============== gvar testing =================" << std::endl <<std::endl;
        
        RoR::App::io_arcade_controls.SetActive(true);
        RoR::App::io_arcade_controls.SetPending(true);
        std::cout << "arcade:" << RoR::App::io_arcade_controls.GetActive() << std::endl;
        RoR::App::io_arcade_controls.ApplyPending();
   
        RoR::App::gfx_envmap_rate.SetActive(2);
        RoR::App::gfx_envmap_rate.SetPending(4);
        std::cout << "envmap:" << RoR::App::gfx_envmap_rate.GetActive() << std::endl;
        RoR::App::gfx_envmap_rate.ApplyPending();

        RoR::App::sim_gearbox_mode.SetActive(RoR::SimGearboxMode::MANUAL_RANGES);
        RoR::App::sim_gearbox_mode.SetPending(RoR::SimGearboxMode::MANUAL_STICK);
        std::cout << "gearbox:" << (RoR::App::sim_gearbox_mode.GetActiveAsStr()) << std::endl;
        RoR::App::sim_gearbox_mode.ApplyPending();

        RoR::App::gfx_flares_mode.SetActive(RoR::GfxFlaresMode::NO_LIGHTSOURCES);
        RoR::App::gfx_flares_mode.SetPending(RoR::GfxFlaresMode::ALL_VEHICLES_ALL_LIGHTS);
        std::cout << "flares:" << (RoR::App::gfx_flares_mode.GetActiveAsStr()) << std::endl;
        RoR::App::gfx_flares_mode.ApplyPending();

        mRoot->startRendering();

        this->Shutdown();
    }

private:
    virtual bool frameRenderingQueued(const Ogre::FrameEvent& evt) override
    {
        m_imgui.Render();

        return (!mWindow->isClosed() && (!mShutDown)); // False means "exit the application"
    }

    virtual bool frameStarted(const Ogre::FrameEvent& evt) override
    {
        // Need to capture/update each device
        mKeyboard->capture();
        mMouse->capture();
        bool ctrl  = mKeyboard->isKeyDown(OIS::KC_LCONTROL);
        bool shift = mKeyboard->isKeyDown(OIS::KC_LSHIFT);
        bool alt   = mKeyboard->isKeyDown(OIS::KC_LMENU);

        // ===== Start IMGUI frame =====
        Ogre::Viewport* vp = mWindow->getViewport(0);
        m_imgui.NewFrame(evt.timeSinceLastFrame, (float)vp->getActualWidth(), (float)vp->getActualHeight(), ctrl, alt, shift);

        // ===== Draw IMGUI  ====
        this->DrawGui();

        return true;
    }

    bool keyPressed( const OIS::KeyEvent &arg ) override
    {
        if (arg.key == OIS::KC_ESCAPE) // All extras we need
        {
            mShutDown = true;
        }

        m_imgui.InjectKeyPressed(arg);
        return true;
    }

    bool keyReleased(const OIS::KeyEvent &arg) override
    {
        m_imgui.InjectKeyReleased(arg);
        return true;
    }

    bool mouseMoved(const OIS::MouseEvent &arg) override
    {
        m_imgui.InjectMouseMoved(arg);
        return true;
    }

    bool mousePressed(const OIS::MouseEvent &arg, OIS::MouseButtonID id) override
    {
        m_imgui.InjectMousePressed(arg, id);
        return true;
    }

    bool mouseReleased(const OIS::MouseEvent &arg, OIS::MouseButtonID id) override
    {
        m_imgui.InjectMouseReleased(arg, id);
        return true;
    }

    // Adjust mouse clipping area
    virtual void windowResized(Ogre::RenderWindow* rw) override
    {
        unsigned int width, height, depth;
        int left, top;
        rw->getMetrics(width, height, depth, left, top);

        const OIS::MouseState &ms = mMouse->getMouseState();
        ms.width = width;
        ms.height = height;
    }

    virtual void windowClosed(Ogre::RenderWindow* rw) override
    {
        this->Shutdown(); // Unattach OIS before window shutdown (very important under Linux)
        mShutDown = true; // Stop application
    }

    void Shutdown()
    {
        if(mInputManager)
        {
            mInputManager->destroyInputObject(mMouse);
            mInputManager->destroyInputObject(mKeyboard);

            OIS::InputManager::destroyInputSystem(mInputManager);
            mInputManager = 0;
        }
        // Remove ourself as a Window listener
        Ogre::WindowEventUtilities::removeWindowEventListener(mWindow, this);
        delete mRoot;
        mRoot = nullptr;
    }

    Ogre::Root*                 mRoot    ;
    Ogre::Camera*               mCamera  ;
    Ogre::SceneManager*         mSceneMgr;
    Ogre::RenderWindow*         mWindow  ;

    bool                        mShutDown;

    //OIS Input devices
    OIS::InputManager*          mInputManager;
    OIS::Mouse*                 mMouse       ;
    OIS::Keyboard*              mKeyboard    ;

    bool                        m_is_test_window_visible;
    bool                        m_is_style_editor_visible;
    bool                        m_is_console_visible;
    bool                        m_is_skeleton_visible;
    bool                        m_is_spinner_visible;
    bool                        m_is_mpconnect_visible;
    OgreImGui                   m_imgui;
    RoR::GameSettings           m_settings_ui;
};


int main(int argc, char *argv[])
{
    try
    {


        DemoApp demo_app;
        demo_app.Go();
    }
    catch(Ogre::Exception& e)
    {
        std::cerr << "An exception has occurred: " << e.getFullDescription().c_str() << std::endl;
    }

    return 0;
}

