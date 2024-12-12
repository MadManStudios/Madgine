#!/bin/bash

cd /emsdk
dos2unix /emsdk/emsdk
dos2unix /emsdk/emsdk_env.sh

source /emsdk/emsdk_env.sh

cd /python-wasm/cpython

python3 ./Tools/wasm/emscripten build