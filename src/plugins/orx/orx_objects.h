#ifndef ORX_OBJECTS_H
#define ORX_OBJECTS_H

#include "orx_object.h"
#include "point_vector.h"

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

#define IMAGE_POSTFIX       "Txt"
#define GRAPHIC_POSTFIX     "Gra"
#define PREFAB_POSTFIX      "Pfb"
#define OBJECT_POSTFIX      "Obj"
#define LAYER_POSTFIX       "Layer"
#define MAP_POSTFIX         "Map"

#define MAX_OBJECT_CHILDREN 255

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

class ShaderLayer;
typedef std::shared_ptr<ShaderLayer>    ShaderLayerPtr;
typedef std::vector<ShaderLayer>        ShaderLayerPtrs;

class MapShader;
typedef std::shared_ptr<MapShader>      MapShaderPtr;

class Image;
typedef std::shared_ptr<Image>          ImagePtr;
typedef std::vector<ImagePtr>           ImagePtrs;

///////////////////////////////////////////////////////////////////////////////
// Image maps to a GRAPHIC that holds the texture file name
class Image : public OrxObject, public IndexGenerator
{
public:
    // trivial c.tor
    Image(const QString & image_name, const QString & filename);
    Image(const QString & image_name, const QString & filename, QString pivot);

public:
    // Texture filename (absolute)
    QString         m_ImageName;
    QString         m_Texture;
    int             m_UseCount;
    Vector2i        m_Size;
    QString         m_Pivot;

public:
    // serializes the element into the given stream
    virtual void serialize(QTextStream & ss);
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

public:
    virtual void serialize(QTextStream & ss);
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
    QString         m_BaseName;
    int             m_TiledId;
    int             m_UseCount;

public:
    virtual void serialize(QTextStream & ss);
};

///////////////////////////////////////////////////////////////////////////////
// Prefab is an OBJECT with a GRAPHIC, a BODY but no transformations
class MapShader : public OrxObject
{
public:
    MapShader();

public:
    virtual void serialize(QTextStream & ss);
};

///////////////////////////////////////////////////////////////////////////////
struct OptimizedCell
{
    bool m_Valid = false;
    const Tiled::Cell * m_Cell = nullptr;
    int m_RepeatX = 1;
    int m_RepeatY = 1;
};

///////////////////////////////////////////////////////////////////////////////
template<typename T>
class Grid2D
{
public:
    Grid2D(int width, int height) : mWidth(width), mHeight(height), mGrid(width * height) {}
    T & at(int x, int y) { return mGrid.at(x + (y * mWidth)); }

private:
    int mWidth;
    int mHeight;
    std::vector<T> mGrid;
};


///////////////////////////////////////////////////////////////////////////////
// Object is an OBJECT that inherits from a Prefab OBJECT adding tranformations
class Object : public OrxObject
{
public:
    Object();
    Object(const QString & name, const QString & parent);

public:
    Vector3f            m_Position;
    Vector3i            m_Scale;
    Vector3i            m_Repeat;
    float               m_Rotation;
    ObjectPtrs          m_Children;
    bool                m_FlipH;
    bool                m_FlipV;
    int                 m_TiledId;
    const Tiled::Cell * m_Cell;

public:
    virtual void serialize(QTextStream & ss);
};

///////////////////////////////////////////////////////////////////////////////
// GroupObject is an Object with children, position and rotation only *used for layers and map
class GroupObject : public Object
{
public:
    GroupObject();
    GroupObject(const QString & name);

public:
    virtual void serialize(QTextStream & ss);
};

///////////////////////////////////////////////////////////////////////////////
// Binary map used in shader based export
class ShaderLayer : public Object
{
public:
    ShaderLayer();
    ShaderLayer(const QString & name, Vector3i tile_size, Vector3i map_size);
    void init(Vector3i tile_size, Vector3i map_size);
public:
    // Image this graphic refers to
    QString             m_Texture;
    ImagePtrs           m_Images;

    QString             m_Graphic;
    QString             m_ShaderList;
    QString             m_UseParentSpace;
    QString             m_ParentCamera;
    Vector3i            m_Scale;
    Vector3i            m_RelativeSize;
    Vector3i            m_Position;
    QString             m_Pivot;
    QString             m_Code;
    QString             m_ParamList;
    Vector3i            m_TileSize;
    QVector<Vector3i>   m_SetSizes;
    Vector3i            m_MapSize;
    QString             m_Resolution;
    Vector3i            m_CameraPos;
    Vector3i            m_CameraSize;
    Vector3i            m_Highlight;

public:
    virtual void serialize(QTextStream & ss);
};


}

#endif // ORX_OBJECTS_H
