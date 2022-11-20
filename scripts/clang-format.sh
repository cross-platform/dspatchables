#!/bin/bash

DIR=$(cd `dirname $0` && pwd)
cd $DIR/..

find Components -iname *.h -o -iname *.cpp | grep -v kissfft | grep -v Mongoose | grep -v RtAudio | xargs clang-format --style=file --verbose -i
