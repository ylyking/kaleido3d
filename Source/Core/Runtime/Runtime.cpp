#include "Kaleido3D.h"
#include "Runtime.h"
#include "Class.h"
#include "Enum.h"
#include "Field.h"
#include "Method.h"

K3D_COMMON_NS
{
StackData::StackData() : m_CurOffset(0)
{
}
StackData::~StackData()
{
}
Field::Field(String Name, int InternalOffset)
: m_InternalOffset(InternalOffset)
, m_Name(Name)
{}
Field::~Field()
{}
Method::Method(String Name, int InternalOffset)
: m_InternalOffset(InternalOffset)
, m_Name(Name)
{}
Method::~Method()
{
}
Class::Class(String Name) : m_Name(Name)
{}
Class::~Class()
{}
Field*
Class::FindField(const char* Name, const char* Signature)
{
    return nullptr;
}
Method*
Class::FindMethod(const char* Name, const char* Signature)
{
    return nullptr;
}
uint32 Class::GetNumFields() const
{
    return m_Fields.Count();
}
uint32 Class::GetNumMethods()const
{
    return m_Methods.Count();
}
bool Class::RegisterMethod(const char *MethodName, const char *Signature)
{
    return true;
}
bool Class::RegisterField(const char *MethodName, const char *Signature, int InternalOffset)
{
    auto Fld = new Field(MethodName, InternalOffset);
    m_Fields.Append(Fld);
    return true;
}
Runtime::Runtime()
{}
Runtime::~Runtime()
{}
Class*
Runtime::FindClass(const char* Name)
{
    return nullptr;
}
Method*
Runtime::FindMethod(const char* Name, const char* Signature)
{
    return nullptr;
}
bool
Runtime::Register()
{
    return false;
}

}
