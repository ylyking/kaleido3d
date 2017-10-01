#pragma once

#include "Config/Config.h"
#include <string>

namespace k3d
{
    class K3D_API StringUtil
    {
    public:
#if K3DPLATFORM_OS_WINDOWS
		static void CharToWchar(const char *chr, wchar_t *wchar, int size);
		static void	WCharToChar(const wchar_t *wchr, char *wchar, int size);
#endif
	};
}
