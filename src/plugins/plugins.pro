TEMPLATE = subdirs
SUBDIRS = csv \
          defold \
          defoldcollection \
          droidcraft \
          flare \
          gmx \
          json \
          json1 \
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

