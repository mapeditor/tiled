# This file is largely based on the way translations are compiled in
# translations.pro from Qt Creator.
#
# Translation howto:
# - Translations are mentioned in the LANGUAGES variable below
# - To update the .ts files, cd into translations and run 'make ts'
# - The .qm files are created as part of a regular make command
#

# The list of supported translations
LANGUAGES = nl pt es de pt_BR ja fr it cs lv

# Helper function to adapt the slashes in a path to the system
defineReplace(fixSlashes) {
    win32:1 ~= s|/|\|g
    return($$1)
}

# Helper function prepending and appending text to all values
# Usage: var, prepend, append
defineReplace(prependAppend) {
    for(a,$$1):result += $$2$${a}$$3
    return($$result)
}

# Large hack to make sure this pro file does not try to compile an application
TEMPLATE = app
TARGET = phony_target
CONFIG -= qt
QT =
LIBS =
QMAKE_LINK = @: IGNORE THIS LINE
OBJECTS_DIR =
win32:CONFIG -= embed_manifest_exe

TRANSLATIONS = $$prependAppend(LANGUAGES, $$PWD/tiled_, .ts)
LUPDATE = $$fixSlashes($$[QT_INSTALL_BINS]/lupdate) -locations relative
LRELEASE = $$fixSlashes($$[QT_INSTALL_BINS]/lrelease)

ts.commands = cd .. && $$LUPDATE src -ts $$TRANSLATIONS
QMAKE_EXTRA_TARGETS += ts

updateqm.input = TRANSLATIONS
updateqm.output = ${QMAKE_FILE_BASE}.qm
isEmpty(vcproj):updateqm.variable_out = PRE_TARGETDEPS
updateqm.commands = $$LRELEASE ${QMAKE_FILE_IN} -qm ${QMAKE_FILE_OUT}
updateqm.name = LRELEASE ${QMAKE_FILE_IN}
updateqm.CONFIG += no_link
QMAKE_EXTRA_COMPILERS += updateqm
