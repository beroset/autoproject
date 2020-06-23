#!/bin/bash
# create the project from the passed md file and return 0 if OK
@autoproject@ --forceoverwrite --configfile "@CMAKE_BINARY_DIR@/autoprojecttest.conf" "$1"
