
/*
NOTES
=====

NODE EDITORS FOUND ON THE INTERNET
    * https://github.com/ocornut/imgui/issues/306#issuecomment-134657997 -- Demo by Omar Cornut. Nodelinks not editable :(
    * https://github.com/ocornut/imgui/issues/306#issuecomment-151167133 -- Looks very good. Editable nodelinks. Has online emscripten demo.
    * https://www.youtube.com/watch?v=m6eteEPQ0Lg -- Fully editable, looks interesting.

*/

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

// --------------------- the reference node editor  -------------------------- //
// Quick intro: https://github.com/ocornut/imgui/issues/306#issuecomment-151167133
// Source code: https://github.com/Flix01/imgui/tree/2015-10-Addons/addons/imguinodegrapheditor

#define NO_IMGUIFILESYSTEM
#include "imguinodegrapheditor.h"

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui_internal.h>

// --------------------- END reference node editor  -------------------------- //

class NodeGraphTool
{
    // Node types
    //  READING - reports XYZ position of node in world space
    //            Inputs: none
    //            Outputs(3): X position, Y position, Z position

public:
    NodeGraphTool():
        m_scroll(0.0f, 0.0f)
    {

        // test dummies
        m_nodes.push_back(new ReadingNode());
        m_nodes.push_back(new DisplayNode());
        m_nodes.push_back(new DisplayNode());

        m_nodes[1]->pos += ImVec2(300.f, -10.f);
        m_nodes[1]->pos += ImVec2(250.f, 133.f);

        // Links
        m_nodes[0]->ToReading()->link_x.node = m_nodes[1]; // socket index 0 = default
        m_nodes[0]->ToReading()->link_z.node = m_nodes[2]; // socket index 0 = default
    }
private:
    struct Vec3 { float x, y, z; };

    struct Style { // Copypaste from https://github.com/Flix01/imgui/tree/2015-10-Addons/addons/imguinodegrapheditor
        //ImVec4 color_background;
        ImU32 color_grid;
        float grid_line_width;
        float grid_size;
        //ImU32 color_node;
        //ImU32 color_node_frame;
        //ImU32 color_node_selected;
        //ImU32 color_node_active;
        //ImU32 color_node_frame_selected;
        //ImU32 color_node_frame_active;
        //ImU32 color_node_hovered;
        //ImU32 color_node_frame_hovered;
        //float node_rounding;
        ImVec2 node_window_padding;
        //ImU32 color_node_input_slots;
        //ImU32 color_node_input_slots_border;
        //ImU32 color_node_output_slots;
        //ImU32 color_node_output_slots_border;
        float node_slots_radius;
        //int node_slots_num_segments;
        ImU32 color_link;
        float link_line_width;
        //float link_control_point_distance;
        //int link_num_segments;  // in AddBezierCurve(...)
        //ImVec4 color_node_title;
        //ImU32 color_node_title_background;
        //float color_node_title_background_gradient;
        //ImVec4 color_node_input_slots_names;
        //ImVec4 color_node_output_slots_names;        
        //ImU32 color_mouse_rectangular_selection;
        //ImU32 color_mouse_rectangular_selection_frame;
        Style() {
            //color_background =          ImColor(60,60,70,200);
            color_grid =                ImColor(200,200,200,40);
            grid_line_width =           1.f;
            grid_size =                 64.f;
            //
            //color_node =                ImColor(60,60,60);
            //color_node_frame =          ImColor(100,100,100);
            //color_node_selected =       ImColor(75,75,85);
            //color_node_active =         ImColor(85,85,65);
            //color_node_frame_selected = ImColor(115,115,115);
            //color_node_frame_active =   ImColor(125,125,105);
            //color_node_hovered =        ImColor(85,85,85);
            //color_node_frame_hovered =  ImColor(125,125,125);
            //node_rounding =             4.f;
            node_window_padding =       ImVec2(8.f,8.f);
            //
            //color_node_input_slots =    ImColor(150,150,150,150);
            //color_node_output_slots =   ImColor(150,150,150,150);
            node_slots_radius =         5.f;
            //
            color_link =                ImColor(200,200,100);
            link_line_width =           3.f;
            //link_control_point_distance = 50.f;
            //link_num_segments =         0;
            //
            //color_node_title = ImGui::GetStyle().Colors[ImGuiCol_Text];
            //color_node_title_background = 0;//ImGui::ColorConvertFloat4ToU32(ImGui::GetStyle().Colors[ImGuiCol_TitleBgActive]);
            //color_node_title_background_gradient = 0.f;   // in [0,0.5f] used only if available (performance is better when 0)
            //color_node_input_slots_names = ImGui::GetStyle().Colors[ImGuiCol_Text];color_node_input_slots_names.w=0.75f;
            //color_node_output_slots_names = ImGui::GetStyle().Colors[ImGuiCol_Text];color_node_output_slots_names.w=0.75f;
            //
            //color_mouse_rectangular_selection =         ImColor(255,0,0,45);
            //color_mouse_rectangular_selection_frame =   ImColor(45,0,0,175);
            //
            //color_node_input_slots_border = color_node_output_slots_border = ImColor(60,60,60,0);
            //node_slots_num_segments = 12;
        }
    }; // struct Style

    struct Node; // Forward
    struct ReadingNode;
    struct DisplayNode;

    struct Link
    {
        Link(): node(nullptr), input_index(0) {}

        Node* node;
        size_t input_index; // only applies when connected
    };

    struct Node
    {
        enum class Type { INVALID, READING, DISPLAY };

        Node(): num_inputs(0), num_outputs(-1), type(Type::INVALID), pos(100.f, 100.f), size(150.f, 100.f) {}

        ReadingNode* ToReading() { assert(type == Type::READING); if (type == Type::READING) { return static_cast<ReadingNode*>(this); } else { return nullptr; } }
        DisplayNode* ToDisplay() { assert(type == Type::DISPLAY); if (type == Type::DISPLAY) { return static_cast<DisplayNode*>(this); } else { return nullptr; } }

        size_t num_inputs;
        size_t num_outputs;
        Type type;
        ImVec2 pos;
        ImVec2 size;

        inline ImVec2 GetInputSlotPos(size_t slot_idx)  { return ImVec2(pos.x,          pos.y + (size.y * (static_cast<float>(slot_idx+1) / static_cast<float>(num_inputs+1)))); }
        inline ImVec2 GetOutputSlotPos(size_t slot_idx) { return ImVec2(pos.x + size.x, pos.y + (size.y * (static_cast<float>(slot_idx+1) / static_cast<float>(num_outputs+1)))); }
    };

    struct ReadingNode: public Node
    {
        ReadingNode() { num_inputs = 0; num_outputs = 3; type = Type::READING; }

        int node_id; // -1 means 'none'
        Vec3 buffer[2000]; // 1 second worth of data
        Link link_x;
        Link link_y;
        Link link_z;
    };

    struct DisplayNode: public Node
    {
        DisplayNode() { num_inputs = 1; num_outputs = 1; type = Type::DISPLAY; }

        Link link_out;
    };

public:
    void Draw()
    {
        // Create a window
        if (!ImGui::Begin("ELSACO NodeGraph"))
            return; // No window -> nothing to do.

        ImGui::Text("(dummy) bla bla");
        ImGui::Text("foo bar baz");

        this->DrawNodeGraphPane();

        // Finalize the window
        ImGui::End();
    }
private:

    inline bool IsInside(ImRect& rect, ImVec2& point) { return ((point.x > rect.Min.x) && (point.y > rect.Min.y)) && ((point.x < rect.Max.x) && (point.y < rect.Max.y)); }

    void DrawLink(Node* src_node, Link& link, size_t src_index)
    {
        // TODO: Channels
        if (link.node == nullptr)
            return; // Not connected

        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        ImVec2 offset = ImGui::GetCursorScreenPos() - m_scroll;
        ImVec2 p1 = offset + src_node->GetOutputSlotPos(src_index);
        ImVec2 p2 = offset + link.node->GetInputSlotPos(link.input_index);
        ImRect window = ImGui::GetCurrentWindow()->Rect();

        if (this->IsInside(window, p1) || this->IsInside(window, p2)) // very basic clipping
        {
            draw_list->AddBezierCurve(p1, p1+ImVec2(+50,0), p2+ImVec2(-50,0), p2, m_style.color_link, m_style.link_line_width);
        }
    }

    void DrawGrid()
    {
        // TODO: channels!
        const ImVec2 win_pos = ImGui::GetCursorScreenPos();
        const ImVec2 offset = ImGui::GetCursorScreenPos() - m_scroll;
        const ImVec2 canvasSize = ImGui::GetWindowSize();
        ImDrawList* draw_list = ImGui::GetWindowDrawList();

        for (float x = fmodf(offset.x,m_style.grid_size); x < canvasSize.x; x += m_style.grid_size)
            draw_list->AddLine(ImVec2(x,0.0f)+win_pos, ImVec2(x,canvasSize.y)+win_pos, m_style.color_grid, m_style.grid_line_width);
        for (float y = fmodf(offset.y,m_style.grid_size); y < canvasSize.y; y += m_style.grid_size)
            draw_list->AddLine(ImVec2(0.0f,y)+win_pos, ImVec2(canvasSize.x,y)+win_pos, m_style.color_grid, m_style.grid_line_width);
    }

    void DrawNodeGraphPane()
    {
        const bool draw_border = false;
        const int flags = ImGuiWindowFlags_NoScrollbar|ImGuiWindowFlags_NoMove|ImGuiWindowFlags_NoScrollWithMouse;
        if (!ImGui::BeginChild("scroll-region", ImVec2(0,0), draw_border, flags))
            return; // Nothing more to do.

        const float baseNodeWidth = 120.f; // same as reference, but hardcoded
        float currentNodeWidth = baseNodeWidth;
        ImGui::PushItemWidth(currentNodeWidth);

        this->DrawGrid();

        for (Node* node: m_nodes)
        {
            switch (node->type)
            {
            case Node::Type::READING:
                this->DrawLink(node, node->ToReading()->link_x, 0);
                this->DrawLink(node, node->ToReading()->link_y, 1);
                this->DrawLink(node, node->ToReading()->link_z, 2);
                break;
            case Node::Type::DISPLAY:
                // No outputs to draw here!
                break;
            default:;
            }
        }

        // Scrolling - clone of Ocornut
        if (ImGui::IsWindowHovered() && !ImGui::IsAnyItemActive() && ImGui::IsMouseDragging(2, 0.0f))
        {
            m_scroll = m_scroll - ImGui::GetIO().MouseDelta;
        }


        ImGui::EndChild();
    }

    std::vector<Node*> m_nodes;
    Style m_style;
    ImVec2 m_scroll;
};

// #################################################### END CUSTOM NODE EDITOR ############################################################# //

#ifdef _DEBUG
    static const std::string mPluginsCfg = "plugins_d.cfg";
#else
    static const std::string mPluginsCfg = "plugins.cfg";
#endif

struct GuiState
{
    bool test_window_visible;
    bool nodes_window_visible;
};

class DemoApp: public Ogre::FrameListener, public OIS::KeyListener, public OIS::MouseListener,  public Ogre::WindowEventListener
{
public:
    void DrawGui()
    {
        static bool show_nodegraph = false;

        if (ImGui::BeginMainMenuBar())
        {


            ImVec2 pos = ImGui::GetCursorPos();
            ImGui::Checkbox("Test", &m_gui_state.test_window_visible);
            ImGui::SameLine();
            ImGui::Checkbox("Nodegraph ref", &m_gui_state.nodes_window_visible);
            ImGui::SameLine();
            ImGui::Checkbox("Nodegraph WIP", &show_nodegraph);

            ImGui::EndMainMenuBar();
        }

        if (m_gui_state.test_window_visible)
        {
            ImGui::ShowTestWindow(&m_gui_state.test_window_visible);
        }

        if (m_gui_state.nodes_window_visible)
        {
            ImGui::SetNextWindowSize(ImVec2(700,600), ImGuiSetCond_FirstUseEver);
            if (ImGui::Begin("Example: Custom Node Graph", NULL))
            {
                ImGui::TestNodeGraphEditor();   // see its code for further info

            }
        }

        if (show_nodegraph)
            m_nodegraph.Draw();
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
        Ogre::ImguiManager::createSingleton();
        Ogre::ImguiManager::getSingleton().init(mSceneMgr, mKeyboard, mMouse); // OIS mouse + keyboard

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
        Ogre::ImguiManager::getSingleton().newFrame(evt.timeSinceLastFrame, Ogre::Rect(left, top, width, height));

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

        Ogre::ImguiManager::getSingleton().keyPressed(arg);
        return true;
    }

    bool keyReleased(const OIS::KeyEvent &arg) override
    {
        Ogre::ImguiManager::getSingleton().keyReleased(arg);
        return true;
    }

    bool mouseMoved(const OIS::MouseEvent &arg) override
    {
        Ogre::ImguiManager::getSingleton().mouseMoved(arg);
        return true;
    }

    bool mousePressed(const OIS::MouseEvent &arg, OIS::MouseButtonID id) override
    {
        Ogre::ImguiManager::getSingleton().mousePressed(arg, id);
        return true;
    }

    bool mouseReleased(const OIS::MouseEvent &arg, OIS::MouseButtonID id) override
    {
        Ogre::ImguiManager::getSingleton().mouseReleased(arg, id);
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
    NodeGraphTool               m_nodegraph;
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

