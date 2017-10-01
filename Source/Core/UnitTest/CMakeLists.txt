if(BUILD_SHARED)
	add_definitions(-DBUILD_SHARED_LIB -DBUILD_WITH_PLUGIN)
endif()
################################## Unit Test For RHI #####################################

include_directories(
    ${FREETYPE2_INCLUDE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

macro(add_core_unittest TARGET_NAME)
    if(ANDROID)
        add_android_app(${TARGET_NAME}
            SRCS ${ARGN} ${Kaleido3D_SOURCE_DIR}/Platform/Android/jni/RendererView.cpp ${Kaleido3D_SOURCE_DIR}/Source/Platform/Android/jni/RendererView_JNI.cpp
            LIBS ${PLATFORM_LIBS} Kaleido3D.Core)
    elseif(WIN32)
        add_executable(${TARGET_NAME} ${ARGN} ${Kaleido3D_SOURCE_DIR}/Source/Platform/Microsoft/Win32/win32icon.rc)
        target_link_libraries(${TARGET_NAME} ${PLATFORM_LIBS} Kaleido3D.Core)
        set_target_properties(${TARGET_NAME} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
    elseif(MACOS)
        add_mac_app(${TARGET_NAME} 
            SRCS ${ARGN} 
            LIBS ${PLATFORM_LIBS} Kaleido3D.Core
            PLUGINS RHI_Metal KawaLog ShaderCompiler)
    elseif(IOS)
        add_ios_app(${TARGET_NAME}
            SRCS ${ARGN} "${Kaleido3D_SOURCE_DIR}/Source/Platform/Apple/iOS/App.mm"
            LIBS ${PLATFORM_LIBS} 
            PDN ${TARGET_NAME}
            OS 8.0
            LDPATH "@executable_path")
    elseif(UNIX)
        add_executable(${TARGET_NAME} ${ARGN})
        target_link_libraries(${TARGET_NAME} ${PLATFORM_LIBS} )
    else()
        add_executable(${TARGET_NAME} ${ARGN})
    endif()
    set_target_properties(${TARGET_NAME} PROPERTIES FOLDER "Unit Test/Core")
endmacro()


add_core_unittest(
	Core-UnitTest-1.Os
	UTCore.Os.cpp
)

add_core_unittest(
	Core-UnitTest-2.WebSocket
	UTCore.WebSocket.cpp
)

add_core_unittest(
	Core-UnitTest-3.SharedPtr
	UTKTL.SharedPtr.cpp
)


add_core_unittest(
	Core-UnitTest-4.String
	UTCore.String.cpp
)

add_core_unittest(
	Core-UnitTest-5.DynArray
	UTKTL.DynArray.cpp
)

add_core_unittest(
	Core-UnitTest-6.RuntimeClass
	UTCore.RuntimeClass.cpp
)

add_core_unittest(
	Core-UnitTest-7.HashMap
	UTKTL.HashMap.cpp
)

add_core_unittest(
	Core-UnitTest-8.Functional
	UTKTL.Functional.cpp
)

add_core_unittest(
	Core-UnitTest-9.Math
	UTCore.Math.cpp
)