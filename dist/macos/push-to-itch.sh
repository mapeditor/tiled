#!/bin/bash
set -ev
openssl aes-256-cbc -K $encrypted_498dac83364f_key -iv $encrypted_498dac83364f_iv -in dist/butler_creds.enc -out butler_creds -d
mkdir itch
mv install/Tiled.app itch/
curl https://dl.itch.ovh/butler/darwin-amd64/head/butler --output butler
chmod +x ./butler
./butler -i butler_creds push --userversion=$TILED_VERSION itch thorbjorn/tiled:macos-snapshot
