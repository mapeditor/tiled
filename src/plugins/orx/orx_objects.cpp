#include "orx_objects.h"

namespace Orx
{
    ///////////////////////////////////////////////////////////////////////////////
    Image::Image(const QString & image_name, const QString & filename) :
        OrxObject(image_name + "_" + IMAGE_POSTFIX), m_UseCount(0), m_ImageName(image_name), m_Texture(filename)
    {
    }

    ///////////////////////////////////////////////////////////////////////////////
    void Image::serialize(SerializationContext & context, QTextStream & ss)
    {
        serialize_name(ss);
        serialize_value(ss, "Texture", m_Texture);
        serialize_value(ss, "TextureSize", m_Size);
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
        if (m_Graphic)
            m_Graphic->serialize(context, ss);

        serialize_name(ss);

        if (m_Graphic)
            serialize_value(ss, "Graphic", m_Graphic->m_Name);

        ss << endl;
        }






    ///////////////////////////////////////////////////////////////////////////////
    Object::Object() : m_Rotation(0)
    {}

    ///////////////////////////////////////////////////////////////////////////////
    Object::Object(const QString & name, const QString & parent) :
        OrxObject(name, parent),
        m_Scale(1),
        m_Repeat(1),
        m_FlipH(false),
        m_FlipV(false),
        m_Rotation(0)
    {}

    ///////////////////////////////////////////////////////////////////////////////
    void Object::serialize(SerializationContext & context, QTextStream & ss)
    {
        serialize_name(ss);

        if (!m_Position.IsZero())
            serialize_value(ss, "Position", m_Position);
        if (!m_Scale.IsOne())
            serialize_value(ss, "Scale", m_Scale);
        if (!m_Repeat.IsOne())
            serialize_value(ss, "Repeat", m_Repeat);
        if (m_Rotation != 0)
            serialize_value(ss, "Rotation", m_Rotation);

        if (m_FlipH && !m_FlipV)
            serialize_value(ss, "Flip", "x");
        if (!m_FlipH && m_FlipV)
            serialize_value(ss, "Flip", "y");
        if (m_FlipH && m_FlipV)
            serialize_value(ss, "Flip", "both");

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

