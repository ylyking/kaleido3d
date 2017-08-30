SRC_DIR=$(dirname $0)/../../..
echo $SRC_DIR
CFG=Debug
BLD_DIR=$(dirname $0)/Intermediate/iOS_$CFG
# build projects
cmake -GXcode -H$SRC_DIR -B$BLD_DIR -DCMAKE_BUILD_TYPE=$CFG -DCMAKE_TOOLCHAIN_FILE=$SRC_DIR/Project/ios.cmake -DIOS_PLATFORM=OS
cmake --build $BLD_DIR --config $CFG