#pragma once
#ifndef __LogUtil_h__
#define __LogUtil_h__

#include <KTL/Singleton.hpp>
#include <Interface/ILog.h>


namespace k3d
{
	extern K3D_API void Log(ELogLevel const & Lv, const char* tag, const char *fmt, ...);
}

#if !K3DPLATFORM_OS_WINDOWS
#define __debugbreak __builtin_trap
#endif

#define K3D_STRINGIFY(x) #x
#define K3D_STRINGIFY_BUILTIN(x) K3D_STRINGIFY(x)
#define K3D_ASSERT(isFalse, ...) \
	if (!(bool)(isFalse)) { \
		::k3d::Log(::k3d::ELogLevel::Error,"Assert","\nAssertion failed in " K3D_STRINGIFY_BUILTIN( __FILE__ ) " @ " K3D_STRINGIFY_BUILTIN( __LINE__ ) "\n"); \
		::k3d::Log(::k3d::ELogLevel::Error,"Assert","\'" #isFalse "\' is false"); \
		::k3d::Log(::k3d::ELogLevel::Error,"Assert", "args is" ##__VA_ARGS__); \
		__debugbreak(); \
    }


#define KLOG(Level, TAG, ...) \
	::k3d::Log(::k3d::ELogLevel::Level, #TAG, __VA_ARGS__);

#endif
