TiledPlugin {
    condition: false   // WIP scaffold — not built by default. See #2741.

    cpp.defines: base.concat(["GLTF_LIBRARY"])

    files: [
        "gltfexporter.cpp",
        "gltfexporter.h",
        "gltf_global.h",
        "gltfplugin.cpp",
        "gltfplugin.h",
        "plugin.json",
    ]
}
