include_directories(${gtest_SOURCE_DIR}/include)
add_executable(GTestNGFX TestVk.cpp)
target_link_libraries(GTestNGFX ngfxu ${GTEST_LIBRARIES} Kaleido3D.Core)
set_target_properties(GTestNGFX PROPERTIES FOLDER "ngfx/gtest")