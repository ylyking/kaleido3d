#pragma once

#include <Kaleido3D.h>

// Templ Lib
#include <KTL/String.hpp>
#include <KTL/Functional.hpp>
#include <KTL/SharedPtr.hpp>
#include <Core/App.h>

// NGFX LIb
#include <ngfx.h>

namespace k3d
{
    class App;
    class Renderer;

    /**
     * Engine Base
     * Handle Messages
     * Kick Renderer
     * Asset Loading
     */
    class K3D_API EngineBase
    {
    public:

        EngineBase();
        ~EngineBase();

        void Init(App& app);

        void Loop();

        // App::ProcessMessage
        void HandleMessage(Message const& msg);

        Renderer& GetRenderer();

    protected:

        virtual void OnInitialize(App& app);
        virtual void OnHandleMessage(Message const& msg);
        virtual void KickRenderer(float deltaTime);

        virtual Renderer& CreateRenderer();

    private:

    };

    extern K3D_API Renderer& GetRenderer();

}