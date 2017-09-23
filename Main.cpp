
#include "stdafx.h" // Precompiled

#include "ImguiManager.h"
#include "Application.h" // Debugging - copy of RoR

#include "RigEditor_Gui.h"
#include "RigEditor_Project.h"

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
        this->DrawSelectionTestPanel();
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
        m_imgui.Init(mSceneMgr); // OIS mouse + keyboard

        this->RoR_SetGuiStyle();

        mRoot->addFrameListener(this);

        mMouse->setEventCallback(this);
        mKeyboard->setEventCallback(this);

        // ============= create test RigEditor project ==================== //

        // Some node presets
        auto* np1 = new RigEditor::SoftbodyNode::Preset();
        np1->name = "Node preset 1";
        np1->friction = 11.11f;
        np1->volume = 10.10f;
        np1->options.option_m_no_mouse_grab = true;
        np1->options.option_y_exhaust_direction = true;

        auto* np2 = new RigEditor::SoftbodyNode::Preset();
        np2->name = "NP2 (node preset with stupidly long name)";
        np2->options.option_h_hook_point = true;
        np2->friction = 22.22f;
        np2->volume = 20.20f;

        auto* np3 = new RigEditor::SoftbodyNode::Preset();
        np3->name = "NODE PRESET 3";
        np3->options.option_x_exhaust_point = true;
        np3->friction = 33.33f;
        np3->volume = 30.30f;

        auto* np4 = new RigEditor::SoftbodyNode::Preset();
        np4->name = "node_preset_4";
        np4->options.option_l_load_weight = true;
        np4->friction = 44.44f;
        np4->volume = 40.40f;
        np4->load_weight = 400.f;

        m_editor_project.softbody.node_presets.push_back(np1);
        m_editor_project.softbody.node_presets.push_back(np2);
        m_editor_project.softbody.node_presets.push_back(np3);
        m_editor_project.softbody.node_presets.push_back(np4);

        // Add some nodes

        RigEditor::SoftbodyNode* n1 = new RigEditor::SoftbodyNode();
        n1->name = "node1";
        n1->node_preset = np1;
        m_editor_project.softbody.nodes.push_back(n1);
        
        RigEditor::SoftbodyNode* n2 = new RigEditor::SoftbodyNode();
        n2->name = "node2";
        n2->node_preset = np2;
        m_editor_project.softbody.nodes.push_back(n2);

        // Add some beam presets

using namespace RigEditor;
        SoftbodyBeam::Preset* bp1 = new SoftbodyBeam::Preset();
        bp1->name = "Beam preset 1";
        m_editor_project.softbody.beam_presets.push_back(bp1);

        SoftbodyBeam::Preset* bp2 = new SoftbodyBeam::Preset();
        bp2->name = "BP2 (beam preset or something)";
        m_editor_project.softbody.beam_presets.push_back(bp2);

        //  Add beams of various types
        // note: node connection don't matter here, the test selection window selects beams directly
        RigEditor::SoftbodyBeam* b = nullptr;

#define MKBEAM(Type__) \
        b= new RigEditor::SoftbodyBeam(); \
        b->type = Type__; \
        b->base_node = n1; \
        b->tip_node = n2; \
        m_editor_project.softbody.beams.push_back(b);

        MKBEAM(RigEditor::SoftbodyBeam::Type::PLAIN);
        m_editor_project.softbody.beams.back()->beam_preset = bp1;
        MKBEAM(RigEditor::SoftbodyBeam::Type::STEERING_HYDRO);  
        MKBEAM(RigEditor::SoftbodyBeam::Type::COMMAND_HYDRO);   
        m_editor_project.softbody.beams.back()->beam_preset = bp2;
        MKBEAM(RigEditor::SoftbodyBeam::Type::SHOCK_ABSORBER);  
        MKBEAM(RigEditor::SoftbodyBeam::Type::SHOCK_ABSORBER_2);
        m_editor_project.softbody.beams.back()->beam_preset = bp2;
        MKBEAM(RigEditor::SoftbodyBeam::Type::ROPE);            
        MKBEAM(RigEditor::SoftbodyBeam::Type::TRIGGER);         
        m_editor_project.softbody.beams.back()->beam_preset = bp1;
        MKBEAM(RigEditor::SoftbodyBeam::Type::GENERATED);        

        m_editor_gui.SetProject(&m_editor_project);

        mRoot->startRendering();

        this->Shutdown();
    }

    void DrawSelectionTestPanel()
    {
        using namespace RigEditor;
        ImGui::Begin("Selection test", nullptr);

        ImGui::Columns(2, "selection test");

        // column1 = nodes
        ImGui::PushID("nodes");
        for (SoftbodyNode* n: m_editor_project.softbody.nodes)
        {
            if (ImGui::Checkbox(n->name, &n->state_is_selected))
            {
                m_editor_project.RefreshNodeSelectionAggregates();
            }
        }
        ImGui::PopID();

        // Column2 = beams
        ImGui::NextColumn();
        ImGui::PushID("beams");
        int index = 0;
        for (SoftbodyBeam* b: m_editor_project.softbody.beams)
        {
            ImGui::PushID(index);
            if (ImGui::Checkbox(b->GetTypeAsStr(), &b->state_is_selected))
            {
                m_editor_project.RefreshBeamSelectionAggregates();
            }
            ImGui::PopID();
            ++index;
        }
        ImGui::PopID();

        ImGui::End();
    }

private:
    virtual bool frameRenderingQueued(const Ogre::FrameEvent& evt) override
    {
        if (mWindow->isClosed() || (mShutDown)) // Returning false means "exit the application"
        {
            return false;
        }
        m_imgui.Render();
        return true;
    }

    virtual bool frameStarted(const Ogre::FrameEvent& evt) override
    {
        // Need to capture/update each device
        mKeyboard->capture();
        mMouse->capture();

        // ===== Start IMGUI frame =====
        int left, top, width, height;
        mWindow->getViewport(0)->getActualDimensions(left, top, width, height); // output params
        bool ctrl  = mKeyboard->isKeyDown(OIS::KC_LCONTROL);
        bool shift = mKeyboard->isKeyDown(OIS::KC_LSHIFT);
        bool alt   = mKeyboard->isKeyDown(OIS::KC_LMENU);
        m_imgui.NewFrame(evt.timeSinceLastFrame, static_cast<float>(width), static_cast<float>(height), ctrl, alt, shift);

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

