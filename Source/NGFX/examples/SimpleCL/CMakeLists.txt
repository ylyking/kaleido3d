add_executable(SimpleCL main.cpp)
target_link_libraries(SimpleCL Core ngfx_vulkan)
set_target_properties(SimpleCL PROPERTIES FOLDER "ngfx/examples")