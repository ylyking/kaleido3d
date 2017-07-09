/*
https://github.com/GameTechDev/asteroids_d3d12
Apache License
*/
#include <Kaleido3D.h>
#include <ngfx.h>

#if _WIN32
#pragma comment(linker,"/subsystem:console")
#endif

using namespace ngfx;

int main()
{
  Ptr<Factory> factory;
  CreateFactory(factory.GetAddressOf());

  Ptr<Device> device;
  uint32_t Count = 0;
  factory->EnumDevice(&Count, nullptr);
  factory->EnumDevice(&Count, device.GetAddressOf());

  Ptr<CommandQueue> queue;
  device->CreateCommandQueue(CommandQueueType::Graphics, queue.GetAddressOf());

  Ptr<SwapChain> swapChain;
  factory->CreateSwapchain(nullptr, queue.Get(), nullptr, swapChain.GetAddressOf());

  Ptr<Buffer> buffer;
  BufferDesc bufferDesc{ BufferViewBit::VertexBuffer, StorageOption::Managed, 10 };
  device->CreateBuffer(&bufferDesc, buffer.GetAddressOf());

  Ptr<Texture> texture;
  device->CreateTexture(nullptr, texture.GetAddressOf());

  Ptr<Fence> fence;
  device->CreateFence(fence.GetAddressOf());

  Ptr<Compiler> compiler;
  factory->CreateCompiler(ngfx::ShaderLang::HLSL, compiler.GetAddressOf());

  Ptr<Function> function;
  compiler->Compile(nullptr, nullptr, 0, function.GetAddressOf());

  Ptr<Pipeline> computePipeline;
  device->CreateComputePipeline(nullptr, nullptr, computePipeline.GetAddressOf());

  Ptr<RenderPass> renderPass;
  device->CreateRenderPass(nullptr, renderPass.GetAddressOf());

  Ptr<Pipeline> renderPipeline;
  device->CreateRenderPipeline(nullptr, nullptr, nullptr, renderPipeline.GetAddressOf());

  Ptr<CommandBuffer> commandBuffer = Ptr<CommandBuffer>(queue->CommandBuffer());
  
  Ptr<RenderCommandEncoder> renderCommand = Ptr<RenderCommandEncoder>(commandBuffer->RenderCommandEncoder());
  //renderCommand->SetViewport();
  renderCommand->EndEncode();

  commandBuffer->Commit(fence.Get());


  return 0;
}