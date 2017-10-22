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
        QVariant ss(value);
        QString ret = ss.toString();
        return ret;
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
        QString ret = QString("(%1, %2)").arg(value.m_X).arg(value.m_Y);
        return ret;
    }
};

///////////////////////////////////////////////////////////////////////////////
template<>
struct string_converter<Vector3f> {
    static QString to_string(Vector3f & value) {
        QString ret = QString("(%1, %2, %3)").arg(value.m_X).arg(value.m_Y).arg(value.m_Z);
        return ret;
    }
};

///////////////////////////////////////////////////////////////////////////////
template<>
struct string_converter<Vector3i> {
    static QString to_string(Vector3i & value) {
        QString ret = QString("(%1, %2, %3)").arg(value.m_X).arg(value.m_Y).arg(value.m_Z);
        return ret;
    }
};

}

#endif // STRING_CONVERTER_H
