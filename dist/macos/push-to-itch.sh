#!/bin/bash
set -ev
mkdir itch
mv install/Tiled.app itch/
curl https://broth.itch.ovh/butler/darwin-amd64/LATEST/archive/default --output default
unzip default
./butler push --userversion=$TILED_VERSION itch thorbjorn/tiled:macos-snapshot
