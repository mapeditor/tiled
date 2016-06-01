#ifndef TOKENDEFINES_H
#define TOKENDEFINES_H

const QString cell_t =
        "cell { \
            x: {{x}}\n \
            y: {{y}}\n \
            tile: {{tile}}\n \
            h_flip: {{h_flip}}\n \
            v_flip: {{v_flip}}\n \
            }";

const QString layers_t =
        "layers {\n \
            id: {{name}}\n \
            z: {{z}}\n \
            is_visible: {{is_visible}}\n \
            {{cells}}\n \
            }";
const QString map_t =
           "tile_set: \"{{tile_set}}\"\n \
            {{layers}}n\ \
            material: \"{{material}}\n \
            blend_mode: {{blend_mode}}\n \
            }";
#endif // TOKENDEFINES_H
