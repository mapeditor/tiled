#ifndef ORX_OBJECTS_H
#define ORX_OBJECTS_H

#include <string>
#include <sstream>
#include <ostream>
#include <memory>
#include <vector>

namespace Orx {

///////////////////////////////////////////////////////////////////////////////
struct Vector2i {
    int m_X;
    int m_Y;
};

///////////////////////////////////////////////////////////////////////////////
struct Vector3f {
    float m_X;
    float m_Y;
    float m_Z;
};

///////////////////////////////////////////////////////////////////////////////
template<typename T>
struct string_converter {
    static std::string to_string(T value) {
        return std::to_string(value);
    }
};

///////////////////////////////////////////////////////////////////////////////
template<>
struct string_converter<std::string> {
    static std::string to_string(std::string & value) {
        return std::string(value);
    }
};

///////////////////////////////////////////////////////////////////////////////
template<>
struct string_converter<Vector2i> {
    static std::string to_string(Vector2i & value) {
        std::stringstream ss;
        ss << "(" << value.m_X << ", " << value.m_Y << ", 0)";
        return ss.str();
    }
};

///////////////////////////////////////////////////////////////////////////////
template<>
struct string_converter<Vector3f> {
    static std::string to_string(Vector3f & value) {
        std::stringstream ss;
        ss << "(" << value.m_X << ", " << value.m_Y << ", " << value.m_Z << ")";
        return ss.str();
    }
};

///////////////////////////////////////////////////////////////////////////////
class OrxObject
{
public:
    OrxObject() {}
    OrxObject(const std::string & name) : m_name(name) {}
    OrxObject(const std::string & name, const std::string & parent) : m_name(name), m_parent(parent) {}
    virtual ~OrxObject() {}

protected:
    virtual void serialize_name(std::stringstream & ss) {
        ss << "[" << m_name;
        if (!m_parent.empty())
            ss << "@" << m_parent;
        ss << "]" << std::endl;
        }

    ///////////////////////////////////////////////////////////////////////////////
    template<typename T>
    static void serialize_value(std::stringstream & ss, const std::string & name, T & value) {
        ss << name << " = " << string_converter<T>::to_string(value) << std::endl;
        }

    ///////////////////////////////////////////////////////////////////////////////
    template<typename T>
    static void serialize_object_list(std::stringstream & ss, const std::string & name, T & container) {
        int element_count = container.size();
        if (element_count)
            {
            auto last = std::prev(container.end());
            if (element_count)
                {
                ss << "ChildList = ";
                for (auto it = container.begin(); it != container.end(); ++it)
                    {
                    ss << (*it)->m_name;
                    if (it != last)
                        ss << " # ";
                    else
                        ss << std::endl;
                    }
                }
            }
        }

public:
    std::string     m_name;
    std::string     m_parent;

public:
    virtual void serialize(std::stringstream & ss) {
        serialize_name(ss);
        }
};

///////////////////////////////////////////////////////////////////////////////
#define CONSTRUCT_ORX_OBJ(obj_name)     obj_name() {}; \
                                        obj_name(const std::string & name) : OrxObject(name) {} \
                                        obj_name(const std::string & name, const std::string & parent) : OrxObject(name, parent) {} \

class Object;
typedef std::shared_ptr<Object>     ObjectPtr;
typedef std::vector<ObjectPtr>      ObjectPtrs;

class GroupObject;
typedef std::shared_ptr<GroupObject> GroupObjectPtr;
typedef std::vector<GroupObjectPtr>  GroupObjectPtrs;

class Prefab;
typedef std::shared_ptr<Prefab>     PrefabPtr;
typedef std::vector<PrefabPtr>      PrefabPtrs;

class Graphic;
typedef std::shared_ptr<Graphic>    GraphicPtr;
typedef std::vector<GraphicPtr>     GraphicPtrs;



///////////////////////////////////////////////////////////////////////////////
class Graphic : public OrxObject
{
public:
    CONSTRUCT_ORX_OBJ(Graphic)

public:
    std::string     m_Texture;
    Vector2i        m_Origin;
    Vector2i        m_Size;
    int             m_TiledId;

public:
    virtual void serialize(std::stringstream & ss)
        {
        serialize_name(ss);
        serialize_value(ss, "Texture", m_Texture);
        serialize_value(ss, "TextureOrigin", m_Origin);
        serialize_value(ss, "TextureSize", m_Size);
        ss << std::endl;
        }
};

///////////////////////////////////////////////////////////////////////////////
class Prefab : public OrxObject
{
public:
    CONSTRUCT_ORX_OBJ(Prefab)

public:
    GraphicPtr      m_Graphic;
    int             m_TiledId;
    int             m_UseCount; // used to generate object names when this obj is a prefab

public:
    virtual void serialize(std::stringstream & ss) {
        serialize_name(ss);

        if (m_Graphic)
            serialize_value(ss, "Graphic", m_Graphic->m_name);

        ss << std::endl;
        }
};

///////////////////////////////////////////////////////////////////////////////
class Object : public OrxObject
{
public:
    CONSTRUCT_ORX_OBJ(Object)

public:
    GraphicPtr      m_Graphic;
    Vector3f        m_Position;
    float           m_Rotation;
    ObjectPtrs      m_Children;
    int             m_TiledId;

public:
    virtual void serialize(std::stringstream & ss) {
        serialize_name(ss);

        if (m_Graphic)
            serialize_value(ss, "Graphic", m_Graphic->m_name);

        serialize_value(ss, "Position", m_Position);
        serialize_value(ss, "Rotation", m_Rotation);
        serialize_object_list(ss, "ChildList", m_Children);

        ss << std::endl;
        }
};

///////////////////////////////////////////////////////////////////////////////
class GroupObject : public Object
{
public:
    GroupObject() {};
    GroupObject(const std::string & name) : Object(name) {}
    GroupObject(const std::string & name, const std::string & parent) : Object(name, parent) {}

public:
    virtual void serialize(std::stringstream & ss) {
        serialize_name(ss);
        serialize_object_list(ss, "ChildList", m_Children);
        ss << std::endl;
        }
};


}

#endif // ORX_OBJECTS_H
