set(NGFX_HEADER ${Kaleido3D_SOURCE_DIR}/Source/NGFX/src/ngfx.h)
set(NGFX_YML ${Kaleido3D_SOURCE_DIR}/Tools/RHIGenerator/ngfx.yml)

set_source_files_properties(${NGFX_HEADER}
	PROPERTIES GENERATED TRUE)

add_library(ngfx_vulkan_glslang SHARED 
vulkan_glslang.h
vulkan_glslang.cpp
)
target_compile_definitions(ngfx_vulkan_glslang PRIVATE BUILD_VULKAN_GLSLANG=1)
target_link_libraries(ngfx_vulkan_glslang ${GLSLANG_LIBRARIES})
set_target_properties(ngfx_vulkan_glslang PROPERTIES FOLDER "ngfx")

add_library(ngfx_vulkan SHARED 
${NGFX_HEADER}
${NGFX_YML}
vulkan.cpp
)
target_compile_definitions(ngfx_vulkan PRIVATE BUILD_SHARED_LIB=1)

add_custom_command(TARGET ngfx_vulkan PRE_BUILD COMMAND
	${PYTHON_EXECUTABLE} ${Kaleido3D_SOURCE_DIR}/Tools/RHIGenerator/run.py -o 
	${NGFX_HEADER} ${NGFX_YML})

target_link_libraries(ngfx_vulkan Core ngfx_vulkan_glslang ${VULKAN_LIB})
set_target_properties(ngfx_vulkan PROPERTIES FOLDER "ngfx")