/*
SimpleCL SPIRV & OpenCL
*/
#include <Kaleido3D.h>
#include <ngfx.h>

#ifdef REFLECTION
enum class ArgumentAccess
{
  Write = 1,
  Read = 2,
};

enum class ArgumentType
{
  Buffer,
  Texture,
  Sampler,
  ThreadGroupMemory
};

enum DataType
{
  None,
  Struct,
  Array,
  Pointer,
  Sampler,
  Texture,
  Floatx,
};

struct StructMember
{
  char name[64];
  uint32_t offset;
  DataType dataType;
};

struct StructType
{
  StructMember members[1];
  StructMember membderByName;
};

struct PointerType
{
  enum {

  };
  uint32_t alignment;
  uint32_t dataSize;
  DataType elementType;
  bool isArgumentBuffer;
};

struct Argument
{
  char name[128];
  ArgumentAccess access;
  uint32_t index;
  ArgumentType type;
  union
  {
    struct {
      uint32_t bufferAlign;
      uint32_t dataSize;
      DataType dataType;
      // struct Type
    } buffer;
    struct {
      DataType textureType;

    };
  };
};

#endif

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
  device->CreateCommandQueue(CommandQueueType::Compute, queue.GetAddressOf());
  queue->SetName("ComputeQueue");

  Ptr<Buffer> buffer;
  BufferDesc bufferDesc{ BufferViewBit::UnOrderedAccess, StorageOption::Managed, 10 };
  device->CreateBuffer(&bufferDesc, buffer.GetAddressOf());
  buffer->SetName("Buffer0");

  Ptr<BufferView> bufferView;
  BufferViewDesc bufferViewDesc = {
    ResourceViewType::UnorderAccessBuffer, 
    ResourceState::UnorderAccess,
    10, 0
  };
  buffer->CreateView(&bufferViewDesc, bufferView.GetAddressOf());

  Ptr<Fence> fence;
  device->CreateFence(fence.GetAddressOf());
  fence->SetName("DefaultFence");

  Ptr<Compiler> compiler;
  factory->CreateCompiler(ngfx::ShaderLang::HLSL, compiler.GetAddressOf());

  Ptr<Function> function;
  ShaderOption shaderOpt = { ShaderType::Compute, ShaderLang::HLSL, "main", ShaderProfile::SM5, ShaderFormat::Text };
  compiler->Compile(&shaderOpt, nullptr, 0, function.GetAddressOf());

  Ptr<PipelineLayout> pipelineLayout;
  PipelineLayoutDesc pipelineLayoutDesc = {};
  device->CreatePipelineLayout(&pipelineLayoutDesc, pipelineLayout.GetAddressOf());

  Ptr<BindingTable> argTable;
  pipelineLayout->CreateBindingTable(argTable.GetAddressOf());

  Ptr<Pipeline> computePipeline;
  device->CreateComputePipeline(nullptr, pipelineLayout.Get(), computePipeline.GetAddressOf());

  argTable->SetBuffer(0, ShaderType::Compute, bufferView.Get());

  Ptr<CommandBuffer> commandBuffer = Ptr<CommandBuffer>(queue->CommandBuffer());
  Ptr<ComputeCommandEncoder> computeCommand = Ptr<ComputeCommandEncoder>(commandBuffer->ComputeCommandEncoder());
  computeCommand->SetPipeline(computePipeline.Get());
  computeCommand->SetPipelineLayout(pipelineLayout.Get());
  computeCommand->SetBindingTable(argTable.Get());
  computeCommand->Dispatch(1024, 1024, 1);
  computeCommand->EndEncode();
  commandBuffer->Commit(fence.Get());

  return 0;
}