TEMPLATE = subdirs
SUBDIRS = flare \
          droidcraft \
          json \
          lua \
          tengine \
          tmw \
          replicaisland

include(python/find_python.pri)

contains(HAVE_PYTHON, yes) {
    SUBDIRS += python
} else {
    !build_pass:system(echo "No Python support")
}
