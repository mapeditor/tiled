#ifndef ORX_POINT_VECTOR_H
#define ORX_POINT_VECTOR_H


namespace Orx {

///////////////////////////////////////////////////////////////////////////////
struct Vector2i {
    Vector2i() : m_X(0), m_Y(0) {}
    Vector2i(int v) : m_X(v), m_Y(v) {}
    Vector2i(int x, int y) : m_X(x), m_Y(y) {}
    bool IsZero() { return ((m_X==0) && (m_Y==0));}
    bool IsOne() { return ((m_X==1) && (m_Y==1));}
    int m_X;
    int m_Y;
};

///////////////////////////////////////////////////////////////////////////////
struct Vector3f {
    Vector3f() : m_X(0), m_Y(0), m_Z(0) {}
    Vector3f(float v) : m_X(v), m_Y(v), m_Z(v) {}
    Vector3f(float x, float y, float z) : m_X(x), m_Y(y), m_Z(z) {}
    bool IsZero() { return ((m_X==0) && (m_Y==0) && (m_Z==0));}
    bool IsOne() { return ((m_X==1) && (m_Y==1) && (m_Z==1));}
    float m_X;
    float m_Y;
    float m_Z;
};

///////////////////////////////////////////////////////////////////////////////
struct Vector3i {
    Vector3i() : m_X(0), m_Y(0), m_Z(0) {}
    Vector3i(int v) : m_X(v), m_Y(v), m_Z(v) {}
    Vector3i(int x, int y, int z) : m_X(x), m_Y(y), m_Z(z) {}
    bool IsZero() { return ((m_X==0) && (m_Y==0) && (m_Z==0));}
    bool IsOne() { return ((m_X==1) && (m_Y==1) && (m_Z==1));}
    int m_X;
    int m_Y;
    int m_Z;
};

}

#endif // ORX_POINT_VECTOR_H
