#!/usr/bin/env xonsh

# A platform independent Xonsh script to build ComPy. Works on Linux and MacOS.

$RAISE_SUBPROC_ERROR = True
trace on

echo "CONDA_PREFIX=$CONDA_PREFIX"
echo "PATH=$PATH"
llvm-config --components

# Generate the `version` file
bash ci/version.sh

# Generate a Fortran ASR from ASR.asdl (C++)
python grammar/asdl_cpp.py grammar/ASR.asdl src/libasr/asr.h

# Generate the tokenizer
pushd src/compy/parser && re2c -W -b tokenizer.re -o tokenizer.cpp && popd

