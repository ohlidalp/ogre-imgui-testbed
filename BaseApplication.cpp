
#include "stdafx.h" // Precompiled

#include "ImguiManager.h"
#include "Application.h" // Debugging - copy of RoR

#include <memory>    // std::unique_ptr
#include <algorithm> // std::min()

#define ROR_ARRAYLEN(_BUF)  (sizeof(_BUF)/sizeof(*_BUF))

// ================================== MP selector prototype ================================================
namespace RoR
{
struct MpServerData
{
    MpServerData(const char* name, const char* terrn, size_t users, size_t cap, const char* ip, size_t port):
        num_users(users), max_users(cap), net_port(port)
    {
        strncpy(server_name,  name,  ROR_ARRAYLEN(server_name ));
        strncpy(terrain_name, terrn, ROR_ARRAYLEN(terrain_name));
        strncpy(ip_addr,      ip,    ROR_ARRAYLEN(ip_addr     ));
        
        snprintf(display_users, ROR_ARRAYLEN(display_users), "%u/%u", num_users, max_users);
        snprintf(display_addr,  ROR_ARRAYLEN(display_addr ), "%s:%d", ip_addr, net_port);
    }

    char        server_name[100];
    char        terrain_name[100];
    size_t      num_users;
    size_t      max_users;
    char        display_users[20];
    char        ip_addr[100];
    size_t      net_port;
    char        display_addr[50];
};

struct ServerListData;

class MultiplayerSelector
{
public:
    MultiplayerSelector();
    void Draw();

private:
    enum class Mode { ONLINE, DIRECT, SETUP };

    std::unique_ptr<ServerListData> m_data;
    int                             m_selected_item;
    Mode                            m_mode;
    bool                            m_is_refreshing;
    char                            m_window_title[50];
};

struct ServerListData
{
    std::vector<MpServerData> servers;
};


const char* const ROR_VERSION_STRING = "0.5";
#define RORNET_VERSION              "RoRnet_2.40"

inline void DrawTableHeader(const char* title)
{
    float table_padding_y = 4.f;
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + table_padding_y);
    ImGui::Text(title);
    ImGui::NextColumn();
}

MultiplayerSelector::MultiplayerSelector():
    m_selected_item(-1), m_mode(Mode::ONLINE), m_is_refreshing(false)
{
    snprintf(m_window_title, 50, "Multiplayer (Rigs of Rods %s | %s)", ROR_VERSION_STRING, RORNET_VERSION); 
    // test dummies
    m_data = std::make_unique<ServerListData>();
    m_data->servers.emplace_back("server A", "A.terrn", 5, 15, "1.1.1.1", 1111);
    m_data->servers.emplace_back("server B", "B.terrn", 4, 14, "2.2.2.2", 2222);
    m_data->servers.emplace_back("server C", "C.terrn", 3, 13, "3.3.3.3", 3333);
}


void MultiplayerSelector::Draw()
{
    int window_flags = ImGuiWindowFlags_NoCollapse;
    if (!ImGui::Begin(m_window_title, nullptr, window_flags))
    {
        return;
    }

    // Window mode buttons
    RoR::MultiplayerSelector::Mode next_mode = m_mode;

    if (ImGui::Button("Online (refresh)"))
    {
        next_mode = Mode::ONLINE;
        m_is_refreshing = !m_is_refreshing; // DEBUG
        // TODO: refresh
    }
    ImGui::SameLine();
    if (ImGui::Button("Direct IP"))
    {
        next_mode = Mode::DIRECT;
    }
    ImGui::SameLine();
    if (ImGui::Button("Setup"))
    {
        next_mode = Mode::SETUP;
    }

    ImGui::Separator();

    if (next_mode != m_mode) // Handle switching window modes
    {
        if (m_mode == Mode::SETUP) // If leaving SETUP mode, reset 'pending' values of GVars
        {
            App::mp_player_name    .SetPending(App::mp_player_name.GetActive()); // TODO: implement 'ResetPending()' ?
            App::mp_server_password.SetPending(App::mp_server_password.GetActive());
        }
        if (m_mode == Mode::DIRECT) // If leaving DIRECT mode, reset 'pending' values of GVars
        {
            App::mp_server_password.SetPending(App::mp_server_password.GetActive()); // TODO: implement 'ResetPending()' ?
            App::mp_server_host    .SetPending(App::mp_server_host.GetActive());
            App::mp_server_port    .SetPending(App::mp_server_port.GetActive());
        }
    }
    m_mode = next_mode;

    if (m_mode == Mode::SETUP)
    {
        ImGui::PushID("setup");

        ImGui::PushItemWidth(250.f);
        ImGui::InputText("Player nickname",         App::mp_player_name.GetPending().buffer,        App::mp_player_name.GetPending().buf_len);
        ImGui::InputText("Default server password", App::mp_server_password.GetPending().buffer,    App::mp_server_password.GetPending().buf_len);
        ImGui::PopItemWidth();

        if (ImGui::Button("Save"))
        {
            App::mp_player_name.ApplyPending();
            App::mp_server_password.ApplyPending();
        }

        ImGui::PopID();
    }
    else if (m_mode == Mode::DIRECT)
    {
        ImGui::PushID("direct");

        ImGui::PushItemWidth(250.f);
        ImGui::InputText("Server host", App::mp_server_host.GetPending().buffer, App::mp_server_host.GetPending().buf_len);
        int port = App::mp_server_port.GetPending();
        if (ImGui::InputInt("Server port", &port))
        {
            App::mp_server_port.SetPending(port);
        }
        ImGui::InputText("Server password (default)", App::mp_server_password.GetPending().buffer, App::mp_server_password.GetPending().buf_len);
        ImGui::PopItemWidth();

        if (ImGui::Button("Save & join"))
        {
            App::mp_server_host.ApplyPending();
            App::mp_server_port.ApplyPending();
            App::mp_server_password.ApplyPending();

            // TODO: perform the join.
        }

        if (ImGui::Button("Save only"))
        {
            App::mp_server_host.ApplyPending();
            App::mp_server_port.ApplyPending();
            App::mp_server_password.ApplyPending();
        }

        ImGui::PopID();
    }
    if (m_mode == Mode::ONLINE && m_is_refreshing)
    {
        const char* refresh_lbl = "... refreshing ...";
        const ImVec2 refresh_size = ImGui::CalcTextSize(refresh_lbl);
        ImGui::SetCursorPosX((ImGui::GetWindowSize().x / 2.f) - (refresh_size.x / 2.f));
        ImGui::SetCursorPosY((ImGui::GetWindowSize().y / 2.f) - (refresh_size.y / 2.f));
        ImGui::Text(refresh_lbl);
    }
    else if (m_mode == Mode::ONLINE && !m_is_refreshing)
    {
        // Setup serverlist table ... the scroll area
        float table_height = ImGui::GetWindowHeight()
            - ((2.f * ImGui::GetStyle().WindowPadding.y) + (3.f * ImGui::GetItemsLineHeightWithSpacing()) - ImGui::GetStyle().ItemSpacing.y);
        ImGui::BeginChild("scrolling", ImVec2(0.f, table_height), true);
        // ... and the table itself
        float width_percent = ImGui::GetWindowContentRegionWidth()/100.f;
        ImGui::Columns(4, "mp-selector-columns");
        ImGui::SetColumnOffset(1, 35.f * width_percent);
        ImGui::SetColumnOffset(2, 75.f * width_percent);
        ImGui::SetColumnOffset(3, 85.f * width_percent);
        // Draw table header
        float table_padding_x = 4.f;
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + table_padding_x);
        DrawTableHeader("Name");
        DrawTableHeader("Terrain");
        DrawTableHeader("Users");
        DrawTableHeader("IP/Port");
        ImGui::Separator();
        // Draw table body
        int num_servers = static_cast<int>(m_data->servers.size());
        for (int i = 0; i < num_servers; i++)
        {
            // First column - selection control
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + table_padding_x);
            MpServerData& server = m_data->servers[i];
            if (ImGui::Selectable(server.server_name, m_selected_item == i, ImGuiSelectableFlags_SpanAllColumns))
            {
                m_selected_item = i;
            }
            ImGui::NextColumn();

            // Other collumns
            ImGui::Text(server.terrain_name);          ImGui::NextColumn();
            ImGui::Text(server.display_users);         ImGui::NextColumn();
            ImGui::Text(server.display_addr);          ImGui::NextColumn();
        }
        ImGui::Columns(1);
        ImGui::EndChild(); // End of scroll area

        // Simple join button
        if (ImGui::Button("Join", ImVec2(200.f, 0.f)))
        {
            std::cout<< "join btn clicked"<<std::endl;
        }

        // Password editbox; right-aligned
        ImGui::SameLine();
        float pw_width = 200.f;
        const char* pw_label = "Password";
        float pw_pos_x = ImGui::GetWindowContentRegionWidth() - (pw_width - ImGui::GetStyle().ItemSpacing.x) - ImGui::CalcTextSize(pw_label).x;
        int input_pw_flags = ImGuiInputTextFlags_Password;
        ImGui::PushItemWidth(pw_width);
        ImGui::SetCursorPosX(pw_pos_x);
  //      if (ImGui::InputText("Password", App::mp_server_password.GetPendingValue().buffer, App::mp_server_password.GetPendingValue().buf_len, input_pw_flags))
  //      {
  //          std::cout << "set password: " << App::mp_server_password.GetPendingValue().buffer <<std::endl; // TEST!!!
  //      }
        ImGui::PopItemWidth();        
    }
    else if (m_mode == Mode::DIRECT)
    {
        float box_width = 300.f;
    }

    ImGui::End();
}

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
        m_is_test_window_visible(false),
        m_is_style_editor_visible(false),
        m_is_multiplayer_visible(false),
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
            ImGui::Checkbox("Multiplayer", &m_is_multiplayer_visible);
            ImGui::SameLine();
            ImGui::Checkbox("Console", &m_is_console_visible);
            ImGui::SameLine();
            ImGui::Checkbox("Skeleton", &m_is_skeleton_visible);

            ImGui::EndMainMenuBar();
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

        if (m_is_multiplayer_visible)
        {
            m_multiplayer.Draw();
        }

        if (m_is_skeleton_visible)
        {
            DrawSkeletonView();
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
        m_imgui.Init(mSceneMgr, mKeyboard, mMouse); // OIS mouse + keyboard

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
        return (!mWindow->isClosed() && (!mShutDown)); // False means "exit the application"
    }

    virtual bool frameStarted(const Ogre::FrameEvent& evt) override
    {
        // Need to capture/update each device
        mKeyboard->capture();
        mMouse->capture();

        // ===== Start IMGUI frame =====
        int left, top, width, height;
        mWindow->getViewport(0)->getActualDimensions(left, top, width, height); // output params
        m_imgui.NewFrame(evt.timeSinceLastFrame, Ogre::Rect(left, top, width, height));

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

        m_imgui.keyPressed(arg);
        return true;
    }

    bool keyReleased(const OIS::KeyEvent &arg) override
    {
        m_imgui.keyReleased(arg);
        return true;
    }

    bool mouseMoved(const OIS::MouseEvent &arg) override
    {
        m_imgui.mouseMoved(arg);
        return true;
    }

    bool mousePressed(const OIS::MouseEvent &arg, OIS::MouseButtonID id) override
    {
        m_imgui.mousePressed(arg, id);
        return true;
    }

    bool mouseReleased(const OIS::MouseEvent &arg, OIS::MouseButtonID id) override
    {
        m_imgui.mouseReleased(arg, id);
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
    bool                        m_is_multiplayer_visible;
    bool                        m_is_console_visible;
    bool                        m_is_skeleton_visible;
    RoR::MultiplayerSelector    m_multiplayer;
    OgreImGui                   m_imgui;
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

