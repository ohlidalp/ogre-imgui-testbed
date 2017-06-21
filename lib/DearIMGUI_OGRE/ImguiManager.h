#pragma once

#include <imgui.h>
#include <OgreCommon.h>
#include <OISMouse.h>
#include <OISKeyboard.h>

#include <OgreRenderQueueListener.h>
#include <OgreSingleton.h>
#include <OgreTexture.h>

#include "ImguiRenderable.h"

// Copied from <imgui_internal.h> to avoid the whole include.
static inline ImVec2 operator*(const ImVec2& lhs, const float rhs)              { return ImVec2(lhs.x*rhs, lhs.y*rhs); }
static inline ImVec2 operator/(const ImVec2& lhs, const float rhs)              { return ImVec2(lhs.x/rhs, lhs.y/rhs); }
static inline ImVec2 operator+(const ImVec2& lhs, const ImVec2& rhs)            { return ImVec2(lhs.x+rhs.x, lhs.y+rhs.y); }
static inline ImVec2 operator-(const ImVec2& lhs, const ImVec2& rhs)            { return ImVec2(lhs.x-rhs.x, lhs.y-rhs.y); }
static inline ImVec2 operator*(const ImVec2& lhs, const ImVec2& rhs)            { return ImVec2(lhs.x*rhs.x, lhs.y*rhs.y); }
static inline ImVec2 operator/(const ImVec2& lhs, const ImVec2& rhs)            { return ImVec2(lhs.x/rhs.x, lhs.y/rhs.y); }
static inline ImVec2& operator+=(ImVec2& lhs, const ImVec2& rhs)                { lhs.x += rhs.x; lhs.y += rhs.y; return lhs; }
static inline ImVec2& operator-=(ImVec2& lhs, const ImVec2& rhs)                { lhs.x -= rhs.x; lhs.y -= rhs.y; return lhs; }
static inline ImVec2& operator*=(ImVec2& lhs, const float rhs)                  { lhs.x *= rhs; lhs.y *= rhs; return lhs; }
static inline ImVec2& operator/=(ImVec2& lhs, const float rhs)                  { lhs.x /= rhs; lhs.y /= rhs; return lhs; }
static inline ImVec4 operator-(const ImVec4& lhs, const ImVec4& rhs)            { return ImVec4(lhs.x-rhs.x, lhs.y-rhs.y, lhs.z-rhs.z, lhs.w-rhs.w); }

namespace Ogre
{
    class SceneManager;

    class ImguiManager : public RenderQueueListener,public OIS::MouseListener,public OIS::KeyListener, public Singleton<ImguiManager>
    {
        public:
        static void createSingleton();

        ImguiManager();
        ~ImguiManager();

        virtual void init(Ogre::SceneManager* mgr,OIS::Keyboard* keyInput, OIS::Mouse* mouseInput);

        virtual void newFrame(float deltaTime,const Ogre::Rect & windowRect);

        //inherited from RenderQueueListener
        virtual void renderQueueEnded(uint8 queueGroupId, const String& invocation,bool& repeatThisInvocation);

        //Inherhited from OIS::MouseListener
        virtual bool mouseMoved( const OIS::MouseEvent &arg );
		virtual bool mousePressed( const OIS::MouseEvent &arg, OIS::MouseButtonID id );
		virtual bool mouseReleased( const OIS::MouseEvent &arg, OIS::MouseButtonID id );
        //Inherhited from OIS::KeyListener
		virtual bool keyPressed( const OIS::KeyEvent &arg );
		virtual bool keyReleased( const OIS::KeyEvent &arg );

        void updateVertexData();

        /** Override standard Singleton retrieval.
        @remarks
        Why do we do this? Well, it's because the Singleton
        implementation is in a .h file, which means it gets compiled
        into anybody who includes it. This is needed for the
        Singleton template to work, but we actually only want it
        compiled into the implementation of the class based on the
        Singleton, not all of them. If we don't change this, we get
        link errors when trying to use the Singleton-based class from
        an outside dll.
        @par
        This method just delegates to the template version anyway,
        but the implementation stays in this single compilation unit,
        preventing link errors.
        */
        static ImguiManager& getSingleton(void);
        /** Override standard Singleton retrieval.
        @remarks
        Why do we do this? Well, it's because the Singleton
        implementation is in a .h file, which means it gets compiled
        into anybody who includes it. This is needed for the
        Singleton template to work, but we actually only want it
        compiled into the implementation of the class based on the
        Singleton, not all of them. If we don't change this, we get
        link errors when trying to use the Singleton-based class from
        an outside dll.
        @par
        This method just delegates to the template version anyway,
        but the implementation stays in this single compilation unit,
        preventing link errors.
        */
        static ImguiManager* getSingletonPtr(void);

        protected:

        void createFontTexture();
        void createMaterial();

        std::list<ImGUIRenderable*> mRenderables;

        SceneManager*				mSceneMgr;
        Pass*						mPass;
        int                         mLastRenderedFrame;

        TexturePtr                  mFontTex;

        bool                        mFrameEnded;
        OIS::Keyboard*              mKeyInput;
        OIS::Mouse*                 mMouseInput;
    };
}
