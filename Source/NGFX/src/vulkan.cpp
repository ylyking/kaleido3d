#include <Kaleido3D.h>
#include <KTL/Allocator.hpp>
#include <Core/Os.h>

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

using Os::File;
using Os::MemMapFile;

#define VULKAN_ALLOCATOR nullptr

static std::vector<const char*> RequiredLayers =
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

const char * StrVkFormat(VkFormat format)
{
  switch (format)
  {
  case VK_FORMAT_UNDEFINED: return "VK_FORMAT_UNDEFINED";
  case VK_FORMAT_R4G4_UNORM_PACK8: return "VK_FORMAT_R4G4_UNORM_PACK8";
  case VK_FORMAT_R4G4B4A4_UNORM_PACK16: return "VK_FORMAT_R4G4B4A4_UNORM_PACK16";
  case VK_FORMAT_B4G4R4A4_UNORM_PACK16: return "VK_FORMAT_B4G4R4A4_UNORM_PACK16";
  case VK_FORMAT_R5G6B5_UNORM_PACK16: return "VK_FORMAT_R5G6B5_UNORM_PACK16";
  case VK_FORMAT_B5G6R5_UNORM_PACK16: return "VK_FORMAT_B5G6R5_UNORM_PACK16";
  case VK_FORMAT_R5G5B5A1_UNORM_PACK16: return "VK_FORMAT_R5G5B5A1_UNORM_PACK16";
  case VK_FORMAT_B5G5R5A1_UNORM_PACK16: return "VK_FORMAT_B5G5R5A1_UNORM_PACK16";
  case VK_FORMAT_A1R5G5B5_UNORM_PACK16: return "VK_FORMAT_A1R5G5B5_UNORM_PACK16";
  case VK_FORMAT_R8_UNORM: return "VK_FORMAT_R8_UNORM";
  case VK_FORMAT_R8_SNORM: return "VK_FORMAT_R8_SNORM";
  case VK_FORMAT_R8_USCALED: return "VK_FORMAT_R8_USCALED";
  case VK_FORMAT_R8_SSCALED: return "VK_FORMAT_R8_SSCALED";
  case VK_FORMAT_R8_UINT: return "VK_FORMAT_R8_UINT";
  case VK_FORMAT_R8_SINT: return "VK_FORMAT_R8_SINT";
  case VK_FORMAT_R8_SRGB: return "VK_FORMAT_R8_SRGB";
  case VK_FORMAT_R8G8_UNORM: return "VK_FORMAT_R8G8_UNORM";
  case VK_FORMAT_R8G8_SNORM: return "VK_FORMAT_R8G8_SNORM";
  case VK_FORMAT_R8G8_USCALED: return "VK_FORMAT_R8G8_USCALED";
  case VK_FORMAT_R8G8_SSCALED: return "VK_FORMAT_R8G8_SSCALED";
  case VK_FORMAT_R8G8_UINT: return "VK_FORMAT_R8G8_UINT";
  case VK_FORMAT_R8G8_SINT: return "VK_FORMAT_R8G8_SINT";
  case VK_FORMAT_R8G8_SRGB: return "VK_FORMAT_R8G8_SRGB";
  case VK_FORMAT_R8G8B8_UNORM: return "VK_FORMAT_R8G8B8_UNORM";
  case VK_FORMAT_R8G8B8_SNORM: return "VK_FORMAT_R8G8B8_SNORM";
  case VK_FORMAT_R8G8B8_USCALED: return "VK_FORMAT_R8G8B8_USCALED";
  case VK_FORMAT_R8G8B8_SSCALED: return "VK_FORMAT_R8G8B8_SSCALED";
  case VK_FORMAT_R8G8B8_UINT: return "VK_FORMAT_R8G8B8_UINT";
  case VK_FORMAT_R8G8B8_SINT: return "VK_FORMAT_R8G8B8_SINT";
  case VK_FORMAT_R8G8B8_SRGB: return "VK_FORMAT_R8G8B8_SRGB";
  case VK_FORMAT_B8G8R8_UNORM: return "VK_FORMAT_B8G8R8_UNORM";
  case VK_FORMAT_B8G8R8_SNORM: return "VK_FORMAT_B8G8R8_SNORM";
  case VK_FORMAT_B8G8R8_USCALED: return "VK_FORMAT_B8G8R8_USCALED";
  case VK_FORMAT_B8G8R8_SSCALED: return "VK_FORMAT_B8G8R8_SSCALED";
  case VK_FORMAT_B8G8R8_UINT: return "VK_FORMAT_B8G8R8_UINT";
  case VK_FORMAT_B8G8R8_SINT: return "VK_FORMAT_B8G8R8_SINT";
  case VK_FORMAT_B8G8R8_SRGB: return "VK_FORMAT_B8G8R8_SRGB";
  case VK_FORMAT_R8G8B8A8_UNORM: return "VK_FORMAT_R8G8B8A8_UNORM";
  case VK_FORMAT_R8G8B8A8_SNORM: return "VK_FORMAT_R8G8B8A8_SNORM";
  case VK_FORMAT_R8G8B8A8_USCALED: return "VK_FORMAT_R8G8B8A8_USCALED";
  case VK_FORMAT_R8G8B8A8_SSCALED: return "VK_FORMAT_R8G8B8A8_SSCALED";
  case VK_FORMAT_R8G8B8A8_UINT: return "VK_FORMAT_R8G8B8A8_UINT";
  case VK_FORMAT_R8G8B8A8_SINT: return "VK_FORMAT_R8G8B8A8_SINT";
  case VK_FORMAT_R8G8B8A8_SRGB: return "VK_FORMAT_R8G8B8A8_SRGB";
  case VK_FORMAT_B8G8R8A8_UNORM: return "VK_FORMAT_B8G8R8A8_UNORM";
  case VK_FORMAT_B8G8R8A8_SNORM: return "VK_FORMAT_B8G8R8A8_SNORM";
  case VK_FORMAT_B8G8R8A8_USCALED: return "VK_FORMAT_B8G8R8A8_USCALED";
  case VK_FORMAT_B8G8R8A8_SSCALED: return "VK_FORMAT_B8G8R8A8_SSCALED";
  case VK_FORMAT_B8G8R8A8_UINT: return "VK_FORMAT_B8G8R8A8_UINT";
  case VK_FORMAT_B8G8R8A8_SINT: return "VK_FORMAT_B8G8R8A8_SINT";
  case VK_FORMAT_B8G8R8A8_SRGB: return "VK_FORMAT_B8G8R8A8_SRGB";
  case VK_FORMAT_A8B8G8R8_UNORM_PACK32: return "VK_FORMAT_A8B8G8R8_UNORM_PACK32";
  case VK_FORMAT_A8B8G8R8_SNORM_PACK32: return "VK_FORMAT_A8B8G8R8_SNORM_PACK32";
  case VK_FORMAT_A8B8G8R8_USCALED_PACK32: return "VK_FORMAT_A8B8G8R8_USCALED_PACK32";
  case VK_FORMAT_A8B8G8R8_SSCALED_PACK32: return "VK_FORMAT_A8B8G8R8_SSCALED_PACK32";
  case VK_FORMAT_A8B8G8R8_UINT_PACK32: return "VK_FORMAT_A8B8G8R8_UINT_PACK32";
  case VK_FORMAT_A8B8G8R8_SINT_PACK32: return "VK_FORMAT_A8B8G8R8_SINT_PACK32";
  case VK_FORMAT_A8B8G8R8_SRGB_PACK32: return "VK_FORMAT_A8B8G8R8_SRGB_PACK32";
  case VK_FORMAT_A2R10G10B10_UNORM_PACK32: return "VK_FORMAT_A2R10G10B10_UNORM_PACK32";
  case VK_FORMAT_A2R10G10B10_SNORM_PACK32: return "VK_FORMAT_A2R10G10B10_SNORM_PACK32";
  case VK_FORMAT_A2R10G10B10_USCALED_PACK32: return "VK_FORMAT_A2R10G10B10_USCALED_PACK32";
  case VK_FORMAT_A2R10G10B10_SSCALED_PACK32: return "VK_FORMAT_A2R10G10B10_SSCALED_PACK32";
  case VK_FORMAT_A2R10G10B10_UINT_PACK32: return "VK_FORMAT_A2R10G10B10_UINT_PACK32";
  case VK_FORMAT_A2R10G10B10_SINT_PACK32: return "VK_FORMAT_A2R10G10B10_SINT_PACK32";
  case VK_FORMAT_A2B10G10R10_UNORM_PACK32: return "VK_FORMAT_A2B10G10R10_UNORM_PACK32";
  case VK_FORMAT_A2B10G10R10_SNORM_PACK32: return "VK_FORMAT_A2B10G10R10_SNORM_PACK32";
  case VK_FORMAT_A2B10G10R10_USCALED_PACK32: return "VK_FORMAT_A2B10G10R10_USCALED_PACK32";
  case VK_FORMAT_A2B10G10R10_SSCALED_PACK32: return "VK_FORMAT_A2B10G10R10_SSCALED_PACK32";
  case VK_FORMAT_A2B10G10R10_UINT_PACK32: return "VK_FORMAT_A2B10G10R10_UINT_PACK32";
  case VK_FORMAT_A2B10G10R10_SINT_PACK32: return "VK_FORMAT_A2B10G10R10_SINT_PACK32";
  case VK_FORMAT_R16_UNORM: return "VK_FORMAT_R16_UNORM";
  case VK_FORMAT_R16_SNORM: return "VK_FORMAT_R16_SNORM";
  case VK_FORMAT_R16_USCALED: return "VK_FORMAT_R16_USCALED";
  case VK_FORMAT_R16_SSCALED: return "VK_FORMAT_R16_SSCALED";
  case VK_FORMAT_R16_UINT: return "VK_FORMAT_R16_UINT";
  case VK_FORMAT_R16_SINT: return "VK_FORMAT_R16_SINT";
  case VK_FORMAT_R16_SFLOAT: return "VK_FORMAT_R16_SFLOAT";
  case VK_FORMAT_R16G16_UNORM: return "VK_FORMAT_R16G16_UNORM";
  case VK_FORMAT_R16G16_SNORM: return "VK_FORMAT_R16G16_SNORM";
  case VK_FORMAT_R16G16_USCALED: return "VK_FORMAT_R16G16_USCALED";
  case VK_FORMAT_R16G16_SSCALED: return "VK_FORMAT_R16G16_SSCALED";
  case VK_FORMAT_R16G16_UINT: return "VK_FORMAT_R16G16_UINT";
  case VK_FORMAT_R16G16_SINT: return "VK_FORMAT_R16G16_SINT";
  case VK_FORMAT_R16G16_SFLOAT: return "VK_FORMAT_R16G16_SFLOAT";
  case VK_FORMAT_R16G16B16_UNORM: return "VK_FORMAT_R16G16B16_UNORM";
  case VK_FORMAT_R16G16B16_SNORM: return "VK_FORMAT_R16G16B16_SNORM";
  case VK_FORMAT_R16G16B16_USCALED: return "VK_FORMAT_R16G16B16_USCALED";
  case VK_FORMAT_R16G16B16_SSCALED: return "VK_FORMAT_R16G16B16_SSCALED";
  case VK_FORMAT_R16G16B16_UINT: return "VK_FORMAT_R16G16B16_UINT";
  case VK_FORMAT_R16G16B16_SINT: return "VK_FORMAT_R16G16B16_SINT";
  case VK_FORMAT_R16G16B16_SFLOAT: return "VK_FORMAT_R16G16B16_SFLOAT";
  case VK_FORMAT_R16G16B16A16_UNORM: return "VK_FORMAT_R16G16B16A16_UNORM";
  case VK_FORMAT_R16G16B16A16_SNORM: return "VK_FORMAT_R16G16B16A16_SNORM";
  case VK_FORMAT_R16G16B16A16_USCALED: return "VK_FORMAT_R16G16B16A16_USCALED";
  case VK_FORMAT_R16G16B16A16_SSCALED: return "VK_FORMAT_R16G16B16A16_SSCALED";
  case VK_FORMAT_R16G16B16A16_UINT: return "VK_FORMAT_R16G16B16A16_UINT";
  case VK_FORMAT_R16G16B16A16_SINT: return "VK_FORMAT_R16G16B16A16_SINT";
  case VK_FORMAT_R16G16B16A16_SFLOAT: return "VK_FORMAT_R16G16B16A16_SFLOAT";
  case VK_FORMAT_R32_UINT: return "VK_FORMAT_R32_UINT";
  case VK_FORMAT_R32_SINT: return "VK_FORMAT_R32_SINT";
  case VK_FORMAT_R32_SFLOAT: return "VK_FORMAT_R32_SFLOAT";
  case VK_FORMAT_R32G32_UINT: return "VK_FORMAT_R32G32_UINT";
  case VK_FORMAT_R32G32_SINT: return "VK_FORMAT_R32G32_SINT";
  case VK_FORMAT_R32G32_SFLOAT: return "VK_FORMAT_R32G32_SFLOAT";
  case VK_FORMAT_R32G32B32_UINT: return "VK_FORMAT_R32G32B32_UINT";
  case VK_FORMAT_R32G32B32_SINT: return "VK_FORMAT_R32G32B32_SINT";
  case VK_FORMAT_R32G32B32_SFLOAT: return "VK_FORMAT_R32G32B32_SFLOAT";
  case VK_FORMAT_R32G32B32A32_UINT: return "VK_FORMAT_R32G32B32A32_UINT";
  case VK_FORMAT_R32G32B32A32_SINT: return "VK_FORMAT_R32G32B32A32_SINT";
  case VK_FORMAT_R32G32B32A32_SFLOAT: return "VK_FORMAT_R32G32B32A32_SFLOAT";
  case VK_FORMAT_R64_UINT: return "VK_FORMAT_R64_UINT";
  case VK_FORMAT_R64_SINT: return "VK_FORMAT_R64_SINT";
  case VK_FORMAT_R64_SFLOAT: return "VK_FORMAT_R64_SFLOAT";
  case VK_FORMAT_R64G64_UINT: return "VK_FORMAT_R64G64_UINT";
  case VK_FORMAT_R64G64_SINT: return "VK_FORMAT_R64G64_SINT";
  case VK_FORMAT_R64G64_SFLOAT: return "VK_FORMAT_R64G64_SFLOAT";
  case VK_FORMAT_R64G64B64_UINT: return "VK_FORMAT_R64G64B64_UINT";
  case VK_FORMAT_R64G64B64_SINT: return "VK_FORMAT_R64G64B64_SINT";
  case VK_FORMAT_R64G64B64_SFLOAT: return "VK_FORMAT_R64G64B64_SFLOAT";
  case VK_FORMAT_R64G64B64A64_UINT: return "VK_FORMAT_R64G64B64A64_UINT";
  case VK_FORMAT_R64G64B64A64_SINT: return "VK_FORMAT_R64G64B64A64_SINT";
  case VK_FORMAT_R64G64B64A64_SFLOAT: return "VK_FORMAT_R64G64B64A64_SFLOAT";
  case VK_FORMAT_B10G11R11_UFLOAT_PACK32: return "VK_FORMAT_B10G11R11_UFLOAT_PACK32";
  case VK_FORMAT_E5B9G9R9_UFLOAT_PACK32: return "VK_FORMAT_E5B9G9R9_UFLOAT_PACK32";
  case VK_FORMAT_D16_UNORM: return "VK_FORMAT_D16_UNORM";
  case VK_FORMAT_X8_D24_UNORM_PACK32: return "VK_FORMAT_X8_D24_UNORM_PACK32";
  case VK_FORMAT_D32_SFLOAT: return "VK_FORMAT_D32_SFLOAT";
  case VK_FORMAT_S8_UINT: return "VK_FORMAT_S8_UINT";
  case VK_FORMAT_D16_UNORM_S8_UINT: return "VK_FORMAT_D16_UNORM_S8_UINT";
  case VK_FORMAT_D24_UNORM_S8_UINT: return "VK_FORMAT_D24_UNORM_S8_UINT";
  case VK_FORMAT_D32_SFLOAT_S8_UINT: return "VK_FORMAT_D32_SFLOAT_S8_UINT";
  case VK_FORMAT_BC1_RGB_UNORM_BLOCK: return "VK_FORMAT_BC1_RGB_UNORM_BLOCK";
  case VK_FORMAT_BC1_RGB_SRGB_BLOCK: return "VK_FORMAT_BC1_RGB_SRGB_BLOCK";
  case VK_FORMAT_BC1_RGBA_UNORM_BLOCK: return "VK_FORMAT_BC1_RGBA_UNORM_BLOCK";
  case VK_FORMAT_BC1_RGBA_SRGB_BLOCK: return "VK_FORMAT_BC1_RGBA_SRGB_BLOCK";
  case VK_FORMAT_BC2_UNORM_BLOCK: return "VK_FORMAT_BC2_UNORM_BLOCK";
  case VK_FORMAT_BC2_SRGB_BLOCK: return "VK_FORMAT_BC2_SRGB_BLOCK";
  case VK_FORMAT_BC3_UNORM_BLOCK: return "VK_FORMAT_BC3_UNORM_BLOCK";
  case VK_FORMAT_BC3_SRGB_BLOCK: return "VK_FORMAT_BC3_SRGB_BLOCK";
  case VK_FORMAT_BC4_UNORM_BLOCK: return "VK_FORMAT_BC4_UNORM_BLOCK";
  case VK_FORMAT_BC4_SNORM_BLOCK: return "VK_FORMAT_BC4_SNORM_BLOCK";
  case VK_FORMAT_BC5_UNORM_BLOCK: return "VK_FORMAT_BC5_UNORM_BLOCK";
  case VK_FORMAT_BC5_SNORM_BLOCK: return "VK_FORMAT_BC5_SNORM_BLOCK";
  case VK_FORMAT_BC6H_UFLOAT_BLOCK: return "VK_FORMAT_BC6H_UFLOAT_BLOCK";
  case VK_FORMAT_BC6H_SFLOAT_BLOCK: return "VK_FORMAT_BC6H_SFLOAT_BLOCK";
  case VK_FORMAT_BC7_UNORM_BLOCK: return "VK_FORMAT_BC7_UNORM_BLOCK";
  case VK_FORMAT_BC7_SRGB_BLOCK: return "VK_FORMAT_BC7_SRGB_BLOCK";
  case VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK: return "VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK";
  case VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK: return "VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK";
  case VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK: return "VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK";
  case VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK: return "VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK";
  case VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK: return "VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK";
  case VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK: return "VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK";
  case VK_FORMAT_EAC_R11_UNORM_BLOCK: return "VK_FORMAT_EAC_R11_UNORM_BLOCK";
  case VK_FORMAT_EAC_R11_SNORM_BLOCK: return "VK_FORMAT_EAC_R11_SNORM_BLOCK";
  case VK_FORMAT_EAC_R11G11_UNORM_BLOCK: return "VK_FORMAT_EAC_R11G11_UNORM_BLOCK";
  case VK_FORMAT_EAC_R11G11_SNORM_BLOCK: return "VK_FORMAT_EAC_R11G11_SNORM_BLOCK";
  case VK_FORMAT_ASTC_4x4_UNORM_BLOCK: return "VK_FORMAT_ASTC_4x4_UNORM_BLOCK";
  case VK_FORMAT_ASTC_4x4_SRGB_BLOCK: return "VK_FORMAT_ASTC_4x4_SRGB_BLOCK";
  case VK_FORMAT_ASTC_5x4_UNORM_BLOCK: return "VK_FORMAT_ASTC_5x4_UNORM_BLOCK";
  case VK_FORMAT_ASTC_5x4_SRGB_BLOCK: return "VK_FORMAT_ASTC_5x4_SRGB_BLOCK";
  case VK_FORMAT_ASTC_5x5_UNORM_BLOCK: return "VK_FORMAT_ASTC_5x5_UNORM_BLOCK";
  case VK_FORMAT_ASTC_5x5_SRGB_BLOCK: return "VK_FORMAT_ASTC_5x5_SRGB_BLOCK";
  case VK_FORMAT_ASTC_6x5_UNORM_BLOCK: return "VK_FORMAT_ASTC_6x5_UNORM_BLOCK";
  case VK_FORMAT_ASTC_6x5_SRGB_BLOCK: return "VK_FORMAT_ASTC_6x5_SRGB_BLOCK";
  case VK_FORMAT_ASTC_6x6_UNORM_BLOCK: return "VK_FORMAT_ASTC_6x6_UNORM_BLOCK";
  case VK_FORMAT_ASTC_6x6_SRGB_BLOCK: return "VK_FORMAT_ASTC_6x6_SRGB_BLOCK";
  case VK_FORMAT_ASTC_8x5_UNORM_BLOCK: return "VK_FORMAT_ASTC_8x5_UNORM_BLOCK";
  case VK_FORMAT_ASTC_8x5_SRGB_BLOCK: return "VK_FORMAT_ASTC_8x5_SRGB_BLOCK";
  case VK_FORMAT_ASTC_8x6_UNORM_BLOCK: return "VK_FORMAT_ASTC_8x6_UNORM_BLOCK";
  case VK_FORMAT_ASTC_8x6_SRGB_BLOCK: return "VK_FORMAT_ASTC_8x6_SRGB_BLOCK";
  case VK_FORMAT_ASTC_8x8_UNORM_BLOCK: return "VK_FORMAT_ASTC_8x8_UNORM_BLOCK";
  case VK_FORMAT_ASTC_8x8_SRGB_BLOCK: return "VK_FORMAT_ASTC_8x8_SRGB_BLOCK";
  case VK_FORMAT_ASTC_10x5_UNORM_BLOCK: return "VK_FORMAT_ASTC_10x5_UNORM_BLOCK";
  case VK_FORMAT_ASTC_10x5_SRGB_BLOCK: return "VK_FORMAT_ASTC_10x5_SRGB_BLOCK";
  case VK_FORMAT_ASTC_10x6_UNORM_BLOCK: return "VK_FORMAT_ASTC_10x6_UNORM_BLOCK";
  case VK_FORMAT_ASTC_10x6_SRGB_BLOCK: return "VK_FORMAT_ASTC_10x6_SRGB_BLOCK";
  case VK_FORMAT_ASTC_10x8_UNORM_BLOCK: return "VK_FORMAT_ASTC_10x8_UNORM_BLOCK";
  case VK_FORMAT_ASTC_10x8_SRGB_BLOCK: return "VK_FORMAT_ASTC_10x8_SRGB_BLOCK";
  case VK_FORMAT_ASTC_10x10_UNORM_BLOCK: return "VK_FORMAT_ASTC_10x10_UNORM_BLOCK";
  case VK_FORMAT_ASTC_10x10_SRGB_BLOCK: return "VK_FORMAT_ASTC_10x10_SRGB_BLOCK";
  case VK_FORMAT_ASTC_12x10_UNORM_BLOCK: return "VK_FORMAT_ASTC_12x10_UNORM_BLOCK";
  case VK_FORMAT_ASTC_12x10_SRGB_BLOCK: return "VK_FORMAT_ASTC_12x10_SRGB_BLOCK";
  case VK_FORMAT_ASTC_12x12_UNORM_BLOCK: return "VK_FORMAT_ASTC_12x12_UNORM_BLOCK";
  case VK_FORMAT_ASTC_12x12_SRGB_BLOCK: return "VK_FORMAT_ASTC_12x12_SRGB_BLOCK";
  }
  return "Unknown VkFormat";
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

VkShaderStageFlagBits ConvertShaderTypeToVulkanEnum(ShaderType const& e) {
  switch (e) {
  case ShaderType::Vertex:
    return VK_SHADER_STAGE_VERTEX_BIT;
  case ShaderType::Fragment:
    return VK_SHADER_STAGE_FRAGMENT_BIT;
  case ShaderType::Compute:
    return VK_SHADER_STAGE_COMPUTE_BIT;
  case ShaderType::Geometry:
    return VK_SHADER_STAGE_GEOMETRY_BIT;
  case ShaderType::TessailationEval:
    return VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
  case ShaderType::TessailationControl:
    return VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
  }
  return VK_SHADER_STAGE_ALL;
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

/*
class VulkanDescriptorSet
{
public:
  VkDescriptorSet Handle = VK_NULL_HANDLE;
};
*/
using ByteCode = std::vector<uint32_t>;

class VulkanDevice;
class VulkanLibrary1;

class VulkanFunction1 : public ngfx::Function
{
public:
  VulkanFunction1(VulkanLibrary1* pLibrary, const char * name);
  ~VulkanFunction1() override;

  ShaderType Type() const override;
  const char * Name() const override;

  VulkanLibrary1* Library;
  VkPipelineShaderStageCreateInfo StageInfo;

private:
  std::string           EntryName;
  std::string           Source;
  ngfx::ShaderType      ShaderType;
  VkShaderModule        ShaderModule = VK_NULL_HANDLE;
};

/**
 * Support Multi-Entry SPIRV
 */
class VulkanLibrary1 : public ngfx::Library
{
  friend class VulkanFunction1;
public:
  VulkanLibrary1(VulkanDevice* pDevice, const void *pBlobData, uint64 Size);
  VulkanLibrary1(VulkanDevice* pDevice, const char *pFilePath);
  ~VulkanLibrary1() override;

  Result MakeFunction(const char * name, Function ** ppFunction) override;
protected:
  void Init(const void *pBlobData, uint64 Size);
  
private:
  FunctionMap DataBlob;
  VulkanDevice* Device;
};

class VulkanDescriptorLayout
{
public:
  VulkanDescriptorLayout(VulkanDevice* pDevice, VkDescriptorSetLayoutBinding*, int);
  ~VulkanDescriptorLayout();

  VkDescriptorSetLayoutCreateInfo Info = {
    VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO
  };
  VkDescriptorSetLayout Handle = VK_NULL_HANDLE;
  VulkanDevice* Device;
};

class VulkanDescriptorPool
{
public:
  VulkanDescriptorPool(VulkanDevice* pDevice, uint64_t MaxSet);
  ~VulkanDescriptorPool();


  VkDescriptorPool Handle = VK_NULL_HANDLE;
  VulkanDevice* Device;
};

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

  TCmdEncoder(VulkanCommandBuffer* pCmd, VkPipelineBindPoint BindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS);
  virtual ~TCmdEncoder();

  void Barrier(Resource * pResource) override;
  void SetPipeline(Pipeline* pPipelineState) override;
  void SetBindingTable(BindingTable * pBindingTable) override;
  virtual void EndEncode() override;

  VkPipelineBindPoint CurrentBindingPoint;

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
  void SetDepthBias(Float32 biasConst, Float32 biasClamp, Float32 biasSlope) override;
  void SetDepthBounds(Float32 minDepth, Float32 maxDepth) override;
  void SetStencilReference(StencilFaceRef face, uint32_t value) override;
  void SetBlendConsts(Float32x4 constant) override;
  void SetLineWidth(Float32 width) override;

  void SetIndexBuffer(Buffer * pIndexBuffer);
  void SetVertexBuffer(uint32_t slot, uint64_t offset, Buffer * pVertexBuffer);

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
public:
  VulkanRenderPass(VulkanDevice* pDevice, const RenderPassDesc* pDesc);
  ~VulkanRenderPass() override;
  VulkanDevice* Device;
  VkRenderPass Handle = VK_NULL_HANDLE;
};

template <class T>
struct PipelineTrait
{
};

template <>
struct PipelineTrait<RenderPipeline>
{
  using CreateInfo = VkGraphicsPipelineCreateInfo;
  static decltype(vkCreateGraphicsPipelines)* Create;
  static PipelineType Type;
};

decltype(vkCreateGraphicsPipelines)* PipelineTrait<RenderPipeline>::Create = &vkCreateGraphicsPipelines;
PipelineType PipelineTrait<RenderPipeline>::Type = PipelineType::Graphics;

template <>
struct PipelineTrait<ComputePipeline>
{
  using CreateInfo = VkComputePipelineCreateInfo;
  static decltype(vkCreateComputePipelines)* Create;
  static PipelineType Type;
};

decltype(vkCreateComputePipelines)* PipelineTrait<ComputePipeline>::Create = &vkCreateComputePipelines;
PipelineType PipelineTrait<ComputePipeline>::Type = PipelineType::Compute;

template <class T>
class TPipeline : public T
{
public:

  TPipeline(VulkanDevice * pDevice);
  virtual ~TPipeline() override;

  Result GetCache(uint64_t * pSize, void * pOutData) override
  {
    RetrieveCache(pOutData, pSize);
    return Result::Ok;
  }

  PipelineType Type() const override
  {
    return PipelineTrait<T>::Type;
  }

  VkPipeline Handle = VK_NULL_HANDLE;

protected:
  using Super = typename TPipeline<T>;
  
  VulkanDevice* OwningDevice = nullptr;
  VkPipelineCache Cache = VK_NULL_HANDLE;
  typename PipelineTrait<T>::CreateInfo Info;

  VkResult CreatePipelineCache(void const* InData, uint64 InSize)
  {
    VkPipelineCacheCreateInfo Info = { VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO, nullptr, 0, InSize, InData};
    return vkCreatePipelineCache(OwningDevice->Handle, &Info, VULKAN_ALLOCATOR, &Cache);
  }
  
  VkResult CreatePipeline()
  {
    return PipelineTrait<T>::Create(OwningDevice->Handle, Cache, 1, &Info, VULKAN_ALLOCATOR, &Handle);
  }

  VkResult RetrieveCache(void* OutData, uint64* OutSize)
  {
    if (Cache)
    {
      return vkGetPipelineCacheData(OwningDevice->Handle, Cache, OutSize, OutData);
    }
    //vkMergePipelineCaches(OwningDevice->Handle)
    return VK_RESULT_MAX_ENUM;
  }

private:
  friend class VulkanPipelineLibrary;
};

class VulkanPipelineLibrary : public PipelineLibrary
{
public:
  VulkanPipelineLibrary(VulkanDevice* pDevice, const void* InData, uint64_t Size);
  ~VulkanPipelineLibrary() override;

  void StorePipeline(const char * key, Pipeline * pPipeline) override;
  uint64_t GetSerializedSize() const override;
  void Serialize(void * pData, uint64_t Size) override;

  VkPipelineCache Handle;
  VulkanDevice* Device;
};

class VulkanRenderPipeline : public TPipeline<RenderPipeline>
{
public:
  VulkanRenderPipeline(VulkanDevice* pDevice, const RenderPipelineDesc* pDesc, RenderPass * pRenderPass, PipelineLayout * pLayout);
  ~VulkanRenderPipeline() override;
  Result GetDesc(RenderPipelineDesc * pDesc) override;

protected:
  void InitShaderStages(const RenderPipelineDesc* pDesc);

private:
  /* shaders */
  std::vector<VkPipelineShaderStageCreateInfo> Stages;

  VkPipelineVertexInputStateCreateInfo    VertexInputState;
  VkPipelineInputAssemblyStateCreateInfo  InputAssemblyState;
  VkPipelineTessellationStateCreateInfo   TessellationState;
  VkPipelineViewportStateCreateInfo       ViewportState;
  VkPipelineRasterizationStateCreateInfo  RasterizationState;
  VkPipelineMultisampleStateCreateInfo    MultisampleState;
  VkPipelineDepthStencilStateCreateInfo   DepthStencilState;
  VkPipelineColorBlendStateCreateInfo     BlendState;
  VkPipelineDynamicStateCreateInfo        DynamicState;
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
  VulkanDevice* Device = nullptr;

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
  Result CreatePipelineLibrary(const void * pData, uint64_t Size, PipelineLibrary ** ppPipelineLibrary) override;
  Result CreateLibrary(const CompileOption * compileOption, const void * pData, uint64_t Size, Library ** ppLibrary) override;
  Result CreateRenderPass(const RenderPassDesc * desc, RenderPass ** ppRenderpass) override;
  Result CreateRenderTarget(const RenderTargetDesc * desc, RenderTarget ** ppRenderTarget) override;
  Result CreateSampler(const SamplerDesc* desc, Sampler ** pSampler) override;
  Result CreateBuffer(const BufferDesc* desc, Buffer ** pBuffer) override;
  Result CreateTexture(const TextureDesc * desc, Texture ** pTexture) override;
  Result CreateFence(Fence ** ppFence) override;
  void WaitIdle() override;
  bool SupportAsyncCompute() const { return IsSupportAsyncCompute; }

  VkDevice Handle = VK_NULL_HANDLE;

private:

  struct QueueInfo
  {
    uint32_t Flags = 0;
    uint32_t Family = 0;
    uint32_t Count = 0;
  };

  VkPhysicalDevice Device = VK_NULL_HANDLE;
  VkPhysicalDeviceFeatures Features;
  VmaAllocator MemoryAllocator = nullptr;
  std::vector<QueueInfo> QueueInfos;
  bool IsSupportAsyncCompute;

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
  return Result::Ok;
}

Result VulkanBuffer::CreateView(const BufferViewDesc * pDesc, BufferView ** ppView)
{
  return Result::Ok;
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
    if (pDesc->option != StorageOption::Private)
    {
      LogPrint(Log::Error, "Texture", "When Texture used as RenderTarget, StorageOption should be PRIVATE!\n");
      assert(0);
    }
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
  
  if (pDesc->option == StorageOption::Managed ||
    pDesc->option == StorageOption::Shared)
  {
    Info.tiling = VK_IMAGE_TILING_LINEAR;
  }
  else if (pDesc->option == StorageOption::Private)
  {
    Info.tiling = VK_IMAGE_TILING_OPTIMAL;
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
    LogPrint(Log::Info, "Factory", "DumpingLayers:\n");
    for(auto l : layers)
    {
      LogPrint(Log::Info, "Factory", "\t%s Desc:%s\n", l.layerName, l.description);
    }

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
      instanceCreateInfo.enabledLayerCount = RequiredLayers.size();
      instanceCreateInfo.ppEnabledLayerNames = RequiredLayers.data();
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

  // Create Command Queue
  float QueuePriority = 0.0f;
  std::vector<VkDeviceQueueCreateInfo> DeviceQueueInfo;
  DeviceQueueInfo.resize(queueCount);
  std::vector<float*> QueuePriorities;
  // Async Compute & Transfer
  uint32_t GfxQueueId = 0;
  uint32_t CptQueueId = 0;
  for (uint32 Id = 0; Id < queueCount; Id++)
  {
    QueueInfos[Id].Family = Id;
    QueueInfos[Id].Flags = QueueFamilyProps[Id].queueFlags;
    QueueInfos[Id].Count = QueueFamilyProps[Id].queueCount;
    if(QueueFamilyProps[Id].queueFlags & VK_QUEUE_GRAPHICS_BIT)
    {
      GfxQueueId = Id;
    }
    if (QueueFamilyProps[Id].queueFlags & VK_QUEUE_COMPUTE_BIT)
    {
      CptQueueId = Id;
    }
    DeviceQueueInfo[Id].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    DeviceQueueInfo[Id].pNext = nullptr;
    DeviceQueueInfo[Id].flags = 0;
    DeviceQueueInfo[Id].queueFamilyIndex = Id;
    DeviceQueueInfo[Id].queueCount = QueueFamilyProps[Id].queueCount;
    float * Priorities = new float[QueueFamilyProps[Id].queueCount]{ 0 };
    DeviceQueueInfo[Id].pQueuePriorities = Priorities;
    QueuePriorities.push_back(Priorities);
  }
  IsSupportAsyncCompute = GfxQueueId != CptQueueId;

  LogPrint(Log::Info, "Device", "Vendor: %s\n\tType: %s\n\tVulkan: %d.%d.%d\n\tAsyncCompute:%d\n",
    Prop.deviceName,
    DeviceType(Prop.deviceType),
    VK_VERSION_MAJOR(Prop.apiVersion),
    VK_VERSION_MINOR(Prop.apiVersion),
    VK_VERSION_PATCH(Prop.apiVersion), IsSupportAsyncCompute);

  uint32_t extCount = 0;
  std::vector<VkExtensionProperties> exts;
  vkEnumerateDeviceExtensionProperties(Device, nullptr, &extCount, nullptr);
  exts.resize(extCount);
  vkEnumerateDeviceExtensionProperties(Device, nullptr, &extCount, exts.data());
  std::set<std::string> deviceExtensions;
  LogPrint(Log::Info, "Device", "DumpVulkanDeviceExtensions:\n");
  for (auto ext : exts)
  {
    deviceExtensions.insert(ext.extensionName);
    LogPrint(Log::Info, "Device", "\t%s\n", ext.extensionName);
  }

  uint32_t layerCount = 0;
  std::vector<VkLayerProperties> layers;
  vkEnumerateDeviceLayerProperties(Device, &layerCount, nullptr);
  layers.resize(layerCount);
  vkEnumerateDeviceLayerProperties(Device, &layerCount, layers.data());

  vkGetPhysicalDeviceFeatures(Device, &Features);

  // KHX ?
  VkDeviceCreateInfo deviceCreateInfo = {};
  deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  deviceCreateInfo.pNext = NULL;
  deviceCreateInfo.queueCreateInfoCount = DeviceQueueInfo.size();
  deviceCreateInfo.pQueueCreateInfos = DeviceQueueInfo.data();
  deviceCreateInfo.pEnabledFeatures = &Features;
  if (OwningRoot->Debug)
  {
    if (deviceExtensions.find(VK_EXT_DEBUG_MARKER_EXTENSION_NAME) != deviceExtensions.end())
    {
      RequiredDeviceExtensions.push_back(VK_EXT_DEBUG_MARKER_EXTENSION_NAME);
    }
    deviceCreateInfo.enabledLayerCount = RequiredLayers.size();
    deviceCreateInfo.ppEnabledLayerNames = RequiredLayers.data();
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
  return Result::Ok;
}

void VulkanSwapChain::InitWithRenderPass(RenderPass * pRenderPass)
{
}

Result VulkanSwapChain::GetTexture(Texture ** ppTexture, uint32_t index)
{
  return Result::Ok;
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
  return 0U;
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

  CreateInfo.imageFormat = ConvertPixelFormatToVulkanEnum(pDesc->pixelFormat);
  CreateInfo.imageExtent = { pDesc->width, pDesc->height };
  CreateInfo.minImageCount = pDesc->numColorBuffers;

#if K3DPLATFORM_OS_WIN
  VkWin32SurfaceCreateInfoKHR SurfaceCreateInfo = {};
  SurfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
  SurfaceCreateInfo.hinstance = GetModuleHandle(nullptr);
  SurfaceCreateInfo.hwnd = (HWND)pHandle;
  vkCreateWin32SurfaceKHR(OwningRoot->Handle, &SurfaceCreateInfo, VULKAN_ALLOCATOR, &Surface);
#endif
  
  CreateInfo.surface = Surface;

  LogPrint(Log::Info, "SwapChain", "Chosen SwapChain Format %s.\n", StrVkFormat(CreateInfo.imageFormat));

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
//  *compiler = new GlslangCompiler;
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
  return Result::Ok;
}

Result VulkanTexture::CreateView(const TextureViewDesc * pDesc, TextureView ** ppView)
{
  return Result::Ok;
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

Result VulkanDevice::CreatePipelineLibrary(const void * pData, uint64_t Size, PipelineLibrary ** ppPipelineLibrary)
{
  *ppPipelineLibrary = new VulkanPipelineLibrary(this, pData, Size);
  return Result::Ok;
}

Result VulkanDevice::CreateLibrary(const CompileOption * compileOption, const void * pData, uint64_t Size, Library ** ppLibrary)
{
  * ppLibrary = new VulkanLibrary1(this, pData, Size);
  return Result::Ok;
}

VulkanRenderPass::VulkanRenderPass(VulkanDevice * pDevice, const RenderPassDesc * pDesc)
  : Device(pDevice)
{
  Device->AddInternalRef();
  
  VkRenderPassCreateInfo Info = { VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO };

  std::vector<VkAttachmentReference> colorRefers;
  std::vector<VkAttachmentDescription> attachments;

  Info.attachmentCount = pDesc->colorAttachmentsCount + pDesc->pDepthStencilAttachment ? 1 : 0;
  attachments.resize(Info.attachmentCount);
  memset(attachments.data(), 0, Info.attachmentCount * sizeof(VkAttachmentDescription));

  uint32 colorId = 0;

  for (int i = 0; i < pDesc->colorAttachmentsCount; i++)
  {
    TextureDesc texDesc;
    pDesc->pColorAttachments[i].texture->GetDesc(&texDesc);
    attachments[i].format = ConvertPixelFormatToVulkanEnum(texDesc.format);
    attachments[i].loadOp = ConvertLoadActionToVulkanEnum(pDesc->pColorAttachments[i].loadAction);
    attachments[i].storeOp = ConvertStoreActionToVulkanEnum(pDesc->pColorAttachments[i].storeAction);
    attachments[i].initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    attachments[i].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    colorRefers.push_back({ colorId, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL });
    colorId++;
  }

  // Create default subpass
  VkSubpassDescription defaultSubpass = {
    0,                                //VkSubpassDescriptionFlags       flags;
    VK_PIPELINE_BIND_POINT_GRAPHICS,  //VkPipelineBindPoint             pipelineBindPoint;
    0,                                //uint32_t                        inputAttachmentCount;
    nullptr,                          //const VkAttachmentReference*    pInputAttachments;
    colorRefers.size(),               //uint32_t                        colorAttachmentCount;
    colorRefers.data(),               //const VkAttachmentReference*    pColorAttachments;
    nullptr,                          //const VkAttachmentReference*    pResolveAttachments;
                                      //const VkAttachmentReference*    pDepthStencilAttachment;
                                      //uint32_t                        preserveAttachmentCount;
                                      //const uint32_t*                 pPreserveAttachments;
  };

  VkAttachmentReference depthStencilRefer = { 1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };
  if (pDesc->pDepthStencilAttachment)
  {
    TextureDesc texDesc;
    pDesc->pDepthStencilAttachment->texture->GetDesc(&texDesc);
    attachments[pDesc->colorAttachmentsCount].format = ConvertPixelFormatToVulkanEnum(texDesc.format);
    attachments[pDesc->colorAttachmentsCount].stencilLoadOp = ConvertLoadActionToVulkanEnum(pDesc->pDepthStencilAttachment->loadAction);
    attachments[pDesc->colorAttachmentsCount].stencilStoreOp = ConvertStoreActionToVulkanEnum(pDesc->pDepthStencilAttachment->storeAction);
    attachments[pDesc->colorAttachmentsCount].initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    attachments[pDesc->colorAttachmentsCount].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    defaultSubpass.pDepthStencilAttachment = &depthStencilRefer;
  }

  Info.pAttachments = attachments.data();
  Info.subpassCount = 1;
  Info.pSubpasses = &defaultSubpass;

  VkSubpassDependency dependency = { 0 };
  dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
  dependency.dstSubpass = 0;
  dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  dependency.srcAccessMask = 0;
  dependency.dstAccessMask = 0;
  Info.dependencyCount = 1;
  Info.pDependencies = &dependency;

  CHECK(vkCreateRenderPass(Device->Handle, &Info, VULKAN_ALLOCATOR, &Handle));
}

VulkanRenderPass::~VulkanRenderPass()
{
  vkDestroyRenderPass(Device->Handle, Handle, VULKAN_ALLOCATOR);
  Device->ReleaseInternal();
}

Result VulkanDevice::CreateRenderPass(const RenderPassDesc * desc, RenderPass ** ppRenderpass)
{
  *ppRenderpass = new VulkanRenderPass(this, desc);
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
  return Result::Ok;
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
  Info = { VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO, nullptr, 0 };
  
  VulkanPipelineLayout* layout = static_cast<VulkanPipelineLayout*>(pLayout);
  Info.layout = layout->Handle;

  VulkanFunction1* computeFunction = static_cast<VulkanFunction1*>(pComputeFunc);
  Info.stage = computeFunction->StageInfo;
  CHECK(CreatePipelineCache(nullptr, 0));
  CHECK(CreatePipeline());
}

VulkanComputePipeline::~VulkanComputePipeline()
{
}

VulkanRenderPipeline::VulkanRenderPipeline(VulkanDevice * pDevice, const RenderPipelineDesc * pDesc, RenderPass * pRenderPass, PipelineLayout * pLayout)
  : VulkanRenderPipeline::Super(pDevice)
{
  Info = { VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO };

  InitShaderStages(pDesc);

  Info.stageCount = Stages.size();
  Info.pStages = Stages.data();

  Info.pRasterizationState = &RasterizationState;
  Info.pDepthStencilState = &DepthStencilState;
  Info.pColorBlendState = &BlendState;
  Info.pDynamicState = &DynamicState;
  Info.pInputAssemblyState = &InputAssemblyState;
  Info.pVertexInputState = &VertexInputState;
  Info.pMultisampleState = &MultisampleState;
  Info.pViewportState = &ViewportState;
  Info.pTessellationState = &TessellationState;

  CHECK(CreatePipelineCache(nullptr, 0));
  CHECK(CreatePipeline());
}

void VulkanRenderPipeline::InitShaderStages(const RenderPipelineDesc* pDesc)
{
  if (pDesc->vertexFunction)
  {
    auto vertShader = static_cast<VulkanFunction1*>(pDesc->vertexFunction);
    Stages.push_back(vertShader->StageInfo);
  }
  if (pDesc->pixelFunction)
  {
    auto pixelShader = static_cast<VulkanFunction1*>(pDesc->pixelFunction);
    Stages.push_back(pixelShader->StageInfo);
  }
}

VulkanRenderPipeline::~VulkanRenderPipeline()
{
}

Result VulkanRenderPipeline::GetDesc(RenderPipelineDesc * pDesc)
{
  return Result::Ok;
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
  if (Handle)
  {
    vkDestroyPipeline(OwningDevice->Handle, Handle, VULKAN_ALLOCATOR);
  }
  if (Cache)
  {
    vkDestroyPipelineCache(OwningDevice->Handle, Cache, VULKAN_ALLOCATOR);
  }
  OwningDevice->ReleaseInternal();
}

VulkanPipelineLibrary::VulkanPipelineLibrary(VulkanDevice * pDevice, const void * InData, uint64_t Size)
  : Device(pDevice), Handle(VK_NULL_HANDLE)
{
  Device->AddInternalRef();
  VkPipelineCacheCreateInfo Info = { VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO, nullptr, 0, Size, InData };
  CHECK(vkCreatePipelineCache(Device->Handle, &Info, VULKAN_ALLOCATOR, &Handle));
}

VulkanPipelineLibrary::~VulkanPipelineLibrary()
{
  vkDestroyPipelineCache(Device->Handle, Handle, VULKAN_ALLOCATOR);
  Device->ReleaseInternal();
}

void VulkanPipelineLibrary::StorePipeline(const char * key, Pipeline * pPipeline)
{
  if (pPipeline)
  {
    switch (pPipeline->Type())
    {
    case PipelineType::Graphics:
    {
      auto Gfx = static_cast<VulkanRenderPipeline*>(pPipeline);
      if (Gfx->Cache)
        CHECK(vkMergePipelineCaches(Device->Handle, Handle, 1, &Gfx->Cache));
      break;
    }
    case PipelineType::Compute:
    {
      auto Cpt = static_cast<VulkanComputePipeline*>(pPipeline);
      if (Cpt->Cache)
        CHECK(vkMergePipelineCaches(Device->Handle, Handle, 1, &Cpt->Cache));
      break;
    }
    }
  }
}

uint64_t VulkanPipelineLibrary::GetSerializedSize() const
{
  uint64_t Size = 0;
  CHECK(vkGetPipelineCacheData(Device->Handle, Handle, &Size, nullptr));
  return Size;
}

void VulkanPipelineLibrary::Serialize(void * pData, uint64_t Size)
{
  CHECK(vkGetPipelineCacheData(Device->Handle, Handle, &Size, pData));
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
//    pSetBindings[i].stageFlags = pDesc->pShaderBindings[i].visibility;
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

VulkanDescriptorLayout::VulkanDescriptorLayout(VulkanDevice * pDevice, VkDescriptorSetLayoutBinding * pBindings, int Count)
  : Device(pDevice)
{
  Device->AddInternalRef();
}

VulkanDescriptorLayout::~VulkanDescriptorLayout()
{
  Device->ReleaseInternal();
}

VulkanPipelineLayout::VulkanPipelineLayout(VulkanDevice * pDevice, const ngfx::PipelineLayoutDesc * pDesc)
  : Device(pDevice)
{
  Device->AddInternalRef();
  assert(pDesc && pDesc->shaderLayoutCount);
  VkPipelineLayoutCreateInfo Info = { VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };
  Info.setLayoutCount = pDesc->shaderLayoutCount;
  VkDescriptorSetLayout* pLayouts = (VkDescriptorSetLayout*)calloc(pDesc->shaderLayoutCount, sizeof(VkDescriptorSetLayout*));
  for(uint32_t i; i < pDesc->shaderLayoutCount; i++)
  {
    pLayouts[i] = static_cast<const VulkanShaderLayout*>(pDesc->pShaderLayout + i)->Handle;
  }
  Info.pSetLayouts = pLayouts;
  vkCreatePipelineLayout(Device->Handle, &Info, VULKAN_ALLOCATOR, &Handle);
}

VulkanPipelineLayout::~VulkanPipelineLayout()
{
  if(Handle)
  {
    vkDestroyPipelineLayout(Device->Handle, Handle, VULKAN_ALLOCATOR);
  }
  Device->ReleaseInternal();
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

VulkanDescriptorPool::VulkanDescriptorPool(VulkanDevice * pDevice, uint64_t MaxSet)
  : Device(pDevice)
{
  Device->AddInternalRef();
  struct DescriptorPoolSize 
  {
    VkDescriptorType    type;
    uint32_t            descriptorCount;
  };
  VkDescriptorPoolCreateInfo createInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO };
  createInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
  createInfo.maxSets = 1;
  createInfo.poolSizeCount = 0;
  createInfo.pPoolSizes = nullptr;
  CHECK(vkCreateDescriptorPool(Device->Handle, &createInfo, NULL, &Handle));
}

VulkanDescriptorPool::~VulkanDescriptorPool()
{
  vkDestroyDescriptorPool(Device->Handle, Handle, VULKAN_ALLOCATOR);
  Device->ReleaseInternal();
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
TCmdEncoder<T>::TCmdEncoder(VulkanCommandBuffer* pCmd, VkPipelineBindPoint Point)
  : OwningCommand(pCmd)
  , CurrentBindingPoint(Point)
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
  if (pPipelineState)
  {
    switch (pPipelineState->Type())
    {
    case PipelineType::Compute:
    {
      VulkanComputePipeline* pipeline = static_cast<VulkanComputePipeline*>(pPipelineState);
      vkCmdBindPipeline(OwningCommand->Handle, CurrentBindingPoint, pipeline->Handle);
      break;
    }
    case PipelineType::Graphics:
    {
      VulkanRenderPipeline* pipeline = static_cast<VulkanRenderPipeline*>(pPipelineState);
      vkCmdBindPipeline(OwningCommand->Handle, CurrentBindingPoint, pipeline->Handle);
      break;
    }
    }
  }
}

template<class T>
void TCmdEncoder<T>::SetBindingTable(BindingTable * pBindingTable)
{
  VulkanBindingTable* bindingTable = static_cast<VulkanBindingTable*>(pBindingTable);
  VkDescriptorSet sets[] = { bindingTable->Handle };
  vkCmdBindDescriptorSets(OwningCommand->Handle, CurrentBindingPoint,
    bindingTable->OwningLayout->Handle, 0, 1, sets, 0, nullptr);
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

void VulkanRenderEncoder::SetDepthBias(Float32 biasConst, Float32 biasClamp, Float32 biasSlope)
{
  vkCmdSetDepthBias(OwningCommand->Handle, biasConst, biasClamp, biasSlope);
}

void VulkanRenderEncoder::SetDepthBounds(Float32 minDepth, Float32 maxDepth)
{
  vkCmdSetDepthBounds(OwningCommand->Handle, minDepth, maxDepth);
}

void VulkanRenderEncoder::SetStencilReference(StencilFaceRef face, uint32_t value)
{
  vkCmdSetStencilReference(OwningCommand->Handle, (VkStencilFaceFlags)face, value);
}

void VulkanRenderEncoder::SetBlendConsts(Float32x4 constant)
{
//  vkCmdSetBlendConstants((const float[4])(&constant));
}

void VulkanRenderEncoder::SetLineWidth(Float32 width)
{
  vkCmdSetLineWidth(OwningCommand->Handle, width);
}

void VulkanRenderEncoder::SetIndexBuffer(Buffer * pIndexBuffer)
{
  VulkanBuffer* pBuffer = static_cast<VulkanBuffer*>(pIndexBuffer);
  vkCmdBindIndexBuffer(OwningCommand->Handle, pBuffer->GetHandle(), 0, VkIndexType::VK_INDEX_TYPE_UINT32);
}

void VulkanRenderEncoder::SetVertexBuffer(uint32_t slot, uint64_t offset, Buffer * pVertexBuffer)
{
//  vkCmdBindVertexBuffers()
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

VulkanLibrary1::VulkanLibrary1(VulkanDevice * pDevice, const void * pBlobData, uint64 Size)
  : Device(pDevice)
{
  Init(pBlobData, Size);
}

VulkanLibrary1::VulkanLibrary1(VulkanDevice * pDevice, const char * pFilePath)
  : Device(pDevice)
{
  MemMapFile MemFile;
  if (MemFile.Open(pFilePath, IORead))
  {
    Init(MemFile.FileData(), MemFile.GetSize());
    MemFile.Close();
  }
}

void VulkanLibrary1::Init(const void * pBlobData, uint64 Size)
{
  const char* pHead = reinterpret_cast<const char*>(pBlobData);
  // parse binary
  if (pHead[0] == 'V' && pHead[1] == 'K' && pHead[2] == 'B' && pHead[3] == 'C') // Binary Version
  {
    const uint32_t* pVersion = reinterpret_cast<const uint32_t*>(pHead + 4);
    const uint32_t* pEntryCount = reinterpret_cast<const uint32_t*>(pHead + 8);
    if (*pEntryCount > 0)
    {
      int curOffset = 12;
      int szEntryInfos = *pEntryCount * sizeof(EntryInfo);
      int spirvOffset = curOffset + szEntryInfos;
      for (int i = 0; i < *pEntryCount; i++)
      {
        const EntryInfo* pInfo = reinterpret_cast<const EntryInfo*>(pHead + curOffset);
        DataBlob[pInfo->Name].Stage = (ngfx::ShaderType)pInfo->ShaderType;
        curOffset += sizeof(EntryInfo);
        DataBlob[pInfo->Name].ByteCode.resize(pInfo->Size / sizeof(uint32_t));
        memcpy(DataBlob[pInfo->Name].ByteCode.data(), pHead + spirvOffset + pInfo->OffSet, pInfo->Size);
      }
    }
  }
  else  // Source Text Version
  {
    std::string ErrorInfo;
    CompileFromSource(CompileOption(), pHead, DataBlob, ErrorInfo);
  }
}

VulkanLibrary1::~VulkanLibrary1()
{
}

VulkanFunction1::VulkanFunction1(VulkanLibrary1 * pLibrary, const char * name)
  : Library(pLibrary)
  , StageInfo{ VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO }
  , ShaderModule(VK_NULL_HANDLE)
  , EntryName(name)
{
  Library->AddInternalRef();
  VkShaderModuleCreateInfo CreateInfo = {
    VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
  };
  Library->DataBlob[name].ByteCode;
  ShaderType = Library->DataBlob[name].Stage;
  CreateInfo.codeSize = sizeof(uint32_t) * Library->DataBlob[name].ByteCode.size();
  CreateInfo.pCode = Library->DataBlob[name].ByteCode.data();
  CHECK(vkCreateShaderModule(Library->Device->Handle, &CreateInfo, VULKAN_ALLOCATOR, &ShaderModule));
  StageInfo.module = ShaderModule;
  StageInfo.stage = ConvertShaderTypeToVulkanEnum(Library->DataBlob[name].Stage);
  StageInfo.pName = EntryName.c_str();
}

VulkanFunction1::~VulkanFunction1()
{
  if(ShaderModule)
  {
    vkDestroyShaderModule(Library->Device->Handle, ShaderModule, VULKAN_ALLOCATOR);
  }
  Library->ReleaseInternal();
}

ShaderType VulkanFunction1::Type() const
{
  return ShaderType::Compute;
}

const char * VulkanFunction1::Name() const
{
  return nullptr;
}

Result VulkanLibrary1::MakeFunction(const char * name, Function ** ppFunction)
{
  if (DataBlob.find(name) != DataBlob.end())
  {
    *ppFunction = new VulkanFunction1(this, name);
    return Result::Ok;
  }
  return Result::Failed;
}
