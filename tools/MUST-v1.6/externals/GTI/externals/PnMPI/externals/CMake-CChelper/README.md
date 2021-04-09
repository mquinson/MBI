# CMake-CChelper

[![](https://img.shields.io/github/issues-raw/alehaa/CMake-CChelper.svg?style=flat-square)](https://github.com/alehaa/CMake-CChelper/issues)
[![3-clause BSD license](http://img.shields.io/badge/license-3_clause_BSD-blue.svg?style=flat-square)](LICENSE)
![](http://img.shields.io/badge/CMake_required-2.6-lightgrey.svg?style=flat-square)

CMake module to enable compiler features depending on the used compiler.



## Include into your project

To use [CCHelper.cmake](cmake/CChelper.cmake), simply add this repository as git submodule into your own repository
```Shell
mkdir externals
git submodule add git://github.com/alehaa/CMake-CChelper.git externals/CMake-CChelper
```
and adding ```externals/CMake-CChelper/cmake``` to your ```CMAKE_MODULE_PATH```
```CMake
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/externals/CMake-CChelper/cmake" ${CMAKE_MODULE_PATH})
```

If you don't use git or dislike submodules you can copy the [CCHelper.cmake](cmake/CChelper.cmake) file into your repository. *Be careful when there are version updates of this repository!*


## Usage

The Usage of [CCHelper.cmake](cmake/CChelper.cmake) is really easy: simply include [CCHelper.cmake](cmake/CChelper.cmake) before you call ```add_subdirectory``` for your source directory. See the examples below.


The following options will be added to your CMake project:

| option | description |default value|
|--------|-------------|-------------|
|```ENABLE_WARNINGS```|Selects whether compiler warnings are enabled.|On|
|```ENABLE_WARNINGS_TO_ERRORS```|Selects whether warnings should be converted to errors.|On|
|```ENABLE_PEDANTIC```|Selects whether pedantic warnings are enabled.|On|

To change the default values you can set the following variables **before** you include [CCHelper.cmake](cmake/CChelper.cmake) to true or false:

| variable | description |
|--------|-------------|
|```CCHELPER_ENABLE_WARNINGS```|Default value for ```ENABLE_WARNINGS```.|
|```CCHELPER_ENABLE_WARNINGS_TO_ERRORS```|Default value for ```ENABLE_WARNINGS_TO_ERRORS```.|
|```CCHELPER_ENABLE_PEDANTIC```|Default value for ```ENABLE_PEDANTIC```.|

If you want to define own compiler flags, you have to change nothing for this: All flags set by this module will be appended only to ```CMAKE_${lang}_FLAGS```.

```CMake
# setup CChelper
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/externals/CMake-CChelper/cmake" ${CMAKE_MODULE_PATH})
include(CChelper)

# recurse into subdirectories
add_subdirectory(src)
```

An example with default values:

```CMake
# setup CChelper
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/externals/CMake-CChelper/cmake" ${CMAKE_MODULE_PATH})
set(CCHELPER_ENABLE_PEDANTIC false)
include(CChelper)

# recurse into subdirectories
add_subdirectory(src)
```

#### Optional modules

**C99**:

Use ``find_package(C99)`` to find C99 compile flags for your compiler. If C99 compile flags were found, ``C99_FOUND`` will be set to true and you can add ``C99_FLAGS`` to the targets compile flags. The ``REQUIRED`` flag of ``find_package`` is supported.

```CMake
# setup CChelper C99 module
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/externals/CMake-CChelper/cmake" ${CMAKE_MODULE_PATH})
find_package(C99 REQUIRED)

add_definitions(${C99_FLAGS}) # Enable C99 for all targets.
add_executable(c99-test test.c)
```


**C11**:

Use `find_package(C11)` to find C11 compile flags. If C11 compile flags have been found, `C11_FOUND` will be set to true and you can add `C11_FLAGS` to the targets compile flags. You may search for the following optional C11 features with the `COMPONENTS` or `OPTIONAL_COMPONENTS` flag in `find_package`:

* `ANALYZABLE`: Analyzability (Annex L)
* `ATOMICS`: Atomic primitives and types (`<stdatomic.h>` and the `_Atomic` type qualifier)
* `COMPLEX`: Complex types (`<complex.h>`)
* `IEC_559`: IEC 60559 floating-point arithmetic (Annex F)
* `IEC_559_COMPLEX`: IEC 60559 compatible complex arithmetic (Annex G)
* `THREADS`: Multithreading (`<threads.h>`)
* `THREAD_LOCAL`: `_Thread_local` type qualifier
* `VLA`: Variable length arrays


**BuiltinAtomic**:

Use `find_package(BuiltinAtomic)` to check if the compiler has support for the `__atomic` functions. `BUILTINATOMIC_FOUND` will be set to true, if the compiler supports them.


**BuiltinSync**:

Use `find_package(BuiltinSync)` to check if the compiler has support for the `__sync` functions. `BUILTINSYNC_FOUND` will be set to true, if the compiler supports them.


**BuiltinAtomic**:

Use `find_package(ThreadKeyword)` to check if the compiler has support for the `__thread` storage class keyword. `THREADKEYWORD_FOUND` will be set to true, if the compiler supports it.


## Copyright

Copyright &copy; 2015-2017 [Alexander Haase](alexander.haase@rwth-aachen.de).

See the [LICENSE](LICENSE) file in the base directory for details.
