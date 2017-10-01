#pragma once
#include <KTL/String.hpp>
#include "Object.h"
K3D_COMMON_NS
{
class Class;
class Method;

class K3D_API StackData
{
public:
    StackData();
    ~StackData();

    template <typename T>
    T* Read()
    {
        T* CurPtr = reinterpret_cast<T*>(m_Data + m_CurOffset);
        m_CurOffset += sizeof(T);
        return CurPtr;
    }

    IObject* ReadObject()
    {
        IObject* CurPtr = reinterpret_cast<IObject*>(m_Data + m_CurOffset);
        m_CurOffset += sizeof(IObject*);
        return CurPtr;
    }

private:
    uint64 m_CurOffset;
    uint64 m_StackSize;
    uint8* m_Data;
};

typedef void (IObject::*NativeFunc)(StackData& Data);

class K3D_API Runtime
{
public:
    Runtime();
    ~Runtime();

    Class*  FindClass(const char* Name);
    Method* FindMethod(const char* Name, const char* Signature);

    bool    Register();

private:
};
}