{
  description = "A flake for developing Tiled";

  inputs = {
    nixpkgs.url = github:NixOS/nixpkgs/nixos-unstable;
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = { self, nixpkgs, flake-utils }:
    flake-utils.lib.eachDefaultSystem (system:
      let pkgs = nixpkgs.legacyPackages.${system};
      in {
        devShell = pkgs.mkShell {
          buildInputs = with pkgs; [
            ccache      # modules.cpp.compilerWrapper:ccache
            gdb
            lld         # modules.cpp.linkerVariant:lld
            pkg-config
            qbs         # More recent than shipping with QtCreator
            (qt6.env "qt6-tiled" [
              libglvnd          # libGLX/libOpenGL for linking
              qt6.qtdeclarative # qml, quick
              qt6.qtshadertools
              qt6.qtsvg
              qt6.qttools       # lrelease for translations
              qt6.qtimageformats
              qt6.qtwayland
            ])
            qtcreator   # IDE
            zlib
          ];

          # Avoid warning spam due to trying to enable hardening in debug builds
          hardeningDisable = [ "fortify" ];
        };
      }
    );
}
