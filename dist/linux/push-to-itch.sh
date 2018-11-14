#!/bin/bash
set -ev
curl -JOL https://broth.itch.ovh/butler/linux-amd64/LATEST/archive/default
unzip butler-linux-amd64.zip
./butler push --userversion=$TILED_VERSION Tiled thorbjorn/tiled:linux-64bit-snapshot
