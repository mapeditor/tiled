TEMPLATE = subdirs
SUBDIRS = csv \
          defold \
          droidcraft \
          flare \
          gmx \
          json \
          lua \
          replicaisland \
          tbin \
          tengine

include(python/find_python.pri)

contains(HAVE_PYTHON, yes) {
    message("Have Python, will slither")
    SUBDIRS += python
} else {
    !build_pass:message("No Python support")
}

