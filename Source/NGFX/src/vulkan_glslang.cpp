#include "vulkan_glslang.h"
using namespace ngfx;

#include <glslang/GlslangToSpv.h>
#include <spirv_cross/spirv_cross.hpp>
//#include <spirv_cross/spirv_glsl.hpp>

using namespace glslang;
using namespace spirv_cross;

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

ngfx::Result GlslangCompiler::Reflect(void * pData, uint32_t size, ngfx::Reflection ** ppResult)
{
  *ppResult = new SPIRVCrossReflection(pData, size);
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

class SPIRVVariable : public ngfx::Variable
{
public:
  const char *    Name() override { return name.c_str(); }
  ArgumentAccess  Access() override;
  VariableType *  Type() override { return type; }
  uint32_t        Index() override { return id; }
  bool            Active() override { return active; }

  std::string     name;
  VariableType*   type;
  uint32_t        id;
  bool            active;
};

class SPIRVArrayType : public ngfx::ArrayType
{
public:
  DataType      GetType() override { return DataType::Array; }

  uint32_t      Length() override { return length; }
  DataType      ElementType() override { return elementType; }
  uint32_t      Stride() override { return stride; }
  VariableType* Elements() override { return elementType_; }

  uint32_t      length;
  uint32_t      stride;
  DataType      elementType;
  VariableType* elementType_;
};

class SPIRVStructType : public ngfx::StructType
{
public:
  DataType      GetType() override { return DataType::Struct; }

};

class SPIRVPointerType : public ngfx::PointerType
{
public:
  DataType        GetType() override { return DataType::Pointer; }

  ArgumentAccess  Access() override { return access; }
  uint32_t        Alignment() override { return alignment; }
  uint32_t        DataSize() override { return dataSize; }
  DataType        ElementType() override { return dataType; }

  SPIRVPointerType() = default;

  ArgumentAccess  access = ArgumentAccess::ReadOnly;
  uint32_t        alignment = 0;
  uint32_t        dataSize = 0;
  DataType        dataType = DataType::Pointer;
};

class SPIRVTextureType : public ngfx::TextureReferType
{
public:
  DataType          GetType() override { return DataType::Texture; }

  ArgumentAccess    Access() override { return access; }

  ArgumentAccess    access;
  DataType          dataType;
  TextureDimension  dim;
};

SPIRVCrossReflection::SPIRVCrossReflection(void* pData, uint32_t size)
  : m_Reflector(nullptr)
{
  m_Reflector = new spirv_cross::Compiler(reinterpret_cast<const uint32_t*>(pData), size/4);

  for (auto& res : m_Reflector->get_shader_resources().storage_buffers) 
  {
    SPIRVVariable* var  = new SPIRVVariable;
    var->name           = m_Reflector->get_name(res.id);
    var->id             = m_Reflector->get_decoration(res.id, spv::DecorationBinding);
    auto type           = m_Reflector->get_type(res.type_id);
    //uint32_t bindingSet = m_Reflector->get_decoration(res.id, spv::DecorationDescriptorSet);
    if (type.pointer)
    {
      SPIRVPointerType* pType = new SPIRVPointerType;
      auto b_type = m_Reflector->get_type(res.base_type_id);
      switch (b_type.basetype)
      {
      case SPIRType::Struct:
        // expand struct
        auto r = m_Reflector->get_type(b_type.member_types[0]);
        break;
      }
      var->type = pType;
    }
    else 
    {
      SPIRVArrayType* aType = new SPIRVArrayType;
      var->active;
      var->type = aType;
    }
    m_Vars.push_back(var);
  }


}

SPIRVCrossReflection::~SPIRVCrossReflection()
{
  if (m_Reflector)
  {
    delete m_Reflector;
    m_Reflector = nullptr;
  }
}

ngfx::ShaderType SPIRVCrossReflection::GetStage()
{
  return ngfx::ShaderType();
}

uint32_t SPIRVCrossReflection::VariableCount()
{
  return m_Vars.size();
}

ngfx::Variable** SPIRVCrossReflection::Variables()
{
  return m_Vars.data();
}

ArgumentAccess SPIRVVariable::Access()
{
  switch (type->GetType())
  {
  case DataType::Array:
    //static_cast<SPIRVArrayType*>(type)->
    break;
  case DataType::Struct:
    //static_cast<SPIRVStructType*>(type)->
    break;
  case DataType::Pointer:
    return static_cast<SPIRVPointerType*>(type)->access;
  case DataType::Texture:
    return static_cast<SPIRVTextureType*>(type)->access;
  }
  return ArgumentAccess::ReadOnly;
}
