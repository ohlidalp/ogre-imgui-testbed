
#include "stdafx.h" // Precompiled

#include <OgreCamera.h>
#include <OgreEntity.h>
#include <OgreLogManager.h>
#include <OgreRoot.h>
#include <OgreViewport.h>
#include <OgreSceneManager.h>
#include <OgreRenderWindow.h>
#include <OgreConfigFile.h>

#include <OISEvents.h>
#include <OISInputManager.h>
#include <OISKeyboard.h>
#include <OISMouse.h>

#include "ImguiManager.h"

#include <string>

#ifdef _DEBUG
    static const std::string mPluginsCfg = "plugins_d.cfg";
#else
    static const std::string mPluginsCfg = "plugins.cfg";
#endif

class GVarBase
{
public:
    GVarBase(const char* name, const char* conf_name): name(name), conf_name(conf_name) {}

    const char* name;
    const char* conf_name;
};

template <typename T>
class GVar: public GVarBase
{
public:
    GVar(const char* name, const char* conf): GVarBase(name, conf) {}

    inline T& GetActiveValue() const { return m_value_active; }
    inline T& GetPendingValue() const { return m_value_pending; }

    void     SetPendingValue(T& const val) { m_value_pending = val; }
    void     ApplyPendingValue();

private:
    T           m_value_active;
    T           m_value_pending;
};

GVar<int>         GVAR_INT("int", "Integer");
GVar<int>         GVAR_INT2("int2", "Integer2");
GVar<std::string> GVAR_STR("str", "String");

GVarBase* GVARS[] = { &GVAR_INT, &GVAR_INT2, &GVAR_STR };

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

struct GuiState
{
    bool test_window_visible;
    bool style_editor_visible;
    bool console_visible;
};

// RoR prototype
class GuiMainMenu
{
public:
    // This class implements hand-made keyboard focus - button count must be known for wrapping
    const size_t NUM_BUTTONS = 4; // Buttons: SinglePlayer, MultiPlayer, Settings, Exit
    const float  WINDOW_WIDTH = 200.f;

    GuiMainMenu(): m_is_visible(false), m_kb_focus_index(-1) {}

    inline bool* GetVisibleFlag()  { return &m_is_visible; }
    inline bool  IsVisible() const { return m_is_visible; }

    // Keyboard updates - move up/down and wrap on top/bottom. Initial index is '-1' which means "no focus"
    void KeyUpPressed()   { m_kb_focus_index = (m_kb_focus_index <= 0) ? (NUM_BUTTONS-1) : (m_kb_focus_index - 1); }
    void KeyDownPressed() { m_kb_focus_index = (m_kb_focus_index < (NUM_BUTTONS - 1)) ? (m_kb_focus_index + 1) : 0; }

    void Draw()
    {
        int flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize;
        ImGui::SetNextWindowPosCenter();
        
        ImGui::SetNextWindowContentWidth(WINDOW_WIDTH);
        if (!ImGui::Begin("Main menu", nullptr, static_cast<ImGuiWindowFlags_>(flags)))
        {
            return;
        }

        ImVec2 btn_size(WINDOW_WIDTH - ImGui::GetStyle().WindowPadding.x, 0.f); // Weird but necessary

        const char* sp_title = (m_kb_focus_index == 0) ? "--> Single player <--" : "Single player";
        ImGui::Button(sp_title, btn_size);
            
        const char* mp_title = (m_kb_focus_index == 1) ? "--> Multi player <--" : "Multi player";
        ImGui::Button(mp_title , btn_size);

        ImGui::Separator();

        const char* settings_title = (m_kb_focus_index == 2) ? "--> Settings <--" : "Settings";
        ImGui::Button(settings_title, btn_size);

        const char* exit_title = (m_kb_focus_index == 3) ? "--> Exit game <--" : "Exit game";
        ImGui::Button(exit_title, btn_size);

        ImGui::End();
    }
private:
    bool   m_is_visible;
    int    m_kb_focus_index; // -1 = no focus; 0+ = button index
};

class DemoApp: public Ogre::FrameListener, public OIS::KeyListener, public OIS::MouseListener,  public Ogre::WindowEventListener
{
public:
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
            ImGui::Checkbox("Test", &m_gui_state.test_window_visible);
            ImGui::SameLine();
            ImGui::Checkbox("Styles", &m_gui_state.style_editor_visible);
            ImGui::SameLine();
            ImGui::Checkbox("MMenu", m_main_menu.GetVisibleFlag());
            ImGui::SameLine();
            ImGui::Checkbox("Console", &m_gui_state.console_visible);

            ImGui::EndMainMenuBar();
        }

        if (m_gui_state.test_window_visible)
        {
            ImGui::ShowTestWindow(&m_gui_state.test_window_visible);
        }

        if (m_gui_state.style_editor_visible)
        {
            ImGui::ShowStyleEditor();
        }

        if (m_main_menu.IsVisible())
        {
            m_main_menu.Draw();
        }

        if (m_gui_state.console_visible)
        {
            this->RoR_DrawConsole();
        }
    }


    void Go()
    {
        memset(&m_gui_state, 0, sizeof(GuiState));

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
        m_imgui.init(mSceneMgr, mKeyboard, mMouse); // OIS mouse + keyboard

        this->RoR_SetGuiStyle();

        mRoot->addFrameListener(this);

        mMouse->setEventCallback(this);
        mKeyboard->setEventCallback(this);

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
        m_imgui.newFrame(evt.timeSinceLastFrame, Ogre::Rect(left, top, width, height));

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
        else if (arg.key == OIS::KC_UP)
        {
            m_main_menu.KeyUpPressed();
        }
        else if (arg.key == OIS::KC_DOWN)
        {
            m_main_menu.KeyDownPressed();
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

    GuiState                    m_gui_state;
    GuiMainMenu                 m_main_menu;
    Ogre::ImguiManager          m_imgui;
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

