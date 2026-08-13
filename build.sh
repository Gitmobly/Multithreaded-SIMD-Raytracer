#!/usr/bin/env bash

set -e

BUILD_DIR="build"
SRC_DIR="src"

CONFIG="debug"
if [[ "$1" == "-release" ]]; then
    CONFIG="release"
fi

mkdir -p "$BUILD_DIR"

CLFLAGS=(
    -std=c++20
    -Wall
    -Wextra
    -Wconversion
    -Wno-unused-function
    -Wno-missing-braces
    -Wno-missing-field-initializers
    -Wno-deprecated-declarations
)

if [[ "$CONFIG" == "release" ]]; then
    CONFIGFLAGS=(
        -DNDEBUG
        -O2
        -mavx2
    )
else
    CONFIGFLAGS=(
        -DDEBUG_BUILD
        -DTESTS_ON
        -g
        -O0
    )
fi

mapfile -t SOURCES < <(find "$SRC_DIR" -name "*.cpp")

echo "Building $CONFIG..."

clang++ \
    "${CLFLAGS[@]}" \
    "${CONFIGFLAGS[@]}" \
    -I"$SRC_DIR" \
    "${SOURCES[@]}" \
    -o "$BUILD_DIR/main"
