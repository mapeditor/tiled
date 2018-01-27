#include "orx_objects.h"

namespace Orx
{
    ///////////////////////////////////////////////////////////////////////////////
    Image::Image(const QString & image_name, const QString & filename) :
        OrxObject(image_name + "_" + IMAGE_POSTFIX), m_UseCount(0), m_ImageName(image_name), m_Texture(filename)
    {
    }

    ///////////////////////////////////////////////////////////////////////////////
    void Image::serialize(QTextStream & ss)
    {
        serialize_name(ss);
        serialize_value(ss, "Texture", m_Texture);
        serialize_value(ss, "TextureSize", m_Size);

        if (!m_Pivot.isEmpty())
            serialize_value(ss, "Pivot", m_Pivot);

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
    void Graphic::serialize(QTextStream & ss)
    {
        serialize_name(ss);

        if (m_Texture.isEmpty() == false)
            serialize_value(ss, "Texture", m_Texture);

        serialize_value(ss, "TextureOrigin", m_Origin);
        ss << endl;
    }



    ///////////////////////////////////////////////////////////////////////////////
    ShaderLayer::ShaderLayer()
    { }

    ///////////////////////////////////////////////////////////////////////////////
    ShaderLayer::ShaderLayer(const QString & name, Vector3i tile_size, Vector3i map_size) : Object(name, nullptr)
    { init(tile_size, map_size); }

    void ShaderLayer::init(Vector3i tile_size, Vector3i map_size)
    {
        m_Scale         = Vector3i(1);
        m_RelativeSize  = Vector3i(1, 1, 0);
        m_Position      = Vector3i(0, 0, 1);
        m_TileSize      = tile_size;
        m_MapSize       = map_size;
        m_CameraPos     = Vector3i(0, 0, 0);
        m_Highlight     = Vector3i(0, 0, 0);
    }

    ///////////////////////////////////////////////////////////////////////////////
    void ShaderLayer::serialize(QTextStream & ss)
    {
        serialize_name(ss);

        serialize_value(ss, "Graphic", "@");
        serialize_value(ss, "Texture", m_Texture);
        serialize_value(ss, "ShaderList", "@");
        serialize_value(ss, "UseParentSpace", "both");
        serialize_value(ss, "ParentCamera", "Camera");
        serialize_value(ss, "Scale", m_Scale);
        serialize_value(ss, "Code", "@MapShader");
        serialize_value(ss, "ParamList", "@MapShader");

        serialize_value(ss, "RelativeSize", m_RelativeSize);
        serialize_value(ss, "Position", m_Position);
        serialize_value(ss, "TileSize", m_TileSize);
        serialize_value(ss, "MapSize", m_MapSize);
        serialize_value(ss, "CameraPos", m_CameraPos);
        serialize_value(ss, "Highlight", m_Highlight);

        QString set;
        for (int a=0; a<m_Images.size(); ++a)
        {
            set += "@" + m_Images[a]->m_Name + ".Texture";
            if (a<(m_Images.size() - 1))
                set += " # ";
        }
        serialize_value(ss, "Set", set);

        set = "";
        for (int a=0; a<m_SetSizes.size(); ++a)
        {
            set += QString("(%1, %2, %3)").arg(m_SetSizes[a].m_X).arg(m_SetSizes[a].m_Y).arg(m_SetSizes[a].m_Z);
            if (a<(m_Images.size() - 1))
                set += " # ";
        }
        serialize_value(ss, "SetSizes", set);

        ss << endl;
    }







    ///////////////////////////////////////////////////////////////////////////////
    Prefab::Prefab()
    {}

    ///////////////////////////////////////////////////////////////////////////////
    Prefab::Prefab(const QString & name) : OrxObject(name)
    {}

    ///////////////////////////////////////////////////////////////////////////////
    void Prefab::serialize(QTextStream & ss)
    {
        if (m_Graphic)
            m_Graphic->serialize(ss);

        serialize_name(ss);

        if (m_Graphic)
            serialize_value(ss, "Graphic", m_Graphic->m_Name);

        ss << endl;
        }




    ///////////////////////////////////////////////////////////////////////////////
    MapShader::MapShader() : OrxObject("MapShader")
    {}

    ///////////////////////////////////////////////////////////////////////////////
    void MapShader::serialize(QTextStream & ss)
    {
        serialize_name(ss);

        serialize_value(ss, "Code", "void main()\n"
                                           "{\n"
                                           "  // Gets correction ratio based on camera's frustum size and display resolution\n"
                                           "  vec2  ratio = CameraSize.xy / Resolution.xy;\n"
                                           "\n"
                                           "  // Reverses the fragment's Y coordinates and gets it in tiles\n"
                                           "  vec2  coord = mod((CameraPos.xy + vec2(gl_FragCoord.x, Resolution.y - gl_FragCoord.y) * ratio) / TileSize.xy, MapSize.xy);\n"
                                           "\n"
                                           "  // Gets position (in tiles)\n"
                                           "  vec2  pos   = floor(coord);\n"
                                           "\n"
                                           "  // Gets tile's index in map\n"
                                           "  float tile  = pos.x + (pos.y * MapSize.x);\n"
                                           "\n"
                                           "  // Gets index of the component pair (as we store two tiles per pixel)\n"
                                           "  int   comp  = int(mod(tile, 2.0));\n"
                                           "\n"
                                           "  // Computes the UV for the color that contains our tile\n"
                                           "  vec2  uv    = vec2((tile + 0.5) / (MapSize.x * MapSize.y), 0.5);\n"
                                           "\n"
                                           "  // Retrieves the pixel containing the tile's index\n"
                                           "  vec4  pixel = texture2D(Map, uv).rgba;\n"
                                           "  vec2  value = mix(pixel.rg, pixel.ba, vec2(comp));\n"
                                           "\n"
                                           "  // Computes index\n"
                                           "  float index = 255.0 * ((256.0 * value.x) + value.y);\n"
                                           "\n"
                                           "  // Computes the UV of the tile's corner in the set\n"
                                           "  vec2  size  = SetSize.xy * vec2(Set_right - Set_left, Set_bottom - Set_top);\n"
                                           "  float x     = Set_left + mod(index, size.x);\n"
                                           "  float y     = Set_top + floor(index / size.x);\n"
                                           "\n"
                                           "  // Computes offset inside the tile\n"
                                           "  vec2 offset = coord.xy - pos.xy;\n"
                                           "\n"
                                           "  // Updates UV with in-tile offset\n"
                                           "  uv          = (vec2(x, y) + offset) / size.xy;\n"
                                           "\n"
                                           "  // Gets highlight position in tiles\n"
                                           "  vec2 hlpos  = floor(mod((CameraPos.xy + vec2(Highlight.x, Highlight.y) * ratio) / TileSize.xy, MapSize.xy));\n"
                                           "\n"
                                           "  // Gets color\n"
                                           "  vec4 color  = texture2D(Set, uv).rgba;\n"
                                           "\n"
                                           "  // Highlighted?\n"
                                           "  if(hlpos == pos)\n"
                                           "  {\n"
                                           "    color.rgb = mix(color.rgb, vec3(0.8941, 0.8157, 0.0392), 0.35);\n"
                                           "  }\n"
                                           "\n"
                                           "  // Updates fragment\n"
                                           "  gl_FragColor = color.rgba;\n"
                                           "}\n");

        serialize_value(ss, "ParamList", "ileSize # SetSizes # MapSize # Map # Set # Resolution # CameraPos # CameraSize # Highlight");

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
    void Object::serialize(QTextStream & ss)
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
    void GroupObject::serialize(QTextStream & ss)
    {
        serialize_name(ss);
        serialize_object_list(ss, "ChildList", m_Children);
        ss << endl;
    }


}

