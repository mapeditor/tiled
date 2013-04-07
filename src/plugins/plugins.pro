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
    message("Have Python, will slither")
    SUBDIRS += python
} else {
    !build_pass:message("No Python support")
}

