#pragma once
#include "ngfx.h"
#include <vulkan/vulkan.h>
#include <string>
#include <vector>
#include <unordered_map>

#ifdef BUILD_VULKAN_GLSLANG
#if _MSC_VER
#define V_API __declspec(dllexport)
#else
#define V_API __attribute__((visibility("default")))
#endif
#else
#if _MSC_VER
#define V_API __declspec(dllimport)
#else
#define V_API
#endif
#endif

using ByteCode = std::vector<uint32_t>;

struct FunctionData
{
  ByteCode          ByteCodes;
  ngfx::ShaderType  Stage;
};

struct EntryInfo
{
  char      Name[128];
  char      Entry[128];
  uint32_t  ShaderType;
  uint32_t  Size;
  uint32_t  OffSet;
};

using FunctionMap = std::unordered_map<std::string, FunctionData>;

extern V_API 
ngfx::Result CompileFromSource(const ngfx::CompileOption& Opt, const char* pSource, FunctionMap& FuncMap, std::string& ErrorInfo);

extern V_API
ngfx::Result SerializeLibrary(const FunctionMap& Data, const char* Path);