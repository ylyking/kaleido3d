#pragma once

namespace k3d
{

    /*
    Frostbite framegraph impl
    class IRenderContext;
    class FrameGraphBuilder;
    class FrameGraphResource;
    class FrameGraph;
    class FrameGraphMutableResource;
    class RenderPassBuilder;

    class RenderPass
    {
    public:
    RenderPass(FrameGraphBuilder& builder, FrameGraphResource input, FrameGraphMutableResource renderTarget) {}
    ~RenderPass() {}
    };

    class FrameGraphResource
    {
    public:

    private:
    Ptr<ngfx::RenderPass>     m_RenderPass;
    Ptr<ngfx::FrameBuffer>    m_TransientFramebuffer;
    };

    class RenderPassResources
    {
    public:

    FrameGraphResource getTexture(FrameGraphResource const& in);

    Ptr<ngfx::Texture> targetTextures[8]
    };

    class FrameGraphMutableResource : public FrameGraphResource
    {
    public:

    };

    class RenderPassBuilder
    {
    public:
    RenderPassBuilder() {}

    FrameGraphResource read(FrameGraphResource in);

    FrameGraphResource useRenderTarget(FrameGraphResource in);

    private:
    };

    class FrameGraph
    {
    public:
    FrameGraph() {}
    ~FrameGraph() {}

    template <typename TPassData>
    RenderPass addCallbackPass(String const& passName,
    Function<void(RenderPassBuilder&, TPassData&)> setup,
    Function<void(const TPassData&, const RenderPassResources&, IRenderContext&)>)
    {

    }
    };

    FrameGraphResource AddPass(FrameGraph& frameGraph,
    FrameGraphResource input, FrameGraphResource output)
    {
    struct PassData
    {
    FrameGraphResource input;
    FrameGraphResource output;
    };
    auto renderPass = frameGraph.addCallbackPass<PassData>("Renderpass",
    [&](RenderPassBuilder& builder, PassData& data) // setup phase
    {
    data.input = builder.read(input);
    data.output = builder.useRenderTarget(output).targetTexture[0];
    },
    [=](const PassData& data, const RenderPassResources& resources, IRenderContext& contex)
    {
    //drawTexture2d(context, resources.getTexture(data.input));
    });
    return renderPass.output;
    }
    */

    class Renderer
    {
    public:
        Renderer();
        ~Renderer();

        void Init(uint32_t width, uint32_t height, ngfx::PixelFormat format, void* winHandle);

    protected:
        virtual void OnInitialize();

    private:
        ngfx::Ptr<ngfx::SwapChain>      m_Swapchain;
        ngfx::Ptr<ngfx::CommandQueue>   m_Queue;
        ngfx::Ptr<ngfx::Device>         m_Device;
        ngfx::Ptr<ngfx::Factory>        m_Factory;
    };
}