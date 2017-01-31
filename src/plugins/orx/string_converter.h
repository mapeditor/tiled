#ifndef STRING_CONVERTER_H
#define STRING_CONVERTER_H

#include "point_vector.h"
#include <QString>
#include <QTextStream>

namespace Orx {

///////////////////////////////////////////////////////////////////////////////
template<typename T>
struct string_converter {
    static QString to_string(T value) {
        QTextStream ss;
        ss << value;
        return ss.readAll();
    }
};

///////////////////////////////////////////////////////////////////////////////
//template<>
//struct string_converter<std::string> {
//    static std::string to_string(std::string & value) {
//        return std::string(value);
//    }
//};

///////////////////////////////////////////////////////////////////////////////
template<>
struct string_converter<Vector2i> {
    static QString to_string(Vector2i & value) {
        QTextStream ss;
        ss << "(" << value.m_X << ", " << value.m_Y << ", 0)";
        return ss.readAll();
    }
};

///////////////////////////////////////////////////////////////////////////////
template<>
struct string_converter<Vector3f> {
    static QString to_string(Vector3f & value) {
        QTextStream ss;
        ss << "(" << value.m_X << ", " << value.m_Y << ", " << value.m_Z << ")";
        return ss.readAll();
    }
};

}

#endif // STRING_CONVERTER_H
