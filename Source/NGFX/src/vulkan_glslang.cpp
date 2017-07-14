#include "vulkan_glslang.h"
using namespace ngfx;

#include <glslang/GlslangToSpv.h>
using namespace glslang;

EShLanguage ConvertShaderTypeToGlslangEnum(ngfx::ShaderType const& e) {
  switch (e) {
  case ShaderType::Vertex:
    return EShLangVertex;
  case ShaderType::Fragment:
    return EShLangFragment;
  case ShaderType::Compute:
    return EShLangCompute;
  case ShaderType::Geometry:
    return EShLangGeometry;
  case ShaderType::TessailationEval:
    return EShLangTessEvaluation;
  case ShaderType::TessailationControl:
    return EShLangTessControl;
  }
}

void sInitializeGlSlang()
{
  static bool sGlSlangIntialized = false;
  if (!sGlSlangIntialized) {
    glslang::InitializeProcess();
    sGlSlangIntialized = true;
  }
}

void sFinializeGlSlang()
{
  static bool sGlSlangFinalized = false;
  if (!sGlSlangFinalized) {
    glslang::FinalizeProcess();
    sGlSlangFinalized = true;
  }
}

void initResources(TBuiltInResource &resources)
{
  resources.maxLights = 32;
  resources.maxClipPlanes = 6;
  resources.maxTextureUnits = 32;
  resources.maxTextureCoords = 32;
  resources.maxVertexAttribs = 64;
  resources.maxVertexUniformComponents = 4096;
  resources.maxVaryingFloats = 64;
  resources.maxVertexTextureImageUnits = 32;
  resources.maxCombinedTextureImageUnits = 80;
  resources.maxTextureImageUnits = 32;
  resources.maxFragmentUniformComponents = 4096;
  resources.maxDrawBuffers = 32;
  resources.maxVertexUniformVectors = 128;
  resources.maxVaryingVectors = 8;
  resources.maxFragmentUniformVectors = 16;
  resources.maxVertexOutputVectors = 16;
  resources.maxFragmentInputVectors = 15;
  resources.minProgramTexelOffset = -8;
  resources.maxProgramTexelOffset = 7;
  resources.maxClipDistances = 8;
  resources.maxComputeWorkGroupCountX = 65535;
  resources.maxComputeWorkGroupCountY = 65535;
  resources.maxComputeWorkGroupCountZ = 65535;
  resources.maxComputeWorkGroupSizeX = 1024;
  resources.maxComputeWorkGroupSizeY = 1024;
  resources.maxComputeWorkGroupSizeZ = 64;
  resources.maxComputeUniformComponents = 1024;
  resources.maxComputeTextureImageUnits = 16;
  resources.maxComputeImageUniforms = 8;
  resources.maxComputeAtomicCounters = 8;
  resources.maxComputeAtomicCounterBuffers = 1;
  resources.maxVaryingComponents = 60;
  resources.maxVertexOutputComponents = 64;
  resources.maxGeometryInputComponents = 64;
  resources.maxGeometryOutputComponents = 128;
  resources.maxFragmentInputComponents = 128;
  resources.maxImageUnits = 8;
  resources.maxCombinedImageUnitsAndFragmentOutputs = 8;
  resources.maxCombinedShaderOutputResources = 8;
  resources.maxImageSamples = 0;
  resources.maxVertexImageUniforms = 0;
  resources.maxTessControlImageUniforms = 0;
  resources.maxTessEvaluationImageUniforms = 0;
  resources.maxGeometryImageUniforms = 0;
  resources.maxFragmentImageUniforms = 8;
  resources.maxCombinedImageUniforms = 8;
  resources.maxGeometryTextureImageUnits = 16;
  resources.maxGeometryOutputVertices = 256;
  resources.maxGeometryTotalOutputComponents = 1024;
  resources.maxGeometryUniformComponents = 1024;
  resources.maxGeometryVaryingComponents = 64;
  resources.maxTessControlInputComponents = 128;
  resources.maxTessControlOutputComponents = 128;
  resources.maxTessControlTextureImageUnits = 16;
  resources.maxTessControlUniformComponents = 1024;
  resources.maxTessControlTotalOutputComponents = 4096;
  resources.maxTessEvaluationInputComponents = 128;
  resources.maxTessEvaluationOutputComponents = 128;
  resources.maxTessEvaluationTextureImageUnits = 16;
  resources.maxTessEvaluationUniformComponents = 1024;
  resources.maxTessPatchComponents = 120;
  resources.maxPatchVertices = 32;
  resources.maxTessGenLevel = 64;
  resources.maxViewports = 16;
  resources.maxVertexAtomicCounters = 0;
  resources.maxTessControlAtomicCounters = 0;
  resources.maxTessEvaluationAtomicCounters = 0;
  resources.maxGeometryAtomicCounters = 0;
  resources.maxFragmentAtomicCounters = 8;
  resources.maxCombinedAtomicCounters = 8;
  resources.maxAtomicCounterBindings = 1;
  resources.maxVertexAtomicCounterBuffers = 0;
  resources.maxTessControlAtomicCounterBuffers = 0;
  resources.maxTessEvaluationAtomicCounterBuffers = 0;
  resources.maxGeometryAtomicCounterBuffers = 0;
  resources.maxFragmentAtomicCounterBuffers = 1;
  resources.maxCombinedAtomicCounterBuffers = 1;
  resources.maxAtomicCounterBufferSize = 16384;
  resources.maxTransformFeedbackBuffers = 4;
  resources.maxTransformFeedbackInterleavedComponents = 64;
  resources.maxCullDistances = 8;
  resources.maxCombinedClipAndCullDistances = 8;
  resources.maxSamples = 4;
  resources.limits.nonInductiveForLoops = 1;
  resources.limits.whileLoops = 1;
  resources.limits.doWhileLoops = 1;
  resources.limits.generalUniformIndexing = 1;
  resources.limits.generalAttributeMatrixVectorIndexing = 1;
  resources.limits.generalVaryingIndexing = 1;
  resources.limits.generalSamplerIndexing = 1;
  resources.limits.generalVariableIndexing = 1;
  resources.limits.generalConstantMatrixVectorIndexing = 1;
}

GlslangCompiler::GlslangCompiler()
{
  sInitializeGlSlang();
}

GlslangCompiler::~GlslangCompiler()
{
  sFinializeGlSlang();
}

ngfx::Result GlslangCompiler::Compile(const ngfx::ShaderOption * option, void * pData, uint32_t size, ngfx::Function ** output)
{
  if (option->format == ShaderFormat::Text)
  {
    EShMessages messages = (EShMessages)(EShMsgSpvRules | EShMsgVulkanRules);
    switch (option->language)
    {
    case ShaderLang::GLSL:
      break;
    case ShaderLang::HLSL:
      messages =
        (EShMessages)(EShMsgVulkanRules | EShMsgSpvRules | EShMsgReadHlsl);
      break;
    default:
      break;
    }
    TProgram& program = *new TProgram;
    TBuiltInResource Resources;
    initResources(Resources);
    const char* shaderStrings[1];
    EShLanguage stage = ConvertShaderTypeToGlslangEnum(option->stage);
    TShader* shader = new TShader(stage);

    shaderStrings[0] = (const char*)pData;
    shader->setStrings(shaderStrings, 1);
    shader->setEntryPoint(option->entryName);

    if (!shader->parse(&Resources, 100, false, messages))
    {
      puts(shader->getInfoLog());
      puts(shader->getInfoDebugLog());
      return Result::Failed;
    }
    program.addShader(shader);
    if (!program.link(messages))
    {
      puts(program.getInfoLog());
      puts(program.getInfoDebugLog());
      return Result::Failed;
    }

    if (program.buildReflection())
    {
      auto function = new VulkanFunction();
      function->EntryName = option->entryName;
      GlslangToSpv(*program.getIntermediate(stage), function->ByteCodes);
      function->ShaderModuleInfo.codeSize = function->ByteCodes.size() * sizeof(uint32_t);
      function->ShaderModuleInfo.pCode = function->ByteCodes.data();
      *output = function;
    }
    else
    {
      return Result::Failed;
    }
  }
  else // byteCode reflection
  {
  }

  return Result::Ok;
}

ngfx::Result GlslangCompiler::Reflect(void * pData, uint32_t size)
{
  return Result::Ok;
}

VulkanFunction::VulkanFunction()
{
}

VulkanFunction::~VulkanFunction()
{
  if (ShaderModule)
  {
    //vkDestroyShaderModule(VK_NULL_HANDLE, ShaderModule, nullptr);
  }
}

ngfx::ShaderType VulkanFunction::Type()
{
  return ShaderType;
}

const char * VulkanFunction::Name()
{
  return EntryName.c_str();
}

VkPipelineShaderStageCreateInfo* 
VulkanFunction::GetPipelineStageInfo()
{
  StageInfo.module = ShaderModule;
  StageInfo.pName = EntryName.c_str();
  StageInfo.flags;
  StageInfo.stage;
  //StageInfo.pSpecializationInfo;
  return &StageInfo;
}
