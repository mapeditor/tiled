#!/bin/bash
set -ev
mkdir itch
mv install/Tiled.app itch/
curl -JOL https://broth.itch.ovh/butler/darwin-amd64/LATEST/archive/default
unzip butler-darwin-amd64.zip
./butler push --userversion=$TILED_VERSION itch thorbjorn/tiled:macos-snapshot
