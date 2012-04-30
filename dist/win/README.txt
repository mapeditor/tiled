
How to create a Windows installer for Tiled
===============================================================================
Prerequisites:
	* Qt is installed
	* MinGW is installed
	* NSIS in installed
	* Tiled executable in release mode has already been built

Tiled's windows installer is created using NSIS. You thus need to
install NSIS first if you want to build your custom windows installer for it.
You can download the latest version of the installer from the NSIS project
website located at http://nsis.sourceforge.net. Another requirement is that the
project be already been built.

The installer script is called "tiled.nsi" and is located in the dist/win
directory. In order for the script to correctly produce binaries we do however
need to modify the shell environment from which the script is run first. This
is due to the fact that a successful compilation depends on some variables
like the actual location of the Qt or MinGW installation path being correctly
set. These values can not be automatically inferred by the installer script.

Three mandatory variables "QTDIR", "MINGW" and "VERSION" need to be set to the
correct paths of their respective packages. If VERSION is not set it will try
to read the content of a file "version.txt" located in the root directory of
tiled into that variable. The commands to set those may look like the
following:
set QTDIR="C:\Qt\4.8.1"
set MINGW="C:\MinGW"
set VERSION="0.8.1"

Optionally you can also set the program architecture which is then used to
deduce the resulting installer filename. It can either be 32 or 64 and defaults
to 32:
set ARCH="32"

After setting the above variables to the correct values you can compile the
NSIS installer script from the command line by executing the provided batch
script "build_installer.bat". Make sure the nsis compiler is in PATH so it can
be found by the command line interpreter. The resulting installer will be
placed in the same directory where the nsis script was located and have a name
similar to "setup-tiled-<VERSION>-<ARCH>.exe", where <VERSION> is replaced with
the value of the "VERSION" variable explained above and <ARCH> will - depending
on the actual value of the "ARCH" variable -  reflect the architecture
(win32 or win64) for which the installer is built.
