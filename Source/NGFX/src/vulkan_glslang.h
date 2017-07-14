#pragma once
#include "ngfx.h"
#include <vulkan/vulkan.h>
#include <string>
#include <vector>

#ifdef BUILD_VULKAN_GLSLANG
#define V_API __declspec(dllexport)
#else
#define V_API __declspec(dllimport)
#endif

using ByteCode = std::vector<uint32_t>;

class V_API VulkanFunction : public ngfx::Function
{
public:
  VulkanFunction();
  ~VulkanFunction();
  ngfx::ShaderType Type() override;
  const char * Name() override;

  VkPipelineShaderStageCreateInfo* GetPipelineStageInfo();

  ByteCode                  ByteCodes;
  std::string               EntryName;
  std::string               Source;
  ngfx::ShaderType          ShaderType;
  VkShaderModule            ShaderModule = VK_NULL_HANDLE;
  VkPipelineShaderStageCreateInfo StageInfo = { VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
  VkShaderModuleCreateInfo  ShaderModuleInfo = { VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };
};

class V_API GlslangCompiler : public ngfx::Compiler
{
public:
  GlslangCompiler();
  ~GlslangCompiler();
  ngfx::Result Compile(const ngfx::ShaderOption * option, void * pData, uint32_t size, ngfx::Function ** output) override;
  ngfx::Result Reflect(void * pData, uint32_t size) override;
};