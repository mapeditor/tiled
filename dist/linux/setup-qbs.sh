if [ "$CXX" == "" ]
then
    CXX=g++
fi

# - Not using Qbs 1.12.0, 1.12.1 or 1.12.2 because of a false "Duplicate source file" error (QBS-1416)
# - Make sure not to build against Qt 5.12 to avoid build issues on macOS (QBS-1417)

git clone --depth 1 -b v1.11.1 git://code.qt.io/qbs/qbs.git qbs-build
pushd qbs-build
qmake -r qbs.pro QMAKE_CXX=$CXX QMAKE_LINK=$CXX \
                 "CONFIG+=qbs_no_dev_install qbs_no_man_install" \
                 && make
export PATH=$PATH:$PWD/bin
popd
