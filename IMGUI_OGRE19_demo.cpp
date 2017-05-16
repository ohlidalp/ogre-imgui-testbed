// IMGUI_OGRE19_demo.cpp : Defines the entry point for the console application.
//

///////////////////////////// HEADER ////////////////////////////////
 
#include "BaseApplication.h"
#include "ImguiManager.h"
 
class TutorialApplication : public BaseApplication
{
public:
    TutorialApplication() {};
    virtual ~TutorialApplication() {};
protected:
  virtual void createScene();
  virtual bool frameStarted(const Ogre::FrameEvent& fe) override; // From Ogre::SceneManager
  virtual void createFrameListener() override; // From BaseApplication
private:
  bool processUnbufferedInput(const Ogre::FrameEvent& fe);
};
 
 
///////////////////////////// SOURCE ///////////////////////////////////


void TutorialApplication::createFrameListener() 
{
    BaseApplication::createFrameListener(); // Also creates OIS objects

    // === Create IMGUI ====
    Ogre::ImguiManager::createSingleton();
    Ogre::ImguiManager::getSingleton().init(mSceneMgr, mKeyboard, mMouse); // OIS mouse + keyboard
}
 
void TutorialApplication::createScene()
{
  mSceneMgr->setAmbientLight(Ogre::ColourValue(.7f, .7f, .7f));
  mCamera->getViewport()->setBackgroundColour(Ogre::ColourValue(0.f, 0.2f, 0.1f)); // Dark green
 
  Ogre::Entity* ninjaEntity = mSceneMgr->createEntity("ninja.mesh");
  Ogre::SceneNode* ninjaNode = mSceneMgr->getRootSceneNode()->createChildSceneNode("NinjaNode");
  ninjaNode->attachObject(ninjaEntity);
}
 
bool TutorialApplication::frameStarted(const Ogre::FrameEvent& fe)
{
    // Need to capture/update each device
    mKeyboard->capture();
    mMouse->capture();

    // ===== Start IMGUI frame =====
    Ogre::Rect screen_size(0,0,mWindow->getWidth(),mWindow->getHeight());
    Ogre::ImguiManager::getSingleton().newFrame(fe.timeSinceLastFrame, screen_size);

    // ===== Draw IMGUI demo window ====
    ImGui::ShowTestWindow();

    return processUnbufferedInput(fe);
}

bool TutorialApplication::processUnbufferedInput(const Ogre::FrameEvent& fe)
{

  static Ogre::Real rotate = static_cast<Ogre::Real>(.13);
  static Ogre::Real move = 250;
 
  // Moving the Ninja
  Ogre::Vector3 dirVec = Ogre::Vector3::ZERO;
 
  if (mKeyboard->isKeyDown(OIS::KC_I))
    dirVec.z -= move;
 
  if (mKeyboard->isKeyDown(OIS::KC_K))
    dirVec.z += move;
 
  if (mKeyboard->isKeyDown(OIS::KC_U))
    dirVec.y += move;
 
  if (mKeyboard->isKeyDown(OIS::KC_O))
    dirVec.y -= move;
 
  if (mKeyboard->isKeyDown(OIS::KC_J))
  {
    if (mKeyboard->isKeyDown(OIS::KC_LSHIFT))
      mSceneMgr->getSceneNode("NinjaNode")->yaw(Ogre::Degree(5 * rotate));
    else
      dirVec.x -= move;
  }
 
  if (mKeyboard->isKeyDown(OIS::KC_L))
  {
    if (mKeyboard->isKeyDown(OIS::KC_LSHIFT))
      mSceneMgr->getSceneNode("NinjaNode")->yaw(Ogre::Degree(-5 * rotate));
    else
      dirVec.x += move;
  }
 
  mSceneMgr->getSceneNode("NinjaNode")->translate(
    dirVec * fe.timeSinceLastFrame,
    Ogre::Node::TS_LOCAL);
 
  return true;
}

int main(int argc, char *argv[])
{
    try
    {
        TutorialApplication app;
        app.go();
    }
    catch(Ogre::Exception& e)
    {
        std::cerr << "An exception has occurred: " << e.getFullDescription().c_str() << std::endl;
    }

    return 0;
}

