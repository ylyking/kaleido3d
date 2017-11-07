#include "CoreMinimal.h"
#include <stdlib.h>

namespace k3d
{

    class SystemAllocator : public IAllocatorAdapter
    {
    public:
        SystemAllocator()
        {}
        ~SystemAllocator()
        {}
        const char* GetName() const override { return "LibCAllocator"; }
        void* Alloc(size_t SzToAlloc, int Alignment, int AlignOffset, int Flags, const char* AllocInfo) override;
        void  DeAlloc(void* Ptr) override;
    };

    void * SystemAllocator::Alloc(size_t SzToAlloc, int Alignment, int AlignOffset, int Flags, const char * AllocInfo)
    {
        if (Alignment == 0)
            return malloc(SzToAlloc);
        else
#if K3DPLATFORM_OS_WINDOWS
            return _aligned_malloc(SzToAlloc, Alignment);
#else
        {
            void *pAddr = nullptr;
            posix_memalign(&pAddr, Alignment, SzToAlloc);
            return pAddr;
        }
#endif
    }

    void SystemAllocator::DeAlloc(void * Ptr)
    {
        free(Ptr);
    }

    IAllocatorAdapter& GetDefaultAllocator()
    {
        static SystemAllocator SysAllc;
        return SysAllc;
    }
}

K3D_CORE_API void* __k3d_malloc__(size_t sizeOfObj)
{
	return malloc(sizeOfObj);
}

K3D_CORE_API void __k3d_free__(void *p, size_t sizeOfObj)
{
	free(p);
}

K3D_CORE_API void* operator new[](size_t size, const char* pName)
{
	return __k3d_malloc__(size);
}

K3D_CORE_API void* operator new(size_t Size, const char* _ClassName, const char* _SourceFile, int _SourceLine)
{
  return __k3d_malloc__(Size);
}

K3D_CORE_API void operator delete(void* _Ptr, const char* _SourceFile, int _SourceLine)
{
    __k3d_free__(_Ptr, 0);
}

//K3D_CORE_API void* operator new[](size_t size)
//{
//    return __k3d_malloc__(size);
//}

//K3D_CORE_API void operator delete(void* _Ptr)
//{
//    __k3d_free__(_Ptr, -1);
//}
