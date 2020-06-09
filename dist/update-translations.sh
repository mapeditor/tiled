# Updates the .ts files based on the strings found in the source code
# Meant to be run from repository root
lupdate -locations relative -no-obsolete src -ts translations/*.ts
