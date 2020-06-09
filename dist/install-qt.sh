#!/usr/bin/env bash
#############################################################################
##
## Copyright (C) 2019 Richard Weickelt.
## Contact: https://www.qt.io/licensing/
##
## This file is part of Qbs.
##
## $QT_BEGIN_LICENSE:LGPL$
## Commercial License Usage
## Licensees holding valid commercial Qt licenses may use this file in
## accordance with the commercial license agreement provided with the
## Software or, alternatively, in accordance with the terms contained in
## a written agreement between you and The Qt Company. For licensing terms
## and conditions see https://www.qt.io/terms-conditions. For further
## information use the contact form at https://www.qt.io/contact-us.
##
## GNU Lesser General Public License Usage
## Alternatively, this file may be used under the terms of the GNU Lesser
## General Public License version 3 as published by the Free Software
## Foundation and appearing in the file LICENSE.LGPL3 included in the
## packaging of this file. Please review the following information to
## ensure the GNU Lesser General Public License version 3 requirements
## will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
##
## GNU General Public License Usage
## Alternatively, this file may be used under the terms of the GNU
## General Public License version 2.0 or (at your option) the GNU General
## Public license version 3 or any later version approved by the KDE Free
## Qt Foundation. The licenses are as published by the Free Software
## Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
## included in the packaging of this file. Please review the following
## information to ensure the GNU General Public License requirements will
## be met: https://www.gnu.org/licenses/gpl-2.0.html and
## https://www.gnu.org/licenses/gpl-3.0.html.
##
## $QT_END_LICENSE$
##
#############################################################################
set -eu

function help() {
    cat <<EOF
usage: install-qt [options] [components]

Examples
  ./install-qt.sh --version 5.12.4 qtbase
  ./install-qt.sh --version 5.12.4 --target android --toolchain android_arm64_v8a qtbase

Positional arguments
  components
        SDK components to be installed. All possible components
        can be found in the online repository at https://download.qt.io,
        for instance: qtbase, qtdeclarative, qtscript, qttools, icu, ...

        In addition to Qt packages, qtcreator is also accepted.

        The actual availability of packages may differ depending on
        target and toolchain.

Options
  --directory <directory>
        Root directory where to install the components.
        Maps to C:/Qt on Windows, /opt/Qt on Linux, /usr/local/Qt on Mac
        by default.

  --host <host-os>
        The host operating system. Can be one of linux_x64, mac_x64,
        windows_x86. Auto-detected by default.

  --target <target-platform>
        The desired target platform. Can be one of desktop, android, ios.
        The default value is desktop.

  --toolchain <toolchain-type>
        The toolchain that has been used to build the binaries.
        Possible values depend on --host and --target, respectively:

            linux_x64
                android
                    android_armv7, android_arm64_v8a
                desktop
                    gcc_64 (default)

            mac_x64
                android
                    android_armv7, android_arm64_v8a
                desktop
                    clang_64 (default),
                ios
                    ios

            windows_x86
                android
                    android_armv7, android_arm64_v8a
                desktop
                    win64_mingw73, win64_msvc2017_64 (default)

  --version <version>
        The desired Qt version. Currently supported are all versions
        above 5.9.0.

EOF
}

TARGET_PLATFORM=desktop
COMPONENTS=
VERSION=

case "$OSTYPE" in
    *linux*)
        HOST_OS=linux_x64
        INSTALL_DIR=/opt/Qt
        TOOLCHAIN=gcc_64
        ;;
    *darwin*)
        HOST_OS=mac_x64
        INSTALL_DIR=/usr/local/Qt
        TOOLCHAIN=clang_64
        ;;
    msys)
        HOST_OS=windows_x86
        INSTALL_DIR=/c/Qt
        TOOLCHAIN=win64_msvc2015_64
        ;;
    *)
        HOST_OS=
        INSTALL_DIR=
        ;;
esac

while [ $# -gt 0 ]; do
    case "$1" in
        --directory|-d)
            INSTALL_DIR="$2"
            shift
            ;;
        --host)
            HOST_OS="$2"
            shift
            ;;
        --target)
            TARGET_PLATFORM="$2"
            shift
            ;;
        --toolchain)
            TOOLCHAIN="$2"
            shift
            ;;
        --version)
            VERSION="$2"
            shift
            ;;
        --help|-h)
            help
            exit 0
            ;;
        *)
            COMPONENTS="${COMPONENTS} $1"
            ;;
    esac
    shift
done

if [ -z "${HOST_OS}" ]; then
    echo "No --host specified or auto-detection failed." >&2
    exit 1
fi

if [ -z "${INSTALL_DIR}" ]; then
    echo "No --directory specified or auto-detection failed." >&2
    exit 1
fi

if [ -z "${VERSION}" ]; then
    echo "No --version specified." >&2
    exit 1
fi

if [ -z "${COMPONENTS}" ]; then
    echo "No components specified." >&2
    exit 1
fi

case "$TARGET_PLATFORM" in
    android)
        ;;
    ios)
        ;;
    desktop)
        ;;
    *)
        echo "Error: TARGET_PLATFORM=${TARGET_PLATFORM} is not valid." >&2
        exit 1
        ;;
esac

DOWNLOAD_DIR=`mktemp -d 2>/dev/null || mktemp -d -t 'install-qt'`

#
# The repository structure is a mess. Try different URL variants
#
function compute_url(){
    local COMPONENT=$1
    local CURL="curl -s -L"
    local BASE_URL="http://download.qt.io/online/qtsdkrepository/${HOST_OS}/${TARGET_PLATFORM}"

    if [[ "${COMPONENT}" =~ "qtcreator" ]]; then

        REMOTE_BASE="tools_qtcreator/qt.tools.qtcreator"
        REMOTE_PATH="$(${CURL} ${BASE_URL}/${REMOTE_BASE}/ | grep -o -E "${VERSION}[0-9\-]*${COMPONENT}\.7z" | tail -1)"

        if [ ! -z "${REMOTE_PATH}" ]; then
            echo "${BASE_URL}/${REMOTE_BASE}/${REMOTE_PATH}"
            return 0
        fi

    else
        # New repository format (>=5.9.6)
        REMOTE_BASE="qt5_${VERSION//./}/qt.qt5.${VERSION//./}.${TOOLCHAIN}"
        REMOTE_PATH="$(${CURL} ${BASE_URL}/${REMOTE_BASE}/ | grep -o -E "[[:alnum:]_.\-]*7z" | grep "${COMPONENT}" | tail -1)"
        echo "$BASE_URL/$REMOTE_BASE/$REMOTE_PATH" >&2

        if [ ! -z "${REMOTE_PATH}" ]; then
            echo "${BASE_URL}/${REMOTE_BASE}/${REMOTE_PATH}"
            return 0
        fi

        REMOTE_BASE="qt5_${VERSION//./}/qt.qt5.${VERSION//./}.${COMPONENT}.${TOOLCHAIN}"
        REMOTE_PATH="$(${CURL} ${BASE_URL}/${REMOTE_BASE}/ | grep -o -E "[[:alnum:]_.\-]*7z" | grep "${COMPONENT}" | tail -1)"

        if [ ! -z "${REMOTE_PATH}" ]; then
            echo "${BASE_URL}/${REMOTE_BASE}/${REMOTE_PATH}"
            return 0
        fi

        # Older repository format (>=5.9.0)
        REMOTE_BASE="qt5_${VERSION//./}/qt.${VERSION//./}.${TOOLCHAIN}"
        REMOTE_PATH="$(${CURL} ${BASE_URL}/${REMOTE_BASE}/ | grep -o -E "[[:alnum:]_.\-]*7z" | grep "${COMPONENT}" | tail -1)"

        if [ ! -z "${REMOTE_PATH}" ]; then
            echo "${BASE_URL}/${REMOTE_BASE}/${REMOTE_PATH}"
            return 0
        fi

        REMOTE_BASE="qt5_${VERSION//./}/qt.${VERSION//./}.${COMPONENT}.${TOOLCHAIN}"
        REMOTE_PATH="$(${CURL} ${BASE_URL}/${REMOTE_BASE}/ | grep -o -E "[[:alnum:]_.\-]*7z" | grep "${COMPONENT}" | tail -1)"

        if [ ! -z "${REMOTE_PATH}" ]; then
            echo "${BASE_URL}/${REMOTE_BASE}/${REMOTE_PATH}"
            return 0
        fi

    fi

    echo "Could not determine a remote URL for ${COMPONENT} with version ${VERSION}">&2
    exit 1
}

mkdir -p ${INSTALL_DIR}

for COMPONENT in ${COMPONENTS}; do

    URL="$(compute_url ${COMPONENT})"

    echo "Downloading ${COMPONENT}..." >&2
    curl --progress-bar -L -o ${DOWNLOAD_DIR}/package.7z ${URL} >&2
    7z x -y -o${INSTALL_DIR} ${DOWNLOAD_DIR}/package.7z >/dev/null 2>&1
    rm -f ${DOWNLOAD_DIR}/package.7z

    #
    # conf file is needed for qmake
    #
    if [ "${COMPONENT}" == "qtbase" ]; then
        if [[ "${TOOLCHAIN}" =~ "win64_mingw" ]]; then
            SUBDIR="${TOOLCHAIN/win64_/}_64"
        elif [[ "${TOOLCHAIN}" =~ "win32_mingw" ]]; then
            SUBDIR="${TOOLCHAIN/win32_/}_32"
        elif [[ "${TOOLCHAIN}" =~ "win64_msvc" ]]; then
            SUBDIR="${TOOLCHAIN/win64_/}"
        elif [[ "${TOOLCHAIN}" =~ "win32_msvc" ]]; then
            SUBDIR="${TOOLCHAIN/win32_/}"
        else
            SUBDIR="${TOOLCHAIN}"
        fi

        CONF_FILE="${INSTALL_DIR}/${VERSION}/${SUBDIR}/bin/qt.conf"
        echo "[Paths]" > ${CONF_FILE}
        echo "Prefix = .." >> ${CONF_FILE}

        # Print the directory so that the caller can
        # adjust the PATH variable.
        echo $(dirname "${CONF_FILE}")
    elif [[ "${COMPONENT}" =~ "qtcreator" ]]; then
        echo "${INSTALL_DIR}/Tools/QtCreator/bin"
    fi

done
