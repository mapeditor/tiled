#ifndef ORX_OBJECT_H
#define ORX_OBJECT_H

#include "serialization_context.h"
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
    template<typename T>
    static void serialize_value(QTextStream & ss, const QString & name, T & value) {
        ss << name << " = " << string_converter<T>::to_string(value) << endl;
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
    virtual void serialize(SerializationContext & context, QTextStream & ss);
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
