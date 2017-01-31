#include "orx_objects.h"

namespace Orx
{
    ///////////////////////////////////////////////////////////////////////////////
    Image::Image()
    {}

    ///////////////////////////////////////////////////////////////////////////////
    Image::Image(const QString & filename) : m_Texture(filename)
    {
        m_Name = get_name_from_file(filename) + IMAGE_POSTFIX;
    }

    ///////////////////////////////////////////////////////////////////////////////
    void Image::serialize(SerializationContext & context, QTextStream & ss)
    {
        serialize_name(ss);
        serialize_value(ss, "Texture", m_Texture);
        ss << endl;
    }








    ///////////////////////////////////////////////////////////////////////////////
    Graphic::Graphic()
    {}

    ///////////////////////////////////////////////////////////////////////////////
    Graphic::Graphic(const QString & name) : OrxObject(name)
    {}

    ///////////////////////////////////////////////////////////////////////////////
    Graphic::Graphic(const QString & name, const QString parent) : OrxObject(name, parent)
    {}

    ///////////////////////////////////////////////////////////////////////////////
    void Graphic::serialize(SerializationContext & context, QTextStream & ss)
    {
        serialize_name(ss);

        if (m_Texture.isEmpty() == false)
            serialize_value(ss, "Texture", m_Texture);

        serialize_value(ss, "TextureOrigin", m_Origin);
        serialize_value(ss, "TextureSize", m_Size);
        ss << endl;
    }







    ///////////////////////////////////////////////////////////////////////////////
    Prefab::Prefab()
    {}

    ///////////////////////////////////////////////////////////////////////////////
    Prefab::Prefab(const QString & name) : OrxObject(name)
    {}

    ///////////////////////////////////////////////////////////////////////////////
    void Prefab::serialize(SerializationContext & context, QTextStream & ss)
    {
        serialize_name(ss);

        if (m_Graphic)
            serialize_value(ss, "Graphic", m_Graphic->m_Name);

        ss << endl;
        }






    ///////////////////////////////////////////////////////////////////////////////
    Object::Object()
    {}

    ///////////////////////////////////////////////////////////////////////////////
    Object::Object(const QString & name, const QString & parent) : OrxObject(name, parent)
    {}

    ///////////////////////////////////////////////////////////////////////////////
    void Object::serialize(SerializationContext & context, QTextStream & ss)
    {
        serialize_name(ss);

        if (m_Graphic)
            serialize_value(ss, "Graphic", m_Graphic->m_Name);

        serialize_value(ss, "Position", m_Position);
        serialize_value(ss, "Rotation", m_Rotation);
        serialize_object_list(ss, "ChildList", m_Children);

        ss << endl;
    }





    ///////////////////////////////////////////////////////////////////////////////
    GroupObject::GroupObject()
    {}

    ///////////////////////////////////////////////////////////////////////////////
    GroupObject::GroupObject(const QString & name) : Object(name, "")
    {}

    ///////////////////////////////////////////////////////////////////////////////
    void GroupObject::serialize(SerializationContext & context, QTextStream & ss)
    {
        serialize_name(ss);
        serialize_object_list(ss, "ChildList", m_Children);
        ss << endl;
    }


}

