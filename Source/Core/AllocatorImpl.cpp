#include "Kaleido3D.h"
#include <KTL/Allocator.hpp>

K3D_API void* __k3d_malloc__(size_t sizeOfObj)
{
	return malloc(sizeOfObj);
}

K3D_API void __k3d_free__(void *p, size_t sizeOfObj)
{
	free(p);
}

K3D_API void* operator new[](size_t size, const char* pName)
{
	return __k3d_malloc__(size);
}

K3D_API void* operator new(size_t Size, const char* _ClassName, const char* _SourceFile, int _SourceLine)
{
  return __k3d_malloc__(Size);
}

K3D_API void operator delete(void* _Ptr, const char* _SourceFile, int _SourceLine)
{
    __k3d_free__(_Ptr, -1);
}

//K3D_API void* operator new[](size_t size)
//{
//    return __k3d_malloc__(size);
//}

//K3D_API void operator delete(void* _Ptr)
//{
//    __k3d_free__(_Ptr, -1);
//}