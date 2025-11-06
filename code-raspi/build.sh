#!/bin/bash

mkdir -p ./bin

g++ main.cpp canvas_ity.cpp \
    -o ./bin/i2c_test \
    -std=c++11

