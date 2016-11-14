#!/bin/bash
set -ev
mkdir Tiled
mv Tiled-$TILED_VERSION-x86_64.AppImage Tiled/
wget https://dl.itch.ovh/butler/linux-amd64/head/butler
chmod +x ./butler
./butler -i butler_creds push Tiled thorbjorn/tiled:linux-64bit-snapshot
