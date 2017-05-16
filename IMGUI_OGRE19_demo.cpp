// IMGUI_OGRE19_demo.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

///////////////////////////// HEADER ////////////////////////////////
 
#include "BaseApplication.h"
 
class TutorialApplication : public BaseApplication
{
public:
  TutorialApplication();
  virtual ~TutorialApplication();
protected:
  virtual void createScene();
  virtual bool frameRenderingQueued(const Ogre::FrameEvent& fe);
private:
  bool processUnbufferedInput(const Ogre::FrameEvent& fe);
};
 
 
///////////////////////////// SOURCE ///////////////////////////////////


TutorialApplication::TutorialApplication()
{
}
 
TutorialApplication::~TutorialApplication()
{
}
 
void TutorialApplication::createScene()
{
  mSceneMgr->setAmbientLight(Ogre::ColourValue(.25, .25, .25));
 
  Ogre::Light* pointLight = mSceneMgr->createLight("PointLight");
  pointLight->setType(Ogre::Light::LT_POINT);
  pointLight->setPosition(250, 150, 250);
  pointLight->setDiffuseColour(Ogre::ColourValue::White);
  pointLight->setSpecularColour(Ogre::ColourValue::White);
 
  Ogre::Entity* ninjaEntity = mSceneMgr->createEntity("ninja.mesh");
  Ogre::SceneNode* ninjaNode = mSceneMgr->getRootSceneNode()->createChildSceneNode(
    "NinjaNode");
  ninjaNode->attachObject(ninjaEntity);
 
}
 
bool TutorialApplication::frameRenderingQueued(const Ogre::FrameEvent& fe)
{
  bool ret = BaseApplication::frameRenderingQueued(fe);
 
  if (!processUnbufferedInput(fe))
    return false;
 
  return ret;
}
 
bool TutorialApplication::processUnbufferedInput(const Ogre::FrameEvent& fe)
{
  static bool mouseDownLastFrame = false;
  static Ogre::Real toggleTimer = 0.0;
  static Ogre::Real rotate = static_cast<Ogre::Real>(.13);
  static Ogre::Real move = 250;
 
  // First toggle method
  bool leftMouseDown = mMouse->getMouseState().buttonDown(OIS::MB_Left);
 
  if (leftMouseDown && !mouseDownLastFrame)
  {
    Ogre::Light* light = mSceneMgr->getLight("PointLight");
    light->setVisible(!light->isVisible());
  }
 
  mouseDownLastFrame = leftMouseDown;
 
  // Second toggle method
  toggleTimer -= fe.timeSinceLastFrame;
 
  if ((toggleTimer < 0) && mMouse->getMouseState().buttonDown(OIS::MB_Right))
  {
    toggleTimer = .5;
 
    Ogre::Light* light = mSceneMgr->getLight("PointLight");
    light->setVisible(!light->isVisible());
  }
 
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
 
 
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
#define WIN32_LEAN_AND_MEAN
#include "windows.h"
#endif

int main(int argc, char *argv[])
  {
    // Create application object
    TutorialApplication app;
 
    try {
      app.go();
    } catch(Ogre::Exception& e)  {
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
      MessageBoxA(
        NULL, 
        e.getFullDescription().c_str(), 
        "An exception has occurred!", 
        MB_OK | MB_ICONERROR | MB_TASKMODAL);
#else
      std::cerr << "An exception has occurred: " <<
	e.getFullDescription().c_str() << std::endl;
#endif
    }
 
    return 0;
  }

