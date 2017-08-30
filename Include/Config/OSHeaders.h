#ifndef __OSHeaders_h__
#define __OSHeaders_h__

#include "Config.h"
#if K3DPLATFORM_OS_WINDOWS
    #include <Windows.h>
#if K3DPLATFORM_OS_WINUWP
    #include <WinSock2.h>
#endif
	#include "Shlwapi.h"
	#pragma comment(lib, "Ws2_32.lib")
	#pragma comment(lib, "User32.lib")
	#pragma comment(lib, "shlwapi.lib")
#elif K3DPLATFORM_OS_UNIX
    #include <unistd.h>
    #include <dlfcn.h>
    #include <cstring>
    #include <errno.h>
    #include <cmath>
    #include <sys/types.h>
    #include <sys/stat.h>
    #include <dirent.h>
    #include <time.h>
/** Socket **/
    #include <sys/socket.h>
    #include <arpa/inet.h>
    #include <pthread.h>
/******/
    #if K3DPLATFORM_OS_IOS
        #include <sys/fcntl.h>
    #else
        #include <fcntl.h>
    #endif
    #include <sys/mman.h>
#endif

#if K3DPLATFORM_OS_ANDROID
#include <android/log.h>
#include <sys/prctl.h>
#endif

#endif
