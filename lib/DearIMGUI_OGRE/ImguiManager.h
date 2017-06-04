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

        class ImGUIRenderable : public Ogre::Renderable
            {

            public:
                ImGUIRenderable();
                ~ImGUIRenderable();

                void updateVertexData(ImDrawData* data,unsigned int cmdIndex);
                Ogre::Real getSquaredViewDepth(const Ogre::Camera* cam) const   { (void)cam; return 0; }

                void setMaterial( const Ogre::String& matName );
		        virtual void setMaterial(const Ogre::MaterialPtr & material);
                virtual const Ogre::MaterialPtr& getMaterial(void) const;
                virtual void getWorldTransforms( Ogre::Matrix4* xform ) const;
                virtual void getRenderOperation( Ogre::RenderOperation& op );
                virtual const Ogre::LightList& getLights(void) const;

                int                      mVertexBufferSize;
                int                      mIndexBufferSize;

            private:

                Ogre::MaterialPtr mMaterial;
                Ogre::RenderOperation mRenderOp;

                void initImGUIRenderable(void);

            };

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

