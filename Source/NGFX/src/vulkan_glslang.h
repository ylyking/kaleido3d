#pragma once
#include "ngfx.h"

#ifdef BUILD_VULKAN_GLSLANG
#define V_API __declspec(dllexport)
#else
#define V_API __declspec(dllimport)
#endif

class V_API GlslangCompiler : public ngfx::Compiler
{
public:
  GlslangCompiler();
  ~GlslangCompiler();
  ngfx::Result Compile(const ngfx::ShaderOption * option, void * pData, uint32_t size, ngfx::Function ** output) override;
  ngfx::Result Reflect(void * pData, uint32_t size) override;
};