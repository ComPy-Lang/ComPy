#!/usr/bin/env xonsh

# A platform independent Xonsh script to build ComPy. Works on Linux and MacOS.

$RAISE_SUBPROC_ERROR = True
trace on

echo "CONDA_PREFIX=$CONDA_PREFIX"
echo "PATH=$PATH"

# Generate the `version` file
bash ci/version.sh

# Generate a Fortran AST from AST.asdl (C++)
python grammar/asdl_cpp.py
# Generate a Fortran ASR from ASR.asdl (C++)
python grammar/asdl_cpp.py src/libasr/ASR.asdl src/libasr/asr.h

# Generate the tokenizer and parser
pushd src/compy/parser && re2c -W -b tokenizer.re -o tokenizer.cpp && popd
pushd src/compy/parser && bison -Wall -d -r all parser.yy && popd

cmake -G $LFORTRAN_CMAKE_GENERATOR -DCMAKE_VERBOSE_MAKEFILE=ON -DWITH_LLVM=yes -DCMAKE_PREFIX_PATH=$CONDA_PREFIX -DCMAKE_INSTALL_PREFIX=$CONDA_PREFIX -DWITH_LFORTRAN_BINARY_MODFILES=no .
cmake --build . --target install
