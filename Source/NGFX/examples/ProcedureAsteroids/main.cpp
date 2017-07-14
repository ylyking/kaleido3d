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
  CreateFactory(factory.GetAddressOf(), true);
  factory->SetName("VulkanFactory");

  Ptr<Device> device;
  uint32_t Count = 0;
  factory->EnumDevice(&Count, nullptr);
  factory->EnumDevice(&Count, device.GetAddressOf());

  Ptr<CommandQueue> queue;
  device->CreateCommandQueue(CommandQueueType::Graphics, queue.GetAddressOf());
  queue->SetName("GraphicsQueue");

  Ptr<SwapChain> swapChain;
  SwapChainDesc swapChainDesc = { PixelFormat::RGBA8UNorm, 800, 600, 2, true, PixelFormat::D32Float };
  factory->CreateSwapchain(&swapChainDesc, queue.Get(), nullptr, swapChain.GetAddressOf());
  swapChain->SetName("DefaultSwapchain");

  Ptr<Buffer> buffer;
  BufferDesc bufferDesc{ BufferViewBit::VertexBuffer, StorageOption::Managed, 10 };
  device->CreateBuffer(&bufferDesc, buffer.GetAddressOf());
  buffer->SetName("Buffer0");

  Ptr<Texture> texture;
  TextureDesc texDesc = { TextureViewBit::RenderTarget, StorageOption::Managed, 
    MultiSampleFlag::MS1x, PixelFormat::RGBA8UNorm,
    1024, 1024, 1, 0, 0 };
  device->CreateTexture(&texDesc, texture.GetAddressOf());
  texture->SetName("Texture0");

  Ptr<Fence> fence;
  device->CreateFence(fence.GetAddressOf());
  fence->SetName("DefaultFence");

  Ptr<Compiler> compiler;
  factory->CreateCompiler(ngfx::ShaderLang::HLSL, compiler.GetAddressOf());

  Ptr<Function> function;
  ShaderOption shaderOpt = { ShaderType::Vertex, ShaderLang::HLSL, "main", ShaderProfile::SM5, ShaderFormat::Text };
  compiler->Compile(&shaderOpt, nullptr, 0, function.GetAddressOf());

  Ptr<PipelineLayout> pipelineLayout;
  PipelineLayoutDesc pipelineLayoutDesc = {};
  device->CreatePipelineLayout(&pipelineLayoutDesc, pipelineLayout.GetAddressOf());

  Ptr<Pipeline> computePipeline;
  device->CreateComputePipeline(nullptr, pipelineLayout.Get(), computePipeline.GetAddressOf());

  Ptr<RenderPass> renderPass;
  RenderPassDesc renderPassDesc = { nullptr, 0, nullptr };
  device->CreateRenderPass(&renderPassDesc, renderPass.GetAddressOf());
  swapChain->InitWithRenderPass(renderPass.Get());

  Ptr<Pipeline> renderPipeline;
  device->CreateRenderPipeline(nullptr, pipelineLayout.Get(), renderPass.Get(), renderPipeline.GetAddressOf());

  Ptr<CommandBuffer> commandBuffer = Ptr<CommandBuffer>(queue->CommandBuffer());
  
  Ptr<Drawable> drawable = Ptr<Drawable>(swapChain->NextDrawable());

  Ptr<RenderCommandEncoder> renderCommand = Ptr<RenderCommandEncoder>(commandBuffer->RenderCommandEncoder(drawable.Get(), renderPass.Get()));
  //renderCommand->SetViewport();
  renderCommand->EndEncode();
  //commandBuffer->Present();
  commandBuffer->Commit(fence.Get());


  return 0;
}