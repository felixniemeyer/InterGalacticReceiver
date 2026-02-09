#!/bin/bash

pushd src/sketches
make || { echo "ERROR: make failed in src/sketches" >&2; exit 1; }
popd

make
cp assets/* bin
