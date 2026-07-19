#version 440

// This faux-enum should be identical to ObjectGroupMaterial::ObjectType
const int TYPE_RECTANGLE = 1;
const int TYPE_POLYGON = 2;
const int TYPE_POLYLINE = 3;
const int TYPE_ELLIPSE = 4;
const int TYPE_CAPSULE = 5;
const int TYPE_TEXT = 6;
const int TYPE_POINT = 7;
const int TYPE_TILE = 8;

// layout(location = 0) in vec2 in_local_pos;
layout(location = 0) in vec2 in_tc;
layout(location = 1) in vec4 in_tint_alpha;
// layout(location = 3) flat in uint in_object_type;

layout(location = 0) out vec4 fragColor;

layout(binding = 1) uniform sampler2D tex;

vec4 tint = vec4(in_tint_alpha.rgb, 1);
float alpha = in_tint_alpha.a;

// vec4 draw_tile()
// {
//     vec4 tex_color = texture(tex, in_tc);
//     return tex_color * tint * alpha;
// }

void main()
{
    vec4 tex_color = texture(tex, in_tc);
    fragColor = tex_color * tint * alpha;
    // vec4 color = vec4(255,0,255,255);

    // switch (in_object_type) {
    // case TYPE_TILE:
    //     color = draw_tile();
    //     break;
    // default:
    //     break;
    // }

    // fragColor = color;
}
