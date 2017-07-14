#include <Kaleido3D.h>
#include <KTL/Allocator.hpp>
#if K3DPLATFORM_OS_WIN
#define VK_USE_PLATFORM_WIN32_KHR 1
#elif K3DPLATFORM_OS_ANDROID
#define VK_USE_PLATFORM_ANDROID_KHR 1
#endif
#include <vulkan/vulkan.h>
#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"
#include "ngfx.h"
#include "vulkan_glslang.h"
#include <vector>
#include <set>

using namespace ngfx;

#define VULKAN_ALLOCATOR nullptr

static const char* RequiredLayers[] = 
{ 
  "VK_LAYER_LUNARG_standard_validation" 
};

static std::vector<const char*> RequiredInstanceExtensions =
{
  VK_KHR_SURFACE_EXTENSION_NAME,
  VK_KHR_WIN32_SURFACE_EXTENSION_NAME
};

static std::vector<const char *> RequiredDeviceExtensions = 
{ 
  VK_KHR_SWAPCHAIN_EXTENSION_NAME 
};

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

const char * VulkanError(VkResult Result)
{
#define ERROR_STR(X) case VK_##X: return #X;
  switch(Result)
  {
    ERROR_STR(SUCCESS)
    ERROR_STR(NOT_READY)
    ERROR_STR(TIMEOUT)
    ERROR_STR(EVENT_SET)
    ERROR_STR(EVENT_RESET)
    ERROR_STR(INCOMPLETE)
    ERROR_STR(ERROR_OUT_OF_HOST_MEMORY)
    ERROR_STR(ERROR_OUT_OF_DEVICE_MEMORY)
    ERROR_STR(ERROR_INITIALIZATION_FAILED)
    ERROR_STR(ERROR_DEVICE_LOST)
    ERROR_STR(ERROR_LAYER_NOT_PRESENT)
    ERROR_STR(ERROR_EXTENSION_NOT_PRESENT)
    ERROR_STR(ERROR_MEMORY_MAP_FAILED)
    ERROR_STR(ERROR_FEATURE_NOT_PRESENT)
    ERROR_STR(ERROR_INCOMPATIBLE_DRIVER)
    ERROR_STR(ERROR_TOO_MANY_OBJECTS)
    ERROR_STR(ERROR_FORMAT_NOT_SUPPORTED)
    ERROR_STR(ERROR_FRAGMENTED_POOL)
    ERROR_STR(ERROR_SURFACE_LOST_KHR)
    ERROR_STR(ERROR_NATIVE_WINDOW_IN_USE_KHR)
    ERROR_STR(ERROR_OUT_OF_DATE_KHR)
    ERROR_STR(ERROR_INCOMPATIBLE_DISPLAY_KHR)
    ERROR_STR(ERROR_VALIDATION_FAILED_EXT)
    ERROR_STR(ERROR_OUT_OF_POOL_MEMORY_KHR)
    ERROR_STR(ERROR_INVALID_EXTERNAL_HANDLE_KHX)
  }
  return "Unknown";
#undef ERROR_STR
}

const char* DeviceType(VkPhysicalDeviceType DType)
{
  switch (DType)
  {
  case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
    return "Discrete GPU";
  case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
    return "Integrated GPU";
  case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
    return "Virtual GPU";
  case VK_PHYSICAL_DEVICE_TYPE_CPU:
    return "CPU";
  case VK_PHYSICAL_DEVICE_TYPE_OTHER:
  default:
    return "Unknown";
  }
  
}

#define CHECK(Ret) \
  if (Ret != VK_SUCCESS) \
  {\
    LogPrint(Log::Error, "CheckRet", "%s !!\n\tReturn Error: %s @line: %d @file: %s.\n", #Ret, VulkanError(Ret), __LINE__, __FILE__); \
  }

VkFormat ConvertPixelFormatToVulkanEnum(PixelFormat const& e) {
  switch (e) {
  case PixelFormat::RGBA16Uint:
    return VK_FORMAT_R16G16B16A16_UINT;
  case PixelFormat::RGBA32Float:
    return VK_FORMAT_R32G32B32A32_SFLOAT;
  case PixelFormat::RGBA8UNorm:
    return VK_FORMAT_R8G8B8A8_UNORM;
  case PixelFormat::RGBA8UNorm_sRGB:
    return VK_FORMAT_R8G8B8A8_SNORM;
  case PixelFormat::R11G11B10Float:
    return VK_FORMAT_B10G11R11_UFLOAT_PACK32;
  case PixelFormat::D32Float:
    return VK_FORMAT_D32_SFLOAT;
  case PixelFormat::RGB32Float:
    return VK_FORMAT_R32G32B32_SFLOAT;
  }
}
VkAttachmentLoadOp ConvertLoadActionToVulkanEnum(LoadAction const& e) {
  switch (e) {
  case LoadAction::Load:
    return VK_ATTACHMENT_LOAD_OP_LOAD;
  case LoadAction::Clear:
    return VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  case LoadAction::DontCare:
    return VK_ATTACHMENT_LOAD_OP_CLEAR;
  }
}
VkAttachmentStoreOp ConvertStoreActionToVulkanEnum(StoreAction const& e) {
  switch (e) {
  case StoreAction::Store:
    return VK_ATTACHMENT_STORE_OP_STORE;
  case StoreAction::DontCare:
    return VK_ATTACHMENT_STORE_OP_DONT_CARE;
  }
}
VkPrimitiveTopology ConvertPrimitiveTypeToVulkanEnum(PrimitiveType const& e) {
  switch (e) {
  case PrimitiveType::Points:
    return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
  case PrimitiveType::Lines:
    return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
  case PrimitiveType::Triangles:
    return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
  case PrimitiveType::TriangleStrips:
    return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
  }
}
VkBlendOp ConvertBlendOperationToVulkanEnum(BlendOperation const& e) {
  switch (e) {
  case BlendOperation::Add:
    return VK_BLEND_OP_ADD;
  case BlendOperation::Sub:
    return VK_BLEND_OP_SUBTRACT;
  }
}
VkBlendFactor ConvertBlendTypeToVulkanEnum(BlendType const& e) {
  switch (e) {
  case BlendType::Zero:
    return VK_BLEND_FACTOR_ZERO;
  case BlendType::One:
    return VK_BLEND_FACTOR_ONE;
  case BlendType::SrcColor:
    return VK_BLEND_FACTOR_SRC_COLOR;
  case BlendType::DstColor:
    return VK_BLEND_FACTOR_DST_COLOR;
  case BlendType::SrcAlpha:
    return VK_BLEND_FACTOR_SRC_ALPHA;
  case BlendType::DstAlpha:
    return VK_BLEND_FACTOR_DST_ALPHA;
  }
}
VkStencilOp ConvertStencilOperationToVulkanEnum(StencilOperation const& e) {
  switch (e) {
  case StencilOperation::Keep:
    return VK_STENCIL_OP_KEEP;
  case StencilOperation::Zero:
    return VK_STENCIL_OP_ZERO;
  case StencilOperation::Replace:
    return VK_STENCIL_OP_REPLACE;
  case StencilOperation::Invert:
    return VK_STENCIL_OP_INVERT;
  case StencilOperation::Increment:
    return VK_STENCIL_OP_INCREMENT_AND_CLAMP;
  case StencilOperation::Decrement:
    return VK_STENCIL_OP_DECREMENT_AND_CLAMP;
  }
}
VkCompareOp ConvertComparisonFunctionToVulkanEnum(ComparisonFunction const& e) {
  switch (e) {
  case ComparisonFunction::Never:
    return VK_COMPARE_OP_NEVER;
  case ComparisonFunction::Less:
    return VK_COMPARE_OP_LESS;
  case ComparisonFunction::Equal:
    return VK_COMPARE_OP_EQUAL;
  case ComparisonFunction::LessEqual:
    return VK_COMPARE_OP_LESS_OR_EQUAL;
  case ComparisonFunction::Greater:
    return VK_COMPARE_OP_GREATER;
  case ComparisonFunction::NotEqual:
    return VK_COMPARE_OP_NOT_EQUAL;
  case ComparisonFunction::GreaterEqual:
    return VK_COMPARE_OP_GREATER_OR_EQUAL;
  case ComparisonFunction::Always:
    return VK_COMPARE_OP_ALWAYS;
  }
}
VkPolygonMode ConvertFillModeToVulkanEnum(FillMode const& e) {
  switch (e) {
  case FillMode::Wire:
    return VK_POLYGON_MODE_LINE;
  case FillMode::Solid:
    return VK_POLYGON_MODE_FILL;
  }
}
VkCullModeFlagBits ConvertCullModeToVulkanEnum(CullMode const& e) {
  switch (e) {
  case CullMode::None:
    return VK_CULL_MODE_NONE;
  case CullMode::Front:
    return VK_CULL_MODE_FRONT_BIT;
  case CullMode::Back:
    return VK_CULL_MODE_BACK_BIT;
  }
}
VkFilter ConvertFilterModeToVulkanEnum(FilterMode const& e) {
  switch (e) {
  case FilterMode::Point:
    return VK_FILTER_NEAREST;
  case FilterMode::Linear:
    return VK_FILTER_LINEAR;
  }
}
VkSamplerAddressMode ConvertAddressModeToVulkanEnum(AddressMode const& e) {
  switch (e) {
  case AddressMode::Wrap:
    return VK_SAMPLER_ADDRESS_MODE_REPEAT;
  case AddressMode::Mirror:
    return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
  case AddressMode::Clamp:
    return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
  case AddressMode::Border:
    return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
  case AddressMode::MirrorOnce:
    return VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE;
  }
}
VkSampleCountFlagBits ConvertMultiSampleFlagToVulkanEnum(MultiSampleFlag const& e) {
  switch (e) {
  case MultiSampleFlag::MS1x:
    return VK_SAMPLE_COUNT_1_BIT;
  case MultiSampleFlag::MS2x:
    return VK_SAMPLE_COUNT_2_BIT;
  case MultiSampleFlag::MS4x:
    return VK_SAMPLE_COUNT_4_BIT;
  case MultiSampleFlag::MS8x:
    return VK_SAMPLE_COUNT_8_BIT;
  case MultiSampleFlag::MS16x:
    return VK_SAMPLE_COUNT_16_BIT;
  }
}

/*
template <class TNgfxObj>
class TDeviceChild : public TNgfxObj
{
public:
};
*/

class VulkanCommandBuffer : public CommandBuffer
{
protected:
  class VulkanQueue* OwningRoot;
public:
  VkCommandBuffer Handle = VK_NULL_HANDLE;
  void Commit(Fence * pFence) override;
  struct RenderCommandEncoder * RenderCommandEncoder(Drawable * pDrawable, RenderPass * pRenderPass) override;
  struct ComputeCommandEncoder * ComputeCommandEncoder() override;
  struct ParallelRenderCommandEncoder * ParallelCommandEncoder() override;
  struct CopyCommandEncoder * CopyCommandEncoder() override;
  VulkanCommandBuffer(VulkanQueue* pQueue);
  ~VulkanCommandBuffer();
};

template <class T>
class TCmdEncoder : public T
{
public:
  using Super = TCmdEncoder<T>;

  TCmdEncoder(VulkanCommandBuffer* pCmd);
  virtual ~TCmdEncoder();

  void Barrier(Resource * pResource) override;
  void SetPipeline(Pipeline* pPipelineState) override;
  void SetPipelineLayout(PipelineLayout * pPipelineLayout) override;
  void SetBindingTable(BindingTable * pBindingTable) override;
  virtual void EndEncode() override;

  VulkanCommandBuffer* OwningCommand = nullptr;
};

class VulkanCopyEncoder : public TCmdEncoder<CopyCommandEncoder>
{
public:
  VulkanCopyEncoder(VulkanCommandBuffer * pCmd);
  ~VulkanCopyEncoder();
  void CopyTexture() override;
  void CopyBuffer(uint64_t srcOffset, uint64_t dstOffset, uint64_t size, Buffer * srcBuffer, Buffer * dstBuffer) override;
};

class VulkanComputeEncoder : public TCmdEncoder<ComputeCommandEncoder>
{
public:
  VulkanComputeEncoder(VulkanCommandBuffer* pCmd);
  ~VulkanComputeEncoder();
  void Dispatch(uint32_t x, uint32_t y, uint32_t z) override;
};

class VulkanRenderEncoder : public TCmdEncoder<RenderCommandEncoder>
{
public:
  VulkanRenderEncoder(VulkanCommandBuffer* pCmd);
  ~VulkanRenderEncoder();

  void SetScissorRect(uint32_t x, uint32_t y, uint32_t w, uint32_t h);
  void SetViewport(const Viewport * pViewport);
  void SetIndexBuffer(Buffer * pIndexBuffer);
  void SetVertexBuffer(uint32_t slot, uint64_t offset, Buffer * pVertexBuffer);
  void SetPrimitiveType(PrimitiveType primitive);
  void DrawInstanced(const DrawInstancedDesc * drawParam);
  void DrawIndexedInstanced(const DrawIndexedInstancedDesc * drawParam);
  void Present(Drawable * pDrawable);
};

class VulkanQueue : public CommandQueue
{
public:
  VulkanQueue(class VulkanDevice* pDevice);
  ~VulkanQueue() override;
  struct CommandBuffer * CommandBuffer() override;
  VulkanDevice* OwningRoot;
  bool IsSupport(CommandQueueType const&type) const;
  VkQueue Handle = VK_NULL_HANDLE;
  uint32_t FamilyId = 0;
  uint32_t QueueId = 0;
};

class VulkanRenderPass : public RenderPass
{
protected:
  VulkanDevice* OwningRoot;
private:
  VkRenderPass Handle = VK_NULL_HANDLE;
};

template <class T>
class TPipeline : public T
{
public:
  using Super = typename TPipeline<T>;

  TPipeline(VulkanDevice * pDevice);
  virtual ~TPipeline() override;

protected:
  VulkanDevice* OwningDevice = nullptr;

  VkPipeline Handle = VK_NULL_HANDLE;
  VkPipelineCache Cache = VK_NULL_HANDLE;

private:
};

class VulkanRenderPipeline : public TPipeline<RenderPipeline>
{
public:
  VulkanRenderPipeline(VulkanDevice* pDevice, const RenderPipelineDesc* pDesc, RenderPass * pRenderPass, PipelineLayout * pLayout);
  ~VulkanRenderPipeline() override;
  Result GetDesc(RenderPipelineDesc * pDesc);
protected:
private:
};

class VulkanComputePipeline : public TPipeline<ComputePipeline>
{
public:
  VulkanComputePipeline(VulkanDevice* pDevice, Function* pComputeFunc, PipelineLayout * pLayout);
  ~VulkanComputePipeline();
};

class VulkanDrawable : public Drawable
{
public:
  VulkanDrawable();
  ~VulkanDrawable() override;

  struct Texture * Texture() override;

  VkFramebuffer Handle = VK_NULL_HANDLE;
  class VulkanTextureView* DepthStencilView = nullptr;
  VulkanTextureView* MainColorView = nullptr;
  std::vector<VulkanTextureView*> OtherAttachments;
};

RenderCommandEncoder* VulkanCommandBuffer::RenderCommandEncoder(Drawable* pDrawable, RenderPass* pRenderPass)
{
  return OwningRoot->IsSupport(CommandQueueType::Graphics)? 
    new VulkanRenderEncoder(this) : nullptr;
}

ComputeCommandEncoder * VulkanCommandBuffer::ComputeCommandEncoder()
{
  return OwningRoot->IsSupport(CommandQueueType::Compute) ?
    new VulkanComputeEncoder(this) : nullptr;
}

ParallelRenderCommandEncoder * VulkanCommandBuffer::ParallelCommandEncoder()
{
  return nullptr;
}

CopyCommandEncoder * VulkanCommandBuffer::CopyCommandEncoder()
{
  return OwningRoot->IsSupport(CommandQueueType::Copy) ?
    new VulkanCopyEncoder(this) : nullptr;
}

VulkanCommandBuffer::VulkanCommandBuffer(VulkanQueue* pQueue)
  : OwningRoot(pQueue)
{
  OwningRoot->AddInternalRef();

}

VulkanCommandBuffer::~VulkanCommandBuffer()
{

  OwningRoot->ReleaseInternal();
}

class VulkanSampler : public Sampler
{
protected:
  VulkanDevice* OwningDevice;
private:
  VkSampler Handle = VK_NULL_HANDLE;
  VkSamplerCreateInfo Info = { VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO };
public:
  VulkanSampler(VulkanDevice* pDevice, const SamplerDesc* pDesc);
  ~VulkanSampler();
  Result GetDesc(SamplerDesc * desc) override;
};

template<class RHIObj>
struct ResTrait
{
};

template<>
struct ResTrait<Buffer>
{
  typedef VkBuffer Obj;
  typedef VkBufferView View;
  typedef VkBufferUsageFlags UsageFlags;
  typedef VkBufferCreateInfo CreateInfo;
  typedef VkBufferViewCreateInfo ViewCreateInfo;
  typedef VkDescriptorBufferInfo DescriptorInfo;
  static decltype(vkCreateBufferView)* CreateView;
  static decltype(vkDestroyBufferView)* DestroyView;
  static decltype(vkCreateBuffer)* Create;
  static decltype(vmaCreateBuffer)* vmaCreate;
  static decltype(vkDestroyBuffer)* Destroy;
  static decltype(vmaDestroyBuffer)* vmaDestroy;
  static decltype(vkGetBufferMemoryRequirements)* GetMemoryInfo;
  static decltype(vkBindBufferMemory)* BindMemory;
};

template<>
struct ResTrait<Texture>
{
  typedef VkImage Obj;
  typedef VkImageView View;
  typedef VkImageUsageFlags UsageFlags;
  typedef VkImageCreateInfo CreateInfo;
  typedef VkImageViewCreateInfo ViewCreateInfo;
  typedef VkDescriptorImageInfo DescriptorInfo;
  static decltype(vkCreateImageView)* CreateView;
  static decltype(vkDestroyImageView)* DestroyView;
  static decltype(vkCreateImage)* Create;
  static decltype(vmaCreateImage)* vmaCreate;
  static decltype(vkDestroyImage)* Destroy;
  static decltype(vmaDestroyImage)* vmaDestroy;
  static decltype(vkGetImageMemoryRequirements)* GetMemoryInfo;
  static decltype(vkBindImageMemory)* BindMemory;
};

// Buffer functors
decltype(vkCreateBufferView)* ResTrait<Buffer>::CreateView =
&vkCreateBufferView;
decltype(vkDestroyBufferView)* ResTrait<Buffer>::DestroyView =
&vkDestroyBufferView;
decltype(vkCreateBuffer)* ResTrait<Buffer>::Create = &vkCreateBuffer;
decltype(vmaCreateBuffer)* ResTrait<Buffer>::vmaCreate = &vmaCreateBuffer;
decltype(vkDestroyBuffer)* ResTrait<Buffer>::Destroy = &vkDestroyBuffer;
decltype(vmaDestroyBuffer)* ResTrait<Buffer>::vmaDestroy = &vmaDestroyBuffer;
decltype(vkGetBufferMemoryRequirements)* ResTrait<Buffer>::GetMemoryInfo =
&vkGetBufferMemoryRequirements;
decltype(vkBindBufferMemory)* ResTrait<Buffer>::BindMemory =
&vkBindBufferMemory;
// Texture
decltype(vkCreateImageView)* ResTrait<Texture>::CreateView = &vkCreateImageView;
decltype(vkDestroyImageView)* ResTrait<Texture>::DestroyView =
&vkDestroyImageView;
decltype(vkCreateImage)* ResTrait<Texture>::Create = &vkCreateImage;
decltype(vmaCreateImage)* ResTrait<Texture>::vmaCreate = &vmaCreateImage;
decltype(vkDestroyImage)* ResTrait<Texture>::Destroy = &vkDestroyImage;
decltype(vmaDestroyImage)* ResTrait<Texture>::vmaDestroy = &vmaDestroyImage;
decltype(vkGetImageMemoryRequirements)* ResTrait<Texture>::GetMemoryInfo =
&vkGetImageMemoryRequirements;
decltype(vkBindImageMemory)* ResTrait<Texture>::BindMemory = &vkBindImageMemory;


template<class TRHIResObj>
class TResource : public TRHIResObj
{
  friend class CommandBufferImpl;
public:
  using TObj = typename ResTrait<TRHIResObj>::Obj;
  using ResInfo = typename ResTrait<TRHIResObj>::CreateInfo;
  using Super = TResource<TRHIResObj>;

  TResource(VulkanDevice* Device) : OwningDevice(Device) {}
  virtual ~TResource();

  void * Map(uint64_t offset, uint64_t size) override;
  void UnMap() override;

  Result Create(ResInfo const& Info, StorageOption const& Option);
  TObj GetHandle() const { return Handle; }

  VulkanDevice* OwningDevice;

protected:
  VkMappedMemoryRange MappedMemoryRange = {};
  
  ResInfo Info = {};
  VmaMemoryRequirements MemReq = {};
  TObj Handle = VK_NULL_HANDLE;
};

class VulkanBuffer : public TResource<Buffer>
{
public:
  VulkanBuffer(VulkanDevice * pDevice, const BufferDesc* pDesc);
  ~VulkanBuffer();
  Result GetDesc(BufferDesc * pDesc);
  Result CreateView(const BufferViewDesc * pDesc, BufferView ** ppView) override;
};

class VulkanTexture : public TResource<Texture>
{
public:
  VulkanTexture(VulkanDevice* pDevice, const TextureDesc* pDesc);
  ~VulkanTexture();
  Result GetDesc(TextureDesc * pDesc) override;
  Result CreateView(const TextureViewDesc * pDesc, TextureView ** ppView) override;
};

class VulkanPipelineLayout : public PipelineLayout
{
public:
  VulkanPipelineLayout(VulkanDevice* pDevice, const PipelineLayoutDesc * pDesc);
  ~VulkanPipelineLayout() override;
  Result CreateBindingTable(BindingTable ** ppBindingTable) override;

  VkPipelineLayout Handle = VK_NULL_HANDLE;
  VulkanDevice* OwningDevice = nullptr;

  friend class VulkanDevice;
};

class VulkanShaderLayout : public ShaderLayout
{
public:
  VulkanDevice* OwningDevice = nullptr;
  VkDescriptorSetLayout Handle = VK_NULL_HANDLE;
  VkDescriptorSetLayoutCreateInfo Info = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };

  VulkanShaderLayout(VulkanDevice* pDevice, const ShaderLayoutDesc* pDesc);
  ~VulkanShaderLayout() override;
};

class VulkanBindingTable : public BindingTable
{
public:
  VulkanBindingTable(VulkanPipelineLayout* pLayout);
  ~VulkanBindingTable() override;

  void SetSampler(uint32_t index, ShaderType shaderVis, Sampler * pSampler) override;
  void SetBuffer(uint32_t index, ShaderType shaderVis, BufferView * bufferView) override;
  void SetTexture(uint32_t index, ShaderType shaderVis, TextureView * textureView) override;

  VkDescriptorSet Handle = VK_NULL_HANDLE;
  VulkanPipelineLayout* OwningLayout;

  friend class VulkanPipelineLayout;
};

class VulkanFence : public Fence
{
public:
  VulkanFence(VulkanDevice* pDevice);
  ~VulkanFence() override;
  void Wait() override;
  void Reset() override;
  VulkanDevice* OwningDevice = nullptr;
  VkFence Handle = VK_NULL_HANDLE;
};

namespace ngfx
{
  class VulkanFactory;
}

class VulkanDevice : public Device
{
public:
  VulkanDevice(class VulkanFactory* pFactory, VkPhysicalDevice PhysicalDevice);
  ~VulkanDevice() override;
  void GetDesc(DeviceDesc * pDesc) override;
  Result CreateCommandQueue(CommandQueueType queueType, CommandQueue ** pQueue) override;
  Result CreateShaderLayout(const ShaderLayoutDesc * pShaderLayoutDesc, ShaderLayout ** ppShaderLayout) override;
  Result CreatePipelineLayout(const PipelineLayoutDesc * pPipelineLayoutDesc, PipelineLayout ** ppPipelineLayout) override;
  Result CreateBindingTable(PipelineLayout * pPipelineLayout, BindingTable ** ppBindingTable) override;
  Result CreateRenderPipeline(const RenderPipelineDesc * pPipelineDesc, PipelineLayout * pPipelineLayout, RenderPass * pRenderPass, Pipeline ** pPipelineState) override;
  Result CreateComputePipeline(Function * pComputeFunction, PipelineLayout * pPipelineLayout, Pipeline ** pPipeline) override;
  Result CreateRenderPass(const RenderPassDesc * desc, RenderPass ** ppRenderpass) override;
  Result CreateRenderTarget(const RenderTargetDesc * desc, RenderTarget ** ppRenderTarget) override;
  Result CreateSampler(const SamplerDesc* desc, Sampler ** pSampler) override;
  Result CreateBuffer(const BufferDesc* desc, Buffer ** pBuffer) override;
  Result CreateTexture(const TextureDesc * desc, Texture ** pTexture) override;
  Result CreateFence(Fence ** ppFence) override;
  void WaitIdle() override;

  VkDevice Handle = VK_NULL_HANDLE;

private:

  struct QueueInfo
  {
    uint32_t Flags = 0;
    uint32_t Family = 0;
    uint32_t Count = 0;
  };

  VkPhysicalDevice Device = VK_NULL_HANDLE;
  VmaAllocator MemoryAllocator = nullptr;
  std::vector<QueueInfo> QueueInfos;

protected:
  VulkanFactory* OwningRoot;
  friend class VulkanFactory;
  friend class VulkanQueue;
  template<class T>
  friend class TResource;
};

VulkanQueue::VulkanQueue(VulkanDevice* pDevice)
  : OwningRoot(pDevice)
{
  OwningRoot->AddInternalRef();
  vkGetDeviceQueue(OwningRoot->Handle, 0, 0, &Handle);
}

VulkanQueue::~VulkanQueue()
{
  OwningRoot->ReleaseInternal();
}

CommandBuffer * VulkanQueue::CommandBuffer()
{
  
  return nullptr;
}

bool VulkanQueue::IsSupport(CommandQueueType const& type) const
{
  return false;
}

template<class TRHIResObj>
void * TResource<TRHIResObj>::Map(uint64_t offset, uint64_t size)
{
  MappedMemoryRange.offset = offset;
  MappedMemoryRange.size = size;
  void * pData = nullptr;
  vmaMapMemory(OwningDevice->MemoryAllocator, &MappedMemoryRange, &pData);
  return pData;
}

template<class TRHIResObj>
void TResource<TRHIResObj>::UnMap()
{
  vmaUnmapMemory(OwningDevice->MemoryAllocator, &MappedMemoryRange);
}

template<class TRHIResObj>
TResource<TRHIResObj>::~TResource()
{
  ResTrait<TRHIResObj>::Destroy(OwningDevice->Handle, Handle, VULKAN_ALLOCATOR);
}

template<class TRHIResObj>
Result TResource<TRHIResObj>::Create(ResInfo const & _Info, StorageOption const & Option)
{
  switch (Option)
  {
  case StorageOption::Shared:
    MemReq.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
    break;
  case StorageOption::Private:
    MemReq.usage = VMA_MEMORY_USAGE_GPU_ONLY;
    break;
  case StorageOption::Managed:
    MemReq.usage = VMA_MEMORY_USAGE_CPU_ONLY;
    break;
  }
  CHECK(ResTrait<TRHIResObj>::vmaCreate(OwningDevice->MemoryAllocator,
    &Info, &MemReq,
    &Handle, &MappedMemoryRange,
    VULKAN_ALLOCATOR));
  return Result::Ok;
}

VulkanBuffer::VulkanBuffer(VulkanDevice * pDevice, const BufferDesc* pDesc)
  : Super(pDevice)
{
  Info = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
  Info.size = pDesc->size;
  if ((uint32_t)pDesc->allowedViewBits & (uint32_t)BufferViewBit::UnOrderedAccess)
  {
    Info.usage |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
  }
  if ((uint32_t)pDesc->allowedViewBits & (uint32_t)BufferViewBit::VertexBuffer)
  {
    Info.usage |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
  }
  if ((uint32_t)pDesc->allowedViewBits & (uint32_t)BufferViewBit::ConstantBuffer)
  {
    Info.usage |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
  }
  if ((uint32_t)pDesc->option & (uint32)StorageOption::Private)
  {
    Info.usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
  }
  Info.flags = 0;
  Info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  Super::Create(Info, pDesc->option);
}

VulkanBuffer::~VulkanBuffer()
{

}

Result VulkanBuffer::GetDesc(BufferDesc * pDesc)
{
  return Result();
}

Result VulkanBuffer::CreateView(const BufferViewDesc * pDesc, BufferView ** ppView)
{
  return Result();
}

VulkanTexture::VulkanTexture(VulkanDevice * pDevice, const TextureDesc * pDesc)
  : Super(pDevice)
{
  Info = { VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
  Info.flags = 0;
  // extents
  Info.format = ConvertPixelFormatToVulkanEnum(pDesc->format);
  Info.extent = { pDesc->width, pDesc->height, pDesc->depth };
  Info.arrayLayers = pDesc->layers;
  Info.mipLevels = pDesc->mipLevels;

  if (pDesc->width > 1)
  {
    Info.imageType = VK_IMAGE_TYPE_1D;
  }
  if (pDesc->height > 1)
  {
    Info.imageType = VK_IMAGE_TYPE_2D;
  }
  if(pDesc->depth > 1) 
  {
    Info.imageType = VK_IMAGE_TYPE_3D;
  }

  Info.samples = ConvertMultiSampleFlagToVulkanEnum(pDesc->samples);

  TextureViewBit usage = pDesc->allowedViewBits;
  if (((uint32_t)usage & (uint32_t)TextureViewBit::RenderTarget))
  {
    Info.usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
  }
  if (((uint32_t)usage & (uint32_t)TextureViewBit::DepthStencil))
  {
    Info.usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
  }
  if (((uint32_t)usage & (uint32_t)TextureViewBit::DepthStencil))
  {
    Info.usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
  }
  if (((uint32_t)usage & (uint32_t)TextureViewBit::ShaderRead))
  {
    Info.usage |= VK_IMAGE_USAGE_SAMPLED_BIT;
  }
  if (((uint32_t)usage & (uint32_t)TextureViewBit::ShaderWrite))
  {
    Info.usage |= VK_IMAGE_USAGE_STORAGE_BIT;
  }
  
  Info.initialLayout;

  Info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

  Super::Create(Info, pDesc->option);
}

VulkanTexture::~VulkanTexture()
{
}

class VulkanSwapChain : public SwapChain
{
public:
  void InitWithRenderPass(RenderPass * pRenderPass) override;
  Result GetTexture(Texture ** ppTexture, uint32_t index) override;
  Drawable * CurrentDrawable() override;
  Drawable * NextDrawable() override;
  uint32_t BufferCount() override;
  VulkanSwapChain(VulkanFactory* pFactory, void* pHandle, const SwapChainDesc* pDesc, VulkanQueue* pQueue);
  ~VulkanSwapChain() override;
private:
  VkSwapchainKHR Handle = VK_NULL_HANDLE;
  VkSurfaceKHR Surface = VK_NULL_HANDLE;
  VkSwapchainCreateInfoKHR CreateInfo;
protected:
  VulkanFactory* OwningRoot;
  VulkanDevice* OwningDevice;
};

namespace ngfx {

class VulkanFactory : public Factory
{
public:
  bool Debug = false;

  Result EnumDevice(uint32_t * count, Device ** ppDevice);
  Result CreateSwapchain(const SwapChainDesc * desc, CommandQueue * pCommandQueue, void * pWindow, SwapChain ** pSwapchain);
  Result CreateCompiler(ShaderLang shaderLang, Compiler ** compiler);
  
  friend NGFX_API Result CreateFactory(Factory ** ppFactory, bool debugEnabled)
  {
    *ppFactory = new VulkanFactory(debugEnabled);
    return Result::Ok;
  }

  VulkanFactory(bool debug) : Debug(debug)
  {
    uint32_t layerCount = 0;
    std::vector<VkLayerProperties> layers;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
    layers.resize(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, layers.data());

    uint32_t layerExtPropCount = 0;
    std::vector<VkExtensionProperties> extProps;
    vkEnumerateInstanceExtensionProperties(nullptr, &layerExtPropCount, nullptr);
    extProps.resize(layerExtPropCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &layerExtPropCount, extProps.data());

    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "test";
    appInfo.pEngineName = "test";
    appInfo.apiVersion = VK_MAKE_VERSION(1, 0, 42);
    appInfo.engineVersion = 1;
    appInfo.applicationVersion = 0;

    VkInstanceCreateInfo instanceCreateInfo = {};
    instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceCreateInfo.pNext = NULL;
    instanceCreateInfo.pApplicationInfo = &appInfo;

    if(Debug) // Debug extension ??
    {
      instanceCreateInfo.enabledLayerCount = 1;
      instanceCreateInfo.ppEnabledLayerNames = RequiredLayers;
    }
    
    instanceCreateInfo.enabledExtensionCount = RequiredInstanceExtensions.size();
    instanceCreateInfo.ppEnabledExtensionNames = RequiredInstanceExtensions.data();
    CHECK(vkCreateInstance(&instanceCreateInfo, VULKAN_ALLOCATOR, &Handle));
  }
  ~VulkanFactory() override
  {
    vkDestroyInstance(Handle, VULKAN_ALLOCATOR);
  }
private:
  friend class VulkanSwapChain;
  VkInstance Handle = VK_NULL_HANDLE;
};

}


VulkanDevice::VulkanDevice(VulkanFactory* pFactory, VkPhysicalDevice PhysicalDevice)
: OwningRoot(pFactory)
, Device(PhysicalDevice)
{
  OwningRoot->AddInternalRef();

  uint32 queueCount = 0;
  std::vector<VkQueueFamilyProperties> QueueFamilyProps;
  vkGetPhysicalDeviceQueueFamilyProperties(PhysicalDevice, &queueCount, NULL);
  QueueFamilyProps.resize(queueCount);
  vkGetPhysicalDeviceQueueFamilyProperties(PhysicalDevice, &queueCount, QueueFamilyProps.data());
  QueueInfos.resize(queueCount);
  VkPhysicalDeviceProperties Prop;
  vkGetPhysicalDeviceProperties(Device, &Prop);

  LogPrint(Log::Info, "Device", "Vendor: %s Type: %s API Version: %d.%d.%d\n",
    Prop.deviceName,
    DeviceType(Prop.deviceType),
    VK_VERSION_MAJOR(Prop.apiVersion),
    VK_VERSION_MINOR(Prop.apiVersion), 
    VK_VERSION_PATCH(Prop.apiVersion));


  // Create Command Queue
  float QueuePriority = 0.0f;
  std::vector<VkDeviceQueueCreateInfo> DeviceQueueInfo;
  DeviceQueueInfo.resize(queueCount);
  std::vector<float*> QueuePriorities;
  // Async Compute & Transfer
  for (uint32 Id = 0; Id < queueCount; Id++)
  {
    QueueInfos[Id].Family = Id;
    QueueInfos[Id].Flags = QueueFamilyProps[Id].queueFlags;
    QueueInfos[Id].Count = QueueFamilyProps[Id].queueCount;
    DeviceQueueInfo[Id].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    DeviceQueueInfo[Id].pNext = nullptr;
    DeviceQueueInfo[Id].flags = 0;
    DeviceQueueInfo[Id].queueFamilyIndex = Id;
    DeviceQueueInfo[Id].queueCount = QueueFamilyProps[Id].queueCount;
    float * Priorities = new float[QueueFamilyProps[Id].queueCount]{ 0 };
    DeviceQueueInfo[Id].pQueuePriorities = Priorities;
    QueuePriorities.push_back(Priorities);
  }

  uint32_t extCount = 0;
  std::vector<VkExtensionProperties> exts;
  vkEnumerateDeviceExtensionProperties(Device, nullptr, &extCount, nullptr);
  exts.resize(extCount);
  vkEnumerateDeviceExtensionProperties(Device, nullptr, &extCount, exts.data());
  std::set<std::string> deviceExtensions;
  for (auto ext : exts)
  {
    deviceExtensions.insert(ext.extensionName);
  }

  uint32_t layerCount = 0;
  std::vector<VkLayerProperties> layers;
  vkEnumerateDeviceLayerProperties(Device, &layerCount, nullptr);
  layers.resize(layerCount);
  vkEnumerateDeviceLayerProperties(Device, &layerCount, layers.data());

  // KHX ?
  VkDeviceCreateInfo deviceCreateInfo = {};
  deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  deviceCreateInfo.pNext = NULL;
  deviceCreateInfo.queueCreateInfoCount = DeviceQueueInfo.size();
  deviceCreateInfo.pQueueCreateInfos = DeviceQueueInfo.data();
  //deviceCreateInfo.pEnabledFeatures = &m_Features;
  if (OwningRoot->Debug)
  {
    if (deviceExtensions.find(VK_EXT_DEBUG_MARKER_EXTENSION_NAME) != deviceExtensions.end())
    {
      RequiredDeviceExtensions.push_back(VK_EXT_DEBUG_MARKER_EXTENSION_NAME);
    }
    deviceCreateInfo.enabledLayerCount = 1;
    deviceCreateInfo.ppEnabledLayerNames = RequiredLayers;
  }
  
  if (RequiredDeviceExtensions.size() > 0)
  {
    deviceCreateInfo.enabledExtensionCount = (uint32_t)RequiredDeviceExtensions.size();
    deviceCreateInfo.ppEnabledExtensionNames = RequiredDeviceExtensions.data();
  }
  CHECK(vkCreateDevice(Device, &deviceCreateInfo, VULKAN_ALLOCATOR, &Handle));

  for(auto ptr : QueuePriorities)
  {
    delete[]ptr;
  }

  VmaAllocatorCreateInfo AllocCreateInfo = {
    Device, Handle, 0, 0,
    VULKAN_ALLOCATOR
  };
  vmaCreateAllocator(&AllocCreateInfo, &MemoryAllocator);
}

VulkanDevice::~VulkanDevice()
{
  vmaDestroyAllocator(MemoryAllocator);
  vkDestroyDevice(Handle, VULKAN_ALLOCATOR);
  OwningRoot->ReleaseInternal();
}

void VulkanDevice::GetDesc(DeviceDesc * pDesc)
{
  VkDebugMarkerObjectNameInfoEXT ext;
  //vkDebugMarkerSetObjectNameEXT(Handle, &ext);
}

Result VulkanDevice::CreateCommandQueue(CommandQueueType queueType, CommandQueue ** pQueue)
{
  VulkanQueue* pVkQueue = new VulkanQueue(this);
  switch (queueType)
  {
  case CommandQueueType::Graphics:
  {
    for(auto Info : QueueInfos)
    {
      if(Info.Flags & VK_QUEUE_GRAPHICS_BIT)
      {
        uint32_t qId = 0;
        vkGetDeviceQueue(Handle, Info.Family, qId, &pVkQueue->Handle);
        pVkQueue->FamilyId = Info.Family;
        pVkQueue->QueueId = qId;
        break;
      }
    }
    break;
  }
  case CommandQueueType::Compute:
  {
    for (auto Info : QueueInfos)
    {
      if (Info.Flags & VK_QUEUE_COMPUTE_BIT)
      {
        uint32_t qId = 0;
        vkGetDeviceQueue(Handle, Info.Family, 0, &pVkQueue->Handle);
        pVkQueue->FamilyId = Info.Family;
        pVkQueue->QueueId = 0;
      }
    }
    break;
  }
  case CommandQueueType::Copy:
  {
    for (auto Info : QueueInfos)
    {
      if (Info.Flags & VK_QUEUE_TRANSFER_BIT)
      {
        uint32_t qId = 0;
        vkGetDeviceQueue(Handle, Info.Family, 0, &pVkQueue->Handle);
        pVkQueue->FamilyId = Info.Family;
        pVkQueue->QueueId = 0;
      }
    }
    break;
  }
  }
  *pQueue = pVkQueue;
  return Result::Ok;
}

Result VulkanDevice::CreateShaderLayout(const ShaderLayoutDesc * pShaderLayoutDesc, ShaderLayout ** ppShaderLayout)
{
  *ppShaderLayout = new VulkanShaderLayout(this, pShaderLayoutDesc);
  return Result::Ok;
}

Result VulkanDevice::CreatePipelineLayout(const PipelineLayoutDesc * pPipelineLayoutDesc, PipelineLayout ** ppPipelineLayout)
{
  *ppPipelineLayout = new VulkanPipelineLayout(this, pPipelineLayoutDesc);
  return Result();
}

void VulkanSwapChain::InitWithRenderPass(RenderPass * pRenderPass)
{
}

Result VulkanSwapChain::GetTexture(Texture ** ppTexture, uint32_t index)
{
  return Result();
}

Drawable * VulkanSwapChain::CurrentDrawable()
{
  return nullptr;
}

Drawable * VulkanSwapChain::NextDrawable()
{
  return nullptr;
}

uint32_t VulkanSwapChain::BufferCount()
{
  return uint32_t();
}

VulkanSwapChain::VulkanSwapChain(VulkanFactory* pFactory, void* pHandle, const SwapChainDesc* pDesc, VulkanQueue* pQueue)
  : OwningRoot(pFactory)
{
  OwningRoot->AddInternalRef();

  OwningDevice = pQueue->OwningRoot;
  OwningDevice->AddInternalRef();

  CreateInfo = { VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR };
  CreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
  CreateInfo.imageArrayLayers = 1;
  CreateInfo.queueFamilyIndexCount = VK_SHARING_MODE_EXCLUSIVE;
  CreateInfo.oldSwapchain = VK_NULL_HANDLE;
  CreateInfo.clipped = VK_TRUE;
  CreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

#if K3DPLATFORM_OS_WIN
  VkWin32SurfaceCreateInfoKHR SurfaceCreateInfo = {};
  SurfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
  SurfaceCreateInfo.hinstance = GetModuleHandle(nullptr);
  SurfaceCreateInfo.hwnd = (HWND)pHandle;
  vkCreateWin32SurfaceKHR(OwningRoot->Handle, &SurfaceCreateInfo, VULKAN_ALLOCATOR, &Surface);
#endif
  
  vkCreateSwapchainKHR(OwningDevice->Handle, &CreateInfo, VULKAN_ALLOCATOR, &Handle);
}

VulkanSwapChain::~VulkanSwapChain()
{
  vkDestroySwapchainKHR(OwningDevice->Handle, Handle, VULKAN_ALLOCATOR);
  OwningDevice->ReleaseInternal();
  vkDestroySurfaceKHR(OwningRoot->Handle, Surface, VULKAN_ALLOCATOR);
  OwningRoot->ReleaseInternal();
}

Result VulkanFactory::CreateSwapchain(const SwapChainDesc* desc, CommandQueue* pCommandQueue, void* pWindow, SwapChain** pSwapchain)
{  
  *pSwapchain = new VulkanSwapChain(this, pWindow, desc, static_cast<VulkanQueue*>(pCommandQueue));
  return Result::Ok;
}

Result VulkanFactory::CreateCompiler(ShaderLang shaderLang, Compiler ** compiler)
{
  *compiler = new GlslangCompiler;
  return Result::Ok;
}

Result VulkanFactory::EnumDevice(uint32_t * count, Device ** ppDevice)
{
  if(!ppDevice)
  {
    vkEnumeratePhysicalDevices(Handle, count, nullptr);
  }
  else
  {
    if(count && *count > 0)
    {
      VkPhysicalDevice * PhysicalDevices = new VkPhysicalDevice[*count];
      vkEnumeratePhysicalDevices(Handle, count, PhysicalDevices);
      for(uint32_t i=0; i<*count; i++)
      {
        ppDevice[i] = new VulkanDevice(this, PhysicalDevices[i]);
      }
      delete[] PhysicalDevices;
    }
    else
    {
      return Result::Failed;
    }
  //    vkEnumeratePhysicalDeviceGroupsKHX()
  }
  return Result::Ok;
}

Result VulkanTexture::GetDesc(TextureDesc * pDesc)
{
  return Result();
}

Result VulkanTexture::CreateView(const TextureViewDesc * pDesc, TextureView ** ppView)
{
  return Result();
}

Result VulkanDevice::CreateBindingTable(PipelineLayout * pPipelineLayout, BindingTable ** ppBindingTable)
{
  return Result::Ok;
}

Result VulkanDevice::CreateRenderPipeline(const RenderPipelineDesc * pPipelineDesc, PipelineLayout * pPipelineLayout, RenderPass * pRenderPass, Pipeline ** pPipelineState)
{
  
  return Result::Ok;
}

Result VulkanDevice::CreateComputePipeline(Function * pComputeFunction, PipelineLayout * pPipelineLayout, Pipeline ** pPipeline)
{
  return Result::Ok;
}

Result VulkanDevice::CreateRenderPass(const RenderPassDesc * desc, RenderPass ** ppRenderpass)
{
  return Result::Ok;
}

Result VulkanDevice::CreateRenderTarget(const RenderTargetDesc * desc, RenderTarget ** ppRenderTarget)
{
  return Result::Ok;
}

Result VulkanDevice::CreateSampler(const SamplerDesc * desc, Sampler ** pSampler)
{
  *pSampler = new VulkanSampler(this, desc);
  return Result::Ok;
}

Result VulkanDevice::CreateBuffer(const BufferDesc * desc, Buffer ** pBuffer)
{
  *pBuffer = new VulkanBuffer(this, desc);
  return Result::Ok;
}

Result VulkanDevice::CreateTexture(const TextureDesc * desc, Texture ** pTexture)
{
  *pTexture = new VulkanTexture(this, desc);
  return Result();
}

Result VulkanDevice::CreateFence(Fence ** ppFence)
{
  *ppFence = new VulkanFence(this);
  return Result::Ok;
}

void VulkanDevice::WaitIdle()
{
  vkDeviceWaitIdle(Handle);
}

VulkanSampler::VulkanSampler(VulkanDevice* pDevice, const SamplerDesc* pDesc)
  : OwningDevice(pDevice)
{
  OwningDevice->AddInternalRef();
  Info.magFilter = ConvertFilterModeToVulkanEnum(pDesc->filter.magFilter);
  Info.minFilter = ConvertFilterModeToVulkanEnum(pDesc->filter.minFilter);
  Info.mipmapMode = pDesc->filter.mipMapFilter == FilterMode::Linear?
    VK_SAMPLER_MIPMAP_MODE_LINEAR : VK_SAMPLER_MIPMAP_MODE_NEAREST;
  Info.addressModeU = ConvertAddressModeToVulkanEnum(pDesc->U);
  Info.addressModeV = ConvertAddressModeToVulkanEnum(pDesc->V);
  Info.addressModeW = ConvertAddressModeToVulkanEnum(pDesc->W);
  Info.mipLodBias = pDesc->mipLodBias;
  Info.anisotropyEnable;
  Info.maxAnisotropy = pDesc->maxAnistropy;
  Info.compareEnable;
  Info.compareOp = ConvertComparisonFunctionToVulkanEnum(pDesc->comparisonFunc);
  Info.minLod = pDesc->minLod;
  Info.maxLod = pDesc->maxLod;
  Info.borderColor;
  Info.unnormalizedCoordinates;
  vkCreateSampler(OwningDevice->Handle, &Info, VULKAN_ALLOCATOR, &Handle);
}

VulkanSampler::~VulkanSampler()
{
  if(Handle)
  {
    vkDestroySampler(OwningDevice->Handle, Handle, VULKAN_ALLOCATOR);
  }
  OwningDevice->ReleaseInternal();
}

Result VulkanSampler::GetDesc(SamplerDesc * desc)
{
  return Result::Ok;
}

VulkanComputePipeline::VulkanComputePipeline(VulkanDevice * pDevice, Function * pComputeFunc, PipelineLayout * pLayout)
  : Super(pDevice)
{
  VkComputePipelineCreateInfo Info = 
  {
    VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO, nullptr, 0
  };
}

VulkanComputePipeline::~VulkanComputePipeline()
{
}

VulkanRenderPipeline::VulkanRenderPipeline(VulkanDevice * pDevice, const RenderPipelineDesc * pDesc, RenderPass * pRenderPass, PipelineLayout * pLayout)
  : Super(pDevice)
{
  VkGraphicsPipelineCreateInfo Info =
  {
    VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO, nullptr, 0
  };
  uint32_t stageCount = 0;
  if (pDesc->vertexFunction)
  {
    auto vertShader = static_cast<VulkanFunction*>(pDesc->vertexFunction);
    vkCreateShaderModule(OwningDevice->Handle, &vertShader->ShaderModuleInfo, VULKAN_ALLOCATOR, &vertShader->ShaderModule);
    stageCount++;
  }
  if (pDesc->pixelFunction)
  {
    stageCount++;
  }
  Info.stageCount = stageCount;
  VkPipelineShaderStageCreateInfo* pStageInfos = 
    (VkPipelineShaderStageCreateInfo*)calloc(stageCount, sizeof(VkPipelineShaderStageCreateInfo));
  //pStageInfos[0] = vertShader->GetPipelineStageInfo();
  Info.pStages = pStageInfos;
}

VulkanRenderPipeline::~VulkanRenderPipeline()
{
}

Result VulkanRenderPipeline::GetDesc(RenderPipelineDesc * pDesc)
{
  return Result();
}


template<class T>
TPipeline<T>::TPipeline(VulkanDevice * pDevice)
  : OwningDevice(pDevice)
{
  OwningDevice->AddInternalRef();
}

template<class T>
TPipeline<T>::~TPipeline()
{
  if(Handle)
  {
    vkDestroyPipeline(OwningDevice->Handle, Handle, VULKAN_ALLOCATOR);
  }
  OwningDevice->ReleaseInternal();
}

VulkanShaderLayout::VulkanShaderLayout(VulkanDevice * pDevice, const ShaderLayoutDesc* pDesc)
  : OwningDevice(pDevice)
{
  OwningDevice->AddInternalRef();
  assert(pDesc && pDesc->count >= 0);
  Info.bindingCount = pDesc->count;
  VkDescriptorSetLayoutBinding* pSetBindings =
    (VkDescriptorSetLayoutBinding*)calloc(pDesc->count, sizeof(VkDescriptorSetLayoutBinding));
  for(uint32_t i; i < pDesc->count; i++)
  {
    pSetBindings[i].binding = pDesc->pShaderBindings[i].slot;
  }
  Info.pBindings = pSetBindings;
  vkCreateDescriptorSetLayout(OwningDevice->Handle, &Info, VULKAN_ALLOCATOR, &Handle);
}

VulkanShaderLayout::~VulkanShaderLayout()
{
  if(Handle)
  {
    vkDestroyDescriptorSetLayout(OwningDevice->Handle, Handle, VULKAN_ALLOCATOR);
  }
  OwningDevice->ReleaseInternal();
}


VulkanPipelineLayout::VulkanPipelineLayout(VulkanDevice * pDevice, const PipelineLayoutDesc * pDesc)
  : OwningDevice(pDevice)
{
  OwningDevice->AddInternalRef();
  assert(pDesc && pDesc->shaderLayoutCount);
  VkPipelineLayoutCreateInfo Info = { VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };
  Info.setLayoutCount = pDesc->shaderLayoutCount;
  VkDescriptorSetLayout* pLayouts = (VkDescriptorSetLayout*)calloc(pDesc->shaderLayoutCount, sizeof(VkDescriptorSetLayout*));
  for(uint32_t i; i < pDesc->shaderLayoutCount; i++)
  {
    pLayouts[i] = static_cast<const VulkanShaderLayout*>(pDesc->pShaderLayout + i)->Handle;
  }
  Info.pSetLayouts = pLayouts;
  vkCreatePipelineLayout(OwningDevice->Handle, &Info, VULKAN_ALLOCATOR, &Handle);
}

VulkanPipelineLayout::~VulkanPipelineLayout()
{
  if(Handle)
  {
    vkDestroyPipelineLayout(OwningDevice->Handle, Handle, VULKAN_ALLOCATOR);
  }
  OwningDevice->ReleaseInternal();
}

Result VulkanPipelineLayout::CreateBindingTable(BindingTable ** ppBindingTable)
{
  *ppBindingTable = new VulkanBindingTable(this);
  return Result::Ok;
}

VulkanBindingTable::VulkanBindingTable(VulkanPipelineLayout * pLayout)
  : OwningLayout(pLayout)
{
}

VulkanBindingTable::~VulkanBindingTable()
{
}

void VulkanBindingTable::SetSampler(uint32_t index, ShaderType shaderVis, Sampler * pSampler)
{
//  vkUpdateDescriptorSets()
}

void VulkanBindingTable::SetBuffer(uint32_t index, ShaderType shaderVis, BufferView * bufferView)
{
}

void VulkanBindingTable::SetTexture(uint32_t index, ShaderType shaderVis, TextureView * textureView)
{
}

VulkanFence::VulkanFence(VulkanDevice * pDevice)
  : OwningDevice(pDevice)
{
  OwningDevice->AddInternalRef();
  VkFenceCreateInfo Info = { VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
  vkCreateFence(OwningDevice->Handle, &Info, VULKAN_ALLOCATOR, &Handle);
}

VulkanFence::~VulkanFence()
{
  if(Handle)
  {
    vkDestroyFence(OwningDevice->Handle, Handle, VULKAN_ALLOCATOR);
  }
  OwningDevice->ReleaseInternal();
}

void VulkanFence::Wait()
{
}

void VulkanFence::Reset()
{
}

VulkanDrawable::VulkanDrawable()
{
}

VulkanDrawable::~VulkanDrawable()
{
}

Texture * VulkanDrawable::Texture()
{
  return nullptr;
}

void VulkanCommandBuffer::Commit(Fence * pFence)
{
  vkQueueSubmit(OwningRoot->Handle, 0, nullptr, static_cast<VulkanFence*>(pFence)->Handle);
}

template<class T>
TCmdEncoder<T>::TCmdEncoder(VulkanCommandBuffer* pCmd)
  : OwningCommand(pCmd)
{
}

template<class T>
TCmdEncoder<T>::~TCmdEncoder()
{
}

template<class T>
void TCmdEncoder<T>::Barrier(Resource * pResource)
{
}

template<class T>
void TCmdEncoder<T>::SetPipeline(Pipeline * pPipelineState)
{
}

template<class T>
void TCmdEncoder<T>::SetPipelineLayout(PipelineLayout * pPipelineLayout)
{
}

template<class T>
void TCmdEncoder<T>::SetBindingTable(BindingTable * pBindingTable)
{
}

template<class T>
void TCmdEncoder<T>::EndEncode()
{
}

VulkanRenderEncoder::VulkanRenderEncoder(VulkanCommandBuffer * pCmd)
  : Super(pCmd)
{
}

VulkanRenderEncoder::~VulkanRenderEncoder()
{
}

void VulkanRenderEncoder::SetScissorRect(uint32_t x, uint32_t y, uint32_t w, uint32_t h)
{
  VkRect2D Rect2D = { {x, y}, {w, h} };
  vkCmdSetScissor(OwningCommand->Handle, 0, 1, &Rect2D);
}

void VulkanRenderEncoder::SetViewport(const Viewport * pViewport)
{
  VkViewport viewPort = { pViewport->left, pViewport->top, pViewport->width, pViewport->height, pViewport->minDepth, pViewport->maxDepth };
  vkCmdSetViewport(OwningCommand->Handle, 0, 1, &viewPort);
}

void VulkanRenderEncoder::SetIndexBuffer(Buffer * pIndexBuffer)
{
}

void VulkanRenderEncoder::SetVertexBuffer(uint32_t slot, uint64_t offset, Buffer * pVertexBuffer)
{
}

void VulkanRenderEncoder::SetPrimitiveType(PrimitiveType primitive)
{
}

void VulkanRenderEncoder::DrawInstanced(const DrawInstancedDesc * drawParam)
{
}

void VulkanRenderEncoder::DrawIndexedInstanced(const DrawIndexedInstancedDesc * drawParam)
{
}

void VulkanRenderEncoder::Present(Drawable * pDrawable)
{
}

VulkanCopyEncoder::VulkanCopyEncoder(VulkanCommandBuffer * pCmd)
  : Super(pCmd)
{
}

VulkanCopyEncoder::~VulkanCopyEncoder()
{
}

void VulkanCopyEncoder::CopyTexture()
{
}

void VulkanCopyEncoder::CopyBuffer(uint64_t srcOffset, uint64_t dstOffset, uint64_t size, Buffer * srcBuffer, Buffer * dstBuffer)
{
  VkBufferCopy copy = { srcOffset, dstOffset, size };
  vkCmdCopyBuffer(OwningCommand->Handle, 
    static_cast<VulkanBuffer*>(srcBuffer)->GetHandle(), 
    static_cast<VulkanBuffer*>(dstBuffer)->GetHandle(), 
    1, &copy);
}

VulkanComputeEncoder::VulkanComputeEncoder(VulkanCommandBuffer * pCmd)
  : Super(pCmd)
{
}

VulkanComputeEncoder::~VulkanComputeEncoder()
{
}

void VulkanComputeEncoder::Dispatch(uint32_t x, uint32_t y, uint32_t z)
{
  vkCmdDispatch(OwningCommand->Handle, x, y, z);
}
