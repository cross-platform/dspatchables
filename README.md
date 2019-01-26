[![Build Status](https://travis-ci.org/MarcusTomlinson/DSPatchables.svg?branch=master)](https://travis-ci.org/MarcusTomlinson/DSPatchables)
[![Build status](https://ci.appveyor.com/api/projects/status/7lixlpl0699oxb73/branch/master?svg=true)](https://ci.appveyor.com/project/MarcusTomlinson/dspatchables/branch/master)

# DSPatchables

DSPatch Component Repository


## Build

```
git clone https://github.com/cross-platform/dspatchables.git
cd dspatchables
git submodule update --init --recursive --remote
mkdir build
cd build
cmake ..
make
```

- *`cmake ..` will auto-detect your IDE / compiler. To manually select one, use `cmake -G`.*
- *When building for an IDE, instead of `make`, simply open the cmake generated project file.*


### See also:

DSPatch (https://github.com/cross-platform/dspatch): A powerful C++ dataflow framework.

DSPatcher (https://github.com/cross-platform/dspatcher): A cross-platform graphical tool for building DSPatch circuits.
