
#include "stdafx.h" // Precompiled

#include "ImguiManager.h"
#include "Application.h" // Debugging - copy of RoR

#include <memory>    // std::unique_ptr
#include <algorithm> // std::min()


// -------------------------------------- loading spinner prototype -------------------------------------------

void DrawImGuiSpinner(float& counter, const ImVec2 pos, const ImVec2 size = ImVec2(16.f, 16.f), const float spacing = 2.f, const float step_sec = 0.15f)
{
    // Hardcoded to 4 segments, counter is reset after full round (4 steps)
    // --------------------------------------------------------------------

    const ImU32 COLORS[] = { ImColor(255,255,255,255), ImColor(210,210,210,255), ImColor(120,120,120,255), ImColor(60,60,60,255) };

    // Update counter, determine coloring
    counter += ImGui::GetIO().DeltaTime;
    int color_start = 0; // Index to GUI_SPINNER_COLORS array for the top middle segment (segment 0)
    if (counter > (step_sec*4.f))
    {
        counter -= (step_sec*4.f);
    }
    else if (counter > (step_sec*3.f))
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

            ImGui::EndMainMenuBar();
        }

        if (m_is_spinner_visible)
        {
            ImGui::Begin("Spinner test");
            static float spinner_counter = 0.f;
            float spinner_x = (ImGui::GetWindowPos().x)+50;
            float spinner_y = (ImGui::GetWindowPos().y)+50;
            DrawImGuiSpinner(spinner_counter, ImVec2(spinner_x, spinner_y));

            static float counter2 = 0.f;
            DrawImGuiSpinner(counter2, ImVec2(spinner_x+31.f, spinner_y+1.f), ImVec2(25.f, 25.f), 3.f);
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

