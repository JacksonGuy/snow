#!/bin/sh

set -e

if [ "$#" -ne 1 ]; then
    echo "Usage: $0 [debug|release]"
    exit 1
fi

case "$1" in
    debug)
        BUILD_TYPE=Debug
        ;;
    release)
        BUILD_TYPE=Release
        ;;
    *)
        echo "Invalid build type: $1"
        echo "Expected: debug or release"
        exit 1
        ;;
esac

BUILD_DIR="build/${BUILD_TYPE}"

cmake -S . -B "$BUILD_DIR" \
    -DCMAKE_BUILD_TYPE="$BUILD_TYPE"

cmake --build "$BUILD_DIR"
