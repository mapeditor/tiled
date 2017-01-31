#ifndef ORX_OBJECTS_H
#define ORX_OBJECTS_H

#include "orx_object.h"
#include "point_vector.h"
#include "string_converter.h"

#include "mapformat.h"
#include "map.h"
#include "savefile.h"
#include "tile.h"
#include "tilelayer.h"
#include "objectgroup.h"
#include "imagelayer.h"
#include "imagereference.h"
#include "mapobject.h"

namespace Orx {

#define IMAGE_POSTFIX       "Image"
#define GRAPHIC_POSTFIX     "Graphic"
#define PREFAB_POSTFIX      ""
#define OBJECT_POSTFIX      ""
#define LAYER_POSTFIX       "Layer"
#define MAP_POSTFIX         "Map"


class Object;
typedef std::shared_ptr<Object>         ObjectPtr;
typedef std::vector<ObjectPtr>          ObjectPtrs;

class GroupObject;
typedef std::shared_ptr<GroupObject>    GroupObjectPtr;
typedef std::vector<GroupObjectPtr>     GroupObjectPtrs;

class Prefab;
typedef std::shared_ptr<Prefab>         PrefabPtr;
typedef std::vector<PrefabPtr>          PrefabPtrs;

class Graphic;
typedef std::shared_ptr<Graphic>        GraphicPtr;
typedef std::vector<GraphicPtr>         GraphicPtrs;

class Image;
typedef std::shared_ptr<Image>          ImagePtr;
typedef std::vector<ImagePtr>           ImagePtrs;

///////////////////////////////////////////////////////////////////////////////
// Image maps to a GRAPHIC that holds the texture file name
class Image : public OrxObject, public IndexGenerator
{
public:
    // trivial c.tor
    Image();
    Image(const QString & filename);

public:
    // Texture filename (absolute)
    QString         m_Texture;

public:
    // serializes the element into the given stream
    virtual void serialize(SerializationContext & context, QTextStream & ss);
};

///////////////////////////////////////////////////////////////////////////////
// Graphic maps to a GRAPHIC that inherits from an Imge GRAPHIC and specify the portion of the texture to use
class Graphic : public OrxObject
{
public:
    Graphic();
    Graphic(const QString & name);
    Graphic(const QString & name, const QString parent);

public:
    // Image this graphic refers to
    QString         m_Texture;
    Vector2i        m_Origin;
    Vector2i        m_Size;
    int             m_TiledId;

public:
    virtual void serialize(SerializationContext & context, QTextStream & ss);
};

///////////////////////////////////////////////////////////////////////////////
// Prefab is an OBJECT with a GRAPHIC, a BODY but no transformations
class Prefab : public OrxObject, public IndexGenerator
{
public:
    Prefab();
    Prefab(const QString & name);

public:
    GraphicPtr      m_Graphic;
    int             m_TiledId;

public:
    virtual void serialize(SerializationContext & context, QTextStream & ss);
};

///////////////////////////////////////////////////////////////////////////////
// Object is an OBJECT that inherits from a Prefab OBJECT adding tranformations
class Object : public OrxObject
{
public:
    Object();
    Object(const QString & name, const QString & parent);

public:
    GraphicPtr      m_Graphic;
    Vector3f        m_Position;
    float           m_Rotation;
    ObjectPtrs      m_Children;
    int             m_TiledId;

public:
    virtual void serialize(SerializationContext & context, QTextStream & ss);
};

///////////////////////////////////////////////////////////////////////////////
// GroupObject is an Object with children, position and rotation only *used for layers and map
class GroupObject : public Object
{
public:
    GroupObject();
    GroupObject(const QString & name);

public:
    virtual void serialize(SerializationContext & context, QTextStream & ss);
};


}

#endif // ORX_OBJECTS_H
