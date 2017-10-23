#ifndef ORX_OBJECT_H
#define ORX_OBJECT_H

#include "point_vector.h"
#include <QString>
#include <QTextStream>
#include <memory>
#include <vector>


namespace Orx {

///////////////////////////////////////////////////////////////////////////////
class OrxObject
{
public:
    OrxObject();
    OrxObject(const QString & name);
    OrxObject(const QString & name, const QString & parent);
    virtual ~OrxObject();

public:
    virtual void serialize_name(QTextStream & ss);
    static QString normalize_name(const QString & name);
    static QString get_name_from_file(const QString & name);

protected:
    ///////////////////////////////////////////////////////////////////////////////
    static void serialize_value(QTextStream & ss, const QString & name, int & value) {
        ss << name << " = " << QString::number(value) << endl;
        }

    ///////////////////////////////////////////////////////////////////////////////
    static void serialize_value(QTextStream & ss, const QString & name, float & value) {
        ss << name << " = " << QString::number(value) << endl;
        }

    ///////////////////////////////////////////////////////////////////////////////
    static void serialize_value(QTextStream & ss, const QString & name, QString value) {
        ss << name << " = " << value << endl;
        }

    ///////////////////////////////////////////////////////////////////////////////
    static void serialize_value(QTextStream & ss, const QString & name, Vector2i & value) {
        ss << name << " = " << QString("(%1, %2)").arg(value.m_X).arg(value.m_Y) << endl;
        }

    ///////////////////////////////////////////////////////////////////////////////
    static void serialize_value(QTextStream & ss, const QString & name, Vector3f & value) {
        ss << name << " = " << QString("(%1, %2, %3)").arg(value.m_X).arg(value.m_Y).arg(value.m_Z) << endl;
        }

    ///////////////////////////////////////////////////////////////////////////////
    static void serialize_value(QTextStream & ss, const QString & name, Vector3i & value) {
        ss << name << " = " << QString("(%1, %2, %3)").arg(value.m_X).arg(value.m_Y).arg(value.m_Z) << endl;
        }

    ///////////////////////////////////////////////////////////////////////////////
    template<typename T>
    static void serialize_object_list(QTextStream & ss, const QString & name, T & container) {
        int element_count = container.size();
        if (element_count)
            {
            auto last = std::prev(container.end());
            if (element_count)
                {
                ss << "ChildList = ";
                for (auto it = container.begin(); it != container.end(); ++it)
                    {
                    ss << (*it)->m_Name;
                    if (it != last)
                        ss << " # ";
                    else
                        ss << endl;
                    }
                }
            }
        }

public:
    QString     m_Name;
    QString     m_Parent;

public:
    virtual void serialize(QTextStream & ss);
};


///////////////////////////////////////////////////////////////////////////////
class IndexGenerator
{
public:
    IndexGenerator() : m_Index(0) {}
    int GetNext() { return m_Index++; }

protected:
    int m_Index;
};


}

#endif // ORX_OBJECT_H
