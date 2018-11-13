#!/bin/bash
set -ev
wget https://broth.itch.ovh/butler/linux-amd64/LATEST/archive/default
chmod +x ./butler
./butler push --userversion=$TILED_VERSION Tiled thorbjorn/tiled:linux-64bit-snapshot
