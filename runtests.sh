#!/bin/bash

ROOT=$(pwd)
BUILD_DIR=$ROOT/build/
RESOURCE_DIR=$ROOT/resources/

rm -rf $BUILD_DIR
mkdir $BUILD_DIR && cd $BUILD_DIR

cmake .. -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=1
make

rc -J $BUILD_DIR/compile_commands.json

$BUILD_DIR/src/main "$RESOURCE_DIR/test.html"

