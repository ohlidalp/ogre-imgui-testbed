

#include <OgreCamera.h>
#include <OgreEntity.h>
#include <OgreLogManager.h>
#include <OgreRoot.h>
#include <OgreViewport.h>
#include <OgreSceneManager.h>
#include <OgreRenderWindow.h>
#include <OgreConfigFile.h>


#  include <OISEvents.h>
#  include <OISInputManager.h>
#  include <OISKeyboard.h>
#  include <OISMouse.h>

#include "ImguiManager.h"

static Ogre::Root*                 mRoot       = nullptr;
static Ogre::Camera*               mCamera     = nullptr;
static Ogre::SceneManager*         mSceneMgr   = nullptr;
static Ogre::RenderWindow*         mWindow     = nullptr;

static bool                        mShutDown = false;

//OIS Input devices
static OIS::InputManager*          mInputManager = nullptr;
static OIS::Mouse*                 mMouse        = nullptr;
static OIS::Keyboard*              mKeyboard     = nullptr;

class MiniFrameListener: public Ogre::FrameListener
{
    virtual bool frameRenderingQueued(const Ogre::FrameEvent& evt) override;
    virtual bool frameStarted(const Ogre::FrameEvent& evt) override;
};

class BaseApplication :  public Ogre::WindowEventListener, public OIS::KeyListener, public OIS::MouseListener
{
public:
    void Shutdown();
    bool setup();
    bool configure(void);
    void chooseSceneManager(void);
    void createCamera(void);
    void createFrameListener(void);
    void createScene(void);
    void createViewports(void);

    void loadResources(void);



    // OIS KeyListener
    virtual bool keyPressed(const OIS::KeyEvent &arg) override;
    virtual bool keyReleased(const OIS::KeyEvent &arg) override;

    // OIS MouseListener
    virtual bool mouseMoved(const OIS::MouseEvent &arg) override;
    virtual bool mousePressed(const OIS::MouseEvent &arg, OIS::MouseButtonID id) override;
    virtual bool mouseReleased(const OIS::MouseEvent &arg, OIS::MouseButtonID id) override;

    // --  Ogre::WindowEventListener --

    // Adjust mouse clipping area
    virtual void windowResized(Ogre::RenderWindow* rw) override;
    // Unattach OIS before window shutdown (very important under Linux)
    virtual void windowClosed(Ogre::RenderWindow* rw) override;

};

int main(int argc, char *argv[])
{
    try
    {
        BaseApplication app;

        if (!app.setup())
        {
            app.Shutdown();
            return 0;
        }

        MiniFrameListener frame_listener;
        mRoot->addFrameListener(&frame_listener);

        mRoot->startRendering();
        app.Shutdown();
    }
    catch(Ogre::Exception& e)
    {
        std::cerr << "An exception has occurred: " << e.getFullDescription().c_str() << std::endl;
    }

    
    return 0;
}

void BaseApplication::createScene()
{
  mSceneMgr->setAmbientLight(Ogre::ColourValue(.7f, .7f, .7f));
  mCamera->getViewport()->setBackgroundColour(Ogre::ColourValue(0.f, 0.2f, 0.1f)); // Dark green
 
}

//---------------------------------------------------------------------------
void BaseApplication::Shutdown(void)
{

    // Remove ourself as a Window listener
    Ogre::WindowEventUtilities::removeWindowEventListener(mWindow, this);
    windowClosed(mWindow);
    delete mRoot;
}

//---------------------------------------------------------------------------
bool BaseApplication::configure(void)
{
    // Show the configuration dialog and initialise the system.
    // You can skip this and use root.restoreConfig() to load configuration
    // settings if you were sure there are valid ones saved in ogre.cfg.
    if(mRoot->showConfigDialog())
    {
        // If returned true, user clicked OK so initialise.
        // Here we choose to let the system create a default rendering window by passing 'true'.
        mWindow = mRoot->initialise(true, "TutorialApplication Render Window");

        return true;
    }
    else
    {
        return false;
    }
}
//---------------------------------------------------------------------------
void BaseApplication::chooseSceneManager(void)
{
    // Get the SceneManager, in this case a generic one
    mSceneMgr = mRoot->createSceneManager(Ogre::ST_GENERIC);
}
//---------------------------------------------------------------------------
void BaseApplication::createCamera(void)
{
    // Create the camera
    mCamera = mSceneMgr->createCamera("PlayerCam");

    // Position it at 500 in Z direction
    mCamera->setPosition(Ogre::Vector3(0,0,80));
    // Look back along -Z
    mCamera->lookAt(Ogre::Vector3(0,0,-300));
    mCamera->setNearClipDistance(5);
}
//---------------------------------------------------------------------------
void BaseApplication::createFrameListener(void)
{
    Ogre::LogManager::getSingletonPtr()->logMessage("*** Initializing OIS ***");
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
   // RoR         pl.insert(OIS::ParamList::value_type("x11_keyboard_grab", "false"));
#else
            pl.insert(OIS::ParamList::value_type("w32_mouse", "DISCL_FOREGROUND"));
            pl.insert(OIS::ParamList::value_type("w32_mouse", "DISCL_NONEXCLUSIVE"));
   // RoR         pl.insert(OIS::ParamList::value_type("w32_keyboard", "DISCL_FOREGROUND"));
   // RoR         pl.insert(OIS::ParamList::value_type("w32_keyboard", "DISCL_NONEXCLUSIVE"));
#endif // LINUX

    mInputManager = OIS::InputManager::createInputSystem(pl);

    mKeyboard = static_cast<OIS::Keyboard*>(mInputManager->createInputObject(OIS::OISKeyboard, true));
    mMouse = static_cast<OIS::Mouse*>(mInputManager->createInputObject(OIS::OISMouse, true));

    mMouse->setEventCallback(this);
    mKeyboard->setEventCallback(this);

    // Set initial mouse clipping size
    windowResized(mWindow);

    // Register as a Window listener
    Ogre::WindowEventUtilities::addWindowEventListener(mWindow, this);

    

    // === Create IMGUI ====
    Ogre::ImguiManager::createSingleton();
    Ogre::ImguiManager::getSingleton().init(mSceneMgr, mKeyboard, mMouse); // OIS mouse + keyboard
}

//---------------------------------------------------------------------------
void BaseApplication::createViewports(void)
{
    // Create one viewport, entire window
    Ogre::Viewport* vp = mWindow->addViewport(mCamera);
    vp->setBackgroundColour(Ogre::ColourValue(0,0,0));

    // Alter the camera aspect ratio to match the viewport
    mCamera->setAspectRatio(Ogre::Real(vp->getActualWidth()) / Ogre::Real(vp->getActualHeight()));
}
//---------------------------------------------------------------------------


bool MiniFrameListener::frameStarted(const Ogre::FrameEvent& fe)
{
    // Need to capture/update each device
    mKeyboard->capture();
    mMouse->capture();

    // ===== Start IMGUI frame =====
    Ogre::Rect screen_size(0,0,mWindow->getWidth(),mWindow->getHeight());
    Ogre::ImguiManager::getSingleton().newFrame(fe.timeSinceLastFrame, screen_size);

    // ===== Draw IMGUI demo window ====
    ImGui::ShowTestWindow();

    return true;
}

//---------------------------------------------------------------------------
void BaseApplication::loadResources(void)
{
    Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups();
}

//---------------------------------------------------------------------------
bool BaseApplication::setup(void)
{
#ifdef _DEBUG
    std::string mPluginsCfg = "plugins_d.cfg";
#else
    std::string mPluginsCfg = "plugins.cfg";
#endif

    mRoot = new Ogre::Root(mPluginsCfg);



    bool carryOn = configure();
    if (!carryOn) return false;

    chooseSceneManager();
    createCamera();
    createViewports();

    // Set default mipmap level (NB some APIs ignore this)
    Ogre::TextureManager::getSingleton().setDefaultNumMipmaps(5);


    // Load resources
    loadResources();

    // Create the scene
    createScene();

    createFrameListener();

    return true;
};
//---------------------------------------------------------------------------
bool MiniFrameListener::frameRenderingQueued(const Ogre::FrameEvent& evt)
{
    if(mWindow->isClosed())
        return false;

    if(mShutDown)
        return false;

    return true;
}
//---------------------------------------------------------------------------
bool BaseApplication::keyPressed( const OIS::KeyEvent &arg )
{

    if (arg.key == OIS::KC_T)   // cycle polygon rendering mode
    {
        Ogre::String newVal;
        Ogre::TextureFilterOptions tfo;
        unsigned int aniso;
        static char tex_filter = 'X';

        switch (tex_filter)
        {
        case 'B':
            newVal = "Trilinear";
            tfo = Ogre::TFO_TRILINEAR;
            aniso = 1;
            break;
        case 'T':
            newVal = "Anisotropic";
            tfo = Ogre::TFO_ANISOTROPIC;
            aniso = 8;
            break;
        case 'A':
            newVal = "None";
            tfo = Ogre::TFO_NONE;
            aniso = 1;
            break;
        default:
            newVal = "Bilinear";
            tfo = Ogre::TFO_BILINEAR;
            aniso = 1;
        }

        Ogre::MaterialManager::getSingleton().setDefaultTextureFiltering(tfo);
        Ogre::MaterialManager::getSingleton().setDefaultAnisotropy(aniso);
        tex_filter = newVal[0];
    }
    else if (arg.key == OIS::KC_R)   // cycle polygon rendering mode
    {
        Ogre::String newVal;
        Ogre::PolygonMode pm;

        switch (mCamera->getPolygonMode())
        {
        case Ogre::PM_SOLID:
            newVal = "Wireframe";
            pm = Ogre::PM_WIREFRAME;
            break;
        case Ogre::PM_WIREFRAME:
            newVal = "Points";
            pm = Ogre::PM_POINTS;
            break;
        default:
            newVal = "Solid";
            pm = Ogre::PM_SOLID;
        }

        mCamera->setPolygonMode(pm);
    }
    else if(arg.key == OIS::KC_F5)   // refresh all textures
    {
        Ogre::TextureManager::getSingleton().reloadAll();
    }
    else if (arg.key == OIS::KC_SYSRQ)   // take a screenshot
    {
        mWindow->writeContentsToTimestampedFile("screenshot", ".jpg");
    }
    else if (arg.key == OIS::KC_ESCAPE)
    {
        mShutDown = true;
    }

    Ogre::ImguiManager::getSingleton().keyPressed(arg);
    return true;
}
//---------------------------------------------------------------------------
bool BaseApplication::keyReleased(const OIS::KeyEvent &arg)
{

    Ogre::ImguiManager::getSingleton().keyReleased(arg);
    return true;
}
//---------------------------------------------------------------------------
bool BaseApplication::mouseMoved(const OIS::MouseEvent &arg)
{

    Ogre::ImguiManager::getSingleton().mouseMoved(arg);
    return true;
}
//---------------------------------------------------------------------------
bool BaseApplication::mousePressed(const OIS::MouseEvent &arg, OIS::MouseButtonID id)
{

    Ogre::ImguiManager::getSingleton().mousePressed(arg, id);
    return true;
}
//---------------------------------------------------------------------------
bool BaseApplication::mouseReleased(const OIS::MouseEvent &arg, OIS::MouseButtonID id)
{

    Ogre::ImguiManager::getSingleton().mouseReleased(arg, id);
    return true;
}
//---------------------------------------------------------------------------
// Adjust mouse clipping area
void BaseApplication::windowResized(Ogre::RenderWindow* rw)
{
    unsigned int width, height, depth;
    int left, top;
    rw->getMetrics(width, height, depth, left, top);

    const OIS::MouseState &ms = mMouse->getMouseState();
    ms.width = width;
    ms.height = height;
}
//---------------------------------------------------------------------------
// Unattach OIS before window shutdown (very important under Linux)
void BaseApplication::windowClosed(Ogre::RenderWindow* rw)
{
    // Only close for window that created OIS (the main window in these demos)
    if(rw == mWindow)
    {
        if(mInputManager)
        {
            mInputManager->destroyInputObject(mMouse);
            mInputManager->destroyInputObject(mKeyboard);

            OIS::InputManager::destroyInputSystem(mInputManager);
            mInputManager = 0;
        }
    }
}
//---------------------------------------------------------------------------