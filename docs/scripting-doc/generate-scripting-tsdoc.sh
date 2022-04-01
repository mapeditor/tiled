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
        --hideBreadcrumbs true \
        --hideInPageTOC true \
        --out temp-docs \
        index.d.ts \
        $2

resultMd="$website_root/docs/scripting.md"
rm -f "$resultMd"
npx concat-md --decrease-title-levels --dir-name-as-title temp-docs >> concatenated.md
prefix="---
layout: default
---
"
echo "$prefix" > "$resultMd"
cat concatenated.md >> "$resultMd"
