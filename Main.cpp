
#include "stdafx.h" // Precompiled

#include "ImguiManager.h"
#include "Application.h" // Debugging - copy of RoR

#include "RigEditor_Gui.h"

#include <memory>    // std::unique_ptr
#include <algorithm> // std::min()

#define ROR_ARRAYLEN(_BUF)  (sizeof(_BUF)/sizeof(*_BUF))

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



    void DrawGui()
    {
        m_editor_gui.Draw();
        ImGui::ShowTestWindow();
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

        // ============= create test RigEditor project ==================== //

        RigEditor::SoftbodyNode::Selection& sel = m_editor_project.softbody.node_selection;
        sel.num_selected = 1;
        sel.name.Clear();
        sel.name << "n123";
        sel.options_values.option_c_no_ground_contact = true;
        sel.options_uniform.option_c_no_ground_contact = true;
        sel.options_values.option_l_load_weight = true;
        sel.options_uniform.option_l_load_weight = true;
        sel.weight_override = 44.44f;
        sel.weight_override_is_uniform = true;

        m_editor_gui.UpdateNodeSelection(sel);

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

    OgreImGui                   m_imgui;
    RigEditor::Gui              m_editor_gui;
    RigEditor::Project          m_editor_project;
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

