#!/bin/bash 
SRC_DIR=$(dirname $0)/../../..
echo $SRC_DIR
CFG=Debug
BLD_DIR=$(dirname $0)/Intermediate/$CFG

cmake -H$SRC_DIR -B$BLD_DIR -G"Unix Makefiles" -DCMAKE_BUILD_TYPE=$CFG -DBUILD_WITH_UNIT_TEST=OFF
cmake --build $BLD_DIR --config $CFG
