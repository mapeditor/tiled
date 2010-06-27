# This include file currently only sets the default install prefix.
# See the README file for instructions.

PREFIX = $$PREFIX
isEmpty(PREFIX):PREFIX = /usr/local
