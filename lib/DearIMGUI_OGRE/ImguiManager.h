#pragma once

#include <imgui.h>
#include <OgreCommon.h>
#include <OISMouse.h>
#include <OISKeyboard.h>

#include <OgreRenderQueueListener.h>
#include <OgreTexture.h>
#include "OgrePrerequisites.h"
#include "OgreRenderable.h"
#include <OgreRenderOperation.h>

/// ImGui rendering for OGRE engine; Usage:
///  1. Call `Init()` after OGRE was started
///  2. Call `NewFrame()` before each render, otherwise IMGUI will crash.
///  3. Use `Inject*()` functions to handle inputs.
///  4. Use any MyGUI functions to create your GUI.
///  5. Call `Render()` to render the GUI.
class OgreImGui
{
public:
    OgreImGui(): mSceneMgr(nullptr) {}

    void Init(Ogre::SceneManager* scenemgr);
    void NewFrame(float deltaTime, float vpWidth, float vpHeight, bool ctrl, bool alt, bool shift);
    void Render();

    // Input-injecting functions
    void InjectMouseMoved( const OIS::MouseEvent &arg );
    void InjectMousePressed( const OIS::MouseEvent &arg, OIS::MouseButtonID id );
    void InjectMouseReleased( const OIS::MouseEvent &arg, OIS::MouseButtonID id );
    void InjectKeyPressed( const OIS::KeyEvent &arg );
    void InjectKeyReleased( const OIS::KeyEvent &arg );

private:

    class ImGUIRenderable : public Ogre::Renderable
    {
    public:
        ImGUIRenderable();
        virtual ~ImGUIRenderable();

        void updateVertexData(const ImDrawVert* vtxBuf, const ImDrawIdx* idxBuf, unsigned int vtxCount, unsigned int idxCount);
        Ogre::Real getSquaredViewDepth(const Ogre::Camera* cam) const   { (void)cam; return 0; }

        void setMaterial( const Ogre::String& matName );
        void setMaterial(const Ogre::MaterialPtr & material);
        virtual const Ogre::MaterialPtr& getMaterial(void) const override;
        virtual void getWorldTransforms( Ogre::Matrix4* xform ) const override;
        virtual void getRenderOperation( Ogre::RenderOperation& op ) override;
        virtual const Ogre::LightList& getLights(void) const override;

        int    mVertexBufferSize;
        int    mIndexBufferSize;

    private:
        void initImGUIRenderable(void);

        Ogre::MaterialPtr mMaterial;
        Ogre::RenderOperation mRenderOp;
    };

    void createFontTexture();
    void createMaterial();

    Ogre::SceneManager*         mSceneMgr;
    Ogre::Pass*                 mPass;
    Ogre::TexturePtr            mFontTex;
};

