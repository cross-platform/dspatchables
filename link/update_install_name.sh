#!/bin/bash
dir=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
install_name_tool -id $dir/libDSPatch.dylib $dir/libDSPatch.dylib
