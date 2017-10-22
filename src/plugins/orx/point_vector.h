#ifndef ORX_POINT_VECTOR_H
#define ORX_POINT_VECTOR_H


namespace Orx {

///////////////////////////////////////////////////////////////////////////////
struct Vector2i {
    Vector2i() : m_X(0), m_Y(0) {}
    bool IsZero() { return ((m_X==0) && (m_Y==0));}
    int m_X;
    int m_Y;
};

///////////////////////////////////////////////////////////////////////////////
struct Vector3f {
    Vector3f() : m_X(0), m_Y(0), m_Z(0) {}
    bool IsZero() { return ((m_X==0) && (m_Y==0) && (m_Z==0));}
    float m_X;
    float m_Y;
    float m_Z;
};


}

#endif // ORX_POINT_VECTOR_H
