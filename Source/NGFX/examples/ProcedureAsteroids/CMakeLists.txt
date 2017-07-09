add_executable(ProcedureAsteroids main.cpp)
target_link_libraries(ProcedureAsteroids Core ngfx_vulkan)
set_target_properties(ProcedureAsteroids PROPERTIES FOLDER "ngfx/examples")