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

class OgreImGui : public OIS::MouseListener, public OIS::KeyListener
{
public:
    OgreImGui();

    void Init(Ogre::SceneManager* mgr, OIS::Keyboard* keyInput, OIS::Mouse* mouseInput);



    //Inherited from OIS::MouseListener
    virtual bool mouseMoved( const OIS::MouseEvent &arg ) override;
    virtual bool mousePressed( const OIS::MouseEvent &arg, OIS::MouseButtonID id ) override;
    virtual bool mouseReleased( const OIS::MouseEvent &arg, OIS::MouseButtonID id ) override;

    //Inherited from OIS::KeyListener
    virtual bool keyPressed( const OIS::KeyEvent &arg ) override;
    virtual bool keyReleased( const OIS::KeyEvent &arg ) override;

    void render();
    void NewFrame(float deltaTime, float displayWidth, float displayHeight);

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

        int                      mVertexBufferSize;
        int                      mIndexBufferSize;

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
    OIS::Keyboard*              mKeyInput;
    OIS::Mouse*                 mMouseInput;
};

