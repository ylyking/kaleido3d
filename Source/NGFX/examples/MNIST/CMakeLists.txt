add_executable(MNIST main.cpp)
target_link_libraries(MNIST Core ngfx_vulkan)
set_target_properties(MNIST PROPERTIES FOLDER "ngfx/examples")