#include <Kaleido3D.h>
#include <KTL/Allocator.hpp>
#include "ngfx.h"

#include <wrl/client.h>

#include <d3d12.h>
#include <d3dcompiler.h>
#include <d3d12sdklayers.h>
#include <dxgi1_4.h>

#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "d3dcompiler.lib")

#include <vector>

enum class Log
{
    Debug,
    Info,
    Warn,
    Error,
};

void LogPrint(Log const& i, const char* Tag, const char* Fmt, ...)
{
#if K3DPLATFORM_OS_WIN
    static char Buffer[1024] = { 0 };
    va_list va;
    va_start(va, Fmt);
    vsnprintf_s(Buffer, 1024, Fmt, va);
    va_end(va);
    OutputDebugStringA(Buffer);
#endif
}

using namespace Microsoft::WRL;
using namespace ngfx;
using namespace std;

namespace ngfx
{
    class DXFactory;
}

template <typename TGfxObj>
struct GfxTrait
{
};

template<>
struct GfxTrait<Device>
{
    typedef ID3D12Device NativeType;
};

template<>
struct GfxTrait<CommandQueue>
{
    typedef ID3D12CommandQueue NativeType;
};

template<>
struct GfxTrait<CommandBuffer>
{
    typedef ID3D12CommandList NativeType;
};

template<>
struct GfxTrait<Buffer>
{
    typedef ID3D12Resource NativeType;
};

template<>
struct GfxTrait<Texture>
{
    typedef ID3D12Resource NativeType;
};

template <typename TGfxObj>
struct DeviceChild : public TGfxObj
{
    friend class DX12Device;
protected:
    ComPtr<typename GfxTrait<TGfxObj>::NativeType> Handle;
};

class DX12Queue : public DeviceChild<CommandQueue>
{
public:
    DX12Queue() {}
    ~DX12Queue() override {}
    Result CreateCommandBuffer(CommandBuffer ** ppComandBuffer) override;
private:
    ComPtr<ID3D12CommandAllocator> Allocator;
};

class DX12CommandBuffer : public DeviceChild<CommandBuffer>
{
public:
    DX12CommandBuffer() {}
    ~DX12CommandBuffer() override {}
};

class DX12Device : public Device
{
    friend class ngfx::DXFactory;
public:
    DX12Device(ComPtr<ID3D12Device> device) : Handle(device) {}
    ~DX12Device() override {}

    void GetDesc(DeviceDesc * pDesc) override;
    Result CreateCommandQueue(CommandQueueType queueType, CommandQueue ** pQueue) override;
    Result CreatePipelineLayout(const PipelineLayoutDesc * pPipelineLayoutDesc, PipelineLayout ** ppPipelineLayout) override;
    Result MakeBindTableEncoder(NotNull const ArgumentDesc * pArgumentDescs, int32_t argumentCount, BindTableEncoder ** ppBindingTableLayout) override;
    Result CreateRenderPipeline(NotNull const RenderPipelineDesc * pPipelineDesc, Nullable RenderPass * pRenderPass, Pipeline ** pPipelineState, NotNull PipelineReflection ** ppReflection) override;
    Result CreateComputePipeline(NotNull Function * pComputeFunction, Pipeline ** pPipeline, NotNull PipelineReflection ** ppReflection) override;
    Result CreateComputePipeline(NotNull const Function * pComputeFunction, NotNull const PipelineLayout * ppPipelineLayout, NotNull Pipeline ** ppPipeline) override;
    Result CreatePipelineLibrary(const void * pData, uint64_t Size, PipelineLibrary ** ppPipelineLibrary) override;
    Result CreateLibrary(const CompileOption * compileOption, const void * pData, uint64_t Size, Library ** ppLibrary) override;
    Result CreateRenderPass(const RenderPassDesc * desc, RenderPass ** ppRenderpass) override;
    Result CreateSampler(const SamplerDesc* desc, Sampler ** pSampler) override;
    Result CreateBuffer(const BufferDesc* desc, Buffer ** pBuffer) override;
    Result CreateTexture(const TextureDesc * desc, Texture ** pTexture) override;
    Result CreateFence(Fence ** ppFence) override;
    void WaitIdle() override;

private:
    ComPtr<ID3D12Device> Handle;
};

namespace ngfx
{
    static const D3D_FEATURE_LEVEL requiredFLs[] = {
        D3D_FEATURE_LEVEL_12_1,
        D3D_FEATURE_LEVEL_12_0,
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0,
    };

    class DXFactory : public ngfx::Factory
    {
    public:
        DXFactory(ComPtr<IDXGIFactory1> factory) : Handle(factory) {}

        ~DXFactory() {}

        Result EnumDevice(uint32_t * count, Device ** ppDevice)
        {
            if (!count)
                return Result::ParamError;
            vector<pair<IDXGIAdapter1*, D3D_FEATURE_LEVEL>> adapters;
            IDXGIAdapter1* pAdapter = nullptr;
            for (UINT adapterIndex = 0; DXGI_ERROR_NOT_FOUND != Handle->EnumAdapters1(adapterIndex, &pAdapter); ++adapterIndex)
            {
                DXGI_ADAPTER_DESC1 desc;
                pAdapter->GetDesc1(&desc);
                char descStr[256] = { 0 };
                WideCharToMultiByte(CP_UTF8, 0, desc.Description, 128, descStr, 256, nullptr, nullptr);
                LogPrint(Log::Info, "NGFX_D3D12", "DX12Factory::EnumDevice %s.\n", descStr);
                if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) 
                {
                    continue;
                }
                for (auto fl : requiredFLs)
                {
                    if (SUCCEEDED(D3D12CreateDevice(pAdapter, fl, __uuidof(ID3D12Device), nullptr)))
                    {
                        adapters.push_back({ pAdapter, fl });
                        break;
                    }
                }
            }
            *count = adapters.size();
            if (!ppDevice)
                return Result::ParamError;
            for (size_t i = 0; i<adapters.size(); i++)
            {
                ComPtr<ID3D12Device> ComPtrDevice;
                if (SUCCEEDED(D3D12CreateDevice(adapters[i].first, adapters[i].second,
                    IID_PPV_ARGS(ComPtrDevice.GetAddressOf()))))
                {
                    ppDevice[i] = new DX12Device(ComPtrDevice);
                }
            }
            return Result::Ok;
        }

        Result CreateSwapchain(
            NotNull const SwapChainDesc * desc,
            NotNull CommandQueue * pCommandQueue,
            void * pWindow,
            NotNull SwapChain ** pSwapchain)
        {
            return Result::Ok;
        }

        friend Result CreateFactory(ngfx::Factory ** ppFactory, bool debugEnabled)
        {
            ComPtr<IDXGIFactory1> Factory;
            HRESULT hr = ::CreateDXGIFactory1(IID_PPV_ARGS(Factory.GetAddressOf()));
            if (SUCCEEDED(hr))
            {
                *ppFactory = new DXFactory(Factory);
                return Result::Ok;
            }
            return Result::Failed;
        }
    private:
        ComPtr<IDXGIFactory1> Handle;
    };
}

void DX12Device::GetDesc(DeviceDesc * pDesc)
{

}

Result DX12Queue::CreateCommandBuffer(CommandBuffer ** ppComandBuffer)
{
    ComPtr<ID3D12Device> Device;
    Handle->GetDevice(IID_PPV_ARGS(Device.GetAddressOf()));
    //Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, nullptr, nullptr, IID_PPV_ARGS());
    return Result::Ok;
}

Result DX12Device::CreateCommandQueue(CommandQueueType queueType, CommandQueue ** pQueue)
{
    D3D12_COMMAND_QUEUE_DESC desc = {};
    switch (queueType)
    {
    case CommandQueueType::Graphics:
        desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
        break;
    case CommandQueueType::Compute:
        desc.Type = D3D12_COMMAND_LIST_TYPE_COMPUTE;
        break;
    case CommandQueueType::Copy:
        desc.Type = D3D12_COMMAND_LIST_TYPE_COPY;
        break;
    }
    desc.NodeMask = 0; // MGPU
    ComPtr<ID3D12CommandQueue> queue;
    HRESULT ret = Handle->CreateCommandQueue(&desc, IID_PPV_ARGS(queue.GetAddressOf()));
    if (SUCCEEDED(ret))
    {
        return Result::Ok;
    }
    return Result::Failed;
}

Result DX12Device::CreatePipelineLayout(const PipelineLayoutDesc * pPipelineLayoutDesc, PipelineLayout ** ppPipelineLayout)
{
    return Result::Ok;
}

Result DX12Device::MakeBindTableEncoder(NotNull const ArgumentDesc * pArgumentDescs, int32_t argumentCount, BindTableEncoder ** ppBindingTableLayout)
{
    return Result::Ok;
}

Result DX12Device::CreateRenderPipeline(NotNull const RenderPipelineDesc * pPipelineDesc, Nullable RenderPass * pRenderPass, Pipeline ** pPipelineState, NotNull PipelineReflection ** ppReflection)
{
    return Result::Ok;
}

Result DX12Device::CreateComputePipeline(NotNull Function * pComputeFunction, Pipeline ** pPipeline, NotNull PipelineReflection ** ppReflection)
{
    return Result::Ok;
}

Result DX12Device::CreateComputePipeline(NotNull const Function * pComputeFunction, NotNull const PipelineLayout * ppPipelineLayout, NotNull Pipeline ** ppPipeline)
{
    return Result::Ok;
}

Result DX12Device::CreatePipelineLibrary(const void * pData, uint64_t Size, PipelineLibrary ** ppPipelineLibrary)
{
    return Result::Ok;
}

Result DX12Device::CreateLibrary(const CompileOption * compileOption, const void * pData, uint64_t Size, Library ** ppLibrary)
{
    return Result::Ok;
}

Result DX12Device::CreateRenderPass(const RenderPassDesc * desc, RenderPass ** ppRenderpass)
{
    return Result::Ok;
}

Result DX12Device::CreateSampler(const SamplerDesc* desc, Sampler ** pSampler)
{
    return Result::Ok;
}

Result DX12Device::CreateBuffer(const BufferDesc* desc, Buffer ** pBuffer)
{
    return Result::Ok;
}

Result DX12Device::CreateTexture(const TextureDesc * desc, Texture ** pTexture)
{
    return Result::Ok;
}

Result DX12Device::CreateFence(Fence ** ppFence)
{
    return Result::Ok;
}

void DX12Device::WaitIdle()
{

}
