#!/usr/bin/bash

BUILDROOT="$(pwd)/tmp-buildroot/"
rm -rf $BUILDROOT
[ -d buildroot ] || mkdir -p ${BUILDROOT}/{SOURCES,SPECS}

pushd $(git rev-parse --show-toplevel)
VERSION=$(git describe | sed 's/^v//' | sed 's/-[^-]*$//' | sed 's/-/\./')
git archive HEAD --prefix=tiled-$VERSION/ -o ${BUILDROOT}/SOURCES/tiled-$VERSION.tar.gz || exit 1
popd

cat tiled.spec | sed "s/^Version:\(.*\)/Version: $VERSION/" > ${BUILDROOT}/SPECS/tiled_tmp.spec
rpmbuild --define "_topdir ${BUILDROOT}/" -bs ${BUILDROOT}/SPECS/tiled_tmp.spec || exit 1

pushd ${BUILDROOT}
    FILE=$(find -name "*.src.rpm")
    cp $FILE ../tiled.src.rpm
popd
rm -rf $BUILDROOT

if [ "$1" = "srpm-only" ]
then
   echo "Wrote tiled.src.rpm. Exiting."
else
   COPR_CONFIG="
[copr-cli]
login = $1
username = $2
token = $3
copr_url = https://copr.fedorainfracloud.org
"
   copr --config <(echo "$COPR_CONFIG") build ablu/tiled-dailies tiled.src.rpm || exit 1

   rm tiled.src.rpm
fi
