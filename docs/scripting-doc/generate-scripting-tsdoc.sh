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
        --theme $website_root/docs/typedoc_theme \
        --highlightTheme dark-plus \
        --out $website_root/docs/scripting index.d.ts \
        $2

