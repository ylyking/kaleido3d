include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/Engine)

set(ENGINE_SRC
    Engine/MiniEngine.h
    Engine/MiniEngine.cpp
    Engine/Component/Primitive.h
    Engine/Component/Primitive.cpp
    Engine/Material/Material.h
    Engine/Material/Material.cpp
)
source_group(Engine FILES ${ENGINE_SRC})

set(RENDERER_SRC
    Renderer/Renderer.h
    Renderer/Renderer.cpp
    Renderer/RenderPass.h
    Renderer/RenderPass.cpp
    Renderer/RenderProxy.h    
    Renderer/RenderProxy.cpp
    Renderer/VertexFactory.h
    Renderer/VertexFactory.cpp
)
source_group(Renderer FILES ${RENDERER_SRC})

add_library(MiniEngine ${ENGINE_SRC} ${RENDERER_SRC})
add_precompiled_header(MiniEngine "MiniEngine.h" "${CMAKE_CURRENT_SOURCE_DIR}/Engine" "${CMAKE_CURRENT_SOURCE_DIR}/Engine/MiniEngine.cpp")
target_link_libraries(MiniEngine ngfx_vulkan)
set_target_properties(MiniEngine PROPERTIES FOLDER "ngfx/examples")