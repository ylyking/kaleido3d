#ifndef __Reflector_h__
#define __Reflector_h__
#pragma once

namespace k3d
{

class Cursor
{
public:
  Cursor(const CXCursor& c);

  String GetName() const;

  using List = DynArray<Cursor>;

  K3D_DEPRECATED("dddd")
  List GetChildren() KFUNCTION(Y);

private:
  CXCursor m_Handle;
} KCLASS(X);


}
#endif