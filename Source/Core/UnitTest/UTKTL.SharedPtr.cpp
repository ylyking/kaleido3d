#include "Common.h"
#include <Core/App.h>
#include <KTL/String.hpp>
#include <Core/Runtime/Field.h>
#include <iostream>
#include <map>
#define OFFSET(structure, member) ((int)&((structure*)0)->member); 

#if K3DPLATFORM_OS_WIN
#pragma comment(linker,"/subsystem:console")
#endif

using namespace std;
using namespace k3d;

class SharedTest
{
public:
	SharedTest()
	{

	}
	~SharedTest()
	{
		cout << "SharedTest Destroy. " << endl;
	}
};

class ESFT;

class ESFTChild1
{
public:
	ESFTChild1(SharedPtr<ESFT> pERoot)
		: Root(pERoot)
	{
		cout << "ESFTChild1 init .." << endl;
#if K3DPLATFORM_OS_WIN
		OutputDebugStringA("ESFTChild1 init ..\n");
#endif
	}
	~ESFTChild1()
	{
        cout << "ESFTChild1 release .." << endl;
#if K3DPLATFORM_OS_WIN
		OutputDebugStringA("ESFTChild1 release ..\n");
#endif
	}
	SharedPtr<ESFT> Root;
};

class Root
{
public:
	static DynArray< SharedPtr< ESFT > > root;
	static DynArray< SharedPtr<ESFTChild1> > childrenTracker;
};

DynArray< SharedPtr< ESFT > > Root::root;
DynArray< SharedPtr<ESFTChild1> > Root::childrenTracker;

class ESFT : public EnableSharedFromThis<ESFT>
{
public:
	ESFT()
    {
#if K3DPLATFORM_OS_WIN
		OutputDebugStringA ("ESFT init ..\n");
#endif
	}
	~ESFT()
    {
#if K3DPLATFORM_OS_WIN
		OutputDebugStringA("ESFT release ..\n");
#endif
		cout << "ESFT release .." << endl;
	}
	SharedPtr<ESFTChild1> NewChild1()
	{
		auto child = MakeShared<ESFTChild1>(SharedFromThis());
		Root::childrenTracker.Append(child);
		return child;
	}
};

struct TestObj
{
    int X;
    int Y;
};

std::map<std::string, void*> memberOffsets;

void TestSharedPtr()
{
    int TestObj::*P = &TestObj::Y;

    memberOffsets["Y"] = (*(void**)(&P));

    //cout << (int)(P) << endl;
    cout << &TestObj::Y << endl;

    TestObj* Obj = new TestObj;
    Obj->X = 20;
    Obj->Y = 60;

    Field field("Y", 4);

    cout << *field.GetValuePtr<int>(Obj) << endl;

	SharedPtr<Os::File> spFile(new Os::File);
	cout << "file:" << spFile.UseCount() << endl;

	SharedPtr<SharedTest> spTest(new SharedTest);
	cout << "SharedTest:" << spTest.UseCount() << endl;

	SharedPtr<IIODevice> ioFile = spFile;
	K3D_ASSERT(ioFile.UseCount()==2);
		
	{
		WeakPtr<IIODevice> weakRef(ioFile);
		cout << "weakREf:" << ioFile.UseCount() << endl;
	}

	auto makeShared = MakeShared<Os::File>();
	makeShared->Open(KT("TestSharedPtr"), IOWrite);

	auto esft = MakeShared<ESFT>();
	Root::root. Append (esft);
	{
		auto child = esft->NewChild1();
	}
	auto child2 = esft->NewChild1();
	auto refMakeShared = makeShared;
	cout << "refMakeShared:" << refMakeShared.UseCount() << endl;

	refMakeShared->Close();
}

int atexit(void)
{
#if K3DPLATFORM_OS_WIN
	OutputDebugStringA("At Exit ..\n");
#endif
	return 0;
}

class TestApp : public App
{
public:
	TestApp() : App("Test", 1920, 1080) {}
	void OnProcess(Message& msg) {}
};

int main(int argc, char**argv)
{
	TestSharedPtr();
	TestApp app;
	app.Run();
	return 0;
}
