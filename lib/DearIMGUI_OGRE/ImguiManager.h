#pragma once

#include <imgui.h>
#include <OgreCommon.h>
#include <OISMouse.h>
#include <OISKeyboard.h>

#include <OgreRenderQueueListener.h>
#include <OgreTexture.h>

#include "ImguiRenderable.h"




    class ImguiManager : public Ogre::RenderQueueListener,public OIS::MouseListener,public OIS::KeyListener
    {
        public:

        ImguiManager();
        void Shutdown();

        virtual void init(Ogre::SceneManager* mgr,OIS::Keyboard* keyInput, OIS::Mouse* mouseInput);

        virtual void newFrame(float deltaTime,const Ogre::Rect & windowRect);

        //inherited from RenderQueueListener
        virtual void renderQueueEnded(Ogre::uint8 queueGroupId, const Ogre::String& invocation,bool& repeatThisInvocation);

        //Inherhited from OIS::MouseListener
        virtual bool mouseMoved( const OIS::MouseEvent &arg );
		virtual bool mousePressed( const OIS::MouseEvent &arg, OIS::MouseButtonID id );
		virtual bool mouseReleased( const OIS::MouseEvent &arg, OIS::MouseButtonID id );
        //Inherhited from OIS::KeyListener
		virtual bool keyPressed( const OIS::KeyEvent &arg );
		virtual bool keyReleased( const OIS::KeyEvent &arg );

        void updateVertexData();

        protected:

        void createFontTexture();
        void createMaterial();

        std::list<ImGUIRenderable*> mRenderables;

        Ogre::SceneManager*				mSceneMgr;
        Ogre::Pass*						mPass;
        int                         mLastRenderedFrame;

        Ogre::TexturePtr                  mFontTex;

        bool                        mFrameEnded;
        OIS::Keyboard*              mKeyInput;
        OIS::Mouse*                 mMouseInput;
    };

