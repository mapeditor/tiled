/*
  Installs all files necessary to run Tiled and other files that should be
  shipped when Tiled is distributed.
*/

import qbs
import qbs.FileInfo

Product {
    name: "distribute"
    type: "installable"
    builtByDefault: false

    Depends { name: "cpp" }
    Depends { name: "Qt.core" }

    Group {
        name: "Examples"
        prefix: "../../examples/"
        files: [
            "desert.tmx",
            "desert.tsx",
            "hexagonal-mini.tmx",
            "hexmini.png",
            "isometric_grass_and_water.png",
            "isometric_grass_and_water.tmx",
            "perspective_walls.png",
            "perspective_walls.tmx",
            "perspective_walls.tsx",
            "sewers.tmx",
            "sewer_tileset.png",
            "tmw_desert_spacing.png",
        ]

        qbs.install: true
        qbs.installDir: "examples"
    }

    Group {
        name: "Examples (automapping)"
        prefix: "../../examples/sewer_automap/"
        files: [
            "rules.txt",
            "rules_sewers.png",
            "rule_001.tmx",
            "rule_002.tmx",
            "rule_003.tmx",
            "rule_004.tmx",
            "rule_005.tmx",
            "rule_006.tmx",
            "rule_007.tmx",
            "rule_008.tmx",
            "rule_009.tmx",
            "sewers.tmx",
        ]
        qbs.install: true
        qbs.installDir: "examples/sewer_automap"
    }

    Group {
        name: "Qt DLLs"
        prefix: Qt.core.binPath + "/"
        property string postfix: qbs.debugInformation ? "d.dll" : ".dll"
        files: [
            "Qt5Core" + postfix,
            "Qt5Gui" + postfix,
            "Qt5Network" + postfix,
            "Qt5Widgets" + postfix,
            "Qt5OpenGL" + postfix,
            "icuin54.dll",
            "icuuc54.dll",
            "icudt54.dll",
        ]
        qbs.install: true
        qbs.installDir: ""
    }

    Group {
        name: "Qt Platform Plugin"
        prefix: FileInfo.joinPaths(Qt.core.binPath, "../plugins/platforms/")
        files: [
            "qwindows.dll",
        ]
        qbs.install: true
        qbs.installDir: "plugins/platforms"
    }

    Group {
        name: "Qt Image Format Plugins"
        prefix: FileInfo.joinPaths(Qt.core.binPath, "../plugins/imageformats/")
        files: [
            "qgif.dll",
            "qjpeg.dll",
            "qtiff.dll",
        ]
        qbs.install: true
        qbs.installDir: "plugins/imageformats"
    }

    Group {
        name: "Qt Translations"
        prefix: FileInfo.joinPaths(Qt.core.binPath, "../translations/")
        files: [
            "qt_cs.qm",
            "qt_de.qm",
            "qt_es.qm",
            "qt_fr.qm",
            "qt_he.qm",
            "qt_ja.qm",
            "qt_pt.qm",
            "qt_ru.qm",
            "qt_zh_CN.qm",
            "qt_zh_TW.qm",
        ]
        qbs.install: true
        qbs.installDir: "translations"
    }

    Group {
        name: "Runtime DLLs"

        prefix: {
            if (qbs.toolchain.contains("mingw"))
                return FileInfo.joinPaths(cpp.toolchainInstallPath) + "/"
            else if (qbs.architecture === "x86_64")
                return "C:/windows/SysWOW64/"
            else
                return "C:/windows/system32/"
        }
        files: {
            if (qbs.toolchain.contains("mingw")) {
                return [
                    "libgcc_s_dw2-1.dll",
                    "libstdc++-6.dll",
                    "libwinpthread-1.dll",
                ]
            } else {
                return [
                    "MSVCP120.DLL",
                    "MSVCR120.DLL",
                ]
            }
        }
        qbs.install: true
        qbs.installDir: ""
    }

    Group {
        name: "Misc Files"
        prefix: "../../"
        files: [
            "COPYING",
            "AUTHORS",
            "README.md",
            "NEWS",
            "LICENSE.APACHE",
            "LICENSE.BSD",
            "LICENSE.GPL",
            "dist/win/qt.conf",
        ]
        qbs.install: true
        qbs.installDir: ""
    }
}
