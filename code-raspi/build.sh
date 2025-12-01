#!/bin/bash

pushd src/sketches
make
popd

make
cp assets/* bin
