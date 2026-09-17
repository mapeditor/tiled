// This file declares the QML modules that are shipped with Tiled for use by
// QML extensions. It is scanned by macdeployqt (through the -qmldir option)
// and by linuxdeploy-plugin-qt (through QML_SOURCES_PATHS) to determine
// which QML modules to deploy.
//
// The Windows packages derive the list of shipped files from the
// qmlImportDirs and qtQuickLibraries properties in tiled.qbs instead, so
// these should be kept in sync.

import QtQml
import QtQml.Models
import QtQml.WorkerScript
import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Basic
import QtQuick.Controls.Fusion
import QtQuick.Layouts
import QtQuick.Templates
import QtQuick.Window

QtObject {}
