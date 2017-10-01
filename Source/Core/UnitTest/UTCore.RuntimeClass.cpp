#include <Kaleido3D.h>
#include <Core/Runtime/Runtime.h>
#include <Core/Runtime/Class.h>
#include <Core/Runtime/Field.h>

#if K3DPLATFORM_OS_WIN
#pragma comment(linker,"/subsystem:console")
#endif

k3d::Runtime CoreRT;

namespace k3d
{
    class TestObject : public IObject
    {
    public:
        
        static Class* NewClass()
        {
            Class* _class = new Class("k3d::TestObject");
            auto Offset = &((TestObject*)0)->X;
            _class->RegisterField("X", "I", reinterpret_cast<uint64>(Offset));
            Offset = &((TestObject*)0)->Y;
            _class->RegisterField("Y", "I", reinterpret_cast<uint64>(Offset));
            auto Func = &TestObject::Show;
            //_class->RegisterMethod("Show", "V", reinterpret_cast<uint64>(Offset));
            return _class;
        }
        
        TestObject() { X = 10; }
        
        void Show() {}
        
    private:
        int X;
        int Y;
    };
    
    void Init()
    {
        CoreRT.Register();
        
        auto Obj = new TestObject;
        
        auto rClass = TestObject::NewClass();
        
        Class* aClass = CoreRT.FindClass("k3d::TestObject");
        Field* aField = rClass->FindField("X", "I");
        
        auto X = aField->GetValuePtr<int>(Obj);
    }
}

int main(int argc, const char* argv[])
{
    k3d::Init();
    return 0;
}
