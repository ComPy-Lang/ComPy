#!/usr/bin/env bash

set -e
set -x

# Generate the `version` file
ci/version.sh

# Generate a Fortran AST from AST.asdl (C++)
python grammar/asdl_cpp.py
# Generate a Fortran ASR from ASR.asdl (C++)
python grammar/asdl_cpp.py grammar/ASR.asdl src/libasr/asr.h

# Generate the tokenizer and parser
(cd src/compy/parser && re2c -W -b tokenizer.re -o tokenizer.cpp)
(cd src/compy/parser && bison -Wall -d -r all parser.yy)

grep -n "'" src/compy/parser/parser.yy && echo "Single quote not allowed" && exit 1
echo "OK"
