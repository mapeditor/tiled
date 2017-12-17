if [ "$CXX" == "" ]
then
    CXX=g++
fi

git clone --depth 1 -b v1.9.1 https://github.com/qbs/qbs.git qbs-build
pushd qbs-build
qmake -r qbs.pro QMAKE_CXX=$CXX QMAKE_LINK=$CXX && make
export PATH=$PATH:$PWD/bin
popd
