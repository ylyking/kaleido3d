#include "Kaleido3D.h"
#include <Config/OSHeaders.h>
#include "StringUtils.h"

#if K3DPLATFORM_OS_WIN
	#include <tchar.h>
	#include <strsafe.h>
#endif


namespace k3d {

#if K3DPLATFORM_OS_WINDOWS
	
	void StringUtil::CharToWchar(const char *chr, wchar_t *wchar, int size)
	{
		::MultiByteToWideChar(CP_ACP, 0, chr, (int)strlen(chr) + 1, wchar, size / sizeof(wchar[0]));
	}


	void StringUtil::WCharToChar(const wchar_t *wchr, char *cchar, int size)
	{
		int nlength = (int)wcslen(wchr);
		int nbytes = WideCharToMultiByte(CP_ACP, 0, wchr, nlength, NULL, 0, NULL, NULL);
		if (nbytes>size)   nbytes = size;
		WideCharToMultiByte(0, 0, wchr, nlength, cchar, nbytes, NULL, NULL);
	}

	std::string GetLastWin32Error()
	{
		LPVOID lpMsgBuf; 
		DWORD dw = GetLastError(); 
		FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, 
						NULL, dw, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&lpMsgBuf, 0, NULL); 
		std::string lastError((LPCSTR)lpMsgBuf);
		LocalFree(lpMsgBuf);
		return lastError;
	}
#endif

}
