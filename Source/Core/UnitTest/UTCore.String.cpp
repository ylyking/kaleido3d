#include "Common.h"
#include <KTL/Functional.hpp>
#include <iostream>

#if K3DPLATFORM_OS_WIN
#pragma comment(linker,"/subsystem:console")
#endif

using namespace k3d;
using namespace std;

String toBeRemoved("Love you later.");

void TestString()
{
    String testString("HeyYou!");
    cout << testString.CStr() << endl;
    String testStringBaby("BabyGirl!!");
    testStringBaby.Swap(testString);
    cout << testString.CStr() << endl;
        
    testString += toBeRemoved;
    testString += "..";
    cout << testString.CStr() << endl;
        
    testString.AppendSprintf("%d %s", 5, "fhdsfjdhjkfdhksfhdkjshfjkdshfk");
	testString += 'B';
    cout << testString.CStr() << endl;
        

    cout << testString.Find("yo") << endl;

	auto testMd5 = MD5Encode(testString);
	cout << "md5:" << testMd5.CStr() << endl;

	cout << "md5:" << MD5Encode(testString).CStr() << endl;

    //Archive ar;
    //ar << testString;
        
    String testMoveString(Move(toBeRemoved));
    cout << "testMove:" << testMoveString.CStr()
    << " original:" << (toBeRemoved.CStr()?toBeRemoved.CStr():"null") << endl;

	auto b64 = Base64Encode(testMoveString);
	cout << "b64:" << b64.CStr() << endl;

	auto d64 = Base64Decode(b64);
	cout << "d64:" << d64.CStr() << endl;

    cout << "Sub(3,4):" << d64.SubStr(3, 4).CStr() << endl;
    cout << d64.FindLastOf('r') << endl;
    cout << d64.FindLastOf('x') << endl;

    cout << "Test Invoke: " << Invoke([](int a, int b) -> int
    {
        return a + b;
    }, 1, 2) << endl;

}

int main(int argc, char**argv)
{
	TestString();
	return 0;
}