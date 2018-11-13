#!/bin/bash
set -ev
mkdir itch
mv install/Tiled.app itch/
curl https://broth.itch.ovh/butler/darwin-amd64/LATEST/archive/default --output butler
chmod +x ./butler
./butler push --userversion=$TILED_VERSION itch thorbjorn/tiled:macos-snapshot
