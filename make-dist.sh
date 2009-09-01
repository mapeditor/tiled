#!/bin/bash
if [ "$#" -eq "0" ]; then
    echo "Usage: make-dist.sh <version>"
    exit 1
fi
name="tiled-qt-$1"
git archive -v --prefix="$name/" HEAD | gzip > "$name.tar.gz"
echo "Release ready as $name.tar.gz"
