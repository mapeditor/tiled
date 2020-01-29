curl -fsS -o butler.exe https://dl.itch.ovh/butler/windows-amd64/head/butler.exe
butler.exe push --userversion=%TILED_ITCH_VERSION% itch/install-root thorbjorn/tiled:%TILED_ITCH_CHANNEL%
