#!/bin/bash

pushd "$1" > /dev/null || { echo "Failed to cd into $1"; exit 1; }

awk '
/^[[:space:]]*SRC[[:space:]]+/ {
    sub(/^[[:space:]]*SRC[[:space:]]+/, "", $0)
    filename = $0

    if ((getline line < filename) < 0) {
        missing = 1
        next
    }

    print line
    while ((getline line < filename) > 0)
        print line

    close(filename)
    next
}
{ print }

END {
    if (missing)
        exit 1
}
' shader_template.h > shaders.h

rc=$?

popd > /dev/null

if [ "$rc" -eq 1 ]; then
    echo "ERROR: One or more SRC files do not exist or cannot be read" >&2
    exit 1
fi

exit 0
