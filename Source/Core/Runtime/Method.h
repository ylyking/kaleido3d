#pragma once
#include "Runtime.h"
K3D_COMMON_NS
{
class K3D_API Method
{
public:
    Method(String Name, int InternalOffset);
    ~Method();

    void Invoke(StackData& Data);

private:
    int         m_InternalOffset;
    String      m_Name;
    NativeFunc  m_NativeFunc;
    int         m_NumParams;
};
}
