
# This file is a part of Simple-XX/SimpleRenderer (https://github.com/Simple-XX/SimpleRenderer).
#
# setup.sh for Simple-XX/SimpleRenderer.

#!/bin/bash
# shell 执行出错时终止运行
set -e
# 输出实际执行内容
# set -x

# 重新编译
mkdir -p ./build/
rm -rf ./build/*
cd ./build
cmake ..
make
cd ./bin

./renderer

open ./output.tga
