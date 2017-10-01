#pragma once
#include <KTL/DynArray.hpp>
K3D_COMMON_NS
{
class Field;
class Method;

class K3D_API Class
{
public:
    Class(String Name);
    ~Class();

    Method* FindMethod(const char* Name, const char* Signature);
    Field*  FindField(const char* Name, const char* Signature);

    uint32 GetNumFields() const;
    uint32 GetNumMethods() const;

    bool RegisterMethod(const char* MethodName, const char* Signature);
    bool RegisterField(const char* MethodName, const char* Signature, int InternalOffset);
private:
    String              m_Name;
    DynArray<Field*>    m_Fields;
    DynArray<Method*>   m_Methods;
};
}
