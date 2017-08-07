#include "ngfxu.h"

using namespace ngfx;

namespace ngfxu
{
  RenderPipelineDesc CreateDefaultRenderPipelineDesc()
  {
    RenderPipelineDesc d;

    d.rasterState.SetCullMode(CullMode::Back)
      .SetDepthBias(0)
      .SetFillMode(FillMode::Solid)
      .SetFrontCCW(true)
      .SetMultiSampleEnable(false);

    d.depthStencil.SetDepthEnable(true)
      .SetDepthFunc(ComparisonFunction::Greater)
      .SetDepthWriteMask(DepthWriteMask::All)
      .SetStencilEnable(true);

    d.blendState.SetEnable(false);

    d.SetPrimitiveTopology(PrimitiveType::Triangles);

    return d;
  }
}