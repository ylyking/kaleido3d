SRC_DIR=$(dirname $0)/../../..
echo $SRC_DIR
CFG=Debug
BLD_DIR=$(dirname $0)/Intermediate/MacOS_$CFG

cmake -GXcode -H$SRC_DIR -B$BLD_DIR -DCMAKE_BUILD_TYPE=$CFG
cmake --build $BLD_DIR --config $CFG