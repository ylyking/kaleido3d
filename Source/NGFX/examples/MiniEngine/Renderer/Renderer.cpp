#include "MiniEngine.h"
#include "Renderer.h"

using namespace ngfx;

namespace k3d
{
    Renderer::Renderer()
    {}

    Renderer::~Renderer()
    {}

    void Renderer::Init(uint32_t width, uint32_t height, ngfx::PixelFormat format, void* winHandle)
    {
        if (!m_Factory)
        {
            CreateFactory(m_Factory.GetAddressOf(), true);
            m_Factory->SetName("RendererFactory");
        }
        if (!m_Device)
        {
            uint32_t Count = 0;
            m_Factory->EnumDevice(&Count, nullptr);
            m_Factory->EnumDevice(&Count, m_Device.GetAddressOf());
        }
        if (!m_Queue)
        {
            m_Device->CreateCommandQueue(CommandQueueType::Graphics, m_Queue.GetAddressOf());
        }
        if (!m_Swapchain)
        {
            SwapChainDesc swapChainDesc = { format, width, height, 2, true, PixelFormat::D32Float };
            m_Factory->CreateSwapchain(&swapChainDesc, m_Queue.Get(), winHandle, m_Swapchain.GetAddressOf());
        }
        OnInitialize();
    }

    void Renderer::OnInitialize()
    {

    }

    SharedPtr<Renderer> GRenderer;

    Renderer& GetRenderer()
    {
        return *GRenderer;
    }

    void EngineBase::OnInitialize(App & app)
    {
        GRenderer = MakeShared<Renderer>();
        //GRenderer->Init()
    }

}