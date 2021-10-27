#!/bin/bash -xe
website_root=$1
if [ ! -n $website_root ]
then
    echo "Usage: $0 <path to checkout of gh-pages branch>"
fi

out_base=""

npx typedoc \
        --name "Tiled Scripting API" \
        --readme none \
        --excludeExternals \
        --disableSources \
        --plugin typedoc-plugin-markdown \
        --theme markdown --out temp-docs \
        index.d.ts \
        $2

resultMd="$website_root/docs/scripting/README.md"
resultHtml="$website_root/docs/scripting/README.html"
rm -f "$resultMd"
npx concat-md --toc --decrease-title-levels --dir-name-as-title temp-docs >> "$resultMd"
npx showdown makehtml -i "$resultMd" -o "${resultHtml}.tmp"
prefix="---
layout: default
---
"
echo "$prefix" > "$resultHtml"
cat "${resultHtml}.tmp" >> "$resultHtml"
rm -f "${resultHtml}.tmp"
