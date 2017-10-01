#pragma once
#include <vector>

namespace k3d
{
    class VertexFactory
    {
        //using std::vector;
    public:
        VertexFactory();
        ~VertexFactory();

    private:
        std::vector<ngfx::VertexLayout>      m_Layout;
        std::vector<ngfx::VertexAttribute>   m_Attribute;
    };
}