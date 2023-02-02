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
          buildInputs = [
            pkgs.ccache      # modules.cpp.compilerWrapper:ccache
            pkgs.gdb
            pkgs.lld         # modules.cpp.linkerVariant:lld
            pkgs.pkg-config
            pkgs.qbs         # More recent than shipping with QtCreator
            pkgs.qt6.full
            pkgs.qtcreator   # IDE
            pkgs.zlib
          ];

          # Avoid warning spam due to trying to enable hardening in debug builds
          hardeningDisable = [ "fortify" ];
        };
      }
    );
}
